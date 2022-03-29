#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_sparkle_data_struct
{
    float chanceToLightUp;
    float chanceToGoOut;
    void *values;
} pattern_sparkle_data_struct;

void pattern_sparkle_data_destroyer(void *data)
{
    if (data)
    {
        pattern_sparkle_data_struct *instance = data;
        if (instance->values)
        {
            free(instance->values);
            instance->values = 0;
        }

        free(data);
    }
}

void *pattern_sparkle_data(uint16_t len, float intensity)
{
    pattern_sparkle_data_struct *instance = malloc(sizeof(pattern_sparkle_data_struct));
    bool *bools = calloc(len, sizeof(bool));
    if (bools == NULL)
    {
        int i = 0;
    }

    instance->chanceToLightUp = randint_weighted_towards_max(10, 10000000, intensity) / (float)10000000000;
    instance->chanceToGoOut = randint_weighted_towards_max(1000, 5000, intensity) / (float)100000;
    instance->values = bools;

    return instance;
}

void pattern_sparkle(uint16_t offset, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    pattern_sparkle_data_struct *data = dataPtr;

    const int v = 1000000;
    int chanceToLightUpInt = v * data->chanceToLightUp;
    int chanceToGoOutInt = v * data->chanceToGoOut;
    bool *bools = (bool *)data->values;

    for (int i = offset; i < len; i++)
    {
        int ptrAddress = sizeof(bool) * i;
        if (randint(v) < chanceToLightUpInt)
        {
            bools[ptrAddress] = true;
        }
        else if (randint(v) < chanceToGoOutInt)
        {
            bools[ptrAddress] = false;
        }

        HsiColor hsi = {0, 0, bools[ptrAddress] ? 1 : 0};
        printer(i, &hsi, dataPtr);
    }
}

void pattern_register_sparkle()
{
    pattern_register("sparkle", pattern_sparkle, pattern_sparkle_data, pattern_sparkle_data_destroyer, &(PatternOptions){1});
}