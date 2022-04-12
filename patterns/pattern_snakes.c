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

static void data_destroyer(void *dataPtr)
{
    data_struct *data = (data_struct *)dataPtr;

    data->snakeModule->destroyer(data->snake1data);
    data->snake1data = NULL;
    data->snakeModule->destroyer(data->snake2data);
    data->snake2data = NULL;
    data->snakeModule->destroyer(data->snake3data);
    data->snake3data = NULL;
    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->snakeModule = pattern_get_by_name("snake");
    data->snake1data = data->snakeModule->creator(len, intensity * 4);
    data->snake2data = data->snakeModule->creator(len, intensity * 2);
    data->snake3data = data->snakeModule->creator(len, intensity);

    return data;
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

typedef struct SnakesPrinter
{
    Printer override;
    int stepIndex;
    HsiaColor pixels[3];

} SnakesPrinter;

static inline void snakes_printer(uint16_t index, HsiaColor *c, Printer *printer)
{
    SnakesPrinter *ourPrinter = (void *)printer;
    ourPrinter->pixels[ourPrinter->stepIndex] = *c;
    ourPrinter->stepIndex++;
}

// TODO: Remove this whole pattern! Instead make it somehow able to inherit from "random" but tell it that the "random" has to be 3 snakes. Then move code from Random into a "Composition" pattern helper of sorts

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, Printer *printer)
{
    data_struct *data = (data_struct *)dataPtr;
    frame_struct *frame = framePtr;

    SnakesPrinter bufferingPrinter = {{snakes_printer}};

    data->snakeModule->executor(i, data->snake1data, frame->frame1, (void *)&bufferingPrinter);
    data->snakeModule->executor(i, data->snake2data, frame->frame2, (void *)&bufferingPrinter);
    data->snakeModule->executor(i, data->snake3data, frame->frame3, (void *)&bufferingPrinter);

    HsiaColor step1 = math_average_hsia(&bufferingPrinter.pixels[0], &bufferingPrinter.pixels[1]);
    HsiaColor c = math_average_hsia(&step1, &bufferingPrinter.pixels[2]);

    printer->print(i, &c, printer);
}

void pattern_register_snakes()
{
    pattern_register("snakes", executor, data_creator, data_destroyer, frame_creator, frame_destroyer, (PatternOptions){1});
}