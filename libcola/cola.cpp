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
        Clusters const *cs,
        double const idealLength,
        std::valarray<double> const * eweights,
        TestConvergence& done)
    : n(rs.size()),
      lap2(valarray<double>(n*n)), 
      Dij(valarray<double>(n*n)),
      tol(0.0001), done(done),
      X(valarray<double>(n)), Y(valarray<double>(n)),
      constrainedLayout(false),
      clusters(cs), linearConstraints(NULL),
      gpX(NULL), gpY(NULL),
      avoidOverlaps(None),
      straightenEdges(NULL),
      pbcx(NULL), pbcy(NULL),
      scx(NULL), scy(NULL),
      acsx(NULL), acsy(NULL),
      bendWeight(0.01), potBendWeight(0.1)
{
    boundingBoxes = new Rectangle*[rs.size()];
    copy(rs.begin(),rs.end(),boundingBoxes);

    done.reset();

    assert(!eweights||eweights->size()==es.size()); 

    double** D=new double*[n];
    for(unsigned i=0;i<n;i++) {
        D[i]=new double[n];
    }
    shortest_paths::johnsons(n,D,es,eweights);
    edge_length = idealLength;
    if(clusters!=NULL) {
        for(Clusters::const_iterator i=clusters->begin();i!=clusters->end();i++) {
            Cluster *c=*i;
            for(Cluster::iterator j=c->begin();j!=c->end();j++) {
                for(Cluster::iterator k=c->begin();k!=c->end();k++) {
                    unsigned a=*j, b=*k;
                    if(a==b) continue;
                    D[a][b]/=2.;
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
        lap2[i*n+i]=-degree;
        delete [] D[i];
    }
    delete [] D;
}

void ConstrainedMajorizationLayout::majlayout(
        valarray<double> const & Dij, GradientProjection* gp, valarray<double>& coords, int N) 
{
    double L_ij,dist_ij,degree;
    if(N<0) {
        N=n;
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
                if (dist_ij > 1e-30 && Dij[i*n+j] > 1e-30) {	/* skip zero distances */
                    /* calculate L_ij := w_{ij}*d_{ij}/dist_{ij} */
                    L_ij = 1.0 / (dist_ij * Dij[i*n+j]);
                    degree -= L_ij;
                    b[i] += L_ij * coords[j];
                }
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
        gpX=new GradientProjection(
            HORIZONTAL,lap2,X,tol,100,acsx,dcsx,avoidOverlaps,boundingBoxes,pbcx,NULL,scx);
        gpY=new GradientProjection(
            VERTICAL,lap2,Y,tol,100,acsy,dcsy,avoidOverlaps,boundingBoxes,pbcy,NULL,scy);
    }
    if(n>0) do {
        /* Axis-by-axis optimization: */
        if(straightenEdges) {
            if(x) straighten(*straightenEdges,HORIZONTAL);
            if(y) straighten(*straightenEdges,VERTICAL);
        } else {
            if(x) majlayout(Dij,gpX,X);
            if(y) majlayout(Dij,gpY,Y);
        }
    } while(!done(compute_stress(Dij),X,Y));
}
void ConstrainedMajorizationLayout::straighten(vector<straightener::Edge*>& sedges, Dim dim) {
	vector<straightener::Node*> snodes;
	for (unsigned i=0;i<n;i++) {
		snodes.push_back(new straightener::Node(i,boundingBoxes[i]));
	}
    SimpleConstraints *gcs=dim==HORIZONTAL?scx:scy;
    SimpleConstraints cs(gcs?gcs->size():0);
    if(gcs) copy(gcs->begin(),gcs->end(),cs.begin());
    straightener::generateConstraints(snodes,sedges,cs,dim);
    unsigned sn=snodes.size();
    X.resize(sn);
    Y.resize(sn);
    for(unsigned i = 0; i<sn; i++) {
        X[i]=snodes[i]->x;
        Y[i]=snodes[i]->y;
    }
    LinearConstraints linearConstraints;
    for(unsigned i=0;i<sedges.size();i++) {
        sedges[i]->nodePath(snodes);
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
		// greater weight for potential bends than actual bends
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
    assert(sn==n+linearConstraints.size());
    valarray<double>& coords=dim==HORIZONTAL?X:Y;
    SparseMatrix::SparseMap Q;
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
    AlignmentConstraints *acs=dim==HORIZONTAL?acsx:acsy;
    DistributionConstraints *dcs=dim==HORIZONTAL?dcsx:dcsy;
    PageBoundaryConstraints *pbcs=dim==HORIZONTAL?pbcx:pbcy;
    SparseMatrix sparseQ(Q,sn);
    GradientProjection gp(dim,lap2,coords,tol,100,
            (AlignmentConstraints*)acs,dcs,None,
            (vpsc::Rectangle**)boundingBoxes,(PageBoundaryConstraints*)pbcs,
            &sparseQ,&cs);
    constrainedLayout = true;
    majlayout(Dij,&gp,coords,sn);
    for(unsigned i=0;i<sedges.size();i++) {
        sedges[i]->createRouteFromPath(X,Y);
        sedges[i]->dummyNodes.clear();
        sedges[i]->path.clear();
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

void ConstrainedMajorizationLayout::setupConstraints(
        AlignmentConstraints* acsx, AlignmentConstraints* acsy,
        NonOverlapConstraints avoidOverlaps, 
        PageBoundaryConstraints* pbcx, PageBoundaryConstraints* pbcy,
        SimpleConstraints* scx, SimpleConstraints* scy,
        vector<straightener::Edge*>* straightenEdges,
        double bendWeight, double potBendWeight) {
    constrainedLayout = true;
    this->avoidOverlaps = avoidOverlaps;
    this->scx=scx; this->scy=scy;
    this->acsx=acsx; this->acsy=acsy;
    this->pbcx = pbcx; this->pbcy = pbcy;
    this->straightenEdges = straightenEdges;
    this->bendWeight = bendWeight;
    this->potBendWeight = potBendWeight;
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
