#include "cola.h"
#include "conjugate_gradient.h"
#include "straightener.h"
#include "shortest_paths.h"
using std::fill;
using std::min;
using std::max;
using std::make_pair;

namespace cola {

ConstrainedMajorizationLayout
::ConstrainedMajorizationLayout(
        vector<Rectangle*>& rs,
        vector<Edge> const & es,
        Clusters *cs,
        double const idealLength,
        std::valarray<double> const * startX,
        std::valarray<double> const * startY,
        std::valarray<double> const * eweights,
        TestConvergence& done,
        PreIteration* preIteration)
    : n(rs.size()),
      lap2(valarray<double>(n*n)), 
      Dij(valarray<double>(n*n)),
      tol(0.0001), done(done), preIteration(preIteration),
      stickyNodes(startX!=NULL), stickyWeight(0.00001),
      startX(startX), startY(startY),
      X(valarray<double>(n)), Y(valarray<double>(n)),
      constrainedLayout(stickyNodes),
      nonOverlappingClusters(false),
      clusters(cs), linearConstraints(NULL),
      gpX(NULL), gpY(NULL),
      ccsx(NULL), ccsy(NULL),
      avoidOverlaps(None),
      straightenEdges(NULL),
      bendWeight(0.01), potBendWeight(0.1)
{
    boundingBoxes.resize(rs.size());
    copy(rs.begin(),rs.end(),boundingBoxes.begin());

    done.reset();

    assert(!eweights||eweights->size()==es.size()); 
    assert(!stickyNodes && !startX && !startY
            ||stickyNodes && startX && startY
              && startX->size()==n && startY->size()==n);

    double** D=new double*[n];
    for(unsigned i=0;i<n;i++) {
        D[i]=new double[n];
    }
    shortest_paths::johnsons(n,D,es,eweights);
    edge_length = idealLength;
    if(clusters) {
        for(Clusters::const_iterator i=clusters->begin();i!=clusters->end();i++) {
            Cluster *c=*i;
            for(vector<unsigned>::iterator j=c->nodes.begin();j!=c->nodes.end();j++) {
                for(vector<unsigned>::iterator k=c->nodes.begin();k!=c->nodes.end();k++) {
                    unsigned a=*j, b=*k;
                    if(a==b) continue;
                    D[a][b]/=c->internalEdgeWeightFactor;
                }
            }
        }
    }
    // Lij_{i!=j}=1/(Dij^2)
    //
    for(unsigned i = 0; i<n; i++) {
        X[i]=rs[i]->getCentreX();
        Y[i]=rs[i]->getCentreY();
        double degree = 0;
        for(unsigned j=0;j<n;j++) {
            double w = edge_length * D[i][j];
            Dij[i*n+j]=w;
            if(i==j) continue;
            degree+=lap2[i*n+j]=w>1e-30?1./(w*w):0;
        }
        if(stickyNodes) {
            // stickyNodes adds a small force attracting nodes 
            // back to their starting positions
            degree+=stickyWeight;
        }
        lap2[i*n+i]=-degree;
        delete [] D[i];
    }
    delete [] D;
}

void ConstrainedMajorizationLayout::majlayout(
        valarray<double> const & Dij, GradientProjection* gp, valarray<double>& coords,valarray<double> const * startCoords) 
{
    double L_ij,dist_ij,degree;
    unsigned N=n;
    if(constrainedLayout) {
        N=gp->getSize();
    }
    /* compute the vector b */
    /* multiply on-the-fly with distance-based laplacian */
    valarray<double> b(N);
    for (unsigned i = 0; i < (unsigned)N; i++) {
        b[i] = degree = 0;
        if(i<n) {
            for (unsigned j = 0; j < n; j++) {
                if (j == i) continue;
                dist_ij = euclidean_distance(i, j);
                /* skip zero distances */
                if (dist_ij > 1e-30 && Dij[i*n+j] > 1e-30) {
                    /* calculate L_ij := w_{ij}*d_{ij}/dist_{ij} */
                    L_ij = 1.0 / (dist_ij * Dij[i*n+j]);
                    degree -= L_ij;
                    b[i] += L_ij * coords[j];
                }
            }
            if(stickyNodes) {
                b[i]-=stickyWeight*(*startCoords)[i];
            }
            b[i] += degree * coords[i];
        }
        assert(!std::isnan(b[i]));
    }
    if(constrainedLayout) {
        gp->solve(b);
    } else {
        conjugate_gradient(lap2, coords, b, n, tol, n);
    }
    moveBoundingBoxes();
}
inline double ConstrainedMajorizationLayout
::compute_stress(valarray<double> const &Dij) {
    double sum = 0, d, diff;
    for (unsigned i = 1; i < n; i++) {
        for (unsigned j = 0; j < i; j++) {
            d = Dij[i*n+j];
            diff = d - euclidean_distance(i,j);
            sum += diff*diff / (d*d);
        }
    }
    return sum;
}

void ConstrainedMajorizationLayout::run(bool x, bool y) {
    if(constrainedLayout) {
        vector<vpsc::Rectangle*>* pbb = boundingBoxes.empty()?NULL:&boundingBoxes;
        gpX=new GradientProjection(
            HORIZONTAL,lap2,X,tol,1000,ccsx,avoidOverlaps,pbb,NULL);
        gpY=new GradientProjection(
            VERTICAL,lap2,Y,tol,1000,ccsy,avoidOverlaps,pbb,NULL);
    }
    if(n>0) do {
        // to enforce clusters with non-intersecting, convex boundaries we
        // could create cluster boundaries here with chains of dummy nodes (a
        // dummy node for each vertex of the convex hull) connected by dummy
        // straightenEdges and we'd then continue on to straightenEdges below.
        // This should work assuming we already have a feasible (i.e. non
        // overlapping cluster) state.  The former could be enforced by an
        // earlier stage involving simple rectangular cluster boundaries.
        vector<straightener::Edge*> cedges;
        if(!straightenEdges && nonOverlappingClusters) {
            straightenEdges = &cedges;
        }
        if(preIteration) {
            (*preIteration)(gpX,gpY);
            for(vector<Lock>::iterator l=preIteration->locks.begin();
                    l!=preIteration->locks.end();l++) {
                X[l->id]=l->x;
                Y[l->id]=l->y;
                boundingBoxes[l->id]->moveCentre(l->x,l->y);
                if(constrainedLayout) {
                    gpX->fixPos(l->id,X[l->id]); 
                    gpY->fixPos(l->id,Y[l->id]);
                }
            }
        }
        /* Axis-by-axis optimization: */
        if(straightenEdges) {
            if(x) straighten(*straightenEdges,HORIZONTAL);
            if(y) straighten(*straightenEdges,VERTICAL);
        } else {
            if(x) majlayout(Dij,gpX,X,startX);
            if(y) majlayout(Dij,gpY,Y,startY);
        }
        if(clusters) {
            for(Clusters::iterator c=clusters->begin();c!=clusters->end();c++) {
                (*c)->computeBoundary(boundingBoxes);
            }
        }
        if(preIteration && constrainedLayout) {
            for(vector<Lock>::iterator l=preIteration->locks.begin();
                    l!=preIteration->locks.end();l++) {
                gpX->unfixPos(l->id);
                gpY->unfixPos(l->id);
            }
        }
    } while(!done(compute_stress(Dij),X,Y));
}
void ConstrainedMajorizationLayout::straighten(vector<straightener::Edge*>& sedges, Dim dim) {
	vector<straightener::Node*> snodes;
	for (unsigned i=0;i<n;i++) {
		snodes.push_back(new straightener::Node(i,boundingBoxes[i]));
	}
    vector<straightener::Cluster*> sclusters;
    if(nonOverlappingClusters && clusters) {
        generateClusterBoundaries(dim,snodes,sedges,boundingBoxes,*clusters,sclusters);
    }
    CompoundConstraints *gcs=dim==HORIZONTAL?ccsx:ccsy;
    CompoundConstraints cs(gcs?gcs->size():0);
    if(gcs) copy(gcs->begin(),gcs->end(),cs.begin());
    straightener::generateConstraints(dim,snodes,sedges,cs);
    unsigned sn=snodes.size();
    X.resize(sn);
    Y.resize(sn);
    for(unsigned i = 0; i<sn; i++) {
        X[i]=snodes[i]->x;
        Y[i]=snodes[i]->y;
    }
    LinearConstraints linearConstraints;
    for(unsigned i=0;i<sedges.size();i++) {
        sedges[i]->nodePath(snodes,!nonOverlappingClusters);
        vector<unsigned>& path=sedges[i]->path;
        vector<unsigned>& activePath=sedges[i]->activePath;
        // take u and v as the ends of the line
        // unsigned u=path[0];
        // unsigned v=path[path.size()-1];
        double total_length=0;
        for(unsigned j=1;j<path.size();j++) {
            unsigned u=path[j-1], v=path[j];
            total_length+=euclidean_distance(u,v);
        }
        // keep potential bends straight
        for(unsigned j=1;j<activePath.size();j++) {
            unsigned uj=activePath[j-1], vj=activePath[j];
            unsigned u=path[uj], v=path[vj];
            for(unsigned k=uj+1;k<vj;k++) {
                unsigned b=path[k];
		// might be useful to have greater weight for potential bends than actual bends
                linearConstraints.push_back(new cola::LinearConstraint(u,v,b,-potBendWeight,X,Y));
            }
        }
        // straighten actual bends
        for(unsigned j=1;j<activePath.size()-1;j++) {
            unsigned u=path[activePath[j-1]], 
                     b=path[activePath[j]],
                     v=path[activePath[j+1]];
            linearConstraints.push_back(new cola::LinearConstraint(u,v,b,-bendWeight,X,Y));
        }
    }
    //std::cout << "Generated "<<linearConstraints.size()<< " linear constraints"<<std::endl;
    //std::cout << "  snodes.size "<<snodes.size()<< " n="<<n<<std::endl;
    //assert(sn==n+linearConstraints.size());
    valarray<double>& coords=dim==HORIZONTAL?X:Y;
    valarray<double> const * startCoords=dim==HORIZONTAL?startX:startY;
    SparseMap Q(sn);
    for(LinearConstraints::iterator i=linearConstraints.begin();
           i!= linearConstraints.end();i++) {
        LinearConstraint* c=*i;
        Q[make_pair(c->u,c->u)]+=c->w*c->duu;
        Q[make_pair(c->u,c->v)]+=c->w*c->duv;
        Q[make_pair(c->u,c->b)]+=c->w*c->dub;
        Q[make_pair(c->v,c->u)]+=c->w*c->duv;
        Q[make_pair(c->v,c->v)]+=c->w*c->dvv;
        Q[make_pair(c->v,c->b)]+=c->w*c->dvb;
        Q[make_pair(c->b,c->b)]+=c->w*c->dbb;
        Q[make_pair(c->b,c->u)]+=c->w*c->dub;
        Q[make_pair(c->b,c->v)]+=c->w*c->dvb;
    }
    double boundaryWeight = 0.0001;
    for(unsigned i=0;i<sclusters.size();i++) {
        // for each cluster boundary chain create an attractive force between
        // each pair of adjacent nodes
        straightener::Cluster* c = sclusters[i];
        for(unsigned j=0;j<c->boundary.size();j++) {
            straightener::Edge* e = c->boundary[j];
            Q[make_pair(e->startNode,e->endNode)]+=boundaryWeight;
            Q[make_pair(e->endNode,e->startNode)]+=boundaryWeight;
            Q[make_pair(e->startNode,e->startNode)]-=boundaryWeight;
            Q[make_pair(e->endNode,e->endNode)]-=boundaryWeight;
        }
    }
    GradientProjection gp(dim,lap2,coords,tol,100,&cs,None,
            &boundingBoxes,&Q);
    if(preIteration) {
        for(vector<Lock>::iterator l=preIteration->locks.begin();
                l!=preIteration->locks.end();l++) {
            gp.fixPos(l->id,dim==HORIZONTAL?X[l->id]:Y[l->id]); 
        }
    }
    constrainedLayout = true;
    majlayout(Dij,&gp,coords,startCoords);
    for(unsigned i=0;i<sedges.size();i++) {
        sedges[i]->createRouteFromPath(X,Y);
        sedges[i]->dummyNodes.clear();
        sedges[i]->path.clear();
    }
    for(unsigned i=0;i<sclusters.size();i++) {
        straightener::Cluster *sc = sclusters[i];
        //sc->updateActualBoundary();
        delete sc;
    }
    for(unsigned i=gcs?gcs->size():0;i<cs.size();i++) {
        delete cs[i];
    }
    for(unsigned i=0;i<linearConstraints.size();i++) {
        delete linearConstraints[i];
    }
    for(unsigned i=0;i<snodes.size();i++) {
        delete snodes[i];
    }
    snodes.resize(n);
}

Rectangle bounds(vector<Rectangle*>& rs) {
    assert(!rs.empty());
    
    double left = rs[0]->getMinX(), right = rs[0]->getMaxX(), 
        top = rs[0]->getMinY(), bottom = rs[0]->getMaxY();
    
    for(unsigned i = 1; i < rs.size(); i++) {
        left = min(left,rs[i]->getMinX());
        right = max(right,rs[i]->getMaxX());
        top = min(top,rs[i]->getMinY());
        bottom = max(bottom,rs[i]->getMaxY());
    }
    return Rectangle(left, right, top, bottom);
}

} // namespace cola

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
