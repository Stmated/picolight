#include "environment_pico.h"

inline void put_pixel(uint16_t index, uint16_t len, const RgbwColor *c)
{
    // TODO: Add dithering

    if (isRgbw(index))
    {
        // A hack to make this as fast as possible. Will only work on certain platforms!
        // Will convert the RGBWColor with its 4 8bit fields into one 32bit integer.
        // (More specifically the fields are saved in GRBW order, as the LEDs require)
        //pio_sm_put_blocking(pio0, 0, *(uint32_t *)c);

        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->g) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->r) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->b) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->w) << 24u);
    }
    else
    {
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->g) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->r) << 24u);
        pio_sm_put_blocking(pio0, 0, ((uint32_t)c->b) << 24u);
    }
}

void put_pin(int gpio, bool value)
{
    gpio_put(gpio, value);
}

bool get_pin(int gpio)
{
    return gpio_get(gpio);
}

void program_init(int offset, int gpio)
{
    ws2812_program_init(pio0, 0, offset, gpio, 800000, false);
}

void init_pin_button(int gpio)
{
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_set_pulls(gpio, true, false);
}

int get_pio_offset()
{
    return pio_add_program(pio0, &ws2812_program);
}

void launch_thread(void (*entry)(void))
{
    multicore_launch_core1(entry);
}

void picolight_blink(bool on)
{
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

uint32_t get_running_ms()
{
    return to_ms_since_boot(get_absolute_time());
}

uint64_t get_running_us()
{
    return to_us_since_boot(get_absolute_time());
}

void picolight_boot(int led_count)
{
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);
}

void picolight_post_boot()
{
    stdio_init_all();
}

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

void register_action_listeners()
{
    // Add GPIO triggers for all action pins
}