#include "patterns.h"

void *pattern_creator_default(uint16_t len, float intensity)
{
    return NULL;
}

void pattern_destroyer_default(void *dataPtr)
{
    if (dataPtr)
    {
        free(dataPtr);
    }
}

void *pattern_frame_creator_default(uint16_t len, uint32_t t, void *dataPtr)
{
    return NULL;
}

void pattern_frame_destroyer_default(void *dataPtr, void *framePtr)
{
    if (framePtr)
    {
        free(framePtr);
    }
}

int getPatternCount()
{
    return state.modules_size;
}

PatternModule *pattern_get_by_index(int index)
{
    return &state.modules[index];
}

PatternModule *pattern_get_by_name(const char *name)
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

static inline void pattern_printer_default(uint16_t index, HsiaColor *c)
{
    static HsiaColor black = (HsiaColor){0, 0, 0, 1};

    // TODO: Create an EXTREMELY simple and fast caching of the last X colors. How? Hashing? Equals? Just previous [1-3] pixels?
    //          Would probably speed things up generally, especially if we're using a filling or similar color next to each other
    if (c->a < 1)
    {
        HsiaColor blended = math_average_hsia(&black, c);
        RgbwColor rgbw = hsia2rgbw(&blended);
        put_pixel(index, &rgbw);
    }
    else
    {
        RgbwColor rgbw = hsia2rgbw(c);
        put_pixel(index, &rgbw);
    }
}

void pattern_find_and_register_patterns()
{
    pattern_register_random_sequence();

    pattern_register_eyes();

    pattern_register_random();
    pattern_register_strobe();
    pattern_register_knightrider();
    pattern_register_snakes();
    pattern_register_gas_fade();
    pattern_register_firework();
    pattern_register_fade_between();
    pattern_register_snake();
    pattern_register_color_lerp();
    pattern_register_sparkle();
    pattern_register_hue_lerp();
    pattern_register_meteor();

    pattern_register_test();
}

void pattern_register(
    const char *name, PatternExecutor executor,
    PatternDataCreator creator, PatternDataDestroyer destroyer,
    PatternFrameDataCreator frameCreator, PatternFrameDataDestroyer frameDestroyer,
    PatternOptions options)
{
    PatternModule *array_new = calloc(state.modules_size + 1, sizeof(PatternModule));
    memcpy(array_new, state.modules, state.modules_size * sizeof(PatternModule));

    PatternModule module = {name,
                            executor,
                            creator ? creator : pattern_creator_default,
                            destroyer ? destroyer : pattern_destroyer_default,
                            frameCreator ? frameCreator : pattern_frame_creator_default,
                            frameDestroyer ? frameDestroyer : pattern_frame_destroyer_default,
                            options};
    array_new[state.modules_size] = module;

    state.modules = array_new;
    state.modules_size++;
}

void pattern_execute(uint16_t len, uint32_t t)
{
    if (state.nextPatternIndex >= 0)
    {
        // Destroy/free any previous memory allocations
        if (state.patternData)
        {
            pattern_get_by_index(state.patternIndex)->destroyer(state.patternData);
            state.patternData = NULL;
        }

        // Create the new pattern data
        PatternModule *newModule = pattern_get_by_index(state.nextPatternIndex);

        void *data = newModule->creator(len, state.nextIntensity);

        // Set to the new (or same) pattern index, and new data
        state.patternIndex = state.nextPatternIndex;
        state.patternData = data;

        state.nextPatternIndex = -1;
        state.nextIntensity = -1;
    }

    // Execute the current pattern inside state
    if (!state.disabled)
    {
        PatternModule *module = pattern_get_by_index(state.patternIndex);
        void *framePtr = module->frameCreator(len, t, state.patternData);
        if (module->executor == NULL)
        {
            for (uint_fast16_t i = 0; i < len; i++)
            {
                // The default executor just takes the frame pointer and converts it into a color.
                // We expect the first field (at offset 0) to be a pixel color.
                // This code is dangerous, but who cares. It gives us ability to inline code easier.
                pattern_printer_default(i, framePtr);
            }
        }
        else
        {
            ExecutorArgs *args = &(ExecutorArgs){0, state.patternData, framePtr};
            while (args->i < len)
            {
                HsiaColor c = module->executor(args);
                pattern_printer_default(args->i, &c);
                args->i++;
            }
        }
        module->frameDestroyer(state.patternData, framePtr);
    }
    else
    {
        for (uint_fast16_t i = 0; i < len; i++)
        {
            pattern_printer_default(i, &COLOR_BLACK);
        }
    }
}
