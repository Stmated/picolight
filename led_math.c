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
        float r = DEG_TO_RAD(h);

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

RgbwaColor math_rgbwa_lerp(RgbwaColor a, RgbwaColor b, float t)
{
    return (RgbwaColor){math_lerp(a.r, b.r, t), math_lerp(a.g, b.g, t), math_lerp(a.b, b.b, t), math_lerp(a.w, b.w, t), math_lerp(a.a, b.a, t)};
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

inline float math_lerpf(float a, float b, float v)
{
    // This has some loss of precision, but is faster than other lerps
    return a + v * (b - a);
}

inline float math_lerp(int a, int b, float v)
{
    if (a == b)
    {
        return a;
    }

    // This has some loss of precision, but is faster than other lerps
    return a + v * (b - a);
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
    RgbwaColor ca = hsia2rgbwa(hsia_a->h, hsia_a->s, hsia_a->i, hsia_a->a);
    RgbwaColor cb = hsia2rgbwa(hsia_b->h, hsia_b->s, hsia_b->i, hsia_b->a);

    int r = (int)((cb.r * hsia_b->a) + (ca.r * (1.0 - hsia_b->a)));
    int g = (int)((cb.g * hsia_b->a) + (ca.g * (1.0 - hsia_b->a)));
    int b = (int)((cb.b * hsia_b->a) + (ca.b * (1.0 - hsia_b->a)));
    int w = (int)((cb.w * hsia_b->a) + (ca.w * (1.0 - hsia_b->a)));

    RgbwColor rgbw = {r, g, b, w};
    float a = hsia_a->a + (hsia_b->a * (1 - hsia_a->a));

    return rgbw2hsia(rgbw, a);
}

inline RgbwaColor math_average_rgbwa(RgbwaColor *ca, RgbwaColor *cb)
{
    int r = (int)((cb->r) + (ca->r)) / 2;
    int g = (int)((cb->g) + (ca->g)) / 2;
    int b = (int)((cb->b) + (ca->b)) / 2;
    int w = (int)((cb->w) + (ca->w)) / 2;
    int a = (int)((cb->a) + (ca->a)) / 2;

    return (RgbwaColor){r, g, b, w, a};
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

// Helper: clamp a value to 0..255
uint8_t clamp8(int16_t x)
{
    if (x < 0)
    {
        return 0;
    }
    else if (x > 255)
    {
        return 255;
    }
    else
    {
        return x;
    }
}

static const uint8_t z_lut[256] = {
    0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90,
    96, 102, 108, 114, 120, 126, 132, 138, 144, 150, 156, 162, 168, 174, 180, 186,
    192, 198, 204, 210, 216, 222, 228, 234, 240, 246, 252, 255, 249, 243, 237, 231,
    225, 219, 213, 207, 201, 195, 189, 183, 177, 171, 165, 159, 153, 147, 141, 135,
    129, 123, 117, 111, 105, 99, 93, 87, 81, 75, 69, 63, 57, 51, 45, 39,
    33, 27, 21, 15, 9, 3, 3, 9, 15, 21, 27, 33, 39, 45, 51, 57,
    63, 69, 75, 81, 87, 93, 99, 105, 111, 117, 123, 129, 135, 141, 147, 153,
    159, 165, 171, 177, 183, 189, 195, 201, 207, 213, 219, 225, 231, 237, 243, 249,
    255, 249, 243, 237, 231, 225, 219, 213, 207, 201, 195, 189, 183, 177, 171, 165,
    159, 153, 147, 141, 135, 129, 123, 117, 111, 105, 99, 93, 87, 81, 75, 69,
    63, 57, 51, 45, 39, 33, 27, 21, 15, 9, 3, 3, 9, 15, 21, 27,
    33, 39, 45, 51, 57, 63, 69, 75, 81, 87, 93, 99, 105, 111, 117, 123,
    129, 135, 141, 147, 153, 159, 165, 171, 177, 183, 189, 195, 201, 207, 213, 219,
    225, 231, 237, 243, 249, 255, 252, 246, 240, 234, 228, 222, 216, 210, 204, 198,
    192, 186, 180, 174, 168, 162, 156, 150, 144, 138, 132, 126, 120, 114, 108, 102,
    96, 90, 84, 78, 72, 66, 60, 54, 48, 42, 36, 30, 24, 18, 12, 6};

RgbwaColor int8_hsia2rgbwa(uint8_t H, uint8_t S, uint8_t I, uint8_t A)
{
    if (I < 10 || A < 10)
    {
        return *RGBWA_TRANSPARENT;
    }

    // Calculate whiteness (desaturation)
    uint8_t W = (uint16_t)(I * (255 - S)) >> 8; // (1 - S) * I

    // C = 3 * I * S / (255 + Z)
    // Approximate Z by remapping hue sector into [0..255]
    // We'll use a simplified approach: assume Z = 1.0 (i.e., C = I * S)
    uint16_t Z = z_lut[H]; // ((uint16_t)H_sector * 6);  // scale back up to 0..255
    uint16_t C = (3 * (uint16_t)I * S) >> 8;
    uint16_t X = ((C * Z) >> 8);

    uint16_t R1 = 0, G1 = 0, B1 = 0;
    if (H < 43)
    {
        R1 = C;
        G1 = X;
        B1 = 0;
    }
    else if (H < 85)
    {
        R1 = X;
        G1 = C;
        B1 = 0;
    }
    else if (H < 128)
    {
        R1 = 0;
        G1 = C;
        B1 = X;
    }
    else if (H < 170)
    {
        R1 = 0;
        G1 = X;
        B1 = C;
    }
    else if (H < 213)
    {
        R1 = X;
        G1 = 0;
        B1 = C;
    }
    else
    {
        R1 = C;
        G1 = 0;
        B1 = X;
    }

    // Add desaturation value `m = (1 - S) * I` directly
    uint16_t m = ((uint16_t)(255 - S) * I) >> 8;

    int16_t R = R1 + m;
    int16_t G = G1 + m;
    int16_t B = B1 + m;

    // Normalize if any value exceeds 255
    uint16_t max_rgb = MAX(R, MAX(G, B));
    if (max_rgb > 255)
    {
        R = (R * 255) / max_rgb;
        G = (G * 255) / max_rgb;
        B = (B * 255) / max_rgb;
    }

    return (RgbwaColor){
        clamp8(R),
        clamp8(G),
        clamp8(B),
        W,
        A};
}

#ifndef MATH_RGBW_BY_COORDINATES

/**
 * TODO: Remove this and `lookup_z` and instead use the int8_-version
 */
RgbwaColor hsia2rgbwa(HSI_H_t H, HSI_S_t S, HSI_I_t I, HSI_A_t A)
{
    if (I <= HSI_I_NEGLIGIBLE || A <= HSI_A_NEGLIGIBLE)
    {
        return *RGBWA_TRANSPARENT;
    }

#ifdef MATH_PRECOMPUTE
    float Z = lookup_z[H];
#else
    float Z = 1 - fabs(fmod(H / (float)60, 2) - 1);
#endif
    float C = (3 * I * S) / (1 + Z);
    float X = C * Z;

    float R1;
    float G1;
    float B1;
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
    float m = I * (1 - S);
    float R = R1 + m;
    float G = G1 + m;
    float B = B1 + m;

    // Limit R, G, B to valid ranges (since not all HSI values are in gamut range):
    float max_rgb = R;
    if (G > max_rgb)
    {
        max_rgb = G;
    }
    if (B > max_rgb)
    {
        max_rgb = B;
    }

    // These two both have their strong suits, but also clipping weaknesses.
    if (max_rgb > 1)
    {
        R = R / max_rgb;
        G = G / max_rgb;
        B = B / max_rgb;
    }

    return (RgbwaColor){
        (int)round(255 * R),
        (int)round(255 * G),
        (int)round(255 * B),
        (int)(255 * ((float)((1 - S) * I))),
        RGB_ALPHA_MAX * A};
}
#else
#ifdef MATH_PRECOMPUTE
RgbwaColor hsia2rgbwa(HsiaColor *hsia)
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

    uint8_t w = 255 * ((1 - hsia->s) * i);
    uint8_t a = RGB_ALPHA_MAX * hsia->a;
    return (RgbwaColor){r, g, b, w, a};
}
#else
RgbwaColor hsia2rgbwa(HsiaColor *hsia)
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

    return (RgbwaColor){r, g, b, 255 * (1 - hsia->s) * i, hsia->a};
}
#endif
#endif