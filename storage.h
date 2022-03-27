#include "pico/stdlib.h"
#include <hardware/flash.h>
#include <hardware/sync.h>

#define FLASH_TARGET_PAGE_COUNT (1) // Number of pages
#define FLASH_TARGET_SIZE (FLASH_PAGE_SIZE * FLASH_TARGET_PAGE_COUNT) // 256k per page
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_TARGET_SIZE) // 2MB - Flash Size Page(s)

const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

struct storage_struct
{

};

int getPermanentInt(int i);

void setPermanentInt(int i, int v);