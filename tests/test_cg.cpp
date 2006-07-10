#include <stdio.h>
#include "conjugate_gradient.h"
#include <gsl/gsl_linalg.h>

using std::valarray;

valarray<double> outer_prod(valarray<double> x, valarray<double> y) {
    valarray<double> result(x.size()*y.size());
    for(int j = 0; j < x.size(); j++) {
        for(int i = 0; i < y.size(); i++) {
            result[j*y.size() + i] = x[j]*y[i];
        }
    }
    return result;
}

double uniform() {
    return double(rand())/ RAND_MAX;
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
		
        conjugate_gradient(A, xx, b, N, tolerance, 2*N, false);
	
        gsl_matrix_view m 
            = gsl_matrix_view_array (A_data, N, N);
        gsl_vector_view bgsl
            = gsl_vector_view_array (b_data, N);
     
        gsl_vector *xgsl = gsl_vector_alloc (N);
       
        int s;
     
        gsl_permutation * p = gsl_permutation_alloc (N);
     
        gsl_linalg_LU_decomp (&m.matrix, p, &s);
     
        gsl_linalg_LU_solve (&m.matrix, p, &bgsl.vector, xgsl);

        double err = 0;
        for(unsigned i = 0; i < xx.size(); i++) {
            double tr = xx[i]-gsl_vector_get(xgsl, i);
            err += tr*tr;
        }
        err = sqrt(err);
        printf ("sqrt((xx-nxgsl)^2) = %g\n", err);
        if(err > tolerance) {
            for(int r = 0; r < N; r++) {
                for(int c = 0; c < N; c++) {
                    printf("%g ", A_data[r*N + c]);
                }
                printf("\n");
            }
            printf("\nx njh-cg = \n");
            for(unsigned i = 0; i < xx.size(); i++)
                printf("%g ", xx[i]);
            printf("\nxgsl = ");
            for(unsigned i = 0; i < xx.size(); i++)
                printf("%g ", gsl_vector_get(xgsl, i));
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
