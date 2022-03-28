#include "environment_local.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

FILE *fp;
uint32_t time_startup = 0;

void put_pixel(uint16_t index, RgbwColor *c)
{
    fp = fopen("local_dev_output.txt", "r+b");
    fseek(fp, (index * 16), SEEK_SET);
    fprintf(fp, "%03d,%03d,%03d,%03d\n", c->r, c->g, c->b, c->w);
    fclose(fp);
}

void put_pin(int gpio, bool value)
{
    // Do what here? Write to another file?
}

bool get_pin(int gpio)
{
    // TODO: Implement

    return false;
}

void init_pin_button(int gpio)
{
    // Do we care?
}

void launch_thread(void (*entry)(void))
{
    // Does this work well enough?
    // thrd_t thr[1];
    // thrd_create(&thr[0], entry, NULL);
}

void program_init(int offset, int gpio)
{
    // We do not care
}

int get_pio_offset()
{
    return 0;
}

void sleep_ms(uint32_t ms)
{
    // Do we even care to? Maybe?
    sleep_us(ms * 1000);
}

void sleep_us(uint64_t ms)
{
    // Do we even care to? Maybe?
}

uint32_t get_running_ms()
{
    // TODO: Get the real time! sys/time does not seem to exist!
    time_startup += 10;
    return time_startup;
}

void picolight_blink(bool on)
{
    // ??
}

void picolight_boot(int led_count)
{
    printf("Booted up");
    fp = fopen("local_dev_output.txt", "w+b");
    fseek(fp, 0, SEEK_SET);

    for (int i = 0; i < led_count; i++)
    {
        fprintf(fp, "000,000,000,000\n");
    }

    fclose(fp);
}

void picolight_post_boot()
{
}