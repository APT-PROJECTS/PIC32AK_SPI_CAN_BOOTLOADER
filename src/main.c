#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "driver.h"
#include "middleware.h"
#include "software.h"

#define MCP2515_STARTUP_ATTEMPTS  5U

int main(void) {
    static uint8_t attempt;

    SYS_Initialize(NULL);

    RED_LED_Set();

    for (attempt = 0U; attempt < MCP2515_STARTUP_ATTEMPTS; attempt++) {
        if (MCP2515_Init() == true) {
            //            GREEN_LED_Set();
            break;
        }
    }

    NVM_EEPROM_Initialize();

    while (true) {
        STATE_ENGINE_Tasks();
    }

    return ( EXIT_FAILURE);
}
