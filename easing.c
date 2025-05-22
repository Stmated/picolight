#include "easing.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

inline float None(float p) { return 1; }
inline float Zero(float p) { return 0; }
inline float Linear(float p) { return p; }
inline float Flip(float p) { return 1 - p; }
inline float FlipEasing(easing easing, float p) { return Flip(easing(Flip(p))); }
inline float InOut(easing in, easing out, float p) { return p < 0.5 ? in(2 * p) : out(2 * Flip(p)); }
inline float InLinear(float p) { return Linear(p); }
inline float OutLinear(float p) { return FlipEasing(Linear, p); }
inline float InQuadratic(float p) { return p * p; }
inline float InCubic(float p) { return p * p * p; }
inline float InQuartic(float p) { return p * p * p * p; }
inline float InQuintic(float p) { return p * p * p * p * p; }
inline float InSextic(float p) { return p * p * p * p * p * p; }
inline float InSeptic(float p) { return p * p * p * p * p * p * p; }
inline float InOctic(float p) { return p * p * p * p * p * p * p * p; }
inline float OutQuadratic(float p) { return FlipEasing(InQuadratic, p); }
inline float OutCubic(float p) { return FlipEasing(InCubic, p); }
inline float OutQuartic(float p) { return FlipEasing(InQuartic, p); }
inline float OutQuintic(float p) { return FlipEasing(InQuintic, p); }
inline float OutSextic(float p) { return FlipEasing(InSextic, p); }
inline float OutSeptic(float p) { return FlipEasing(InSeptic, p); }
inline float OutOctic(float p) { return FlipEasing(InOctic, p); }
inline float InOutQuadratic(float p) { return InOut(InQuadratic, OutQuadratic, p); }
inline float InOutCubic(float p) { return InOut(InCubic, OutCubic, p); }
inline float InOutQuartic(float p) { return InOut(InQuartic, OutQuartic, p); }
inline float InOutQuintic(float p) { return InOut(InQuintic, OutQuintic, p); }
inline float InOutSextic(float p) { return InOut(InSextic, OutSextic, p); }
inline float InOutSeptic(float p) { return InOut(InSeptic, OutSeptic, p); }
inline float InOutOctic(float p) { return InOut(InOctic, OutOctic, p); }

inline float InOutLinear(float p) { return InOut(InLinear, OutLinear, p); }

// Standard -- grouped by Type
inline float InBack(float p) { return p * p * (p * (1.70158 + 1) - 1.70158); }
inline float OutBack(float p)
{
    float m = p - 1;
    return 1 + m * m * (m * (1.70158 + 1) + 1.70158);
}

inline float InOutBack(float p) { return InOut(InBack, OutBack, p); }

// const float r = 1 / 2.75;   // reciprocal
const float k1 = (1 / 2.75);         // 36.36%
const float k2 = 2 * (1 / 2.75);     // 72.72%
const float k3 = 1.5 * (1 / 2.75);   // 54.54%
const float k4 = 2.5 * (1 / 2.75);   // 90.90%
const float k5 = 2.25 * (1 / 2.75);  // 81.81%
const float k6 = 2.625 * (1 / 2.75); // 95.45%
const float k0 = 7.5625;

inline float OutBounce(float p)
{
    if (p < k1)
    {
        return k0 * p * p;
    }
    else if (p < k2)
    {
        float t = p - k3;
        return k0 * t * t + 0.75;
    }
    else if (p < k4)
    {
        float t = p - k5;
        return k0 * t * t + 0.9375;
    }
    else
    {
        float t = p - k6;
        return k0 * t * t + 0.984375;
    }
}
inline float InBounce(float p) { return FlipEasing(OutBounce, p); }
inline float InOutBounce(float p) { return InOut(InBounce, OutBounce, p); }

inline float InCircle(float p) { return 1 - sqrt(1 - p * p); }
inline float OutCircle(float p)
{
    float m = p - 1;
    return sqrt(1 - m * m);
}
inline float InOutCircle(float p) { return InOut(InCircle, OutCircle, p); }

inline float InElastic(float p)
{
    float m = p - 1;
    return -pow(2, 10 * m) * sin((m * 40 - 3) * M_PI / 6);
}
inline float OutElastic(float p) { return 1 + (pow(2, 10 * -p) * sin((-p * 40 - 3) * M_PI / 6)); }
inline float InOutElastic(float p) { return InOut(InElastic, OutElastic, p); }

// NOTE: 'Exponent2' needs clamping for 0 and 1 respectively
inline float InExponent2(float p) { return (p <= 0) ? 0 : pow(2, 10 * (p - 1)); }
inline float OutExponent2(float p) { return (p >= 1) ? 1 : 1 - pow(2, -10 * p); }
inline float InOutExponent2(float p) { return InOut(InExponent2, OutExponent2, p); }

inline float InSine(float p) { return 1 - cos(p * M_PI * 0.5); }
inline float OutSine(float p) { return sin(p * M_PI * 0.5); }
inline float InOutSine(float p) { return 0.5 * (1 - cos(p * M_PI)); }

// Non-Standard
// Scale 0..1 -> p^-10 .. p^0
inline float InExponentE(float p) { return (p <= 0) ? 0 : pow(M_E, -10 * (1 - p)); }
inline float OutExponentE(float p) { return FlipEasing(InExponentE, p); }
inline float InOutExponentE(float p) { return InOut(InExponentE, OutExponentE, p); }

// Scale 0..1 -> Log10( 1 ) .. Log10( 10 )
inline float OutLog10(float p) { return log10((p * 9) + 1); }
inline float InLog10(float p) { return FlipEasing(OutLog10, p); }
inline float InOutLog10(float p) { return InOut(InLog10, OutLog10, p); }
inline float OutSquareRoot(float p) { return sqrt(p); }
inline float InSquareRoot(float p) { return FlipEasing(OutSquareRoot, p); }
inline float InOutSquareRoot(float p) { return InOut(InSquareRoot, OutSquareRoot, p); }

const easing list_in[] = {
    InLinear,
    InBack,
    InBounce,
    InCircle,
    InCubic,
    InElastic,
    InSine,
    InSquareRoot,
    InExponent2,
    InExponentE,
    InLog10,
    InQuadratic,
    InQuartic,
};

const easing list_out[] = {
    OutLinear,
    OutBack,
    OutBounce,
    OutCircle,
    OutCubic,
    OutElastic,
    OutSine,
    OutSquareRoot,
    OutExponent2,
    OutExponentE,
    OutLog10,
    OutQuadratic,
    OutQuartic,
};

const int list_in_size = (sizeof list_in / sizeof list_in[0]);
const int list_out_size = (sizeof list_out / sizeof list_out[0]);
const int easing_count = list_in_size * list_out_size;

int getEasingCount()
{
    return easing_count;
}

float invokeEasing(EasingContext context, float t)
{
    return InOut(context.in, context.out, t);
}

/**
 * Will take any `t` and normalize it to be between 0..1
 */
float invokeOverflowingEasing(EasingContext context, float t)
{
    if (context.out == NULL)
    {
        return context.in(fmodf(t, 1));
    }
    else
    {
        return InOut(context.in, context.out, fmodf(t, 1));
    }
}

CurriedEasing createCurriedEasing(easing in, easing out, bool mod)
{
    EasingContext ctx;
    ctx.in = in;
    ctx.out = out;

    CurriedEasing easing;
    easing.func = mod ? invokeOverflowingEasing : invokeEasing;
    easing.ctx = ctx;

    return easing;
}

CurriedEasing getSpecificEasing(int inIndex, int outIndex, bool mod)
{
    return createCurriedEasing(list_in[inIndex], (outIndex == -1) ? NULL : list_out[outIndex], mod);
}

CurriedEasing getEasing(int index)
{
    int inIndex = ((int)floor(index / list_in_size)) % list_in_size;
    int outIndex = index % list_out_size;

    return getSpecificEasing(inIndex, outIndex, false);
}

CurriedEasing getRepeatingEasing(int offset)
{
    return getSpecificEasing(offset % list_in_size, -1, true);
}

CurriedEasing getRepeatingInOutEasing(int offset)
{
    int inIndex = ((int)floor(offset / list_in_size)) % list_in_size;
    int outIndex = offset % list_out_size;

    return getSpecificEasing(inIndex, outIndex, true);
}

// TODO: Might need to do some optimizations to use pointers? So that we don't always send the context as value!
//          Should be safe to make the `invoke` functions take a pointer to the context instead!

/**
 * Prefer to use `getEasing` and `invokeEasing` instead, so we only do some calculations once
 */
float executeEasing(int index, float t)
{
    // TODO: This is too slow. Need a way to not do the division
    easing in = list_in[(int)floor(index / list_in_size)];
    easing out = list_out[index % list_out_size];

    return InOut(in, out, t);
}