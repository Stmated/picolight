#include "../patterns.h"

typedef struct data_struct
{
    CurriedEasing easing;
    int period;
    float width;

} data_struct;

typedef struct frame_struct
{
    int hue;
    uint16_t index_center;
    float strength;

} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->period = (float) randint_weighted_towards_min(2000, 4000, intensity);
    data->width = (float) randint_weighted_towards_min(MAX(4, len / 10.0f), MAX(8, len / 3.0f), intensity);

    data->easing = createCurriedEasing(OutBounce, None, true);

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
    
    frame->hue = HSI_H_MAX * data->easing.func(data->easing.ctx, p);

    unsigned int seed = (unsigned int)floorf(p) * 2654435761u;
    frame->index_center = floorf((rand_r(&seed) / (RAND_MAX + 1.0f)) * len);
    frame->strength = 1 - powf((t % data->period) / (float) data->period, 1);
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    if (args->i == frame->index_center)
    {
        return hsia2rgbwa(frame->hue, HSI_S_MAX, HSI_I_HALF, frame->strength);
    }

    return RGBWA_TRANSPARENT;
}

/**
 * Pattern which has a randomly moving dot that bounces in color and fades out
 */
void pattern_register_rainbow_bounce_dot()
{
    pattern_register("rainbow_bounce_dot", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){1, 0, true});
}