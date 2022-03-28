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
 * Higher power means higher probability to be lower.
 * Weight can be anything from 0 to Inf.
 * */
int randint_weighted_towards_min(int min, int max, float weight)
{
    float random = randint(100000) / (float)100000; // 0..1
    return (int)floorf(min + (max - min) * (powf(random, weight)));
}

/**
 * Higher power means higher probability to be higher
 * Weight can be anything from 0 to Inf. 
 * */
int randint_weighted_towards_max(int min, int max, float weight)
{
    if (weight > 1)
    {
        // If weight is above 1, then we should go further below 1 the higher the number.
        weight = 1 / weight;
    }

    float random = randint(100000) / (float)100000; // 0..1
    return (int)floorf(min + (max - min) * (powf(random, weight)));
}

/**
 * Gives a value where 0 is the most likely, and numbers below and above that become increasingly unlikely.
 * The regular result is a complete tapering off after -3 and +3.
 * */
double rand_gaussian() {
	double a = ((double)(rand()))/((double)RAND_MAX);
	double b = ((double)(rand()))/((double)RAND_MAX);

	double R0 = sqrt(-2.0 * log(a)) * cos(2 * M_PI * b);
	/*
		double R1 = sqrt(-2.0 * log(a)) * sin(2 * M_PI * b);
	*/

	return R0;
}