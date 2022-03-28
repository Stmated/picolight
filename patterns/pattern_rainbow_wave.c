#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_rainbow_wave_struct
{
    int easing;
    bool endless;
    int period;
    float hsi_s;
    float hsi_i;
} pattern_rainbow_wave_struct;

void *pattern_rainbow_wave_data(uint16_t len, float intensity)
{
    pattern_rainbow_wave_struct *instance = malloc(sizeof(pattern_rainbow_wave_struct));

    instance->easing = randint(getEasingCount());
    instance->endless = randint(1000) > 500;
    instance->period = randint_weighted_towards_min(1000, 30000, intensity);
    instance->hsi_s = 0.9 + (0.1 * (randint_weighted_towards_max(100, 1000, intensity) / (float)1000));
    instance->hsi_i = 0.1 + (0.4 * (randint_weighted_towards_max(0, 1000, intensity) / (float)1000));
    return instance;
}

void pattern_rainbow_wave(uint16_t len, uint32_t t, void *data, PatternPrinter printer)
{
    pattern_rainbow_wave_struct *instance = data;

    float p;
    if (instance->endless)
    {
        // p is total progress for an endless looping, never stopping.
        p = t / (float)instance->period;
    }
    else
    {
        p = executeEasing(instance->easing, (t % instance->period) / (float)instance->period);
    }

    float huePerLed = (360.0 / (float)len);

    HsiColor hsi = {0, instance->hsi_s, instance->hsi_i};
    for (int i = 0; i < len; i++)
    {
        float base = (huePerLed * i);
        float offset = (360 * p);
        hsi.h = (int)roundf(base + offset) % 360;
        printer(i, &hsi, data);
    }
}

void pattern_register_rainbow_wave()
{
    pattern_register(pattern_rainbow_wave, pattern_rainbow_wave_data, pattern_destroyer_default, &(PatternOptions){1});
}