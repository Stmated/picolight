#include "../patterns.h"

typedef struct frame_struct
{
    uint32_t remainder;
    HsiaColor white;
    HsiaColor black;
} frame_struct;

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    frame->remainder = t % 100;
    frame->white = (HsiaColor){0, 0, 1, 1};
    frame->black = (HsiaColor){0, 0, 0, 0};

    return frame;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, Printer *printer)
{
    frame_struct *frame = framePtr;

    if (frame->remainder < 50)
    {
        printer->print(i, &frame->white, printer);
    }
    else
    {
        printer->print(i, &frame->black, printer);
    }
}

void pattern_register_strobe()
{
    pattern_register("strobe", executor, NULL, NULL, frame_creator, NULL, (PatternOptions){0.1});
}