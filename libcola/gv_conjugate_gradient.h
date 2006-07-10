#ifndef _CONJUGATE_GRADIENT_H
#define _CONJUGATE_GRADIENT_H
#ifdef __cplusplus
extern "C" {
#else
#define bool int
#endif
extern void conjugate_gradient
    (double **A, double *x, double *b, int n, double tol,
     int max_iterations, bool ortho1);
#ifdef __cplusplus
}
#endif
#endif // _CONJUGATE_GRADIENT_H
