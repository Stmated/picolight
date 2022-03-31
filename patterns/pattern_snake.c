#include "../patterns.h"

typedef struct data_struct
{
    int easing;
    int hue;
    int period;
    int width;
    bool affectSaturation;
    float saturation;
    float brightness;
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
    data->hue = randint(360);
    data->width = randint_weighted_towards_min(MAX(3, len / 32), len / 8, intensity);
    data->period = randint_weighted_towards_min(2000, 30000, intensity);
    data->offset = randint(data->period * 3);
    data->saturation = randint_weighted_towards_max(800, 1000, intensity * 4) / (float)1000;
    data->affectSaturation = randint_weighted_towards_min(0, 1000, intensity) > 500;
    data->brightness = randint_weighted_towards_max(300, 1000, intensity) / (float)1000;

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));
    frame->t_into_period = (((t + data->offset) % data->period) / (float)data->period);
    frame->p = len * executeEasing(data->easing, frame->t_into_period);

    return frame;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    float distance = fabsf(i - frame->p);

    if (distance <= data->width)
    {
        // Move "hsi" into frame memory, and keep writing over the hue + i attributes? We save a couple of cpu cycles?
        float distanceMultiplier = (1 - (distance / (float)data->width));
        HsiColor hsi = {data->hue, data->saturation, data->brightness * distanceMultiplier};
        if (data->affectSaturation)
        {
            hsi.s *= distanceMultiplier;   
        }
        
        printer(i, &hsi, dataPtr, parentDataPtr);
    }
    else
    {
        printer(i, &black, dataPtr, parentDataPtr);
    }
}

void pattern_register_snake()
{
    pattern_register("snake", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1, 3});
}