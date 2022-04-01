#include "../patterns.h"

typedef struct data_struct
{
    void *data1;
    void *data2;
    PatternModule *pattern1;
    PatternModule *pattern2;
    int period;
    uint32_t updatedAt;
    float intensity;

} data_struct;

typedef struct frame_struct
{
    void *frame1;
    void *frame2;

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
    data->period = randint_weighted_towards_min(10000, 15000, intensity);
    data->updatedAt = 0;

    data->pattern1 = pattern_get_by_name("rainbow_wave");
    data->pattern2 = pattern_get_by_name("snake");
    data->data1 = data->pattern1->creator(len, intensity);
    data->data2 = data->pattern2->creator(len, intensity);

    // Allocate memory for each sub-pattern, which we will need to blend
    //data->base.pixels = calloc(2, sizeof(HsiaColor));

    return data;
}

static int pattern_random_get_next_pattern_index(int previous, int other)
{
    // TODO: Some patterns do not mix well. Those that cover 100% with 100% alpha. Do we care? Up to chance?
    int chances = 3;
    float f100 = (float)100;
    int next = -1;
    while (chances > 0)
    {
        next = randint(getPatternCount());
        PatternModule *module = pattern_get_by_index(next);

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

        break;
    }

    return next;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    /*
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

        printf("Going to '%s' and '%s'\n", data->pattern1->name, data->pattern2->name);

        // Ugly workaround so progress goes 0->1,1->0, and so on.
        // This way there should not be an instant JUMP when we switch to next pattern.
        // data->base.progressReversed = !data->base.progressReversed;
    }
    */

    // Set the progress so we can calculate the proper crossover
    //int age = t - data->updatedAt;

    frame->frame1 = data->pattern1->frameCreator(len, t, data->data1);
    frame->frame2 = data->pattern2->frameCreator(len, t, data->data2);

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

typedef struct RandomPrinter
{
    Printer base;
    int stepIndex;
    HsiaColor pixels[2];

} RandomPrinter;

static inline void random_printer(uint16_t index, HsiaColor *c, void *printerPtr)
{
    RandomPrinter *printer = printerPtr;
    printer->pixels[printer->stepIndex] = *c;
    printer->stepIndex++;
}

static void executor(uint16_t i, void *dataPtr, void *framePtr, Printer *printer)
{
    // TODO: Need to test each combination separately, and accurately, and make each one work EXACTLY as intended -- because something is seriously wrong... especially rainbow_wave + whatever.

    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    RandomPrinter randomPrinter = {*printer};
    Printer *downcast = (void*)&randomPrinter;

    data->pattern1->executor(i, data->data1, frame->frame1, downcast);
    data->pattern2->executor(i, data->data2, frame->frame2, downcast);

    // Now let's send the data to the original printer
    // TODO: The blending should be done differently! It should be done by a percentage! So we can smoothly transition between patterns!
    // TODO: Could this be sent to the parent printer directly somehow? So we do not need to average twice?
    // TODO: Can we skip sending along the parentDataPtr, and instead sent Printer as a semi-opaque struct that contains its own functionality?
    HsiaColor c = math_average_hsia(&randomPrinter.pixels[0], &randomPrinter.pixels[1]);
    printer->print(i, &c, printer); // Parent as ourself, since we are just a virtual pattern
}

void pattern_register_random()
{
    pattern_register("random", executor, data_creator, data_destroyer, frame_creator, frame_destroyer, (PatternOptions){0});
}