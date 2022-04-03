#include "../patterns.h"

typedef struct data_struct
{
    HsiaColor colors[10];
    int time_per_color;
} data_struct;

typedef struct frame_struct
{
    HsiaColor hsi;
} frame_struct;

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;
    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    // TODO: Create a bunch of data for different kinds of fades
    data_struct *data = malloc(sizeof(data_struct));

    //data->time_per_color = 3000 + randint_weighted_towards_min(0, 30000, intensity);
    data->time_per_color = 1000; // + randint_weighted_towards_min(0, 30000, intensity);

    // We use the same saturation and intensity for all different hues for this pattern.
    float hsi_s = MIN(1, 0.65 + 0.35 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000));
    float hsi_i = MIN(1, 0.4 + 0.5 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000));

    for (int i = 0; i < 10; i++)
    {
        data->colors[i] = (HsiaColor){randint(360), hsi_s, hsi_i, 1};
    }

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    int colorIndex1 = (t / data->time_per_color) % 10;
    int colorIndex2 = (colorIndex1 + 1) % 10;

    HsiaColor hsi_from = data->colors[colorIndex1];
    HsiaColor hsi_to = data->colors[colorIndex2];

    float pOfColor = ((t % data->time_per_color) / (float) data->time_per_color);

    float distance = math_shortest_hue_distance_lerp(hsi_from.h, hsi_to.h, pOfColor);
    int hue = floorf(hsi_from.h + distance);
    if (hue < 0)
    {
        hue = 360 + hue;
    }

    frame->hsi = (HsiaColor) {
        hue,
        hsi_from.s,
        hsi_from.i,
        hsi_from.a
    };

    return frame;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, Printer *printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    printer->print(i, &frame->hsi, printer);
}

void pattern_register_fade_between()
{
    pattern_register("fade", executor, data_creator, data_destroyer, frame_creator, NULL, (PatternOptions){1, 0, true});
}