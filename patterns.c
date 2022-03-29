#include "patterns.h"

HsiColor black = {0, 0, 0};
HsiColor white = {0, 0, 1};

void setAll(uint16_t offset, uint16_t len, HsiColor *c, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{

    // We sleep 1 ms, since this is usually too fast.
    sleep_ms(1);
}

void *pattern_creator_default(uint16_t len, float intensity)
{
    return NULL;
}

void pattern_destroyer_default(void *data)
{
    if (data)
    {
        free(data);
    }
}

void *pattern_cycle_creator_default(uint16_t len, uint32_t t, void *dataPtr)
{
    return NULL;
}

void pattern_cycle_destroyer_default(void *data)
{
    if (data)
    {
        free(data);
    }
}

int getPatternCount()
{
    return state.modules_size;
}

PatternModule *getPatternByIndex(int index)
{
    return &state.modules[index];
}

PatternModule *getPatternByName(const char *name)
{
    for (int i = 0; i < getPatternCount(); i++)
    {
        if (strcmp(state.modules[i].name, name) == 0)
        {
            return &state.modules[i];
        }
    }

    return NULL;
}

void pattern_find_and_register_patterns()
{
    pattern_register_snakes();

    pattern_register_random();

    pattern_register_rainbow_wave();
    pattern_register_rainbow();
    pattern_register_snake();
    pattern_register_fade_between();
    pattern_register_fill_sway();

    pattern_register_sparkle();
    pattern_register_strobe();
}

void pattern_register(
    const char *name, PatternExecutor executor,
    PatternDataCreator creator, PatternDataDestroyer destroyer,
    PatternCycleDataCreator cycleCreator, PatternCycleDataDestroyer cycleDestroyer,
    PatternOptions *options)
{
    PatternModule *array_new = calloc(state.modules_size + 1, sizeof(PatternModule));
    memcpy(array_new, state.modules, state.modules_size * sizeof(PatternModule));

    PatternModule module = {name, executor,
                            creator ? creator : pattern_creator_default,
                            destroyer ? destroyer : pattern_destroyer_default,
                            cycleCreator ? cycleCreator : pattern_cycle_creator_default,
                            cycleDestroyer ? cycleDestroyer : pattern_cycle_destroyer_default,
                            options};
    array_new[state.modules_size] = module;

    state.modules = array_new;
    state.modules_size++;
}

void pattern_printer_default(uint16_t index, HsiColor *c, void *dataPtr)
{
    // TODO: Create an EXTREMELY simple and fast caching of the last X colors. How? Hashing? Equals?
    //          Would probably speed things up generally, especially if we're using a filling or similar color next to each other
    RgbwColor rgbw;
    hsi2rgbw(c, &rgbw);
    put_pixel(index, &rgbw);
}

void pattern_printer_merging(uint16_t index, HsiColor *c, void *dataPtr)
{
    // Important to use the global data here, since "dataPtr" is from the sub-pattern
    data_pixel_blending_struct *data = state.patternData;

    data->pixels[sizeof(HsiColor) * data->stepIndex] = *c;
    data->stepIndex++;
}

void pattern_execute(uint16_t len, uint32_t t)
{
    if (state.nextPatternIndex >= 0)
    {
        // Destroy/free any previous memory allocations
        if (state.patternData)
        {
            getPatternByIndex(state.patternIndex)->destroyer(state.patternData);
            state.patternData = NULL;
        }

        // Create the new pattern data
        void *data = getPatternByIndex(state.nextPatternIndex)->creator(len, state.nextIntensity);

        // Set to the new (or same) pattern index, and new data
        state.patternIndex = state.nextPatternIndex;
        state.patternData = data;

        state.nextPatternIndex = -1;
        state.nextIntensity = -1;
    }

    // Execute the current pattern inside state
    if (!state.disabled)
    {
        PatternModule *module = getPatternByIndex(state.patternIndex);
        void *cyclePtr = module->cycleCreator(len, t, state.patternData);
        module->executor(0, len, len, t, state.patternData, cyclePtr, pattern_printer_default);
        module->cycleDestroyer(cyclePtr);
    }
    else
    {
        HsiColor black = {0, 0, 0};
        setAll(0, len, &black, NULL, NULL, pattern_printer_default);
    }
}
