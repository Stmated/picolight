#include "../patterns.h"
#include "../led_math.h"

typedef struct data_struct
{
    data_pixels_struct base;
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

    if (data->base.pixels)
    {
        free(data->base.pixels);
        data->base.pixels = NULL;
    }

    if (data->data1 && data->data1 != NULL)
    {
        data->pattern1->destroyer(data->data1);
        data->data1 = NULL;
    }

    if (data->data2 && data->data2 != NULL)
    {
        data->pattern2->destroyer(data->data2);
        data->data2 = NULL;
    }

    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->intensity = intensity;
    // instance->period = randint_probability(10000, 60000, intensity);
    data->period = randint_weighted_towards_min(5000, 6000, intensity);
    data->updatedAt = 0;
    data->patternIndex1 = -1; // 1 + randint(getPatternCount() - 2);
    data->patternIndex2 = -1, // 1 + randint(getPatternCount() - 2);
        // instance->data1 = getPattern(instance->patternIndex1).creator(len, instance->intensity);
        // instance->data2 = getPattern(instance->patternIndex2).creator(len, instance->intensity);
        // instance->data1_destroyer = getPattern(instance->patternIndex1).destroyer;
        // instance->data2_destroyer = getPattern(instance->patternIndex2).destroyer;

        // This is... a lot of data. Can we shrink it?
        data->base.pixels = calloc(len, sizeof(HsiColor)); // Allocate memory for each pixel, which we will need to blend

    return data;
}

// TODO: Currently pretty ugly that we do not get the HSI here. Double conversions and everything.
// TODO: Figure out if there is a good way of being able to send either RGB or HSI or both!
/*
static void pattern_printer_merging(uint16_t index, HsiColor *c, void *data)
{
    // Important to use the global data here, since "data" is from the sub-pattern
    struct data_struct *instance = state.patternData;

    if (instance->base->subsequent)
    {
        // Blend. FIX SO THIS CAN HANDLE HSI AND NOT NEED TO CONVERT BACK AND FORTH!

        float p = 1 - instance->base->progress;
        float invp = instance->base->progress;
        if (instance->base->progressReversed)
        {
            p = 1 - p;
            invp = 1 - invp;
        }

        // TODO: LerpHSI does not look good. It lerps the saturation too much, so it all becomes just a white blur
        // TODO: Figure out a good middle-ground between dividing by 2 and using LerpHSI. Maybe only ever LERP the hue and brightness?
        // TODO: Maybe add functionality where we have a random easing between the two patterns? Even more diversity == Good

        HsiColor lerped = LerpHSI(&instance->base->pixels[index], c, (instance->base->progressReversed ? p : 1 - p));
        lerped.s = MAX(instance->base->pixels[index].s, c->s); // Use the max saturation between the two. Is this up for randomization?

        instance->base->pixels[index] = lerped;

        // TODO: Maybe add functionality where we have a random easing between the two patterns? Even more diversity == Good

        //instance->pixels[index].r = ((instance->pixels[index].r * p) + (c->r * invp)) / 2;
        //instance->pixels[index].g = ((instance->pixels[index].g * p) + (c->g * invp)) / 2;
        //instance->pixels[index].b = ((instance->pixels[index].b * p) + (c->b * invp)) / 2;
        ///instance->pixels[index].w = ((instance->pixels[index].w * p) + (c->w * invp)) / 2;
        
    }
    else
    {
        // Just assign it
        instance->base->pixels[index] = *c;
    }
}
*/

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

static void executor(uint16_t offset, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
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
        data->base.progressReversed = !data->base.progressReversed;
    }

    // Set the progress so we can calculate the proper crossover
    int age = t - data->updatedAt;
    data->base.progress = age / (float)data->period;

    data->base.subsequent = false;

    void *cyclePtr1 = data->pattern1->cycleCreator(len, t, data->data1);
    data->pattern1->executor(offset, len, t, data->data1, cyclePtr1, pattern_printer_merging);
    data->pattern1->cycleDestroyer(cyclePtr1);

    data->base.subsequent = true;

    void *cyclePtr2 = data->pattern2->cycleCreator(len, t, data->data2);
    data->pattern2->executor(offset, len, t, data->data2, cyclePtr2, pattern_printer_merging);
    data->pattern2->cycleDestroyer(cyclePtr2);

    for (int i = offset; i < len; i++)
    {
        // Now let's send the data to the original printer
        printer(i, &data->base.pixels[i], dataPtr);
    }
}

void pattern_register_random()
{
    pattern_register("random", executor, data_creator, data_destroyer, &(PatternOptions){0});
}