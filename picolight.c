#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "actions.h"

#define PERFORMANCE_STATS true
#undef PERFORMANCE_STATS
#define PERFORMANCE_SAMPLES 100
#define NANO_IN_SECOND 1000000000.0

GlobalState state;

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
    //uint32_t time_elapsed = (time - time_start);
    //uint32_t time_dilated = (time_start + (time_elapsed * state.speed));

    //if (state.withOffset)
    //{
    //    time_dilated += (pinIndex * 123456);
    //}

    pattern_execute(LED_COUNT, time);
}

int main()
{
    picolight_boot(LED_COUNT);

    state.patternIndex = 0;
    state.speed = 1;
    state.withOffset = false;
    state.nextPatternIndex = 0;
    state.nextIntensity = 0.4;

    for (int i = PIN_BUTTONS_START; i < PIN_BUTTONS_END; i++)
    {
        init_pin_button(i);
    }

    int offset = get_pio_offset();

    launch_thread(core1_entry);
    picolight_post_boot();

    uint32_t time_start = get_running_ms();

    pattern_find_and_register_patterns();
    math_precompute();
    state.nextPatternIndex = 0;

    register_action_listeners();

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
                // TODO: We should never sleep; we should instead process pre-frame and only wait if we're done too early
                sleep_us(150 / count_of(PIN_TX)); // minimum is 50, but need safety margins
            }
        }
    }
    else
    {
        program_init(offset, PIN_TX[0]);
#ifdef PERFORMANCE_STATS
        uint32_t timings[PERFORMANCE_SAMPLES];
        int timingIndex = 0;
#endif
        while (1)
        {
#ifdef PERFORMANCE_STATS
            uint64_t before = get_running_us();
#endif

            execute_for_led_pin(time_start, offset, 0);

            // TODO: We should never sleep; we should instead process pre-frame and only wait if we're done too early
            //sleep_us(150); // minimum is 50us, but need safety margins
            sleep_us(300); // minimum is 50us, but need safety margins

#ifdef PERFORMANCE_STATS
            uint64_t after = get_running_us();
            timings[timingIndex] = (after - before);
            timingIndex = (timingIndex + 1) % PERFORMANCE_SAMPLES;
            if (timingIndex == 0)
            {
                uint64_t total = 0;
                for (int i = 0; i < PERFORMANCE_SAMPLES; i++)
                {
                    total += timings[i];
                }

                float average = total / (float)PERFORMANCE_SAMPLES;
                float fps = NANO_IN_SECOND / average;

                // TODO: Show this some other way depending on environment. If on Pico, the first LEDs should have a colored base64 system of showing the FPS (higher hue, higher value)
                printf("%f, %f\n", average, fps);
            }
#endif
        }
    }
}

// TODO:
// * https://github.com/google/orbit
//      Profile like a madman, find out where the time is spent!
//      (or add simple #DEFINE guards and run all patterns and print a performance table)

// TODO:
// * Make two kinds of "random" -- one that blends while fading between, and one that fades in and out one by one
// * Make a test where we try to blend different colors at different opacities and look at which one actually turns out the most realistic (rgbw blend vs hsia blend)
// * Need a better system of "random" that has more versatile weighting system so we can more reliably say what kind of number we prefer, unless the intensity is really strong/weak
// * Move away from only "intensity" and instead move into multiple parameters: Colorfulness, Intensity, Speed, Brightness, Randomness