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

void *pattern_frame_allocator_default(uint16_t len, void *dataPtr)
{
    return NULL;
}

void pattern_frame_creator_default(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
    
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

RgbwaColor RGBWA_BLACK = (RgbwaColor){0, 0, 0, 0, 1};
RgbwaColor RGBWA_TRANSPARENT = (RgbwaColor){0, 0, 0, 0, 0};

static inline RgbwaColor pattern_default_executor(ExecutorArgs *args)
{
    // The default executor just takes the frame pointer and converts it into a color.
    // We expect the first field (at offset 0) to be a pixel color.
    // This code is dangerous, but who cares. It gives us ability to inline code easier... maybe.
    return *((RgbwaColor *)args->framePtr);
}

void pattern_find_and_register_patterns()
{
    //pattern_register_random_sequence();
    pattern_register_random();

    // OK:
    pattern_register_hue_lerp();
    pattern_register_knightrider();
    pattern_register_color_lerp();
    pattern_register_rainbow_splash();
    pattern_register_rainbow_wave();
    pattern_register_snake();
    pattern_register_snakes();
    pattern_register_firework();
    pattern_register_fade_between();
    pattern_register_strobe();
    pattern_register_sparkle();
    pattern_register_meteor();

    //pattern_register_test();

    // TODO:
    // rainbow wave
    // rainbow pulse
    // -- copy ideas from the Moonlander, add as many as possible :)

    // WEIRD:

    // SLOW:
    //pattern_register_gas_fade();

    // Hardfault???
    // pattern_register_eyes();
}

void pattern_register(
    const char *name, PatternExecutor executor,
    PatternDataCreator creator, PatternDataDestroyer destroyer,
    PatternFrameDataAllocator frameAllocator, PatternFrameDataCreator frameCreator, PatternFrameDataDestroyer frameDestroyer,
    PatternOptions options)
{
    
    PatternModule *array_new = calloc(state.modules_size + 1, sizeof(PatternModule));
    memcpy(array_new, state.modules, state.modules_size * sizeof(PatternModule));

    if (state.modules != NULL) {

        // Free the previous modules array, or we'd leak.
        free(state.modules);
    }

    PatternModule module = {name,
                            executor ? executor : pattern_default_executor,
                            creator ? creator : pattern_creator_default,
                            destroyer ? destroyer : pattern_destroyer_default,
                            frameAllocator ? frameAllocator : pattern_frame_allocator_default,
                            frameCreator ? frameCreator : pattern_frame_creator_default,
                            frameDestroyer ? frameDestroyer : pattern_frame_destroyer_default,
                            options};
    array_new[state.modules_size] = module;

    state.modules = array_new;
    state.modules_size++;
}

void pattern_execute(uint16_t len, uint64_t t_us)
{
    static uint32_t previous_t_us = -1;
    uint32_t delta_t_us = (previous_t_us == -1) ? 0 : (t_us - previous_t_us);

    uint32_t t_ms = t_us / 1000;

    if (state.nextPatternIndex >= 0)
    {
        // Destroy/free any previous memory allocations
        if (state.frameData)
        {
            pattern_get_by_index(state.patternIndex)->frameDestroyer(state.patternData, state.frameData);
            state.frameData = NULL;
        }
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

        // We allocate the memory for the frame data right away, so we do not need to allocate/deallocate over and over.
        state.frameData = newModule->frameAllocator(len, state.patternData);
    }

    // Execute the current pattern inside state
    if (!state.disabled)
    {
        PatternModule *module = pattern_get_by_index(state.patternIndex);

        module->frameCreator(len, t_ms, state.patternData, state.frameData);

        ExecutorArgs *args = &(ExecutorArgs){0, state.patternData, state.frameData};
        while (args->i < len)
        {
            RgbwaColor rgbwa = module->executor(args);
            put_pixel(args->i, len, t_us, delta_t_us, &rgbwa);
            args->i++;
        }
    }
    else
    {
        for (uint16_t i = 0; i < len; i++)
        {
            put_pixel(i, len, t_us, delta_t_us, &RGBWA_BLACK);
        }
    }

    previous_t_us = t_us;
}
