#include "environment_local.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

FILE *fp;
uint32_t time_startup = 0;

void put_pixel(uint16_t index, RgbwColor *c)
{
    // We do nothing here for. The impact should be negligible, and don't know what to do here. Write ANSI text to file and follow file?

    fp = fopen("local_dev_output.ansi", "r+b");
    fseek(fp, (index * 16), SEEK_SET);
    fprintf(fp, "\e[48;5;%03d;%03d;%03dm \033[0;00m", c->r, c->g, c->b);
    fclose(fp);

    //system("clear");
    // \033[0;31m%03d,\033[0;32m%03d,\033[0;34m%03d,\033[0;37m%03d
    //printf("\033[48;5;%dm    \033[0;00m\n", rgbToAnsi256(c->r, c->g, c->b));
    //printf("\e[48;5;%d;%d;%dm***\033[0;00m\n", c->r, c->g, c->b);
    //printf("\x1b[38;%d;%d;%d;249mTRUECOLOR\x1b[0m\n", c->r, c->g, c->b);
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
    printf("Booted up\n");

    fp = fopen("local_dev_output.ansi", "w+b");
    fseek(fp, 0, SEEK_SET);

    for (int i = 0; i < led_count; i++)
    {
        fprintf(fp, "000,000,000,000\n");
    }
}

void picolight_post_boot()
{
}

int getPermanentInt(int i)
{
    return 0;
}

void setPermanentInt(int i, int v)
{
    // TODO: Do something?
}