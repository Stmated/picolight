#include "../patterns.h"

typedef struct cycle_struct
{
    uint32_t remainder;
    HsiColor white;
    HsiColor black;
} cycle_struct;

static void *cycle_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    cycle_struct *cycle = calloc(1, sizeof(cycle_struct));

    cycle->remainder = t % 100;
    cycle->white = (HsiColor){0, 0, 1};
    cycle->black = (HsiColor){0, 0, 0};

    return cycle;
}

static inline void executor(uint16_t i, void *dataPtr, void *cyclePtr, void *parentDataPtr, PatternPrinter printer)
{
    cycle_struct *cycle = cyclePtr;

    if (cycle->remainder < 50)
    {
        printer(i, &cycle->white, dataPtr, parentDataPtr);
    }
    else
    {
        printer(i, &cycle->black, dataPtr, parentDataPtr);
    }
}

void pattern_register_strobe()
{
    pattern_register("strobe", executor, NULL, NULL, NULL, NULL, (PatternOptions){0.1});
}