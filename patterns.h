#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#include "colors.h"

RgbColor HsvToRgb(HsvColor hsv);

typedef void (*printer)(uint16_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w, void* data);

typedef void (*pattern)(uint16_t len, uint32_t t, void* data, printer printer);
typedef void* (*pattern_data_creator)(uint16_t len, float intensity);
typedef void* (*pattern_data_destroyer)(void* data);

int getPatternCount();
void pattern_execute(int patternIndex, uint16_t len, uint32_t t, void* data);
void pattern_update_data(int patternIndex, float intensity);

pattern getPattern(int patternIndex);
pattern_data_creator getPatternCreator(int patternIndex);
pattern_data_destroyer getPatternDestroyer(int patternIndex);

struct pattern_instance;
typedef struct s_state
{
    int patternIndex;
    bool mutex;
    void* patternData;
    bool disabled;
    int clickCount;
    bool clicking;
    float speed;
    bool withOffset;
} t_state;

extern t_state state;