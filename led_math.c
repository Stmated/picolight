#include "led_math.h"
#include "pico/float.h"

bool gcd(int a, int b)
{
    while (b)
    {
        int t = a % b;
        a = b;
        b = t;
    }
    return a == 1;
}

int getCoPrime(int a)
{

    int coprime = -1;
    int max = a * 100;
    for (int i = a; i < max; i++)
    {
        if (gcd(a, i))
        {
            coprime = i;
            break;
        }
    }

    return coprime;
}

double getAverageAngle(double *angles, int length)
{
    double x = 0;
    double y = 0;

    for (int i = 0; i < length; i++)
    {
        double r = angles[i] * (M_PI / 180.0);
        x += cos(r);
        y += sin(r);
    }

    double radians = atan2(y, x);
    int degrees = (int)round(radians * (180.0 / M_PI));
    int fixedDegreees = (degrees + 360) % 360;
    return fixedDegreees;
}

int randint(int n)
{
    if ((n - 1) == RAND_MAX)
    {
        return rand();
    }
    else
    {
        // Chop off all of the values that would cause skew...
        int end = RAND_MAX / n; // truncate skew
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        int r;
        while ((r = rand()) >= end)
            ;

        return r % n;
    }
}

/**
 * Higher power means higher probability to be lower
 * */
int randint_probability(int min, int max, float probabilityPower)
{
    float random = randint(100000) / (float)100000;
    return (int)floorf(min + (max - min) * (powf(random, probabilityPower)));
}