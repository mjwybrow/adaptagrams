/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
*/

#include <vector>
#include <cmath>
#include <limits>

#include "libvpsc/solve_VPSC.h"
#include "libvpsc/variable.h"
#include "libvpsc/constraint.h"
#include "libvpsc/rectangle.h"
#include "libtopology/topology_graph.h"
#include "libtopology/topology_constraints.h"
#include "commondefs.h"
#include "cola.h"
#include "shortest_paths.h"
#include "straightener.h"
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
    ASSERT(x.size()==y.size());
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
    const double* eLengths,
    TestConvergence& done,
    PreIteration* preIteration)
: n(rs.size()),
  X(valarray<double>(n)),
  Y(valarray<double>(n)),
  done(done),
  preIteration(preIteration),
  ccs(NULL),
  topologyNodes(NULL),
  topologyRoutes(NULL),
  rungekutta(true),
  desiredPositions(NULL)
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

    if(eLengths == NULL) {
        computePathLengths(es,idealLength,NULL);
    } else {
        valarray<double> eLengthsArray(eLengths,es.size());
        computePathLengths(es,idealLength,&eLengthsArray);
    }    
}

void dijkstra(const unsigned s, const unsigned n, double* d, 
        const vector<Edge>& es, const double* eLengths)
{
    const valarray<double> eLengthsArray(eLengths, es.size());
    shortest_paths::dijkstra(s,n,d,es,&eLengthsArray);
}

/**
 * Sets up the D and G matrices.  D is the required euclidean distances
 * between pairs of nodes based on the shortest paths between them (using
 * idealLength*eLengths[edge] as the edge length, if eLengths array 
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
        const std::valarray<double>* eLengths) 
{
    shortest_paths::johnsons(n,D,es,eLengths);
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
            } else if(!eLengths) {
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
    ASSERT(Y.size()==n);
    ASSERT(pos.size()==2*n);
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
    ASSERT(Y.size()==X.size());
    ASSERT(pos.size()==2*X.size());
    moveTo(vpsc::HORIZONTAL,pos);
    moveTo(vpsc::VERTICAL,pos);
}
/**
 * Layout is performed by minimizing the P-stress goal function iteratively.
 * At each iteration taking a step in the steepest-descent direction.
 * x0 is the current position, x1 is the x0 - descentvector.
 */
void ConstrainedFDLayout::computeDescentVectorOnBothAxes(
        const bool xAxis, const bool yAxis,
        double stress, Position& x0, Position& x1) {
    setPosition(x0);
    if(xAxis) {
        applyForcesAndConstraints(vpsc::HORIZONTAL,stress);
    }
    if(yAxis) {
        applyForcesAndConstraints(vpsc::VERTICAL,stress);
    }
    getPosition(X,Y,x1);
}

/**
 * run() implements the main layout loop, taking descent steps until
 * stress is no-longer significantly reduced.
 * done is a callback used to check stress but also to report updated 
 * positions.
 */
void ConstrainedFDLayout::run(const bool xAxis, const bool yAxis) 
#ifdef ASSERTION_EXCEPTIONS
        throw(InvalidVariableIndexException, vpsc::CriticalFailure)
#else
        throw(InvalidVariableIndexException)
#endif
{
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
/**
 * Same as run, but only applies one iteration.  This may be useful
 * where it's too hard to implement a call-back (e.g. in java apps)
 */
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


void ConstrainedFDLayout::setTopology(std::vector<topology::Node*>* tnodes, 
        std::vector<topology::Edge*>* routes)
{
    // Setting nodes with NULL routes will result in strange behaviour,
    // so catch this case.  An empy list for topologyRoutes is okay though.
    ASSERT(!topologyNodes || topologyRoutes);

    topologyNodes=tnodes;
    topologyRoutes=routes;
}

void ConstrainedFDLayout::setAvoidNodeOverlaps(void)
{
    unsigned nodesTotal = boundingBoxes.size();
    topology::Nodes *topologyNodes = new topology::Nodes(nodesTotal);
    for (unsigned id = 0; id < nodesTotal; ++id)
    {
        (*topologyNodes)[id] = new topology::Node(id, boundingBoxes[id]);
    }

    // Empty edge set will just result in nonoverlap constraints for nodes.
    topology::Edges *topologyEdges = new topology::Edges(0);

    setTopology(topologyNodes, topologyEdges);
}


static void setupVarsAndConstraints(unsigned n, const CompoundConstraints* ccs,
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& cs) 
{
    vs.resize(n);
    for (unsigned i = 0; i < n; ++i)
    {
        vs[i] = new vpsc::Variable(i);
    }

    if(ccs) 
    {
        for (CompoundConstraints::const_iterator c = ccs->begin();
                c != ccs->end(); ++c) 
        {
            (*c)->generateVariables(dim, vs);
        }
        for (CompoundConstraints::const_iterator c = ccs->begin();
                c != ccs->end(); ++c) 
        {
            (*c)->generateSeparationConstraints(dim, vs, cs);
        }
    }
}


void updateCompoundConstraints(const vpsc::Dim dim,
        const CompoundConstraints* ccs) 
{
    if(ccs) 
    {
        for (CompoundConstraints::const_iterator c = ccs->begin();
                c != ccs->end(); ++c) 
        {
            (*c)->updatePosition(dim);
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
    ASSERT(vs.size()>=n);
    for(unsigned i=0;i<n;++i) {
        vpsc::Variable* v=vs[i];
        v->desiredPosition = coords[i];
        v->weight=1;
    }
    for(topology::DesiredPositions::const_iterator d=des.begin();
            d!=des.end();++d) {
        ASSERT(d->first<vs.size());
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
        ASSERT(topologyRoutes==NULL);
        return;
    }
    ASSERT(topologyRoutes!=NULL);
    // all shapes to be resized are wrapped in a ResizeInfo and
    // placed in a lookup table, resizes, indexed by id
    topology::ResizeMap resizes;
    for(Resizes::const_iterator r=resizeList.begin();r!=resizeList.end();++r) {
        topology::ResizeInfo ri((*topologyNodes)[r->getID()],r->getTarget());
        resizes.insert(make_pair(r->getID(),ri));
    }
    vpsc::Variables xvs, yvs;
    vpsc::Constraints xcs, ycs;
    setupVarsAndConstraints(n, ccs, vpsc::HORIZONTAL, xvs, xcs);
    setupVarsAndConstraints(n, ccs, vpsc::VERTICAL, yvs, ycs);
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
void ConstrainedFDLayout::moveTo(const vpsc::Dim dim, Position& target) {
    ASSERT(target.size()==2*n);
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::moveTo(): dim="<<dim;
    valarray<double> &coords = (dim==vpsc::HORIZONTAL)?X:Y;
    vpsc::Variables vs;
    vpsc::Constraints cs;
    setupVarsAndConstraints(n, ccs, dim, vs, cs);
    topology::DesiredPositions des;
    if(preIteration) {
        for(vector<Lock>::iterator l=preIteration->locks.begin();
                l!=preIteration->locks.end();l++) {
            des.push_back(make_pair(l->getID(),l->pos(dim)));
            FILE_LOG(logDEBUG1)<<"desi: v["<<l->getID()<<"]=("<<l->pos(vpsc::HORIZONTAL)
                <<","<<l->pos(vpsc::VERTICAL)<<")";
        }
    }
    for(unsigned i=0, j=(dim==vpsc::HORIZONTAL?0:n);i<n;++i,++j) {
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
    updateCompoundConstraints(dim, ccs);
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
}
/**
 * The following computes an unconstrained solution then uses Projection to
 * make this solution feasible with respect to constraints by moving things as
 * little as possible.  If "meta-constraints" such as avoidOverlaps or edge
 * straightening are required then dummy variables will be generated.
 */
double ConstrainedFDLayout::applyForcesAndConstraints(const vpsc::Dim dim, const double oldStress) {
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::applyForcesAndConstraints(): dim="<<dim;
    valarray<double> g(n);
    valarray<double> &coords = (dim==vpsc::HORIZONTAL)?X:Y;
    topology::DesiredPositions des;
    if(preIteration) {
        for(vector<Lock>::iterator l=preIteration->locks.begin();
                l!=preIteration->locks.end();l++) {
            des.push_back(make_pair(l->getID(),l->pos(dim)));
            FILE_LOG(logDEBUG1)<<"desi: v["<<l->getID()<<"]=("<<l->pos(vpsc::HORIZONTAL)
                <<","<<l->pos(vpsc::VERTICAL)<<")";
        }
    }
    vpsc::Variables vs;
    vpsc::Constraints cs;
    double stress;
    setupVarsAndConstraints(n, ccs, dim, vs, cs);
    if(topologyRoutes) {
        FILE_LOG(logDEBUG1) << "applying topology preserving layout...";
        vpsc::Rectangle::setXBorder(0);
        vpsc::Rectangle::setYBorder(0);
        if(dim==vpsc::HORIZONTAL) {
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
    updateCompoundConstraints(dim, ccs);
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
    ASSERT(d.size()==oldCoords.size());
    ASSERT(d.size()==coords.size());
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
 * Computes:
 *  - the matrix of second derivatives (the Hessian) H, used in 
 *    calculating stepsize; and
 *  - the vector g, the negative gradient (steepest-descent) direction.
 */
void ConstrainedFDLayout::computeForces(
        const vpsc::Dim dim,
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
            double dx=dim==vpsc::HORIZONTAL?rx:ry;
            double dy=dim==vpsc::HORIZONTAL?ry:rx;
            g[u]+=dx*(l-d)/(d2*l);
            Huu-=H(u,v)=(d*dy*dy/(l*l*l)-1)/d2;
        }
        H(u,u)=Huu;
    }
    if(desiredPositions) {
        for(DesiredPositions::const_iterator p=desiredPositions->begin();
            p!=desiredPositions->end();++p) {
            unsigned i = p->id;
            double d=(dim==vpsc::HORIZONTAL)
                ?p->x-X[i]:p->y-Y[i];
            d*=p->weight;
            g[i]-=d;
            H(i,i)+=p->weight;
        }
    }
}
/**
 * Returns the optimal step-size in the direction d, given gradient g and 
 * hessian H.
 */
double ConstrainedFDLayout::computeStepSize(
        SparseMatrix const &H, 
        valarray<double> const &g, 
        valarray<double> const &d) const
{
    ASSERT(g.size()==d.size());
    ASSERT(g.size()==H.rowSize());
    // stepsize = g'd / (d' H d)
    double numerator = dotProd(g,d);
    valarray<double> Hd(d.size());
    H.rightMultiply(d,Hd);
    double denominator = dotProd(d,Hd);
    //ASSERT(numerator>=0);
    //ASSERT(denominator>=0);
    if(denominator==0) return 0;
    return numerator/denominator;
}
/**
 * Just computes the cost (Stress) at the current X,Y position
 * used to test termination.
 * This method will call preIteration if one is set.
 */
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
                double dx=l->pos(vpsc::HORIZONTAL)-X[l->getID()], dy=l->pos(vpsc::VERTICAL)-Y[l->getID()];
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
    if(desiredPositions) {
        for(DesiredPositions::const_iterator p = desiredPositions->begin();
            p!=desiredPositions->end();++p) {
            double dx = X[p->id] - p->x, dy = Y[p->id] - p->y;
            stress+=0.5*p->weight*(dx*dx+dy*dy);
        }
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

