#include "../patterns.h"

typedef struct data_struct
{
    data_pixel_blending_struct base;
    void *data1;
    void *data2;
    PatternModule *pattern1;
    PatternModule *pattern2;
    int period;
    uint32_t updatedAt;
    float intensity;
    int patternIndex1;
    int patternIndex2;

} data_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;

    free(data->base.pixels);

    if (data->data1)
    {
        data->pattern1->destroyer(data->data1);
        data->pattern2->destroyer(data->data2);
    }

    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->intensity = intensity;
    data->period = randint_weighted_towards_min(5000, 6000, intensity);
    data->updatedAt = 0;
    data->patternIndex1 = -1;
    data->patternIndex2 = -1;

    // This is... a lot of data. Can we shrink it?
    data->base.pixels = calloc(len, sizeof(HsiColor)); // Allocate memory for each pixel, which we will need to blend

    return data;
}

static int pattern_random_get_next_pattern_index(int previous, int other)
{
    // TODO: This is bad. Does not take chance into account, and not that got the same twice, or the same as the other pattern
    int chances = 3;
    float f100 = (float)100;
    while (chances > 0)
    {
        int next = randint(getPatternCount());
        PatternModule *module = getPatternByIndex(next);

        float ourChance = randint(100) / f100;
        if (ourChance >= module->options->randomChance)
        {
            // That module had too low chance to be picked this time (or ever).
            continue;
        }

        if (next == previous || next == other)
        {
            // Give it another chance.
            chances--;
            continue;
        }

        return next;
    }
}

static void executor(uint16_t start, uint16_t stop, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    // TODO: Delegate to a random pattern
    // TODO: The best thing would be if we could intercept all the pixels, and blend different patterns into one
    struct data_struct *data = dataPtr;

    if (data->updatedAt == 0 || t > (data->updatedAt + data->period))
    {
        // It is time to move on to the next random mix.
        data->updatedAt = t;

        if (data->data1)
        {
            data->pattern1->destroyer(data->data1);
            data->data1 = NULL;
        }

        if (!data->patternIndex2 == -1)
        {
            // This is the first time. So assign pattern 2, which will be moved to 1 straight away.
            data->patternIndex2 = pattern_random_get_next_pattern_index(data->patternIndex2, data->patternIndex1);
        }

        data->patternIndex1 = data->patternIndex2;
        data->patternIndex2 = pattern_random_get_next_pattern_index(data->patternIndex2, data->patternIndex1);

        PatternModule *pattern2 = getPatternByIndex(data->patternIndex2);

        data->data1 = data->data2;
        data->data2 = pattern2->creator(len, data->intensity);

        data->pattern1 = data->pattern2;
        data->pattern2 = pattern2;

        // Ugly workaround so progress goes 0->1,1->0, and so on.
        // This way there should not be an instant JUMP when we switch to next pattern.
        // data->base.progressReversed = !data->base.progressReversed;
    }

    // Set the progress so we can calculate the proper crossover
    int age = t - data->updatedAt;

    void *cyclePtr1 = data->pattern1->cycleCreator(len, t, data->data1);
    void *cyclePtr2 = data->pattern2->cycleCreator(len, t, data->data2);

    for (int i = start; i < stop; i++)
    {
        // TODO: Could we change this so we go through all, and work more like it did before? Less overhead?
        data->base.stepIndex = 0;
        data->pattern1->executor(i, i + 1, len, t, data->data1, cyclePtr1, pattern_printer_merging);
        data->pattern2->executor(i, i + 1, len, t, data->data2, cyclePtr2, pattern_printer_merging);

        // Now let's send the data to the original printer
        HsiColor c = math_average_hsi(data->base.pixels, 3);
        printer(i, &c, dataPtr);
    }

    data->pattern1->cycleDestroyer(cyclePtr1);
    data->pattern2->cycleDestroyer(cyclePtr2);
}

void pattern_register_random()
{
    pattern_register("random", executor, data_creator, data_destroyer, NULL, NULL, &(PatternOptions){0});
}