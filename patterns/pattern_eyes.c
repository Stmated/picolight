#include "../patterns.h"

#define BUCKET_SIZE_PERIOD_NONE 2
#define BUCKET_SIZE_PERIOD_WINK 2
#define BUCKET_SIZE_PERIOD_STARE 2
#define BUCKET_SIZE_WINK_REPEATS 2

typedef struct data_struct
{
    int32_t period;

    int32_t period_none[BUCKET_SIZE_PERIOD_NONE];
    int32_t period_wink[BUCKET_SIZE_PERIOD_WINK];
    int32_t period_stare[BUCKET_SIZE_PERIOD_STARE];

    int32_t wink_repeats[BUCKET_SIZE_WINK_REPEATS];
    // int32_t wink_repeats;

} data_struct;

typedef struct frame_struct
{
} frame_struct;

static void data_destroyer(void *dataPtr)
{
    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    return frame;
}

static inline HsiaColor executor(uint16_t i, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    return (HsiaColor){0, 1, 1, 1};
}

void pattern_register_eyes()
{
    pattern_register("eyes", executor, data_creator, data_destroyer, frame_creator, NULL, (PatternOptions){1});
}