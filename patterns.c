#include "patterns.h"

void setAll(uint16_t len, HsiColor *c, void *data, PatternPrinter printer)
{
    for (uint16_t i = 0; i < len; i++)
    {
        printer(i, c, data);
    }

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

int getPatternCount()
{
    return state.modules_size;
}

PatternModule getPattern(int patternIndex)
{
    return state.modules[patternIndex];
}

void pattern_find_and_register_patterns()
{
    pattern_register_rainbow_wave();

    pattern_register_rainbow();

    pattern_register_random();

    pattern_register_bouncer(); // Bad name
    pattern_register_fade_between();
    pattern_register_fill_sway();
    
    pattern_register_snakes();
    pattern_register_sparkle();
    pattern_register_strobe();
}

void pattern_register(PatternExecutor pattern, PatternDataCreator creator, PatternDataDestroyer destroyer, PatternOptions *options)
{
    PatternModule *array_new = calloc(state.modules_size + 1, sizeof(PatternModule));
    memcpy(array_new, state.modules, state.modules_size * sizeof(PatternModule));

    PatternModule module = {pattern, creator, destroyer};
    array_new[state.modules_size] = module;

    state.modules = array_new;
    state.modules_size++;
}

void pattern_put_pixel_default(uint16_t index, HsiColor *hsi, void *data)
{
    RgbwColor rgbw;
    hsi2rgbw(hsi, &rgbw);
    put_pixel(index, &rgbw);
}

void pattern_execute(uint16_t len, uint32_t t)
{
    if (state.nextPatternIndex >= 0)
    {
        // Destroy/free any previous memory allocations
        if (state.patternData)
        {
            getPattern(state.patternIndex).destroyer(state.patternData);
            state.patternData = NULL;
        }

        // pattern_update_data(len, state.nextPatternIndex, state.nextIntensity);

        // Create the new pattern data
        void *data = getPattern(state.nextPatternIndex).creator(len, state.nextIntensity);
        // pattern_table[state.nextPatternIndex].creator(len, state.nextIntensity);

        // Set to the new (or same) pattern index, and new data
        state.patternIndex = state.nextPatternIndex;
        state.patternData = data;

        state.nextPatternIndex = -1;
        state.nextIntensity = -1;
    }

    // Execute the current pattern inside state
    if (!state.disabled)
    {
        getPattern(state.patternIndex).executor(len, t, state.patternData, pattern_put_pixel_default);
    }
    else
    {
        HsiColor black = {0, 0, 0};
        setAll(len, &black, state.patternData, pattern_put_pixel_default);
    }
}
