#include "../patterns.h"
#include "../led_math.h"

typedef struct data_struct
{
    data_pixels_struct base;

    void *snake1data;
    void *snake2data;
    void *snake3data;

} data_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = (data_struct *)dataPtr;

    PatternModule *module = getPatternByName("snake");

    if (data->snake1data)
    {
        module->destroyer(data->snake1data);
        data->snake1data = NULL;
    }

    if (data->snake2data)
    {
        module->destroyer(data->snake2data);
        data->snake2data = NULL;
    }

    if (data->snake3data)
    {
        module->destroyer(data->snake3data);
        data->snake3data = NULL;
    }

    if (data->base.pixels)
    {
        free(data->base.pixels);
        data->base.pixels = NULL;
    }
}

void *data_creator(uint16_t len, float intensity)
{
    struct data_struct *data = malloc(sizeof(struct data_struct));

    PatternModule *module = getPatternByName("snake");

    data->snake1data = module->creator(len, intensity * 4);
    data->snake2data = module->creator(len, intensity * 2);
    data->snake3data = module->creator(len, intensity);
    data->base.pixels = calloc(len, sizeof(HsiColor));

    return data;
}

/*
void pattern_printer_merging(uint16_t index, HsiColor *c, void *dataPtr)
{
    data_struct *data = (data_struct *)dataPtr;

    //default

    pattern_printer_default(index, c, dataPtr);
}
*/

void executor(uint16_t len, uint32_t t, void *dataPtr, PatternPrinter printer)
{
    data_struct *data = (data_struct *)dataPtr;
    PatternModule *module = getPatternByName("snake");

    // TODO: Abstract out the aggregating printer from Random pattern, and use it here as well.
    // TODO: Can we create different types of printers, which can stream one pixel through different executors?

    data->base.progress = 1;
    module->executor(len, t, data->snake1data, pattern_printer_merging);
    data->base.progress = 0.5;
    //data->base.progress = 0.5;
    module->executor(len, t, data->snake2data, pattern_printer_merging);
    //data->base.progress = 0.5;
    module->executor(len, t, data->snake3data, pattern_printer_merging);

    for (int i = 0; i < len; i++)
    {
        // Now let's send the data to the original printer
        printer(i, &data->base.pixels[i], data);
    }

    /*
    struct data_struct *instance = data;

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
    }

    for (int i = 0; i < len; i++)
    {
        printer(i, &colors[i], data);
    }
    */
}

void pattern_register_snakes()
{
    pattern_register("snakes", executor, data_creator, data_destroyer, &(PatternOptions){1});
}