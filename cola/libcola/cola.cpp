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

template <typename EdgeOrSideLength, typename Done >
void 
constrained_majorization_layout_impl<EdgeOrSideLength, Done >
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
template <typename EdgeOrSideLength, typename Done >
void constrained_majorization_layout_impl<
 EdgeOrSideLength, Done >
::majlayout(
        double** Dij, GradientProjection* gp, double* coords) 
{
    double b[n];
    double L_ij,dist_ij,degree;
    /* compute the vector b */
    /* multiply on-the-fly with distance-based laplacian */
    for (unsigned i = 0; i < n; i++) {
        degree = 0;
        b[i] = 0;
        if(i<lapSize) {
            for (unsigned j = 0; j < lapSize; j++) {
                if (j == i) continue;
                dist_ij = euclidean_distance(i, j);
                if (dist_ij > 1e-30 && Dij[i][j] > 1e-30) {	/* skip zero distances */
                    /* calculate L_ij := w_{ij}*d_{ij}/dist_{ij} */
                    L_ij = 1.0 / (dist_ij * Dij[i][j]);
                    degree -= L_ij;
                    b[i] += L_ij * coords[j];
                }
            }
            b[i] += degree * coords[i];
        }
        assert(!isnan(b[i]));
    }
    if(constrainedLayout) {
        setupDummyVars();
        gp->solve(b);
        moveBoundingBoxes();
    } else {
        conjugate_gradient(lap2, coords, b, n, tol, n, true);
    }
}
template <typename EdgeOrSideLength, typename Done >
inline double constrained_majorization_layout_impl<EdgeOrSideLength, Done >
::compute_stress(double **Dij) {
    double sum = 0, d, diff;
    for (unsigned i = 1; i < lapSize; i++) {
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
template <typename EdgeOrSideLength, typename Done >
void constrained_majorization_layout_impl<EdgeOrSideLength, Done >
::addLinearConstraints(LinearConstraints* linearConstraints, vector<Rectangle*>& rs) {
    n=lapSize+linearConstraints->size();
    Q=new double*[n];
    X=new double[n];
    Y=new double[n];
    boundingBoxes = new Rectangle*[n];
    copy(rs.begin(),rs.end(),boundingBoxes);
    for(unsigned i = 0; i<n; i++) {
        X[i]=rs[i]->getCentreX();
        Y[i]=rs[i]->getCentreY();
        Q[i]=new double[n];
        for(unsigned j=0; j<n; j++) {
            if(i<lapSize&&j<lapSize) {
                Q[i][j]=lap2[i][j];
            } else {
                Q[i][j]=0;
            }
        }
    }
    for(LinearConstraints::iterator i=linearConstraints->begin();
           i!= linearConstraints->end();i++) {
        LinearConstraint* c=*i;
        Q[c->u][c->u]+=c->w*c->duu;
        Q[c->u][c->v]+=c->w*c->duv;
        Q[c->u][c->b]+=c->w*c->dub;
        Q[c->v][c->u]+=c->w*c->duv;
        Q[c->v][c->v]+=c->w*c->dvv;
        Q[c->v][c->b]+=c->w*c->dvb;
        Q[c->b][c->b]+=c->w*c->dbb;
        Q[c->b][c->u]+=c->w*c->dub;
        Q[c->b][c->v]+=c->w*c->dvb;
    }
}

template <typename EdgeOrSideLength, typename Done >
bool constrained_majorization_layout_impl<EdgeOrSideLength, Done >
::run() {
    /*
    for(unsigned i=0;i<n;i++) {
        for(unsigned j=0;j<n;j++) {
            cout << lap2[i][j] << " ";
        }
        cout << endl;
    }
    */
    do {
        /* Axis-by-axis optimization: */
        if(layoutXAxis) majlayout(Dij,gpX,X);
        if(layoutYAxis) majlayout(Dij,gpY,Y);
    } while(!done(compute_stress(Dij),g,X,Y));
    for(unsigned i = 0; i<n; i++) {
        moveBoundingBoxes();
    }
    return true;
}
template <typename EdgeOrSideLength, typename Done >
void constrained_majorization_layout_impl<EdgeOrSideLength, Done >
::setupConstraints(
        AlignmentConstraints* acsx, AlignmentConstraints* acsy,
        bool avoidOverlaps, 
        PageBoundaryConstraints* pbcx, PageBoundaryConstraints* pbcy,
        SimpleConstraints* scx, SimpleConstraints* scy,
        Clusters* cs) {
    constrainedLayout = true;
    this->avoidOverlaps = avoidOverlaps;
    if(cs) {
        clusters=cs;
    }
	gpX=new GradientProjection(
            HORIZONTAL,n,Q,X,tol,100,acsx,avoidOverlaps,boundingBoxes,pbcx,scx);
	gpY=new GradientProjection(
            VERTICAL,n,Q,Y,tol,100,acsy,avoidOverlaps,boundingBoxes,pbcy,scy);
}
} // namespace cola
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
