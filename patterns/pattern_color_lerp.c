#include "pattern.h"

typedef struct data_struct
{
    int hue_start;
    int hue_width;
    int speedh;
    int speeds;
    int speedi;

    CurriedEasing easing_h;
    CurriedEasing easing_s;
    CurriedEasing easing_i;

    float sat_from;
    float sat_width;

    float brightness_from;
    float brightness_width;
} data_struct;

typedef struct frame_struct
{
    RgbwaColor rgbwa;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->hue_start = randint(HSI_H_MAX);
    data->hue_width = 60 + randint_weighted_towards_max(0, 120, intensity);
    data->speedh = 10000 + randint_weighted_towards_min(0, 30000, intensity);
    data->speeds = 5000 + randint_weighted_towards_min(0, 20000, intensity);
    data->speedi = 5000 + randint_weighted_towards_min(0, 10000, intensity);

    data->easing_h = getEasing(randint(getEasingCount()));
    data->easing_s = getEasing(randint(getEasingCount()));
    data->easing_i = getEasing(randint(getEasingCount()));

    float sat1 = 0.65 + (0.35 * (randint_weighted_towards_max(0, 100, intensity) / (float)100));
    float sat2 = 0.65 + (0.35 * (randint_weighted_towards_max(0, 100, intensity) / (float)100));

    float int1 = 0.15 + (0.35 * (randint_weighted_towards_max(0, 100, intensity) / (float)100));
    float int2 = 0.15 + (0.35 * (randint_weighted_towards_max(0, 100, intensity) / (float)100));

    data->sat_from = MIN(sat1, sat2);
    data->sat_width = MAX(sat1, sat2) - data->sat_from;

    data->brightness_from = MIN(int1, int2);
    data->brightness_width = MAX(int1, int2) - data->brightness_from;

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

    float ph = InOutLinear((t % data->speedh) / (float)data->speedh);
    float ps = data->easing_s.func(data->easing_s.ctx, (t % data->speeds) / (float)(data->speeds));
    float pi = data->easing_s.func(data->easing_i.ctx, (t % data->speedi) / (float)(data->speedi));

    int h = (int)(data->hue_start + (ph * data->hue_width)) % HSI_H_MAX;
    //HsiaColor hsia = (HsiaColor){};
    frame->rgbwa = hsia2rgbwa(h, data->sat_from + (ps * data->sat_width), data->brightness_from + (pi * data->brightness_width), 1);
}

void pattern_register_color_lerp()
{
    pattern_register("color_lerp", NULL, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){1, 0, true});
}