#include <math.h>
#include "definitions.h"
#include "state_engine.h"
#include "../driver/driver_CANbus.h"
#include "../driver/driver_mcp2515.h"
#include "../driver/device_stat.h"

//--------------------------------------------------
// STATIC STATE VARIABLE
//--------------------------------------------------
static STATE_ENGINE_STATES currentState = STATE_INIT;


//--------------------------------------------------
// STATE MACHINE TASKS
//--------------------------------------------------

void STATE_ENGINE_Tasks(void) {
    
    switch (currentState) {
            //--------------------------------------------------
        case STATE_INIT:
            //--------------------------------------------------
        {
            Get_MAC_ID();
            currentState = STATE_RUN;
            break;
        }

            //--------------------------------------------------
        case STATE_RUN:
            //--------------------------------------------------
        {
            MCP2515_Task();
            CANbus_read();
            break;
        }

            //--------------------------------------------------
        default:
            //--------------------------------------------------
        {
            break;
        }
    }
}
