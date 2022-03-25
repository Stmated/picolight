#include "led_math.h"

inline bool gcd(int a, int b)
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