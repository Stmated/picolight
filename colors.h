#include "math.h"
#include <stdio.h>
#include <stdint.h>

typedef struct RgbColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RgbColor;

typedef struct RgbwColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
} RgbwColor;

typedef struct HsvColor
{
    int h;
    double s;
    double v;
} HsvColor;

typedef struct HsiColor
{
    int h;
    double s;
    double i;
} HsiColor;

HsiColor LerpHSI(HsiColor a, HsiColor b, float t);
void hsi2rgb(float H, float S, float I, uint8_t *rgb);
void hsi2rgbw(float H, float S, float I, uint8_t *rgbw);