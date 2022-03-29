#include "../patterns.h"

typedef struct data_struct
{
    HsiColor *colors;
    int colors_size;
    int time_per_color;
} data_struct;

static void data_destroyer(void *data)
{
    data_struct *instance = data;
    if (instance->colors)
    {
        free(instance->colors);
    }

    free(data);
}

static void *data_creator(uint16_t len, float intensity)
{
    // TODO: Create a bunch of data for different kinds of fades
    data_struct *data = malloc(sizeof(data_struct));

    data->colors_size = 3 + randint(10);
    data->time_per_color = 3000 + randint_weighted_towards_min(0, 30000, intensity);
    data->colors = calloc(data->colors_size, sizeof(HsiColor));

    // We use the same saturation and intensity for all different hues for this pattern.
    float hsi_s = 0.5 + 0.5 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000);
    float hsi_i = 0.1 + 0.5 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000);

    for (int i = 0; i < data->colors_size; i++)
    {
        data->colors[i * sizeof(HsiColor)] = (HsiColor){randint(360), hsi_s, hsi_i};
    }

    return data;
}

static void executor(uint16_t start, uint16_t stop, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;

    float totalColorStepProgress = (t / (float)data->time_per_color);
    int colorIndex = ((int)totalColorStepProgress) % data->colors_size;
    int colorIndex2 = (colorIndex + 1) % data->colors_size;

    HsiColor hsi_from = data->colors[colorIndex * sizeof(HsiColor)];
    HsiColor hsi_to = data->colors[colorIndex2 * sizeof(HsiColor)];

    // If on color step 3.2, then we will get 0.2, since the integral value is removed.
    float percentage_into_color = totalColorStepProgress - ((int)floorf(totalColorStepProgress));
    HsiColor result = LerpHSI(&hsi_from, &hsi_to, percentage_into_color);

    for (int i = start; i < stop; i++)
    {
        printer(i, &result, dataPtr);
    }
}

void pattern_register_fade_between()
{
    pattern_register("fade", executor,
                     data_creator, data_destroyer,
                     NULL, NULL,
                     &(PatternOptions){1});
}