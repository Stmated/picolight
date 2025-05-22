#pragma once
#ifndef PICOL_TYPES
#define PICOL_TYPES

#include <stdio.h>
#include <stdint.h>

// TODO:
// Use uint_least8_t or uint_fast8_t ?
// Somehow use a very small vector for colors, so we can predict future movement and use automatic dithering where applicable?

#define CHANNEL_MAX 255
#define RGB_ALPHA_MAX CHANNEL_MAX
#define ALPHA_NEGLIGIBLE_MAX 245

typedef struct RgbwColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
} RgbwColor;

typedef struct RgbwaColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
    uint8_t a;
} RgbwaColor;

#define HSIA_S float

// TODO: Convert S and I into an uint8_t, with range being 0 to 255.
//       Also convert `h` into uint8_t -- we can live with a little loss in precision
typedef struct HsiaColor
{
    uint16_t h;
    float s;
    float i;
    float a;
} HsiaColor;

extern HsiaColor transparent;
extern HsiaColor white;

#endif
