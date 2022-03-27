#include "../patterns.h"
#include "../led_math.h"

typedef struct pattern_fill_sway_struct
{
    int hue_start;
    int hue_width;
    int speedh;
    int speeds;
    int speedi;

    float sat;
    float sat_width;

    float brightness;
    float brightness_width;
} pattern_fill_sway_struct;

void *pattern_fill_sway_data(uint16_t len, float intensity)
{
    struct pattern_fill_sway_struct *instance = calloc(1, sizeof(struct pattern_fill_sway_struct));

    instance->hue_start = randint(360);
    instance->hue_width = 10 + randint(30);
    instance->speedh = 40000 - (30000 * intensity);
    instance->speeds = 10000 + randint(20000);
    instance->speedi = 10000 + randint(30000);

    float s = (0.75 + (0.25 * intensity));
    float sw = MIN(1, s + 0.2 * (randint(100) / (float)100)) - s;
    float i = (0.05 + (0.5 * intensity));
    float iw = 0.2 * (randint(100) / (float)100);
    if (i + iw > 0.6)
    {
        iw = (0.6 - i);
    }

    instance->sat = s;
    instance->sat_width = sw;
    instance->brightness = i;
    instance->brightness_width = iw;

    return instance; // TODO: Improve this so there's a great range, and much more INTENSE when intensity goes up, and no white
}

void pattern_fill_sway(uint16_t len, uint32_t t, void *data, printer printer)
{
    struct pattern_fill_sway_struct *instance = data;

    // TODO: Randomize the 3 speeds, and make them vastly different from one another
    float ph = InOutCubic((t % instance->speedh) / (float)instance->speedh);
    float ps = InOutCubic((t % instance->speeds) / (float)(instance->speeds));
    float pi = InOutCubic((t % instance->speedi) / (float)(instance->speedi));

    int h = ((int)(instance->hue_start + (ph * instance->hue_width))) % 360;

    HsiColor hsi = {h, instance->sat + (ps * instance->sat_width), instance->brightness + (pi * instance->brightness_width)};

    setAll(len, &hsi, data, printer);
}

void pattern_register_fill_sway()
{
    pattern_register(pattern_fill_sway, pattern_fill_sway_data, pattern_destroyer_default);
}