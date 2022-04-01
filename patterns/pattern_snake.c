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

//void hello()
//{
//    int i = 0;
//}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, Printer *printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;
    // TODO: Ability for a pattern to say "I am done. Ignore me for the rest of the frame"
    // TODO: Also add ability for pattern to say "I will not start yet for this frame. I will start on index 'i'"
    float distance = fabsf(i - frame->p);

    if (distance <= data->width)
    {
        float distanceMultiplier = (1 - (distance / (float)data->width));
        HsiaColor hsi = {data->hue, data->saturation, data->brightness, distanceMultiplier};
        if (data->affectSaturation)
        {
            //if (distanceMultiplier > 1)
            //{
            //    hello();
            //}
            hsi.s *= distanceMultiplier;
            //if (hsi.s > 1)
            //{
            //    hello();
            //}
        }
        
        printer->print(i, &hsi, printer);
    }
    else
    {
        printer->print(i, &transparent, printer);
    }
}

void pattern_register_snake()
{
    pattern_register("snake", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1, 3});
}