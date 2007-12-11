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
using vpsc::Constraint;
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
Projection::Projection(const unsigned n, CompoundConstraints * ccs, FixedList const & fixed) 
    : ccs(ccs), n(n), fixed(fixed)
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
void Projection::solve(Variables &lvs, set<Constraint*> &lcs, valarray<double> & X) {
    unsigned N=X.size();
    //assert(X.size()==vars.size()+lvs.size());
    Variables vs(vars);
    vs.insert(vs.end(),lvs.begin(),lvs.end());
    for(unsigned i=0;i<N;i++) {
        vs[i]->desiredPosition = X[i];
        vs[i]->weight=fixed.check(i)?100000:1;
        //vs[i]->block=NULL;
    }
    Constraints cs(gcs);
    cs.insert(cs.end(),lcs.begin(),lcs.end());
    vpsc::IncSolver vpsc(vs,cs);
    vpsc.solve();
    for(unsigned i=0;i<N;i++) {
        X[i] = vs[i]->finalPosition;
    }
    printf("all local constraints after solve:\n");
    for(set<Constraint*>::iterator i=lcs.begin();i!=lcs.end();i++) {
        Constraint* c=*i;
        printf("    v[%d]+%f<=v[%d]:\n",c->left->id,c->gap,c->right->id);
    }
    if(ccs) {
        for(CompoundConstraints::const_iterator c=ccs->begin();
                c!=ccs->end();c++) {
            (*c)->updatePosition();
        }
    }
}
void Projection::solve(Variables &lvs, set<Constraint*> &lcs) {
    Variables vs(vars);
    vs.insert(vs.end(),lvs.begin(),lvs.end());
    for(unsigned i=0;i<vs.size();i++) {
        vs[i]->weight=fixed.check(i)?100000:1;
        //printf("solve: vs[%d]=%f\n",i,vs[i]->desiredPosition);
    }
    Constraints cs(gcs);
    cs.insert(cs.end(),lcs.begin(),lcs.end());
    vpsc::IncSolver vpsc(vs,cs);
    vpsc.solve();
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
      fixed(FixedList(n)),
      constrainedX(false), constrainedY(false),
      ccsx(NULL), ccsy(NULL),
      avoidOverlaps(false),
      px(NULL), py(NULL),
      topology(false)
{
    boundingBoxes.resize(n);
    copy(rs.begin(),rs.end(),boundingBoxes.begin());
    done.reset();
    for(unsigned i=0;i<n;i++) {
        vpsc::Rectangle *r=rs[i];
        X[i]=r->getCentreX();
        Y[i]=r->getCentreY();
        //printf("r[%d]=(%f,%f,%f,%f)\n",i,r->getMinX(),r->getMinY(),r->width(),r->height());
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
    // uses steepest descent techniques with linesearch stepsize selection to
    // ensure monotonic reduction of stress
    //   However: while monotonic reduction of stress ensures convergence, it is not 
    //   enough to prevent apparent vibration
    //   On the plus side, we can easily try out any goal function as long as we
    //   can compute the derivative g to find the descent direction and the second
    //   derivative H to compute the initial stepsize g'g/(g'Hg)
    //
    //   To handle constraints we use a gradient projection technique, treating the
    //   axes separately.  Note that separating axes may ruin convergence to true
    //   local minima, but projection in a single axis is usually a tractable problem
    //   whereas projection over both axes with a disjunction of constraints is most
    //   likely NP-hard.
    if(constrainedX) {
        px.reset(new Projection(n,ccsx,fixed));
    }
    if(constrainedY) {
        py.reset(new Projection(n,ccsy,fixed));
    }
    //printf("n==%d\n",n);
    if(n==0) return;
    double stress=computeStress();
    /*
    if(straightenEdges) {
        straightener::setEdgeLengths(D,*straightenEdges);
        stress+=straightener::computeStressFromRoutes(straighteningStrength,*straightenEdges);
    }
    */
    bool firstPass=true;
    do {
        if(preIteration) {
            if ((*preIteration)()) {
                bool stressNeedsUpdate=false;
                for(vector<Lock>::iterator l=preIteration->locks.begin();
                        l!=preIteration->locks.end();l++) {
                    unsigned id=l->id;
                    double x=l->x, y=l->y;
                    X[id]=x;
                    Y[id]=y;
                    boundingBoxes[id]->moveCentre(x,y);
                    fixed.set(id);
                    stressNeedsUpdate=true;
                }
                if(stressNeedsUpdate) {
                    stress=computeStress();
                    /*
                    if(straightenEdges) {
                        stress+=straightener::computeStressFromRoutes(straighteningStrength,*straightenEdges);
                    }
                    */
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
        fixed.unsetAll();
    } while(!done(stress,X,Y));
}
/**
 * The following computes an unconstrained solution then uses Projection to
 * make this solution feasible with respect to constraints by moving things as
 * little as possible.  If "meta-constraints" such as avoidOverlaps or edge
 * straightening are required then dummy variables will be generated.
 */
double ConstrainedFDLayout::applyForcesAndConstraints(const cola::Dim dim, const double oldStress,
        const bool firstPass) {
    valarray<double> g(n);
    valarray<double>& coords=dim==cola::HORIZONTAL?X:Y;
    SparseMap HMap(n);
    computeForces(dim,HMap,g);
    /*
    printf("g=[");
    for(unsigned i=0;i<n;i++) {
        printf("%f ",g[i]);
    }
    printf("]\n");
    */
    //printf("sparse matrix nonzero size=%d\n",HMap.nonZeroCount());
    //printf(" dim=%d alpha: ",dim);
    Projection *p = dim==HORIZONTAL?px.get():py.get();
    if(!p)  { 
        // unconstrained
        SparseMatrix H(HMap);
        //H.print();
        valarray<double> oldCoords=coords;
        double stress = applyDescentVector(g,oldCoords,coords,oldStress,computeStepSize(H,g,g));
        return stress;
    }
    Variables lvs;
    set<Constraint*> lcs;
    if(avoidOverlaps) {
        /*
        if(p->ccs) {
            for(CompoundConstraints::const_iterator c=p->ccs->begin();
                    c!=p->ccs->end();c++) {
                OrthogonalEdgeConstraint* e=dynamic_cast<OrthogonalEdgeConstraint*>(*c);
                if(e) {
                    e->generateTopologyConstraints(dim,boundingBoxes,p->vars,lcs);
                }
            }
        }
        */
        Constraints overlapConstraints;
        if(dim==HORIZONTAL) {
            Rectangle::setXBorder(0.1);
            generateXConstraints(boundingBoxes,p->vars,overlapConstraints,true); 
            Rectangle::setXBorder(0);
        } else {
            for(unsigned i=0;i<n;i++) { 
                // need to make sure that the bounding box position matches most up-to-date
                // x-position before computing y-constraints
                boundingBoxes[i]->moveCentreX(X[i]);
            }
            generateYConstraints(boundingBoxes,p->vars,overlapConstraints); 
        }
        lcs.insert(overlapConstraints.begin(),overlapConstraints.end());
    }
    SparseMatrix H(HMap);
    valarray<double> oldCoords=coords;
    applyDescentVector(g,oldCoords,coords,oldStress,computeStepSize(H,g,g));
    p->solve(lvs,lcs,coords);
    delete_vector(lvs);
    for_each(lcs.begin(),lcs.end(),delete_object());
    if(!firstPass) {
        valarray<double> d(n);
        d=oldCoords-coords;
        double stepsize=computeStepSize(H,g,d);
        stepsize=max(0.,min(stepsize,1.));
        //printf(" dim=%d beta: ",dim);
        return applyDescentVector(d,oldCoords,coords,oldStress,stepsize);
    }
    return computeStress();
}
double ConstrainedFDLayout::applyDescentVector(
        valarray<double> const &d,
        valarray<double> const &oldCoords,
        valarray<double> &coords,
        const double oldStress,
        double stepsize/*,
        topology::TopologyConstraints *s*/
        ) const {
    assert(d.size()==oldCoords.size());
    assert(d.size()==coords.size());
    while(fabs(stepsize)>0.00000000001) {
        coords=oldCoords-stepsize*d;
        double stress=computeStress(/*s*/);
        //printf(" applyDV: oldstress=%f, stress=%f, stepsize=%f\n", oldStress,stress,stepsize);
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
        //if(fixedPos[u]) continue;
        // Stress model
        double Huu=0;
        for(unsigned v=0;v<n;v++) {
            if(u==v) continue;
            // no forces between disconnected parts of the graph
            if(G[u][v]==numeric_limits<unsigned>::max()) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            // we don't want long range attractive forces between
            // not immediately connected nodes
            if(G[u][v]>1 && l>d) continue;
            // if we compute straightenEdge forces then they replace
            // forces between adjacent nodes
            if(topology && G[u][v]==1) continue;
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
    //assert(numerator>=0);
    //assert(denominator>=0);
    if(denominator==0) return 0;
    return numerator/denominator;
}
double ConstrainedFDLayout::computeStress(/*topology::TopologyConstraints *t*/) const {
    double stress=0;
    for(unsigned u=0;u<n-1;u++) {
        for(unsigned v=u+1;v<n;v++) {
            // no forces between disconnected parts of the graph
            if(G[u][v]==numeric_limits<unsigned>::max()) continue;
            // if we compute topology forces then they replace
            // forces between adjacent nodes
            if(topology && G[u][v]==1) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            // we don't want long range attractive forces between 
            // not immediately connected nodes
            if(G[u][v]>1 && l>d) continue;
            double d2=d*d;
            double rl=d-l;
            stress+=rl*rl/d2;
        }
    }
    /*
    if(t) {
        stress+=t->computeStress();
    }
    */
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
