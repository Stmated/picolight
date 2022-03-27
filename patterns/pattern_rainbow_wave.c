#include "../patterns.h"
#include "../led_math.h"

void macro_rainbow(uint32_t from, uint32_t to, float s, float i, uint32_t t, uint32_t t_period, int easingIndex, void *data, printer printer)
{
    // TODO: Figure out way to make it not "overflow" with distracting colours
    //          Make it so that it ends at 300, but shows the other 60 on the other end
    //          So that the "huePerLed" is less, but it moves faster over the spectrum, so "0" reach the top at the end of the animation

    uint32_t len = (to - from);

    float p = executeEasing(easingIndex, (t % t_period) / (float)t_period);
    float offsetHue = (360.0) * p;

    float hue_start = (360.0 * 100.0);
    float hue = hue_start;
    float huePerLed = (360.0 / (float)len);

    //RgbwColor rgbw;
    HsiColor hsi = {0, s, i};
    for (int i = from; i < to; i++)
    {
        float totalHue = (hue - offsetHue);

        int ledH = ((int)totalHue) % 360;

        // TODO: Add the intensity increment!

        //hsi2rgbw(ledH, s, i, &rgbw);
        hsi.h = ledH;
        printer(i, &hsi, data);

        hue -= huePerLed;
    }
}

typedef struct pattern_rainbow_wave_struct
{
    int easing;
    int period;
    float hsi_s;
    float hsi_i;
} pattern_rainbow_wave_struct;

void *pattern_rainbow_wave_data(uint16_t len, float intensity)
{
    pattern_rainbow_wave_struct *instance = malloc(sizeof(pattern_rainbow_wave_struct));

    instance->easing = randint(getEasingCount());
    instance->period = MAX(1000, 15000 - (randint(10000) * intensity) - (randint(10000) * intensity));
    instance->hsi_s = 0.6 + (0.4 * ((randint(10000) / (float)10000) * intensity));
    instance->hsi_i = 0.2 + (0.5 * ((randint(10000) / (float)10000) * intensity));
    return instance;
}

void pattern_rainbow_wave(uint16_t len, uint32_t t, void *data, printer printer)
{
    pattern_rainbow_wave_struct *instance = data;
    macro_rainbow(0, len, instance->hsi_s, instance->hsi_i, t, instance->period, instance->easing, data, printer);
}

void pattern_register_rainbow_wave()
{
    pattern_register(pattern_rainbow_wave, pattern_rainbow_wave_data, pattern_destroyer_default);
}