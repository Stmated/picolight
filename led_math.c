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

int math_shortest_path_lerp(int origin, int target, float t)
{

    if (origin > target)
    {
        int raw_diff = origin - target;
        int mod_diff = raw_diff % HSI_H_MAX;
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
        int raw_diff = target - origin;
        int mod_diff = raw_diff % HSI_H_MAX;
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

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif
#ifndef MIN
#define MIN(a, b) (a > b ? b : a)
#endif

HsiaColor rgbw2hsia(RgbwColor c, float a)
{
    float r = c.r / (float)255.0;
    float g = c.g / (float)255.0;
    float b = c.b / (float)255.0;
    float intensity = (r + g + b) / (float)3;

    float M = MAX(r, MAX(g, b));
    float m = MIN(r, MIN(g, b));
    // float C = M - m;

    float saturation = 0.0;
    if (intensity == 0.0)
        saturation = 0.0;
    else
        saturation = 1.0 - (m / intensity);

    float hue = 0;
    if (M == m)
    {
        hue = 0;
    }
    if (M == r)
    {
        if (M == m)
            hue = 0.0;
        else
            hue = 60.0 * (0.0 + ((g - b) / (M - m)));
    }
    if (M == g)
    {
        if (M == m)
            hue = 0.0;
        else
            hue = 60.0 * (2.0 + ((b - r) / (M - m)));
    }
    if (M == b)
    {
        if (M == m)
            hue = 0.0;
        else
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
    if (hsia_b->a <= 0.01)
        return *hsia_a;
    if (hsia_b->a >= 0.99)
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
    int H = hsia->h;
    double S = hsia->s;
    double I = hsia->i;
    
    //if (hsia->a < 1)
    //{
        // Since we are doing the final conversion from HSIA to RGBW, we cannot bring along the alpha channel.
        // We will simulate this by moving the alpha as a multiplier of the given intensity.
    //    I = I * hsia->a;
    //}

    double Htag = H / (double) 60;
    double Z = 1 - fabs(fmod(Htag, 2) - 1);
    double C = (3 * I * S) / (1 + Z);
    double X = C * Z;

    double R1;
    double G1;
    double B1;
    if (0 <= Htag && Htag <= 1) {
        R1 = C;
        G1 = X;
        B1 = 0; //R1, G1, B1 = C, X, 0
    }
    else if (1 <= Htag && Htag <= 2) {
        R1 = X;
        G1 = C;
        B1 = 0; //R1, G1, B1 = X, C, 0
    }
    else if (2 <= Htag && Htag <= 3) {
        R1 = 0;
        G1 = C;
        B1 = X; //R1, G1, B1 = 0, C, X
    }
    else if (3 <= Htag && Htag <= 4) {
        R1 = 0;
        G1 = X;
        B1 = C; //R1, G1, B1 = 0, X, C
    }
    else if (4 <= Htag && Htag <= 5) {
        R1 = X;
        G1 = 0;
        B1 = C; //R1, G1, B1 = X, 0, C
    }
    else if (5 <= Htag && Htag <= 6) {
        R1 = C;
        G1 = 0;
        B1 = X; //R1, G1, B1 = C, 0, X
    }
    else {
        R1 = 0;
        G1 = 0;
        B1 = 0; //R1, G1, B1 = 0, 0, 0  # Undefined
    }

    // Calculation rgb
    double m = I * (1 - S);
    double R = R1 + m;
    double G = G1 + m;
    double B = B1 + m; //, G, B = R1 + m, G1 + m, B1 + m

    // Limit R, G, B to valid range:
    //R = max(min(R, 1), 0)
    //G = max(min(G, 1), 0)
    //B = max(min(B, 1), 0)

    // Handling RGB values above 1:
    // -----------------------------
    // Avoiding weird colours - see the comment of Giacomo Catenazzi.
    // Find the maximum between R, G, B, and if the value is above 1, divide the 3 channels with such numbers.
    //int max_rgb = MAX(R, MAX(G, B));
    double max_rgb = R;
    if (G > max_rgb) {
        max_rgb = G;
    }
    if (B > max_rgb) {
        max_rgb = B;
    }

    if (max_rgb > 1) {
        //printf("More than 1, %f, %f, %f\n", R, G, B);
        R = R / 3; //R / max_rgb;
        G = G / 3; //G / max_rgb;
        B = B / 3; //B / max_rgb;
    }
    
    return (RgbwColor){
        (int)floorf(255 * R),
        (int)floorf(255 * G),
        (int)floorf(255 * B),
        (int) (255 * ((float)((1 - S) * I)))
    };

    /*
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
    */
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