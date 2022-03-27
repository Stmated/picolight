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

// This method has to be blazingly fast! Find ways to spee it up if possible!
void hsi2rgbw(HsiColor* hsi, RgbwColor *c)
{
    uint8_t r, g, b, w;
    float cos_h, cos_1047_h;
    float H = hsi->h; // fmod(hsi->h, 360);                // cycle H around to 0-360 degrees
    H = 3.14159 * H / (float)180;    // Convert to radians.
    float S = hsi->s; // hsi->s > 0 ? (hsi->s < 1 ? hsi->s : 1) : 0; // clamp S and I to interval [0,1]
    float I = hsi->i; // hsi->i > 0 ? (hsi->i < 1 ? hsi->i : 1) : 0;

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