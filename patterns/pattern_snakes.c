#include "../patterns.h"
#include "../led_math.h"

// Should "sweep" more -- from mid brightness to max brightness, and pulse fast, to give feeling of weird shadows in room

typedef struct pattern_snakes_data_struct
{
    int easing1;
    int easing2;
    int easing3;

    int period1;
    int period2;
    int period3;

    float width1;
    float width2;
    float width3;

    int hue1;
    int hue2;
    int hue3;

    float sat1;
    float sat2;
    float sat3;

    float int1;
    float int2;
    float int3;
} pattern_snakes_data_struct;

void *pattern_snakes_data(uint16_t len, float intensity)
{
    struct pattern_snakes_data_struct *instance = malloc(sizeof(struct pattern_snakes_data_struct));

    // Easings
    instance->easing1 = randint(getEasingCount());
    instance->easing2 = randint(getEasingCount());
    instance->easing3 = randint(getEasingCount());

    // Periods
    instance->period1 = randint_probability(6000, 25000, 1 - intensity);
    instance->period2 = randint_probability(4000, 25000, 1 - intensity);
    instance->period3 = randint_probability(3000, 25000, 1 - intensity);

    // Width
    instance->width1 = (float)randint_probability(len / 8, len / 4, 1 - intensity);
    instance->width2 = (float)randint_probability(len / 16, len / 8, 1 - intensity);
    instance->width3 = (float)randint_probability(3, 5, 1 - intensity);

    // Hue
    instance->hue1 = (intensity > 0.5 ? randint_probability(0, 360, 1 - intensity) : randint(360));
    instance->hue2 = (intensity > 0.5 ? randint_probability(0, 360, 1 - intensity) : randint(360));
    instance->hue3 = (intensity > 0.5 ? randint_probability(0, 360, 1 - intensity) : randint(360));

    // Saturation
    instance->sat1 = randint_probability(0, 1000, 1 - intensity) / (float)1000;
    instance->sat2 = randint_probability(0, 1000, 1 - intensity) / (float)1000;
    instance->sat3 = randint_probability(0, 1000, 1 - intensity) / (float)1000;

    // Intensity
    instance->int1 = randint_probability(200, 500, 1 - intensity) / (float)1000;
    instance->int2 = randint_probability(200, 500, 1 - intensity) / (float)1000;
    instance->int3 = randint_probability(200, 500, 1 - intensity) / (float)1000;

    return instance;
}

void pattern_snakes(uint16_t len, uint32_t t, void *data, printer printer)
{
    struct pattern_snakes_data_struct *instance = data;

    float p1 = len * executeEasing(instance->easing1, (t % instance->period1) / (float)instance->period1);
    float p2 = len * executeEasing(instance->easing2, (t % instance->period2) / (float)instance->period2);
    float p3 = len * executeEasing(instance->easing3, (t % instance->period3) / (float)instance->period3);

    HsiColor colors[len];

    for (int i = 0; i < len; i++)
    {
        float dist1 = fabsf(i - p1);
        float dist2 = fabsf(i - p2);
        float dist3 = fabsf(i - p3);

        float v_s = 0;
        float v_i = 0;
        int matches = 0;
        double angles[3];
        if (dist1 <= instance->width1)
        {
            float strength1 = 1 - (dist1 / instance->width1);

            // Increase light generally in big area
            angles[matches] = instance->hue1;
            v_s += instance->sat1 * strength1;
            v_i += instance->int1 * strength1;
            matches++;
        }

        if (dist2 <= instance->width2)
        {
            // Increase light more generally in medium area
            float strength = 1 - (dist2 / instance->width2);
            angles[matches] = instance->hue2;
            v_s += instance->sat2 * strength;
            v_i += instance->int2 * strength;
            matches++;
        }

        if (dist3 <= instance->width3)
        {
            // Small area
            float strength = 1 - (dist3 / instance->width3);
            angles[matches] = instance->hue3;
            v_s += instance->sat3 * strength;
            v_i += instance->int3 * strength;
            matches++;
        }

        int v_h = 0;
        if (matches > 0)
        {
            v_h = (matches > 1) ? getAverageAngle(angles, matches) : angles[0];
            v_s /= matches;
            v_i /= matches;
        }

        HsiColor hsi = {v_h % 360, 1, v_i};
        colors[i] = hsi;
        // uint8_t rgbw[4];
        /// hsi2rgbw(hsi.h, hsi.s, hsi.i, rgbw);

        //hsi2rgbw(hsi.h, hsi.s, hsi.i, &rgbw[i]);
    }

    for (int i = 0; i < len; i++)
    {
        printer(i, &colors[i], data);
    }
}

void pattern_register_snakes()
{
    pattern_register(pattern_snakes, pattern_snakes_data, pattern_destroyer_default);
}