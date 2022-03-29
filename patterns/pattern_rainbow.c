#include "../patterns.h"

typedef struct data_struct
{
    int easing;
    bool endless;
    int period;
    int hue_from;
    int hue_width;
    float hsi_s;
    float hsi_i;
} data_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *instance = malloc(sizeof(data_struct));

    instance->easing = randint(getEasingCount());
    instance->endless = randint(1000) > 500;
    instance->period = randint_weighted_towards_min(5000, 60000, intensity);
    instance->hue_from = randint(360);
    instance->hue_width = 360 + (360 * randint_weighted_towards_min(10, 1, intensity));
    instance->hsi_s = 0.5 + (0.5 * (randint_weighted_towards_max(100, 1000, intensity) / (float)1000));
    instance->hsi_i = 0.1 + (0.4 * (randint_weighted_towards_max(0, 1000, intensity) / (float)1000));
    return instance;
}

static void executor(uint16_t start, uint16_t stop, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;

    float periodProgress;
    if (data->endless)
    {
        // p is total progress for an endless looping, never stopping.
        periodProgress = t / (float)data->period;
    }
    else
    {
        periodProgress = executeEasing(data->easing, (t % data->period) / (float)data->period);
    }

    HsiColor hsi = {(int)(roundf(data->hue_from + (data->hue_width * periodProgress))) % 360, data->hsi_s, data->hsi_i};
    for (int i = start; i < stop; i++)
    {
        printer(i, &hsi, dataPtr);
    }
}

void pattern_register_rainbow()
{
    pattern_register("rainbow", executor,
                     data_creator, NULL,
                     NULL, NULL,
                     &(PatternOptions){1});
}