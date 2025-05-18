#include "../patterns.h"

typedef struct data_struct
{
    uint16_t len;
} data_struct;

typedef struct frame_struct
{
    //int 
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));
    data->len = len;

    return data;
}

static void *frame_allocator(uint16_t len, uint32_t t, void *dataPtr)
{
    return calloc(1, sizeof(frame_struct));
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr; // calloc(1, sizeof(frame_struct));

    return frame;
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    if (args->i < data->len * 0.1)
    {
        RgbwaColor c1 = {0, RGB_ALPHA_MAX, RGB_ALPHA_MAX, 0, RGB_ALPHA_MAX};
        RgbwaColor c = c1;
        return c;
    }
    else if (args->i < data->len * 0.2)
    {
        RgbwaColor c1 = {0, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor c2 = {240, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor c = math_average_rgbwa(&c1, &c2);
        return c;
    }
    else if (args->i < data->len * 0.3)
    {
        RgbwaColor c1 = {0, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor c2 = {240, 1, 1, 0, RGB_ALPHA_MAX * 0.8};
        RgbwaColor c = math_average_rgbwa(&c1, &c2);
        return c;
    }
    else if (args->i < data->len * 0.4)
    {
        RgbwaColor c1 = {0, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor c2 = {240, 1, 1, 0, RGB_ALPHA_MAX * 0.6};
        RgbwaColor c = math_average_rgbwa(&c1, &c2);
        return c;
    }
    else if (args->i < data->len * 0.5)
    {
        RgbwaColor c1 = {0, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor c2 = {240, 1, 1, 0, RGB_ALPHA_MAX * 0.4};
        RgbwaColor c = math_average_rgbwa(&c1, &c2);
        return c;
    }
    else if (args->i < data->len * 0.6)
    {
        RgbwaColor c1 = {0, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor c2 = {240, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor c3 = {120, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor mix1 = math_average_rgbwa(&c1, &c2);
        RgbwaColor c = math_average_rgbwa(&mix1, &c3);
        return c;
    }
    else if (args->i < data->len * 0.7)
    {
        RgbwaColor c1 = {0, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor c2 = {240, 1, 1, 0, RGB_ALPHA_MAX * 0.4};
        RgbwaColor c3 = {120, 1, 1, 0, RGB_ALPHA_MAX * 0.5};
        RgbwaColor mix1 = math_average_rgbwa(&c1, &c2);
        RgbwaColor c = math_average_rgbwa(&mix1, &c3);
        return c;
    }
    else if (args->i < data->len * 0.9)
    {
        float start = data->len * 0.7;
        float end = data->len * 0.9;
        float width = (end - start);
        float mid = start + (width / (float)2);
        float p = (args->i - start) / width;
        float distance = fabsf(args->i - mid) / (width / (float)2);
        //RgbwaColor c = {317, 1, 0.588, 0, RGB_ALPHA_MAX * (1 - distance)};
        RgbwaColor c = {255, 1, 0.588, 0, RGB_ALPHA_MAX * (1 - distance)};
        return c;
    }
    else
    {
        RgbwaColor c1 = {0, 1, 1, 1, RGB_ALPHA_MAX};
        RgbwaColor c2 = {120, 1, 1, 0, RGB_ALPHA_MAX * 0.5};
        RgbwaColor c = math_average_rgbwa(&c1, &c2);
        return c;
    }
}

void pattern_register_test()
{
    pattern_register("test", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){0});
}