#include "definitions.h"
#include "driver.h"
#include "middleware.h"

void CANbus_write(uint32_t Sadr, uint8_t *Sdata, uint8_t Sdata_L) {
    bool sstat;
    bool idType;

    /* Standard CAN ID (11-bit) */
    if (Sadr <= 0x7FFU) {
        idType = false;
    }/* Extended CAN ID (29-bit) */
    else {
        idType = true;
    }

    sstat = MCP2515_SendMessage(Sadr, idType, Sdata_L, Sdata);

    (void) sstat; // Suppress unused variable warning
}

void CANbus_read(void) {
    uint8_t Rdata[8];
    bool rstat = 0;
    uint8_t msg_len = 0;
    uint32_t msg_id = 0;
    bool idType;

    rstat = MCP2515_ReceiveMessage(&msg_id, &idType, &msg_len, &Rdata[0]);

    if (rstat) {
        rstat = 0;

        switch (msg_id) {

            case BOOT_CAN_ID_HOST:
            {
                Run_Boot_Handler(msg_id, Rdata);
                break;
            }

            default:
                break;
        }
    }
    Nop();
}