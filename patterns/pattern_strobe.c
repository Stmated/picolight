#include "../patterns.h"

int a = 10;

void pattern_strobe(uint16_t len, uint32_t t, void *data, PatternPrinter printer)
{
    uint32_t remainder = t % 100;
    if (remainder < 50)
    {
        HsiColor white = {0, 0, 1};
        for (int i = 0; i < len; i++)
        {
            printer(i, &white, data);
        }
    }
    else
    {
        HsiColor black = {0, 0, 0};
        for (int i = 0; i < len; i++)
        {
            printer(i, &black, data);
        }
    }
}

void pattern_register_strobe()
{
    pattern_register(pattern_strobe, pattern_creator_default, pattern_destroyer_default, &(PatternOptions){0.25});
}