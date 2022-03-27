#include "storage.h"

int getPermanentInt(int i)
{
    // char *p = (char *)XIP_BASE;
    // char *first_byte = *p;

    // XIP_BASE


        return 0;
}

void setPermanentInt(int i, int v)
{
    uint32_t interrupts = save_and_disable_interrupts();
    // flash_range_erase (...);

    // flash_range_program(FLASH_TARGET_OFFSET, );

    restore_interrupts(interrupts);
}