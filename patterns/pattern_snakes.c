#include "../patterns.h"

typedef struct data_struct
{
    data_pixel_blending_struct base;

    void *snake1data;
    void *snake2data;
    void *snake3data;

    PatternModule *snakeModule;

} data_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = (data_struct *)dataPtr;
    PatternModule *module = getPatternByName("snake");

    module->destroyer(data->snake1data);
    data->snake1data = NULL;
    module->destroyer(data->snake2data);
    data->snake2data = NULL;
    module->destroyer(data->snake3data);
    data->snake3data = NULL;
    free(data->base.pixels);
    data->base.pixels = NULL;
    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->snakeModule = getPatternByName("snake");
    data->snake1data = data->snakeModule->creator(len, intensity * 4);
    data->snake2data = data->snakeModule->creator(len, intensity * 2);
    data->snake3data = data->snakeModule->creator(len, intensity);
    data->base.pixels = calloc(3, sizeof(HsiColor));

    return data;
}

TODO: This will write to the wrong data pointer if Snakes is ran inside Random! The parentDataPtr was not a good idea!
TODO: Redo so that we send a struct with all the common arguments, so we do not send so many
TODO: Figure out how to handle nested Random -> Snakes
TODO: Figure out how to make it so that if inside Random, that we do not re-create the cycleData every time! It is too slow!

static void executor(uint16_t start, uint16_t stop, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = (data_struct *)dataPtr;

    void *snake1CyclePtr = data->snakeModule->cycleCreator(len, t, data->snake1data);
    void *snake2CyclePtr = data->snakeModule->cycleCreator(len, t, data->snake2data);
    void *snake3CyclePtr = data->snakeModule->cycleCreator(len, t, data->snake3data);

    for (int i = start; i < stop; i++)
    {
        data->base.stepIndex = 0;
        data->snakeModule->executor(i, i + 1, len, t, data->snake1data, snake1CyclePtr, data, pattern_printer_set);
        data->snakeModule->executor(i, i + 1, len, t, data->snake2data, snake2CyclePtr, data, pattern_printer_set);
        data->snakeModule->executor(i, i + 1, len, t, data->snake3data, snake3CyclePtr, data, pattern_printer_set);

        HsiColor c = math_average_hsi(data->base.pixels, 3);
        printer(i, &c, dataPtr, dataPtr); // Parent as ourself, since we are just a virtual pattern -- THIS IS WRONG! IT ONLY WORKS FOR RANDOM, NOT NESTED!
    }

    data->snakeModule->cycleDestroyer(snake1CyclePtr);
    data->snakeModule->cycleDestroyer(snake2CyclePtr);
    data->snakeModule->cycleDestroyer(snake3CyclePtr);
}

void pattern_register_snakes()
{
    pattern_register("snakes", executor, data_creator, data_destroyer, NULL, NULL, (PatternOptions){1});
}