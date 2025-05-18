#include "../patterns.h"

typedef struct frame_struct
{
    RgbwaColor c;
} frame_struct;

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    if ((t % 100) < 50)
    {
        frame->c = (RgbwaColor){RGB_ALPHA_MAX, RGB_ALPHA_MAX, RGB_ALPHA_MAX, RGB_ALPHA_MAX, RGB_ALPHA_MAX};
    }
    else
    {
        frame->c = (RgbwaColor){0, 0, 0, 0, 0};
    }

    return frame;
}

void pattern_register_strobe()
{
    pattern_register("strobe", NULL, NULL, NULL, frame_creator, NULL, (PatternOptions){0.1});
}