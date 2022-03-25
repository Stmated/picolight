#include "colors.h"

#define DEG_TO_RAD(X) (M_PI * (X) / 180)

// Add "brightness" that is used to modify the values right before it it sent to the GPIO
// Add "dithering" which should be done by giving a rounding method that alternates between "round up" and "round down"

// Add "color correction"
// TypicalSMD5050 =0xFFB0F0, TypicalLEDStrip =0xFFB0F0, Typical8mmPixel =0xFFE08C, TypicalPixelString =0xFFE08C,
// UncorrectedColor =0xFFFFFF

// Need a way of converting HSV to RGBW -- where the W has dynamic warmth!

void hsi2rgb(float H, float S, float I, uint8_t *rgb)
{
    uint8_t r, g, b;
    H = fmod(H, 360);                // cycle H around to 0-360 degrees
    H = 3.14159 * H / (float)180;    // Convert to radians.
    S = S > 0 ? (S < 1 ? S : 1) : 0; // clamp S and I to interval [0,1]
    I = I > 0 ? (I < 1 ? I : 1) : 0;

    // Math! Thanks in part to Kyle Miller.
    if (H < 2.09439)
    {
        r = 255 * I / 3 * (1 + S * cos(H) / cos(1.047196667 - H));
        g = 255 * I / 3 * (1 + S * (1 - cos(H) / cos(1.047196667 - H)));
        b = 255 * I / 3 * (1 - S);
    }
    else if (H < 4.188787)
    {
        H = H - 2.09439;
        g = 255 * I / 3 * (1 + S * cos(H) / cos(1.047196667 - H));
        b = 255 * I / 3 * (1 + S * (1 - cos(H) / cos(1.047196667 - H)));
        r = 255 * I / 3 * (1 - S);
    }
    else
    {
        H = H - 4.188787;
        b = 255 * I / 3 * (1 + S * cos(H) / cos(1.047196667 - H));
        r = 255 * I / 3 * (1 + S * (1 - cos(H) / cos(1.047196667 - H)));
        g = 255 * I / 3 * (1 - S);
    }
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

void hsi2rgbw(float H, float S, float I, uint8_t *rgbw)
{
    uint8_t r, g, b, w;
    float cos_h, cos_1047_h;
    H = fmod(H, 360);                // cycle H around to 0-360 degrees
    H = 3.14159 * H / (float)180;    // Convert to radians.
    S = S > 0 ? (S < 1 ? S : 1) : 0; // clamp S and I to interval [0,1]
    I = I > 0 ? (I < 1 ? I : 1) : 0;

    if (H < 2.09439)
    {
        cos_h = cos(H);
        cos_1047_h = cos(1.047196667 - H);
        r = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
        g = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
        b = 0;
        w = 255 * (1 - S) * I;
    }
    else if (H < 4.188787)
    {
        H = H - 2.09439;
        cos_h = cos(H);
        cos_1047_h = cos(1.047196667 - H);
        g = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
        b = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
        r = 0;
        w = 255 * (1 - S) * I;
    }
    else
    {
        H = H - 4.188787;
        cos_h = cos(H);
        cos_1047_h = cos(1.047196667 - H);
        b = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
        r = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
        g = 0;
        w = 255 * (1 - S) * I;
    }

    rgbw[0] = r;
    rgbw[1] = g;
    rgbw[2] = b;
    rgbw[3] = w;
}

HsiColor LerpHSI(HsiColor a, HsiColor b, float t)
{
    // Hue interpolation
    int h;
    int d = b.h - a.h;
    if (a.h > b.h)
    {
        // Swap (a.h, b.h)
        int h3 = b.h;
        b.h = a.h;
        a.h = h3;
        d = -d;
        t = 1 - t;
    }

    if (d > 180) // 180deg
    {
        a.h = a.h + 360;
        h = ((int) (a.h + t * (b.h - a.h))) % 360;
    }
    else
    {
        h = a.h + t * d;
    }

    // Interpolates the rest
    return (HsiColor){
        h,
        a.s + t * (b.s - a.s),
        a.i + t * (b.i - a.i)};
}

RgbColor HsvToRgb(HsvColor HSV)
{
    RgbColor RGB;
    double H = HSV.h, S = HSV.s, V = HSV.v,
           P, Q, T,
           fract;

    (H == 360.) ? (H = 0.) : (H /= 60.);
    fract = H - floor(H);

    P = V * (1. - S);
    Q = V * (1. - S * fract);
    T = V * (1. - S * (1. - fract));

    if (0. <= H && H < 1.)
        RGB = (RgbColor){.r = V * 255, .g = T * 255, .b = P * 255};
    else if (1. <= H && H < 2.)
        RGB = (RgbColor){.r = Q * 255, .g = V * 255, .b = P * 255};
    else if (2. <= H && H < 3.)
        RGB = (RgbColor){.r = P * 255, .g = V * 255, .b = T * 255};
    else if (3. <= H && H < 4.)
        RGB = (RgbColor){.r = P * 255, .g = Q * 255, .b = V * 255};
    else if (4. <= H && H < 5.)
        RGB = (RgbColor){.r = T * 255, .g = P * 255, .b = V * 255};
    else if (5. <= H && H < 6.)
        RGB = (RgbColor){.r = V * 255, .g = P * 255, .b = Q * 255};
    else
        RGB = (RgbColor){.r = 0., .g = 0., .b = 0.};

    return RGB;
}