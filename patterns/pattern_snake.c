#include "../patterns.h"
#include "../led_math.h"

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

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->easing = randint(getEasingCount());
    data->hue = randint(360);
    data->width = 3 + randint_weighted_towards_min(len / 16, len / 8, intensity);
    data->period = randint_weighted_towards_min(2000, 30000, intensity);
    data->offset = randint(data->period * 3);
    data->saturation = randint_weighted_towards_max(0, 1000, intensity) / (float)1000;
    data->brightness = randint_weighted_towards_max(200, 500, intensity) / (float)1000;

    return data;
}

static void executor(uint16_t len, uint32_t t, void *dataPtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;

    const float t_into_period = (((t + data->offset) % data->period) / (float)data->period);
    const float p = len * executeEasing(data->easing, t_into_period);

    HsiColor black = {0, 0, 0};
    for (int i = 0; i < len; i++)
    {
        float distance = fabsf(i - p);

        if (distance <= data->width)
        {
            HsiColor hsi = {data->hue, 1, (1 - (distance / (float)data->width))};
            printer(i, &hsi, data);
        }
        else
        {
            printer(i, &black, data);
        }
    }
}

void pattern_register_snake()
{
    pattern_register("snake", executor, data_creator, pattern_destroyer_default, &(PatternOptions){1, 3});
}