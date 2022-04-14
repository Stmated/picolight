#include "../patterns.h"

typedef struct data_struct
{
    float period;
    float width;
    //HsiaColor color;
    float s;
    float i;

} data_struct;

typedef struct frame_struct
{
    float p;
    float middle;
    int hue;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));
    data->period = randint_weighted_towards_min(5000, 7000, intensity);
    data->s = 1;
    data->i = 1;// (HsiaColor){0, 1, 1, 1};
    data->width = randint_weighted_towards_max(8, len / (float)20, intensity);

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    frame->p = (fmodf(t, data->period) / data->period);
    frame->middle = fmodf(34.5 * ceilf(t / data->period), len);
    frame->hue = fmodf(47.37 * ceilf(t / data->period), 360);

    return frame;
}

static inline HsiaColor executor(ExecutorArgs *args)
{
    // TODO: Add an edge sparkle, like actual fireworks do

    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    float fullDistance = fabsf(args->i - frame->middle);
    if (fullDistance < data->width)
    {
        float currentWidth = data->width * InOut(InExponentE, OutSquareRoot, frame->p);
        if (currentWidth > 0.5 && fullDistance < currentWidth)
        {
            // Only do something if larger than half a pixel.
            float p = 1 - (fullDistance / currentWidth);

            return (HsiaColor){
                frame->hue,
                data->s,
                data->i,
                1 * p};
        }
        else
        {
            return (HsiaColor){0, 0, 0, 0};
        }
    }
    else
    {
        return (HsiaColor){0, 0, 0, 0};
    }
}

void pattern_register_firework()
{
    pattern_register("firework", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1});
}