#include "commondefs.h"

#include <cmath>
#include "cola.h"
#include "shortest_paths.h"
#include "straightener.h"
#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/rectangle.h>

namespace cola {
template <class T>
void delete_vector(vector<T*> &v) {
    for_each(v.begin(),v.end(),delete_object());
    v.clear();
}
inline double dotProd(valarray<double> x, valarray<double> y) {
    assert(x.size()==y.size());
    double dp=0;
    for(unsigned i=0;i<x.size();i++) {
        dp+=x[i]*y[i]; 
    }
    return dp;
}
/**
 * Single instance of projection per call to ConstrainedFDLayout::run()
 *
 * Created in Projection::Projection:
 *   o Global variables:
 *     - for each actual graph node, and dummy vars for each compound constraint
 *   o Global constraints:
 *     - Compound constraints specified by set[X|Y]Constraints
 * 
 * Generated in ConstrainedFDLayout::applyConstraints
 * and persisting for a single call to Projection::solve()):
 *   o local variables
 *     - dummy straightening vars
 *   o local constraints
 *     - non-overlap constraints
 *     - straightening constraints
 *
 */
Projection::Projection(const unsigned n, CompoundConstraints * ccs, valarray<bool> const & fixed) 
    : n(n), ccs(ccs), fixed(fixed)
{
    for(unsigned i=0;i<n;i++) {
        vars.push_back(new vpsc::Variable(i,1,1));
    }
    if(ccs) {
        for(CompoundConstraints::const_iterator c=ccs->begin();
                c!=ccs->end();c++) {
            (*c)->generateVariables(vars);
        }
        for(CompoundConstraints::const_iterator c=ccs->begin();
                c!=ccs->end();c++) {
            (*c)->generateSeparationConstraints(vars,gcs);
        }
    }
}
Projection::~Projection() {
    delete_vector(vars);
    vars.clear();
}
void Projection::solve(Variables &lvs, Constraints &lcs, valarray<double> & X) {
    unsigned N=X.size();
    assert(X.size()==vars.size()+lvs.size());
    Variables vs(vars);
    vs.insert(vs.end(),lvs.begin(),lvs.end());
    for(unsigned i=0;i<N;i++) {
        vs[i]->desiredPosition = X[i];
        vs[i]->weight=i<n&&fixed[i]?100000:1;
    }
    Constraints cs(gcs);
    cs.insert(cs.end(),lcs.begin(),lcs.end());
    vpsc::IncSolver vpsc(vs,cs);
    vpsc.solve();
    for(unsigned i=0;i<N;i++) {
        X[i] = vs[i]->finalPosition;
    }
    if(ccs) {
        for(CompoundConstraints::const_iterator c=ccs->begin();
                c!=ccs->end();c++) {
            (*c)->updatePosition();
        }
    }
}
template <typename T>
void dumpSquareMatrix(unsigned n, T** L) {
    printf("Matrix %dX%d\n{",n,n);
    for(unsigned i=0;i<n;i++) {
        printf("{");
        for(unsigned j=0;j<n;j++) {
            std::cout<<L[i][j];
            char c=j==n-1?'}':',';
            printf("%c",c);
        }
        char c=i==n-1?'}':',';
        printf("%c\n",c);
    }
}
ConstrainedFDLayout::ConstrainedFDLayout(
        vector<Rectangle*>& rs,
        vector<Edge> const & es,
        RootCluster* clusterHierarchy,
        double const idealLength,
        std::valarray<double> const * eweights,
        TestConvergence& done,
        PreIteration* preIteration)
    : n(rs.size()),
      X(valarray<double>(n)),
      Y(valarray<double>(n)),
      done(done),
      preIteration(preIteration),
      fixedPos(valarray<bool>(n)),
      constrainedX(false), constrainedY(false),
      ccsx(NULL), ccsy(NULL),
      avoidOverlaps(false),
      px(NULL), py(NULL),
      straightenEdges(NULL)
{
    boundingBoxes.resize(n);
    copy(rs.begin(),rs.end(),boundingBoxes.begin());
    done.reset();
    for(unsigned i=0;i<n;i++) {
        X[i]=rs[i]->getCentreX();
        Y[i]=rs[i]->getCentreY();
    }
    D=new double*[n];
    G=new unsigned*[n];
    for(unsigned i=0;i<n;i++) {
        D[i]=new double[n];
        G[i]=new unsigned[n];
    }
    shortest_paths::johnsons(n,G,es);
    shortest_paths::johnsons(n,D,es,eweights);
    //dumpSquareMatrix<unsigned>(n,G);
    //dumpSquareMatrix<double>(n,D);
    fixedPos=false;
    for(unsigned i=0;i<n;i++) {
        for(unsigned j=0;j<n;j++) {
            if(i==j) continue;
            if(!eweights) {
                D[i][j]*=idealLength;
            }
        }
    }
}

void ConstrainedFDLayout::run(const bool xAxis, const bool yAxis) {
    if(constrainedX) {
        px.reset(new Projection(n,ccsx,fixedPos));
    }
    if(constrainedY) {
        py.reset(new Projection(n,ccsy,fixedPos));
    }
    double stress=DBL_MAX;//computeStress();
    bool firstPass=true;
    printf("n==%d\n",n);
    if(n>0) do {
        if(preIteration) {
            if ((*preIteration)()) {
                for(vector<Lock>::iterator l=preIteration->locks.begin();
                        l!=preIteration->locks.end();l++) {
                    unsigned id=l->id;
                    double x=l->x, y=l->y;
                    X[id]=x;
                    Y[id]=y;
                    boundingBoxes[id]->moveCentre(x,y);
                    fixedPos[id]=true;
                }
            } else { break; }
        }
        if(xAxis) {
            stress=applyForcesAndConstraints(HORIZONTAL,stress,firstPass);
        }
        if(yAxis) {
            stress=applyForcesAndConstraints(VERTICAL,stress,firstPass);
        }
        firstPass=false;
        move();
        fixedPos=false;
    } while(!done(stress,X,Y));
}
/**
 * The following computes an unconstrained solution then uses Projection to
 * make this solution feasible with respect to constraints by moving things as
 * little as possible.  If "meta-constraints" such as avoidOverlaps or edge
 * straightening are required then dummy variables will be generated.
 */
double ConstrainedFDLayout::applyForcesAndConstraints(const Dim dim, const double oldStress,
        const bool firstPass) {
    valarray<double> g(n);
    valarray<double> &coords = (dim==HORIZONTAL)?X:Y;
    SparseMap HMap(n);
    computeForces(dim,HMap,g);
    //printf("sparse matrix nonzero size=%d\n",HMap.nonZeroCount());
    //printf(" dim=%d alpha: ",dim);
    Projection *p = dim==HORIZONTAL?px.get():py.get();
    if(!p)  { 
        // unconstrained
        SparseMatrix H(HMap);
        valarray<double> oldCoords=coords;
        return applyDescentVector(g,oldCoords,coords,oldStress,computeStepSize(H,g,g));
    }
    Variables vs;
    Variables lvs;
    Constraints lcs;
    vector<Rectangle*> lrs;
    if(avoidOverlaps||straightenEdges) {
        assert(p!=NULL);
        for(unsigned i=0;i<boundingBoxes.size();i++) {
            vpsc::Rectangle* r=boundingBoxes[i];
            if(!r->allowOverlap()) {
                vpsc::Rectangle* r2=new vpsc::Rectangle(*r);
                r2->moveCentre(X[i],Y[i]);
                vs.push_back(p->vars[i]);
                lrs.push_back(r2);
            }
        }
    }
    if(avoidOverlaps) {
        if(dim==HORIZONTAL) {
            Rectangle::setXBorder(0.1);
            generateXConstraints(lrs,vs,lcs,true); 
            Rectangle::setXBorder(0);
        } else {
            generateYConstraints(lrs,vs,lcs); 
        }
    }
    if(straightenEdges) {
        straightener::Straightener s(straighteningStrength, dim,lrs,*straightenEdges,vs,lvs,lcs,coords,g);
        HMap.resize(s.N);
        valarray<double> oldCoords=s.coords;
        s.computeForces(HMap);
        SparseMatrix H(HMap);
        double stress=applyDescentVector(s.g,oldCoords,s.coords,oldStress,computeStepSize(H,s.g,s.g),&s);
        p->solve(lvs,lcs,s.coords);
        s.updateNodePositions();
        dummyNodesX.resize(s.dummyNodesX.size());
        dummyNodesY.resize(s.dummyNodesY.size());
        dummyNodesX=s.dummyNodesX;
        dummyNodesY=s.dummyNodesY;
        /*
        printf("have %d dummy nodes\n", (int) dummyNodesX.size());
        for(unsigned i=0;i<lcs.size();i++) {
            cout<<"  "<<*lcs[i]<<endl;
        }
        */
        delete_vector(lrs);
        delete_vector(lvs);
        delete_vector(lcs);
        if(!firstPass) {
            valarray<double> d(s.N);
            d=oldCoords-s.coords;
            double stepsize=computeStepSize(H,s.g,d);
            stepsize=max(0.,min(stepsize,1.));
            //printf(" dim=%d beta: ",dim);
            stress=applyDescentVector(d,oldCoords,s.coords,oldStress,stepsize,&s);
            s.updateNodePositions();
        }
        double* p=&s.coords[0];
        copy(p,p+n,&coords[0]);
        s.finalizeRoutes();
        return stress;
    } else {
        SparseMatrix H(HMap);
        valarray<double> oldCoords=coords;
        applyDescentVector(g,oldCoords,coords,oldStress,computeStepSize(H,g,g));
        p->solve(lvs,lcs,coords);
        delete_vector(lrs);
        delete_vector(lvs);
        delete_vector(lcs);
        if(!firstPass) {
            valarray<double> d(n);
            d=oldCoords-coords;
            double stepsize=computeStepSize(H,g,d);
            stepsize=max(0.,min(stepsize,1.));
            //printf(" dim=%d beta: ",dim);
            return applyDescentVector(d,oldCoords,coords,oldStress,stepsize);
        }
    }
    return computeStress();
}
double ConstrainedFDLayout::applyDescentVector(
        valarray<double> const &d,
        valarray<double> const &oldCoords,
        valarray<double> &coords,
        const double oldStress,
        double stepsize,
        straightener::Straightener *s
        ) const {
    assert(d.size()==oldCoords.size());
    assert(d.size()==coords.size());
    while(stepsize>0.00000000001) {
        coords=oldCoords-stepsize*d;
        double stress=computeStress();
        if(s) {
            s->updateNodePositions();
            double sstress=s->computeStress(coords);
            //printf("  s1=%f,s2=%f ",stress,sstress);
            stress+=sstress;
        }
        printf(" applyDV: oldstress=%f, stress=%f, stepsize=%f\n", oldStress,stress,stepsize);
        if(oldStress>=stress) {
            return stress;
        }
        coords=oldCoords;
        stepsize*=0.5;
    }
    return oldStress;
}
void ConstrainedFDLayout::computeForces(
        const Dim dim,
        SparseMap &H,
        valarray<double> &g) {
    if(n==1) return;
    g=0;
    // for each node:
    for(unsigned u=0;u<n;u++) {
        if(fixedPos[u]) continue;
        // Stress model
        double Huu=0;
        for(unsigned v=0;v<n;v++) {
            if(u==v) continue;
            // no forces between disconnected parts of the graph
            if(G[u][v]==numeric_limits<unsigned>::max()) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            // we don't want long range attractive forces between not immediately connected nodes
            if(G[u][v]>1 && l>d) continue;
            double d2=d*d;
            /* force apart zero distances */
            if (l < 1e-30) {
                l=0.1;
            }
            double dx=dim==HORIZONTAL?rx:ry;
            double dy=dim==HORIZONTAL?ry:rx;
            g[u]+=dx*(l-d)/(d2*l);
            Huu-=H(u,v)=(d*dy*dy/(l*l*l)-1)/d2;
        }
        H(u,u)=Huu;
    }
}
double ConstrainedFDLayout::computeStepSize(
        SparseMatrix const &H, 
        valarray<double> const &g, 
        valarray<double> const &d) const
{
    assert(g.size()==d.size());
    assert(g.size()==H.rowSize());
    // stepsize = g'd / (d' H d)
    double numerator = dotProd(g,d);
    valarray<double> Hd(d.size());
    H.rightMultiply(d,Hd);
    double denominator = dotProd(d,Hd);
    return numerator/(2.*denominator);
}
double ConstrainedFDLayout::computeStress() const {
    double stress=0;
    for(unsigned u=0;u<n;u++) {
        for(unsigned v=0;v<n;v++) {
            if(u==v) continue;
            // no forces between disconnected parts of the graph
            if(G[u][v]==numeric_limits<unsigned>::max()) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            // we don't want long range attractive forces between not immediately connected nodes
            if(G[u][v]>1 && l>d) continue;
            double d2=d*d;
            double rl=d-l;
            stress+=rl*rl/d2;
        }
    }
    return stress;
}
void ConstrainedFDLayout::move() {
        for(unsigned i=0;i<n;i++) {
            boundingBoxes[i]->moveCentre(X[i],Y[i]);
        }
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
// vim: filetype=cpp:cindent:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
