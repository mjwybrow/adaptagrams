#include "commondefs.h"
#include <vector>
#include <cmath>
#include <limits>
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
	const vpsc::Rectangles & rs,
	const std::vector< Edge > & es, 
        const double idealLength,
        const std::valarray<double> * eweights,
		TestConvergence& done,
		PreIteration* preIteration)
    : n(rs.size()),
      X(valarray<double>(n)),
      Y(valarray<double>(n)),
      done(done),
      preIteration(preIteration),
      ccsx(NULL), ccsy(NULL),
      topologyNodes(NULL),
      topologyRoutes(NULL),
      rungekutta(true)
{
    //FILELog::ReportingLevel() = logDEBUG1;
    FILELog::ReportingLevel() = logERROR;
    boundingBoxes.resize(n);
    copy(rs.begin(),rs.end(),boundingBoxes.begin());
    done.reset();
	unsigned i=0;
	for(vpsc::Rectangles::const_iterator ri=rs.begin();ri!=rs.end();++ri,++i) {
        X[i]=(*ri)->getCentreX();
        Y[i]=(*ri)->getCentreY();
        FILE_LOG(logDEBUG) << *ri;
    }
    D=new double*[n];
    G=new unsigned short*[n];
    for(unsigned i=0;i<n;i++) {
        D[i]=new double[n];
        G[i]=new unsigned short[n];
    }
    computePathLengths(es,idealLength,eweights);
}
/**
 * Sets up the D and G matrices.  D is the required euclidean distances
 * between pairs of nodes based on the shortest paths between them (using
 * idealLength*eweights[edge] as the edge length, if eweights array 
 * is provided otherwise just idealLength).  G is a matrix of unsigned ints
 * such that G[u][v]=
 *   0 if there are no forces required between u and v 
 *     (for example, if u and v are in unconnected components)
 *   1 if attractive forces are required between u and v
 *     (i.e. if u and v are immediately connected by an edge and there is
 *      no topology route between u and v (for which an attractive force
 *      is computed elsewhere))
 *   2 if no attractive force is required between u and v but there is
 *     a connected path between them.
 */
void ConstrainedFDLayout::computePathLengths(
        const vector<Edge>& es,
        const double idealLength,
        const std::valarray<double>* eweights) 
{
    shortest_paths::johnsons(n,D,es,eweights);
    //dumpSquareMatrix<double>(n,D);
    for(unsigned i=0;i<n;i++) {
        for(unsigned j=0;j<n;j++) {
            if(i==j) continue;
            double& d=D[i][j];
            unsigned short& p=G[i][j];
            p=2;
            if(d==DBL_MAX) {
                // i and j are in disconnected subgraphs
                p=0;
            } else if(!eweights) {
                D[i][j]*=idealLength;
            }
        }
    }
    for(vector<Edge>::const_iterator e=es.begin();e!=es.end();++e) {
        unsigned u=e->first, v=e->second; 
        G[u][v]=G[v][u]=1;
    }
    // we don't need to compute attractive forces between nodes connected
    // by an edge if there is a topologyRoute between them (since the
    // p-stress force will be used instead)
    if(topologyRoutes) {
        for(vector<topology::Edge*>::iterator i=topologyRoutes->begin();
                i!=topologyRoutes->end();++i) {
            topology::Edge* e=*i;
            if(!e->cycle()) {
                unsigned u=e->firstSegment->start->node->id,
                         v=e->lastSegment->end->node->id;
                G[u][v]=G[v][u]=2;
            }
        }
    }
    //dumpSquareMatrix<short>(n,G);
}

typedef valarray<double> Position;
void getPosition(Position& X, Position& Y, Position& pos) {
    unsigned n=X.size();
    assert(Y.size()==n);
    assert(pos.size()==2*n);
    for(unsigned i=0;i<n;++i) {
        pos[i]=X[i];
        pos[i+n]=Y[i];
    }
}
/**
 * moves all rectangles to the desired position while respecting
 * constraints.
 * @param pos target positions of both axes
 */
void ConstrainedFDLayout::setPosition(Position& pos) {
    assert(Y.size()==X.size());
    assert(pos.size()==2*X.size());
    moveTo(HORIZONTAL,pos);
    moveTo(VERTICAL,pos);
}
void ConstrainedFDLayout::computeDescentVectorOnBothAxes(
        const bool xAxis, const bool yAxis,
        double stress, Position& x0, Position& x1) {
    setPosition(x0);
    if(xAxis) {
        applyForcesAndConstraints(HORIZONTAL,stress);
    }
    if(yAxis) {
        applyForcesAndConstraints(VERTICAL,stress);
    }
    getPosition(X,Y,x1);
}

void ConstrainedFDLayout::run(const bool xAxis, const bool yAxis) {
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::run...";
    if(n==0) return;
    double stress=DBL_MAX;
    do {
        if(preIteration) {
            if(!(*preIteration)()) {
                break;
            }
            printf("preIteration->changed=%d\n",preIteration->changed);
            if(preIteration->changed) {
                stress=DBL_MAX;
            }
            if(preIteration->resizes.size()>0) {
                FILE_LOG(logDEBUG) << " Resize event!";
                handleResizes(preIteration->resizes);
            }
        }
        unsigned N=2*n;
        Position x0(N),x1(N);
        getPosition(X,Y,x0);
        if(rungekutta) {
            Position a(N),b(N),c(N),d(N),ia(N),ib(N);
            computeDescentVectorOnBothAxes(xAxis,yAxis,stress,x0,a);
            ia=x0+(a-x0)/2.0;
            computeDescentVectorOnBothAxes(xAxis,yAxis,stress,ia,b);
            ib=x0+(b-x0)/2.0;
            computeDescentVectorOnBothAxes(xAxis,yAxis,stress,ib,c);
            computeDescentVectorOnBothAxes(xAxis,yAxis,stress,c,d);
            x1=a+2.0*b+2.0*c+d;
            x1/=6.0;
        } else {
            computeDescentVectorOnBothAxes(xAxis,yAxis,stress,x0,x1);
        }
        setPosition(x1);
        stress=computeStress();
        FILE_LOG(logDEBUG) << "stress="<<stress;
    } while(!done(stress,X,Y));
    for(unsigned i=0;i<n;i++) {
        vpsc::Rectangle *r=boundingBoxes[i];
	FILE_LOG(logDEBUG) << *r;
    }
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::run done.";
}
void ConstrainedFDLayout::runOnce(const bool xAxis, const bool yAxis) {
    if(n==0) return;
    double stress=DBL_MAX;
    unsigned N=2*n;
    Position x0(N),x1(N);
    getPosition(X,Y,x0);
	if(rungekutta) {
		Position a(N),b(N),c(N),d(N),ia(N),ib(N);
        computeDescentVectorOnBothAxes(xAxis,yAxis,stress,x0,a);
        ia=x0+(a-x0)/2.0;
        computeDescentVectorOnBothAxes(xAxis,yAxis,stress,ia,b);
        ib=x0+(b-x0)/2.0;
        computeDescentVectorOnBothAxes(xAxis,yAxis,stress,ib,c);
        computeDescentVectorOnBothAxes(xAxis,yAxis,stress,c,d);
        x1=a+2.0*b+2.0*c+d;
        x1/=6.0;
	} else {
        computeDescentVectorOnBothAxes(xAxis,yAxis,stress,x0,x1);
    }
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
void project(vpsc::Variables& vs, vpsc::Constraints& cs, valarray<double>& coords) {
    unsigned n=coords.size();
    vpsc::IncSolver s(vs,cs);
    s.solve();
    for(unsigned i=0;i<n;++i) {
        coords[i]=vs[i]->finalPosition;
    }
}
void setVariableDesiredPositions(vpsc::Variables& vs, vpsc::Constraints& cs, const topology::DesiredPositions& des, valarray<double>& coords) {
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
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::handleResizes()...";
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
    vpsc::Variables xvs, yvs;
    vpsc::Constraints xcs, ycs;
    setupVarsAndConstraints(n,ccsx,xvs,xcs);
    setupVarsAndConstraints(n,ccsy,yvs,ycs);
    topology::applyResizes(*topologyNodes, *topologyRoutes, resizes,
            xvs, xcs, yvs, ycs);
    for_each(xvs.begin(), xvs.end(), delete_object());
    for_each(yvs.begin(), yvs.end(), delete_object());
    for_each(xcs.begin(), xcs.end(), delete_object());
    for_each(ycs.begin(), ycs.end(), delete_object());
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::handleResizes()... done.";
}
/**
 * move positions of nodes in specified axis while respecting constraints
 * @param dim axis
 * @param target array of desired positions (for both axes)
 */
void ConstrainedFDLayout::moveTo(const Dim dim, Position& target) {
    assert(target.size()==2*n);
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::moveTo(): dim="<<dim;
    valarray<double> &coords = (dim==HORIZONTAL)?X:Y;
    vpsc::Variables vs;
    vpsc::Constraints cs;
    CompoundConstraints* ccs=dim==HORIZONTAL?ccsx:ccsy;
    setupVarsAndConstraints(n,ccs,vs,cs);
    topology::DesiredPositions des;
    if(preIteration) {
        for(vector<Lock>::iterator l=preIteration->locks.begin();
                l!=preIteration->locks.end();l++) {
            des.push_back(make_pair(l->getID(),l->pos(dim)));
            FILE_LOG(logDEBUG1)<<"desi: v["<<l->getID()<<"]=("<<l->pos(HORIZONTAL)
                <<","<<l->pos(VERTICAL)<<")";
        }
    }
    for(unsigned i=0, j=(dim==HORIZONTAL?0:n);i<n;++i,++j) {
		vpsc::Variable* v=vs[i];
        v->desiredPosition = target[j];
    }
    setVariableDesiredPositions(vs,cs,des,coords);
    if(topologyRoutes) {
        topology::setNodeVariables(*topologyNodes,vs);
        topology::TopologyConstraints t(dim,*topologyNodes,*topologyRoutes,vs,cs);
        bool interrupted;
        int loopBreaker=100;
        do {
            interrupted=t.solve();
            loopBreaker--;
        } while(interrupted&&loopBreaker>0);
        for(topology::Nodes::iterator i=topologyNodes->begin();
                i!=topologyNodes->end();++i) {
            topology::Node* v=*i;
            coords[v->id]=v->rect->getCentreD(dim);
        }
    } else {
        project(vs,cs,coords);
        moveBoundingBoxes();
    }
    updateCompoundConstraints(ccs);
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
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
        FILE_LOG(logDEBUG1) << "applying topology preserving layout...";
		vpsc::Rectangle::setXBorder(0);
		vpsc::Rectangle::setYBorder(0);
        if(dim==cola::HORIZONTAL) {
            vpsc::Rectangle::setXBorder(0);
        }
        topology::setNodeVariables(*topologyNodes,vs);
        topology::TopologyConstraints t(dim,*topologyNodes,*topologyRoutes,vs,cs);
        bool interrupted;
        int loopBreaker=100;
        SparseMap HMap(n);
        computeForces(dim,HMap,g);
        valarray<double> oldCoords=coords;
        t.computeForces(g,HMap);
        cola::SparseMatrix H(HMap);
        applyDescentVector(g,oldCoords,coords,oldStress,
                computeStepSize(H,g,g));
        setVariableDesiredPositions(vs,cs,des,coords);
        do {
            interrupted=t.solve();
            unsigned vptr=0;
            for(topology::Nodes::iterator i=topologyNodes->begin();
                    i!=topologyNodes->end();++i,++vptr) {
                topology::Node* v=*i;
                coords[v->id]=v->rect->getCentreD(dim);
            }
            for(;vptr<coords.size();vptr++) {
                double d = vs[vptr]->finalPosition;
                coords[vptr]=d;
                boundingBoxes[vptr]->moveCentreD(dim,d);
            }
            loopBreaker--;
        } while(interrupted&&loopBreaker>0);
		vpsc::Rectangle::setXBorder(0);
		vpsc::Rectangle::setYBorder(0);
        stress=computeStress();
    } else {
        SparseMap HMap(n);
        computeForces(dim,HMap,g);
        SparseMatrix H(HMap);
        valarray<double> oldCoords=coords;
        applyDescentVector(g,oldCoords,coords,oldStress,computeStepSize(H,g,g));
        setVariableDesiredPositions(vs,cs,des,coords);
        project(vs,cs,coords);
        valarray<double> d(n);
        d=oldCoords-coords;
        double stepsize=computeStepSize(H,g,d);
        stepsize=max(0.,min(stepsize,1.));
        //printf(" dim=%d beta: ",dim);
        stress = applyDescentVector(d,oldCoords,coords,oldStress,stepsize);
        moveBoundingBoxes();
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
            unsigned short p = G[u][v];
            // no forces between disconnected parts of the graph
            if(p==0) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            if(l>d && p>1) continue; // attractive forces not required
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
            unsigned short p=G[u][v];
            // no forces between disconnected parts of the graph
            if(p==0) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            if(l>d && p>1) continue; // no attractive forces required
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
        double s=topology::computeStress(*topologyRoutes);
        FILE_LOG(logDEBUG2)<<"s(topology)="<<s;
        stress+=s;
    }
    return stress;
}
void ConstrainedFDLayout::moveBoundingBoxes() {
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

