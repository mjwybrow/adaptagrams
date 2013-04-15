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

#ifndef _CONJUGATE_GRADIENT_H
#define _CONJUGATE_GRADIENT_H

#include <valarray>

double
inner(std::valarray<double> const &x, 
      std::valarray<double> const &y);

void 
conjugate_gradient(std::valarray<double> const &A, 
           std::valarray<double> &x, 
           std::valarray<double> const &b, 
           unsigned const n, double const tol,
           unsigned const max_iterations);
#endif // _CONJUGATE_GRADIENT_H
