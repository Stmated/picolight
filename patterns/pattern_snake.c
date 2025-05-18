#include "../patterns.h"

typedef struct data_struct
{
    int easing;
    // int hue;
    int period;
    double width;
    // bool affectSaturation;
    //double edgeHarshness;
    // float saturation;
    // float brightness;

    RgbwaColor color;

    int offset;
} data_struct;

typedef struct frame_struct
{
    float t_into_period;
    float p;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->easing = randint(getEasingCount());

    HsiaColor hsia = (HsiaColor){
        randint(360),
        randint_weighted_towards_max(800, 1000, intensity * 4) / (float)1000,
        randint_weighted_towards_max(500, 1000, intensity) / (float)1000,
        1};
    data->color = (RgbwaColor)hsia2rgbwa(&hsia);

    // data->hue = ;
    // data->saturation = ;
    // data->affectSaturation = randint_weighted_towards_min(0, 1000, intensity) > 500;
    // data->brightness = ;

    data->width = randint_weighted_towards_min(MAX(3, len / 32), MAX(4, len / 8), intensity);
    data->period = randint_weighted_towards_min(2000, 30000, intensity);
    data->offset = randint(data->period * 3);
    //data->edgeHarshness = randint_weighted_towards_max(1, 100, intensity / (float)4);

    return data;
}

static void *frame_allocator(uint16_t len, uint32_t t, void *dataPtr)
{
    //frame_struct *frame = 
    return calloc(1, sizeof(frame_struct));
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr; // calloc(1, sizeof(frame_struct));
    frame->t_into_period = (((t + data->offset) % data->period) / (float)data->period);
    frame->p = len * executeEasing(data->easing, frame->t_into_period);

    return frame;
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;
    float distance = fabsf(args->i - frame->p);

    if (distance <= data->width)
    {
        //return (RgbwaColor){data->color.r, data->color.g, data->color.b, data->color.w, RGB_ALPHA_MAX * (1 - pow(distance / data->width, data->edgeHarshness))};
        return (RgbwaColor){data->color.r, data->color.g, data->color.b, data->color.w, RGB_ALPHA_MAX * (1 - (distance / data->width))};
    }
    else
    {
        return (RgbwaColor){0, 0, 0, 0, 0};
    }
}

void pattern_register_snake()
{
    pattern_register("snake", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){0.5, 3});
}