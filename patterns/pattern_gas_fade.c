#include "../patterns.h"

#define BUCKET_SIZE_COLORS 25

typedef struct data_struct
{
    RgbwaColor color_bag[BUCKET_SIZE_COLORS];
    int color_bag_length;

    double frequency_multiplier_1;
    double frequency_multiplier_2;
    double frequency_multiplier_3;

    double len;
} data_struct;

typedef struct frame_struct
{
    int offset;
} frame_struct;

static int set_fire(data_struct *data)
{
    data->color_bag[0] = hsia2rgbwa(&(HsiaColor){0, 0, 0, 0.0});
    data->color_bag[1] = hsia2rgbwa(&(HsiaColor){12, 0, 0, 0});
    data->color_bag[2] = hsia2rgbwa(&(HsiaColor){12, 0.84, 0.61, 1});
    data->color_bag[3] = hsia2rgbwa(&(HsiaColor){26, 0.87, 0.63, 1});
    data->color_bag[4] = hsia2rgbwa(&(HsiaColor){50, 0.95, 0.65, 1});
    data->color_bag[5] = hsia2rgbwa(&(HsiaColor){55, 0.75, 0.75, 1});
    data->color_bag[6] = hsia2rgbwa(&(HsiaColor){55, 0, 1, 1});
    data->color_bag[7] = hsia2rgbwa(&(HsiaColor){0, 0, 1, 1});

    for (int i = 0; i < 8; i++)
    {
        data->color_bag[i].w = 0;
    }

    return 8;
}

static int set_grayscale(data_struct *data)
{
    data->color_bag[0] = hsia2rgbwa(&(HsiaColor){0, 0, 0, 0.0});
    data->color_bag[1] = hsia2rgbwa(&(HsiaColor){0, 0, 0, 0});
    data->color_bag[2] = hsia2rgbwa(&(HsiaColor){0, 0, 1, 0.2});
    data->color_bag[3] = hsia2rgbwa(&(HsiaColor){0, 0, 1, 0.4});
    data->color_bag[4] = hsia2rgbwa(&(HsiaColor){0, 0, 1, 0.6});
    data->color_bag[5] = hsia2rgbwa(&(HsiaColor){0, 0, 1, 0.8});
    data->color_bag[6] = hsia2rgbwa(&(HsiaColor){0, 0, 1, 0.9});
    data->color_bag[7] = hsia2rgbwa(&(HsiaColor){0, 0, 1, 1});

    return 8;
}

// TODO: Something is wrong with this pattern! There is a memory leak! Remove the color bag, or redo it so it is *simpler*. Maybe just make it an array of size 25, but don't use all the colors?

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = calloc(1, sizeof(data_struct));

    data->len = len;

    data->frequency_multiplier_1 = (40 + randint(60)) / (double)1000;   // 0.07 is a good value
    data->frequency_multiplier_2 = (180 + randint(150)) / (double)1000; // 0.22 is a good value
    data->frequency_multiplier_3 = (250 + randint(220)) / (double)1000; // 0.31 is a good value

    // HsiaColor colors[BUCKET_SIZE_COLORS];
    /*for (int i = 0; i < BASE_COLOR_LENGTH; i++)
    {
        // Initialize as -1 to say that this is not set.
        colors[i].a = -1;
    }
    */

    // int interpolations;
    if (randint_weighted_towards_max(0, 100, intensity) > 75)
    {
        if (randint_weighted_towards_max(0, 100, intensity) > 50)
        {
            // interpolations = 0;
            data->color_bag_length = set_grayscale(data);
        }
        else
        {
            int randomColorCount = randint_weighted_towards_max(2, 5, intensity);
            for (int i = 0; i < randomColorCount; i++)
            {
                // TODO: Make this less random and a bit more beautiful.
                data->color_bag[i] = (RgbwaColor){randint(255), randint(255), randint(255), 0, RGB_ALPHA_MAX};
            }

            data->color_bag_length = randomColorCount;

            // interpolations = randint_weighted_towards_min(0, 5, intensity);
        }
    }
    else
    {
        // This is for "fire" -- but we should be able to handle any "sparkle" with a disappearing effect
        data->color_bag_length = set_fire(data);
        // interpolations = randint_weighted_towards_min(0, 5, intensity);
    }

    // int base_bag_length = 0;
    // data->color_bag_length =

    /*
     base_bag_length + ((base_bag_length - 1) * interpolations);
    data->color_bag = calloc(data->color_bag_length, sizeof(HsiaColor));

    int stepSize = (interpolations + 1);
    for (int c = 0, b = 0; c < base_bag_length; c++, b += stepSize)
    {
        data->color_bag[sizeof(HsiaColor) * b] = colors[c];
        if (c == base_bag_length - 1)
        {
            break;
        }

        HsiaColor current = colors[c];
        HsiaColor next = colors[c + 1];

        for (int n = 1; n <= interpolations; n++)
        {
            float p = (n / (float)(interpolations + 1));
            HsiaColor interpolated = math_average_hsia_lerp(&current, &next, p);
            data->color_bag[(sizeof(HsiaColor) * b) + (sizeof(HsiaColor) * n)] = interpolated; // SEGMENTATION FAULT???
        }
    }
    */

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    // The offset should be in integral steps according to the length, to avoid flicker
    frame->offset = (-1 * (t / 1000.0)) * 10.0;

    return frame;
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
    data_struct *data = args->dataPtr;
    frame_struct *frame = args->framePtr;

    // TODO: Figure out a way to cache the sin() calculations!
    // (Could be done by calculating ### points, and then interpolating between them here, like with the colors bag)
    // Can use a method in math.c, and if precomputation is enabled, then use it. The accuracy should be DYNAMIC
    int virtualIndex = (args->i + frame->offset);

    double wave_a_radian = virtualIndex * data->frequency_multiplier_1;
    double wave_b_radian = virtualIndex * data->frequency_multiplier_2;
    double wave_c_radian = virtualIndex * data->frequency_multiplier_3;

    double wave_a = fabs(sin(wave_a_radian));
    double wave_b = fabs(sin(wave_b_radian));
    double wave_c = fabs(sin(wave_c_radian));
    double y = ((wave_a + wave_b + wave_c) * (1 - (args->i / data->len))) / 3.0;

    // Find the closest color in the bag.
    int bagIndex = (int)round((data->color_bag_length - 1) * y);
    return data->color_bag[bagIndex];
}

void pattern_register_gas_fade()
{
    pattern_register("gas_fade", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){0.5});
}