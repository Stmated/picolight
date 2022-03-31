#include "../patterns.h"

typedef struct data_struct
{
    data_pixel_blending_struct base;

    PatternModule *snakeModule;

    void *snake1data;
    void *snake2data;
    void *snake3data;

} data_struct;

typedef struct frame_struct
{
    void *snake1frame;
    void *snake2frame;
    void *snake3frame;

} frame_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = (data_struct *)dataPtr;

    data->snakeModule->destroyer(data->snake1data);
    data->snake1data = NULL;
    data->snakeModule->destroyer(data->snake2data);
    data->snake2data = NULL;
    data->snakeModule->destroyer(data->snake3data);
    data->snake3data = NULL;
    free(data->base.pixels);
    data->base.pixels = NULL;
    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->snakeModule = pattern_get_by_name("snake");
    data->snake1data = data->snakeModule->creator(len, intensity * 4);
    data->snake2data = data->snakeModule->creator(len, intensity * 2);
    data->snake3data = data->snakeModule->creator(len, intensity);
    data->base.pixels = calloc(3, sizeof(HsiaColor));

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    frame->snake1frame = data->snakeModule->frameCreator(len, t, data->snake1data);
    frame->snake2frame = data->snakeModule->frameCreator(len, t, data->snake2data);
    frame->snake3frame = data->snakeModule->frameCreator(len, t, data->snake3data);

    return frame;
}

static void frame_destroyer(void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    data->snakeModule->frameDestroyer(data->snake1data, frame->snake1frame);
    data->snakeModule->frameDestroyer(data->snake2data, frame->snake2frame);
    data->snakeModule->frameDestroyer(data->snake3data, frame->snake3frame);
    free(framePtr);
}

static inline void snakes_printer(uint16_t index, HsiaColor *c, void *dataPtr, void *parentDataPtr)
{
    // Important to use the parent data here, since "dataPtr" is from the sub-pattern
    data_struct *data = parentDataPtr;

    data->base.pixels[sizeof(HsiaColor) * data->base.stepIndex] = *c;
    data->base.stepIndex++;
}

// TODO: This will write to the wrong data pointer if Snakes is ran inside Random! The parentDataPtr was not a good idea!
// TODO: Redo so that we send a struct with all the common arguments, so we do not send so many
// TODO: Figure out how to handle nested Random -> Snakes
// TODO: Figure out how to make it so that if inside Random, that we do not re-create the frameData every time! It is too slow!

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, void *parentDataPtr, PatternPrinter printer)
{
    data_struct *data = (data_struct *)dataPtr;
    frame_struct *frame = framePtr;

    data->base.stepIndex = 0;
    data->snakeModule->executor(i, data->snake1data, frame->snake1frame, data, snakes_printer);
    data->snakeModule->executor(i, data->snake2data, frame->snake2frame, data, snakes_printer);
    data->snakeModule->executor(i, data->snake3data, frame->snake3frame, data, snakes_printer);

    // TODO: Can we somehow skip doing the averaging here if not needed? Could it instead be done by the parent printer if there is one?
    HsiaColor c = math_average_hsia(data->base.pixels, 3);

    // Parent as ourself, since we are just a virtual pattern
    // THIS IS WRONG! IT ONLY WORKS FOR RANDOM, NOT NESTED!
    printer(i, &c, dataPtr, dataPtr);
}

void pattern_register_snakes()
{
    pattern_register("snakes", executor, data_creator, data_destroyer, frame_creator, frame_destroyer, (PatternOptions){1});
}