#include "../patterns.h"

typedef struct frame_struct
{
    uint32_t remainder;
} frame_struct;

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    frame->remainder = t % 100;

    return frame;
}

static inline HsiaColor executor(uint16_t i, void *dataPtr, void *framePtr)
{
    frame_struct *frame = framePtr;

    if (frame->remainder < 50)
    {
        return COLOR_WHITE;
    }
    else
    {
        return COLOR_BLACK;
    }
}

void pattern_register_strobe()
{
    pattern_register("strobe", executor, NULL, NULL, frame_creator, NULL, (PatternOptions){0.1});
}