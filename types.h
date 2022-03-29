#include <stdio.h>
#include <stdint.h>

// TODO:
// Use uint_least8_t or uint_fast8_t ?
// Somehow use a very small vector for colors, so we can predict future movement and use automatic dithering where applicable?

typedef struct RgbwColor
{
    uint_least8_t r;
    uint_least8_t g;
    uint_least8_t b;
    uint_least8_t w;
} RgbwColor;

// TODO: Convert S and I into an uint8_t, with range being 0 to 255
typedef struct HsiColor
{
    float s;
    float i;
    // TODO: Change h into a float with 0..1 -- should normalize a lot of calculations? Or S and I to int?
    uint16_t h;
} HsiColor;

typedef struct HsiColorWide
{
    float s;
    float i;
    // TODO: Change h into a float with 0..1 -- should normalize a lot of calculations? Or S and I to int?
    uint64_t h;
} HsiColorWide;