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

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    if (i < data->len * 0.1)
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor c = black;
        printer(i, &c, dataPtr, parentDataPtr);
    }
    else if (i < data->len * 0.2)
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor turqoise = {190, 1, 1, 1};
        HsiaColor c = math_average_hsia2(&black, &turqoise);
        printer(i, &c, dataPtr, parentDataPtr);
    }
    else if (i < data->len * 0.3)
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor turqoise80 = {190, 1, 1, 0.8};
        HsiaColor c = math_average_hsia2(&black, &turqoise80);
        printer(i, &c, dataPtr, parentDataPtr);
    }
    else if (i < data->len * 0.4)
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor turqoise60 = {190, 1, 1, 0.6};
        HsiaColor c = math_average_hsia2(&black, &turqoise60);
        printer(i, &c, dataPtr, parentDataPtr);
    }
    else if (i < data->len * 0.5)
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor turqoise40 = {190, 1, 1, 0.4};
        HsiaColor c = math_average_hsia2(&black, &turqoise40);
        printer(i, &c, dataPtr, parentDataPtr);
    }
    else if (i < data->len * 0.6)
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor turqoise = {190, 1, 1, 1};
        HsiaColor green = {115, 1, 1, 1};
        HsiaColor mix1 = math_average_hsia2(&black, &turqoise);
        HsiaColor c = math_average_hsia2(&mix1, &green);
        printer(i, &c, dataPtr, parentDataPtr);
    }
    else if (i < data->len * 0.7)
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor turqoise80 = {190, 1, 1, 0.8};
        HsiaColor green50 = {115, 1, 1, 0.5};
        HsiaColor mix1 = math_average_hsia2(&black, &turqoise80);
        HsiaColor c = math_average_hsia2(&mix1, &green50);
        printer(i, &c, dataPtr, parentDataPtr);
    }
    else if (i < data->len * 0.8)
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor turqoise60 = {190, 1, 1, 0.6};
        HsiaColor green50 = {115, 1, 1, 0.5};
        HsiaColor mix1 = math_average_hsia2(&black, &turqoise60);
        HsiaColor c = math_average_hsia2(&mix1, &green50);
        printer(i, &c, dataPtr, parentDataPtr);
    }
    else if (i < data->len * 0.9)
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor turqoise40 = {190, 1, 1, 0.4};
        HsiaColor green50 = {115, 1, 1, 0.5};
        HsiaColor mix1 = math_average_hsia2(&black, &turqoise40);
        HsiaColor c = math_average_hsia2(&mix1, &green50);
        printer(i, &c, dataPtr, parentDataPtr);
    }
    else
    {
        HsiaColor black = {0, 0, 0, 1};
        HsiaColor green50 = {115, 1, 1, 0.5};
        HsiaColor c = math_average_hsia2(&black, &green50);
        printer(i, &c, dataPtr, parentDataPtr);
    }

    /*
    

    float distance = fabsf(i - frame->p);

    if (distance <= data->width)
    {
        float distanceMultiplier = (1 - (distance / (float)data->width));
        HsiaColor hsi = {data->hue, data->saturation, data->brightness, distanceMultiplier};
        if (data->affectSaturation)
        {
            hsi.s *= distanceMultiplier;   
        }
        
        printer(i, &hsi, dataPtr, parentDataPtr);
    }
    else
    {
        printer(i, &transparent, dataPtr, parentDataPtr);
    }
    */
}

void pattern_register_test()
{
    pattern_register("test", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){0});
}