#ifndef PICOL_GLOBAL
#define PICOL_GLOBAL

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const int PIN_TX[] = {21}; //, 21, 20, 19 };

#define PIN_BUTTONS_START 2
#define PIN_BUTTONS_END 15
#define PIN_BUTTONS_COUNT (PIN_BUTTONS_END - PIN_BUTTONS_START)

#endif
