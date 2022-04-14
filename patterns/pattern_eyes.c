#include "../patterns.h"

#define BUCKET_SIZE_POSITION 20
#define BUCKET_SIZE_PERIOD_NONE 2
#define BUCKET_SIZE_PERIOD_WINK 2
#define BUCKET_SIZE_PERIOD_STARE 2
#define BUCKET_SIZE_WINK_REPEATS 2
#define BUCKET_SIZE_EYES_COLORS 2

typedef struct data_struct
{
    uint16_t period;

    uint16_t positions[BUCKET_SIZE_POSITION];

    uint16_t period_none[BUCKET_SIZE_PERIOD_NONE];
    uint16_t period_wink[BUCKET_SIZE_PERIOD_WINK];
    uint16_t wink_repeats[BUCKET_SIZE_WINK_REPEATS];
    uint16_t period_stare[BUCKET_SIZE_PERIOD_STARE];

    HsiaColor eye_colors[BUCKET_SIZE_EYES_COLORS];

} data_struct;

static void data_destroyer(void *dataPtr)
{
    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    data->period = randint_weighted_towards_min(10000, 30000, intensity);

    for (int i = 0; i < BUCKET_SIZE_POSITION; i++)
    {
        data->positions[i] = 5 + randint(len - 5);
    }

    for (int i = 0; i < BUCKET_SIZE_PERIOD_NONE; i++)
    {
        data->period_none[i] = randint_weighted_towards_min(3000, 10000, intensity);
    }

    for (int i = 0; i < BUCKET_SIZE_PERIOD_WINK; i++)
    {
        data->period_wink[i] = randint_weighted_towards_min(500, 3000, intensity);
    }

    for (int i = 0; i < BUCKET_SIZE_WINK_REPEATS; i++)
    {
        data->wink_repeats[i] = randint_weighted_towards_min(1, 5, intensity);
    }

    for (int i = 0; i < BUCKET_SIZE_PERIOD_STARE; i++)
    {
        data->period_stare[i] = randint_weighted_towards_min(2000, 10000, intensity);
    }

    for (int i = 0; i < BUCKET_SIZE_EYES_COLORS; i++)
    {
        data->eye_colors[i] = (HsiaColor){randint(360), 1, 1, 1};
    }

    return data;
}

typedef struct frame_struct
{
    uint16_t position; // Index of first eye pixel
} frame_struct;

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    int era = floorf(t / (float)data->period);
    int timeIntoEra = data->period;

    // TODO: Fade out the eyes in the last 1 second (or 10%, whichever is shortest)

    frame->position = (data->positions[era % BUCKET_SIZE_POSITION] + (era * 13)) % len;


    


    return frame;
}

static inline HsiaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;



    return (HsiaColor){0, 1, 1, 1};
}

void pattern_register_eyes()
{
    pattern_register("eyes", executor, data_creator, data_destroyer, frame_creator, NULL, (PatternOptions){1});
}
