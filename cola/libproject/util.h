#ifndef _LIBPROJECT_UTIL_H
#define _LIBPROJECT_UTIL_H

#include <cassert>
#include <iostream>

#ifdef NDEBUG 
#define LIBPROJECT_ASSERT(expr)  static_cast<void>(0)
#else // Not NDEBUG
#ifdef USE_CASSERT
#define LIBPROJECT_ASSERT(expr)  assert(expr)
#else // Not USE_CASSERT
#define LIBPROJECT_ASSERT(expr) \
    if(!(expr)) { \
        throw CriticalFailure( \
                __STRING(expr), \
                __FILE__, \
                __LINE__, \
                __ASSERT_FUNCTION); \
    }
#endif // USE_CASSERT
#endif // NDEBUG

#ifdef LOGGING
#define LIBPROJECT_LOG(msg)  printf msg 
#else
#define LIBPROJECT_LOG(msg)  // nop
#endif

namespace project {
/**
 * templated delete functor for use in for_each loop over vector
 */
struct delete_object
{
    template <typename T>
    void operator()(T *ptr){ delete ptr;}
};

/** 
 * Sum over the results of calling operation for each member in the
 * iterator.  Handier than std::accumulate because we can use with
 * mem_fun to pass in a getter method.
 */
template <class InputIterator, class T, class Operation >
T sum_over(InputIterator beg, InputIterator end, T init, Operation op)
{
    for ( ; beg != end; ++beg)
    init = init + op(*beg);
    return init;
}

/**
 * Item 6 in Meyers' Effective C++: 
 * Explicitly disallow the use of compiler-generated functions you do not want
 */
class Uncopyable {
protected:                                   // allow construction
    Uncopyable() {}                            // and destruction of
    ~Uncopyable() {}                           // derived objects...
private:
    Uncopyable(const Uncopyable&);             // ...but prevent copying
    Uncopyable& operator=(const Uncopyable&);
};

extern const double epsilon;
bool approx_equals(double a, double b);

} // namespace project

#endif // _LIBPROJECT_UTIL_H
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
