#include "../patterns.h"

typedef struct data_struct
{
    int easing;
    int endless;
    uint32_t period;
    float hsi_s;
    float hsi_i;
} data_struct;

typedef struct frame_struct
{
    int hueOffset;
    float huePerLed;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->easing = randint(getEasingCount());
    if (randint(1000) > 500)
    {
        if (randint(1000) > 500)
        {
            data->endless = 1;
        }
        else
        {
            // Reverse direction
            data->endless = -1;
        }
    }
    data->period = randint_weighted_towards_min(1000, 30000, intensity);
    data->hsi_s = 0.9 + (0.1 * (randint_weighted_towards_max(100, 1000, intensity) / (float)1000));
    data->hsi_i = 0.2 + (0.3 * (randint_weighted_towards_max(0, 1000, intensity) / (float)1000));
    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    float p;
    if (data->endless != 0)
    {
        // p is total progress for an endless looping, never stopping.
        p = data->endless * (t / (float)data->period);
    }
    else
    {
        p = executeEasing(data->easing, (t % data->period) / (float)data->period);
    }

    frame->huePerLed = (HSI_H_MAX / (float)len);
    frame->hueOffset = ((int)roundf(HSI_H_MAX * p)) % HSI_H_MAX;

    return frame;
}

static inline HsiaColor executor(uint16_t i, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    uint16_t rowHueOffset = (uint16_t)roundf(frame->huePerLed * i);
    int32_t h = (rowHueOffset + frame->hueOffset);
    if (h < 0)
    {
        h = HSI_H_MAX - h;
    }

    HsiaColor c = (HsiaColor){h % HSI_H_MAX, data->hsi_s, data->hsi_i, 1};

    return c;
}

void pattern_register_rainbow_wave()
{
    pattern_register("rainbow_wave", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1, 0, true});
}