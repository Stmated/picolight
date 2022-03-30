#include "../patterns.h"

typedef struct data_struct
{
    int chanceToLightUp;
    int chanceToGoOut;
    void *values;
    HsiColor colorOn;
    HsiColor colorOff;
} data_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;
    free(data->values);
    free(dataPtr);
}

static const int v = 1000000;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    data->chanceToLightUp = v * (randint_weighted_towards_max(10, 10000000, intensity) / (float)1000000000);
    data->chanceToGoOut = v * (randint_weighted_towards_max(1000, 5000, intensity) / (float)100000);
    data->values = calloc(len, sizeof(bool));
    data->colorOn = (HsiColor) {0, 0, 1};
    data->colorOff = (HsiColor) {0, 0, 0};

    return data;
}

static inline void executor(uint16_t i, void *dataPtr, void *cyclePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    bool *bools = (bool *)data->values;

    int ptrAddress = sizeof(bool) * i;
    if (randint(v) < data->chanceToLightUp)
    {
        bools[ptrAddress] = true;
    }
    else if (randint(v) < data->chanceToGoOut)
    {
        bools[ptrAddress] = false;
    }

    HsiColor hsi = (bools[ptrAddress] ? data->colorOn : data->colorOff);
    printer(i, &hsi, dataPtr, parentDataPtr);
}

void pattern_register_sparkle()
{
    pattern_register("sparkle", executor, data_creator, data_destroyer, NULL, NULL, (PatternOptions){1});
}