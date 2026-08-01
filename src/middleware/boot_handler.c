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

typedef void (*APP_ENTRY_FUNC)(void);

void Send_PICID_1(void)
{
    uint8_t txBuf[8];

    /* UDID0 */
    txBuf[0] = (uint8_t)(udid[0] >> 24);
    txBuf[1] = (uint8_t)(udid[0] >> 16);
    txBuf[2] = (uint8_t)(udid[0] >> 8);
    txBuf[3] = (uint8_t)(udid[0]);

    /* UDID1 */
    txBuf[4] = (uint8_t)(udid[1] >> 24);
    txBuf[5] = (uint8_t)(udid[1] >> 16);
    txBuf[6] = (uint8_t)(udid[1] >> 8);
    txBuf[7] = (uint8_t)(udid[1]);

    CANbus_write(CANID_RESP_PICID_1, txBuf, 8);
}

void Send_PICID_2(void)
{
    uint8_t txBuf[8];

    /* UDID2 */
    txBuf[0] = (uint8_t)(udid[2] >> 24);
    txBuf[1] = (uint8_t)(udid[2] >> 16);
    txBuf[2] = (uint8_t)(udid[2] >> 8);
    txBuf[3] = (uint8_t)(udid[2]);

    /* UDID3 */
    txBuf[4] = (uint8_t)(udid[3] >> 24);
    txBuf[5] = (uint8_t)(udid[3] >> 16);
    txBuf[6] = (uint8_t)(udid[3] >> 8);
    txBuf[7] = (uint8_t)(udid[3]);

    CANbus_write(CANID_RESP_PICID_2, txBuf, 8);
}

void Send_Software_ID(void)
{
    uint8_t TxBuf[8];

    uint32_t project_id;
    uint32_t firmware_id;

    /* Read IDs from parameter matrix */
    project_id = (uint32_t)R_PROJECT_ID;
    firmware_id = (uint32_t)R_FIRMWARE_ID;

    /* Extract Project ID */
    uint16_t oem_id = (uint16_t)((project_id >> 16) & 0xFFFFU);
    uint16_t application_id = (uint16_t)(project_id & 0xFFFFU);

    /* Extract Firmware ID */
    uint16_t major = (uint16_t)((firmware_id >> 16) & 0xFFFFU);
    uint16_t minor = (uint16_t)(firmware_id & 0xFFFFU);

    /* OEM ID */
    TxBuf[0] = (uint8_t)(oem_id >> 8);
    TxBuf[1] = (uint8_t)(oem_id);

    /* Application ID */
    TxBuf[2] = (uint8_t)(application_id >> 8);
    TxBuf[3] = (uint8_t)(application_id);

    /* Firmware Major */
    TxBuf[4] = (uint8_t)(major >> 8);
    TxBuf[5] = (uint8_t)(major);

    /* Firmware Minor */
    TxBuf[6] = (uint8_t)(minor >> 8);
    TxBuf[7] = (uint8_t)(minor);

    CANbus_write(CANID_RESP_SOFTWARE_ID, TxBuf, sizeof(TxBuf));
}

void Boot_Status(void)
{
    uint8_t resp = (uint8_t)CMD_RESP_BOOTLOADER_MODE;
    CANbus_write(CANID_MCU_RESP, &resp, sizeof(resp));
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
    switch (ID)
    {
        case CANID_REQ_PICID_1:
            Send_PICID_1();
            break;

        case CANID_REQ_PICID_2:
            Send_PICID_2();
            break;

        case CANID_REQ_SOFTWARE_ID:
            Send_Software_ID();
            break;

        case CANID_SCU_REQ:

            switch (rxBuffer[0])
            {
                case CMD_REQ_STATUS:
                    Boot_Status();
                    break;

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