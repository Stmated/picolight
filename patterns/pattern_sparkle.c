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

    instance->chanceToLightUp = randint_probability(10, 10000000, 1 - intensity) / (float)1000000000;
    instance->chanceToGoOut = randint_probability(1000, 5000, 1 - intensity) / (float)100000;
    instance->values = bools;

    return instance;
}

void pattern_sparkle(uint16_t len, uint32_t t, void *data, printer printer)
{
    pattern_sparkle_data_struct *instance = data;

    const int v = 1000000;
    int chanceToLightUpInt = v * instance->chanceToLightUp;
    int chanceToGoOutInt = v * instance->chanceToGoOut;
    bool *bools = (bool *)instance->values;

    for (int i = 0; i < len; i++)
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
        printer(i, &hsi, data);
    }
}

void pattern_register_sparkle()
{
    pattern_register(pattern_sparkle, pattern_sparkle_data, pattern_sparkle_data_destroyer);
}