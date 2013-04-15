/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libtopology - Classes used in generating and managing topology constraints.
 *
 * Copyright (C) 2007-2008  Monash University
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

#ifndef TOPOLOGY_UTIL_H
#define TOPOLOGY_UTIL_H
namespace topology {
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
} // namespace topology
#endif // TOPOLOGY_UTIL_H
