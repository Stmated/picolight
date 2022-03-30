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

    /*
    for (int h = HSI_H_MIN; h < HSI_H_MAX; h++)
    {
        float r = h * (M_PI / 180.0) - 2.09439;
        // float H = r
        float cos_h = cosf(r);
        float cos_1047_h = cosf(1.047196667 - r);
        float div = cos_h / cos_1047_h;

        lookup_hsi2rgbw_cos_240[h] = div;
    }

    for (int h = HSI_H_MIN; h < HSI_H_MAX; h++)
    {
        float r = h * (M_PI / 180.0) - 4.188787;
        // float H = h - 4.188787;
        float cos_h = cosf(r);
        float cos_1047_h = cosf(1.047196667 - r);
        float div = cos_h / cos_1047_h;

        lookup_hsi2rgbw_cos_360[h] = div;
    }
    */
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

// TODO: This can be simplified if we change hue into float? Or should everything else become ints?
//          This method is SLOW -- have to SPEED IT UP SIGNIFICANTLY!
/*float math_average_angle(float *angles, int length)
{
    float x = 0;
    float y = 0;

    for (int i = 0; i < length; i++)
    {
        float r = angles[i] * (M_PI / 180.0);
        x += cosf(r);
        y += sinf(r);
    }

    float radians = atan2f(y, x);
    int degrees = (int)roundf(radians * (180.0 / M_PI));
    int fixedDegreees = (degrees + 360) % 360;
    return fixedDegreees;
}
*/

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

// TODO: Speed up this method! It is slow! Especially patterns_average_angles!
inline HsiColor math_average_hsi(HsiColor *colors, uint8_t length)
{
    float x = 0;
    float y = 0;
    float sMax = 0;
    float iMax = 0;
    for (int i = 0; i < length; i++)
    {
        HsiColor c = colors[sizeof(HsiColor) * i];
        if (c.i == 0)
        {
            // Do not include if it's black/transparent
            continue;
        }

#ifdef MATH_PRECOMPUTE
        x += lookup_h2cos[c.h];
        y += lookup_h2sin[c.h];
#else
        float r = c.h * (M_PI / 180.0);
        x += cosf(r);
        y += sinf(r);
#endif

        if (c.s > sMax)
            sMax = c.s;
        if (c.i > iMax)
            iMax = c.i;
    }

    uint16_t degrees = (uint16_t)roundf(atan2f(y, x) * RADIAN_TO_PI);

    return (HsiColor){degrees, sMax, iMax};
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

RgbwColor hsi2rgbw(HsiColor *hsi)
{
    uint8_t r, g, b;

#ifdef MATH_PRECOMPUTE
    if (hsi->h == 0)
    {
        r = 255 * (hsi->i + 2 * hsi->i * hsi->s);
        g = 255 * (hsi->i - hsi->i * hsi->s);
        b = 255 * (hsi->i - hsi->i * hsi->s);
    }
    else if (hsi->h < 120)
    {
        r = 255 * (hsi->i + hsi->i * hsi->s * lookup_h2cos[hsi->h] / lookup_h2cos_60h[hsi->h]);
        g = 255 * (hsi->i + hsi->i * hsi->s * (1 - lookup_h2cos[hsi->h] / lookup_h2cos_60h[hsi->h]));
        b = 255 * (hsi->i - hsi->i * hsi->s);
    }
    else if (hsi->h == 120)
    {
        r = 255 * (hsi->i - hsi->i * hsi->s);
        g = 255 * (hsi->i + 2 * hsi->i * hsi->s);
        b = 255 * (hsi->i - hsi->i * hsi->s);
    }
    else if (hsi->h < 240)
    {
        r = 255 * (hsi->i - hsi->i * hsi->s);
        g = 255 * (hsi->i + hsi->i * hsi->s * lookup_h2cos_h120[hsi->h] / lookup_h2cos_180h[hsi->h]);
        b = 255 * (hsi->i + hsi->i * hsi->s * (1 - lookup_h2cos_h120[hsi->h] / lookup_h2cos_180h[hsi->h]));
    }
    else if (hsi->h == 240)
    {
        r = 255 * (hsi->i - hsi->i * hsi->s);
        g = 255 * (hsi->i - hsi->i * hsi->s);
        b = 255 * (hsi->i + 2 * hsi->i * hsi->s);
    }
    else
    {
        r = 255 * (hsi->i + hsi->i * hsi->s * (1 - lookup_h2cos_h240[hsi->h] / lookup_h2cos_300h[hsi->h]));
        g = 255 * (hsi->i - hsi->i * hsi->s);
        b = 255 * (hsi->i + hsi->i * hsi->s * lookup_h2cos_h240[hsi->h] / lookup_h2cos_300h[hsi->h]);
    }
#else
    if (hsi->h < 120)
    {
        float rad = DEG_TO_RAD(hsi->h);
        float rad_offset = DEG_TO_RAD(60 - hsi->h);
        r = 255 * (hsi->i + hsi->i * hsi->s * cosf(rad) / cosf(rad_offset));
        g = 255 * (hsi->i + hsi->i * hsi->s * (1 - cosf(rad) / cosf(rad_offset)));
        b = 255 * (hsi->i - hsi->i * hsi->s);
    }
    else if (hsi->h < 240)
    {
        float H = M_PI * hsi->h / (float)180; // Convert to radians
        H = H - 2.09439;
        float cos_h = cosf(H);
        float cos_1047_h = cosf(1.047196667 - H);
        g = hsi->s * 255 * hsi->i / 3 * (1 + cos_h / cos_1047_h);
        b = hsi->s * 255 * hsi->i / 3 * (1 + (1 - cos_h / cos_1047_h));
        r = 0;
    }
    else
    {

        float H = M_PI * hsi->h / (float)180; // Convert to radians
        H = H - 4.188787;
        float cos_h = cosf(H);
        float cos_1047_h = cosf(1.047196667 - H);
        b = hsi->s * 255 * hsi->i / 3 * (1 + cos_h / cos_1047_h);
        r = hsi->s * 255 * hsi->i / 3 * (1 + (1 - cos_h / cos_1047_h));
        g = 0;
    }
#endif

    // c->r = r;
    // c->g = g;
    // c->b = b;
    // c->w = 255 * (1 - hsi->s) * hsi->i;

    return (RgbwColor){r, g, b, 255 * (1 - hsi->s) * hsi->i};
}

HsiColor LerpHSI(HsiColor *a, HsiColor *b, float t)
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
    return (HsiColor){
        h,
        a->s + t * (b->s - a->s),
        a->i + t * (b->i - a->i)};
}