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

int
main (void)
{
	double A_data[4*4] = {1, 0.2, 0, 0.5,
			      0.2, 1, 0, 0,
			      0, 0, 1, 0,
			      0.5, 0, 0, 1};
	
	double * A_c[4] = {&A_data[0], &A_data[4], &A_data[8], &A_data[12]};
	
	double b_data[] = { 1.0, 2.0, 3.0, 4.0 };
	std::valarray<double> b(b_data, 4), xx(0.0, 4);
	std::valarray<double> A(A_data, 16);

	
	conjugate_gradient(A, xx, b, 4, 1e-6, 10, false);
	
	gsl_matrix_view m 
		= gsl_matrix_view_array (A_data, 4, 4);
	gsl_vector_view bgsl
		= gsl_vector_view_array (b_data, 4);
     
	gsl_vector *xgsl = gsl_vector_alloc (4);
       
	int s;
     
	gsl_permutation * p = gsl_permutation_alloc (4);
     
	gsl_linalg_LU_decomp (&m.matrix, p, &s);
     
	gsl_linalg_LU_solve (&m.matrix, p, &bgsl.vector, xgsl);

	printf ("\nxx = ");
	for(unsigned i = 0; i < xx.size(); i++)
		printf("%g ", xx[i]);
	printf ("\nxgsl = ");
	gsl_vector_fprintf (stdout, xgsl, "%g");
	printf("\n");
	return 0;
}
