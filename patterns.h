#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "global.h"
#include "environment/environment.h"
#include "easing.h"
#include "options.h"
#include "led_math.h"

typedef void (*PatternPrinter)(uint16_t index, HsiColor *c, void *dataPtr);

// TODO: Make it possible for Executor to take "offset" and "len" -- so we can execute one pixel at a time, and avoid saving a pixel buffer if merging patterns
//          Or do a few pixels at a time, in different threads or whatnot.
//          Need a new concept for a data structure to speed things up, like a "CycleData" -- so we have that and a "PatternData"
//          We must have this, otherwise we'd need to do some calculations commons to a cycle way too often
typedef void (*PatternExecutor)(uint16_t offset, uint16_t len, uint32_t t, void *dataPtr, void *cyclePtr, PatternPrinter printer);
typedef void *(*PatternDataCreator)(uint16_t len, float intensity);
typedef void (*PatternDataDestroyer)(void *dataPtr);
typedef void *(*PatternCycleDataCreator)(uint16_t len, uint32_t t, void *dataPtr);
typedef void (*PatternCycleDataDestroyer)(void *cyclePtr);

typedef void *(*PatternRegistrator)(void);

int getPatternCount();
void pattern_execute(uint16_t len, uint32_t t);
void pattern_update_data(uint16_t len, int patternIndex, float intensity);

typedef struct PatternOptions
{
    float randomChance;
    int randomMultiplier;
} PatternOptions;

typedef struct PatternModule
{
    const char *name;
    PatternExecutor executor;
    PatternDataCreator creator;
    PatternDataDestroyer destroyer;
    PatternCycleDataCreator cycleCreator;
    PatternCycleDataDestroyer cycleDestroyer;
    PatternOptions *options;
} PatternModule;

typedef struct data_pixels_struct
{
    HsiColor *pixels;

    // TODO: Remove this -- instead rename "progress" to "blend" and use only that to set the 100% the first pixels
    // DEPRECATED! NOT USED!
    bool subsequent;

    // TODO: This needs to be rewritten somehow, so it can handle X amounts of different merging printers.
    float progress;

    // TODO: Remove this -- instead rename "progress" to "blend" and use that correctly to decide how to assign pixels
    bool progressReversed;

} data_pixels_struct;

PatternModule *getPatternByIndex(int index);
PatternModule *getPatternByName(const char* name);

void *pattern_creator_default(uint16_t len, float intensity);
void pattern_destroyer_default(void *data);

void *pattern_cycle_creator_default(uint16_t len, uint32_t t, void *dataPtr);
void pattern_cycle_destroyer_default(void *data);

void pattern_printer_default(uint16_t index, HsiColor *c, void *dataPtr);
void pattern_printer_merging(uint16_t index, HsiColor *c, void *dataPtr);

void setAll(uint16_t offset, uint16_t len, HsiColor *c, void *dataPtr, void *cyclePtr, PatternPrinter printer);

void pattern_find_and_register_patterns();

void pattern_register(const char *name, PatternExecutor pattern, PatternDataCreator creator, PatternDataDestroyer destroyer, PatternOptions *options);

void pattern_register_snake();
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