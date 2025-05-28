#pragma once
#ifndef PICOL_GLOBAL
#define PICOL_GLOBAL

#include <stdbool.h>
#include <stdint.h>

#include "patterns.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const int PIN_TX[] = {21};

typedef struct GlobalState
{
    int patternIndex;
    void *patternData;
    void *frameData;

    float speed;
    bool withOffset;

    int nextPatternIndex;
    float intensity;
    PatternModule *modules;
    int modules_size;

    int ledCount;
    int nextLedCount;

    uint8_t* buffer;
    bool buffer_invalidated;
} GlobalState;

extern GlobalState state;

#endif