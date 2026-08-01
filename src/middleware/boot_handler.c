#include <string.h>
#include "definitions.h"
#include "boot_handler.h"
#include "../driver/driver_CANbus.h"
#include "../driver/device_stat.h"

#define FLASH_CHUNK_SIZE    4096U
#define FLASH_ROW_SIZE      (NVM_FLASH_ROWSIZE * sizeof(uint32_t))
#define BOOT_FLASH_PAGE_SIZE (NVM_FLASH_PAGESIZE * sizeof(uint32_t))
#define CAN_DATA_SIZE       7U

static uint8_t flashBuffer[FLASH_CHUNK_SIZE];
static uint32_t flashBufferIndex = 0;
static uint32_t flashWriteAddress = APPLICATION_START_ADDRESS;
static bool firmwareUpdateActive = false;
static bool firmwareUpdateFailed = false;
static bool firmwareImageReady = false;

static uint8_t CHUNK_ACK_DATA[8] =
{
    0x88, 0x77, 0x66, 0xFF,
    0x11, 0x22, 0x44, 0x33
};

static uint8_t canRxBuffer[8] = {0U};

typedef void (*APP_ENTRY_FUNC)(void);

void Boot_PIC_ID_Handler(void)
{
    uint8_t txBuf[8];
    uint32_t udidValue;

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_PIC_ID)
    {
        return;
    }

    switch (canRxBuffer[2])
    {
        /*------------------------- READ --------------------------*/
        case BOOT_CAN_CMD_2_READ:

            txBuf[0] = BOOT_CAN_CMD_0_PIC_ID;
            txBuf[1] = canRxBuffer[1];
            txBuf[2] = BOOT_CAN_CMD_2_READ;
            txBuf[3] = 0x00;

            switch (canRxBuffer[1])
            {
                case BOOT_CAN_CMD_1_PIC_ID_1:
                    udidValue = udid[0];
                    break;

                case BOOT_CAN_CMD_1_PIC_ID_2:
                    udidValue = udid[1];
                    break;

                case BOOT_CAN_CMD_1_PIC_ID_3:
                    udidValue = udid[2];
                    break;

                case BOOT_CAN_CMD_1_PIC_ID_4:
                    udidValue = udid[3];
                    break;

                default:
                    return;
            }

            txBuf[4] = (uint8_t)(udidValue >> 24);
            txBuf[5] = (uint8_t)(udidValue >> 16);
            txBuf[6] = (uint8_t)(udidValue >> 8);
            txBuf[7] = (uint8_t)(udidValue);

            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof(txBuf));
            break;

        default:
            break;
    }
}

void Boot_Software_ID_Handler(void) {
    uint8_t txBuf[8];
    float softwareId;

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_SOFTWARE_ID) {
        return;
    }

    switch (canRxBuffer[2]) {
            /*------------------------- READ --------------------------*/
        case BOOT_CAN_CMD_2_READ:

            txBuf[0] = BOOT_CAN_CMD_0_SOFTWARE_ID;
            txBuf[1] = canRxBuffer[1];
            txBuf[2] = BOOT_CAN_CMD_2_READ;
            txBuf[3] = 0x00;

            switch (canRxBuffer[1]) {
                case BOOT_CAN_CMD_1_PROJECT_ID:
                    softwareId = R_PROJECT_ID;
                    break;

                case BOOT_CAN_CMD_1_FIRMWARE_ID:
                    softwareId = R_FIRMWARE_ID;
                    break;

                default:
                    return;
            }

            __builtin_memcpy(&txBuf[4], &softwareId, sizeof (float));

            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            break;

            /*------------------------- WRITE -------------------------*/
        case BOOT_CAN_CMD_2_WRITE:

            txBuf[0] = BOOT_CAN_CMD_0_SOFTWARE_ID;
            txBuf[1] = canRxBuffer[1];
            txBuf[2] = BOOT_CAN_CMD_2_WRITE;
            txBuf[3] = 0x00;

            switch (canRxBuffer[1]) {
                case BOOT_CAN_CMD_1_PROJECT_ID:
                    __builtin_memcpy(&W_PROJECT_ID, &canRxBuffer[4], sizeof (float));
                    break;

                case BOOT_CAN_CMD_1_FIRMWARE_ID:
                    __builtin_memcpy(&W_FIRMWARE_ID, &canRxBuffer[4], sizeof (float));
                    break;

                default:
                    return;
            }

            __builtin_memset(eepromBuffer, 0xFF, EEPROM_DATA_SIZE);
            __builtin_memcpy(eepromBuffer, writeMatrix, sizeof (writeMatrix));

            if (FLASH_EEPROM_Write_Page(eepromBuffer, sizeof (writeMatrix)) == FLASH_EEPROM_OK) {
                txBuf[3] = BOOT_CAN_CMD_3_SUCCESS;
            } else {
                txBuf[3] = BOOT_CAN_CMD_3_FAIL;
            }

            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof (txBuf));
            break;

        default:
            break;
    }
}

void Boot_Status_Handler(void)
{
    uint8_t txBuf[8] = {0};
    uint32_t addr;

    if (canRxBuffer[0] != BOOT_CAN_CMD_0_STATUS)
    {
        return;
    }

    switch (canRxBuffer[2])
    {
        /*------------------------- READ --------------------------*/
        case BOOT_CAN_CMD_2_READ:

            /* Get the address of this function */
            addr = (uint32_t)&Boot_Status_Handler;

            txBuf[0] = BOOT_CAN_CMD_0_STATUS;
            txBuf[2] = BOOT_CAN_CMD_2_READ;

            if ((addr >= BOOTLOADER_START_ADDRESS) &&
                (addr <= BOOTLOADER_END_ADDRESS))
            {
                txBuf[1] = BOOT_CAN_CMD_1_STATUS_BOOT;
            }
            else if ((addr >= APPLICATION_START_ADDRESS) &&
                     (addr <= APPLICATION_END_ADDRESS))
            {
                txBuf[1] = BOOT_CAN_CMD_1_STATUS_APP;
            }
            else
            {
                txBuf[1] = BOOT_CAN_CMD_1_STATUS_UNKNOWN;
            }

            CANbus_write(BOOT_CAN_ID_CLIENT, txBuf, sizeof(txBuf));
            break;

        default:
            break;
    }
}

void Boot_Jump_Success(void)
{
    uint8_t resp = (uint8_t)CMD_RESP_JUMP_OK;
    CANbus_write(CANID_MCU_RESP, &resp, sizeof(resp));
}

bool App_Erase(void)
{
    uint32_t address;

    for (address = APPLICATION_START_ADDRESS;
         address <= APPLICATION_END_ADDRESS;
         address += BOOT_FLASH_PAGE_SIZE)
    {
        while (NVM_IsBusy())
        {
        }

        if (!NVM_PageErase(address))
        {
            return false;
        }

        while (NVM_IsBusy())
        {
        }
    }

    return true;
}

bool App_Erase_Verify(void)
{
    uint32_t address;

    for (address = APPLICATION_START_ADDRESS;
         address <= APPLICATION_END_ADDRESS;
         address += 4U)
    {
        if (*(volatile const uint32_t *)address != 0xFFFFFFFFUL)
        {
            return false;
        }
    }

    return true;
}

void App_Erase_Success(void)
{
    uint8_t resp = (uint8_t)CMD_RESP_FLASH_READY;
    static int ctr = 0;

    for (ctr = 0; ctr <= 2000000; ctr++)
    {
    }

    CANbus_write(CANID_MCU_RESP, &resp, sizeof(resp));
}

static void App_Update_End_Success(void)
{
    uint8_t response[8] = {0U};

    CANbus_write(CANID_RESP_END, response, sizeof(response));
}

static bool App_Firmware_Write_Flash(void)
{
    uint32_t rowAddr;
    uint32_t offset;
    uint32_t bytesToWrite = flashBufferIndex;

    /* NVM_FLASH_ROWSIZE is in 32-bit words; the NVM needs all 512 bytes. */
    static uint32_t rowData[NVM_FLASH_ROWSIZE] __attribute__((aligned(4)));

    if (bytesToWrite == 0U)
    {
        return true;
    }

    rowAddr = flashWriteAddress;

    for (offset = 0; offset < bytesToWrite; offset += FLASH_ROW_SIZE)
    {
        uint32_t rowBytes;

        if (rowAddr > (APPLICATION_END_ADDRESS - (FLASH_ROW_SIZE - 1U)))
        {
            return false;
        }

        __builtin_memset(rowData, 0xFF, sizeof(rowData));

        rowBytes = FLASH_ROW_SIZE < (bytesToWrite - offset)
                       ? FLASH_ROW_SIZE
                       : (bytesToWrite - offset);

        (void)memcpy(
            rowData,
            &flashBuffer[offset],
            rowBytes);

        if (NVM_RowWrite(rowData, rowAddr) == false)
        {
            return false;
        }

        while (NVM_IsBusy())
        {
        }

        if (NVM_ErrorGet() != NVM_ERROR_NONE)
        {
            return false;
        }

        /* Do not acknowledge a corrupt row to the OTA device. */
        if (memcmp((const void *)rowAddr, rowData, FLASH_ROW_SIZE) != 0)
        {
            return false;
        }

        rowAddr += FLASH_ROW_SIZE;
    }

    flashWriteAddress += bytesToWrite;
    flashBufferIndex = 0;

    __builtin_memset(flashBuffer, 0xFF, sizeof(flashBuffer));

    return true;
}

void App_Firmware_Write_Buffer(uint8_t *rxBuffer)
{
    uint32_t copyCount;
    uint32_t remainingBytes;
    uint32_t payloadOffset;

    /* First byte must be 0x52 */
    if ((!firmwareUpdateActive) || firmwareUpdateFailed ||
        (rxBuffer[0] != CMD_REQ_DATA))
    {
        return;
    }

    if (flashBufferIndex + CAN_DATA_SIZE > FLASH_CHUNK_SIZE)
    {
        remainingBytes = FLASH_CHUNK_SIZE - flashBufferIndex;

        if (remainingBytes > 0U)
        {
            copyCount = (remainingBytes < CAN_DATA_SIZE)
                            ? remainingBytes
                            : CAN_DATA_SIZE;

            memcpy(&flashBuffer[flashBufferIndex], &rxBuffer[1], copyCount);
            flashBufferIndex += copyCount;
        }

        if (flashBufferIndex >= FLASH_CHUNK_SIZE)
        {
            if (!App_Firmware_Write_Flash())
            {
                firmwareUpdateFailed = true;
                flashBufferIndex = 0U;
                return;
            }

            CANbus_write(
                CANID_MCU_RESP,
                CHUNK_ACK_DATA,
                sizeof(CHUNK_ACK_DATA));

            flashBufferIndex = 0U;
        }

        if (flashBufferIndex < FLASH_CHUNK_SIZE)
        {
            payloadOffset = copyCount;
            copyCount = CAN_DATA_SIZE - payloadOffset;

            if (copyCount > 0U)
            {
                memcpy(
                    &flashBuffer[flashBufferIndex],
                    &rxBuffer[1 + payloadOffset],
                    copyCount);

                flashBufferIndex += copyCount;
            }
        }
    }
    else
    {
        memcpy(
            &flashBuffer[flashBufferIndex],
            &rxBuffer[1],
            CAN_DATA_SIZE);

        flashBufferIndex += CAN_DATA_SIZE;

        if (flashBufferIndex >= FLASH_CHUNK_SIZE)
        {
            if (!App_Firmware_Write_Flash())
            {
                firmwareUpdateFailed = true;
                flashBufferIndex = 0U;
                return;
            }

            CANbus_write(
                CANID_MCU_RESP,
                CHUNK_ACK_DATA,
                sizeof(CHUNK_ACK_DATA));

            flashBufferIndex = 0U;
        }
    }
}

bool Is_App_Present(void)
{
    uint32_t appEntry;

    appEntry = *(volatile const uint32_t *)APPLICATION_START_ADDRESS;

    if ((appEntry == 0xFFFFFFFFUL) || (appEntry == 0x00000000UL))
    {
        return false;
    }

    if ((appEntry < APPLICATION_START_ADDRESS) ||
        (appEntry > APPLICATION_END_ADDRESS))
    {
        return false;
    }

    return true;
}

void App_Jump(void)
{
    APP_ENTRY_FUNC appEntry;
    uint32_t appEntryAddress;

    if (Is_App_Present() == false)
    {
        return;
    }

    appEntryAddress = *(volatile const uint32_t *)APPLICATION_START_ADDRESS;
    appEntry = (APP_ENTRY_FUNC)appEntryAddress;

    /* Disable Global Interrupts */
    (void)__builtin_disable_interrupts();

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

    __builtin_setIVTBASE((void *)0x808040);

    /* Jump to application's reset entry */
    appEntry();

    while (1)
    {
        /* Should never return */
    }
}

void Run_Boot_Handler(uint32_t ID, uint8_t *rxBuffer)
{
    __builtin_memcpy(canRxBuffer, rxBuffer, sizeof (canRxBuffer));

    switch (ID)
    {
        case BOOT_CAN_ID_HOST:
        {
            switch (rxBuffer[0]) {
                case BOOT_CAN_CMD_0_PIC_ID:
                {
                    Boot_PIC_ID_Handler();
                    break;
                }

                case BOOT_CAN_CMD_0_SOFTWARE_ID:
                {
                    Boot_Software_ID_Handler();
                    break;
                }
                    
                case BOOT_CAN_CMD_0_STATUS:
                {
                    Boot_Status_Handler();
                    break;
                }
                    
                default:
                    break;
            }
            break;
        }

        case CANID_SCU_REQ:

            switch (rxBuffer[0])
            {
                case CMD_REQ_BOOTLOADER:
                    Boot_Jump_Success();
                    break;

                case CMD_REQ_START:
                    flashWriteAddress = APPLICATION_START_ADDRESS;
                    flashBufferIndex = 0;
                    firmwareUpdateActive = false;
                    firmwareUpdateFailed = false;
                    firmwareImageReady = false;

                    __builtin_memset(
                        flashBuffer,
                        0xFF,
                        sizeof(flashBuffer));

                    if (App_Erase())
                    {
                        if (App_Erase_Verify())
                        {
                            firmwareUpdateActive = true;
                            App_Erase_Success();
                        }
                    }
                    break;

                case CMD_REQ_DATA:
                    App_Firmware_Write_Buffer(rxBuffer);
                    break;

                case CMD_REQ_APP_JUMP:
                    if (firmwareImageReady && !firmwareUpdateFailed &&
                        (flashBufferIndex > 0U))
                    {
                        if (!App_Firmware_Write_Flash())
                        {
                            firmwareUpdateFailed = true;
                            break;
                        }
                    }

                    if (firmwareImageReady && !firmwareUpdateFailed)
                    {
                        firmwareUpdateActive = false;
                        App_Jump();
                    }
                    break;

                default:
                    break;
            }
            break;

        case CANID_REQ_END:
            if (firmwareUpdateActive && !firmwareUpdateFailed &&
                (flashBufferIndex > 0U))
            {
                if (!App_Firmware_Write_Flash())
                {
                    firmwareUpdateFailed = true;
                }
            }

            firmwareImageReady = firmwareUpdateActive && !firmwareUpdateFailed;
            firmwareUpdateActive = false;

            if (firmwareImageReady)
            {
                App_Update_End_Success();
            }
            break;

        default:
            break;
    }
}