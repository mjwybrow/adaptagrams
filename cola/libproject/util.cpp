#include "util.h"

#ifndef NDEBUG
#include <cmath>
namespace project {
static const double epsilon=1e-8;
bool approx_equals(double a, double b) {
    return fabs(a-b)<epsilon;
}
} // namespace project
#endif // NDEBUG

