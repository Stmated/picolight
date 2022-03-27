#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "global.h"
#include "environment.h"
#include "easing.h"
#include "options.h"

typedef void (*printer)(uint16_t index, HsiColor *c, void *data);

typedef void (*pattern)(uint16_t len, uint32_t t, void *data, printer printer);
typedef void *(*pattern_data_creator)(uint16_t len, float intensity);
typedef void (*pattern_data_destroyer)(void *data);

typedef void *(*pattern_registrator)(void);

int getPatternCount();
void pattern_execute(uint16_t len, uint32_t t);
void pattern_update_data(uint16_t len, int patternIndex, float intensity);

typedef struct pattern_module
{
    pattern pat;
    pattern_data_creator creator;
    pattern_data_destroyer destroyer;
} pattern_module;

pattern_module getPattern(int patternIndex);
// pattern_data_creator getPatternCreator(int patternIndex);
// pattern_data_destroyer getPatternDestroyer(int patternIndex);

void *pattern_creator_default(uint16_t len, float intensity);
void pattern_destroyer_default(void *data);

void setAll(uint16_t len, HsiColor *c, void *data, printer printer);

void pattern_find_and_register_patterns();

void pattern_register(pattern pattern, pattern_data_creator creator, pattern_data_destroyer destroyer);

void pattern_register_bouncer();
void pattern_register_fade_between();
void pattern_register_fill_sway();
void pattern_register_rainbow_wave();
void pattern_register_random();
void pattern_register_snakes();
void pattern_register_sparkle();
void pattern_register_strobe();

typedef struct s_state
{
    int patternIndex;
    void *patternData;
    bool disabled;
    int clickCount;
    bool clicking;
    float speed;
    bool withOffset;

    int nextPatternIndex;
    float nextIntensity;
    pattern_module *modules;
    int modules_size;
} t_state;

extern t_state state;