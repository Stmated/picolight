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

typedef struct frame_struct
{
    HsiaColor hsi;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->easing = randint(getEasingCount());
    data->endless = randint(1000) > 500;
    printf("3\n");
    data->period = randint_weighted_towards_min(5000, 60000, intensity);
    printf("4\n");
    data->hue_from = randint(360);
    data->hue_width = 360 + (360 * randint_weighted_towards_min(10, 1, intensity));
    data->hsi_s = 0.7 + (0.3 * (randint_weighted_towards_max(100, 1000, intensity) / (float)1000));
    data->hsi_i = 0.1 + (0.4 * (randint_weighted_towards_max(0, 1000, intensity) / (float)1000));
    
    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

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

    frame->hsi = (HsiaColor){(int)(roundf(data->hue_from + (data->hue_width * periodProgress))) % 360, data->hsi_s, data->hsi_i, 1};

    return frame;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    printer(i, &frame->hsi, dataPtr, parentDataPtr);
}

void pattern_register_rainbow()
{
    pattern_register("rainbow", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1});
}