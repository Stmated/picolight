#include "pattern.h"

#define BUCKET_SIZE_MIDDLE 10
#define BUCKET_SIZE_COLORS 10

typedef struct data_struct
{
    int period;
    float width;
    uint16_t middles[BUCKET_SIZE_MIDDLE];
    HsiaColor colors[BUCKET_SIZE_MIDDLE];
} data_struct;

typedef struct frame_struct
{
    float width;
    float middle;
    RgbwaColor rgbwa;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));
    data->period = randint_weighted_towards_min(2500, 5000, intensity);
    data->width = randint_weighted_towards_max(8, len / (float)20, intensity);

    for (int i = 0; i < BUCKET_SIZE_MIDDLE; i++)
    {
        data->middles[i] = randint(len);
    }

    for (int i = 0; i < BUCKET_SIZE_MIDDLE; i++)
    {
        data->colors[i] = (HsiaColor){randint(360), randint_weighted_towards_max(800, 1000, intensity * 2) / 1000.0, 0.4, 1};
    }

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

    int era = floorf(t / (float)data->period);

    double p = ((t % data->period) / (double)data->period);
    double eased_p = InOut(InExponentE, OutSquareRoot, p);
    frame->width = data->width * eased_p;
    frame->middle = data->width + fmodf(data->middles[era % BUCKET_SIZE_MIDDLE] + (34.5 * (float)era), len - (data->width * 2));

    HsiaColor hsia = data->colors[era % BUCKET_SIZE_COLORS];
    if (p > 0.5)
    {
        frame->rgbwa = hsia2rgbwa(hsia.h, hsia.s, hsia.i * eased_p, hsia.a);
    }
    else
    {
        frame->rgbwa = hsia2rgbwa(hsia.h, hsia.s, hsia.i, hsia.a);
    }
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    float distance = fabsf(args->i - frame->middle);
    if (frame->width > 0.5 && distance < frame->width)
    {
        // Only do something if larger than half a pixel.
        uint8_t alpha = MIN(RGB_ALPHA_MAX, MAX(0, RGB_ALPHA_MAX * (1 - (distance / frame->width))));
        return (RgbwaColor){frame->rgbwa.r, frame->rgbwa.g, frame->rgbwa.b, frame->rgbwa.w, alpha};
    }

    return *RGBWA_TRANSPARENT;
}

void pattern_register_firework()
{
    pattern_register("firework", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){0.5});
}