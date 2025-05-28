#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include "actions.h"
#include "storage.h"
#include "environment/environment.h"

#include "global.h"
#include "pico/time.h"

#define PERFORMANCE_STATS true
#undef PERFORMANCE_STATS
#define PERFORMANCE_SAMPLES 100
#define NANO_IN_SECOND 1000000000.0

GlobalState state;

/*
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
*/

uint32_t getTotalHeap(void) {
   extern char __StackLimit, __bss_end__;
   return &__StackLimit  - &__bss_end__;
}

static alarm_id_t debounce_store_state_alarm_id; // handle for the delay alert
volatile bool should_store_state = false;

static int64_t debounce_store_state(alarm_id_t id, void *user_data)
{
    should_store_state = true;
    return 0;
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

        void *data = newModule->creator(len, state.intensity);

        // Set to the new (or same) pattern index, and new data
        state.patternIndex = state.nextPatternIndex;
        state.patternData = data;

        state.nextPatternIndex = -1;

        // We allocate the memory for the frame data right away, so we do not need to allocate/deallocate over and over.
        state.frameData = newModule->frameAllocator(len, state.patternData);

        // Settings have changed, so we store them after a set time.
        if (debounce_store_state_alarm_id != 0)
        {
            // Cancel any already running alarm
            cancel_alarm(debounce_store_state_alarm_id);
        }

        // Wait a certain time and then store the state. Avoid doing it often since it hurts the flash memory.
        debounce_store_state_alarm_id = add_alarm_in_ms(30000, debounce_store_state, NULL, false);
    }

    // Execute the current pattern inside state.
    PatternModule *module = pattern_get_by_index(state.patternIndex);

    module->frameCreator(len, t_ms, state.patternData, state.frameData);

    ExecutorArgs *args = &(ExecutorArgs){0, state.patternData, state.frameData};
    while (args->i < len)
    {
        RgbwaColor rgbwa = module->executor(args);
        put_pixel(args->i, len, t_us, delta_t_us, &rgbwa);
        args->i++;
    }

    previous_t_us = t_us;
}

inline static void execute_for_led_pin(uint32_t time_start, int offset, int pinIndex)
{
    uint32_t time_us = get_running_us();
    
    if (state.withOffset)
    {
        uint32_t time_elapsed = (time_us - time_start);
        uint32_t time_dilated = (time_start + (time_elapsed * state.speed));

        time_dilated += (pinIndex * 123456);

        pattern_execute(state.ledCount, time_dilated);
    } else {
        pattern_execute(state.ledCount, time_us);
    }
}

int main()
{
    // We assume 100 lights as default
    state.ledCount = 100;

    picolight_boot(state.ledCount);

    state.patternIndex = 0;
    state.speed = 3;
    state.withOffset = false;
    state.nextPatternIndex = 0;
    state.nextLedCount = -1;
    state.intensity = 0.4;
    state.buffer_invalidated = false;

    state_load();

    // Starting at PIN 4 (from upper left of card): 4 pins, 1 ground, then 4 more pins
    /*
    for (int i = PIN_BUTTONS_START; i < PIN_BUTTONS_START + getButtonCount(); i++)
    {
        init_pin_button(i);
    }
    */

    registerCallbacks();

    int offset = get_pio_offset();

    // TODO: Make use of the 2nd core in the future, perhaps calculating every Nth pixel, and then joining with a semaphore
    //launch_thread(core1_entry);

    picolight_post_boot(offset);

    uint64_t time_start_us = get_running_us();

    pattern_find_and_register_patterns();
    math_precompute();
    //state.nextPatternIndex = 0;

    register_action_listeners();

    if (count_of(PIN_TX) > 1)
    {
        while (1)
        {
            for (int pinIndex = 0; pinIndex < count_of(PIN_TX); pinIndex++)
            {
                int pin = PIN_TX[pinIndex];
                program_init(offset, pin);
                execute_for_led_pin(time_start_us, offset, pinIndex);

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
            if (should_store_state)
            {
                should_store_state = false;
                state_store();
            }

#ifdef PERFORMANCE_STATS
            uint64_t before = get_running_us();
#endif

            execute_for_led_pin(time_start_us, offset, 0);

            // TODO: We should never sleep; we should instead process pre-frame and only wait if we're done too early
            //sleep_us(300); // minimum is 50us, but need safety margins

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