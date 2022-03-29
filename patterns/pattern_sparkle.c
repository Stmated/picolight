#include "../patterns.h"

typedef struct data_struct
{
    int chanceToLightUp;
    int chanceToGoOut;
    void *values;
} data_struct;

static void data_destroyer(void *dataPtr)
{
    if (dataPtr)
    {
        data_struct *data = dataPtr;
        free(data->values);
        free(data);
    }
}

static const int v = 1000000;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    data->chanceToLightUp = v * (randint_weighted_towards_max(10, 10000000, intensity) / (float)10000000000);
    data->chanceToGoOut = v * (randint_weighted_towards_max(1000, 5000, intensity) / (float)100000);
    data->values = calloc(len, sizeof(bool));

    return data;
}

static void executor(uint16_t start, uint16_t stop, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    data_struct *data = dataPtr;
    bool *bools = (bool *)data->values;

    for (int i = start; i < stop; i++)
    {
        int ptrAddress = sizeof(bool) * i;
        if (randint(v) < data->chanceToLightUp)
        {
            bools[ptrAddress] = true;
        }
        else if (randint(v) < data->chanceToGoOut)
        {
            bools[ptrAddress] = false;
        }

        HsiColor hsi = {0, 0, bools[ptrAddress] ? 1 : 0};
        printer(i, &hsi, dataPtr);
    }
}

void pattern_register_sparkle()
{
    pattern_register("sparkle", executor,
                     data_creator, data_destroyer,
                     NULL, NULL,
                     &(PatternOptions){1});
}