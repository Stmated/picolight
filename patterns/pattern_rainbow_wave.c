#include "../patterns.h"

typedef struct data_struct
{
    CurriedEasing easing;
    uint32_t period;
    uint8_t hsi_s;
    uint8_t hsi_i;
    float hue_per_led;
} data_struct;

typedef struct frame_struct
{
    uint8_t hue_offset;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    if (randint(1000) > 500)
    {
        data->easing = getRepeatingEasing(rand());
    }
    else
    {
        data->easing = getRepeatingInOutEasing(rand());
    }

    data->period = randint_weighted_towards_min(1000, 30000, intensity);
    data->hsi_s = (255 * 0.9) + randint_weighted_towards_max(0, 255 * 0.1, intensity);
    data->hsi_i = (255 * 0.2) + randint_weighted_towards_max(0, 255 * 0.3, intensity);
    data->hue_per_led = randint_weighted_towards_min((255 / (float)len) / 3, (255 / (float)len) * 2, intensity);
    return data;
}

static void *frame_allocator(uint16_t len, void *dataPtr)
{
    return malloc(sizeof(frame_struct));
}

static void frame_creator(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    frame->hue_offset = 255 * data->easing.func(data->easing.ctx, t / (float)data->period);
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    uint16_t h = ((uint32_t) ((data->hue_per_led * args->i) + frame->hue_offset)) % 255;

    return int8_hsia2rgbwa(h, data->hsi_s, data->hsi_i, 255);
}

void pattern_register_rainbow_wave()
{
    pattern_register("rainbow_wave", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){1, 0, true});
}