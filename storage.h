#include <stdint.h>
#include "pico/stdlib.h"

typedef struct {
    uint32_t magic;
    uint16_t version;

    int led_count;
    int pattern_index;
    float intensity;
    float speed;
    bool unused_with_offset;

} __attribute__((packed, aligned(4))) save_state_t;

bool state_store();
bool state_load();
