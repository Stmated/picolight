#include "easing.h"

float None(float p) { return 1; }
float Zero(float p) { return 0; }
float Linear(float p) { return p; }
float Flip(float p) { return 1 - p; }
float FlipEasing(easing easing, float p) { return Flip(easing(Flip(p))); }
float InOut(easing in, easing out, float p) { return p < 0.5 ? in(2 * p) : out(2 * Flip(p)); }
float InLinear(float p) { return Linear(p); }
float OutLinear(float p) { return FlipEasing(Linear, p); }
float InQuadratic(float p) { return p * p; }
float InCubic(float p) { return p * p * p; }
float InQuartic(float p) { return p * p * p * p; }
float InQuintic(float p) { return p * p * p * p * p; }
float InSextic(float p) { return p * p * p * p * p * p; }
float InSeptic(float p) { return p * p * p * p * p * p * p; }
float InOctic(float p) { return p * p * p * p * p * p * p * p; }
float OutQuadratic(float p) { return FlipEasing(InQuadratic, p); }
float OutCubic(float p) { return FlipEasing(InCubic, p); }
float OutQuartic(float p) { return FlipEasing(InQuartic, p); }
float OutQuintic(float p) { return FlipEasing(InQuintic, p); }
float OutSextic(float p) { return FlipEasing(InSextic, p); }
float OutSeptic(float p) { return FlipEasing(InSeptic, p); }
float OutOctic(float p) { return FlipEasing(InOctic, p); }
float InOutQuadratic(float p) { return InOut(InQuadratic, OutQuadratic, p); }
float InOutCubic(float p) { return InOut(InCubic, OutCubic, p); }
float InOutQuartic(float p) { return InOut(InQuartic, OutQuartic, p); }
float InOutQuintic(float p) { return InOut(InQuintic, OutQuintic, p); }
float InOutSextic(float p) { return InOut(InSextic, OutSextic, p); }
float InOutSeptic(float p) { return InOut(InSeptic, OutSeptic, p); }
float InOutOctic(float p) { return InOut(InOctic, OutOctic, p); }

float InOutLinear(float p) { return InOut(InLinear, OutLinear, p); }

// Standard -- grouped by Type
float InBack(float p) { return p * p * (p * (1.70158 + 1) - 1.70158); }
float OutBack(float p)
{
    float m = p - 1;
    return 1 + m * m * (m * (1.70158 + 1) + 1.70158);
}

float InOutBack(float p) { return InOut(InBack, OutBack, p); }

float OutBounce(float p)
{
    float r = 1 / 2.75;   // reciprocal
    float k1 = r;         // 36.36%
    float k2 = 2 * r;     // 72.72%
    float k3 = 1.5 * r;   // 54.54%
    float k4 = 2.5 * r;   // 90.90%
    float k5 = 2.25 * r;  // 81.81%
    float k6 = 2.625 * r; // 95.45%
    float k0 = 7.5625;
    float t;

    /**/ if (p < k1)
    {
        return k0 * p * p;
    }
    else if (p < k2)
    {
        t = p - k3;
        return k0 * t * t + 0.75;
    } // 48/64
    else if (p < k4)
    {
        t = p - k5;
        return k0 * t * t + 0.9375;
    } // 60/64
    else
    {
        t = p - k6;
        return k0 * t * t + 0.984375;
    } // 63/64
}
float InBounce(float p) { return FlipEasing(OutBounce, p); }
float InOutBounce(float p) { return InOut(InBounce, OutBounce, p); }

float InCircle(float p) { return 1 - sqrt(1 - p * p); }
float OutCircle(float p)
{
    float m = p - 1;
    return sqrt(1 - m * m);
}
float InOutCircle(float p) { return InOut(InCircle, OutCircle, p); }

float InElastic(float p)
{
    float m = p - 1;
    return -pow(2, 10 * m) * sin((m * 40 - 3) * M_PI / 6);
}
float OutElastic(float p) { return 1 + (pow(2, 10 * -p) * sin((-p * 40 - 3) * M_PI / 6)); }
float InOutElastic(float p) { return InOut(InElastic, OutElastic, p); }

// NOTE: 'Exponent2' needs clamping for 0 and 1 respectively
float InExponent2(float p) { return (p <= 0) ? 0 : pow(2, 10 * (p - 1)); }
float OutExponent2(float p) { return (p >= 1) ? 1 : 1 - pow(2, -10 * p); }
float InOutExponent2(float p) { return InOut(InExponent2, OutExponent2, p); }

float InSine(float p) { return 1 - cos(p * M_PI * 0.5); }
float OutSine(float p) { return sin(p * M_PI * 0.5); }
float InOutSine(float p) { return 0.5 * (1 - cos(p * M_PI)); }

// Non-Standard
// Scale 0..1 -> p^-10 .. p^0
float InExponentE(float p) { return (p <= 0) ? 0 : pow(M_E, -10 * (1 - p)); }
float OutExponentE(float p) { return FlipEasing(InExponentE, p); }
float InOutExponentE(float p) { return InOut(InExponentE, OutExponentE, p); }

// Scale 0..1 -> Log10( 1 ) .. Log10( 10 )
float OutLog10(float p) { return log10((p * 9) + 1); }
float InLog10(float p) { return FlipEasing(OutLog10, p); }
float InOutLog10(float p) { return InOut(InLog10, OutLog10, p); }
float OutSquareRoot(float p) { return sqrt(p); }
float InSquareRoot(float p) { return FlipEasing(OutSquareRoot, p); }
float InOutSquareRoot(float p) { return InOut(InSquareRoot, OutSquareRoot, p); }

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
    //InOctic,
    InQuadratic,
    InQuartic,
    //InQuintic,
    //InSeptic,
    //InSextic,
    //Zero
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
    //OutOctic,
    OutQuadratic,
    OutQuartic,
    //OutQuintic,
    //OutSeptic,
    //OutSextic,
    //Zero
};

const int list_in_size = (sizeof list_in / sizeof list_in[0]);
const int list_out_size = (sizeof list_out / sizeof list_out[0]);

int getEasingCount()
{
    return list_in_size * list_out_size;
}

float executeEasing(int index, float t)
{
    // TODO: This is too slow. Need a way to not do the division
    easing in = list_in[(int)floor(index / list_in_size)];
    easing out = list_out[index % list_out_size];

    return InOut(in, out, t);
}