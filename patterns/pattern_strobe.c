#include "../patterns.h"

typedef struct frame_struct
{
    uint32_t remainder;
    HsiColor white;
    HsiColor black;
} frame_struct;

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    frame->remainder = t % 100;
    frame->white = (HsiColor){0, 0, 1};
    frame->black = (HsiColor){0, 0, 0};

    return frame;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, void *parentDataPtr, PatternPrinter printer)
{
    frame_struct *frame = framePtr;

    if (frame->remainder < 50)
    {
        printer(i, &frame->white, dataPtr, parentDataPtr);
    }
    else
    {
        printer(i, &frame->black, dataPtr, parentDataPtr);
    }
}

void pattern_register_strobe()
{
    pattern_register("strobe", executor, NULL, NULL, frame_creator, NULL, (PatternOptions){0.1});
}