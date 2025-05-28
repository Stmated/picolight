#pragma once
#ifndef PICOL_EASING
#define PICOL_EASING

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef float (*easing)(float t);

typedef struct {
    easing in;
    /**
     * If NULL, then it will not be in InOut easing, it will only apply the In easing.
     */
    easing out;
} EasingContext;
typedef float (*easing_func)(EasingContext ctx, float t);
typedef struct {
    easing_func func;
    EasingContext ctx;
} CurriedEasing;

//float invokeEasing(EasingContext ctx, float t);
CurriedEasing getEasing(int index);

/**
 * Can give any offset and it will be used to fetch an easing
 */
CurriedEasing getRepeatingEasing(int offset);
CurriedEasing getRepeatingInOutEasing(int offset);
CurriedEasing createCurriedEasing(easing in, easing out, bool mod);

int getEasingCount();
float executeEasing(int index, float t);

float None(float p);
float Zero(float p);
float Linear(float p);
float Flip(float p);
float FlipEasing(easing easing, float p);
float InOut(easing in, easing out, float p);

float InLinear(float p);
float OutLinear(float p);
float InOutLinear(float p);

float InQuadratic(float p);
float InCubic(float p);
float InQuartic(float p);
float InQuintic(float p);
float InSextic(float p);
float InSeptic(float p);
float InOctic(float p);

float OutQuadratic(float p);
float OutCubic(float p);
float OutQuartic(float p);
float OutQuintic(float p);
float OutSextic(float p);
float OutSeptic(float p);
float OutOctic(float p);

float InOutQuadratic(float p);
float InOutCubic(float p);
float InOutQuartic(float p);
float InOutQuintic(float p);
float InOutSextic(float p);
float InOutSeptic(float p);
float InOutOctic(float p);

float InBack(float p);
float OutBack(float p);
float InOutBack(float p);

float OutBounce(float p);
float InBounce(float p);
float InOutBounce(float p);

float InCircle(float p);
float OutCircle(float p);
float InOutCircle(float p);

float InElastic(float p);
float OutElastic(float p);
float InOutElastic(float p);

float InExponent2(float p);
float OutExponent2(float p);
float InOutExponent2(float p);

float InSine(float p);
float OutSine(float p);
float InOutSine(float p);

float InExponentE(float p);
float OutExponentE(float p);
float InOutExponentE(float p);

float OutLog10(float p);
float InLog10(float p);
float InOutLog10(float p);

float OutSquareRoot(float p);
float InSquareRoot(float p);
float InOutSquareRoot(float p);

#endif