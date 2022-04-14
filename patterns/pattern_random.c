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

    data->intensity = intensity;
    data->period = randint_weighted_towards_min(10000, 60000, intensity);
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

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    if (data->updatedAt == 0 || t > (data->updatedAt + data->period))
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

        // Ugly workaround so progress goes 0->1,1->0, and so on.
        // This way there should not be an instant JUMP when we switch to next pattern.
        // data->base.progressReversed = !data->base.progressReversed;
    }

    // Set the progress so we can calculate the proper crossover
    //int age = t - data->updatedAt;

    frame->frame1 = data->pattern1->frameCreator(len, t, data->data1);
    frame->frame2 = data->pattern2->frameCreator(len, t, data->data2);
    frame->p = ((t - data->updatedAt) % data->period) / (float)data->period;

    return frame;
}

static void frame_destroyer(void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    data->pattern1->frameDestroyer(data->data1, frame->frame1);
    data->pattern2->frameDestroyer(data->data2, frame->frame2);

    free(framePtr);
}

static HsiaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    HsiaColor a = data->pattern1->executor(&(ExecutorArgs){args->i, data->data1, frame->frame1});
    HsiaColor b = data->pattern2->executor(&(ExecutorArgs){args->i, data->data2, frame->frame2});

    a.a = a.a * (1 - frame->p);
    b.a = b.a * frame->p;

    // Now let's send the data to the original printer
    return math_average_hsia(&a, &b);
}

void pattern_register_random()
{
    pattern_register("random", executor, data_creator, data_destroyer, frame_creator, frame_destroyer, (PatternOptions){0});
}