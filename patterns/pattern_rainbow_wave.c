#include "../patterns.h"

typedef struct data_struct
{
    int easing;
    bool endless;
    int period;
    float hsi_s;
    float hsi_i;
} data_struct;

typedef struct frame_struct
{
    float p;
    float huePerLed;
    HsiColor hsi;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->easing = randint(getEasingCount());
    data->endless = randint(1000) > 500;
    data->period = randint_weighted_towards_min(1000, 30000, intensity);
    data->hsi_s = 0.9 + (0.1 * (randint_weighted_towards_max(100, 1000, intensity) / (float)1000));
    data->hsi_i = 0.1 + (0.4 * (randint_weighted_towards_max(0, 1000, intensity) / (float)1000));
    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

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

    frame->p = p;
    frame->huePerLed = (360.0 / (float)len);
    frame->hsi = (HsiColor){0, data->hsi_s, data->hsi_i};

    return frame;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    float base = (frame->huePerLed * i);
    float offset = (360 * frame->p);
    frame->hsi.h = (int)roundf(base + offset) % 360;
    printer(i, &frame->hsi, dataPtr, parentDataPtr);
}

void pattern_register_rainbow_wave()
{
    pattern_register("rainbow_wave", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1});
}