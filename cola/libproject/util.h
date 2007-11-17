#ifndef _PROJECT_FEASIBLE_PROJECT_H
#define _PROJECT_FEASIBLE_PROJECT_H

struct delete_object
{
  template <typename T>
  void operator()(T *ptr){ delete ptr; }
};

#ifndef NDEBUG
#include <cmath>
static const double epsilon=1e-8;
bool approx_equals(double a, double b) {
    return fabs(a-b)<epsilon;
}
#endif

#endif // _PROJECT_UTIL_H
