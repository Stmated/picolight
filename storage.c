#include "storage.h"
#include "global.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#define SAVE_MAGIC 0xCAFEBABE

// #define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
//#define FLASH_SECTOR_SIZE 4096
// #define SAVE_SLOT_SIZE

#define SLOT0_OFFSET (PICO_FLASH_SIZE_BYTES - (2 * FLASH_SECTOR_SIZE))
#define SLOT1_OFFSET (PICO_FLASH_SIZE_BYTES - (1 * FLASH_SECTOR_SIZE))

#define SLOT0_ADDR (XIP_BASE + SLOT0_OFFSET)
#define SLOT1_ADDR (XIP_BASE + SLOT1_OFFSET)

// const save_state_t *flash_save = (const save_state_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

const save_state_t *slot0 = (const save_state_t *)(SLOT0_ADDR);
const save_state_t *slot1 = (const save_state_t *)(SLOT1_ADDR);

const save_state_t *get_latest_flash_save()
{
    save_state_t *flash_save;
    if (slot0->magic == SAVE_MAGIC && slot1->magic == SAVE_MAGIC)
    {
        return (slot0->version >= slot1->version) ? slot0 : slot1;
    }
    else if (slot0->magic == SAVE_MAGIC)
    {
        return slot0;
    }
    else if (slot1->magic == SAVE_MAGIC)
    {
        return slot1;
    }

    return NULL;
}

bool state_store()
{
    // Create the temporary buffer outside of the stack
    uint8_t *buffer = calloc(FLASH_SECTOR_SIZE, sizeof(uint8_t));

    const save_state_t *flash_save = get_latest_flash_save();
    uint16_t new_version = flash_save ? flash_save->version + 1 : 1;

    save_state_t persisted_state = {
        .magic = SAVE_MAGIC,
        .version = new_version,
        .led_count = state.ledCount,
        .pattern_index = state.patternIndex,
        .intensity = state.intensity,
        .speed = state.speed,
        .with_offset = state.withOffset,
    };

    // Copy to temporary buffer (must be 4 KB since we must erase full sector)
    //uint8_t buffer[FLASH_SECTOR_SIZE] = {0};
    void *res1 = memset(buffer, 0, FLASH_SECTOR_SIZE);
    void *res2 = memcpy(buffer, &persisted_state, sizeof(save_state_t));

    // Disable interrupts while doing flash operations
    uint32_t ints = save_and_disable_interrupts();

    uint32_t target_offset = (new_version % 2) == 0 ? SLOT1_OFFSET : SLOT0_OFFSET;

    flash_range_erase(target_offset, FLASH_SECTOR_SIZE);           // erase 4KB
    flash_range_program(target_offset, buffer, FLASH_SECTOR_SIZE); // program 4KB

    restore_interrupts(ints);

    // Free the temporary buffer
    free(buffer);
}

bool state_load()
{
    const save_state_t *flash_save = get_latest_flash_save();
    if (flash_save == NULL)
    {
        return false;
    }

    state.ledCount = flash_save->led_count;
    state.nextPatternIndex = flash_save->pattern_index % getPatternCount(); // Could in theory be out of bounds from code changes.
    state.intensity = flash_save->intensity;
    state.speed = flash_save->speed;
    state.withOffset = flash_save->with_offset;

    return true;
}