#include "actions.h"

void button_increment_pattern_index(GlobalState *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    if (isDown && sinceLastDown > 250)
    {
        state->patternIndex = (state->patternIndex + 1) % getPatternCount();
    }
}

void button_decrement_pattern_index(GlobalState *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
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

void button_decrement_speed(GlobalState *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    /*
    if (isDown && sinceLastDown > 250)
    {
        state->speed = (state->speed * 0.8);
    }
    */
}

void button_increment_speed(GlobalState *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    /*
    if (isDown && sinceLastDown > 250)
    {
        state->speed = (state->speed * 1.2);
    }
    */
}

void button_toggle_offset(GlobalState *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
{
    if (isDown && sinceLastDown > 250)
    {
        state->withOffset = !state->withOffset;
    }
}

void button_toggle_strobe(GlobalState *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
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
    float r = (rand() / (int)RAND_MAX);
    return min + ((max * strength) * r);
}

void button_intensity(GlobalState *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp)
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
                float intensity = (state->clickCount > 10 ? 10 : state->clickCount) / (int) 10;
                state->nextPatternIndex = (state->patternIndex + 1) % getPatternCount();
                state->nextIntensity = intensity;
            }
            else
            {
                // More than 2 clicks, so we change some properties based on intensity.
                // Intensity is any value between 0 and Inf.
                // The higher the intensity, the lower values will be generated by the weighted randoms.
                float intensity;
                if (state->clickCount > 5)
                {
                    // If more than 5, then we will start to force harder.
                    // From 1 to Inf, depending on how many clicks we got
                    intensity = (state->clickCount - 4) * 1.5;
                }
                else
                {
                    // Lower or same as 5, we take it a bit easier.
                    // From 0.1 to 1
                    intensity = state->clickCount / (float) 5;
                }
                
                state->nextPatternIndex = state->patternIndex;
                state->nextIntensity = intensity;
            }
        }
    }
}

typedef void (*button_pattern)(GlobalState *state, bool isDown, uint32_t sinceLastDown, uint32_t sinceLastUp);

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
        return true;
    }

    return false;
}