#include "actions.h"
#include "patterns.h"

void button_increment_pattern_index(t_state *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    if (isDown && sinceLastDown > 250)
    {
        state->patternIndex = (state->patternIndex + 1) % getPatternCount();
    }
}

void button_decrement_pattern_index(t_state *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    if (isDown && sinceLastDown > 250)
    {
        if (state->patternIndex == 0)
        {
            state->patternIndex = getPatternCount() - 1;
        }
        else
        {
            state->patternIndex = (state->patternIndex - 1);
        }
    }
}

void button_decrement_speed(t_state *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    /*
    if (isDown && sinceLastDown > 250)
    {
        state->speed = (state->speed * 0.8);
    }
    */
}

void button_increment_speed(t_state *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    /*
    if (isDown && sinceLastDown > 250)
    {
        state->speed = (state->speed * 1.2);
    }
    */
}

void button_toggle_offset(t_state *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    if (isDown && sinceLastDown > 250)
    {
        state->withOffset = !state->withOffset;
    }
}

void button_toggle_strobe(t_state *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    /*
    if (isDown && sinceLastDown > 250)
    {
        if (state->tempPatternIndex == -1)
        {
            state->tempPatternIndex = getPatternCount() - 1;
        }
        else
        {
            state->tempPatternIndex = -1;
        }
    }
    */
}

float getRandomChance(float min, float max, float strength)
{
    // TODO: Need to improve!
    float r = (rand() / (float)RAND_MAX);
    return min + ((max * strength) * r);
}

void button_intensity(t_state *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    if (isDown)
    {
        if (sinceLastDown > 2000)
        {
            // Reset the intensity before starting to count it back up again.
            state->clicking = true;
            state->clickCount = 1;
        }
        else
        {
            if (sinceLastUp < sinceLastDown)
            {
                // The button has been up since it now went down again.
                state->clickCount++;
            }
        }
    }
    else
    {
        if (state->clicking && sinceLastDown > 1000)
        {
            // One second after release, we do the actions based on number of clicks.
            state->clicking = false;

            if (state->clickCount == 1)
            {
                // There was only one click. Disable the lights.
                state->disabled = !state->disabled;
            }
            else if (state->clickCount == 2)
            {
                // TODO: Replace this with calling a pattern factory that gives a version of the theme
                float intensity = (state->clickCount > 10 ? 10 : state->clickCount) / (float) 10;
                pattern_update_data((state->patternIndex + 1) % getPatternCount(), intensity);
            }
            else
            {
                // More than 2 clicks, so we change some properties based on intensity.
                // TODO: Replace this with calling a pattern factory that gives a new version of the current pattern theme
                float intensity = (state->clickCount > 10 ? 10 : state->clickCount) / (float) 10;
                pattern_update_data(state->patternIndex, intensity);

                //float speed = getRandomChance(0.25, 3, intensity);
                //int patternCount = getPatternCount();
                //state->speed = getRandomChance(0.25, 3, state->intensity);
            }
        }
    }
}

typedef void (*button_pattern)(t_state *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp);

const struct
{
    button_pattern pat;
} button_map[] = {

    {button_decrement_pattern_index},
    {button_increment_pattern_index},

    {button_decrement_speed},
    {button_increment_speed},

    {button_toggle_offset},

    {button_toggle_strobe},

    {button_intensity}};

bool executeButton(int i, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    int funcIndex = (i - PIN_BUTTONS_START);
    int buttonCount = sizeof button_map / sizeof button_map[0];
    if (funcIndex < buttonCount)
    {
        button_map[funcIndex].pat(&state, isDown, sinceLastDown, sinceLastUp);
    }
}