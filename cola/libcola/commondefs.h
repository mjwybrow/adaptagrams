#ifndef _COMMONDEFS_H
#define _COMMONDEFS_H

/* Notes to compilation using MSVC.

   Compiler options:
      /Zc:forScope
*/ 
// magmy20070405: Added for MSVC
#if defined(_MSC_VER)
// Microsoft Visual C++ (MS VC) specific code goes here
#include <float.h>
#include <malloc.h>     // Contains _alloca
namespace std {
inline bool isnan(double const &x) { return _isnan(x) != 0; }
inline bool isinf(double const &x) { return !(_finite(x) || _isnan(x)); }
} // end std

#endif

#include <valarray>
#include <cassert>

namespace cola {
/**
 * resolve overlaps:
 *   None: not at all
 *   Horizontal: only horizontally
 *   Both: resolve in either Horizontal or Vertical direction
 *         depending on which leads to less displacement
 */
enum NonOverlapConstraints { None, Horizontal, Both };
enum Dim { HORIZONTAL, VERTICAL };
class FixedList {
public:
    FixedList(const unsigned n) : array(std::valarray<bool>(n)),allFixed(false) 
    { 
        array=false; 
    }
    void set(const unsigned i, const bool value=true) {
        assert(i<array.size());
        array[i]=value;
    }
    bool check(const unsigned i) const {
        if(allFixed||i>=array.size()) {
            return false;
        }
        return array[i];
    }
    void unsetAll() {
        array=false;
    }
    void fixAll(bool val) {
	    allFixed=val;
    }
private:
    std::valarray<bool> array;
    bool allFixed;
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
} // namespace cola

#endif