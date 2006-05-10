#include "conjugate_gradient.h"
#include "generate-constraints.h"

namespace cola {

/**
 * Find the euclidean distance between a pair of dummy variables
 */
inline double dummy_var_euclidean_dist(GradientProjection* gpx, GradientProjection* gpy, unsigned i) {
    double dx = gpx->dummy_vars[i]->place_r - gpx->dummy_vars[i]->place_l,
        dy = gpy->dummy_vars[i]->place_r - gpy->dummy_vars[i]->place_l;
    return sqrt(dx*dx + dy*dy);
}

template <typename PositionMap, typename EdgeOrSideLength, typename Done >
void 
constrained_majorization_layout_impl<PositionMap, EdgeOrSideLength, Done >
::setupDummyVars() {
    double* coords[2]={X,Y};
    GradientProjection* gp[2]={gpX,gpY};
    for(unsigned k=0;k<2;k++) {
        gp[k]->clearDummyVars();
        if(clusters) {
            for(Clusters::iterator cit=clusters->begin();
                    cit!=clusters->end(); ++cit) {
                Cluster *c = *cit;
                DummyVarPair* p = new DummyVarPair(edge_length);
                gp[k]->dummy_vars.push_back(p);
                double minPos=DBL_MAX, maxPos=-DBL_MAX;
                for(Cluster::iterator vit=c->begin();
                        vit!=c->end(); ++vit) {
                    double pos = coords[k][*vit];
                    minPos=min(pos,minPos);
                    maxPos=max(pos,maxPos);
                    p->leftof.push_back(make_pair(*vit,0)); 
                    p->rightof.push_back(make_pair(*vit,0)); 
                }
                p->place_l = minPos;
                p->place_r = maxPos;
            }
        }
    }
    for(unsigned k=0;k<2;k++) {
        unsigned n_d = gp[k]->dummy_vars.size();
        if(n_d > 0) {
            for(unsigned i=0; i<n_d; i++) {
                gp[k]->dummy_vars[i]->computeLinearTerm(dummy_var_euclidean_dist(gpX, gpY, i));
            }
        }
    }
}
template <typename PositionMap, typename EdgeOrSideLength, typename Done >
void constrained_majorization_layout_impl<PositionMap, EdgeOrSideLength, Done >
::majlayout(double** Dij) {
    double b[n];
    double L_ij,dist_ij,degree;
    double* coords[2]={X,Y};
    GradientProjection* gp[2]={gpX,gpY};
    do {
        /* Axis-by-axis optimization: */
        for (unsigned k = 0; k < 2; k++) {
            /* compute the vector b */
            /* multiply on-the-fly with distance-based laplacian */
            for (unsigned i = 0; i < n; i++) {
                degree = 0;
                b[i] = 0;
                for (unsigned j = 0; j < n; j++) {
                    if (j == i) continue;
                    dist_ij = euclidean_distance(i, j);
                    if (dist_ij > 1e-30) {	/* skip zero distances */
                        /* calculate L_ij := w_{ij}*d_{ij}/dist_{ij} */
                        L_ij = 1.0 / (dist_ij * Dij[i][j]);
                        degree -= L_ij;
                        b[i] += L_ij * coords[k][j];
                    }
                }
                b[i] += degree * coords[k][i];
            }
            if(constrainedLayout) {
                setupDummyVars();
                gp[k]->solve(b);
                if(boundingBoxes) moveBoundingBoxes();
            } else {
                conjugate_gradient(lap2, coords[k], b, n, tol, n, true);
            }
        }
    } while(!done(compute_stress(Dij),g,X,Y));
}	  
template <typename PositionMap, typename EdgeOrSideLength, typename Done >
inline double constrained_majorization_layout_impl<PositionMap, EdgeOrSideLength, Done >
::compute_stress(double **Dij) {
    double sum = 0, d, diff;
    for (unsigned i = 1; i < n; i++) {
        for (unsigned j = 0; j < i; j++) {
            d = Dij[i][j];
            diff = d - euclidean_distance(i,j);
            sum += diff*diff / (d*d);
        }
    }
    if(constrainedLayout) {
        for(unsigned i=0; i<gpX->dummy_vars.size(); i++) {
            sum += gpX->dummy_vars[i]->stress(dummy_var_euclidean_dist(gpX, gpY, i));
        }
    }
    return sum;
}
template <typename PositionMap, typename EdgeOrSideLength, typename Done >
bool constrained_majorization_layout_impl<PositionMap, EdgeOrSideLength, Done >
::run() {
    typedef typename property_traits<WeightMap>::value_type weight_type;
    vec_adj_list_vertex_id_map<no_property, unsigned int> index = get(vertex_index,g);
    typedef std::vector<weight_type> weight_vec;
    std::vector<weight_vec> distance(n,weight_vec(n));

    if (!johnson_all_pairs_shortest_paths(g, distance, index, weight, weight_type(0)))
        return false;
    edge_length = detail::graph::compute_edge_length(g, distance, index,
                                     edge_or_side_length);
    // Lij_{i!=j}=1/(Dij^2)
    //
    double** Dij = new double*[n];
    for(unsigned i = 0; i<n; i++) {
        X[i]=position[i].x;
        Y[i]=position[i].y;
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
    majlayout(Dij);	
    for(unsigned i = 0; i<n; i++) {
        position[i].x=X[i];
        position[i].y=Y[i];
        delete [] lap2[i];
        delete [] Dij[i];
    }
    delete [] Dij;
    return true;
}
template <typename PositionMap, typename EdgeOrSideLength, typename Done >
void constrained_majorization_layout_impl<PositionMap, EdgeOrSideLength, Done >
::setupConstraints(
        AlignmentConstraints* acsx, AlignmentConstraints* acsy,
        bool avoidOverlaps, PositionMap* dim,
        PageBoundaryConstraints *pbcx, PageBoundaryConstraints *pbcy,
        Clusters* cs) {
    constrainedLayout = true;
    this->avoidOverlaps = avoidOverlaps;
    if(dim) {
        boundingBoxes = new Rectangle*[n]; 
        for(unsigned i = 0; i < n; i++) {
            double x=position[i].x, y=position[i].y, 
                w=(*dim)[i].x/2.0, h=(*dim)[i].y/2.0;
            boundingBoxes[i] = new Rectangle(x-w,x+w,y-h,y+h);
        }
    }
    if(cs) {
        clusters=cs;
    }
	gpX=new GradientProjection(
            HORIZONTAL,n,lap2,X,tol,100,acsx,avoidOverlaps,boundingBoxes,pbcx);
	gpY=new GradientProjection(
            VERTICAL,n,lap2,Y,tol,100,acsy,avoidOverlaps,boundingBoxes,pbcy);
}
} // namespace cola
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
