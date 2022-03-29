#include "../patterns.h"

void pattern_strobe(uint16_t offset, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    uint32_t remainder = t % 100;
    if (remainder < 50)
    {
        HsiColor white = {0, 0, 1};
        for (int i = offset; i < len; i++)
        {
            printer(i, &white, dataPtr);
        }
    }
    else
    {
        HsiColor black = {0, 0, 0};
        for (int i = offset; i < len; i++)
        {
            printer(i, &black, dataPtr);
        }
    }
}

void pattern_register_strobe()
{
    pattern_register("strobe", pattern_strobe,
                     pattern_creator_default, pattern_destroyer_default,
                     pattern_cycle_creator_default, pattern_cycle_destroyer_default,
                     &(PatternOptions){0.25});
}