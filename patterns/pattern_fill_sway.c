#include "../patterns.h"

typedef struct data_struct
{
    int hue_start;
    int hue_width;
    int speedh;
    int speeds;
    int speedi;

    int easing_h;
    int easing_s;
    int easing_i;

    float sat_from;
    float sat_width;

    float brightness_from;
    float brightness_width;
} data_struct;

typedef struct frame_struct
{
    HsiColor hsi;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->hue_start = randint(HSI_H_MAX);
    data->hue_width = 60 + randint_weighted_towards_max(0, 120, intensity);
    data->speedh = 10000 + randint_weighted_towards_min(0, 30000, intensity);
    data->speeds = 5000 + randint_weighted_towards_min(0, 20000, intensity);
    data->speedi = 5000 + randint_weighted_towards_min(0, 10000, intensity);

    // TODO: Need a way to select an easing based on intensity, so they are ranked by fastness/intensity
    data->easing_h = randint(getEasingCount());
    data->easing_s = randint(getEasingCount());
    data->easing_i = randint(getEasingCount());

    // TODO: The values MUST NOT GO OVER 1! The calculations become WONKY!!!!
    data->sat_from = 0.8 + (0.2 * (randint_weighted_towards_max(0, 100, intensity) / (float)100));
    data->brightness_from = 0.2 + (0.1 * (randint_weighted_towards_max(0, 100, intensity) / (float)100));

    float sat_to = MIN(1, data->sat_from + 0.1 + (0.4 * (randint_weighted_towards_max(0, 100, intensity) / (float)100)));
    float brightness_to = MIN(1, data->brightness_from + 0.2 + (0.10 * (randint_weighted_towards_max(0, 100, intensity) / (float)100)));

    data->sat_width = (sat_to - data->sat_from);
    data->brightness_width = (brightness_to - data->brightness_from);

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    float ph = executeEasing(data->easing_h, (t % data->speedh) / (float)data->speedh);
    float ps = executeEasing(data->easing_s, (t % data->speeds) / (float)(data->speeds));
    float pi = executeEasing(data->easing_i, (t % data->speedi) / (float)(data->speedi));

    int h = ((int)(data->hue_start + (ph * data->hue_width))) % HSI_H_MAX;

    frame->hsi = (HsiColor){h, data->sat_from + (ps * data->sat_width), data->brightness_from + (pi * data->brightness_width)};

    return frame;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    printer(i, &frame->hsi, dataPtr, parentDataPtr);
}

void pattern_register_fill_sway()
{
    pattern_register("sway", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1});
}