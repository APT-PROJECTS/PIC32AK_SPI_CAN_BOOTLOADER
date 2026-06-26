#include "boot_handler.h"
#include "peripheral/gpio/plib_gpio.h"

typedef void (*APP_ENTRY_FUNC)(void);

bool Boot_IsApplicationPresent(void)
{
    uint32_t appEntryAddress = *(volatile const uint32_t *)APPLICATION_START_ADDRESS;

    if ((appEntryAddress == 0xFFFFFFFFUL) || (appEntryAddress == 0x00000000UL))
    {
        return false;
    }

    if ((appEntryAddress < APPLICATION_START_ADDRESS) ||
        (appEntryAddress > APPLICATION_END_ADDRESS))
    {
        return false;
    }

    return true;
}

void App_Jump(void)
{
    uint32_t appEntryAddress;
    APP_ENTRY_FUNC appEntry;

    if (Boot_IsApplicationPresent() == false)
    {
        return;
    }

    appEntryAddress = *(volatile const uint32_t *)APPLICATION_START_ADDRESS;
    appEntry = (APP_ENTRY_FUNC)appEntryAddress;

    (void)__builtin_disable_interrupts();

    appEntry();

    while (true)
    {
        /* Should never return from application startup code. */
    }
}

void Run_Boot_Handler(void)
{
    static int ctr = 0;
    static BOOT_STATE bootState = BOOT_STATE_INIT;

    switch (bootState)
    {
        case BOOT_STATE_INIT:
        {
            bootState = BOOT_STATE_FIRMWARE_UPGRADE;
            break;
        }

        case BOOT_STATE_FIRMWARE_UPGRADE:
        {
            RED_LED_Set();
            if(ctr < 2000000)
            {
                ctr++;
            }
            else
            {
                ctr = 0;
                RED_LED_Clear();
                bootState = BOOT_STATE_APP_JMP;
            }
            break;
        }

        case BOOT_STATE_APP_JMP:
        {
            App_Jump();
            break;
        }

        default:
        {
            bootState = BOOT_STATE_INIT;
            break;
        }
    }
}
