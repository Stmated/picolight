#include "../patterns.h"

typedef struct data_struct
{
    CurriedEasing easing;
    CurriedEasing easing_width;
    int period;
    float width;
    float inv_width;

} data_struct;

typedef struct frame_struct
{
    uint16_t hue;
    uint16_t index_center;
    float strength;
    float period_percentage;

} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->period = (float) randint_weighted_towards_min(3000, 10000, intensity);
    data->width = (float) randint_weighted_towards_min(MAX(4, len / 10.0f), MAX(8, len / 3.0f), intensity);
    data->inv_width = 1.0f / data->width;

    data->easing = getRepeatingInOutEasing(rand());
    data->easing_width = getRepeatingInOutEasing(rand());

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

    float p = t / (float) data->period;

    frame->period_percentage = (t % data->period) / (float) data->period;
    
    frame->hue = HSI_H_MAX * data->easing.func(data->easing.ctx, p);

    unsigned int seed = (unsigned int)floorf(p) * 2654435761u;
    frame->index_center = data->width + floorf((rand_r(&seed) / (RAND_MAX + 1.0f)) * (len - (data->width * 2)));
    frame->strength = data->easing_width.func(data->easing_width.ctx, frame->period_percentage);
}

static inline RgbwaColor executor(ExecutorArgs *restrict args)
{
    data_struct *restrict data = args->dataPtr;
    frame_struct *restrict frame = args->framePtr;

    float distance = fabsf(frame->index_center - args->i);

    if (distance <= data->width)
    {
        // TODO: Possible to speed this up? Perhaps by doing all math using integers?
        float distance_percentage = 1 - (distance * data->inv_width);

        uint16_t hue = ((uint16_t)(distance_percentage * HSI_H_MAX) + frame->hue) % HSI_H_MAX;
        float hsia_i = frame->strength * distance_percentage * HSI_I_HALF;
        float hsia_a = frame->strength * distance_percentage;

        return hsia2rgbwa(hue, HSI_S_MAX, hsia_i, hsia_a);
    }

    return RGBWA_TRANSPARENT;
}

/**
 * Pattern which starts at a random pixel and then splashes out a rainbow pattern that then fades, with random easings in and out.
 */
void pattern_register_rainbow_splash()
{
    pattern_register("rainbow_splash", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){1, 0, true});
}