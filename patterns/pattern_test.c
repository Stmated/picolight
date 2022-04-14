#include "../patterns.h"

typedef struct data_struct
{
    uint16_t len;
} data_struct;

typedef struct frame_struct
{
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));
    data->len = len;

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    return frame;
}

static inline HsiaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    if (args->i < data->len * 0.1)
    {
        HsiaColor c1 = {0, 1, 1, 1};
        HsiaColor c = c1;
        return c;
    }
    else if (args->i < data->len * 0.2)
    {
        HsiaColor c1 = {0, 1, 1, 1};
        HsiaColor c2 = {240, 1, 1, 1};
        HsiaColor c = math_average_hsia(&c1, &c2);
        return c;
    }
    else if (args->i < data->len * 0.3)
    {
        HsiaColor c1 = {0, 1, 1, 1};
        HsiaColor c2 = {240, 1, 1, 0.8};
        HsiaColor c = math_average_hsia(&c1, &c2);
        return c;
    }
    else if (args->i < data->len * 0.4)
    {
        HsiaColor c1 = {0, 1, 1, 1};
        HsiaColor c2 = {240, 1, 1, 0.6};
        HsiaColor c = math_average_hsia(&c1, &c2);
        return c;
    }
    else if (args->i < data->len * 0.5)
    {
        HsiaColor c1 = {0, 1, 1, 1};
        HsiaColor c2 = {240, 1, 1, 0.4};
        HsiaColor c = math_average_hsia(&c1, &c2);
        return c;
    }
    else if (args->i < data->len * 0.6)
    {
        HsiaColor c1 = {0, 1, 1, 1};
        HsiaColor c2 = {240, 1, 1, 1};
        HsiaColor c3 = {120, 1, 1, 1};
        HsiaColor mix1 = math_average_hsia(&c1, &c2);
        HsiaColor c = math_average_hsia(&mix1, &c3);
        return c;
    }
    else if (args->i < data->len * 0.7)
    {
        HsiaColor c1 = {0, 1, 1, 1};
        HsiaColor c2 = {240, 1, 1, 0.4};
        HsiaColor c3 = {120, 1, 1, 0.5};
        HsiaColor mix1 = math_average_hsia(&c1, &c2);
        HsiaColor c = math_average_hsia(&mix1, &c3);
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
        HsiaColor c = {317, 1, 0.588, 1 - distance};
        return c;
    }
    else
    {
        HsiaColor c1 = {0, 1, 1, 1};
        HsiaColor c2 = {120, 1, 1, 0.5};
        HsiaColor c = math_average_hsia(&c1, &c2);
        return c;
    }
}

void pattern_register_test()
{
    pattern_register("test", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){0});
}