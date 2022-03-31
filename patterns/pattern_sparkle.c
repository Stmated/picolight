#include "../patterns.h"

typedef struct data_struct
{
    int chanceToLightUp;
    int chanceToGoOut;
    void *values;
    HsiaColor colorOn;
    HsiaColor colorOff;
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
    data->colorOn = (HsiaColor){0, 0, 1, 1};
    data->colorOff = (HsiaColor){0, 0, 0, 0};

    return data;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    bool *bools = (bool *)data->values;

    int ptrAddress = sizeof(bool) * i;
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
        printer(i, &data->colorOn, dataPtr, parentDataPtr);
    }
    else
    {
        printer(i, &data->colorOff, dataPtr, parentDataPtr);
    }
}

void pattern_register_sparkle()
{
    pattern_register("sparkle", executor, data_creator, data_destroyer, NULL, NULL, (PatternOptions){1});
}