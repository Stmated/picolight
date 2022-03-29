#include "../patterns.h"

typedef struct data_struct
{
    data_pixel_blending_struct base;

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
        module->destroyer(data->snake2data);
        module->destroyer(data->snake3data);
        free(data->base.pixels);
    }
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    PatternModule *module = getPatternByName("snake");

    data->snake1data = module->creator(len, intensity * 4);
    data->snake2data = module->creator(len, intensity * 2);
    data->snake3data = module->creator(len, intensity);
    data->base.pixels = calloc(3, sizeof(HsiColor));

    return data;
}

static void executor(uint16_t start, uint16_t stop, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer)
{
    data_struct *data = (data_struct *)dataPtr;
    PatternModule *module = getPatternByName("snake");

    void *snake1CyclePtr = module->cycleCreator(len, t, data->snake1data);
    void *snake2CyclePtr = module->cycleCreator(len, t, data->snake2data);
    void *snake3CyclePtr = module->cycleCreator(len, t, data->snake3data);

    //HsiColor *pixels = calloc(stop - start, sizeof(HsiColor));

    for (int i = start; i < stop; i++)
    {
        data->base.stepIndex = 0;
        module->executor(i, i + 1, len, t, data->snake1data, snake1CyclePtr, pattern_printer_merging);
        module->executor(i, i + 1, len, t, data->snake2data, snake2CyclePtr, pattern_printer_merging);
        module->executor(i, i + 1, len, t, data->snake3data, snake3CyclePtr, pattern_printer_merging);

        //HsiColor c1 = data->base.pixels[sizeof(HsiColor) * 0];
        //HsiColor c2 = data->base.pixels[sizeof(HsiColor) * 1];
        //HsiColor c3 = data->base.pixels[sizeof(HsiColor) * 2];

        //HsiColor c = {
        //    MAX(c1.h, MAX(c2.h, c3.h)),
        //    MAX(c1.s, MAX(c2.s, c3.s)),
        //    MAX(c1.i, MAX(c2.i, c3.i))
        //};

        HsiColor c = math_average_hsi(data->base.pixels, 3);

        //pixels[sizeof(HsiColor) * (i - start)] = c;
        //printer(i, &c, dataPtr);

        printer(i, &c, dataPtr);
    }

    //for (int i = start; i < stop; i++)
    //{
        // Now let's send the data to the original printer
    //    printer(i, &pixels[sizeof(HsiColor) * (i - start)], dataPtr);
    //}
    
    //free(pixels);

    module->cycleDestroyer(snake1CyclePtr);
    module->cycleDestroyer(snake2CyclePtr);
    module->cycleDestroyer(snake3CyclePtr);

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
    pattern_register("snakes", executor, data_creator, data_destroyer, NULL, NULL, &(PatternOptions){1});
}