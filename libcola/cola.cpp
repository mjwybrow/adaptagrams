#include "cola.h"
#include "conjugate_gradient.h"
#include "straightener.h"

namespace cola {

/**
 * Find the euclidean distance between a pair of dummy variables
 */
inline double dummy_var_euclidean_dist(GradientProjection* gpx, GradientProjection* gpy, unsigned i) {
    double dx = gpx->dummy_vars[i]->place_r - gpx->dummy_vars[i]->place_l,
        dy = gpy->dummy_vars[i]->place_r - gpy->dummy_vars[i]->place_l;
    return sqrt(dx*dx + dy*dy);
}

void 
ConstrainedMajorizationLayout
::setupDummyVars() {
    if(clusters==NULL) return;
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
void ConstrainedMajorizationLayout::majlayout(
        double** Dij, GradientProjection* gp, double* coords) 
{
    double b[n];
    fill(b,b+n,0);
    majlayout(Dij,gp,coords,b);
}
void ConstrainedMajorizationLayout::majlayout(
        double** Dij, GradientProjection* gp, double* coords, double* b) 
{
    double L_ij,dist_ij,degree;
    /* compute the vector b */
    /* multiply on-the-fly with distance-based laplacian */
    for (unsigned i = 0; i < n; i++) {
        degree = 0;
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
    } else {
        conjugate_gradient(lap2, coords, b, n, tol, n);
    }
    moveBoundingBoxes();
}
inline double ConstrainedMajorizationLayout
::compute_stress(double **Dij) {
    double sum = 0, d, diff;
    for (unsigned i = 1; i < lapSize; i++) {
        for (unsigned j = 0; j < i; j++) {
            d = Dij[i][j];
            diff = d - euclidean_distance(i,j);
            sum += diff*diff / (d*d);
        }
    }
    if(clusters!=NULL) {
        for(unsigned i=0; i<gpX->dummy_vars.size(); i++) {
            sum += gpX->dummy_vars[i]->stress(dummy_var_euclidean_dist(gpX, gpY, i));
        }
    }
    return sum;
}
/*
void ConstrainedMajorizationLayout
::addLinearConstraints(LinearConstraints* linearConstraints) {
    n=lapSize+linearConstraints->size();
    Q=new double*[n];
    X=new double[n];
    Y=new double[n];
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
*/

bool ConstrainedMajorizationLayout::run() {
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
        if(straightenEdges) {
            straighten(*straightenEdges,HORIZONTAL);
            straighten(*straightenEdges,VERTICAL);
        } else {
            majlayout(Dij,gpX,X);
            majlayout(Dij,gpY,Y);
        }
    } while(!done(compute_stress(Dij),X,Y));
    return true;
}
static bool straightenToProjection=true;
void ConstrainedMajorizationLayout::straighten(vector<straightener::Edge*>& sedges, Dim dim) {
	vector<straightener::Node*> snodes;
	for (unsigned i=0;i<lapSize;i++) {
		snodes.push_back(new straightener::Node(i,boundingBoxes[i]));
	}
    SimpleConstraints cs;
    straightener::generateConstraints(snodes,sedges,cs,dim);
    n=snodes.size();
    Q=new double*[n];
    delete [] X;
    delete [] Y;
    X=new double[n];
    Y=new double[n];
    for(unsigned i = 0; i<n; i++) {
        X[i]=snodes[i]->x;
        Y[i]=snodes[i]->y;
        Q[i]=new double[n];
        for(unsigned j=0; j<n; j++) {
            if(i<lapSize&&j<lapSize) {
                Q[i][j]=lap2[i][j];
            } else {
                Q[i][j]=0;
            }
        }
    }
    LinearConstraints linearConstraints;
    for(unsigned i=0;i<sedges.size();i++) {
        sedges[i]->nodePath(snodes);
        vector<unsigned>& path=sedges[i]->path;
        // take u and v as the ends of the line
        //unsigned u=path[0];
        //unsigned v=path[path.size()-1];
        double total_length=0;
        for(unsigned j=1;j<path.size();j++) {
            unsigned u=path[j-1], v=path[j];
            total_length+=euclidean_distance(u,v);
        }
        for(unsigned j=1;j<path.size()-1;j++) {
            // find new u and v for each line segment
            unsigned u=path[j-1];
            unsigned b=path[j];
            unsigned v=path[j+1];
            double weight=-0.01;
            double wub=euclidean_distance(u,b)/total_length;
            double wbv=euclidean_distance(b,v)/total_length;
            linearConstraints.push_back(new cola::LinearConstraint(u,v,b,weight,wub,wbv,X,Y));
        }
    }
    //cout << "Generated "<<linearConstraints.size()<< " linear constraints"<<endl;
    assert(snodes.size()==lapSize+linearConstraints.size());
    double b[n],*coords=dim==HORIZONTAL?X:Y,dist_ub,dist_bv;
    fill(b,b+n,0);
    for(LinearConstraints::iterator i=linearConstraints.begin();
           i!= linearConstraints.end();i++) {
        LinearConstraint* c=*i;
        if(straightenToProjection) {
            Q[c->u][c->u]+=c->w*c->duu;
            Q[c->u][c->v]+=c->w*c->duv;
            Q[c->u][c->b]+=c->w*c->dub;
            Q[c->v][c->u]+=c->w*c->duv;
            Q[c->v][c->v]+=c->w*c->dvv;
            Q[c->v][c->b]+=c->w*c->dvb;
            Q[c->b][c->b]+=c->w*c->dbb;
            Q[c->b][c->u]+=c->w*c->dub;
            Q[c->b][c->v]+=c->w*c->dvb;
        } else {
            double wub=edge_length*c->frac_ub;
            double wbv=edge_length*c->frac_bv;
            dist_ub=euclidean_distance(c->u,c->b)*wub;
            dist_bv=euclidean_distance(c->b,c->v)*wbv;
            wub=max(wub,0.00001);
            wbv=max(wbv,0.00001);
            dist_ub=max(dist_ub,0.00001);
            dist_bv=max(dist_bv,0.00001);
            wub=1/(wub*wub);
            wbv=1/(wbv*wbv);
            Q[c->u][c->u]-=wub;
            Q[c->u][c->b]+=wub;
            Q[c->v][c->v]-=wbv;
            Q[c->v][c->b]+=wbv;
            Q[c->b][c->b]-=wbv+wub;
            Q[c->b][c->u]+=wub;
            Q[c->b][c->v]+=wbv;

            b[c->u]+=(coords[c->b]-coords[c->u]) / dist_ub;
            b[c->v]+=(coords[c->b]-coords[c->v]) / dist_bv;
            b[c->b]+=coords[c->u] / dist_ub + coords[c->v] / dist_bv
                   - coords[c->b] / dist_ub - coords[c->b] / dist_bv;
        }
    }
	GradientProjection gp(dim,n,Q,coords,tol,100,
            (AlignmentConstraints*)NULL,false,(Rectangle**)NULL,(PageBoundaryConstraints*)NULL,&cs);
    constrainedLayout = true;
    majlayout(Dij,&gp,coords,b);
    for(unsigned i=0;i<sedges.size();i++) {
        sedges[i]->createRouteFromPath(X,Y);
        sedges[i]->dummyNodes.clear();
        sedges[i]->path.clear();
    }
    for(unsigned i=0;i<cs.size();i++) {
        delete cs[i];
    }
    for(unsigned i=0;i<linearConstraints.size();i++) {
        delete linearConstraints[i];
    }
    for(unsigned i=0;i<snodes.size();i++) {
        delete snodes[i];
    }
    for(unsigned i = 0; i<n; i++) {
        delete [] Q[i];
    }
    delete [] Q;
    snodes.resize(lapSize);
}

void ConstrainedMajorizationLayout::setupConstraints(
        AlignmentConstraints* acsx, AlignmentConstraints* acsy,
        bool avoidOverlaps, 
        PageBoundaryConstraints* pbcx, PageBoundaryConstraints* pbcy,
        SimpleConstraints* scx, SimpleConstraints* scy,
        Clusters* cs,
        vector<straightener::Edge*>* straightenEdges) {
    constrainedLayout = true;
    this->avoidOverlaps = avoidOverlaps;
    if(cs) {
        clusters=cs;
    }
	gpX=new GradientProjection(
            HORIZONTAL,n,Q,X,tol,100,acsx,avoidOverlaps,boundingBoxes,pbcx,scx);
	gpY=new GradientProjection(
            VERTICAL,n,Q,Y,tol,100,acsy,avoidOverlaps,boundingBoxes,pbcy,scy);
    this->straightenEdges = straightenEdges;
}
} // namespace cola
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
