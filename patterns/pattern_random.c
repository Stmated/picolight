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

typedef struct cycle_struct
{
    void *cycle1;
    void *cycle2;

} cycle_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;

    free(data->base.pixels);

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
    data->period = randint_weighted_towards_min(5000, 6000, intensity);
    data->updatedAt = 0;
    data->patternIndex1 = -1;
    data->patternIndex2 = -1;

    // Allocate memory for each sub-pattern, which we will need to blend
    data->base.pixels = calloc(2, sizeof(HsiColor));

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
        if (ourChance >= module->options.randomChance)
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

static void *cycle_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    cycle_struct *cycle = calloc(1, sizeof(cycle_struct));

    if (data->updatedAt == 0 || t > (data->updatedAt + data->period))
    {
        // It is time to move on to the next random mix.
        data->updatedAt = t;

        if (data->patternIndex2 == -1)
        {
            // This is the first time. So assign pattern 2, which will be moved to 1 straight away.
            data->patternIndex2 = pattern_random_get_next_pattern_index(data->patternIndex2, data->patternIndex1);
            data->pattern2 = getPatternByIndex(data->patternIndex2);
            data->data2 = data->pattern2->creator(len, data->intensity);
        }

        if (data->data1)
        {
            data->pattern1->destroyer(data->data1);
            data->data1 = NULL;
        }

        data->patternIndex1 = data->patternIndex2;
        data->data1 = data->data2;
        data->pattern1 = data->pattern2;

        data->patternIndex2 = pattern_random_get_next_pattern_index(data->patternIndex2, data->patternIndex1);
        data->pattern2 = getPatternByIndex(data->patternIndex2);
        data->data2 = data->pattern2->creator(len, data->intensity);

        // Ugly workaround so progress goes 0->1,1->0, and so on.
        // This way there should not be an instant JUMP when we switch to next pattern.
        // data->base.progressReversed = !data->base.progressReversed;
    }

    // Set the progress so we can calculate the proper crossover
    //int age = t - data->updatedAt;

    cycle->cycle1 = data->pattern1->cycleCreator(len, t, data->data1);
    cycle->cycle2 = data->pattern2->cycleCreator(len, t, data->data2);

    return cycle;
}

static void cycle_destroyer(void *dataPtr, void *cyclePtr)
{
    data_struct *data = dataPtr;
    cycle_struct *cycle = cyclePtr;

    data->pattern1->cycleDestroyer(data->data1, cycle->cycle1);
    data->pattern2->cycleDestroyer(data->data2, cycle->cycle2);
    
    free(cyclePtr);
}

static void executor(uint16_t i, void *dataPtr, void *cyclePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    cycle_struct *cycle = cyclePtr;

    data->base.stepIndex = 0;
    data->pattern1->executor(i, data->data1, cycle->cycle1, data, pattern_printer_set);
    data->pattern2->executor(i, data->data2, cycle->cycle2, data, pattern_printer_set);

    // Now let's send the data to the original printer
    // TODO: The blending should be done differently! It should be done by a percentage! So we can smoothly transition between patterns!
    // TODO: Could this be sent to the parent printer directly somehow? So we do not need to average twice?
    // TODO: Can we skip sending along the parentDataPtr, and instead sent Printer as a semi-opaque struct that contains its own functionality?
    HsiColor c = math_average_hsi(data->base.pixels, 2);
    printer(i, &c, dataPtr, dataPtr); // Parent as ourself, since we are just a virtual pattern
}

void pattern_register_random()
{
    pattern_register("random", executor, data_creator, data_destroyer, cycle_creator, cycle_destroyer, (PatternOptions){0});
}