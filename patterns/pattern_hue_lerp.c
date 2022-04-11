#include "../patterns.h"

typedef struct data_struct
{
    int easing;
    bool endless;
    int period;
    int hue_from;
    int hue_to;
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
    data->period = randint_weighted_towards_min(5000, 60000, intensity);
    data->hue_from = randint(360);
    data->hue_to = randint_weighted_towards_max(data->hue_from + 50, data->hue_from + 180, intensity) % 360;
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

    //TODO: IF ENDLESS, THEN PERIODPROGRESS CAN BE OVER 1, HENCE GIVING FAULTY HUE IN THE THOUSANDS! NEED TO COVER THIS CASE!
    //ALSO: Rename this pattern from "rainbow" to HUE_LERP, and rename FILL_SWAY to COLOR_LERP

    int hue = math_hue_lerp(data->hue_from, data->hue_to, periodProgress);
    frame->hsi = (HsiaColor){hue, data->hsi_s, data->hsi_i, 1};

    return frame;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, Printer *printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    printer->print(i, &frame->hsi, printer);
}

void pattern_register_hue_lerp()
{
    pattern_register("hue_lerp", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1, 0, true});
}