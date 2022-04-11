#include "../patterns.h"

typedef struct data_struct
{
    HsiaColor *color_bag;
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

static void data_destroyer(void *dataPtr)
{
    data_struct *data = dataPtr;
    free(data->color_bag);
    free(dataPtr);
}

static void set_fire(HsiaColor colors[10])
{
    colors[0] = (HsiaColor){0, 0, 0, 0.0};
    colors[1] = (HsiaColor){12, 0, 0, 0};
    colors[2] = (HsiaColor){12, 0.84, 0.91, 1};
    colors[3] = (HsiaColor){26, 0.87, 0.93, 1};
    colors[4] = (HsiaColor){50, 0.95, 0.95, 1};
    colors[5] = (HsiaColor){55, 0.75, 0.95, 1};
    colors[6] = (HsiaColor){55, 0, 1, 1};
    colors[7] = (HsiaColor){0, 0, 1, 1};
}

static void set_grayscale(HsiaColor colors[10])
{
    colors[0] = (HsiaColor){0, 0, 0, 0.0};
    colors[1] = (HsiaColor){0, 0, 0, 0};
    colors[2] = (HsiaColor){0, 0, 1, 0.2};
    colors[3] = (HsiaColor){0, 0, 1, 0.4};
    colors[4] = (HsiaColor){0, 0, 1, 0.6};
    colors[5] = (HsiaColor){0, 0, 1, 0.8};
    colors[6] = (HsiaColor){0, 0, 1, 0.9};
    colors[7] = (HsiaColor){0, 0, 1, 1};
}

#define BASE_COLOR_LENGTH 10

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    data->len = len;

    data->frequency_multiplier_1 = (40 + randint(60)) / (double)1000;   // 0.07 is a good value
    data->frequency_multiplier_2 = (180 + randint(150)) / (double)1000; // 0.22 is a good value
    data->frequency_multiplier_3 = (250 + randint(220)) / (double)1000; // 0.31 is a good value

    HsiaColor colors[BASE_COLOR_LENGTH];
    for (int i = 0; i < BASE_COLOR_LENGTH; i++)
    {
        // Initialize as -1 to say that this is not set.
        colors[i].a = -1;
    }

    int interpolations;
    if (randint_weighted_towards_max(0, 100, intensity) > 50)
    {
        if (randint_weighted_towards_max(0, 100, intensity) > 50)
        {
            interpolations = 0;
            set_grayscale(colors);
        }
        else
        {
            int randomColorCount = randint_weighted_towards_max(2, 5, intensity);
            for (int i = 0; i < randomColorCount; i++)
            {
                // TODO: Make this less random and a bit more beautiful.
                colors[i] = (HsiaColor){randint(360), 1, 1, 1 - (i / (float) randomColorCount)};
            }

            interpolations = randint_weighted_towards_min(0, 5, intensity);
        }
    }
    else
    {
        // This is for "fire" -- but we should be able to handle any "sparkle" with a disappearing effect
        set_fire(colors);
        interpolations = randint_weighted_towards_min(0, 5, intensity);
    }

    int base_bag_length = 0;
    for (int i = 0; i < BASE_COLOR_LENGTH; i++)
    {
        if (colors[i].a >= 0)
        {
            base_bag_length++;
        }
    }

    data->color_bag_length = base_bag_length + ((base_bag_length - 1) * interpolations);
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
            data->color_bag[(sizeof(HsiaColor) * b) + (sizeof(HsiaColor) * n)] = interpolated;
        }
    }

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

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, Printer *printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    // TODO: Figure out a way to cache the sin() calculations!
    // (Could be done by calculating ### points, and then interpolating between them here, like with the colors bag)
    // Can use a method in math.c, and if precomputation is enabled, then use it. The accuracy should be DYNAMIC
    double wave_a = fabs(sin((i + frame->offset) * data->frequency_multiplier_1));
    double wave_b = fabs(sin((i + frame->offset) * data->frequency_multiplier_2));
    double wave_c = fabs(sin((i + frame->offset) * data->frequency_multiplier_3));
    double y = ((wave_a + wave_b + wave_c) * (1 - (i / data->len))) / 3.0;

    // Find the closest color in the bag.
    int bagIndex = (int)round((data->color_bag_length - 1) * y);
    printer->print(i, &data->color_bag[sizeof(HsiaColor) * bagIndex], printer);
}

void pattern_register_gas_fade()
{
    pattern_register("gas_fade", executor, data_creator, data_destroyer, frame_creator, NULL, (PatternOptions){1});
}