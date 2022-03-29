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
    pattern_rainbow_wave_struct *data = malloc(sizeof(pattern_rainbow_wave_struct));

    data->easing = randint(getEasingCount());
    data->endless = randint(1000) > 500;
    data->period = randint_weighted_towards_min(1000, 30000, intensity);
    data->hsi_s = 0.9 + (0.1 * (randint_weighted_towards_max(100, 1000, intensity) / (float)1000));
    data->hsi_i = 0.1 + (0.4 * (randint_weighted_towards_max(0, 1000, intensity) / (float)1000));
    return data;
}

void pattern_rainbow_wave(uint16_t offset, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    pattern_rainbow_wave_struct *data = dataPtr;

    float p;
    if (data->endless)
    {
        // p is total progress for an endless looping, never stopping.
        p = t / (float)data->period;
    }
    else
    {
        p = executeEasing(data->easing, (t % data->period) / (float)data->period);
    }

    float huePerLed = (360.0 / (float)len);

    HsiColor hsi = {0, data->hsi_s, data->hsi_i};
    for (int i = offset; i < len; i++)
    {
        float base = (huePerLed * i);
        float offset = (360 * p);
        hsi.h = (int)roundf(base + offset) % 360;
        printer(i, &hsi, dataPtr);
    }
}

void pattern_register_rainbow_wave()
{
    pattern_register("rainbow_wave", pattern_rainbow_wave,
                     pattern_rainbow_wave_data, pattern_destroyer_default,
                     pattern_cycle_creator_default, pattern_cycle_destroyer_default,
                     &(PatternOptions){1});
}