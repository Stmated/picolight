#include "../patterns.h"

typedef struct data_struct
{
    void *data1;
    void *data2;
    PatternModule *pattern1;
    PatternModule *pattern2;
    float pattern2alpha;
    int period;
    uint32_t updatedAt;
    float intensity;

} data_struct;

typedef struct frame_struct
{
    void *frame1;
    void *frame2;
    float p;
    float transition_percentage;

} frame_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;

    if (data->data1)
    {
        data->pattern1->destroyer(data->data1);
        data->data1 = NULL;
        data->pattern2->destroyer(data->data2);
        data->data2 = NULL;
    }

    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    int intIntensityMul = 1000;
    int intIntesity = roundf(intensity * intIntensityMul);
    data->intensity = randint_weighted_towards_max(MAX(100, intIntesity / 2), MAX(800, intensity * 2), intensity) / (float)intIntensityMul;
    data->period = randint_weighted_towards_min(1000, 6000, intensity);
    data->updatedAt = 0;

    return data;
}

static PatternModule *pattern_get_next(PatternModule *prev, PatternModule *other)
{
    // TODO: Some patterns do not mix well. Those that cover 100% with 100% alpha. Do we care? Up to chance?
    int chances = 3;
    float f100 = (float)100;
    PatternModule *module = NULL;
    while (chances > 0)
    {
        module = pattern_get_by_index(randint(getPatternCount()));

        float ourChance = randint(100) / f100;
        if (ourChance >= module->options.randomChance)
        {
            // That module had too low chance to be picked this time (or ever).
            continue;
        }

        if (module == prev || module == other)
        {
            // Give it another chance.
            chances--;
            continue;
        }

        return module;
    }

    return module;
}

static void *frame_allocator(uint16_t len, void *dataPtr)
{
    return calloc(1, sizeof(frame_struct));
}

static void frame_creator(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    if (frame->frame1 == NULL || data->updatedAt == 0 || t > (data->updatedAt + data->period))
    {
        // It is time to move on to the next random mix.
        data->updatedAt = t;

        if (!data->pattern2)
        {
            // This is the first time. So assign pattern 2, which will be moved to 1 straight away.
            data->pattern2 = pattern_get_next(NULL, NULL);
            data->data2 = data->pattern2->creator(len, data->intensity);
        }

        if (data->data1)
        {
            data->pattern1->destroyer(data->data1);
            data->data1 = NULL;
        }

        PatternModule *next = pattern_get_next(data->pattern2, data->pattern1);

        data->data1 = data->data2;
        data->pattern1 = data->pattern2;

        data->pattern2 = next;
        data->data2 = data->pattern2->creator(len, data->intensity);

        printf("Going to '%s' and '%s'\n", data->pattern1->name, data->pattern2->name);

        frame->frame1 = data->pattern1->frameAllocator(len, data->data1);
        frame->frame2 = data->pattern2->frameAllocator(len, data->data2);
    }

    frame->p = ((t - data->updatedAt) % data->period) / (float)data->period;

    data->pattern1->frameCreator(len, t, data->data1, frame->frame1);
    if (frame->p >= 0.90f)
    {
        data->pattern2->frameCreator(len, t, data->data2, frame->frame2);


        frame->transition_percentage = (frame->p - 0.90f) * 10.0f;
    }
    else
    {
        frame->transition_percentage = 0;
    }
}

static void frame_destroyer(void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    data->pattern1->frameDestroyer(data->data1, frame->frame1);
    data->pattern2->frameDestroyer(data->data2, frame->frame2);

    free(framePtr);
}

static RgbwaColor executor(ExecutorArgs *restrict  args)
{
    data_struct *restrict data = args->dataPtr;
    frame_struct *restrict frame = args->framePtr;

    if (frame->transition_percentage > 0)
    {
        RgbwaColor a = data->pattern1->executor(&(ExecutorArgs){args->i, data->data1, frame->frame1});
        RgbwaColor b = data->pattern2->executor(&(ExecutorArgs){args->i, data->data2, frame->frame2});

        //float v = (frame->p - 0.90f) * 10.0f;
        return math_rgbwa_lerp(a, b, frame->transition_percentage);
    }
    else
    {
        return data->pattern1->executor(&(ExecutorArgs){args->i, data->data1, frame->frame1});
    }
}

void pattern_register_random()
{
    pattern_register("random", executor, data_creator, data_destroyer, frame_allocator, frame_creator, frame_destroyer, (PatternOptions){0});
}