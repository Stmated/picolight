#include "../patterns.h"

typedef struct data_struct
{
    PatternModule *snakeModule;

    void *snake1data;
    void *snake2data;
    void *snake3data;

} data_struct;

typedef struct frame_struct
{
    void *frame1;
    void *frame2;
    void *frame3;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->snakeModule = pattern_get_by_name("snake");
    data->snake1data = data->snakeModule->creator(len, intensity * 4);
    data->snake2data = data->snakeModule->creator(len, intensity * 2);
    data->snake3data = data->snakeModule->creator(len, intensity);

    return data;
}

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;

    data->snakeModule->destroyer(data->snake1data);
    data->snake1data = NULL;
    data->snakeModule->destroyer(data->snake2data);
    data->snake2data = NULL;
    data->snakeModule->destroyer(data->snake3data);
    data->snake3data = NULL;
    free(dataPtr);
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    frame->frame1 = data->snakeModule->frameCreator(len, t, data->snake1data);
    frame->frame2 = data->snakeModule->frameCreator(len, t, data->snake2data);
    frame->frame3 = data->snakeModule->frameCreator(len, t, data->snake3data);

    return frame;
}

static void frame_destroyer(void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    data->snakeModule->frameDestroyer(data->snake1data, frame->frame1);
    data->snakeModule->frameDestroyer(data->snake2data, frame->frame2);
    data->snakeModule->frameDestroyer(data->snake3data, frame->frame3);
    free(framePtr);
}

// TODO: Remove this whole pattern! Instead make it somehow able to inherit from "random" but tell it that the "random" has to be 3 snakes. Then move code from Random into a "Composition" pattern helper of sorts

static inline HsiaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    HsiaColor a = data->snakeModule->executor(&(ExecutorArgs){args->i, data->snake1data, frame->frame1});
    HsiaColor b = data->snakeModule->executor(&(ExecutorArgs){args->i, data->snake2data, frame->frame2});
    HsiaColor c = data->snakeModule->executor(&(ExecutorArgs){args->i, data->snake3data, frame->frame3});

    HsiaColor blend_a_b = math_average_hsia(&a, &b);
    return math_average_hsia(&blend_a_b, &c);
}

void pattern_register_snakes()
{
    pattern_register("snakes", executor, data_creator, data_destroyer, frame_creator, frame_destroyer, (PatternOptions){1});
}