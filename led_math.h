#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

bool gcd(int a, int b);

int getCoPrime(int a);

int randint(int n);

int randint_weighted_towards_min(int min, int max, float weight);

int randint_weighted_towards_max(int min, int max, float weight);

double getAverageAngle(double *angles, int length);