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

    // return state.modules[patternIndex];
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

void pattern_register(const char *name, PatternExecutor executor, PatternDataCreator creator, PatternDataDestroyer destroyer, PatternOptions *options)
{
    PatternModule *array_new = calloc(state.modules_size + 1, sizeof(PatternModule));
    memcpy(array_new, state.modules, state.modules_size * sizeof(PatternModule));

    PatternModule module = {name, executor, creator, destroyer, options};
    array_new[state.modules_size] = module;

    state.modules = array_new;
    state.modules_size++;
}

void pattern_printer_default(uint16_t index, HsiColor *c, void *dataPtr)
{
    RgbwColor rgbw;
    hsi2rgbw(c, &rgbw);
    put_pixel(index, &rgbw);
}

void pattern_printer_merging(uint16_t index, HsiColor *c, void *data)
{
    // Important to use the global data here, since "data" is from the sub-pattern
    data_pixels_struct *instance = state.patternData;

    float p = instance->progress;

    // TODO: LerpHSI does not look good. It lerps the saturation too much, so it all becomes just a white blur
    // TODO: Figure out a good middle-ground between dividing by 2 and using LerpHSI. Maybe only ever LERP the hue and brightness?
    // TODO: Maybe add functionality where we have a random easing between the two patterns? Even more diversity == Good

    if (instance->progress >= 1)
    {
        // Just a straight replace
        instance->pixels[index] = *c;
    }
    else
    {
        HsiColor existing = instance->pixels[index];
        float angles[2] = {existing.h, c->h};
        // TODO: Could this somehow be applied to ALL the previous hues? Is it possible? Bitmask multiple values in float?
        // TODO: Then at the end we need to apply an aggregation of the different values
        float averageHue = getAverageAngle(angles, 2);

        instance->pixels[index].h = (uint16_t)roundf(averageHue);
        instance->pixels[index].s = MAX(existing.s, c->s);
        instance->pixels[index].i = MAX(existing.i, c->i);
    }
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
        getPatternByIndex(state.patternIndex)->executor(len, t, state.patternData, pattern_printer_default);
    }
    else
    {
        HsiColor black = {0, 0, 0};
        setAll(len, &black, state.patternData, pattern_printer_default);
    }
}
