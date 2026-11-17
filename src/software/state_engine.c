#include "definitions.h"
#include "driver.h"
#include "middleware.h"
#include "software.h"

//--------------------------------------------------
// STATIC STATE VARIABLE
//--------------------------------------------------
static STATE_ENGINE_STATES currentState = STATE_INIT;


//--------------------------------------------------
// STATE MACHINE TASKS
//--------------------------------------------------
void STATE_ENGINE_Tasks(void) {
    
    switch (currentState) {
        case STATE_INIT:
        {
            MCP2515_Init();
            Flash_EEPROM_Init();
            Metadata_Init();
            currentState = STATE_RUN;
            break;
        }

        case STATE_RUN:
        {
            MCP2515_Task();
            CANbus_read();
            break;
        }

        default:
        {
            break;
        }
    }
}