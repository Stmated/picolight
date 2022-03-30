#include "../patterns.h"

typedef struct data_struct
{
    data_pixel_blending_struct base;

    void *snake1data;
    void *snake2data;
    void *snake3data;

    PatternModule *snakeModule;

} data_struct;

typedef struct cycle_struct
{
    void *cycle1;
    void *cycle2;
    void *cycle3;

} cycle_struct;

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

static void *cycle_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    cycle_struct *cycle = calloc(1, sizeof(cycle_struct));

    cycle->cycle1 = data->snakeModule->cycleCreator(len, t, data->snake1data);
    cycle->cycle2 = data->snakeModule->cycleCreator(len, t, data->snake2data);
    cycle->cycle3 = data->snakeModule->cycleCreator(len, t, data->snake3data);

    return cycle;
}

static void cycle_destroyer(void *dataPtr, void *cyclePtr)
{
    data_struct *data = dataPtr;
    cycle_struct *cycle = cyclePtr;

    data->snakeModule->cycleDestroyer(data->snake1data, cycle->cycle1);
    data->snakeModule->cycleDestroyer(data->snake2data, cycle->cycle2);
    data->snakeModule->cycleDestroyer(data->snake3data, cycle->cycle3);

    free(cyclePtr);
}

// TODO: This will write to the wrong data pointer if Snakes is ran inside Random! The parentDataPtr was not a good idea!
// TODO: Redo so that we send a struct with all the common arguments, so we do not send so many
// TODO: Figure out how to handle nested Random -> Snakes
// TODO: Figure out how to make it so that if inside Random, that we do not re-create the cycleData every time! It is too slow!

static inline void executor(uint16_t i, void *dataPtr, void *cyclePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = (data_struct *)dataPtr;
    cycle_struct *cycle = cyclePtr;

    data->base.stepIndex = 0;
    data->snakeModule->executor(i, data->snake1data, cycle->cycle1, data, pattern_printer_set);
    data->snakeModule->executor(i, data->snake2data, cycle->cycle2, data, pattern_printer_set);
    data->snakeModule->executor(i, data->snake3data, cycle->cycle3, data, pattern_printer_set);

    // TODO: Can we somehow skip doing the averaging here if not needed? Could it instead be done by the parent printer if there is one?
    HsiColor c = math_average_hsi(data->base.pixels, 3);
    printer(i, &c, dataPtr, dataPtr); // Parent as ourself, since we are just a virtual pattern -- THIS IS WRONG! IT ONLY WORKS FOR RANDOM, NOT NESTED!
}

void pattern_register_snakes()
{
    pattern_register("snakes", executor, data_creator, data_destroyer, cycle_creator, cycle_destroyer, (PatternOptions){1});
}