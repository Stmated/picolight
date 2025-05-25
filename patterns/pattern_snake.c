#include "../patterns.h"

typedef struct data_struct
{
    CurriedEasing easing;
    uint32_t period;
    float width;

    RgbwaColor color;

    uint32_t offset;
} data_struct;

typedef struct frame_struct
{
    float head_index;
} frame_struct;

static RgbwaColor getNewColor(float intensity)
{
    HsiaColor hsia = (HsiaColor){
        randint(HSI_H_MAX),
        randint_weighted_towards_max(800, 1000, intensity * 4) / (float)1000,
        randint_weighted_towards_max(200, 750, intensity) / (float)1000,
        1};

    return (RgbwaColor)hsia2rgbwa(hsia.h, hsia.s, hsia.i, hsia.a);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->easing = getEasing(randint(getEasingCount()));
    data->color = getNewColor(intensity);

    int width_min = MAX(3, len / 32.0);
    int width_max = width_min + MAX(2, len / 12.0);

    data->width = randint_weighted_towards_min(width_min, width_max, intensity);
    data->period = randint_weighted_towards_min(4000, 45000, intensity);
    data->offset = randint(data->period * 3);

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

    float percentage_into_period = (((t + data->offset) % data->period) / (float)data->period);
    
    // Set index of the snake head
    frame->head_index = len * data->easing.func(data->easing.ctx, percentage_into_period);
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    // TODO: "Darken" the HSIA here, instead of only relying on the alpha -- or perhaps we need to improve the alpha rendering?
    float distance = fabsf(args->i - frame->head_index);

    if (distance <= data->width)
    {
        uint8_t a = RGB_ALPHA_MAX * (1 - (distance / data->width));
        return (RgbwaColor){data->color.r, data->color.g, data->color.b, data->color.w, a};
    }
    else
    {
        return RGBWA_TRANSPARENT;
    }
}

void pattern_register_snake()
{
    pattern_register("snake", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){0.5, 3});
}