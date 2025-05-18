#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "../options.h"
#include "../led_math.h"

void put_pixel(uint16_t index, uint16_t len, RgbwColor *c);

void init_pin_button(int gpio);

void put_pin(int gpio, bool value);

bool get_pin(int gpio);

int get_pio_offset();

void sleep_ms(uint32_t ms);

void sleep_us(uint64_t us);

void picolight_boot(int led_count);

void picolight_post_boot();

uint32_t get_running_ms();

uint64_t get_running_us();

void launch_thread(void (*entry)(void));

void picolight_blink(bool on);

void program_init(int offset, int gpio);

int getPermanentInt(int i);
void setPermanentInt(int i, int v);

void register_action_listeners();

// TODO: Move below to only be included when compiling for local dev?
#ifndef count_of
#define count_of(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((b) > (a) ? (a) : (b))
#endif