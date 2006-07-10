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
	for(int iters = 0; iters < 10; iters++) {
		double A_data[4*4] = {1, 0.2, 0, 0.5,
				      0.2, 1, 0, 0,
				      0, 0, 1, 0,
				      0.5, 0, 0, 1};
	
		for(int r = 0; r < 4; r++) {
			for(int c = 0; c < r; c++) {
				A_data[r*4 + c] = A_data[c*4 + r] = uniform();
			}
		}
	
		double * A_c[4] = {&A_data[0], &A_data[4], &A_data[8], &A_data[12]};
	
		double b_data[] = { 1.0, 2.0, 3.0, 4.0 };
		std::valarray<double> b(b_data, 4), xx(0.0, 4);
		std::valarray<double> A(A_data, 16);

		
		conjugate_gradient(A, xx, b, 4, tolerance, 10, false);
	
		gsl_matrix_view m 
			= gsl_matrix_view_array (A_data, 4, 4);
		gsl_vector_view bgsl
			= gsl_vector_view_array (b_data, 4);
     
		gsl_vector *xgsl = gsl_vector_alloc (4);
       
		int s;
     
		gsl_permutation * p = gsl_permutation_alloc (4);
     
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
			printf("FAILED!!!!!!!!!!!!!!!!!!!!!!!!\n");
			exit(1);
		}
	}
	return 0;
}
