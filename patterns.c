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

void *pattern_frame_allocator_default(uint16_t len, uint32_t t, void *dataPtr)
{
    return NULL;
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

static inline RgbwColor to_rgbw(uint16_t index, RgbwaColor *c)
{
    static RgbwColor black_rgbw = (RgbwColor){0, 0, 0, 0};

    if (c->a < 0.05)
    {
        return black_rgbw;
    }
    else
    {
        if (c->a < ALPHA_NEGLIGIBLE_MAX)
        {
            double a = (c->a / (double)RGB_ALPHA_MAX);
            return (RgbwColor){c->r * a, c->g * a, c->b * a, c->w * a};
        }

        // TODO: Skip the moving, just return the RGBWA instead. How?
        return (RgbwColor){c->r, c->g, c->b, c->w};
    }
}

static inline RgbwaColor pattern_default_executor(ExecutorArgs *args)
{
    // The default executor just takes the frame pointer and converts it into a color.
    // We expect the first field (at offset 0) to be a pixel color.
    // This code is dangerous, but who cares. It gives us ability to inline code easier... maybe.
    return *((RgbwaColor *)args->framePtr);
}

void pattern_find_and_register_patterns()
{
    pattern_register_test();

    pattern_register_random_sequence();
    pattern_register_random();

    // OK:
    pattern_register_hue_lerp();
    pattern_register_strobe();
    pattern_register_knightrider();
    pattern_register_snake();
    pattern_register_snakes();
    pattern_register_firework();
    pattern_register_fade_between();
    pattern_register_color_lerp();
    pattern_register_sparkle();
    pattern_register_meteor();

    // WEIRD:

    // SLOW:
    pattern_register_gas_fade();

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

void pattern_execute(uint16_t len, uint32_t t)
{
    static RgbwColor black_rgbw = (RgbwColor){0, 0, 0, 0};

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
        state.frameData = newModule->frameAllocator(len, state.nextIntensity, state.patternData);
    }

    // Execute the current pattern inside state
    if (!state.disabled)
    {
        PatternModule *module = pattern_get_by_index(state.patternIndex);

        // TODO: Do not allocate the frame every time, it should be kept in the state!
        //void *framePtr = module->frameAllocator(len, t, state.patternData);
        module->frameCreator(len, t, state.patternData, state.frameData);

        ExecutorArgs *args = &(ExecutorArgs){0, state.patternData, state.frameData};
        while (args->i < len)
        {
            RgbwaColor rgbwa = module->executor(args);
            RgbwColor rgbw = to_rgbw(args->i, &rgbwa);
            put_pixel(args->i, &rgbw);
            args->i++;
        }

        // TODO: Do not destroy the frame every time, it should be kept in the state!
        //module->frameDestroyer(state.patternData, framePtr);
    }
    else
    {
        for (uint_fast16_t i = 0; i < len; i++)
        {
            put_pixel(i, &black_rgbw);
        }
    }
}
