#include "pattern.h"

typedef struct data_struct
{
    void *data;
    PatternModule *pattern;
    int period;
    uint32_t updatedAt;
    float intensity;

} data_struct;

typedef struct frame_struct
{
    void *frame;
    float p;
    float alpha;

} frame_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;

    if (data->data)
    {
        data->pattern->destroyer(data->data);
        data->data = NULL;
    }

    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    int intIntensityMul = 1000;
    int intIntesity = roundf(intensity * intIntensityMul);
    data->intensity = randint_weighted_towards_max(MAX(100, intIntesity / 2), MAX(800, intensity * 2), intensity) / (float)intIntensityMul;
    data->period = randint_weighted_towards_min(15000, 60000, intensity);
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

    if (data->data == NULL || data->updatedAt == 0 || t > (data->updatedAt + data->period))
    {
        // It is time to move on to the next random mix.
        data->updatedAt = t;
        PatternModule *next = pattern_get_next(data->pattern, NULL);

        if (frame->frame)
        {
            data->pattern->frameDestroyer(data->data, frame->frame);
            frame->frame = NULL;
        }

        if (data->data)
        {
            data->pattern->destroyer(data->data);
            data->data = NULL;
        }

        data->pattern = next;

        data->data = data->pattern->creator(len, data->intensity);
        frame->frame = data->pattern->frameAllocator(len, data->data);
    }

    frame->p = ((t - data->updatedAt) % data->period) / (float)data->period;

    data->pattern->frameCreator(len, t, data->data, frame->frame);
    if (frame->p >= 0.90f)
    {
        frame->alpha = 1 - ((frame->p - 0.90f) * 10.0f);
    }
    else if (frame->p <= 0.10f)
    {
        frame->alpha = frame->p * 10.0f;
    }
    else
    {
        frame->alpha = 1.0f;
    }
}

static void frame_destroyer(void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    data->pattern->frameDestroyer(data->data, frame->frame);

    free(framePtr);
}

static RgbwaColor executor(ExecutorArgs *restrict  args)
{
    data_struct *restrict data = args->dataPtr;
    frame_struct *restrict frame = args->framePtr;

    RgbwaColor c = data->pattern->executor(&(ExecutorArgs){args->i, data->data, frame->frame});

    if (frame->alpha < 1)
    {
        return (RgbwaColor) {c.r, c.g, c.b, c.w, c.a * frame->alpha};
    }

    return c;
}

void pattern_register_random()
{
    pattern_register("random", executor, data_creator, data_destroyer, frame_allocator, frame_creator, frame_destroyer, (PatternOptions){0});
}