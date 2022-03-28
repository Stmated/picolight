#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_fade_between_struct
{
    int time_per_color;
    float hsi_saturation;
    float hsi_intensity;
} pattern_fade_between_struct;

void *pattern_fade_between_data(uint16_t len, float intensity)
{
    // TODO: Create a bunch of data for different kinds of fades
    pattern_fade_between_struct *instance = malloc(sizeof(pattern_fade_between_struct));

    instance->time_per_color = 20000 - randint_weighted_towards_max(0, 18000, intensity);
    instance->hsi_saturation = 0.5 + 0.5 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000);
    instance->hsi_intensity = 0.1 + 0.5 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000);
    return instance;
}

void pattern_fade_between(uint16_t len, uint32_t t, void *data, PatternPrinter printer)
{
    pattern_fade_between_struct *instance = data;

    int time_into_color = (t % instance->time_per_color);
    double percentage_into_color = (time_into_color / (double)instance->time_per_color);

    int64_t steps = floor(t / (double)instance->time_per_color);

    int hue_from = ((steps)*203) % 360;
    int hue_to = ((steps + 1) * 203) % 360;

    HsiColor hsi_from = {hue_from, instance->hsi_saturation, instance->hsi_intensity};
    HsiColor hsi_to = {hue_to, instance->hsi_saturation, instance->hsi_intensity};

    HsiColor result = LerpHSI(&hsi_from, &hsi_to, percentage_into_color);

    for (int i = 0; i < len; i++)
    {
        printer(i, &result, data);
    }
}

void pattern_register_fade_between()
{
    pattern_register(pattern_fade_between, pattern_fade_between_data, pattern_destroyer_default, &(PatternOptions){1});
}