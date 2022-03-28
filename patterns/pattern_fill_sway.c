#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_fill_sway_struct
{
    int hue_start;
    int hue_width;
    int speedh;
    int speeds;
    int speedi;

    int easing_h;
    int easing_s;
    int easing_i;

    float sat_from;
    float sat_width;

    float brightness_from;
    float brightness_width;
} pattern_fill_sway_struct;

void *pattern_fill_sway_data(uint16_t len, float intensity)
{
    struct pattern_fill_sway_struct *instance = calloc(1, sizeof(struct pattern_fill_sway_struct));

    instance->hue_start = randint(360);
    instance->hue_width = 60 + randint_weighted_towards_max(0, 120, intensity);
    instance->speedh = 10000 + randint_weighted_towards_min(0, 30000, intensity);
    instance->speeds = 5000 + randint_weighted_towards_min(0, 20000, intensity);
    instance->speedi = 5000 + randint_weighted_towards_min(0, 10000, intensity);

    // TODO: Need a way to select an easing based on intensity, so they are ranked by fastness/intensity
    instance->easing_h = randint(getEasingCount());
    instance->easing_s = randint(getEasingCount());
    instance->easing_i = randint(getEasingCount());

    // TODO: The values MUST NOT GO OVER 1! The calculations become WONKY!!!!
    instance->sat_from = 0.8 + (0.2 * (randint_weighted_towards_max(0, 100, intensity) / (float) 100));
    instance->brightness_from = 0.2 + (0.1 * (randint_weighted_towards_max(0, 100, intensity) / (float) 100));

    float sat_to = MIN(1, instance->sat_from + 0.1 + (0.4 * (randint_weighted_towards_max(0, 100, intensity) / (float) 100)));
    float brightness_to = MIN(1, instance->brightness_from + 0.2 + (0.10 * (randint_weighted_towards_max(0, 100, intensity) / (float) 100)));

    instance->sat_width = (sat_to - instance->sat_from);
    instance->brightness_width = (brightness_to - instance->brightness_from);

    return instance;
}

void pattern_fill_sway(uint16_t len, uint32_t t, void *data, PatternPrinter printer)
{
    struct pattern_fill_sway_struct *instance = data;

    float ph = executeEasing(instance->easing_h, (t % instance->speedh) / (float)instance->speedh);
    float ps = executeEasing(instance->easing_s, (t % instance->speeds) / (float)(instance->speeds));
    float pi = executeEasing(instance->easing_i, (t % instance->speedi) / (float)(instance->speedi));

    int h = ((int)(instance->hue_start + (ph * instance->hue_width))) % 360;

    HsiColor hsi = {h, instance->sat_from + (ps * instance->sat_width), instance->brightness_from + (pi * instance->brightness_width)};

    setAll(len, &hsi, data, printer);
}

void pattern_register_fill_sway()
{
    pattern_register("sway", pattern_fill_sway, pattern_fill_sway_data, pattern_destroyer_default, &(PatternOptions){1});
}