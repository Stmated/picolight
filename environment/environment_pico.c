#include "environment_pico.h"

uint8_t premul_lut[256][256];

inline void put_pixel(const uint16_t index, const uint16_t len, const uint64_t t_us, const uint32_t dt_us, const RgbwaColor *restrict c)
{
    if (c->a < ALPHA_NEGLIGIBLE_MIN)
    {
        pio_sm_put_blocking(pio0, 0, 0);
    }
    else
    {
        uint32_t r;
        uint32_t g;
        uint32_t b;
        if (c->a < ALPHA_NEGLIGIBLE_MAX)
        {
            uint8_t a = c->a;

            r = premul_lut[c->r][a];
            g = premul_lut[c->g][a];
            b = premul_lut[c->b][a];

            // TODO: Should add time-based dithering here
            //          - But seems the only stable way is to add a pixel buffer (which goes against the idea of the project...)
        }
        else
        {
            r = c->r;
            g = c->g;
            b = c->b;
        }

        if (c->w != 0)
        {
            // Distribute white component evenly to R, G, B
            // Scale it down slightly to preserve color balance (e.g., 2/3 strength)
            // Fast approximation: (w * 170) >> 8  ≈ w * 2/3
            uint8_t w_contrib = (c->w * 170) >> 8;

            // Saturated add
            r = MIN(255, r + w_contrib);
            g = MIN(255, g + w_contrib);
            b = MIN(255, b + w_contrib);
        }

        uint32_t packed = (g << 24) | (r << 16) | (b << 8);
        pio_sm_put_blocking(pio0, 0, packed);
    }

    /*
    if (isRgbw(index))
    {
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
    */
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
    // TODO: Check how to store this on the flash and read that memory on demand? Is that slow or fast?
    for (int val = 0; val < 256; val++)
    {
        for (int alpha = 0; alpha < 256; alpha++)
        {
            premul_lut[val][alpha] = (val * alpha) >> 8;
        }
    }

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