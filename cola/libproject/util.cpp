#include "util.h"

#ifndef NDEBUG
#include <cmath>
static const double epsilon=1e-8;
bool approx_equals(double a, double b) {
    return fabs(a-b)<epsilon;
}
#endif // NDEBUG

