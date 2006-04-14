#include <math.h>
#include <stdlib.h>

#include "conjugate_gradient.h"
/**********************************
 * This code is adapted from
 * graphviz matrix_ops code,
 * which is licensed under 
 * the Common Public License (CPL)
 * http://www.graphviz.org/License.php
 * If you use it please abide by the
 * conditions of the CPL.
 *
 * C.G. method - DENSE      
 **********************************/

static void copy_vector(int n, double *source, double *dest)
{
    int i;
    for (i = 0; i < n; i++)
	dest[i] = source[i];
}
static void orthog1(int n, double *vec	/* vector to be orthogonalized against 1 */
    )
{
    int i;
    double *pntr;
    double sum;

    sum = 0.0;
    pntr = vec;
    for (i = n; i; i--) {
	sum += *pntr++;
    }
    sum /= n;
    pntr = vec;
    for (i = n; i; i--) {
	*pntr++ -= sum;
    }
}
static void right_mult_with_vector_d(double **matrix,
			 int dim1, int dim2,
			 double *vector, double *result)
{
    /* matrix is dim1 x dim2, vector has dim2 components, result=matrix x vector */
    int i, j;

    double res;
    for (i = 0; i < dim1; i++) {
	res = 0;
	for (j = 0; j < dim2; j++)
	    res += matrix[i][j] * vector[j];
	result[i] = res;
    }
}

static void
vectors_subtraction(int n, double *vector1, double *vector2,
		    double *result)
{
    int i;
    for (i = 0; i < n; i++) {
	result[i] = vector1[i] - vector2[i];
    }
}
static double 
vectors_inner_product(int n, double *vector1, double *vector2)
{
    int i;
    double result = 0;
    for (i = 0; i < n; i++) {
	result += vector1[i] * vector2[i];
    }

    return result;
}
static double max_abs(int n, double *vector)
{
    double max_val = -1e50;
    int i;
    for (i = 0; i < n; i++)
	if (fabs(vector[i]) > max_val)
	    max_val = fabs(vector[i]);

    return max_val;
}
static void
vectors_scalar_mult(int n, double *vector, double alpha, double *result)
{
    int i;
    for (i = 0; i < n; i++) {
	result[i] = vector[i] * alpha;
    }
}
static void
vectors_addition(int n, double *vector1, double *vector2, double *result)
{
    int i;
    for (i = 0; i < n; i++) {
	result[i] = vector1[i] + vector2[i];
    }
}
void conjugate_gradient
    (double **A, double *x, double *b, int n, double tol,
     int max_iterations, bool ortho1) {
    /* Solves Ax=b using Conjugate-Gradients method */
    /* 'x' and 'b' are orthogonalized against 1 if 'ortho1=true' */

    int i;

    double alpha, beta, r_r, r_r_new, p_Ap;
    double r[n];
    double p[n];
    double Ap[n];
    double Ax[n];
    double alphap[n];
    double orth_b[n];

    copy_vector(n, b, orth_b);
    if (ortho1) {
        orthog1(n, orth_b);
        orthog1(n, x);
    }
    right_mult_with_vector_d(A, n, n, x, Ax);
    vectors_subtraction(n, orth_b, Ax, r);
    copy_vector(n, r, p);
    r_r = vectors_inner_product(n, r, r);

    for (i = 0; i < max_iterations && max_abs(n, r) > tol; i++) {
        right_mult_with_vector_d(A, n, n, p, Ap);
        p_Ap = vectors_inner_product(n, p, Ap);
        if (p_Ap == 0) break;
        alpha = r_r / p_Ap;

        /* derive new x: */
        vectors_scalar_mult(n, p, alpha, alphap);
        vectors_addition(n, x, alphap, x);

        /* compute values for next iteration: */
        if (i < max_iterations - 1) {	/* not last iteration */
            vectors_scalar_mult(n, Ap, alpha, Ap);
            vectors_subtraction(n, r, Ap, r);	/* fast computation of r, the residual */

            /* Alternaive accurate, but slow, computation of the residual - r */
            /* right_mult_with_vector(A, n, x, Ax); */
            /* vectors_subtraction(n,b,Ax,r); */

            r_r_new = vectors_inner_product(n, r, r);
            if (r_r == 0) exit(1);
            beta = r_r_new / r_r;
            r_r = r_r_new;
            vectors_scalar_mult(n, p, beta, p);
            vectors_addition(n, r, p, p);
        }
    }
}
