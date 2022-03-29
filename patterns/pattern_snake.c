#include "../patterns.h"

typedef struct data_struct
{
    int easing;
    int hue;
    int period;
    int width;
    float saturation;
    float brightness;
    int offset;
} data_struct;

typedef struct cycle_struct
{
    float t_into_period;
    float p;
} cycle_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->easing = randint(getEasingCount());
    data->hue = randint(360);
    data->width = 3 + randint_weighted_towards_min(len / 16, len / 8, intensity);
    data->period = randint_weighted_towards_min(2000, 30000, intensity);
    data->offset = randint(data->period * 3);
    data->saturation = randint_weighted_towards_max(800, 1000, intensity * 4) / (float)1000;
    data->brightness = randint_weighted_towards_max(300, 1000, intensity) / (float)1000;

    return data;
}

static void *cycle_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    cycle_struct *cycle = calloc(1, sizeof(cycle_struct));
    cycle->t_into_period = (((t + data->offset) % data->period) / (float)data->period);
    cycle->p = len * executeEasing(data->easing, cycle->t_into_period);

    return cycle;
}

inline static void executor(uint16_t start, uint16_t stop, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    cycle_struct *cycle = cyclePtr;

    for (int i = start; i < stop; i++)
    {
        float distance = fabsf(i - cycle->p);

        if (distance <= data->width)
        {
            // Move "hsi" into cycle memory, and keep writing over the hue + i attributes? We save a couple of cycles?
            HsiColor hsi = {data->hue, data->saturation, data->brightness * (1 - (distance / (float)data->width))};
            printer(i, &hsi, dataPtr);
        }
        else
        {
            printer(i, &black, dataPtr);
        }
    }
}

void pattern_register_snake()
{
    pattern_register("snake", executor, data_creator, NULL, cycle_creator, NULL, &(PatternOptions){1, 3});
}