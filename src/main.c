#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "middleware/boot_handler.h"


int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    Run_Boot_Handler();

    while ( true )
    {
        RED_LED_Set();
    }

    return ( EXIT_FAILURE );
}
