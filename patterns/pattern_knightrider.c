#include "../patterns.h"

typedef struct data_struct
{
    int period;
    float width;
    int len;
} data_struct;

typedef struct frame_struct
{
    float p;
    float headIndex;
} frame_struct;

static void data_destroyer(void *dataPtr)
{
    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    data->period = randint_weighted_towards_min(5000, 12000, intensity);
    data->width = len * 0.60;
    data->len = len;

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    frame->p = (t % data->period) / (float)data->period;
    frame->headIndex = len * InOutLinear(frame->p);

    return frame;
}

static inline HsiaColor executor(uint16_t i, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    float distance = fabsf(i - frame->headIndex);

    if (frame->p < 0.5)
    {
        // Going up
        if (i > frame->headIndex)
        {
            // Add distance to tail
            distance = (i) + (frame->headIndex);
        }
    }
    else
    {
        // Going back down
        if (i < frame->headIndex)
        {
            // Add distance to tail
            distance = (data->len - i) + (data->len - frame->headIndex);
        }
    }

    float a = 1 - (distance / data->width);
    return (HsiaColor){0, 1, 1, a};
}

void pattern_register_knightrider()
{
    pattern_register("knightrider", executor, data_creator, data_destroyer, frame_creator, NULL, (PatternOptions){1});
}