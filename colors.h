#include "math.h"
#include <stdio.h>
#include <stdint.h>

// TODO:
// Use uint_least8_t or uint_fast8_t ?
// Somehow use a very small vector for colors, so we can predict future movement and use automatic dithering where applicable?

typedef struct RgbwColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
} RgbwColor;

// TODO: Convert S and I into an uint8_t, with range being 0 to 255
typedef struct HsiColor
{
    uint8_t h;
    float s;
    float i;
} HsiColor;

#define HSI_H_MIN 0
#define HSI_H_MAX 1

#define HSI_S_MIN 0
#define HSI_S_MAX 1

#define HSI_I_MIN 0
#define HSI_I_MAX 1

HsiColor LerpHSI(HsiColor *a, HsiColor *b, float t);
void hsi2rgb(float H, float S, float I, uint8_t *rgb);
void hsi2rgbw(HsiColor *hsi, RgbwColor *c);