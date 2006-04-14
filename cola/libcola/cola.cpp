#include "cola.h"
#include "conjugate_gradient.h"

namespace cola {

template <typename PositionMap, typename EdgeOrSideLength, typename Done >
void constrained_majorization_layout_impl<PositionMap, EdgeOrSideLength, Done >
::majlayout(double** lap2, double** Dij) {
    bool Verbose = false;
    double b[n];
    double L_ij,dist_ij,degree,tol=0.0001;
    GradientProjection gp[]={
	GradientProjection(n,lap2,coords[0],tol,100),
	GradientProjection(n,lap2,coords[1],tol,100)
    };
    while(!done(compute_stress(Dij),g)) {
        /* Axis-by-axis optimization: */
        for (unsigned k = 0; k < 2; k++) {
            /* compute the vector b */
            /* multiply on-the-fly with distance-based laplacian */
            /* (for saving storage we don't construct this Laplacian explicitly) */
            for (unsigned i = 0; i < n; i++) {
            degree = 0;
            b[i] = 0;
            for (unsigned j = 0; j < n; j++) {
                if (j == i) continue;
                dist_ij = euclidean_distance(coords, i, j);
                if (dist_ij > 1e-30) {	/* skip zero distances */
                /* calculate L_ij := w_{ij}*d_{ij}/dist_{ij} */
                L_ij = 1.0 / (dist_ij * Dij[i][j]);
                degree -= L_ij;
                b[i] += L_ij * coords[k][j];
                }
            }
            b[i] += degree * coords[k][i];
            }
            if(avoidOverlaps) {
                if(boundingBoxes) gp[k].generateNonOverlapConstraints(k,boundingBoxes);
                gp[k].solve(b);
                if(boundingBoxes) moveBoundingBoxes();
            } else {
                conjugate_gradient(lap2, coords[k], b, n, tol, n, true);
            }
        }
    }
    if (Verbose) fprintf(stderr, "\nfinal e = %f\n", compute_stress(Dij));
}	  
template <typename PositionMap, typename EdgeOrSideLength, typename Done >
inline double constrained_majorization_layout_impl<PositionMap, EdgeOrSideLength, Done >
::compute_stress(double **Dij) {
    double sum = 0, d, diff;
    for (unsigned i = 1; i < n; i++) {
        for (unsigned j = 0; j < i; j++) {
            d = Dij[i][j];
            diff = d - euclidean_distance(coords,i,j);
            sum += diff*diff / (d*d);
        }
    }
    return sum;
}
template <typename PositionMap, typename EdgeOrSideLength, typename Done >
bool constrained_majorization_layout_impl<PositionMap, EdgeOrSideLength, Done >
::run(double** coords) {
    this->coords=coords;
    typedef typename property_traits<WeightMap>::value_type weight_type;
    vec_adj_list_vertex_id_map<no_property, unsigned int> index = get(vertex_index,g);
    typedef std::vector<weight_type> weight_vec;
    std::vector<weight_vec> distance(n,weight_vec(n));

    if (!johnson_all_pairs_shortest_paths(g, distance, index, weight, weight_type(0)))
        return false;
    weight_type edge_length = detail::graph::compute_edge_length(g, distance, index,
                                     edge_or_side_length);
    // Lij_{i!=j}=1/(Dij^2)
    //
    double** lap2 = new double*[n];
    double** Dij = new double*[n];
    for(unsigned i = 0; i<n; i++) {
        coords[0][i]=position[i].x+boundingBoxes[i]->width()/2.0;
        coords[1][i]=position[i].y+boundingBoxes[i]->height()/2.0;
    }
    for(unsigned i = 0; i<n; i++) {
        weight_type degree = weight_type(0);
        lap2[i]=new double[n];
        Dij[i]=new double[n];
        for(unsigned j=0;j<n;j++) {
            weight_type w = edge_length * distance[i][j];
            Dij[i][j]=w;
            if(i==j) continue;
            degree+=lap2[i][j]=weight_type(1)/(w*w);
        }
        lap2[i][i]=-degree;
    }
    majlayout(lap2, Dij);	
    for(unsigned i = 0; i<n; i++) {
        position[i].x=coords[0][i];
        position[i].y=coords[1][i];
        delete [] lap2[i];
        delete [] Dij[i];
    }
    delete [] lap2;
    delete [] Dij;
    delete [] coords[0];
    delete [] coords[1];
    delete [] coords;
    return true;
}
template <typename PositionMap, typename EdgeOrSideLength, typename Done >
bool constrained_majorization_layout_impl<PositionMap, EdgeOrSideLength, Done >
::run() {
    double** coords;
    coords = new double*[2];
    coords[0] = new double[n];
    coords[1] = new double[n];
    return run(coords);
}
} // namespace cola
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
