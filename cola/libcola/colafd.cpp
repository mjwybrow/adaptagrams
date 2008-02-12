#include "commondefs.h"

#include <cmath>
#include "cola.h"
#include "shortest_paths.h"
#include "straightener.h"
#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/rectangle.h>
#include <libtopology/topology_graph.h>
#include <libtopology/topology_constraints.h>
#include "cola_log.h"

namespace cola {
template <class T>
void delete_vector(vector<T*> &v) {
    for_each(v.begin(),v.end(),delete_object());
    v.clear();
}
Resizes PreIteration::__resizesNotUsed;
Locks PreIteration::__locksNotUsed;
inline double dotProd(valarray<double> x, valarray<double> y) {
    assert(x.size()==y.size());
    double dp=0;
    for(unsigned i=0;i<x.size();i++) {
        dp+=x[i]*y[i]; 
    }
    return dp;
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
        vector<vpsc::Rectangle*>& rs,
        vector<Edge> const & es,
        double const idealLength,
        std::valarray<double> const * eweights,
        TestConvergence& done,
        PreIteration* preIteration)
    : n(rs.size()),
      X(valarray<double>(n)),
      Y(valarray<double>(n)),
      done(done),
      preIteration(preIteration),
      constrainedX(false), constrainedY(false),
      ccsx(NULL), ccsy(NULL),
      topologyNodes(NULL),
      topologyRoutes(NULL)
{
    FILELog::ReportingLevel() = logDEBUG1;
    //FILELog::ReportingLevel() = logERROR;
    boundingBoxes.resize(n);
    copy(rs.begin(),rs.end(),boundingBoxes.begin());
    done.reset();
    for(unsigned i=0;i<n;i++) {
        vpsc::Rectangle *r=rs[i];
        X[i]=r->getCentreX();
        Y[i]=r->getCentreY();
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
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::run...";
    if(n==0) return;
    double stress=DBL_MAX;
    bool firstPass=true;
    do {
        if(preIteration) {
            if(!(*preIteration)()) {
                break;
            }
            //printf("preIteration->changed=%d\n",preIteration->changed);
            firstPass=preIteration->changed;
            if(preIteration->resizes.size()>0) {
                FILE_LOG(logDEBUG) << " Resize event!";
                handleResizes(preIteration->resizes);
            }
        }
        if(xAxis) {
            if (firstPass) {
                stress=DBL_MAX;
            }
            stress=applyForcesAndConstraints(HORIZONTAL,stress);
        }
        if(yAxis) {
            if (firstPass) {
                stress=DBL_MAX;
            }
            stress=applyForcesAndConstraints(VERTICAL,stress);
        }
        firstPass=false;
        FILE_LOG(logDEBUG) << "stress="<<stress;
    } while(!done(stress,X,Y));
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::run done.";
}
void setupVarsAndConstraints(unsigned n, const CompoundConstraints* ccs,
        vpsc::Variables& vs, vpsc::Constraints& cs) {
    vs.resize(n);
    for(unsigned i=0;i<n;++i) {
        vs[i]=new vpsc::Variable(i);
    }
    if(ccs) {
        for(CompoundConstraints::const_iterator c=ccs->begin();
                c!=ccs->end();++c) {
            (*c)->generateVariables(vs);
        }
        for(CompoundConstraints::const_iterator c=ccs->begin();
                c!=ccs->end();++c) {
            (*c)->generateSeparationConstraints(vs,cs);
        }
    }
}
void updateCompoundConstraints(const CompoundConstraints* ccs) {
    if(ccs) {
        for(CompoundConstraints::const_iterator c=ccs->begin();
                c!=ccs->end();c++) {
            (*c)->updatePosition();
        }
    }
}
void project(vpsc::Variables& vs, vpsc::Constraints& cs, const topology::DesiredPositions& des, valarray<double>& coords) {
    unsigned n=coords.size();
    assert(vs.size()>=n);
    for(unsigned i=0;i<n;++i) {
        vpsc::Variable* v=vs[i];
        v->desiredPosition = coords[i];
        v->weight=1;
    }
    for(topology::DesiredPositions::const_iterator d=des.begin();
            d!=des.end();++d) {
        assert(d->first<vs.size());
        vpsc::Variable* v=vs[d->first];
        v->desiredPosition = d->second;
        v->weight=10000;
    }
    vpsc::IncSolver s(vs,cs);
    s.solve();
    for(unsigned i=0;i<n;++i) {
        coords[i]=vs[i]->finalPosition;
    }
}
void checkUnsatisfiable(const vpsc::Constraints& cs, 
        UnsatisfiableConstraintInfos* unsatisfiable) {
    for(vpsc::Constraints::const_iterator c=cs.begin();c!=cs.end();++c) {
        if((*c)->unsatisfiable) {
            UnsatisfiableConstraintInfo* i=new UnsatisfiableConstraintInfo(*c);
            unsatisfiable->push_back(i);
        }
    }
}

void ConstrainedFDLayout::handleResizes(const Resizes& resizeList) {
    if(topologyNodes==NULL) {
        assert(topologyRoutes==NULL);
        return;
    }
    assert(topologyRoutes!=NULL);
    // all shapes to be resized are wrapped in a ResizeInfo and
    // placed in a lookup table, resizes, indexed by id
    topology::ResizeMap resizes;
    for(Resizes::const_iterator r=resizeList.begin();r!=resizeList.end();++r) {
        topology::ResizeInfo ri((*topologyNodes)[r->getID()],r->getTarget());
        resizes.insert(make_pair(r->getID(),ri));
    }
    topology::applyResizes(*topologyNodes, *topologyRoutes, resizes);
}
/**
 * The following computes an unconstrained solution then uses Projection to
 * make this solution feasible with respect to constraints by moving things as
 * little as possible.  If "meta-constraints" such as avoidOverlaps or edge
 * straightening are required then dummy variables will be generated.
 */
double ConstrainedFDLayout::applyForcesAndConstraints(const Dim dim, const double oldStress) {
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::applyForcesAndConstraints(): dim="<<dim;
    valarray<double> g(n);
    valarray<double> &coords = (dim==HORIZONTAL)?X:Y;
    /*
    printf("g=[");
    for(unsigned i=0;i<n;i++) {
        printf("%f ",g[i]);
    }
    printf("]\n");
    */
    topology::DesiredPositions des;
    if(preIteration) {
        for(vector<Lock>::iterator l=preIteration->locks.begin();
                l!=preIteration->locks.end();l++) {
            des.push_back(make_pair(l->getID(),l->pos(dim)));
            FILE_LOG(logDEBUG1)<<"desi: v["<<l->getID()<<"]=("<<l->pos(HORIZONTAL)
                <<","<<l->pos(VERTICAL)<<")";
        }
    }
    vpsc::Variables vs;
    vpsc::Constraints cs;
    CompoundConstraints* ccs=dim==HORIZONTAL?ccsx:ccsy;
    double stress;
    setupVarsAndConstraints(n,ccs,vs,cs);
    if(topologyRoutes) {
        FILE_LOG(logDEBUG1) << "applying topology preserving layout...\n";
		vpsc::Rectangle::setXBorder(0);
		vpsc::Rectangle::setYBorder(0);
        if(dim==cola::HORIZONTAL) {
            vpsc::Rectangle::setXBorder(0);
        }
        topology::TopologyConstraints t(dim,*topologyNodes,*topologyRoutes,
                vs,cs);
        bool interrupted;
        int loopBreaker=10;
        do {
            SparseMap HMap(n);
            computeForces(dim,HMap,g);
            t.gradientProjection(g,HMap,des);
            interrupted=t.solve();
            loopBreaker--;
        } while(interrupted&&loopBreaker>0);
		vpsc::Rectangle::setXBorder(0);
		vpsc::Rectangle::setYBorder(0);
        for(topology::Nodes::iterator i=topologyNodes->begin();
                i!=topologyNodes->end();++i) {
            topology::Node* v=*i;
            coords[v->id]=v->rect->getCentreD(dim);
        }
        stress=computeStress();
    } else {
        SparseMap HMap(n);
        computeForces(dim,HMap,g);
        SparseMatrix H(HMap);
        valarray<double> oldCoords=coords;
        applyDescentVector(g,oldCoords,coords,oldStress,computeStepSize(H,g,g));
        project(vs,cs,des,coords);
        valarray<double> d(n);
        d=oldCoords-coords;
        double stepsize=computeStepSize(H,g,d);
        stepsize=max(0.,min(stepsize,1.));
        //printf(" dim=%d beta: ",dim);
        stress = applyDescentVector(d,oldCoords,coords,oldStress,stepsize);
        move();
    }
    updateCompoundConstraints(ccs);
    if(unsatisfiable.size()==2) {
        checkUnsatisfiable(cs,unsatisfiable[dim]);
    }
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::applyForcesAndConstraints... done, stress="<<stress;
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
    return stress;
}
/**
 * Attempts to set coords=oldCoords-stepsize*d.  If this does not reduce
 * the stress from oldStress then stepsize is halved.  This is repeated
 * until stepsize falls below a threshhold.
 * @param d is a descent vector (a movement vector intended to reduce the
 * stress)
 * @param oldCoords are the previous position vector
 * @param coords will hold the new position after applying d
 * @param stepsize is a scalar multiple of the d to apply
 */
double ConstrainedFDLayout::applyDescentVector(
        valarray<double> const &d,
        valarray<double> const &oldCoords,
        valarray<double> &coords,
        const double oldStress,
        double stepsize
        ) {
    assert(d.size()==oldCoords.size());
    assert(d.size()==coords.size());
    while(fabs(stepsize)>0.00000000001) {
        coords=oldCoords-stepsize*d;
        double stress=computeStress();
        //printf(" applyDV: oldstress=%f, stress=%f, stepsize=%f\n", oldStress,stress,stepsize);
        //if(oldStress>=stress) {
            return stress;
        //}
        coords=oldCoords;
        stepsize*=0.5;
    }
    return computeStress();
}
/**
 * When considering a given pair of nodes we, in some circumstances, do not want to
 * compute forces between them.
 * Specifically, if nodes are further apart than their desired separation (l>d) and:
 *  - they are not immediately connected (g>1); or
 *  - they are immediately connected but we have topology routes (in which case
 *    the attractive force will be computed over the path length rather than
 *    the euclidean separation).
 * @param l actual separation between the pair of nodes
 * @param d desired separation between them
 * @param g graph path length between them
 * @return true if forces should NOT be computed
 */
bool ConstrainedFDLayout::noForces(double l, double d, unsigned g) const {
    if(l>d) {
        // we don't want long range attractive forces between 
        // not immediately connected nodes
        if(g>1) return true;
        // if we compute forces over topological edge paths then they replace
        // short range repulsive forces between adjacent nodes
        if(topologyRoutes && g==1) return true;
    }
    return false;
}
        
void ConstrainedFDLayout::computeForces(
        const Dim dim,
        SparseMap &H,
        valarray<double> &g) {
    if(n==1) return;
    g=0;
    // for each node:
    for(unsigned u=0;u<n;u++) {
        // Stress model
        double Huu=0;
        for(unsigned v=0;v<n;v++) {
            if(u==v) continue;
            unsigned p = G[u][v];
            // no forces between disconnected parts of the graph
            if(p==numeric_limits<unsigned>::max()) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            if(noForces(l,d,p)) continue;
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
double ConstrainedFDLayout::computeStress() const {
    FILE_LOG(logDEBUG)<<"ConstrainedFDLayout::computeStress()";
    double stress=0;
    for(unsigned u=0;u<n-1;u++) {
        for(unsigned v=u+1;v<n;v++) {
            unsigned p=G[u][v];
            // no forces between disconnected parts of the graph
            if(p==numeric_limits<unsigned>::max()) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            if(noForces(l,d,p)) continue;
            double d2=d*d;
            double rl=d-l;
            double s=rl*rl/d2;
            stress+=s;
            FILE_LOG(logDEBUG2)<<"s("<<u<<","<<v<<")="<<s;
        }
    }
    if(preIteration) {
        if ((*preIteration)()) {
            for(vector<Lock>::iterator l=preIteration->locks.begin();
                    l!=preIteration->locks.end();l++) {
                double dx=l->pos(HORIZONTAL)-X[l->getID()], dy=l->pos(VERTICAL)-Y[l->getID()];
                double s=10000*(dx*dx+dy*dy);
                stress+=s;
                FILE_LOG(logDEBUG2)<<"d("<<l->getID()<<")="<<s;
            }
        }
    }
    if(topologyRoutes) {
        double s=topology::compute_stress(*topologyRoutes);
        FILE_LOG(logDEBUG2)<<"s(topology)="<<s;
        stress+=s;
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

