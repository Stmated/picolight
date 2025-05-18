#include "../patterns.h"

typedef struct data_struct
{
    int chanceToLightUp;
    int chanceToGoOut;
    void *values;
    RgbwaColor colorOn;
    RgbwaColor colorOff;
} data_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;
    free(data->values);
    free(dataPtr);
}

// static const int v = 10000;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    data->chanceToLightUp = randint_weighted_towards_max(1, 10, intensity);
    data->chanceToGoOut = randint_weighted_towards_min(10, 100, intensity);
    data->values = calloc(len, sizeof(bool));
    data->colorOn = (RgbwaColor){CHANNEL_MAX, CHANNEL_MAX, CHANNEL_MAX, CHANNEL_MAX, RGB_ALPHA_MAX};
    data->colorOff = (RgbwaColor){0, 0, 0, 0, 0};

    return data;
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    bool *bools = (bool *)data->values;

    int ptrAddress = sizeof(bool) * args->i;
    int chance = randint(10000);
    if (chance < data->chanceToLightUp)
    {
        bools[ptrAddress] = true;
    }
    else if (chance < data->chanceToGoOut)
    {
        bools[ptrAddress] = false;
    }

    if (bools[ptrAddress])
    {
        return data->colorOn;
    }
    else
    {
        return data->colorOff;
    }
}

void pattern_register_sparkle()
{
    pattern_register("sparkle", executor, data_creator, data_destroyer, NULL, NULL, NULL, (PatternOptions){0.5});
}