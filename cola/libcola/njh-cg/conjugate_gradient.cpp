#include <math.h>
#include <stdlib.h>

#include <valarray>
#include "conjugate_gradient.h"

/* lifted wholely from wikipedia. */

using std::valarray;

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

static double Linfty(valarray<double> const &vec) {
	return std::max(vec.max(), -vec.min());
}

double
inner(valarray<double> const &x, 
      valarray<double> const &y) {
	double total = 0;
	for(unsigned i = 0; i < x.size(); i++)
		total += x[i]*y[i];
	return total;// (x*y).sum(); <- this is more concise, but ineff
}

void 
conjugate_gradient(valarray<double> const &A, 
		   valarray<double> &x, 
		   valarray<double> const &b, 
		   unsigned n, double tol,
		   unsigned max_iterations, bool ortho1) {
	valarray<double> Ap(n), p(n), r = b;
	double r_r = inner(r,r);
	unsigned k = 0;
	tol *= tol;
	while(k < max_iterations && r_r > tol) {
		k++;
		double r_r_new = r_r;
		if(k == 1)
			p = r;
		else {
			r_r_new = inner(r,r);
			p = r + (r_r_new/r_r)*p;
		}
		matrix_times_vector(A, p, Ap);
		double alpha_k = r_r_new / inner(p, Ap);
		x += alpha_k*p;
		r -= alpha_k*Ap;
		r_r = r_r_new;
	}
	printf("njh: %d iters, Linfty = %g L2 = %g\n", k, 
	       std::max(-r.min(), r.max()), sqrt(r_r));
	// x is solution
}
