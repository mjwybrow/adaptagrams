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

#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H
#include <vector>
#include <valarray>

namespace hull {
void convex(const unsigned n, const double* X, const double* Y, std::vector<unsigned> & hull);
void convex(const std::valarray<double> & X, const std::valarray<double> & Y, std::vector<unsigned> & hull);
}

#endif // CONVEX_HULL_H
