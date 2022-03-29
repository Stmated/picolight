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
    instance->sat_from = 0.8 + (0.2 * (randint_weighted_towards_max(0, 100, intensity) / (float)100));
    instance->brightness_from = 0.2 + (0.1 * (randint_weighted_towards_max(0, 100, intensity) / (float)100));

    float sat_to = MIN(1, instance->sat_from + 0.1 + (0.4 * (randint_weighted_towards_max(0, 100, intensity) / (float)100)));
    float brightness_to = MIN(1, instance->brightness_from + 0.2 + (0.10 * (randint_weighted_towards_max(0, 100, intensity) / (float)100)));

    instance->sat_width = (sat_to - instance->sat_from);
    instance->brightness_width = (brightness_to - instance->brightness_from);

    return instance;
}

void pattern_fill_sway(uint16_t offset, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    struct pattern_fill_sway_struct *data = dataPtr;

    float ph = executeEasing(data->easing_h, (t % data->speedh) / (float)data->speedh);
    float ps = executeEasing(data->easing_s, (t % data->speeds) / (float)(data->speeds));
    float pi = executeEasing(data->easing_i, (t % data->speedi) / (float)(data->speedi));

    int h = ((int)(data->hue_start + (ph * data->hue_width))) % 360;

    HsiColor hsi = {h, data->sat_from + (ps * data->sat_width), data->brightness_from + (pi * data->brightness_width)};

    setAll(offset, len, &hsi, dataPtr, cyclePtr, printer);
}

void pattern_register_fill_sway()
{
    pattern_register("sway", pattern_fill_sway,
                     pattern_fill_sway_data, pattern_destroyer_default,
                     pattern_cycle_creator_default, pattern_cycle_destroyer_default,
                     &(PatternOptions){1});
}