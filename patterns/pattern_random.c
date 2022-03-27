#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_random_data_struct
{
    void *data1;
    void *data2;
    pattern_data_destroyer data1_destroyer;
    pattern_data_destroyer data2_destroyer;
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

void pattern_random_data_destroyer(void *data)
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

void *pattern_random_data(uint16_t len, float intensity)
{
    pattern_random_data_struct *instance = calloc(1, sizeof(pattern_random_data_struct));

    instance->intensity = intensity;
    instance->period = randint_probability(10000, 60000, intensity);
    instance->updatedAt = 0;
    instance->patternIndex1 = 1 + randint(getPatternCount() - 2);
    instance->patternIndex2 = 1 + randint(getPatternCount() - 2);
    instance->data1 = getPattern(instance->patternIndex1).creator(len, instance->intensity);
    instance->data2 = getPattern(instance->patternIndex2).creator(len, instance->intensity);
    instance->data1_destroyer = getPattern(instance->patternIndex1).destroyer;
    instance->data2_destroyer = getPattern(instance->patternIndex2).destroyer;

    // This is... a lot of data. Can we shrink it?
    instance->pixels = calloc(len, sizeof(HsiColor)); // Allocate memory for each pixel, which we will need to blend

    return instance;
}

// TODO: Currently pretty ugly that we do not get the HSI here. Double conversions and everything.
// TODO: Figure out if there is a good way of being able to send either RGB or HSI or both!
void pattern_random_data_printer(uint16_t index, HsiColor *c, void *data)
{
    // Important to use the global data here, since "data" is from the sub-pattern
    struct pattern_random_data_struct *instance = state.patternData;

    // TODO: Use the HsiLerp here instead! Might be a bit more accurate/smooth!

    // qweqwe

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

        instance->pixels[index] = LerpHSI(&instance->pixels[index], c, (instance->progressReversed ? p : 1 - p));

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
        /*
        instance->pixels[index].g = c->g;
        instance->pixels[index].b = c->b;
        instance->pixels[index].w = c->w;
        */
    }
}

void pattern_random(uint16_t len, uint32_t t, void *data, printer printer)
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

        instance->patternIndex1 = instance->patternIndex2;
        instance->patternIndex2 = 1 + randint(getPatternCount() - 2);
        if (instance->patternIndex1 == instance->patternIndex2)
        {
            // Give one more chance to be different
            instance->patternIndex2 = 1 + randint(getPatternCount() - 2);
        }

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
    getPattern(instance->patternIndex1).pat(len, t, instance->data1, pattern_random_data_printer);
    instance->subsequent = true;
    getPattern(instance->patternIndex2).pat(len, t, instance->data2, pattern_random_data_printer);

    for (int i = 0; i < len; i++)
    {
        // Now let's send the data to the original printer
        printer(i, &instance->pixels[i], data);
    }
}

void pattern_register_random()
{
    pattern_register(pattern_random, pattern_random_data, pattern_random_data_destroyer);
}