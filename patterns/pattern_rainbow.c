#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_rainbow_struct
{
    int easing;
    bool endless;
    int period;
    int hue_from;
    int hue_width;
    float hsi_s;
    float hsi_i;
} pattern_rainbow_struct;

void *pattern_rainbow_data(uint16_t len, float intensity)
{
    pattern_rainbow_struct *instance = malloc(sizeof(pattern_rainbow_struct));

    instance->easing = randint(getEasingCount());
    instance->endless = randint(1000) > 500;
    instance->period = randint_weighted_towards_min(5000, 60000, intensity);
    instance->hue_from = randint(360);
    instance->hue_width = 360 + (360 * randint_weighted_towards_min(10, 1, intensity));
    instance->hsi_s = 0.5 + (0.5 * (randint_weighted_towards_max(100, 1000, intensity) / (float)1000));
    instance->hsi_i = 0.1 + (0.4 * (randint_weighted_towards_max(0, 1000, intensity) / (float)1000));
    return instance;
}

void pattern_rainbow(uint16_t len, uint32_t t, void *data, PatternPrinter printer)
{
    pattern_rainbow_struct *instance = data;

    float periodProgress;
    if (instance->endless)
    {
        // p is total progress for an endless looping, never stopping.
        periodProgress = t / (float)instance->period;
    }
    else
    {
        periodProgress = executeEasing(instance->easing, (t % instance->period) / (float)instance->period);
    }

    HsiColor hsi = {(int)(roundf(instance->hue_from + (instance->hue_width * periodProgress))) % 360, instance->hsi_s, instance->hsi_i};
    for (int i = 0; i < len; i++)
    {
        printer(i, &hsi, data);
    }
}

void pattern_register_rainbow()
{
    pattern_register("rainbow", pattern_rainbow, pattern_rainbow_data, pattern_destroyer_default, &(PatternOptions){1});
}