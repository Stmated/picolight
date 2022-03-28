#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_red_bouncer_smooth_struct
{
    int easing;
    int hue;
    int period;
    int width;
} pattern_red_bouncer_smooth_struct;

static void *pattern_bouncer_data(uint16_t len, float intensity)
{
    pattern_red_bouncer_smooth_struct *instance = calloc(1, sizeof(pattern_red_bouncer_smooth_struct));

    instance->easing = randint(getEasingCount());
    instance->hue = randint_weighted_towards_max(0, 360, intensity);
    instance->period = 500 + randint_weighted_towards_min(0, 10000, intensity);
    instance->width = 1 + randint(6);

    return instance;
}

static void pattern_bouncer(uint16_t len, uint32_t t, void *data, PatternPrinter printer)
{
    pattern_red_bouncer_smooth_struct *instance = data;

    const float p = len * executeEasing(instance->easing, (t % instance->period) / (float)instance->period);

    HsiColor black = {0, 0, 0};
    for (int i = 0; i < len; i++)
    {
        float distance = fabsf(i - p);

        if (distance <= instance->width)
        {
            HsiColor hsi = {instance->hue, 1, (1 - (distance / (float)instance->width))};
            printer(i, &hsi, data);
        }
        else
        {
            printer(i, &black, data);
        }
    }
}

void pattern_register_bouncer()
{
    pattern_register(pattern_bouncer, pattern_bouncer_data, pattern_destroyer_default, &(PatternOptions){1});
}