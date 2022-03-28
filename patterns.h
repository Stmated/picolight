#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "global.h"
#include "environment.h"
#include "easing.h"
#include "options.h"

typedef void (*PatternPrinter)(uint16_t index, HsiColor *c, void *data);

typedef void (*PatternExecutor)(uint16_t len, uint32_t t, void *data, PatternPrinter printer);
typedef void *(*PatternDataCreator)(uint16_t len, float intensity);
typedef void (*PatternDataDestroyer)(void *data);

typedef void *(*PatternRegistrator)(void);

int getPatternCount();
void pattern_execute(uint16_t len, uint32_t t);
void pattern_update_data(uint16_t len, int patternIndex, float intensity);

typedef struct PatternOptions
{
    float randomChance;
} PatternOptions;

typedef struct PatternModule
{
    PatternExecutor executor;
    PatternDataCreator creator;
    PatternDataDestroyer destroyer;
    PatternOptions options;
} PatternModule;

PatternModule getPattern(int patternIndex);

void *pattern_creator_default(uint16_t len, float intensity);
void pattern_destroyer_default(void *data);

void setAll(uint16_t len, HsiColor *c, void *data, PatternPrinter printer);

void pattern_find_and_register_patterns();

void pattern_register(PatternExecutor pattern, PatternDataCreator creator, PatternDataDestroyer destroyer, PatternOptions *options);

void pattern_register_bouncer();
void pattern_register_fade_between();
void pattern_register_fill_sway();
void pattern_register_rainbow();
void pattern_register_rainbow_wave();
void pattern_register_random();
void pattern_register_snakes();
void pattern_register_sparkle();
void pattern_register_strobe();

typedef struct GlobalState
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
    PatternModule *modules;
    int modules_size;
} GlobalState;

extern GlobalState state;