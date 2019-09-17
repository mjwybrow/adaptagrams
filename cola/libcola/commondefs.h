/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

#ifndef _COMMONDEFS_H
#define _COMMONDEFS_H

/* Notes to compilation using MSVC.

   Compiler options:
      /Zc:forScope
*/ 
#if defined(_MSC_VER)
// Microsoft Visual C++ (MS VC) specific code goes here
#include <float.h>
#include <malloc.h>     // Contains _alloca
#endif

#include "libvpsc/assertions.h"
#include <valarray>

namespace cola {

/*
 * resolve overlaps:
 *   None: not at all
 *   Horizontal: only horizontally
 *   Both: resolve in either Horizontal or Vertical direction
 *         depending on which leads to less displacement
 */
enum NonOverlapConstraintsMode { None, Horizontal, Both };

class FixedList {
public:
    FixedList(const unsigned n) : array(std::valarray<bool>(n)),allFixed(false) 
    { 
        array=false; 
    }
    void set(const unsigned i, const bool value=true) {
        COLA_ASSERT(i<array.size());
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

/*
 * templated delete functor for use in for_each loop over vector
 */
struct delete_object
{
  template <typename T>
  void operator()(T *ptr){ delete ptr;}
};
/* 
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
