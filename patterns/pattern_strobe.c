#include "../patterns.h"

static void executor(uint16_t start, uint16_t stop, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, void *parentDataPtr, PatternPrinter printer)
{
    uint32_t remainder = t % 100;
    if (remainder < 50)
    {
        HsiColor white = {0, 0, 1};
        for (int i = start; i < stop; i++)
        {
            printer(i, &white, dataPtr, parentDataPtr);
        }
    }
    else
    {
        HsiColor black = {0, 0, 0};
        for (int i = start; i < stop; i++)
        {
            printer(i, &black, dataPtr, parentDataPtr);
        }
    }
}

void pattern_register_strobe()
{
    pattern_register("strobe", executor, NULL, NULL, NULL, NULL, (PatternOptions){0.1});
}