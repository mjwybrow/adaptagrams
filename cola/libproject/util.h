#ifndef _LIBPROJECT_UTIL_H
#define _LIBPROJECT_UTIL_H

#include <cassert>
#include <iostream>

#ifdef NDEBUG 
#define LIBPROJECT_ASSERT(expr)  (__ASSERT_VOID_CAST (0))
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

struct CriticalFailure {
	CriticalFailure(const char *expr, 
			const char *file, 
			int line, 
			const char *function)
		: expr(expr), file(file), line(line), function(function)
	{}
	void print() {
		fprintf(stderr,"ERROR: Critical sanity check failed in libproject!\n"
				"  expression: %s\n  at line %d of %s\n"
				"  in: %s\n", expr, line, file, function);
	}
	const char *expr;
	const char *file;
	int line;
	const char *function;
};
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

#ifndef NDEBUG
bool approx_equals(double a, double b);
#endif // NDEBUG

#endif // _LIBPROJECT_UTIL_H
