#include "led_math.h"

#ifdef MATH_PRECOMPUTE
static float lookup_h2cos[HSI_H_MAX] = {0};
static float lookup_h2sin[HSI_H_MAX] = {0};
static float lookup_h2cos_60h[HSI_H_MAX] = {0};
static float lookup_h2cos_h120[HSI_H_MAX] = {0};
static float lookup_h2cos_180h[HSI_H_MAX] = {0};
static float lookup_h2cos_h240[HSI_H_MAX] = {0};
static float lookup_h2cos_300h[HSI_H_MAX] = {0};
#endif

void math_precompute()
{
#ifdef MATH_PRECOMPUTE
    for (int h = HSI_H_MIN; h < HSI_H_MAX; h++)
    {
        float r = DEG_TO_RAD(h); // h * (M_PI / 180.0);
        // float cos_h = cosf(r);
        // float cos_1047_h = cosf(1.047196667 - r);
        // float div = cos_h / cos_1047_h;

        lookup_h2sin[h] = sinf(r);
        lookup_h2cos[h] = cosf(r);
        lookup_h2cos_60h[h] = cosf(DEG_TO_RAD(60 - h));
        lookup_h2cos_h120[h] = cosf(DEG_TO_RAD(h - 120));
        lookup_h2cos_180h[h] = cosf(DEG_TO_RAD(180 - h));
        lookup_h2cos_h240[h] = cosf(DEG_TO_RAD(h - 240));
        lookup_h2cos_300h[h] = cosf(DEG_TO_RAD(300 - h));
    }
#endif
}

bool gcd(int a, int b)
{
    while (b)
    {
        int t = a % b;
        a = b;
        b = t;
    }
    return a == 1;
}

int getCoPrime(int a)
{

    int coprime = -1;
    int max = a * 100;
    for (int i = a; i < max; i++)
    {
        if (gcd(a, i))
        {
            coprime = i;
            break;
        }
    }

    return coprime;
}

inline int math_average_angle(int *angles, int length)
{
    float x = 0;
    float y = 0;

    for (int i = 0; i < length; i++)
    {
#ifdef MATH_PRECOMPUTE
        x += lookup_h2cos[angles[i]];
        y += lookup_h2sin[angles[i]];
#else
        float r = angles[i] * (M_PI / 180.0);
        x += cosf(r);
        y += sinf(r);
#endif
    }

    return (int)roundf(atan2f(y, x) * (180.0 / M_PI)) % 360;
}

static const float RADIAN_TO_PI = (180.0 / M_PI);

// TODO: Speed up this method!
inline HsiaColor math_average_hsia(HsiaColor *colors, uint8_t length)
{
    float x = 0;
    float y = 0;
    HsiaColor result = {colors[0].h, colors[0].s, colors[0].i, colors[0].a};
    //uint16_t hsia_h = colors[0].h;
    //float hsia_s = colors[0].s;
    //float hsia_i = colors[0].i;
    //float hsia_a = colors[0].a;
    for (uint_fast8_t i = 1; i < length; i++)
    {
        HsiaColor c = colors[sizeof(HsiaColor) * i];
        if (c.a <= 0)
        {
            // Do not include at all if it's transparent.
            continue;
        }

#ifdef MATH_PRECOMPUTE
        x += lookup_h2cos[c.h];
        y += lookup_h2sin[c.h];
#else
        float r = DEG_TO_RAD(c.h);
        x += cosf(r);
        y += sinf(r);
#endif

        uint16_t degrees = (uint16_t)roundf(atan2f(y, x) * RADIAN_TO_PI);

        float a = c.a + result.a * (1 - result.a);

        //return c;
        //if (c.s > sMax)
        //    sMax = c.s;
        //if (c.i > iMax)
        //    iMax = c.i;
    }

    uint16_t degrees = (uint16_t)roundf(atan2f(y, x) * RADIAN_TO_PI);

    return (HsiaColor){degrees, 0, 0, 1};
}

inline HsiaColor math_average_hsia2(HsiaColor *a, HsiaColor *b)
{
    float x = 0;
    float y = 0;
    HsiaColor result = {a->h, a->s, a->i, a->a};

    x += lookup_h2cos[a->h];
    y += lookup_h2sin[a->h];
    x += lookup_h2cos[b->h];
    y += lookup_h2sin[b->h];

    uint16_t degrees = (uint16_t)roundf(atan2f(y, x) * RADIAN_TO_PI);

    float alpha = b->a + result.a * (1 - result.a);

    //uint16_t degrees = (uint16_t)roundf(atan2f(y, x) * RADIAN_TO_PI);

    return (HsiaColor){degrees, 1, 1, alpha};
}

int randint(int n)
{
    if (n >= RAND_MAX)
    {
        // n is larger than RAND_MAX, then we just use the regular rand.
        return rand();
    }
    else
    {
        // Chop off all of the values that would cause skew...
        int end = RAND_MAX / n; // truncate skew
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        int r;
        while ((r = rand()) >= end)
            ;

        return r % n;
    }
}

/**
 * Higher power means higher probability to be lower.
 * Weight can be anything from 0 to Inf.
 * */
int randint_weighted_towards_min(int min, int max, float weight)
{
    float random = rand() / (float)RAND_MAX; // 0..1
    return (int)floorf(min + (max - min) * (powf(random, weight)));
}

/**
 * Higher power means higher probability to be higher
 * Weight can be anything from 0 to Inf.
 * */
int randint_weighted_towards_max(int min, int max, float weight)
{
    weight = 1 / weight;
    float random = rand() / (float)RAND_MAX; // 0..1
    return (int)floorf(min + (max - min) * (powf(random, weight)));
}

/**
 * Gives a value where 0 is the most likely, and numbers below and above that become increasingly unlikely.
 * The regular result is an almost complete tapering off after -3 and +3.
 * */
double rand_gaussian()
{
    double a = ((double)(rand())) / ((double)RAND_MAX);
    double b = ((double)(rand())) / ((double)RAND_MAX);

    double R0 = sqrt(-2.0 * log(a)) * cos(2 * M_PI * b);
    /*
        double R1 = sqrt(-2.0 * log(a)) * sin(2 * M_PI * b);
    */

    return R0;
}

// Add "brightness" that is used to modify the values right before it it sent to the GPIO
// Add "dithering" which should be done by giving a rounding method that alternates between "round up" and "round down"

// Add "color correction"
// TypicalSMD5050 =0xFFB0F0, TypicalLEDStrip =0xFFB0F0, Typical8mmPixel =0xFFE08C, TypicalPixelString =0xFFE08C,
// UncorrectedColor =0xFFFFFF

// Need a way of converting HSV to RGBW -- where the W has dynamic warmth!

// This method has to be blazingly fast! Find ways to spee it up if possible!

#define HUE_UPPER_LIMIT 360

RgbwColor hsia2rgbw(HsiaColor *hsia)
{
    float i = hsia->i;
    if (hsia->a < 1)
    {
        // Since we are doing the final conversion from HSIA to RGBW, we cannot bring along the alpha channel.
        // We will simulate this by moving the alpha as a multiplier of the given intensity.
        i = i * hsia->a;
    }

    uint8_t r, g, b;

#ifdef MATH_PRECOMPUTE
    if (hsia->h == 0)
    {
        r = 255 * (i + 2 * i * hsia->s);
        g = 255 * (i - i * hsia->s);
        b = 255 * (i - i * hsia->s);
    }
    else if (hsia->h < 120)
    {
        r = 255 * (i + i * hsia->s * lookup_h2cos[hsia->h] / lookup_h2cos_60h[hsia->h]);
        g = 255 * (i + i * hsia->s * (1 - lookup_h2cos[hsia->h] / lookup_h2cos_60h[hsia->h]));
        b = 255 * (i - i * hsia->s);
    }
    else if (hsia->h == 120)
    {
        r = 255 * (i - i * hsia->s);
        g = 255 * (i + 2 * i * hsia->s);
        b = 255 * (i - i * hsia->s);
    }
    else if (hsia->h < 240)
    {
        r = 255 * (i - i * hsia->s);
        g = 255 * (i + i * hsia->s * lookup_h2cos_h120[hsia->h] / lookup_h2cos_180h[hsia->h]);
        b = 255 * (i + i * hsia->s * (1 - lookup_h2cos_h120[hsia->h] / lookup_h2cos_180h[hsia->h]));
    }
    else if (hsia->h == 240)
    {
        r = 255 * (i - i * hsia->s);
        g = 255 * (i - i * hsia->s);
        b = 255 * (i + 2 * i * hsia->s);
    }
    else
    {
        r = 255 * (i + i * hsia->s * (1 - lookup_h2cos_h240[hsia->h] / lookup_h2cos_300h[hsia->h]));
        g = 255 * (i - i * hsia->s);
        b = 255 * (i + i * hsia->s * lookup_h2cos_h240[hsia->h] / lookup_h2cos_300h[hsia->h]);
    }
#else
    if (hsia->h < 120)
    {
        float rad = DEG_TO_RAD(hsia->h);
        float rad_offset = DEG_TO_RAD(60 - hsia->h);
        r = 255 * (i + i * hsia->s * cosf(rad) / cosf(rad_offset));
        g = 255 * (i + i * hsia->s * (1 - cosf(rad) / cosf(rad_offset)));
        b = 255 * (i - i * hsia->s);
    }
    else if (hsia->h < 240)
    {
        float H = M_PI * hsia->h / (float)180; // Convert to radians
        H = H - 2.09439;
        float cos_h = cosf(H);
        float cos_1047_h = cosf(1.047196667 - H);
        g = hsia->s * 255 * i / 3 * (1 + cos_h / cos_1047_h);
        b = hsia->s * 255 * i / 3 * (1 + (1 - cos_h / cos_1047_h));
        r = 0;
    }
    else
    {

        float H = M_PI * hsia->h / (float)180; // Convert to radians
        H = H - 4.188787;
        float cos_h = cosf(H);
        float cos_1047_h = cosf(1.047196667 - H);
        b = hsia->s * 255 * i / 3 * (1 + cos_h / cos_1047_h);
        r = hsia->s * 255 * i / 3 * (1 + (1 - cos_h / cos_1047_h));
        g = 0;
    }
#endif

    return (RgbwColor){r, g, b, 255 * (1 - hsia->s) * i};
}

HsiaColor LerpHSIA(HsiaColor *a, HsiaColor *b, float t)
{
    // Hue interpolation
    int h;
    int d = b->h - a->h;
    if (a->h > b->h)
    {
        // Swap (a.h, b.h)
        int h3 = b->h;
        b->h = a->h;
        a->h = h3;
        d = -d;
        t = 1 - t;
    }

    if (d > 180) // 180deg
    {
        a->h = a->h + 360;
        h = ((int)(a->h + t * (b->h - a->h))) % 360;
    }
    else
    {
        h = a->h + t * d;
    }

    // Interpolates the rest
    return (HsiaColor){
        h,
        a->s + t * (b->s - a->s),
        a->i + t * (b->i - a->i),
        (a->a + b->a) / 2};
}