#include "boot_handler.h"

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
    App_Jump();

    while (true)
    {
        /* Stay in bootloader if application flash is empty or invalid. */
    }
}
