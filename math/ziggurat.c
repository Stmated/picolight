#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ziggurat.h"

// CONG_SEEDED evaluates the CONG congruential random number generator.
// Input/output, uint32_t *JCONG, the seed, which is updated on each call.
// Output, uint32_t CONG_SEEDED, the new value.
uint32_t cong_seeded(uint32_t *jcong)
{
    uint32_t value;

    *jcong = 69069 * (*jcong) + 1234567;

    value = *jcong;

    return value;
}

// KISS_SEEDED evaluates the KISS random number generator.
// Input/output, uint32_t *JCONG, uint32_t *JSR, uint32_t *W, uint32_t *Z, the seeds, which are updated on each call.
// Output, uint32_t KISS_SEEDED, the new value.
uint32_t kiss_seeded(uint32_t *jcong, uint32_t *jsr, uint32_t *w, uint32_t *z)
{
    uint32_t value;

    value = (mwc_seeded(w, z) ^ cong_seeded(jcong)) + shr3_seeded(jsr);

    return value;
}

// MWC_SEEDED evaluates the MWC multiply-with-carry random number generator.
// Input/output, uint32_t *W, uint32_t *Z, the seeds, which are updated on each call.
// Output, uint32_t MWC_SEEDED, the new value.
uint32_t mwc_seeded(uint32_t *w, uint32_t *z)
{
    uint32_t value;

    *z = 36969 * (*z & 65535) + (*z >> 16);
    *w = 18000 * (*w & 65535) + (*w >> 16);

    value = (*z << 16) + *w;

    return value;
}

// R4_EXP returns an exponentially distributed single precision real value.
// Input/output, uint32_t *JSR, the seed.
// Input, uint32_t KE[256], data computed by R4_EXP_SETUP.
// Input, float FE[256], WE[256], data computed by R4_EXP_SETUP.
// Output, float R4_EXP, an exponentially distributed random value.
float r4_exp(uint32_t *jsr, uint32_t ke[256], float fe[256], float we[256])

{
    uint32_t iz;
    uint32_t jz;
    float value;
    float x;

    jz = shr3_seeded(jsr);
    iz = (jz & 255);

    if (jz < ke[iz])
    {
        value = (float)(jz)*we[iz];
    }
    else
    {
        for (;;)
        {
            if (iz == 0)
            {
                value = 7.69711 - log(r4_uni(jsr));
                break;
            }

            x = (float)(jz)*we[iz];

            if (fe[iz] + r4_uni(jsr) * (fe[iz - 1] - fe[iz]) < exp(-x))
            {
                value = x;
                break;
            }

            jz = shr3_seeded(jsr);
            iz = (jz & 255);

            if (jz < ke[iz])
            {
                value = (float)(jz)*we[iz];
                break;
            }
        }
    }
    return value;
}

// R4_EXP_SETUP sets data needed by R4_EXP.
// Output, uint32_t KE[256], data needed by R4_EXP.
// Output, float FE[256], WE[256], data needed by R4_EXP.
void r4_exp_setup(uint32_t ke[256], float fe[256], float we[256])
{
    double de = 7.697117470131487;
    int i;
    const double m2 = 2147483648.0;
    double q;
    double te = 7.697117470131487;
    const double ve = 3.949659822581572E-03;

    q = ve / exp(-de);

    ke[0] = (uint32_t)((de / q) * m2);
    ke[1] = 0;

    we[0] = (float)(q / m2);
    we[255] = (float)(de / m2);

    fe[0] = 1.0;
    fe[255] = (float)(exp(-de));

    for (i = 254; 1 <= i; i--)
    {
        de = -log(ve / de + exp(-de));
        ke[i + 1] = (uint32_t)((de / te) * m2);
        te = de;
        fe[i] = (float)(exp(-de));
        we[i] = (float)(de / m2);
    }
    return;
}

// R4_NOR returns a normally distributed single precision real value.
// Input/output, uint32_t *JSR, the seed.
// Input, uint32_t KN[128], data computed by R4_NOR_SETUP.
// Input, float FN[128], WN[128], data computed by R4_NOR_SETUP.
// Output, float R4_NOR, a normally distributed random value.
float r4_nor(uint32_t *jsr, uint32_t kn[128], float fn[128], float wn[128])
{
    int hz;
    uint32_t iz;
    const float r = 3.442620;
    float value;
    float x;
    float y;

    hz = (int)shr3_seeded(jsr);
    iz = (hz & 127);

    if (fabs(hz) < kn[iz])
    {
        value = (float)(hz)*wn[iz];
    }
    else
    {
        for (;;)
        {
            if (iz == 0)
            {
                for (;;)
                {
                    x = -0.2904764 * log(r4_uni(jsr));
                    y = -log(r4_uni(jsr));
                    if (x * x <= y + y)
                    {
                        break;
                    }
                }

                if (hz <= 0)
                {
                    value = -r - x;
                }
                else
                {
                    value = +r + x;
                }
                break;
            }

            x = (float)(hz)*wn[iz];

            if (fn[iz] + r4_uni(jsr) * (fn[iz - 1] - fn[iz]) < exp(-0.5 * x * x))
            {
                value = x;
                break;
            }

            hz = (int)shr3_seeded(jsr);
            iz = (hz & 127);

            if (fabs(hz) < kn[iz])
            {
                value = (float)(hz)*wn[iz];
                break;
            }
        }
    }

    return value;
}

// R4_NOR_SETUP sets data needed by R4_NOR.
// Output, uint32_t KN[128], data needed by R4_NOR.
// Output, float FN[128], WN[128], data needed by R4_NOR.
void r4_nor_setup(uint32_t kn[128], float fn[128], float wn[128])
{
    double dn = 3.442619855899;
    int i;
    const double m1 = 2147483648.0;
    double q;
    double tn = 3.442619855899;
    const double vn = 9.91256303526217E-03;

    q = vn / exp(-0.5 * dn * dn);

    kn[0] = (uint32_t)((dn / q) * m1);
    kn[1] = 0;

    wn[0] = (float)(q / m1);
    wn[127] = (float)(dn / m1);

    fn[0] = 1.0;
    fn[127] = (float)(exp(-0.5 * dn * dn));

    for (i = 126; 1 <= i; i--)
    {
        dn = sqrt(-2.0 * log(vn / dn + exp(-0.5 * dn * dn)));
        kn[i + 1] = (uint32_t)((dn / tn) * m1);
        tn = dn;
        fn[i] = (float)(exp(-0.5 * dn * dn));
        wn[i] = (float)(dn / m1);
    }

    return;
}

// R4_UNI returns a uniformly distributed real value.
// Input, uint32_t *JSR, the seed.
// Output, float R4_UNI, a uniformly distributed random value in the range [0,1].
float r4_uni(uint32_t *jsr)
{
    uint32_t jsr_input;
    float value;

    jsr_input = *jsr;

    *jsr = (*jsr ^ (*jsr << 13));
    *jsr = (*jsr ^ (*jsr >> 17));
    *jsr = (*jsr ^ (*jsr << 5));

    value = fmod(0.5 + (float)(jsr_input + *jsr) / 65536.0 / 65536.0, 1.0);

    return value;
}

// SHR3_SEEDED evaluates the SHR3 generator for integers.
// Input/output, uint32_t *JSR, the seed, which is updated on each call.
// Output, uint32_t SHR3_SEEDED, the new value.
uint32_t shr3_seeded(uint32_t *jsr)
{
    uint32_t value;

    value = *jsr;

    *jsr = (*jsr ^ (*jsr << 13));
    *jsr = (*jsr ^ (*jsr >> 17));
    *jsr = (*jsr ^ (*jsr << 5));

    value = value + *jsr;

    return value;
}