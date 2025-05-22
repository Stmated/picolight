#include "../patterns.h"

typedef struct data_struct
{
    CurriedEasing easing;
    uint32_t period;
    float hsi_s;
    float hsi_i;
    float huePerLed;
} data_struct;

typedef struct frame_struct
{
    float hueOffset;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    if (randint(1000) > 500)
    {
        data->easing = getRepeatingEasing(rand());
    }
    else
    {
        data->easing = getRepeatingInOutEasing(rand());
    }

    data->period = randint_weighted_towards_min(1000, 30000, intensity);
    data->hsi_s = 0.9 + (0.1 * (randint_weighted_towards_max(100, 1000, intensity) / (float)1000));
    data->hsi_i = 0.2 + (0.3 * (randint_weighted_towards_max(0, 1000, intensity) / (float)1000));
    data->huePerLed = (HSI_H_MAX / (float)len);
    return data;
}

static void *frame_allocator(uint16_t len, void *dataPtr)
{
    return calloc(1, sizeof(frame_struct));
}

static void frame_creator(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    float p = data->easing.func(data->easing.ctx, t / (float)data->period);
    frame->hueOffset = HSI_H_MAX * p;
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    uint16_t h = ((uint32_t) ((data->huePerLed * args->i) + frame->hueOffset)) % HSI_H_MAX;

    return hsia2rgbwa(h, data->hsi_s, data->hsi_i, 1);
}

void pattern_register_rainbow_wave()
{
    pattern_register("rainbow_wave", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){1, 0, true});
}