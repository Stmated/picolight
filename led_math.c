#include "led_math.h"

#ifdef MATH_PRECOMPUTE
static float lookup_h2cos[HSI_H_MAX] = {};
static float lookup_h2sin[HSI_H_MAX] = {};
static float lookup_hsi2rgbw_cos_120[HSI_H_MAX] = {};
static float lookup_hsi2rgbw_cos_240[HSI_H_MAX] = {};
static float lookup_hsi2rgbw_cos_360[HSI_H_MAX] = {};
#endif

void math_precompute()
{
#ifdef MATH_PRECOMPUTE
    for (int h = HSI_H_MIN; h < HSI_H_MAX; h++)
    {
        float r = h * (M_PI / 180.0);
        float cos_h = cosf(r);
        float cos_1047_h = cosf(1.047196667 - r);
        float div = cos_h / cos_1047_h;

        lookup_h2sin[h] = sinf(r);
        lookup_h2cos[h] = cos_h;
        lookup_hsi2rgbw_cos_120[h] = div;
    }

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

    float radians = atan2f(y, x);
    int degrees = (int)roundf(radians * (180.0 / M_PI));
    int fixedDegreees = (degrees + 360) % 360;
    return fixedDegreees;
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
    if ((n - 1) == RAND_MAX)
    {
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
    float random = randint(100000) / (float)100000; // 0..1
    return (int)floorf(min + (max - min) * (powf(random, weight)));
}

/**
 * Higher power means higher probability to be higher
 * Weight can be anything from 0 to Inf.
 * */
int randint_weighted_towards_max(int min, int max, float weight)
{
    if (weight > 1)
    {
        // If weight is above 1, then we should go further below 1 the higher the number.
        weight = 1 / weight;
    }
    else if (weight < 1)
    {
        // If weight is under 1, then we should go further ABOVE 1.
        weight = 1 / weight;
    }

    float random = randint(100000) / (float)100000; // 0..1
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

#define DEG_TO_RAD(X) (M_PI * (X) / 180)

// Add "brightness" that is used to modify the values right before it it sent to the GPIO
// Add "dithering" which should be done by giving a rounding method that alternates between "round up" and "round down"

// Add "color correction"
// TypicalSMD5050 =0xFFB0F0, TypicalLEDStrip =0xFFB0F0, Typical8mmPixel =0xFFE08C, TypicalPixelString =0xFFE08C,
// UncorrectedColor =0xFFFFFF

// Need a way of converting HSV to RGBW -- where the W has dynamic warmth!

// This method has to be blazingly fast! Find ways to spee it up if possible!
void hsi2rgbw(HsiColor *hsi, RgbwColor *c)
{
    uint8_t r, g, b, w;

    if (hsi->h < 120)
    {
#ifdef MATH_PRECOMPUTE
        float cos_div_cached = lookup_hsi2rgbw_cos_120[hsi->h];
        r = hsi->s * 255 * hsi->i / 3 * (1 + cos_div_cached);
        g = hsi->s * 255 * hsi->i / 3 * (1 + (1 - cos_div_cached));
#else
        float H = M_PI * hsi->h / (float)180; // Convert to radians
        cos_h = cosf(H);
        cos_1047_h = cosf(1.047196667 - H);
        r = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
        g = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
#endif
        b = 0;
        w = 255 * (1 - hsi->s) * hsi->i;
    }
    else if (hsi->h < 240)
    {
#ifdef MATH_PRECOMPUTE
        float cos_div_cached = lookup_hsi2rgbw_cos_240[hsi->h];
        g = hsi->s * 255 * hsi->i / 3 * (1 + cos_div_cached);
        b = hsi->s * 255 * hsi->i / 3 * (1 + (1 - cos_div_cached));
#else
        float H = M_PI * hsi->h / (float)180; // Convert to radians
        H = H - 2.09439;
        cos_h = cosf(H);
        cos_1047_h = cosf(1.047196667 - H);
        g = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
        b = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
#endif
        r = 0;
        w = 255 * (1 - hsi->s) * hsi->i;
    }
    else
    {
#ifdef MATH_PRECOMPUTE
        float cos_div_cached = lookup_hsi2rgbw_cos_360[hsi->h];
        b = hsi->s * 255 * hsi->i / 3 * (1 + cos_div_cached);
        r = hsi->s * 255 * hsi->i / 3 * (1 + (1 - cos_div_cached));
#else
        float H = M_PI * hsi->h / (float)180; // Convert to radians
        H = H - 4.188787;
        float cos_h = cosf(H);
        float cos_1047_h = cosf(1.047196667 - H);
        b = hsi->s * 255 * hsi->i / 3 * (1 + cos_h / cos_1047_h);
        r = hsi->s * 255 * hsi->i / 3 * (1 + (1 - cos_h / cos_1047_h));
#endif
        g = 0;
        w = 255 * (1 - hsi->s) * hsi->i;
    }

    c->r = r;
    c->g = g;
    c->b = b;
    c->w = w;
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