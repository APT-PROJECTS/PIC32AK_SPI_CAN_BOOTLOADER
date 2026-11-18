#include "definitions.h"
#include "driver.h"
#include "middleware.h"

#define FLASH_CHUNK_SIZE    1024U
#define FLASH_ROW_SIZE      (NVM_FLASH_ROWSIZE * sizeof(uint32_t))
#define BOOT_FLASH_PAGE_SIZE (NVM_FLASH_PAGESIZE * sizeof(uint32_t))

#define BOOT_STAY_WINDOW_COUNT    2000UL

#define BOOT_CAN_CMD_3_SET_OPERATION(operation) \
        ((uint8_t)(((operation) & 0x07U) << 5U))

#define BOOT_CAN_CMD_3_SET_STATUS(status) \
        ((uint8_t)(((status) & 0x03U) << 3U))

#define BOOT_CAN_CMD_3_GET_OPERATION(value) \
        ((uint8_t)(((value) >> 5U) & 0x07U))

#define BOOT_CAN_CMD_3_GET_STATUS(value) \
        ((uint8_t)(((value) >> 3U) & 0x03U))

static uint8_t flashBuffer[FLASH_CHUNK_SIZE];
static uint32_t flashBufferIndex = 0;
static uint32_t flashWriteAddress = APPLICATION_START_ADDRESS;
static bool firmwareUpdateActive = false;
static bool firmwareUpdateFailed = false;
static bool firmwareImageReady = false;
static bool firmwareCrcVerified = false;
static volatile bool bootStayRequested = false;
static uint32_t firmwareExpectedSize = 0U;
static uint8_t expectedChunkIndex = 1U;
static uint8_t lastAcceptedChunkIndex = 0U;

static uint8_t canRxBuffer[8] = {0U};

typedef void (*APP_ENTRY_FUNC)(void);

void Boot_Init(void) {
    uint32_t ctr = 0U;
    
    if (strcmp(stayInBootFlag, "1") == 0) {
        strcpy(stayInBootFlag, "0");

        Get_Software_ID();
        Get_Firmware_ID();
        Build_Metadata();

        __builtin_memset(&eepromBuffer[EEPROM_METADATA_OFFSET], 0xFF,
                EEPROM_METADATA_SIZE);

        __builtin_memcpy(&eepromBuffer[EEPROM_METADATA_OFFSET],
                metadata,
                METADATA_LENGTH);

        Flash_EEPROM_Write_Page(eepromBuffer, EEPROM_DATA_SIZE);
    } else {
        while ((ctr < BOOT_STAY_WINDOW_COUNT) &&
                !Boot_Stay_In_Boot_Requested()) {
            ctr++;
            MCP2515_Task();
            CANbus_read();
        }

        if (!Boot_Stay_In_Boot_Requested() &&
                (Boot_Is_App_Present() == true)) {
            Boot_App_Jump();
        }
    }
}

void Boot_PIC_ID_Handler(void) {
    uint8_t txBuf[8] = {0U};
    uint8_t operation;
    uint32_t picid;

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_PIC_ID) {
        return;
    }

    operation = BOOT_CAN_CMD_3_GET_OPERATION(canRxBuffer[3]);

    switch (operation) {
            /*------------------------- READ --------------------------*/
        case BOOT_CAN_CMD_3_OPERATION_READ:

            txBuf[0] = BOOT_CAN_CMD_0_PIC_ID;
            txBuf[1] = canRxBuffer[1];
            txBuf[2] = 0x00;

            /* Check if PIC ID is available */
            if ((udid[0] == 0U) &&
                    (udid[1] == 0U) &&
                    (udid[2] == 0U) &&
                    (udid[3] == 0U)) {
                txBuf[3] = BOOT_CAN_CMD_3_SET_OPERATION(operation);
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_FAIL);
            } else {
                txBuf[3] = BOOT_CAN_CMD_3_SET_OPERATION(operation);
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_SUCCESS);
            }

            switch (canRxBuffer[1]) {
                case BOOT_CAN_CMD_1_PIC_MAC_ID_1:
                    picid = udid[0];
                    break;

                case BOOT_CAN_CMD_1_PIC_MAC_ID_2:
                    picid = udid[1];
                    break;

                case BOOT_CAN_CMD_1_PIC_MAC_ID_3:
                    picid = udid[2];
                    break;

                case BOOT_CAN_CMD_1_PIC_MAC_ID_4:
                    picid = udid[3];
                    break;

                case BOOT_CAN_CMD_1_PIC_DEVICE_ID:
                    picid = devid;
                    break;

                default:
                    return;
            }

            txBuf[4] = (uint8_t) (picid >> 24);
            txBuf[5] = (uint8_t) (picid >> 16);
            txBuf[6] = (uint8_t) (picid >> 8);
            txBuf[7] = (uint8_t) (picid);

            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            break;

        default:
            break;
    }
}

/* Read one four-byte metadata chunk, using the same protocol as tuner CAN. */
void Boot_Metadata_Handler(void) {
    uint8_t txBuf[8] = {0U};
    uint8_t operation;
    uint8_t chunkNumber;
    uint8_t chunkCount;
    uint16_t metadataOffset;
    uint16_t i;
    bool metadataEmpty = true;

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_METADATA) {
        return;
    }

    operation = BOOT_CAN_CMD_3_GET_OPERATION(canRxBuffer[3]);
    chunkNumber = canRxBuffer[1];
    /* Reserve the final metadata byte for internal use. */
    chunkCount = (METADATA_LENGTH - 1U) / CAN_FRAME_DATA_SIZE;

    if ((chunkNumber == 0U) ||
            (chunkNumber > chunkCount)) {
        return;
    }

    metadataOffset = (uint16_t) (chunkNumber - 1U) * CAN_FRAME_DATA_SIZE;
    txBuf[0] = BOOT_CAN_CMD_0_METADATA;
    txBuf[1] = chunkNumber;
    txBuf[2] = 0x00U;

    switch (operation) {
        case BOOT_CAN_CMD_3_OPERATION_READ:
            for (i = 0U; i < (METADATA_LENGTH - 1U); i++) {
                if ((uint8_t) metadata[i] != 0x00U) {
                    metadataEmpty = false;
                    break;
                }
            }

            txBuf[3] = BOOT_CAN_CMD_3_SET_OPERATION(operation);
            txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                    (metadataEmpty ? BOOT_CAN_CMD_3_STATUS_FAIL :
                    BOOT_CAN_CMD_3_STATUS_SUCCESS);

            if (!metadataEmpty) {
                for (i = 0U; (i < CAN_FRAME_DATA_SIZE) &&
                        ((metadataOffset + i) < (METADATA_LENGTH - 1U)); i++) {
                    txBuf[4U + i] = (uint8_t) metadata[metadataOffset + i];
                }
            }
            break;

        default:
            return;
    }

    CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
}

void Boot_Status_Handler(void) {
    uint8_t txBuf[8] = {0U};
    uint8_t operation;
    uint32_t addr;

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_STATUS) {
        return;
    }

    operation = BOOT_CAN_CMD_3_GET_OPERATION(canRxBuffer[3]);

    switch (operation) {
            /*------------------------- READ --------------------------*/
        case BOOT_CAN_CMD_3_OPERATION_READ:

            /* Get the address of this function */
            addr = (uint32_t) & Boot_Status_Handler;

            txBuf[0] = BOOT_CAN_CMD_0_STATUS;
            txBuf[1] = canRxBuffer[1];
            txBuf[2] = 0x00U;
            txBuf[3] = BOOT_CAN_CMD_3_SET_OPERATION(operation);
            txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                    (BOOT_CAN_CMD_3_STATUS_SUCCESS);

            if ((addr >= BOOTLOADER_START_ADDRESS) &&
                    (addr <= BOOTLOADER_END_ADDRESS)) {
                txBuf[1] = BOOT_CAN_CMD_1_STATUS_BOOT;
            } else if ((addr >= APPLICATION_START_ADDRESS) &&
                    (addr <= APPLICATION_END_ADDRESS)) {
                txBuf[1] = BOOT_CAN_CMD_1_STATUS_APP;
            } else {
                txBuf[1] = BOOT_CAN_CMD_1_STATUS_UNKNOWN;
            }

            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            break;

        default:
            return;
    }
}

void Boot_App_Size_Handler(void) {
    uint8_t txBuf[8] = {0U};
    uint8_t operation;
    uint32_t firmwareSize;
    uint32_t maxAppSize;

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_APP_SIZE) {
        return;
    }

    operation = BOOT_CAN_CMD_3_GET_OPERATION(canRxBuffer[3]);

    switch (operation) {
            /*----------------------- COMPARE -------------------------*/
        case BOOT_CAN_CMD_3_OPERATION_COMPARE:

            /* Extract firmware size (bytes) from CAN bytes 4-7 */
            firmwareSize = ((uint32_t) canRxBuffer[4] << 24) |
                    ((uint32_t) canRxBuffer[5] << 16) |
                    ((uint32_t) canRxBuffer[6] << 8) |
                    ((uint32_t) canRxBuffer[7]);

            /* Maximum allowed firmware size = Application partition - 4 KB */
            maxAppSize = (APPLICATION_END_ADDRESS - APPLICATION_START_ADDRESS + 1U) - (1U * 1024U);

            txBuf[0] = BOOT_CAN_CMD_0_APP_SIZE;
            txBuf[1] = 0x00U;
            txBuf[2] = 0x00U;
            txBuf[3] = BOOT_CAN_CMD_3_SET_OPERATION(operation);

            firmwareExpectedSize = 0U;

            if ((firmwareSize > 0U) && (firmwareSize < maxAppSize)) {
                /* CRC covers only this exact file size, never CAN padding. */
                firmwareExpectedSize = firmwareSize;
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_SUCCESS);
            } else {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_FAIL);
            }

            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            break;

            /*------------------------- WRITE -------------------------*/
        case BOOT_CAN_CMD_3_OPERATION_WRITE:

            __builtin_memcpy(appSize, &canRxBuffer[4], APP_SIZE);
            appSize[APP_SIZE] = '\0';

            Get_Software_ID();
            Get_Firmware_ID();
            Build_Metadata();

            __builtin_memset(&eepromBuffer[EEPROM_METADATA_OFFSET], 0xFF,
                    EEPROM_METADATA_SIZE);
            __builtin_memcpy(&eepromBuffer[EEPROM_METADATA_OFFSET], metadata,
                    METADATA_LENGTH);

            txBuf[0] = BOOT_CAN_CMD_0_APP_SIZE;
            txBuf[1] = 0x00;
            txBuf[2] = 0x00;
            txBuf[3] = BOOT_CAN_CMD_3_SET_OPERATION(operation);

            if (Flash_EEPROM_Write_Page(eepromBuffer,
                    EEPROM_DATA_SIZE) == FLASH_EEPROM_OK) {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_SUCCESS);
            } else {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_FAIL);
            }

            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            break;

        default:
            break;
    }
}

bool Boot_Is_App_Present(void) {
    uint32_t appEntryAddress;

    appEntryAddress = *(volatile const uint32_t *) APPLICATION_START_ADDRESS;

    if ((appEntryAddress == 0xFFFFFFFFUL) ||
            (appEntryAddress == 0x00000000UL)) {
        return false;
    }

    if ((appEntryAddress < APPLICATION_START_ADDRESS) ||
            (appEntryAddress > APPLICATION_END_ADDRESS)) {
        return false;
    }

    return true;
}

void Boot_App_Jump(void) {
    APP_ENTRY_FUNC appEntry;
    uint32_t appEntryAddress;

    if (Boot_Is_App_Present() == false) {
        return;
    }

    appEntryAddress = *(volatile const uint32_t *) APPLICATION_START_ADDRESS;
    appEntry = (APP_ENTRY_FUNC) appEntryAddress;

    /* Disable Global Interrupts */
    (void) __builtin_disable_interrupts();

    /* Disable all Interrupt Sources */
    IEC0 = 0;
    IEC1 = 0;
    IEC2 = 0;
    IEC3 = 0;

    /* Clear Pending Interrupt Flags */
    IFS0 = 0;
    IFS1 = 0;
    IFS2 = 0;
    IFS3 = 0;

    /* Set Application IVT */
    __builtin_setIVTBASE((void *) APPLICATION_IVT_ADDRESS);

    /* Jump to application */
    appEntry();

    while (1) {
        /* Should never return */
    }
}

void Boot_Jump_Handler(void) {
    uint8_t txBuf[8] = {0U};
    uint32_t addr;

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_JUMP) {
        return;
    }

    addr = (uint32_t) & Boot_Jump_Handler;

    txBuf[0] = BOOT_CAN_CMD_0_JUMP;
    txBuf[1] = canRxBuffer[1];
    txBuf[2] = 0x00U;

    switch (canRxBuffer[1]) {
        case BOOT_CAN_CMD_1_BOOT_JUMP:

            /* Already running in Bootloader */
            if ((addr >= BOOTLOADER_START_ADDRESS) &&
                    (addr <= BOOTLOADER_END_ADDRESS)) {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_SUCCESS);
                CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
                break;
            }

            stayInBootFlag[0] = '1';
            stayInBootFlag[STAY_IN_BOOT_FLAG] = '\0';
            Get_Software_ID();
            Get_Firmware_ID();
            Build_Metadata();

            __builtin_memset(&eepromBuffer[EEPROM_METADATA_OFFSET], 0xFF,
                    EEPROM_METADATA_SIZE);
            __builtin_memcpy(&eepromBuffer[EEPROM_METADATA_OFFSET], metadata,
                    METADATA_LENGTH);

            if (Flash_EEPROM_Write_Page(eepromBuffer,
                    EEPROM_DATA_SIZE) == FLASH_EEPROM_OK) {
                asm("reset");
            } else {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_FAIL);
                CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            }
            break;

        case BOOT_CAN_CMD_1_APP_JUMP:
            /* Already running in Application */
            if ((addr >= APPLICATION_START_ADDRESS) &&
                    (addr <= APPLICATION_END_ADDRESS)) {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_SUCCESS);
                CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
                break;
            }

            /* Safety fallback: never accept a received image without CRC. */
            if (firmwareUpdateActive && !firmwareUpdateFailed &&
                    (flashBufferIndex > 0U)) {
                if (!App_Firmware_Write_Flash()) {
                    firmwareUpdateFailed = true;
                }
            }

            if (firmwareUpdateActive) {
                firmwareImageReady = firmwareCrcVerified && !firmwareUpdateFailed;
                firmwareUpdateActive = false;

                if (!firmwareImageReady) {
                    txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                            (BOOT_CAN_CMD_3_STATUS_FAIL);
                    CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
                    break;
                }
            }

            if (!firmwareUpdateFailed && (Boot_Is_App_Present() == true)) {
                Boot_App_Jump();
            } else {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_FAIL);
                CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            }
            break;

        default:
            return;
    }
}

void Boot_App_Erase_Handler(void) {
    uint8_t txBuf[8] = {0U};
    uint32_t address;

    flashWriteAddress = APPLICATION_START_ADDRESS;
    flashBufferIndex = 0;
    firmwareUpdateActive = false;
    firmwareUpdateFailed = false;
    firmwareImageReady = false;
    firmwareCrcVerified = false;
    expectedChunkIndex = 1U;
    lastAcceptedChunkIndex = 0U;

    __builtin_memset(flashBuffer, 0xFF, sizeof (flashBuffer));

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_APP_ERASE) {
        return;
    }

    txBuf[0] = BOOT_CAN_CMD_0_APP_ERASE;
    txBuf[1] = 0x00U;
    txBuf[2] = 0x00U;

    /* Erase complete application region */
    for (address = APPLICATION_START_ADDRESS;
            address <= APPLICATION_END_ADDRESS;
            address += BOOT_FLASH_PAGE_SIZE) {
        while (NVM_IsBusy()) {
        }

        if (!NVM_PageErase(address)) {
            txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS(BOOT_CAN_CMD_3_STATUS_FAIL);
            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            return;
        }

        while (NVM_IsBusy()) {
        }
    }

    /* Verify erased flash */
    for (address = APPLICATION_START_ADDRESS;
            address <= APPLICATION_END_ADDRESS;
            address += 4U) {
        if (*(volatile const uint32_t *) address != 0xFFFFFFFFUL) {
            txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS(BOOT_CAN_CMD_3_STATUS_FAIL);
            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            return;
        }
    }

    firmwareUpdateActive = true;

    /* Erase successful */
    txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS(BOOT_CAN_CMD_3_STATUS_SUCCESS);
    CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
}

bool App_Firmware_Write_Flash(void) {
    uint32_t rowAddr;
    uint32_t offset;
    uint32_t bytesToWrite = flashBufferIndex;

    /* NVM_FLASH_ROWSIZE is in 32-bit words; the NVM needs all 512 bytes. */
    static uint32_t rowData[NVM_FLASH_ROWSIZE] __attribute__((aligned(4)));

    if (bytesToWrite == 0U) {
        return true;
    }

    rowAddr = flashWriteAddress;

    for (offset = 0; offset < bytesToWrite; offset += FLASH_ROW_SIZE) {
        uint32_t rowBytes;

        if (rowAddr > (APPLICATION_END_ADDRESS - (FLASH_ROW_SIZE - 1U))) {
            return false;
        }

        __builtin_memset(rowData, 0xFF, sizeof (rowData));

        rowBytes = FLASH_ROW_SIZE < (bytesToWrite - offset)
                ? FLASH_ROW_SIZE
                : (bytesToWrite - offset);

        (void) memcpy(
                rowData,
                &flashBuffer[offset],
                rowBytes);

        if (NVM_RowWrite(rowData, rowAddr) == false) {
            return false;
        }

        while (NVM_IsBusy()) {
        }

        if (NVM_ErrorGet() != NVM_ERROR_NONE) {
            return false;
        }

        /* Do not acknowledge a corrupt row to the OTA device. */
        if (memcmp((const void *) rowAddr, rowData, FLASH_ROW_SIZE) != 0) {
            return false;
        }

        rowAddr += FLASH_ROW_SIZE;
    }

    flashWriteAddress += bytesToWrite;
    flashBufferIndex = 0;

    __builtin_memset(flashBuffer, 0xFF, sizeof (flashBuffer));

    return true;
}

uint8_t Next_Chunk_Index(uint8_t chunkIndex) {
    return (chunkIndex == 0xFFU) ? 1U : (uint8_t) (chunkIndex + 1U);
}

void Boot_App_Chunk_Ack(uint8_t chunkIndex, uint8_t operation, uint8_t status) {
    uint8_t txBuf[8] = {0U};

    txBuf[0] = BOOT_CAN_CMD_0_APP_CHUNK;
    txBuf[1] = chunkIndex;
    txBuf[2] = 0x00U;
    txBuf[3] = (uint8_t) (BOOT_CAN_CMD_3_SET_OPERATION(operation) |
            BOOT_CAN_CMD_3_SET_STATUS(status));

    CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
}

void Boot_App_Chunk_Handler(void) {
    uint8_t chunkIndex = canRxBuffer[1];
    uint8_t operation = BOOT_CAN_CMD_3_GET_OPERATION(canRxBuffer[3]);

    if ((canRxBuffer[0] != BOOT_CAN_CMD_0_APP_CHUNK) ||
            (operation != BOOT_CAN_CMD_3_OPERATION_WRITE) ||
            (chunkIndex == 0U) || !firmwareUpdateActive || firmwareUpdateFailed) {
        Boot_App_Chunk_Ack(chunkIndex, operation,
                BOOT_CAN_CMD_3_STATUS_FAIL);
        return;
    }

    /* An ACK can be lost. Re-ACK a duplicate without programming it twice. */
    if (chunkIndex == lastAcceptedChunkIndex) {
        Boot_App_Chunk_Ack(chunkIndex, operation,
                BOOT_CAN_CMD_3_STATUS_SUCCESS);
        return;
    }

    if ((chunkIndex != expectedChunkIndex) ||
            ((flashBufferIndex + CAN_FRAME_DATA_SIZE) > FLASH_CHUNK_SIZE)) {
        Boot_App_Chunk_Ack(chunkIndex, operation,
                BOOT_CAN_CMD_3_STATUS_FAIL);
        return;
    }

    memcpy(&flashBuffer[flashBufferIndex], &canRxBuffer[4], CAN_FRAME_DATA_SIZE);
    flashBufferIndex += CAN_FRAME_DATA_SIZE;

    /* One OTA chunk is 256 four-byte frames (1024 bytes). */
    if (flashBufferIndex < FLASH_CHUNK_SIZE) {
        return;
    }

    if (!App_Firmware_Write_Flash()) {
        firmwareUpdateFailed = true;
        flashBufferIndex = 0U;
        Boot_App_Chunk_Ack(chunkIndex, operation,
                BOOT_CAN_CMD_3_STATUS_FAIL);
        return;
    }

    lastAcceptedChunkIndex = chunkIndex;
    expectedChunkIndex = Next_Chunk_Index(chunkIndex);
    Boot_App_Chunk_Ack(chunkIndex, operation,
            BOOT_CAN_CMD_3_STATUS_SUCCESS);
}

uint32_t Boot_APP_CRC32_Software(const volatile uint8_t *data, uint32_t length) {
    uint32_t crc = 0xFFFFFFFFU;
    uint8_t bit;

    while (length-- > 0U) {
        crc ^= *data++;

        for (bit = 0U; bit < 8U; bit++) {
            crc = ((crc & 1U) != 0U) ?
                    ((crc >> 1U) ^ 0xEDB88320U) : (crc >> 1U);
        }
    }

    return crc ^ 0xFFFFFFFFU;
}

void Boot_APP_CRC_Handler(void) {
    uint8_t txBuf[8] = {0U};
    uint8_t operation;
    uint32_t flashCrc;
    uint32_t crcLength;

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_APP_CRC) {
        return;
    }

    operation = BOOT_CAN_CMD_3_GET_OPERATION(canRxBuffer[3]);

    txBuf[0] = BOOT_CAN_CMD_0_APP_CRC;
    txBuf[1] = 0x00U;
    txBuf[2] = 0x00U;

    switch (operation) {
        case BOOT_CAN_CMD_3_OPERATION_READ:

            txBuf[3] = BOOT_CAN_CMD_3_SET_OPERATION(operation);

            /*
             * During firmware update, use the current firmware size.
             * Otherwise, use the previously stored application size.
             */
            crcLength = (firmwareUpdateActive &&
                    (firmwareExpectedSize != 0U))
                    ? firmwareExpectedSize
                    : (uint32_t) strtoul(appSize, NULL, 10);

            if (firmwareUpdateActive && firmwareUpdateFailed) {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_FAIL);
                CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
                break;
            }

            /* Do not calculate CRC while a partial chunk is still buffered. */
            if (firmwareUpdateActive && (flashBufferIndex != 0U)) {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_FAIL);
                CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
                break;
            }

            if ((crcLength == 0U) ||
                    (crcLength > (APPLICATION_END_ADDRESS -
                    APPLICATION_START_ADDRESS + 1U))) {
                txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                        (BOOT_CAN_CMD_3_STATUS_FAIL);
                CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
                break;
            }

            flashCrc = Boot_APP_CRC32_Software(
                    (const volatile uint8_t *)APPLICATION_START_ADDRESS,
                    crcLength);

            firmwareCrcVerified = true;

            txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS
                    (BOOT_CAN_CMD_3_STATUS_SUCCESS);
            txBuf[4] = (uint8_t) (flashCrc >> 24);
            txBuf[5] = (uint8_t) (flashCrc >> 16);
            txBuf[6] = (uint8_t) (flashCrc >> 8);
            txBuf[7] = (uint8_t) flashCrc;

            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            break;

        default:
            break;
    }
}

void Boot_Stay_In_Boot_Handler(void) {
    uint8_t txBuf[8] = {0U};

    if (canRxBuffer[0] == BOOT_CAN_CMD_0_STAY_IN_BOOT) {
        bootStayRequested = true;
    }

    txBuf[0] = BOOT_CAN_CMD_0_STAY_IN_BOOT;
    txBuf[1] = 0x00U;
    txBuf[2] = 0x00U;
    txBuf[3] = BOOT_CAN_CMD_3_SET_STATUS(BOOT_CAN_CMD_3_STATUS_SUCCESS);
    CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
}

bool Boot_Stay_In_Boot_Requested(void) {
    return bootStayRequested;
}

void Run_Boot_Handler(uint32_t id, uint8_t *rxBuffer) {
    __builtin_memcpy(canRxBuffer, rxBuffer, sizeof (canRxBuffer));

    switch (id) {
        case BOOT_CAN_ID_HOST:
        {
            switch (rxBuffer[0]) {
                case BOOT_CAN_CMD_0_PIC_ID:
                {
                    Boot_PIC_ID_Handler();
                    break;
                }

                case BOOT_CAN_CMD_0_METADATA:
                {
                    Boot_Metadata_Handler();
                    break;
                }

                case BOOT_CAN_CMD_0_STATUS:
                {
                    Boot_Status_Handler();
                    break;
                }

                case BOOT_CAN_CMD_0_APP_SIZE:
                {
                    Boot_App_Size_Handler();
                    break;
                }

                case BOOT_CAN_CMD_0_JUMP:
                {
                    Boot_Jump_Handler();
                    break;
                }

                case BOOT_CAN_CMD_0_APP_ERASE:
                {
                    Boot_App_Erase_Handler();
                    break;
                }

                case BOOT_CAN_CMD_0_APP_CHUNK:
                {
                    Boot_App_Chunk_Handler();
                    break;
                }

                case BOOT_CAN_CMD_0_APP_CRC:
                {
                    Boot_APP_CRC_Handler();
                    break;
                }

                case BOOT_CAN_CMD_0_STAY_IN_BOOT:
                {
                    Boot_Stay_In_Boot_Handler();
                    break;
                }

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }
}
