#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "global.h"
#include "environment/environment.h"
#include "easing.h"
#include "options.h"

struct Printer;

//typedef void (*PrinterFunction)(uint16_t index, HsiaColor *c, struct Printer *printer);

// TODO: Add a way of being able to send different color-spaces to the printer, and they are propagated upwards.
//       This way we can send RGBW directly if that is all we need, and skip any translation from HSIA to RGBW
//       Could theoretically add more obscure color spaces then, like CIECAM02

#define COLOR_TRANSPARENT (HsiaColor) {0, 0, 0, 0}
#define COLOR_WHITE (HsiaColor) {0, 0, 1, 1}
#define COLOR_BLACK (HsiaColor) {0, 0, 0, 1}

typedef struct ExecutorArgs
{
    uint16_t i;
    void *dataPtr;
    void *framePtr;
} ExecutorArgs;

// TODO: Make it possible for Executor to take "offset" and "len" -- so we can execute one pixel at a time, and avoid saving a pixel buffer if merging patterns
//          Or do a few pixels at a time, in different threads or whatnot.
//          Need a new concept for a data structure to speed things up, like a "FrameData" -- so we have that and a "PatternData"
//          We must have this, otherwise we'd need to do some calculations commons to a frame way too often
typedef RgbwaColor (*PatternExecutor)(ExecutorArgs *args);
typedef void *(*PatternDataCreator)(uint16_t len, float intensity);
typedef void (*PatternDataDestroyer)(void *dataPtr);

// TODO: Create a new FrameDataUpdater instead of always creating and destroying. Faster to update than to alloc and free every frame.
typedef void *(*PatternFrameDataAllocator)(uint16_t len, uint32_t t, void *dataPtr);
typedef void *(*PatternFrameDataCreator)(uint16_t len, uint32_t t, void *dataPtr, void *framePtr);
typedef void (*PatternFrameDataDestroyer)(void *dataPtr, void *framePtr);

typedef void *(*PatternRegistrator)(void);

int getPatternCount();
void pattern_execute(uint16_t len, uint32_t t);
void pattern_update_data(uint16_t len, int patternIndex, float intensity);

typedef struct PatternOptions
{
    float randomChance;
    int randomMultiplier;
    bool opaque;
} PatternOptions;

typedef struct PatternModule
{
    const char *name;
    PatternExecutor executor;
    PatternDataCreator creator;
    PatternDataDestroyer destroyer;
    PatternFrameDataAllocator frameAllocator;
    PatternFrameDataCreator frameCreator;
    PatternFrameDataDestroyer frameDestroyer;
    PatternOptions options;
} PatternModule;

PatternModule *pattern_get_by_index(int index);
PatternModule *pattern_get_by_name(const char* name);

void *pattern_creator_default(uint16_t len, float intensity);
void pattern_destroyer_default(void *data);

void *pattern_frame_creator_default(uint16_t len, uint32_t t, void *dataPtr);
void pattern_frame_destroyer_default(void *data, void *framePtr);
//HsiaColor pattern_executor_default(uint16_t i, void *dataPtr, void *framePtr);

void pattern_find_and_register_patterns();

void pattern_register(const char *name, PatternExecutor pattern, PatternDataCreator creator, PatternDataDestroyer destroyer, PatternFrameDataAllocator frameAllocator, PatternFrameDataCreator frameCreator, PatternFrameDataDestroyer frameDestroyer, PatternOptions options);

void pattern_register_test();
void pattern_register_snake();
void pattern_register_fade_between();
void pattern_register_color_lerp();
void pattern_register_hue_lerp();
void pattern_register_rainbow_wave();
void pattern_register_snakes();
void pattern_register_sparkle();
void pattern_register_gas_fade();
void pattern_register_strobe();
void pattern_register_knightrider();
void pattern_register_firework();
void pattern_register_meteor();
void pattern_register_eyes();

void pattern_register_random();
void pattern_register_random_sequence();

typedef struct GlobalState
{
    int patternIndex;
    void *patternData;
    void *frameData;
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