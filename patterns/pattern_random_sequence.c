#include "../patterns.h"

typedef struct data_struct
{
    void *data;
    PatternModule *pattern;
    int period;
    int era;
    float intensity;

} data_struct;

typedef struct frame_struct
{
    void *frame;
    float p;

} frame_struct;

static PatternModule *pattern_get_next(PatternModule *prev)
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

        if (module == prev)
        {
            // Give it another chance.
            chances--;
            continue;
        }

        return module;
    }

    return module;
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->period = randint_weighted_towards_min(10000, 60000, intensity);
    data->intensity = intensity;
    data->era = -1;

    return data;
}

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;

    if (data->data)
    {
        data->pattern->destroyer(data->data);
        data->data = NULL;
        data->pattern = NULL;
    }

    free(dataPtr);
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    int era = (int)floor(t / data->period);
    int timeIntoPeriod = t % data->period;
    frame->p = (timeIntoPeriod / (float)data->period);

    if (era != data->era)
    {
        PatternModule *nextModule = pattern_get_next(data->pattern);
        if (data->pattern == NULL)
        {
            printf("Going to '%s'\n", nextModule->name);
        }
        else
        {
            printf("Going from '%s' to '%s'\n", data->pattern->name, nextModule->name);
        }

        if (data->pattern != NULL)
        {
            // Destroy the previous pattern, moving into the new era.
            data->pattern->destroyer(data->data);
            data->pattern = NULL;
        }

        data->pattern = nextModule;
        data->data = data->pattern->creator(len, data->intensity);
    }

    data->era = era;
    frame->frame = data->pattern->frameCreator(len, t, data->data);

    return frame;
}

static void frame_destroyer(void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    if (data->pattern != NULL)
    {
        data->pattern->frameDestroyer(data->data, frame->frame);
        frame->frame = NULL;
    }

    free(framePtr);
}

static HsiaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    if (data->pattern != NULL)
    {
        if (data->pattern->executor == NULL)
        {
            // TODO: This is ugly. Got to remove this "feature"
            return *(HsiaColor *)frame->frame;
        }
        else
        {
            return data->pattern->executor(&(ExecutorArgs){args->i, data->data, frame->frame});
        }
    }
    else
    {
        return (HsiaColor){0, 0, 0, 0};
    }
}

void pattern_register_random_sequence()
{
    pattern_register("random_sequence", executor, data_creator, data_destroyer, frame_creator, frame_destroyer, (PatternOptions){0});
}