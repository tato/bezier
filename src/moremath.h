#include <math.h>

#define TAU 6.283185307179586

double binomial(double n, double k)
{
    double denom = tgamma(k + 1) * tgamma(n - k + 1);
    return tgamma(n + 1) / denom;
}
