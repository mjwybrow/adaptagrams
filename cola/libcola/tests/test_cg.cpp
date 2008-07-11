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
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
*/

#include <stdio.h>
#include <cassert>
#include <libcola/conjugate_gradient.h>
#include <gsl/gsl_linalg.h>

using std::valarray;

static valarray<double> 
outer_prod(valarray<double> x, valarray<double> y) {
    valarray<double> result(x.size()*y.size());
    for(int j = 0; j < x.size(); j++) {
        for(int i = 0; i < y.size(); i++) {
            result[j*y.size() + i] = x[j]*y[i];
        }
    }
    return result;
}

static double 
uniform() {
    return double(rand())/ RAND_MAX;
}

static void 
matrix_times_vector(valarray<double> const &matrix, /* m * n */
		    valarray<double> const &vec,  /* n */
		    valarray<double> &result) /* m */
{
    unsigned n = vec.size();
    unsigned m = result.size();
    assert(m*n == matrix.size());
    const double* mp = &matrix[0];
    for (unsigned i = 0; i < m; i++) {
        double res = 0;
        for (unsigned j = 0; j < n; j++)
            res += *mp++ * vec[j];
        result[i] = res;
    }
}

static double 
Linfty(valarray<double> const &vec) {
    return std::max(vec.max(), -vec.min());
}

int
main (void)
{
    double tolerance = 1e-6;
    for(int iters = 0; iters < 100; iters++) {
        const unsigned  N = unsigned(uniform()*40) + 1;
        double A_data[N*N];
        printf("%ux%u matrix\n", N,N);
        for(int r = 0; r < N; r++) {
            for(int c = 0; c <= r; c++) {
                A_data[r*N + c] = A_data[c*N + r] = fabs(uniform());
            }
        }
	
        double * A_c[N];
        for(int i = 0; i < N; i++)
            A_c[i] = &A_data[N*i];
		
        double b_data[N];
        for(int i = 0; i < N; i++)
            b_data[i] = uniform()*3;
        std::valarray<double> b(b_data, N), xx(0.0, N);
        std::valarray<double> A(A_data, N*N);
		
        conjugate_gradient(A, xx, b, N, tolerance, 2*N);
	
        gsl_matrix_view m 
            = gsl_matrix_view_array (A_data, N, N);
        gsl_vector_view bgsl
            = gsl_vector_view_array (b_data, N);
     
        gsl_vector *xgsl = gsl_vector_alloc (N);
       
        int s;
     
        gsl_permutation * p = gsl_permutation_alloc (N);
     
        gsl_linalg_LU_decomp (&m.matrix, p, &s);
     
        gsl_linalg_LU_solve (&m.matrix, p, &bgsl.vector, xgsl);
        
        std::valarray<double> gsl_xx(0.0, N);
        for(unsigned i = 0; i < xx.size(); i++) {
            gsl_xx[i] = gsl_vector_get(xgsl, i);
        }

        double err = Linfty(xx - gsl_xx);
        printf ("|xx-nxgsl|_infty = %g\n", err);
        if(err > tolerance) {
#if 0 //dubious value
            for(int r = 0; r < N; r++) {
                for(int c = 0; c < N; c++) {
                    printf("%g ", A_data[r*N + c]);
                }
                printf("\n");
            }
#endif
            printf("\nx njh-cg = \n");
            for(unsigned i = 0; i < xx.size(); i++)
                printf("%g ", xx[i]);
            printf("\nxgsl = ");
            for(unsigned i = 0; i < xx.size(); i++)
                printf("%g ", gsl_xx[i]);
            printf("\n");
            valarray<double> result(0.0,N);
            matrix_times_vector(A, xx, result);
            result -= b;
            printf("\nA xx -b = ");
            for(unsigned i = 0; i < xx.size(); i++)
                printf("%g ", result[i]);
            printf("\n");
            matrix_times_vector(A, gsl_xx, result);
            result -= b;
            printf("\nA gsl_xx -b = ");
            for(unsigned i = 0; i < xx.size(); i++)
                printf("%g ", result[i]);
            printf("\n");
            
            printf("FAILED!!!!!!!!!!!!!!!!!!!!!!!!\n");
            exit(1);
        }
    }
    return 0;
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
