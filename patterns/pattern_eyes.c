#include "pattern.h"

#define BUCKET_SIZE_ACTION 20
#define BUCKET_SIZE_PERIOD_HIDDEN 3
#define BUCKET_SIZE_PERIOD_STARE 4
#define BUCKET_SIZE_PERIOD_WINK 5
#define BUCKET_SIZE_EYES_COLORS BUCKET_SIZE_ACTION

typedef struct data_struct
{
    uint16_t period;

    uint8_t eye_width;

    uint16_t positions[BUCKET_SIZE_ACTION];

    // 0. Hidden, 1. Stare, 2. Wink
    uint8_t actions[BUCKET_SIZE_ACTION];

    uint16_t period_hidden[BUCKET_SIZE_PERIOD_HIDDEN];
    uint16_t period_stare[BUCKET_SIZE_PERIOD_STARE];
    uint16_t period_wink[BUCKET_SIZE_PERIOD_WINK];

    HsiaColor eye_colors[BUCKET_SIZE_EYES_COLORS];

} data_struct;

static void data_destroyer(void *dataPtr)
{
    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    data->period = 2000; // randint_weighted_towards_min(10000, 30000, intensity);
    data->eye_width = 3;

    for (int i = 0; i < BUCKET_SIZE_PERIOD_HIDDEN; i++)
    {
        data->period_hidden[i] = randint_weighted_towards_min(3000, 10000, intensity);
    }

    for (int i = 0; i < BUCKET_SIZE_PERIOD_STARE; i++)
    {
        data->period_stare[i] = randint_weighted_towards_min(2000, 10000, intensity);
    }

    for (int i = 0; i < BUCKET_SIZE_PERIOD_WINK; i++)
    {
        data->period_wink[i] = randint_weighted_towards_min(500, 3000, intensity);
    }

    for (int i = 0; i < BUCKET_SIZE_EYES_COLORS; i++)
    {
        data->eye_colors[i] = (HsiaColor){randint(360), 1, 1, 1};
    }

    int previousAction = 0;
    for (int i = 0; i < BUCKET_SIZE_ACTION; i++)
    {
        // 0 = Hidden
        // 1 = Stare
        // 2 = Wink
        data->actions[i] = randint(3);
    }

    // Last action is always "Hiddden", so we can start over without weird ordering.
    data->actions[BUCKET_SIZE_ACTION - 1] = 0;

    int currentPosition = 5 + randint(len - 5);
    HsiaColor currentColor = (HsiaColor){randint(360), 1, 1, 1};
    for (int i = 0; i < BUCKET_SIZE_ACTION; i++)
    {
        if (data->actions[i] == 0)
        {
            // Every time we hide, we change position.
            currentPosition = data->eye_width + randint(len - (data->eye_width * 2));
            currentColor = (HsiaColor){randint(360), 1, 1, 1};
        }

        // But we keep the same position between times we hide. A bit ugly. Waste of memory.
        data->positions[i] = currentPosition;
        data->eye_colors[i] = currentColor;
    }

    return data;
}

typedef struct frame_struct
{
    uint16_t left_eye_center_index; // Index of left eye pixel
    HsiaColor c;
    float eye_width;
} frame_struct;

static void *frame_allocator(uint16_t len, void *dataPtr)
{
    return calloc(1, sizeof(frame_struct));
}

static void frame_creator(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr; // calloc(1, sizeof(frame_struct));

    int era = floorf(t / (float)data->period);
    int timeIntoEra = t % data->period;
    float pIntoEra = (timeIntoEra / (float)data->period);

    frame->left_eye_center_index = data->positions[era % BUCKET_SIZE_ACTION];

    int currentActionIndex = (era % BUCKET_SIZE_ACTION);
    int currentAction = data->actions[currentActionIndex];
    int previousAction = (currentActionIndex == 0) ? data->actions[BUCKET_SIZE_ACTION - 1] : data->actions[currentActionIndex - 1];
    int nextAction = data->actions[(currentActionIndex + 1) % BUCKET_SIZE_ACTION];

    // 0 = Hidden
    // 1 = Stare
    // 2 = Wink

    float alpha = 1;
    if (previousAction == 0 && currentAction != 0)
    {
        // Going from hidden to visible
        // We do this in the first 10% of the progress.
        if (pIntoEra < 0.1)
        {
            alpha = MIN(1, (pIntoEra * 10));
        }
    }
    else if (currentAction != 0 && nextAction == 0)
    {
        // Going from visible to hidden
        if (pIntoEra > 0.9)
        {
            alpha = MIN(1, MAX(0, (1 - pIntoEra) * 10));
        }
    }
    else if (currentAction == 0)
    {
        alpha = 0;
    }

    if (currentAction == 2)
    {
        frame->eye_width = data->eye_width * Flip(InOutCubic(pIntoEra));
    }
    else
    {
        frame->eye_width = data->eye_width;
    }

    HsiaColor c = data->eye_colors[era % BUCKET_SIZE_EYES_COLORS];
    frame->c = (HsiaColor){c.h, c.s, c.i, alpha};
}

/**
 * TODO:
 * This pattern sucks. It flickers (a feature? :D) and the code is bad. Needs to be completely redone in a smarter way.
 *
 * */
static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    double distanceFromLeftEyeCenter = abs(args->i - frame->left_eye_center_index);
    double distanceFromRightEyeCenter = abs(args->i - (frame->left_eye_center_index + data->eye_width + 4));

    int closestDistance = MIN(distanceFromLeftEyeCenter, distanceFromRightEyeCenter);

    if (closestDistance <= frame->eye_width)
    {
        return hsia2rgbwa(frame->c.h, frame->c.s, frame->c.i, frame->c.a * (1 - (closestDistance / frame->eye_width)));
    }

    return (RgbwaColor){0, 0, 0, 0, 0};
}

void pattern_register_eyes()
{
    pattern_register("eyes", executor, data_creator, data_destroyer, frame_allocator, frame_creator, NULL, (PatternOptions){1});
}
