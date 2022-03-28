#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_fade_between_struct
{
    HsiColor *colors;
    int colors_size;
    int time_per_color;
} pattern_fade_between_struct;

void pattern_fade_between_data_destroyer(void *data)
{
    pattern_fade_between_struct *instance = data;
    if (instance->colors)
    {
        free(instance->colors);
    }

    free(data);
}

void *pattern_fade_between_data(uint16_t len, float intensity)
{
    // TODO: Create a bunch of data for different kinds of fades
    pattern_fade_between_struct *instance = malloc(sizeof(pattern_fade_between_struct));

    instance->colors_size = 3 + randint(10);
    instance->time_per_color = 3000 + randint_weighted_towards_min(0, 30000, intensity);
    instance->colors = calloc(instance->colors_size, sizeof(HsiColor));

    // We use the same saturation and intensity for all different hues for this pattern.
    float hsi_s = 0.5 + 0.5 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000);
    float hsi_i = 0.1 + 0.5 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000);

    for (int i = 0; i < instance->colors_size; i++)
    {
        instance->colors[i * sizeof(HsiColor)] = (HsiColor){randint(360), hsi_s, hsi_i};
    }

    return instance;
}

void pattern_fade_between(uint16_t len, uint32_t t, void *data, PatternPrinter printer)
{
    pattern_fade_between_struct *instance = data;

    float totalColorStepProgress = (t / (float)instance->time_per_color);
    int colorIndex = ((int)totalColorStepProgress) % instance->colors_size;
    int colorIndex2 = (colorIndex + 1) % instance->colors_size;

    HsiColor hsi_from = instance->colors[colorIndex * sizeof(HsiColor)];
    HsiColor hsi_to = instance->colors[colorIndex2 * sizeof(HsiColor)];

    // If on color step 3.2, then we will get 0.2, since the integral value is removed.
    float percentage_into_color = totalColorStepProgress - ((int)floorf(totalColorStepProgress));
    HsiColor result = LerpHSI(&hsi_from, &hsi_to, percentage_into_color);

    for (int i = 0; i < len; i++)
    {
        printer(i, &result, data);
    }
}

void pattern_register_fade_between()
{
    pattern_register("fade", pattern_fade_between, pattern_fade_between_data, pattern_fade_between_data_destroyer, &(PatternOptions){1});
}