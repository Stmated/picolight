#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_random_data_struct
{
    void *data1;
    void *data2;
    PatternDataDestroyer data1_destroyer;
    PatternDataDestroyer data2_destroyer;
    int period;
    uint32_t updatedAt;
    float intensity;
    int patternIndex1;
    int patternIndex2;
    HsiColor *pixels;
    float progress;
    bool progressReversed;
    bool subsequent;
} pattern_random_data_struct;

static void pattern_random_data_destroyer(void *data)
{
    pattern_random_data_struct *instance = data;

    if (instance->pixels)
    {
        free(instance->pixels);
        instance->pixels = NULL;
    }

    if (instance->data1_destroyer && instance->data1 && instance->data1 != NULL)
    {
        instance->data1_destroyer(instance->data1);
        instance->data1 = NULL;
    }

    if (instance->data2_destroyer && instance->data2 && instance->data2 != NULL)
    {
        instance->data2_destroyer(instance->data2);
        instance->data2 = NULL;
    }

    free(data);
}

static void *pattern_random_data(uint16_t len, float intensity)
{
    pattern_random_data_struct *instance = calloc(1, sizeof(pattern_random_data_struct));

    instance->intensity = intensity;
    //instance->period = randint_probability(10000, 60000, intensity);
    instance->period = randint_weighted_towards_min(5000, 6000, intensity);
    instance->updatedAt = 0;
    instance->patternIndex1 = -1; // 1 + randint(getPatternCount() - 2);
    instance->patternIndex2 = -1, // 1 + randint(getPatternCount() - 2);
    //instance->data1 = getPattern(instance->patternIndex1).creator(len, instance->intensity);
    //instance->data2 = getPattern(instance->patternIndex2).creator(len, instance->intensity);
    //instance->data1_destroyer = getPattern(instance->patternIndex1).destroyer;
    //instance->data2_destroyer = getPattern(instance->patternIndex2).destroyer;

    // This is... a lot of data. Can we shrink it?
    instance->pixels = calloc(len, sizeof(HsiColor)); // Allocate memory for each pixel, which we will need to blend

    return instance;
}

// TODO: Currently pretty ugly that we do not get the HSI here. Double conversions and everything.
// TODO: Figure out if there is a good way of being able to send either RGB or HSI or both!
static void pattern_random_data_printer(uint16_t index, HsiColor *c, void *data)
{
    // Important to use the global data here, since "data" is from the sub-pattern
    struct pattern_random_data_struct *instance = state.patternData;

    if (instance->subsequent)
    {
        // Blend. FIX SO THIS CAN HANDLE HSI AND NOT NEED TO CONVERT BACK AND FORTH!

        float p = 1 - instance->progress;
        float invp = instance->progress;
        if (instance->progressReversed)
        {
            p = 1 - p;
            invp = 1 - invp;
        }

        // TODO: LerpHSI does not look good. It lerps the saturation too much, so it all becomes just a white blur
        // TODO: Figure out a good middle-ground between dividing by 2 and using LerpHSI. Maybe only ever LERP the hue and brightness?
        // TODO: Maybe add functionality where we have a random easing between the two patterns? Even more diversity == Good

        HsiColor lerped = LerpHSI(&instance->pixels[index], c, (instance->progressReversed ? p : 1 - p));
        lerped.s = MAX(instance->pixels[index].s, c->s); // Use the max saturation between the two. Is this up for randomization?

        instance->pixels[index] = lerped;

        // TODO: Maybe add functionality where we have a random easing between the two patterns? Even more diversity == Good

        /*
        instance->pixels[index].r = ((instance->pixels[index].r * p) + (c->r * invp)) / 2;
        instance->pixels[index].g = ((instance->pixels[index].g * p) + (c->g * invp)) / 2;
        instance->pixels[index].b = ((instance->pixels[index].b * p) + (c->b * invp)) / 2;
        instance->pixels[index].w = ((instance->pixels[index].w * p) + (c->w * invp)) / 2;
        */

    }
    else
    {
        // Just assign it
        instance->pixels[index] = *c;
    }
}


static int getNextPatternIndex(int previous, int other)
{
    // TODO: This is bad. Does not take chance into account, and not that got the same twice, or the same as the other pattern
    int chances = 3;
    float f100 = (float) 100;
    while (chances > 0)
    {
        int next = randint(getPatternCount());
        PatternModule module = getPattern(next);

        float ourChance = randint(100) / f100;
        if (ourChance >= module.options.randomChance)
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

static void pattern_random(uint16_t len, uint32_t t, void *data, PatternPrinter printer)
{
    // TODO: Delegate to a random pattern
    // TODO: The best thing would be if we could intercept all the pixels, and blend different patterns into one
    struct pattern_random_data_struct *instance = data;

    if (instance->updatedAt == 0 || t > (instance->updatedAt + instance->period))
    {
        // It is time to move on to the next random mix.
        instance->updatedAt = t;

        if (instance->data1)
        {
            instance->data1_destroyer(instance->data1);
            instance->data1 = NULL;
        }

        if (!instance->patternIndex2 == -1)
        {
            // This is the first time. So assign pattern 2, which will be moved to 1 straight away.
            instance->patternIndex2 = getNextPatternIndex(instance->patternIndex2, instance->patternIndex1);
        }

        instance->patternIndex1 = instance->patternIndex2;
        instance->patternIndex2 = getNextPatternIndex(instance->patternIndex2, instance->patternIndex1); // 1 + randint(getPatternCount() - 2);

        instance->data1 = instance->data2;
        instance->data2 = getPattern(instance->patternIndex2).creator(len, instance->intensity);

        instance->data1_destroyer = instance->data2_destroyer;
        instance->data2_destroyer = getPattern(instance->patternIndex2).destroyer;

        // Ugly workaround so progress goes 0->1,1->0, and so on.
        // This way there should not be an instant JUMP when we switch to next pattern.
        instance->progressReversed = !instance->progressReversed;
    }

    // Set the progress so we can calculate the proper crossover
    int age = t - instance->updatedAt;
    instance->progress = age / (float)instance->period;

    instance->subsequent = false;
    getPattern(instance->patternIndex1).executor(len, t, instance->data1, pattern_random_data_printer);
    instance->subsequent = true;
    getPattern(instance->patternIndex2).executor(len, t, instance->data2, pattern_random_data_printer);

    for (int i = 0; i < len; i++)
    {
        // Now let's send the data to the original printer
        printer(i, &instance->pixels[i], data);
    }
}

void pattern_register_random()
{
    pattern_register(pattern_random, pattern_random_data, pattern_random_data_destroyer, &(PatternOptions){0});
}