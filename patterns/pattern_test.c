#include "pattern.h"
#include "../environment/environment.h"

typedef struct data_struct
{
    uint16_t len;
    float alpha_width;
    int time_elapsed_width;
    // float fps_value_per_led;
} data_struct;

typedef struct frame_struct
{
    uint16_t time_elapsed;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));
    data->len = len;
    data->alpha_width = 20;
    data->time_elapsed_width = 16;
    // data->fps_value_per_led = FLT_MAX / data->time_elapsed_width;

    return data;
}

static void *frame_allocator(uint16_t len, void *dataPtr)
{
    return calloc(1, sizeof(frame_struct));
}

static void frame_creator(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
    static uint64_t previous_t = -1;

    // Use more resolution with microseconds, and to ignore any time dilation.
    uint64_t current_t = get_running_us();

    frame_struct *frame = framePtr;

    if (previous_t != -1)
    {
        // uint16_t time_elapsed = ;
        frame->time_elapsed = (current_t - previous_t); // (1000.0f / time_elapsed);
    }
    else
    {
        frame->time_elapsed = 0;
    }

    previous_t = current_t;
}

char nth_digit(int val, int n)
{
    char buffer[7]; // enough space for large negative int and null terminator
    sprintf(buffer, "%i", val);
    return buffer[0] == '-' ? buffer[n + 1] : buffer[n];
}

//static const uint32_t powers[8] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    int alpha_start = 15;
    int fps_start = alpha_start + data->alpha_width + 2;

    if (args->i == 0)
    {
        return (RgbwaColor){CHANNEL_MAX, 0, 0, 0, RGB_ALPHA_MAX};
    }
    else if (args->i == 1)
    {
        return (RgbwaColor){0, CHANNEL_MAX, 0, 0, RGB_ALPHA_MAX};
    }
    else if (args->i == 2)
    {
        return (RgbwaColor){0, 0, CHANNEL_MAX, 0, RGB_ALPHA_MAX};
    }
    else if (args->i == 4)
    {
        return (RgbwaColor){0, 0, 0, CHANNEL_MAX, RGB_ALPHA_MAX};
    }
    else if (args->i == 6 || args->i == 7)
    {
        return (RgbwaColor){RGB_ALPHA_MAX, RGB_ALPHA_MAX, 0, 0, RGB_ALPHA_MAX};
    }
    else if (args->i == 9 || args->i == 10)
    {
        return (RgbwaColor){0, RGB_ALPHA_MAX, RGB_ALPHA_MAX, 0, RGB_ALPHA_MAX};
    }
    else if (args->i == 12 || args->i == 13)
    {
        return (RgbwaColor){RGB_ALPHA_MAX, 0, RGB_ALPHA_MAX, 0, RGB_ALPHA_MAX};
    }
    else if (args->i > alpha_start && args->i <= (alpha_start + data->alpha_width))
    {
        float percentage = (args->i - alpha_start) / data->alpha_width;
        return (RgbwaColor){RGB_ALPHA_MAX, 0, 0, 0, RGB_ALPHA_MAX * percentage};
    }
    else if (args->i == fps_start)
    {
        int hue = HSI_H_QUARTER + ((frame->time_elapsed / 10000.0f) * (HSI_H_MAX - HSI_H_QUARTER));
        
        HsiaColor c = (HsiaColor){hue, HSI_S_MAX, HSI_I_HALF, HSI_A_MAX};
        RgbwaColor rgb = hsia2rgbwa(c.h, c.s, c.i, c.a);

        return rgb;
    }
    else if (args->i == 10)
    {
        return (RgbwaColor){0, 0, 0, 0, RGB_ALPHA_MAX};
    }
    /*
    else if (args->i < data->len * 0.1)
    {
        return (RgbwaColor) {0, RGB_ALPHA_MAX, RGB_ALPHA_MAX, 0, RGB_ALPHA_MAX};
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
    */
    else
    {
        return *RGBWA_TRANSPARENT;

        // RgbwaColor c1 = {0, 1, 1, 1, RGB_ALPHA_MAX};
        // RgbwaColor c2 = {120, 1, 1, 0, RGB_ALPHA_MAX * 0.5};
        // RgbwaColor c = math_average_rgbwa(&c1, &c2);
        // return c;
    }
}

void pattern_register_test()
{
    pattern_register("test", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){0});
}