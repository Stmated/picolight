#include "global.h"
#include "patterns.h"
#include "easing.h"
#include "options.h"
#include "environment.h"

const uint32_t pattern_sweeping_length = 40;
// const float pattern_sweeping_length_half = pattern_sweeping_length / (float) 2;
const uint32_t pattern_sweeping_speed = 1000;
const float pattern_sweeping_per_step = 6.375; // 255 / (float)pattern_sweeping_length;

int randint(int n)
{
    if ((n - 1) == RAND_MAX)
    {
        return rand();
    }
    else
    {
        // Chop off all of the values that would cause skew...
        int end = RAND_MAX / n; // truncate skew
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        int r;
        while ((r = rand()) >= end)
            ;

        return r % n;
    }
}

/**
 * Higher power means higher probability to be lower
 * */
int randint_probability(int min, int max, float probabilityPower)
{
    float random = randint(100000) / (float)100000;
    return (int)floorf(min + (max - min) * (powf(random, probabilityPower)));
}

inline void setAll(uint16_t len, uint8_t red, uint8_t green, uint8_t blue, uint8_t white, void *data, printer printer)
{
    for (uint16_t i = 0; i < len; i++)
    {
        printer(i, red, green, blue, white, data);
    }

    // We sleep 1 ms, since this is usually too fast.
    sleep_ms(1);
}

void *pattern_fill_sway_data(uint16_t len, float intensity)
{
    void *data = malloc((sizeof(int) * 5) + sizeof(float) * 4);
    int *intData = (int *)data;
    float *floatData = (float *)(data + (sizeof(int) * 5));

    intData[0] = randint(360);
    intData[sizeof(int)] = 10 + randint(30);
    intData[sizeof(int) * 2] = 40000 - (30000 * intensity);
    intData[sizeof(int) * 3] = 10000 + randint(20000);
    intData[sizeof(int) * 4] = 10000 + randint(30000);

    float s = (0.75 + (0.25 * intensity));
    float sw = MIN(1, s + 0.2 * (randint(100) / (float)100)) - s;
    float i = (0.05 + (0.5 * intensity));
    float iw = 0.2 * (randint(100) / (float)100);
    if (i + iw > 0.6)
    {
        iw = (0.6 - i);
    }

    floatData[0] = s;                  // sat1
    floatData[sizeof(float)] = sw;     // sat width
    floatData[sizeof(float) * 2] = i;  // brightness1
    floatData[sizeof(float) * 3] = iw; // brightness width

    return data; // TODO: Improve this so there's a great range, and much more INTENSE when intensity goes up, and no white
}

void pattern_fill_sway(uint16_t len, uint32_t t, void *data, printer printer)
{
    int *intData = (int *)(data);
    float *floatData = (float *)(data + (sizeof(int) * 5));

    int hue_start = intData[0];
    int hue_width = intData[sizeof(int)];
    int speedh = intData[sizeof(int) * 2];
    int speeds = intData[sizeof(int) * 3];
    int speedi = intData[sizeof(int) * 4];
    float s = floatData[0];
    float sw = floatData[sizeof(float)];
    float i = floatData[sizeof(float) * 2];
    float iw = floatData[sizeof(float) * 3];

    // TODO: Randomize the 3 speeds, and make them vastly different from one another
    float ph = InOutCubic((t % speedh) / (float)speedh);
    float ps = InOutCubic((t % speeds) / (float)(speeds));
    float pi = InOutCubic((t % speedi) / (float)(speedi));

    int h = ((int)(hue_start + (ph * hue_width))) % 360;

    HsiColor hsi = {h, s + (ps * sw), i + (pi * iw)};

    uint8_t rgbw[4];
    hsi2rgbw(hsi.h, hsi.s, hsi.i, rgbw);

    setAll(len, rgbw[0], rgbw[1], rgbw[2], rgbw[3], data, printer);
}

struct pattern_random_data_instance
{
    void *data1;
    void *data2;
    pattern_data_destroyer data1_destroyer;
    pattern_data_destroyer data2_destroyer;
    int period;
    uint32_t updatedAt;
    float intensity;
    int patternIndex1;
    int patternIndex2;
    RgbwColor *pixels;
    float progress;
    bool progressReversed;
    bool subsequent;
};

void *pattern_random_data_destroyer(void *data)
{
    struct pattern_random_data_instance *instance = data;

    if (instance->pixels)
    {
        free(instance->pixels);
        instance->pixels = 0;
    }

    if (instance->data1_destroyer)
    {
        instance->data1_destroyer(instance->data1);
        instance->data1 = 0;
    }

    if (instance->data2_destroyer)
    {
        instance->data2_destroyer(instance->data2);
        instance->data2 = 0;
    }

    free(data);
}

void *pattern_random_data(uint16_t len, float intensity)
{
    struct pattern_random_data_instance *instance = calloc(1, sizeof(struct pattern_random_data_instance));

    instance->intensity = intensity;
    instance->period = randint_probability(500, 20000, intensity);
    instance->updatedAt = 0;
    instance->patternIndex1 = 1 + randint(getPatternCount() - 2);
    instance->patternIndex2 = 1 + randint(getPatternCount() - 2);
    instance->data1 = getPatternCreator(instance->patternIndex1)(len, instance->intensity);
    instance->data2 = getPatternCreator(instance->patternIndex2)(len, instance->intensity);
    instance->data1_destroyer = getPatternDestroyer(instance->patternIndex1);
    instance->data2_destroyer = getPatternDestroyer(instance->patternIndex2);
    instance->pixels = calloc(len, sizeof(struct RgbwColor)); // Allocate memory for each pixel, which we will need to blend

    return instance;
}

// TODO: Currently pretty ugly that we do not get the HSI here. Double conversions and everything.
// TODO: Figure out if there is a good way of being able to send either RGB or HSI or both!
void pattern_random_data_printer(uint16_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w, void *data)
{
    // Important to use the global data here, since "data" is from the sub-pattern
    struct pattern_random_data_instance *instance = state.patternData;

    // RgbwColor pixel = ;

    if (instance->subsequent)
    {
        // Blend. FIX SO THIS CAN HANDLE HSI AND NOT NEED TO CONVERT BACK AND FORTH!

        float p = 1 - instance->progress;
        float invp = instance->progress;

        instance->pixels[index].r = ((instance->pixels[index].r * p) + (r * invp)) / 2;
        instance->pixels[index].g = ((instance->pixels[index].g * p) + (g * invp)) / 2;
        instance->pixels[index].b = ((instance->pixels[index].b * p) + (b * invp)) / 2;
        instance->pixels[index].w = ((instance->pixels[index].w * p) + (w * invp)) / 2;
    }
    else
    {
        // Just assign it
        instance->pixels[index].r = r;
        instance->pixels[index].g = g;
        instance->pixels[index].b = b;
        instance->pixels[index].w = w;
    }
}

void pattern_random(uint16_t len, uint32_t t, void *data, printer printer)
{
    // TODO: Delegate to a random pattern
    // TODO: The best thing would be if we could intercept all the pixels, and blend different patterns into one
    struct pattern_random_data_instance *instance = data;

    if (instance->updatedAt == 0 || t > (instance->updatedAt + instance->period))
    {
        // It is time to move on to the next random mix.
        instance->updatedAt = t;
        instance->progress = 0;

        if (instance->data1)
        {
            instance->data1_destroyer(instance->data1);
            instance->data1 = 0;
        }

        instance->patternIndex1 = instance->patternIndex2;
        instance->patternIndex2 = 1 + randint(getPatternCount() - 1);

        instance->data1 = instance->data2;
        instance->data2 = getPatternCreator(instance->patternIndex2)(len, instance->intensity);

        instance->data1_destroyer = instance->data2_destroyer;
        instance->data2_destroyer = getPatternDestroyer(instance->patternIndex2);

        // Ugly workaround so progress goes 0->1,1->0, and so on.
        // This way there should not be an instant JUMP when we switch to next pattern.
        instance->progressReversed = !instance->progressReversed;
    }

    // Set the progress so we can calculate the proper crossover
    int age = t - instance->updatedAt;
    instance->progress = age / (float)instance->period;
    if (instance->progressReversed)
    {
        instance->progress = 1 - instance->progress;
    }

    instance->subsequent = false;
    getPattern(instance->patternIndex1)(len, t, instance->data1, pattern_random_data_printer);
    instance->subsequent = true;
    getPattern(instance->patternIndex2)(len, t, instance->data2, pattern_random_data_printer);

    for (int i = 0; i < len; i++)
    {
        // Now let's send the data to the original printer
        RgbwColor pixel = instance->pixels[i];
        printer(i, pixel.r, pixel.g, pixel.b, pixel.w, data);
    }
}

double getAverageAngle(double *angles, int length)
{
    double x = 0;
    double y = 0;

    for (int i = 0; i < length; i++)
    {
        double r = angles[i] * (M_PI / 180.0);
        x += cos(r);
        y += sin(r);
    }

    double radians = atan2(y, x);
    int degrees = (int)round(radians * (180.0 / M_PI));
    int fixedDegreees = (degrees + 360) % 360;
    return fixedDegreees;
}

void *pattern_snakes_data(uint16_t len, float intensity)
{
    void *data = (void *)calloc(18, sizeof(float));
    // int *intData = (int *)data;
    float *floatData = (float *)data;

    // Easings
    floatData[sizeof(float) * 0] = (float)randint(getEasingCount());
    floatData[sizeof(float) * 1] = (float)randint(getEasingCount());
    floatData[sizeof(float) * 2] = (float)randint(getEasingCount());

    // Periods
    floatData[sizeof(float) * 3] = (float)randint_probability(5000, 20000, 1 - intensity);
    floatData[sizeof(float) * 4] = (float)randint_probability(3000, 20000, 1 - intensity);
    floatData[sizeof(float) * 5] = (float)randint_probability(2000, 25000, 1 - intensity);

    // Width
    floatData[sizeof(float) * 6] = (float)randint_probability(len / 8, len / 4, 1 - intensity);
    floatData[sizeof(float) * 7] = (float)randint_probability(len / 16, len / 8, 1 - intensity);
    floatData[sizeof(float) * 8] = (float)randint_probability(3, 5, 1 - intensity);

    // Hue
    floatData[sizeof(float) * 9] = (float)(intensity > 0.5 ? randint_probability(0, 360, 1 - intensity) : randint(360));
    floatData[sizeof(float) * 10] = (float)(intensity > 0.5 ? randint_probability(0, 360, 1 - intensity) : randint(360));
    floatData[sizeof(float) * 11] = (float)(intensity > 0.5 ? randint_probability(0, 360, 1 - intensity) : randint(360));

    // Saturation
    floatData[sizeof(float) * 12] = randint_probability(0, 1000, 1 - intensity) / (float)1000;
    floatData[sizeof(float) * 13] = randint_probability(0, 1000, 1 - intensity) / (float)1000;
    floatData[sizeof(float) * 14] = randint_probability(0, 1000, 1 - intensity) / (float)1000;

    // Intensity
    floatData[sizeof(float) * 15] = randint_probability(200, 500, 1 - intensity) / (float)1000;
    floatData[sizeof(float) * 16] = randint_probability(200, 500, 1 - intensity) / (float)1000;
    floatData[sizeof(float) * 17] = randint_probability(200, 500, 1 - intensity) / (float)1000;

    return data;
}

void pattern_snakes(uint16_t len, uint32_t t, void *data, printer printer)
{
    float *floatData = ((float *)data);
    // float *floatData = ((float *)data + sizeof(int) * 11);

    int easing1 = (int)floatData[sizeof(float) * 0];
    int easing2 = (int)floatData[sizeof(float) * 1];
    int easing3 = (int)floatData[sizeof(float) * 2];

    int period1 = (int)floatData[sizeof(float) * 3];
    int period2 = (int)floatData[sizeof(float) * 4];
    int period3 = (int)floatData[sizeof(float) * 5];

    float width1 = floatData[sizeof(float) * 6];
    float width2 = floatData[sizeof(float) * 7];
    float width3 = floatData[sizeof(float) * 8];

    float hue1 = floatData[sizeof(float) * 9];
    float hue2 = floatData[sizeof(float) * 10];
    float hue3 = floatData[sizeof(float) * 11];

    float sat1 = floatData[sizeof(float) * 12];
    float sat2 = floatData[sizeof(float) * 13];
    float sat3 = floatData[sizeof(float) * 14];

    float int1 = floatData[sizeof(float) * 15];
    float int2 = floatData[sizeof(float) * 16];
    float int3 = floatData[sizeof(float) * 17];

    float p1 = len * executeEasing(easing1, (t % period1) / (float)period1);
    float p2 = len * executeEasing(easing2, (t % period2) / (float)period2);
    float p3 = len * executeEasing(easing3, (t % period3) / (float)period3);

    uint8_t rgbw[len][4];

    for (int i = 0; i < len; i++)
    {
        float dist1 = fabsf(i - p1);
        float dist2 = fabsf(i - p2);
        float dist3 = fabsf(i - p3);

        float v_s = 0;
        float v_i = 0;
        int matches = 0;
        double angles[3];
        if (dist1 <= width1)
        {
            float strength1 = 1 - (dist1 / width1);

            // Increase light generally in big area
            angles[matches] = hue1;
            v_s += sat1 * strength1;
            v_i += int1 * strength1;
            matches++;
        }

        if (dist2 <= width2)
        {
            // Increase light more generally in medium area
            float strength = 1 - (dist2 / width2);
            angles[matches] = hue2;
            v_s += sat2 * strength;
            v_i += int2 * strength;
            matches++;
        }

        if (dist3 <= width3)
        {
            // Small area
            float strength = 1 - (dist3 / width3);
            angles[matches] = hue3;
            v_s += sat3 * strength;
            v_i += int3 * strength;
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
        // uint8_t rgbw[4];
        /// hsi2rgbw(hsi.h, hsi.s, hsi.i, rgbw);

        hsi2rgbw(hsi.h, hsi.s, hsi.i, rgbw[i]);
    }

    for (int i = 0; i < len; i++)
    {
        printer(i, rgbw[i][0], rgbw[i][1], rgbw[i][2], rgbw[i][3], data);
    }
}

void *pattern_red_bouncer_smooth_data(uint16_t len, float intensity)
{
    void *data = malloc((sizeof(int) * 3));
    int *intData = (int *)data;

    intData[sizeof(int) * 0] = randint(getEasingCount());
    intData[sizeof(int) * 1] = 45 + randint_probability(0, 315, 1 - intensity);
    intData[sizeof(int) * 2] = 500 + randint_probability(0, 10000, 3 * intensity);
    return data;
}

void pattern_red_bouncer_smooth(uint16_t len, uint32_t t, void *data, printer printer)
{
    const int *intData = (int *)data;
    const int easingIndex = intData[sizeof(int) * 0];
    const int hue = intData[sizeof(int) * 1];
    const int period = intData[sizeof(int) * 2];

    const float p = len * executeEasing(easingIndex, (t % period) / (float)period);

    uint8_t rgbw[4];

    for (int i = 0; i < len; i++)
    {
        float distance = fabsf(i - p);

        if (distance <= 2)
        {
            HsiColor hsi = {hue, 1, (1 - (distance / (float)2))};
            hsi2rgbw(hsi.h, hsi.s, hsi.i, rgbw);
            printer(i, rgbw[0], rgbw[1], rgbw[2], rgbw[3], data);
        }
        else
        {
            printer(i, 0, 0, 0, 0, data);
        }
    }
}

void *pattern_fade_between_data(uint16_t len, float intensity)
{
    // TODO: Create a bunch of data for different kinds of fades

    void *data = malloc((sizeof(int) + sizeof(float) * 2));
    int *intData = (int *)data;
    float *floatData = (float *)(data + sizeof(int));

    intData[sizeof(int) * 0] = 20000 - randint_probability(0, 18000, 1 - intensity);
    floatData[sizeof(float) * 0] = 0.5 + 0.5 * (randint_probability(0, 10000, 1 - intensity) / (float)10000);
    floatData[sizeof(float) * 1] = 0.1 + 0.5 * (randint_probability(0, 10000, 1 - intensity) / (float)10000);
    return data;
}

void pattern_fade_between(uint16_t len, uint32_t t, void *data, printer printer)
{
    int *intData = (int *)data;
    float *floatData = (float *)(data + sizeof(int));
    int time_per_color = intData[0];
    int time_into_color = (t % time_per_color);
    double percentage_into_color = (time_into_color / (double)time_per_color);
    float s = floatData[sizeof(float) * 0];
    float i = floatData[sizeof(float) * 1];

    int64_t steps = floor(t / (double)time_per_color);

    int hue_from = ((steps)*203) % 360;
    int hue_to = ((steps + 1) * 203) % 360;

    HsiColor hsi_from = {hue_from, s, i};
    HsiColor hsi_to = {hue_to, s, i};

    HsiColor result = LerpHSI(hsi_from, hsi_to, percentage_into_color);

    uint8_t rgbw[4];
    hsi2rgbw(result.h, result.s, result.i, rgbw);

    for (int i = 0; i < len; i++)
    {
        printer(i, rgbw[0], rgbw[1], rgbw[2], rgbw[3], data);
    }
}

void pattern_strobe(uint16_t len, uint32_t t, void *data, printer printer)
{
    uint32_t remainder = t % 100;
    if (remainder < 50)
    {
        for (int i = 0; i < len; i++)
        {
            printer(i, 255, 255, 255, 255, data);
        }
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            printer(i, 0, 0, 0, 0, data);
        }
    }
}

void macro_rainbow(uint32_t from, uint32_t to, float s, float i, uint32_t t, uint32_t t_period, int easingIndex, void *data, printer printer)
{
    // TODO: Figure out way to make it not "overflow" with distracting colours
    //          Make it so that it ends at 300, but shows the other 60 on the other end
    //          So that the "huePerLed" is less, but it moves faster over the spectrum, so "0" reach the top at the end of the animation

    uint32_t len = (to - from);

    float p = executeEasing(easingIndex, (t % t_period) / (float)t_period);
    float offsetHue = (360.0) * p;

    float hue_start = (360.0 * 100.0);
    float hue = hue_start;
    float huePerLed = (360.0 / (float)len);

    uint8_t rgbw[4];
    for (int i = from; i < to; i++)
    {
        float totalHue = (hue - offsetHue);

        int ledH = ((int)totalHue) % 360;

        // TODO: Add the intensity increment!

        hsi2rgbw(ledH, s, i, rgbw);
        printer(i, rgbw[0], rgbw[1], rgbw[2], rgbw[3], data);

        hue -= huePerLed;
    }
}

void *pattern_rainbow_wave_data(uint16_t len, float intensity)
{
    void *data = malloc((sizeof(int) * 2) + sizeof(float) * 2);
    int *intData = (int *)data;
    float *floatData = data + (sizeof(int) * 2);
    intData[0] = randint(getEasingCount());
    intData[sizeof(int)] = MAX(1000, 15000 - (randint(10000) * intensity) - (randint(10000) * intensity));
    floatData[0] = 0.6 + (0.4 * ((randint(10000) / (float)10000) * intensity));
    floatData[sizeof(float)] = 0.2 + (0.5 * ((randint(10000) / (float)10000) * intensity));
    return data;
}

void pattern_rainbow_wave(uint16_t len, uint32_t t, void *data, printer printer)
{
    int *intData = (int *)data;
    float *floatData = (float *)(data + (sizeof(int) * 2));
    int easingIndex = ((int *)data)[0];
    int period = ((int *)data)[sizeof(int)];
    float s = floatData[0];
    float i = floatData[sizeof(float)];
    macro_rainbow(0, len, s, i, t, period, easingIndex, data, printer);
}

struct pattern_sparkle_data_instance
{
    float chanceToLightUp;
    float chanceToGoOut;
    bool *values;
};

void *pattern_sparkle_data_destroyer(void *data)
{
    if (data)
    {
        struct pattern_sparkle_data_instance *instance = data;
        free(instance->values);
        instance->values = 0;
        free(data);
    }
}

void *pattern_sparkle_data(uint16_t len, float intensity)
{
    struct pattern_sparkle_data_instance *instance = malloc(sizeof(struct pattern_sparkle_data_instance));

    instance->chanceToLightUp = randint_probability(10, 10000000, 1 - intensity) / (float)1000000000;
    instance->chanceToGoOut = randint_probability(1000, 5000, 1 - intensity) / (float)100000;
    instance->values = calloc(len, sizeof(bool));

    return instance;
}

void pattern_sparkle(uint16_t len, uint32_t t, void *data, printer printer)
{
    struct pattern_sparkle_data_instance *instance = data;

    const int v = 1000000;
    int chanceToLightUpInt = v * instance->chanceToLightUp;
    int chanceToGoOutInt = v * instance->chanceToGoOut;

    for (int i = 0; i < len; i++)
    {
        int ptrAddress = sizeof(bool) * i;
        if (randint(v) < chanceToLightUpInt)
        {
            instance->values[ptrAddress] = true;
        }
        else if (randint(v) < chanceToGoOutInt)
        {
            instance->values[ptrAddress] = false;
        }

        uint8_t color = instance->values[ptrAddress] ? 0xff : 0;
        printer(i, color, color, color, color, data);
    }
}

void *noop(uint16_t len, float intensity)
{
    return malloc(0);
}

void *default_destroyer(void *data)
{
    if (data)
    {
        free(data);
    }
}

const struct
{
    pattern pat;
    pattern_data_creator creator;
    pattern_data_destroyer destroyer;
} pattern_table[] = {

    {pattern_random, pattern_random_data, pattern_random_data_destroyer},

    {pattern_sparkle, pattern_sparkle_data, pattern_sparkle_data_destroyer},

    // Should "sweep" more -- from mid brightness to max brightness, and pulse fast, to give feeling of weird shadows in room
    {pattern_snakes, pattern_snakes_data, default_destroyer},

    {pattern_fade_between, pattern_fade_between_data, default_destroyer},

    {pattern_red_bouncer_smooth, pattern_red_bouncer_smooth_data, default_destroyer},

    {pattern_fill_sway, pattern_fill_sway_data, default_destroyer},
    {pattern_rainbow_wave, pattern_rainbow_wave_data, default_destroyer},

    {pattern_strobe, noop, default_destroyer},
};

// TODO: Fix pattern factory by reading https://github.com/huawenyu/Design-Patterns-in-C/tree/master/auto-gen/factory/simple_factory

inline int getPatternCount()
{
    return count_of(pattern_table);
}

pattern getPattern(int patternIndex)
{
    return pattern_table[patternIndex].pat;
}

pattern_data_creator getPatternCreator(int patternIndex)
{
    return pattern_table[patternIndex].creator;
}

pattern_data_destroyer getPatternDestroyer(int patternIndex)
{
    return pattern_table[patternIndex].destroyer;
}

void pattern_put_pixel_default(uint16_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w, void *data)
{
    put_pixel(index, r, g, b, w);
}

void pattern_execute(int patternIndex, uint16_t len, uint32_t t, void *data)
{
    if (state.nextPatternIndex >= 0)
    {
        pattern_update_data(state.nextPatternIndex, state.nextIntensity);
        state.nextPatternIndex = -1;
        state.nextIntensity = -1;
    }

    // Execute the current pattern inside state
    if (!state.disabled)
    {
        pattern_table[patternIndex].pat(len, t, data, pattern_put_pixel_default);
    }
    else
    {
        setAll(len, 0, 0, 0, 0, data, pattern_put_pixel_default);
    }
}

void pattern_update_data(int patternIndex, float intensity)
{
    // Destroy/free any previous memory allocations
    pattern_table[patternIndex].destroyer(state.patternData);
    state.patternData = 0;

    // Create the new pattern data
    void *data = pattern_table[patternIndex].creator(LED_COUNT, intensity); // TODO: Do not use LED_COUNT, could be different strips

    // Set to the new (or same) pattern index, and new data
    state.patternIndex = patternIndex;
    state.patternData = data;
}