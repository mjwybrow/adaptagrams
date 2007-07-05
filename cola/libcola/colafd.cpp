#include "commondefs.h"

#include <cmath>
#include "cola.h"
#include "shortest_paths.h"
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
    for(CompoundConstraints::const_iterator c=ccs->begin();
            c!=ccs->end();c++) {
        (*c)->generateVariables(vars);
    }
    for(CompoundConstraints::const_iterator c=ccs->begin();
            c!=ccs->end();c++) {
        (*c)->generateSeparationConstraints(vars,gcs);
    }
}
Projection::~Projection() {
    delete_vector(vars);
    vars.clear();
}
void Projection::setDesiredPositions(valarray<double> &X) {
    for (unsigned i=0;i<X.size();i++) {
        vars[i]->desiredPosition=X[i];
        vars[i]->weight=fixed[i]?100000:1;
    }
}
void Projection::solve(Variables &lvs, Constraints &lcs) {
    Variables vs(vars);
    vs.insert(vs.end(),lvs.begin(),lvs.end());
    Constraints cs(gcs);
    cs.insert(cs.end(),lcs.begin(),lcs.end());
    vpsc::IncSolver vpsc(vs,cs);
    vpsc.solve();
}
void Projection::updatePositions(valarray<double> &X) {
    for (unsigned i=0;i<n;i++) {
        X[i]=vars[i]->position();
    }
    for(CompoundConstraints::const_iterator c=ccs->begin();
            c!=ccs->end();c++) {
        (*c)->updatePosition();
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
    fs=0;
    for(unsigned i=0;i<n;i++) {
        for(unsigned j=0;j<n;j++) {
            if(i==j) continue;
            if(!eweights) {
                D[i][j]*=idealLength;
            }
            double d=D[i][j];
            fs+=1./(d*d);
        }
    }
    // why (n-1)? Well 1/fs wasn't enough for big graphs and 2/fs caused oscillation in 2 nodes graphs!
    fs=(n-1)/fs;
}

void ConstrainedFDLayout::run() {
    double displacement;
    if(constrainedX) px.reset(new Projection(n,ccsx,fixedPos));
    if(constrainedY) py.reset(new Projection(n,ccsy,fixedPos));
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
        valarray<double> X0=X, Y0=Y;
        applyForcesAndConstraints(HORIZONTAL);
        applyForcesAndConstraints(VERTICAL);
        displacement=move(X0,Y0);
        fixedPos=false;
    } while(!done(displacement,X,Y));
}
/**
 * We have an unconstrained solution in X or Y (depending on dim).
 * The following uses Projection to make this solution feasible
 * with respect to constraints by moving things as little as possible.
 * If "meta-constraints" such as avoidOverlaps or edge straightening
 * are required then dummy variables will be generated.
 */
void ConstrainedFDLayout::applyForcesAndConstraints(const Dim dim) {
    valarray<double> f(n);
    computeForces(dim,f);
    valarray<double> &R = (dim==HORIZONTAL)?X:Y;
    for(unsigned u=0;u<n;u++) {
        double dx=f[u];
        R[u]+=dx;
    }
    Projection *p = dim==HORIZONTAL?px.get():py.get();
    auto_ptr<straightener::Straightener> s(NULL);
    Variables vs;
    Variables lvs;
    Constraints lcs;
    vector<Rectangle*> lrs;
    p->setDesiredPositions(R);
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
        s.reset(new straightener::Straightener(dim,lrs,*straightenEdges,vs,lvs,lcs));
        printf("Straightener: vs.size()=%d,lvs.size()=%d,lcs.size()=%d\n",vs.size(),lvs.size(),lcs.size());
        s->applyForces();
        for(unsigned i=0;i<n;i++) {
            if(fixedPos[i]) {
                p->vars[i]->desiredPosition=R[i];
            }
        }
    }
    if(p) {
        p->solve(lvs,lcs);
        p->updatePositions(R);
    }
    if(straightenEdges) {
        s->updateNodePositions();
        dummyNodesX.resize(s->dummyNodesX.size());
        dummyNodesY.resize(s->dummyNodesY.size());
        dummyNodesX=s->dummyNodesX;
        dummyNodesY=s->dummyNodesY;
        printf("have %d dummy nodes\n",dummyNodesX.size());
    }
    delete_vector(lrs);
    delete_vector(lvs);
    delete_vector(lcs);
}
void ConstrainedFDLayout::computeForces(const Dim dim, valarray<double> &f) {
    f=0;
    // for each node:
    for(unsigned u=0;u<n;u++) {
        if(fixedPos[u]) continue;
        // Stress model
        for(unsigned v=0;v<n;v++) {
            if(u==v) continue;
            // no forces between disconnected parts of the graph
            if(G[u][v]==numeric_limits<unsigned>::max()) continue;
            double d=D[u][v];
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            // we don't want long range attractive forces between not immediately connected nodes
            if(G[u][v]>1 && l>d) continue;
            double d2=d*d;
            double rl=(d-l)/l;
            double dx=dim==HORIZONTAL?rx:ry;
            f[u]+=fs*dx*rl/d2;
        }
    }
}
double ConstrainedFDLayout::move(valarray<double> const &X0, valarray<double> const &Y0) {
        double displacement=0;
        for(unsigned i=0;i<n;i++) {
            double x0=X0[i], y0=Y0[i], x1=X[i], y1=Y[i];
            double dx=x1-x0, dy=y1-y0;
            displacement+=sqrt(dx*dx+dy*dy);
            boundingBoxes[i]->moveCentre(x1,y1);
        }
        return displacement;
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
