#include "pattern.h"

typedef struct data_struct
{
    HsiaColor colors[10];
    int time_per_color;
} data_struct;

typedef struct frame_struct
{
    RgbwaColor rgbwa;
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

    data->time_per_color = 3000 + randint_weighted_towards_min(0, 30000, intensity);

    // We use the same saturation and intensity for all different hues for this pattern.
    float hsi_s = MIN(1, 0.65 + 0.35 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000));
    float hsi_i = MIN(1, 0.4 + 0.5 * (randint_weighted_towards_max(0, 10000, intensity) / (float)10000));

    for (int i = 0; i < 10; i++)
    {
        data->colors[i] = (HsiaColor){randint(360), hsi_s, hsi_i, 1};
    }

    return data;
}

static void *frame_allocator(uint16_t len, void *dataPtr)
{
    return calloc(1, sizeof(frame_struct));
}

static void frame_creator(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    int colorIndex1 = (t / data->time_per_color) % 10;
    int colorIndex2 = (colorIndex1 + 1) % 10;

    HsiaColor hsi_from = data->colors[colorIndex1];
    HsiaColor hsi_to = data->colors[colorIndex2];

    float pOfColor = ((t % data->time_per_color) / (float) data->time_per_color);
    int hue = math_hue_lerp(hsi_from.h, hsi_to.h, pOfColor);

    frame->rgbwa = hsia2rgbwa(
        hue,
        hsi_from.s,
        hsi_from.i,
        hsi_from.a
    );
}

void pattern_register_fade_between()
{
    pattern_register("fade", NULL, data_creator, data_destroyer, frame_allocator, frame_creator, NULL, (PatternOptions){1, 0, true});
}