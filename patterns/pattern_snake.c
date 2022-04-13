#include "../patterns.h"

typedef struct data_struct
{
    int easing;
    int hue;
    int period;
    int width;
    bool affectSaturation;
    float edgeHarshness;
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
    data->brightness = randint_weighted_towards_max(500, 1000, intensity) / (float)1000;
    data->edgeHarshness = randint_weighted_towards_max(1, 100, intensity / 4);

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

static inline HsiaColor executor(uint16_t i, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;
    float distance = fabsf(i - frame->p);

    if (distance <= data->width)
    {
        float distanceMultiplier = 1 - powf(distance / (float)data->width, data->edgeHarshness);
        HsiaColor hsi = {data->hue, data->saturation, data->brightness, distanceMultiplier};
        if (data->affectSaturation)
        {
            hsi.s *= distanceMultiplier;
        }
        
        return hsi;
    }
    else
    {
        return COLOR_TRANSPARENT;
    }
}

void pattern_register_snake()
{
    pattern_register("snake", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1, 3});
}