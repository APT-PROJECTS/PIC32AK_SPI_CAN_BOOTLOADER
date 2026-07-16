#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "software/state_engine.h"
#include "driver/device_stat.h"
#include "driver/driver_mcp2515.h"

#define MCP2515_STARTUP_ATTEMPTS  5U

int main ( void )
{
    static uint8_t attempt;
    
    SYS_Initialize ( NULL );
    
    EEPROM_Initialize();
    
    for(attempt = 0U; attempt < MCP2515_STARTUP_ATTEMPTS; attempt++)
    {
        if(MCP2515_Init() == true)
        {
//            GREEN_LED_Set();
            break;
        }
    }

    while ( true )
    {
        RED_LED_Set();
        STATE_ENGINE_Tasks();
    }

    return ( EXIT_FAILURE );
}
