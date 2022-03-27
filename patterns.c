#include "patterns.h"

void setAll(uint16_t len, HsiColor *c, void *data, printer printer)
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



/*
pattern_table[] = {

    {pattern_random, pattern_random_data, pattern_random_data_destroyer},

    {pattern_red_bouncer_smooth, pattern_red_bouncer_smooth_data, default_destroyer},

    {pattern_sparkle, pattern_sparkle_data, pattern_sparkle_data_destroyer},

    {pattern_snakes, pattern_snakes_data, default_destroyer},

    {pattern_fade_between, pattern_fade_between_data, default_destroyer},

    {pattern_fill_sway, pattern_fill_sway_data, default_destroyer},
    {pattern_rainbow_wave, pattern_rainbow_wave_data, default_destroyer},

    {pattern_strobe, noop, default_destroyer},
};
*/

// TODO: Fix pattern factory by reading https://github.com/huawenyu/Design-Patterns-in-C/tree/master/auto-gen/factory/simple_factory

int getPatternCount()
{
    return state.modules_size;
}

pattern_module getPattern(int patternIndex)
{
    return state.modules[patternIndex];
}

void findAndRegisterPatterns()
{
    pattern_register_bouncer();
    pattern_register_fade_between();
    pattern_register_fill_sway();
    pattern_register_rainbow_wave();
    pattern_register_snakes();
    pattern_register_sparkle();
    pattern_register_strobe();

    pattern_register_random();
}

void registerPattern(pattern pattern, pattern_data_creator creator, pattern_data_destroyer destroyer)
{
    // Do magic here. Add to the table, and expand it if needed

    //int first_array[10] = {45, 2, 48, 3, 6};
    //int scnd_array[] = {8, 14, 69, 23, 5};

    // 5 is the number of the elements which are going to be appended
    pattern_module *new = calloc(state.modules_size + 1, sizeof(pattern_module));
    memcpy(new, state.modules, state.modules_size * sizeof(pattern_module));
    state.modules = new;
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
        getPattern(state.patternIndex).pat(len, t, state.patternData, pattern_put_pixel_default);
    }
    else
    {
        HsiColor black = {0, 0, 0};
        setAll(len, &black, state.patternData, pattern_put_pixel_default);
    }
}
