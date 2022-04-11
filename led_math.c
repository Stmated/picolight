#include "led_math.h"

// Add "brightness" that is used to modify the values right before it it sent to the GPIO
// Add "dithering" which should be done by giving a rounding method that alternates between "round up" and "round down"

// Add "color correction"
// TypicalSMD5050 =0xFFB0F0, TypicalLEDStrip =0xFFB0F0, Typical8mmPixel =0xFFE08C, TypicalPixelString =0xFFE08C,
// UncorrectedColor =0xFFFFFF

// Need a way of converting HSV to RGBW -- where the W has dynamic warmth!

// This method has to be blazingly fast! Find ways to spee it up if possible!

#ifdef MATH_PRECOMPUTE
static double lookup_h2cos[HSI_H_MAX] = {0};
static double lookup_h2sin[HSI_H_MAX] = {0};
static double lookup_h2cos_60h[HSI_H_MAX] = {0};
static double lookup_h2cos_h120[HSI_H_MAX] = {0};
static double lookup_h2cos_180h[HSI_H_MAX] = {0};
static double lookup_h2cos_h240[HSI_H_MAX] = {0};
static double lookup_h2cos_300h[HSI_H_MAX] = {0};
#ifndef MATH_RGBW_BY_COORDINATES
static double lookup_z[HSI_H_MAX] = {0};
#endif
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

        lookup_h2sin[h] = sin(r);
        lookup_h2cos[h] = cos(r);
        lookup_h2cos_60h[h] = cos(DEG_TO_RAD(60 - h));
        lookup_h2cos_h120[h] = cos(DEG_TO_RAD(h - 120));
        lookup_h2cos_180h[h] = cos(DEG_TO_RAD(180 - h));
        lookup_h2cos_h240[h] = cos(DEG_TO_RAD(h - 240));
        lookup_h2cos_300h[h] = cos(DEG_TO_RAD(300 - h));

#ifndef MATH_RGBW_BY_COORDINATES
        lookup_z[h] = 1 - fabs(fmod(h / (double)60, 2) - 1);
#endif
    }
#endif
}

float math_shortest_hue_distance_lerp(float origin, float target, float t)
{
    if (origin > target)
    {
        float raw_diff = origin - target;
        float mod_diff = fmodf(raw_diff, HSI_H_MAX);
        if (mod_diff > HSI_H_HALF)
        {
            return t * (HSI_H_MAX - mod_diff);
        }
        else
        {
            return t * (mod_diff * -1);
        }
    }
    else
    {
        float raw_diff = target - origin;
        float mod_diff = fmodf(raw_diff, HSI_H_MAX);
        if (mod_diff > HSI_H_HALF)
        {
            float signedDiff = (HSI_H_MAX - mod_diff);
            return t * (signedDiff * -1);
        }
        else
        {
            return t * (mod_diff);
        }
    }
}

int math_hue_lerp(float origin, float target, float t)
{
    float distance = math_shortest_hue_distance_lerp(origin, target, t);
    float hue = origin + distance;

    if (hue < 0)
    {
        return floorf(360 + hue);
    }
    else if (hue > 360)
    {
        return (int)floorf(hue - 360) % HSI_H_MAX;
    }

    return floorf(hue);
}

HsiaColor rgbw2hsia(RgbwColor c, float a)
{
    double r = c.r / (double)255.0;
    double g = c.g / (double)255.0;
    double b = c.b / (double)255.0;
    double intensity = (r + g + b) / (double)3;

    double M = MAX(r, MAX(g, b));
    double m = MIN(r, MIN(g, b));

    double saturation = 0.0;
    if (intensity == 0.0)
        saturation = 0.0;
    else
        saturation = 1.0 - (m / intensity);

    double hue = 0;
    if (M == m)
    {
        hue = 0;
    }
    else
    {
        if (M == r)
            hue = 60.0 * (0.0 + ((g - b) / (M - m)));
        else if (M == g)
            hue = 60.0 * (2.0 + ((b - r) / (M - m)));
        else if (M == b)
            hue = 60.0 * (4.0 + ((r - g) / (M - m)));
    }

    if (hue < 0.0)
    {
        hue = hue + 360;
    }

    return (HsiaColor){(int)hue, fabs(saturation), intensity, a};
}

// TODO: There is a slight loss in precision from using this method! The RGB becomes ever so slightly off! Fix this!
// Try and work with only HSIA if possible :(
inline HsiaColor math_average_hsia(HsiaColor *hsia_a, HsiaColor *hsia_b)
{
    // If above very low/high alpha, just replace. Faster and won't notice a difference.
    if (hsia_b->a <= 0.02)
        return *hsia_a;
    if (hsia_b->a >= 0.98)
        return *hsia_b;

    // TODO: Can this be replaced with full ONLY HSI calculations?
    // Or even cooler:  https://en.wikipedia.org/wiki/CIECAM02
    //                  https://github.com/dannyvi/ciecam02
    RgbwColor ca = hsia2rgbw(hsia_a);
    RgbwColor cb = hsia2rgbw(hsia_b);

    int r = (int)((cb.r * hsia_b->a) + (ca.r * (1.0 - hsia_b->a)));
    int g = (int)((cb.g * hsia_b->a) + (ca.g * (1.0 - hsia_b->a)));
    int b = (int)((cb.b * hsia_b->a) + (ca.b * (1.0 - hsia_b->a)));
    int w = (int)((cb.w * hsia_b->a) + (ca.w * (1.0 - hsia_b->a)));

    RgbwColor rgbw = {r, g, b, w};
    float a = hsia_a->a + (hsia_b->a * (1 - hsia_a->a));

    return rgbw2hsia(rgbw, a);
}

inline HsiaColor math_average_hsia_lerp(HsiaColor *hsia_a, HsiaColor *hsia_b, float p)
{
    /*
    if (p <= 0.02)
        return *hsia_a;
    if (p >= 0.98)
        return *hsia_b;

    // TODO: Can this be replaced with full ONLY HSI calculations?
    // Or even cooler:  https://en.wikipedia.org/wiki/CIECAM02
    //                  https://github.com/dannyvi/ciecam02
    RgbwColor ca = hsia2rgbw(hsia_a);
    RgbwColor cb = hsia2rgbw(hsia_b);

    int r = (int)((cb.r * p) + (ca.r * (1.0 - p)));
    int g = (int)((cb.g * p) + (ca.g * (1.0 - p)));
    int b = (int)((cb.b * p) + (ca.b * (1.0 - p)));
    int w = (int)((cb.w * p) + (ca.w * (1.0 - p)));

    RgbwColor rgbw = {r, g, b, w};
    float a = ((hsia_b->a * p) + (hsia_a->a * (1.0 - p))); //hsia_a->a + (hsia_b->a * (1 - hsia_a->a));

    return rgbw2hsia(rgbw, a);
    */

    // ???
    int hue = math_hue_lerp(hsia_a->h, hsia_b->h, p);
    float saturation = hsia_a->s + (hsia_b->s - hsia_a->s) * p;
    float intensity = hsia_a->i + (hsia_b->i - hsia_a->i) * p;
    float alpha = hsia_a->a + (hsia_b->a - hsia_a->a) * p;

    return (HsiaColor){hue, saturation, intensity, alpha};
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
    return (int)roundf(min + (max - min) * (powf(random, weight)));
}

/**
 * Higher power means higher probability to be higher
 * Weight can be anything from 0 to Inf.
 * */
int randint_weighted_towards_max(int min, int max, float weight)
{
    weight = 1 / weight;
    float random = rand() / (float)RAND_MAX; // 0..1
    return (int)roundf(min + (max - min) * (powf(random, weight)));
}

/**
 * Gives a value where 0 is the most likely, and numbers below and above that become increasingly unlikely.
 * The regular result is an almost complete tapering off after -3 and +3.
 * */
double rand_gaussian()
{
    double a = ((double)(rand())) / ((double)RAND_MAX);
    double b = ((double)(rand())) / ((double)RAND_MAX);

    return sqrt(-2.0 * log(a)) * cos(2 * M_PI * b);
}

#ifndef MATH_RGBW_BY_COORDINATES
RgbwColor hsia2rgbw(HsiaColor *hsia)
{
    int H = hsia->h;
    double S = hsia->s;
    double I = hsia->i;

    if (H < 0 || H > 360 || S < 0 || S > 1 || I < 0 || I > 1)
    {
        printf("THERE IS AN INCORRECT COLOR HERE\n");
    }

#ifdef MATH_PRECOMPUTE
    double Z = lookup_z[H];
#else
    double Z = 1 - fabs(fmod(H / (double)60, 2) - 1);
#endif
    double C = (3 * I * S) / (1 + Z);
    double X = C * Z;

    double R1;
    double G1;
    double B1;
    if (H <= 60)
    {
        R1 = C;
        G1 = X;
        B1 = 0;
    }
    else if (H <= 120)
    {
        R1 = X;
        G1 = C;
        B1 = 0;
    }
    else if (H <= 180)
    {
        R1 = 0;
        G1 = C;
        B1 = X;
    }
    else if (H <= 240)
    {
        R1 = 0;
        G1 = X;
        B1 = C;
    }
    else if (H <= 300)
    {
        R1 = X;
        G1 = 0;
        B1 = C;
    }
    else if (H <= 360)
    {
        R1 = C;
        G1 = 0;
        B1 = X;
    }
    else
    {
        R1 = 0;
        G1 = 0;
        B1 = 0;
    }

    // Calculation rgb
    double m = I * (1 - S);
    double R = R1 + m;
    double G = G1 + m;
    double B = B1 + m;

    // Limit R, G, B to valid ranges (since not all HSI values are in gamut range):
    double max_rgb = R;
    if (G > max_rgb)
    {
        max_rgb = G;
    }
    if (B > max_rgb)
    {
        max_rgb = B;
    }

    // These two both have their strong suits, but also clipping weaknesses.
    /*
    if (max_rgb > 1)
    {
        R = R / 3; // R / max_rgb;
        G = G / 3; // G / max_rgb;
        B = B / 3; // B / max_rgb;
    }
    */
    if (max_rgb > 1)
    {
        R = R / max_rgb;
        G = G / max_rgb;
        B = B / max_rgb;
    }

    return (RgbwColor){
        (int)round(255 * R),
        (int)round(255 * G),
        (int)round(255 * B),
        (int)(255 * ((float)((1 - S) * I)))};
}
#else
#ifdef MATH_PRECOMPUTE
RgbwColor hsia2rgbw(HsiaColor *hsia)
{
    float i = hsia->i;
    uint8_t r, g, b;
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

    return (RgbwColor){r, g, b, 255 * (1 - hsia->s) * i};
}
#else
RgbwColor hsia2rgbw(HsiaColor *hsia)
{
    float i = hsia->i;
    uint8_t r, g, b;

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

    return (RgbwColor){r, g, b, 255 * (1 - hsia->s) * i};
}
#endif
#endif