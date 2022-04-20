#include "environment.h"
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>

#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 666
#endif



