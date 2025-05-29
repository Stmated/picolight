#include "hardware/gpio.h"

#include "actions.h"
#include "global.h"
#include "environment/environment.h"

void button_decrement_pattern_index()
{
    if (state.patternIndex == 0)
    {
        state.nextPatternIndex = getPatternCount() - 1;
    }
    else
    {
        state.nextPatternIndex = (state.patternIndex - 1);
    }
}

void button_increment_pattern_index()
{
    state.nextPatternIndex = (state.patternIndex + 1) % getPatternCount();
}

void button_decrement_speed()
{
    state.speed = (state.speed * 0.8);
    state.nextPatternIndex = state.patternIndex;
}

void button_increment_speed()
{
    state.speed = (state.speed * 1.2);
    state.nextPatternIndex = state.patternIndex;
}

void intensity_increase()
{
    state.intensity = state.intensity + 0.1;
    state.nextPatternIndex = state.patternIndex;
}

void intensity_decrease()
{
    state.intensity = MAX(0, state.intensity - 0.1);
    state.nextPatternIndex = state.patternIndex;
}

void length_decrement()
{
    state.nextLedCount = state.ledCount - 1;
    state.nextPatternIndex = state.patternIndex;
}

void length_increment()
{
    state.nextLedCount = state.ledCount + 1;
    state.nextPatternIndex = state.patternIndex;
}

void reset()
{
    state.speed = 1;
    state.intensity = 0.5;
    state.nextPatternIndex = 0;
}

void button_callback(uint gpio, uint32_t events)
{
    static uint64_t last_callback = -1;

    if (gpio >= 2 && gpio <= 15 && (events & GPIO_IRQ_EDGE_FALL))
    {
        if (last_callback != -1)
        {
            uint64_t duration_since_last_callback = get_running_ms() - last_callback;
            if (duration_since_last_callback < 100)
            {
                // Skip this one, since things are activating too fast.
                return;
            }
        }
        last_callback = get_running_ms();

        // 0 used
        // 1 used
        // gnd
        if (gpio == 2) button_decrement_pattern_index();
        else if (gpio == 3) button_increment_pattern_index();
        
        else if (gpio == 4) button_decrement_speed();
        else if (gpio == 5) button_increment_speed();
        // gnd
        else if (gpio == 6) intensity_decrease();
        else if (gpio == 7) intensity_increase();

        else if (gpio == 8) length_decrement();
        else if (gpio == 9) length_increment();
        // gnd
        else if (gpio == 10) reset();
    }
}

void registerCallbacks()
{
    for (int i = 2; i <= 15; i++)
    {
        init_pin_button(i);
        gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_FALL, true, &button_callback);
    }
}
