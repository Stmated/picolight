#include "pattern.h"

typedef struct data_struct
{
    CurriedEasing easing_hue;
    CurriedEasing easing_width;
    float period;
    float width;
    float inv_width;
    uint8_t hue_offset;
    float hue_speed_multiplier;

} data_struct;

typedef struct frame_struct
{
    uint8_t hue;
    uint16_t index_center;
    uint8_t strength;

} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    data->period = (float)randint_weighted_towards_min(3000, 10000, intensity);

    data->width = (float)randint_weighted_towards_max(MAX(4, len / 10.0f), MAX(8, len / 3.0f), intensity);
    data->hue_speed_multiplier = randint_weighted_towards_max(10, 100, intensity / 2.0f) / 10.0f;
    data->hue_offset = randint(255);
    
    data->inv_width = 1.0f / data->width;

    if (rand() > RAND_MAX / 2.0f)
    {
        data->easing_hue = getRepeatingInOutEasing(rand());
    }
    else
    {
        data->easing_hue = getRepeatingEasing(rand());
    }

    //data->easing_hue = getRepeatingEasing(rand());
    data->easing_width = getRepeatingInOutEasing(rand());

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

    float p = t / data->period;

    frame->hue = (data->hue_offset + (uint16_t) (255 * data->easing_hue.func(data->easing_hue.ctx, p * data->hue_speed_multiplier))) % 255;

    unsigned int seed = (unsigned int)floorf(p) * 2654435761u;
    frame->index_center = data->width + floorf((rand_r(&seed) / (RAND_MAX + 1.0f)) * (len - (data->width * 2)));
    frame->strength = 255 * data->easing_width.func(data->easing_width.ctx, p);
}

static inline RgbwaColor executor(ExecutorArgs *restrict args)
{
    data_struct *restrict data = args->dataPtr;
    frame_struct *restrict frame = args->framePtr;

    uint16_t distance_from_center = abs(frame->index_center - args->i);

    if (distance_from_center <= data->width)
    {
        uint8_t distance_from_center_as_1byte = (1 - (distance_from_center * data->inv_width)) * 255;

        uint8_t hue = (distance_from_center_as_1byte + frame->hue) % 255;

        uint16_t weighted_dp = frame->strength * distance_from_center_as_1byte;
        uint8_t hsia_i = weighted_dp >> 9; // ~half as strong
        uint8_t hsia_a = weighted_dp >> 8; // full

        return int8_hsia2rgbwa(hue, 255, hsia_i, hsia_a);
    }

    return *RGBWA_TRANSPARENT;
}

/**
 * Pattern which starts at a random pixel and then splashes out a rainbow pattern that then fades, with random easings in and out.
 */
void pattern_register_rainbow_splash()
{
    pattern_register("rainbow_splash", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){1, 0, true});
}