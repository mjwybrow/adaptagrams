/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006  Nathan Hurst <njh@njhurst.com>
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
 * Author(s):  Nathan Hurst
 *             Tim Dwyer
 *            
*/


#include <cmath>
#include <cstdlib>
#include <cassert>
#include <valarray>

#include "libvpsc/assertions.h"
#include "libcola/commondefs.h"
#include "libcola/conjugate_gradient.h"

/* lifted wholely from wikipedia.  Well, apart from the bug in the wikipedia version. */

using std::valarray;

static void 
matrix_times_vector(valarray<double> const &matrix, /* m * n */
            valarray<double> const &vec,  /* n */
            valarray<double> &result) /* m */
{
    unsigned n = vec.size();
    unsigned m = result.size();
    COLA_ASSERT(m*n == matrix.size());
#   if defined(_MSC_VER)
    // magmy: The following lines show how operator[] is defined for valarray under MSVC
    // _Ty valarray<_Ty>::operator[](size_t _Off) const;
    // _Ty &valarray<_Ty>::operator[](size_t _Off);
    // As a consequence, it is not possible to take the address of a constant valarray[n].
    // This looks like a bug in the Microsoft's <valarray> file.
    // Below is a workaround
    double const *mp = &const_cast<valarray<double> &>(matrix)[0];
#   else
    const double* mp = &matrix[0];
#   endif
    for (unsigned i = 0; i < m; i++) {
        double res = 0;
        for (unsigned j = 0; j < n; j++)
            res += *mp++ * vec[j];
        result[i] = res;
    }
}

/*
static double Linfty(valarray<double> const &vec) {
    return std::max(vec.max(), -vec.min());
}
*/

double
inner(valarray<double> const &x, 
      valarray<double> const &y) {
    double total = 0;
    for(unsigned i = 0; i < x.size(); i++)
        total += x[i]*y[i];
    return total;// (x*y).sum(); <- this is more concise, but ineff
}

double compute_cost(valarray<double> const &A, 
        valarray<double> const &b,
        valarray<double> const &x,
        const unsigned n) {
    // computes cost = 2 b x - x A x
    double cost = 2. * inner(b,x);
    valarray<double> Ax(n);
    for (unsigned i=0; i<n; i++) {
        Ax[i] = 0;
        for (unsigned j=0; j<n; j++) {
            Ax[i] += A[i*n+j]*x[j];
        }
    }
    return cost - inner(x,Ax);
}
void 
conjugate_gradient(valarray<double> const &A, 
           valarray<double> &x, 
           valarray<double> const &b, 
           unsigned const n, double const tol,
           unsigned const max_iterations) {
    //printf("Conjugate Gradient...\n");
    valarray<double> Ap(n), p(n), r(n);
    matrix_times_vector(A,x,Ap);
    r=b-Ap; 
    double r_r = inner(r,r);
    unsigned k = 0;
    double tol_squared = tol*tol;
#ifdef EXAMINE_COST
    double previousCost=compute_cost(A,b,x,n),cost;
#endif
    while(k < max_iterations && r_r > tol_squared) {
        k++;
        double r_r_new = r_r;
        if(k == 1)
            p = r;
        else {
            r_r_new = inner(r,r);
            if(r_r_new<tol_squared) break;
            p = r + (r_r_new/r_r)*p;
        }
        matrix_times_vector(A, p, Ap);
        double alpha_k = r_r_new / inner(p, Ap);
        x += alpha_k*p;
#ifdef EXAMINE_COST
        cost=compute_cost(A,b,x,n);
        printf("  CG[%d] %.15f %.15f\n",k,previousCost,cost);
        previousCost=cost;
#endif
        r -= alpha_k*Ap;
        r_r = r_r_new;
    }
    //printf("  CG finished after %d iterations\n",k);
    //printf("njh: %d iters, Linfty = %g L2 = %g\n", k, 
    //std::max(-r.min(), r.max()), sqrt(r_r));
    // x is solution
}
