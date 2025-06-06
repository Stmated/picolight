#include "pattern.h"

typedef struct data_struct
{
    CurriedEasing easing;
    bool endless;
    int period;
    int hue_from;
    int hue_to;
    float hsi_s;
    float hsi_i;
} data_struct;

typedef struct frame_struct
{
    RgbwaColor rgbwa;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->easing = getEasing(randint(getEasingCount()));
    data->endless = randint(1000) > 500;
    data->period = randint_weighted_towards_min(5000, 60000, intensity);
    data->hue_from = HSI_H_MIN; // randint(360);
    data->hue_to = HSI_H_MAX / 2; // randint_weighted_towards_max(data->hue_from + 50, data->hue_from + 180, intensity) % 360;
    /*for (int i = 0; i < 10 && abs(data->hue_from - data->hue_to) < 30; i++) {
        data->hue_to = randint(360);
    }
    */

    data->hsi_s = 0.7 + (0.3 * (randint_weighted_towards_max(100, 1000, intensity) / (float)1000));
    data->hsi_i = 0.1 + (0.4 * (randint_weighted_towards_max(0, 1000, intensity) / (float)1000));
    
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

    float periodProgress;
    if (data->endless)
    {
        // p is total progress for an endless looping, never stopping.
        periodProgress = t / (float)data->period;
    }
    else
    {
        periodProgress = data->easing.func(data->easing.ctx, (t % data->period) / (float)data->period);
    }

    int hue = math_hue_lerp(data->hue_from, data->hue_to, periodProgress);
    frame->rgbwa = hsia2rgbwa(hue, data->hsi_s, data->hsi_i, 1);
}

void pattern_register_hue_lerp()
{
    pattern_register("hue_lerp", NULL, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){1, 0, true});
}