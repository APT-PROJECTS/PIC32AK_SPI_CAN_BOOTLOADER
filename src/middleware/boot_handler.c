#include "definitions.h"
#include "boot_handler.h"
#include "../driver/driver_CANbus.h"
#include "../driver/device_stat.h"

typedef void (*APP_ENTRY_FUNC)(void);

void Send_PICID_1(void) {
    uint8_t txBuf[8];

    /* UDID0 */
    txBuf[0] = (uint8_t) (udid[0] >> 24);
    txBuf[1] = (uint8_t) (udid[0] >> 16);
    txBuf[2] = (uint8_t) (udid[0] >> 8);
    txBuf[3] = (uint8_t) (udid[0]);

    /* UDID1 */
    txBuf[4] = (uint8_t) (udid[1] >> 24);
    txBuf[5] = (uint8_t) (udid[1] >> 16);
    txBuf[6] = (uint8_t) (udid[1] >> 8);
    txBuf[7] = (uint8_t) (udid[1]);

    CANbus_write(CANID_RESP_PICID_1, txBuf, 8);
}

void Send_PICID_2(void) {
    uint8_t txBuf[8];

    /* UDID2 */
    txBuf[0] = (uint8_t) (udid[2] >> 24);
    txBuf[1] = (uint8_t) (udid[2] >> 16);
    txBuf[2] = (uint8_t) (udid[2] >> 8);
    txBuf[3] = (uint8_t) (udid[2]);

    /* UDID3 */
    txBuf[4] = (uint8_t) (udid[3] >> 24);
    txBuf[5] = (uint8_t) (udid[3] >> 16);
    txBuf[6] = (uint8_t) (udid[3] >> 8);
    txBuf[7] = (uint8_t) (udid[3]);

    CANbus_write(CANID_RESP_PICID_2, txBuf, 8);
}

void Send_Software_ID(void) {
    uint8_t TxBuf[8];
    uint32_t project_id;
    uint32_t firmware_id;

    /* Read IDs from parameter matrix */
    project_id = (uint32_t) R_PROJECT_ID;
    firmware_id = (uint32_t) R_FIRMWARE_ID;

    /* Extract Project ID */
    uint16_t oem_id = (uint16_t) ((project_id >> 16) & 0xFFFFU);
    uint16_t application_id = (uint16_t) (project_id & 0xFFFFU);

    /* Extract Firmware ID */
    uint16_t major = (uint16_t) ((firmware_id >> 16) & 0xFFFFU);
    uint16_t minor = (uint16_t) (firmware_id & 0xFFFFU);

    /* OEM ID */
    TxBuf[0] = (uint8_t) (oem_id >> 8);
    TxBuf[1] = (uint8_t) (oem_id);

    /* Application ID */
    TxBuf[2] = (uint8_t) (application_id >> 8);
    TxBuf[3] = (uint8_t) (application_id);

    /* Firmware Major */
    TxBuf[4] = (uint8_t) (major >> 8);
    TxBuf[5] = (uint8_t) (major);

    /* Firmware Minor */
    TxBuf[6] = (uint8_t) (minor >> 8);
    TxBuf[7] = (uint8_t) (minor);

    CANbus_write(CANID_RESP_SOFTWARE_ID, TxBuf, sizeof (TxBuf));
}

void Boot_Status(void) {
    uint8_t resp = (uint8_t) CMD_RESP_BOOTLOADER_MODE;

    CANbus_write(CANID_MCU_RESP, &resp, sizeof (resp));
}

bool App_Erase(void) {
    uint32_t address;

    for (address = APPLICATION_START_ADDRESS;
            address <= APPLICATION_END_ADDRESS;
            address += FLASH_PAGE_SIZE) {
        while (NVM_IsBusy()) {
        }

        if (!NVM_PageErase(address)) {
            return false;
        }

        while (NVM_IsBusy()) {
        }
    }

    return true;
}

bool App_Erase_Verify(void) {
    uint32_t address;

    for (address = APPLICATION_START_ADDRESS;
            address <= APPLICATION_END_ADDRESS;
            address += 4U) {
        if (*(volatile const uint32_t *) address != 0xFFFFFFFFUL) {
            return false;
        }
    }

    return true;
}

void App_Erase_Success(void) {
    uint8_t resp = (uint8_t) CMD_RESP_FLASH_READY;

    CANbus_write(CANID_MCU_RESP, &resp, sizeof (resp));
}

bool Is_App_Present(void)
{
    uint32_t appEntry;

    appEntry = *(volatile const uint32_t *)APPLICATION_START_ADDRESS;

    if ((appEntry == 0xFFFFFFFFUL) ||
        (appEntry == 0x00000000UL))
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

    appEntryAddress =
        *(volatile const uint32_t *)APPLICATION_START_ADDRESS;

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

    /* Jump to application's reset entry */
    appEntry();

    while (1)
    {
        /* Should never return */
    }
}

void Run_Boot_Handler(uint32_t ID, uint8_t CMD) {
    switch (ID) {
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
            switch (CMD) {
                case CMD_REQ_START:
                    if (App_Erase()) {
                        if (App_Erase_Verify()) {
                            App_Erase_Success();
                        }
                    }

                case CMD_REQ_APP_JUMP:
                    App_Jump();
                    break;

                case CMD_REQ_STATUS:
                    Boot_Status();
                    break;

                default:
                    break;
            }
        default:
            break;
    }
}