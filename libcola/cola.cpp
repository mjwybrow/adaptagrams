#include "cola.h"
#include <iostream>
#include "conjugate_gradient.h"

static inline double euclidean_distance(double **coords, int i, int j)
{
    double sum = 
	    (coords[0][i] - coords[0][j]) * (coords[0][i] - coords[0][j]) +
	    (coords[1][i] - coords[1][j]) * (coords[1][i] - coords[1][j]);
    return sqrt(sum);
}

static inline double compute_stress(unsigned n, double **coords, double **Dij)
{
    /* compute the overall stress */
    double sum = 0, d, diff;
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < i; j++) {
            d = Dij[i][j];
            diff = d - euclidean_distance(coords,i,j);
            sum += diff*diff / (d*d);
        }
    }
    return sum;
}

void majlayout(unsigned n, double** lap2, double** Dij, double** coords) {
	/*************************************************
	 Layout optimization
	*************************************************/
	
    bool Verbose = true, converged;

    double b[n];
    double L_ij,dist_ij,degree,old_stress,new_stress, conj_tol,Epsilon;
    conj_tol=Epsilon=0.0001;
    unsigned n_iterations = 100, iterations;
    old_stress = compute_stress(n, coords, Dij);
    for (converged = false, iterations = 0;
	 iterations < n_iterations && !converged; iterations++) {

        /* Axis-by-axis optimization: */
        for (int k = 0; k < 2; k++) {
            /* compute the vector b */
            /* multiply on-the-fly with distance-based laplacian */
            /* (for saving storage we don't construct this Laplacian explicitly) */
            for (int i = 0; i < n; i++) {
                degree = 0;
                b[i] = 0;
                for (int j = 0; j < n; j++) {
                    if (j == i) continue;
                    dist_ij = euclidean_distance(coords, i, j);
                    if (dist_ij > 1e-30) {	/* skip zero distances */
                        /* calculate L_ij := w_{ij}*d_{ij}/dist_{ij} */
                        L_ij = -1.0 / (dist_ij * Dij[i][j]);
                        degree -= L_ij;
                        b[i] += L_ij * coords[k][j];
                    }
                }
                b[i] += degree * coords[k][i];
            }
            conjugate_gradient(lap2, coords[k], b, n, conj_tol, n, true);
        }

        if ((converged = (iterations % 2 == 0))) {	/* check for convergence every two iterations */
            new_stress = compute_stress(n, coords, Dij);
            converged =
            fabs(new_stress - old_stress) / (new_stress + 1e-10) < Epsilon;
            old_stress = new_stress;
            if (Verbose && (iterations % 10 == 0)) {
            fprintf(stderr, "%.3f ", new_stress);
            if (iterations % 100 == 0)
                fprintf(stderr, "\n");
            }
        }
    }
    if (Verbose) fprintf(stderr, "\nfinal e = %f\n", compute_stress(n, coords, Dij));

}
