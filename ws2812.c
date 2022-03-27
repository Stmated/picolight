#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "actions.h"

t_state state;

void core1_entry()
{
    uint32_t firstMs = get_running_ms();

    uint32_t downMs[PIN_BUTTONS_COUNT];
    uint32_t upMs[PIN_BUTTONS_COUNT];
    bool lastState[PIN_BUTTONS_COUNT];

    for (int i = 0; i < PIN_BUTTONS_COUNT; i++)
    {
        downMs[i] = firstMs;
        upMs[i] = firstMs;
        lastState[i] = get_pin(PIN_BUTTONS_START + i);
    }

    bool upValue = lastState[0];

    while (1)
    {
        // We only check for new actions every X milliseconds
        uint32_t nowMs = get_running_ms();
        for (int i = PIN_BUTTONS_START; i < PIN_BUTTONS_END; i++)
        {
            bool currentState = get_pin(i);
            bool previousState = lastState[i - PIN_BUTTONS_START];
            bool isDown = (currentState != upValue);

            executeButton(i, isDown, nowMs - downMs[i - PIN_BUTTONS_START], nowMs - upMs[i - PIN_BUTTONS_START]);

            if (isDown)
            {
                downMs[i - PIN_BUTTONS_START] = nowMs;
            }
            else
            {
                upMs[i - PIN_BUTTONS_START] = nowMs;
            }
        }

        sleep_ms(10);
    }
}

inline static void execute_for_led_pin(uint32_t time_start, int offset, int pinIndex)
{
    uint32_t time = get_running_ms();
    uint32_t time_elapsed = (time - time_start);

    // TODO: The speed should be random based on the intensity, and only calculated once.
    // TODO: We can do this if we get the factory pattern working!
    uint32_t time_dilated = (time_start + (time_elapsed * state.speed));

    if (state.withOffset)
    {
        time_dilated += (pinIndex * 123456);
    }

    pattern_execute(LED_COUNT, time_dilated);
}

int main()
{
    picolight_boot(LED_COUNT);

    state.patternIndex = 0;
    state.speed = 1;
    state.withOffset = false;
    state.nextPatternIndex = 0;
    state.nextIntensity = 0.1;

    for (int i = PIN_BUTTONS_START; i < PIN_BUTTONS_END; i++)
    {
        init_pin_button(i);
    }

    int offset = get_pio_offset();

    launch_thread(core1_entry);
    picolight_post_boot();

    uint32_t time_start = get_running_ms();

    pattern_find_and_register_patterns();
    state.nextPatternIndex = 0;
    //pattern_update_data(LED_COUNT, state.patternIndex, 0.1);

    if (count_of(PIN_TX) > 1)
    {
        while (1)
        {
            for (int pinIndex = 0; pinIndex < count_of(PIN_TX); pinIndex++)
            {
                int pin = PIN_TX[pinIndex];
                program_init(offset, pin);
                execute_for_led_pin(time_start, offset, pinIndex);

                // We sleep less between each program, the more programs we have. How to sleep less than 1?
                // TODO: We should never sleep; we should instead process pre-frame and just wait if we're done too early
                sleep_us(150 / count_of(PIN_TX)); // minimum is 50, but need safety margins
            }
        }
    }
    else
    {
        program_init(offset, PIN_TX[0]);
        while (1)
        {
            execute_for_led_pin(time_start, offset, 0);

            // TODO: We should never sleep; we should instead process pre-frame and just wait if we're done too early
            sleep_us(300); // minimum is 50us, but need safety margins
        }
    }
}