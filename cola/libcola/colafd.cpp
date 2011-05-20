/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2010  Monash University
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
 * Author(s): Tim Dwyer
 *            Michael Wybrow
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
#include "cc_clustercontainmentconstraints.h"
#include "cc_nonoverlapconstraints.h"

#ifdef MAKEFEASIBLE_DEBUG
  #include "output_svg.h"
#endif

using vpsc::XDIM;
using vpsc::YDIM;

namespace cola {

template <class T>
void delete_vector(vector<T*> &v) {
    for_each(v.begin(),v.end(),delete_object());
    v.clear();
}
Resizes PreIteration::__resizesNotUsed;
Locks PreIteration::__locksNotUsed;
inline double dotProd(valarray<double> x, valarray<double> y) {
    COLA_ASSERT(x.size()==y.size());
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

ConstrainedFDLayout::ConstrainedFDLayout(const vpsc::Rectangles& rs,
        const std::vector< Edge >& es, const double idealLength,
        const bool preventOverlaps, const double* eLengths, 
        TestConvergence& done, PreIteration* preIteration) 
    : n(rs.size()),
      X(valarray<double>(n)),
      Y(valarray<double>(n)),
      done(done),
      preIteration(preIteration),
      rungekutta(true),
      desiredPositions(NULL),
      clusterHierarchy(NULL),
      rectClusterBuffer(0),
      m_generateNonOverlapConstraints(preventOverlaps)
{
    topologyNodes.clear(),
    topologyRoutes.clear(),
    //FILELog::ReportingLevel() = logDEBUG1;
    FILELog::ReportingLevel() = logERROR;
    boundingBoxes = rs;
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
    if(!topologyRoutes.empty()) {
        for(vector<topology::Edge*>::iterator i=topologyRoutes.begin();
                i!=topologyRoutes.end();++i) {
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
    COLA_ASSERT(Y.size()==n);
    COLA_ASSERT(pos.size()==2*n);
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
    COLA_ASSERT(Y.size()==X.size());
    COLA_ASSERT(pos.size()==2*X.size());
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
{
    if (extraConstraints.empty())
    {
        // This generates constraints for non-overlap inside and outside
        // of clusters.  To assign correct variable indexes it requires
        // that vs[] contains elements equal to the number of rectangles.
        vpsc::Variables vs[2];
        vs[0].resize(n);
        vs[1].resize(n);
        generateNonOverlapAndClusterCompoundConstraints(vs);
    }
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::run...";
    double stress=DBL_MAX;
    do {
        if(preIteration) {
            if(!(*preIteration)()) {
                break;
            }
            //printf("preIteration->changed=%d\n",preIteration->changed);
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


// Used for sorting the CompoundConstraints from lowest priority to highest. 
static bool cmpCompoundConstraintPriority(const cola::CompoundConstraint *lhs, 
        const cola::CompoundConstraint *rhs)
{
    return lhs->priority() < rhs->priority();
}


void ConstrainedFDLayout::recGenerateClusterVariablesAndConstraints(
        vpsc::Variables (&vars)[2], unsigned int& priority, 
        cola::NonOverlapConstraints *noc, Cluster *cluster, 
        cola::CompoundConstraints& idleConstraints)
{
    for (std::vector<Cluster*>::iterator curr = cluster->clusters.begin();
            curr != cluster->clusters.end(); ++curr)
    {
        // For each of the child clusters, recursively call this function.
        recGenerateClusterVariablesAndConstraints(vars, priority, 
                noc, *curr, idleConstraints);
    }

    if ( (noc == NULL) && (dynamic_cast<RootCluster *> (cluster) == NULL) )
    {
        double freeWeight = 0.00000000001;
        // Then create left and right variables for the boundary of this 
        // cluster.
        vpsc::Variable *variable = NULL;
        cluster->clusterVarId = vars[XDIM].size();
        COLA_ASSERT(vars[XDIM].size() == vars[YDIM].size());
        // Left:
        variable = new vpsc::Variable(vars[XDIM].size(), 
                cluster->bounds.getMinX(), freeWeight);
        vars[XDIM].push_back(variable);
        // Right:
        variable = new vpsc::Variable(vars[XDIM].size(), 
                cluster->bounds.getMaxX(), freeWeight);
        vars[XDIM].push_back(variable);
        // Bottom::
        variable = new vpsc::Variable(vars[YDIM].size(), 
                cluster->bounds.getMinY(), freeWeight);
        vars[YDIM].push_back(variable);
        // Top:
        variable = new vpsc::Variable(vars[YDIM].size(), 
                cluster->bounds.getMaxY(), freeWeight);
        vars[YDIM].push_back(variable);

        RectangularCluster *rc = dynamic_cast<RectangularCluster *> (cluster);
        if (rc)
        {
            rc->generateFixedRectangleConstraints(idleConstraints, 
                    boundingBoxes, vars);
        }

        priority--;
        cola::ClusterContainmentConstraints *ccc = 
                new cola::ClusterContainmentConstraints(cluster, priority,
                        boundingBoxes);
        idleConstraints.push_back(ccc);
    }

    if (noc)
    {
        // Enforce non-overlap between all the shapes and clusters at this 
        // level.
        printf("Cluster #%d non-overlap constraints - nodes %d clusters %d\n",
                (int) cluster->clusterVarId, (int) cluster->nodes.size(), 
                (int) cluster->clusters.size());
        unsigned int group = cluster->clusterVarId;
        for (std::vector<unsigned>::iterator curr = cluster->nodes.begin();
                curr != cluster->nodes.end(); ++curr)
        {
            unsigned id = *curr;
            noc->addShape(id, boundingBoxes[id]->width() / 2,
                    boundingBoxes[id]->height() / 2, group);
        }
        for (std::vector<Cluster*>::iterator curr = cluster->clusters.begin();
                curr != cluster->clusters.end(); ++curr)
        {
            Cluster *cluster = *curr;
            RectangularCluster *rectCluster = 
                    dynamic_cast<RectangularCluster *> (cluster);
            if (rectCluster && rectCluster->clusterIsFromFixedRectangle())
            {
                // Treat it like a shape for non-overlap.
                unsigned id = rectCluster->rectangleIndex();
                noc->addShape(id, boundingBoxes[id]->width() / 2,
                        boundingBoxes[id]->height() / 2, group);
            }
            else
            {
                // Normal cluster.
                noc->addCluster(cluster, group);
            }
        }
    }
}

void ConstrainedFDLayout::generateNonOverlapAndClusterCompoundConstraints(
        vpsc::Variables (&vs)[2])
{
    if (clusterHierarchy && !clusterHierarchy->flat())
    {
        // Add remaining nodes that aren't contained within any clusters
        // as children of the root cluster.
        for (unsigned int i = 0; i < boundingBoxes.size(); ++i)
        {
            if (!clusterHierarchy->containsShape(i))
            {
                clusterHierarchy->nodes.push_back(i);
            }
        }

        // Add non-overlap and containment constraints for all clusters
        // and nodes.
        unsigned int priority = PRIORITY_NONOVERLAP;
        clusterHierarchy->computeBoundingRect(boundingBoxes);
        
        // Generate the containment constraints
        recGenerateClusterVariablesAndConstraints(vs, priority, 
                NULL, clusterHierarchy, extraConstraints);
        
        // Generate non-overlap constraints between all clusters and 
        // all contained nodes.
        if (m_generateNonOverlapConstraints)
        {
            priority--;
            cola::NonOverlapConstraints *noc = 
                    new cola::NonOverlapConstraints(priority);
            recGenerateClusterVariablesAndConstraints(vs, priority, 
                    noc, clusterHierarchy, extraConstraints);
            extraConstraints.push_back(noc);
        }
    }
    else if (m_generateNonOverlapConstraints)
    {
        // Add standard non-overlap constraints between each pair of
        // nodes.
        cola::NonOverlapConstraints *noc = 
                new cola::NonOverlapConstraints();
        for (unsigned int i = 0; i < boundingBoxes.size(); ++i)
        {
            noc->addShape(i, boundingBoxes[i]->width() / 2,
                    boundingBoxes[i]->height() / 2);
        }
        extraConstraints.push_back(noc);
    }
}

void ConstrainedFDLayout::makeFeasible(void)
{
    vpsc::Variables vs[2];
    vpsc::Constraints valid[2];

    vpsc::Rectangle::setXBorder(1);
    vpsc::Rectangle::setYBorder(1);
    
    // Populate all the variables for shapes.
    for (unsigned int dim = 0; dim < 2; ++dim)
    {
        vs[dim] = vpsc::Variables(boundingBoxes.size());
        for (unsigned int i = 0; i < vs[dim].size(); ++i)
        {
            double pos = (dim == 0) ? 
                    boundingBoxes[i]->getCentreX() : 
                    boundingBoxes[i]->getCentreY();
            vs[dim][i] = new vpsc::Variable(i, pos, 1);
        }
    }

    vector<double> priorPos(boundingBoxes.size());

    // Clear extra constraints for cluster containment and non-overlap.
    // We keep a separate list of these since we keep them around for 
    // later solving.
    for_each(extraConstraints.begin(), extraConstraints.end(), delete_object());
    extraConstraints.clear();

    generateNonOverlapAndClusterCompoundConstraints(vs);

    // Make a copy of the compound constraints and sort them by priority.
    cola::CompoundConstraints idleConstraints = ccs;
    // Append extraConstraints to idleConstraints.
    idleConstraints.insert(idleConstraints.end(), 
            extraConstraints.begin(), extraConstraints.end());

    std::sort(idleConstraints.begin(), idleConstraints.end(), 
            cmpCompoundConstraintPriority);

    // Initialise extra variables for compound constraints.
    for (unsigned int dim = 0; dim < 2; ++dim)
    {
        generateVariables(idleConstraints, (vpsc::Dim) dim, vs[dim]);
    }

#ifdef MAKEFEASIBLE_DEBUG
    char filename[200];
    int iteration = 0;
    vector<string> labels(boundingBoxes.size());
    for(unsigned i=0;i<boundingBoxes.size();++i)
    {
        stringstream ss;
        ss << i;
        labels[i]=ss.str();
    }
#endif

    // Main makeFeasible loop.
    while (!idleConstraints.empty())
    {
        // idleConstraints is sorted lowest to highest priority, so the 
        // highest priority constraint will be at the back of the vector.
        cola::CompoundConstraint *cc = idleConstraints.back();
        idleConstraints.pop_back();

#ifdef MAKEFEASIBLE_DEBUG
        // Debugging SVG time slice output.
        std::vector<cola::Edge> es;
        for (unsigned int i = 0; i < boundingBoxes.size(); ++i)
        {
            boundingBoxes[i]->moveCentreX(vs[0][i]->finalPosition);
            boundingBoxes[i]->moveCentreY(vs[1][i]->finalPosition);
        }
        iteration++;
        sprintf(filename, "out/file-%05d.pdf", iteration);

        OutputFile of(boundingBoxes,es,clusterHierarchy,filename,true,false);
        of.setLabels(labels);
        of.generate();
#endif

        cc->markAllSubConstraintsAsInactive();
        bool subConstraintSatisfiable = true;
        
        if (cc->shouldCombineSubConstraints())
        {
            // We are processing a combined set of satisfiable constraints,
            // such as for containment within cluster boundary variables, so
            // we just add all the required constraints and solve in both
            // the X and Y dimension once to set the cluster boundaries to 
            // meaningful values.
            while (cc->subConstraintsRemaining())
            {
                cola::SubConstraintAlternatives alternatives = 
                        cc->getCurrSubConstraintAlternatives(vs);
                // There should be no alternatives, just guaranteed
                // satisfiable constraints.
                COLA_ASSERT(alternatives.size() == 1);
                vpsc::Dim& dim = alternatives.front().dim;
                vpsc::Constraint& constraint = alternatives.front().constraint;
                valid[dim].push_back(new vpsc::Constraint(constraint));
                cc->markCurrSubConstraintAsActive(subConstraintSatisfiable);
            }
            for (size_t dim = 0; dim < 2; ++dim)
            {
                vpsc::IncSolver vpscInstance(vs[dim], valid[dim]);
                vpscInstance.satisfy();
            }
            continue;
        }

        while (cc->subConstraintsRemaining())
        {
            cola::SubConstraintAlternatives alternatives = 
                    cc->getCurrSubConstraintAlternatives(vs);
            alternatives.sort();

            while (!alternatives.empty())
            {
                // Reset subConstraintSatisfiable for new solve.
                subConstraintSatisfiable = true;

                vpsc::Dim& dim = alternatives.front().dim;
                vpsc::Constraint& constraint = alternatives.front().constraint;
            
                // Store current values for variables.
                for (unsigned int i = 0; i < priorPos.size(); ++i)
                {
                    priorPos[i] = vs[dim][i]->finalPosition;
                }

                // Some solving...
                try 
                {
                    // Add the constraint from this alternative to the 
                    // valid constraint set.
                    valid[dim].push_back(new vpsc::Constraint(constraint));

                    //fprintf(stderr, ".%d %3d - ", dim, valid[dim].size());
                    // Solve with this constraint set.
                    vpsc::IncSolver vpscInstance(vs[dim], valid[dim]);
                    vpscInstance.satisfy();
                }
                catch (char *str) 
                {
                    subConstraintSatisfiable = false;
                    
                    std::cerr << "++++ IN ERROR BLOCK" << std::endl;
                    std::cerr << str << std::endl;
                    for (vpsc::Rectangles::iterator r = boundingBoxes.begin(); 
                            r != boundingBoxes.end(); ++r) 
                    {
                        std::cerr << **r <<std::endl;
                    }
                }
                for (size_t i = 0; i < valid[dim].size(); ++i) 
                {
                    if (valid[dim][i]->unsatisfiable) 
                    {
                        // It might have made one of the earlier added 
                        // constraints unsatisfiable, so we mark that one 
                        // as okay since we will be reverting the most 
                        // recent one.
                        valid[dim][i]->unsatisfiable = false;
                        
                        subConstraintSatisfiable = false;
                    }
                }

                if (!subConstraintSatisfiable)
                {
                    //fprintf(stderr, "*");

                    // Restore previous values for variables.
                    for (unsigned int i = 0; i < priorPos.size(); ++i)
                    {
                        vs[dim][i]->finalPosition = priorPos[i];
                    }
                    
                    // Delete the newly added (and unsatisfiable) 
                    // constraint from the valid constraint set.
                    delete valid[dim].back();
                    valid[dim].pop_back();
                }
                else
                {
                    break;
                }
                // Move on to the next alternative.
                alternatives.pop_front();
            }
#ifdef MAKEFEASIBLE_DEBUG
            if (true || idleConstraints.size() == 0)
            {
                // Debugging SVG time slice output, but don't show this for
                // constraints that promised satisfiable.
                std::vector<cola::Edge> es;
                for (unsigned int i = 0; i < boundingBoxes.size(); ++i)
                {
                    boundingBoxes[i]->moveCentreX(vs[0][i]->finalPosition);
                    boundingBoxes[i]->moveCentreY(vs[1][i]->finalPosition);
                }
                iteration++;
                sprintf(filename, "out/file-%05d.pdf", iteration);

                OutputFile of(boundingBoxes,es,clusterHierarchy,filename,
                        true,false);
                of.setLabels(labels);
                of.generate();
            }
#endif
            cc->markCurrSubConstraintAsActive(subConstraintSatisfiable);
        }
    }

    // Write positions from solver variables back to Rectangles.
    for (unsigned int i = 0; i < boundingBoxes.size(); ++i)
    {
        boundingBoxes[i]->moveCentreX(vs[0][i]->finalPosition);
        boundingBoxes[i]->moveCentreY(vs[1][i]->finalPosition);
    }

    vpsc::Rectangle::setXBorder(0);
    vpsc::Rectangle::setYBorder(0);

    if (m_generateNonOverlapConstraints)
    {
        // Set up topologyNodes:
        unsigned nodesTotal = boundingBoxes.size();
        topologyNodes = topology::Nodes(nodesTotal);
        for (unsigned id = 0; id < nodesTotal; ++id)
        {
            topologyNodes[id] = new topology::Node(id, boundingBoxes[id]);
        }
    }

    // Cleanup.
    for (unsigned int dim = 0; dim < 2; ++dim)
    {
        for_each(valid[dim].begin(), valid[dim].end(), delete_object());
        for_each(vs[dim].begin(), vs[dim].end(), delete_object());
    }
   
    if (clusterHierarchy)
    {
        // create cluster boundaries
        unsigned clusterCount=0;
        for (vector<cola::Cluster*>::iterator i = 
                clusterHierarchy->clusters.begin();
                i != clusterHierarchy->clusters.end(); ++i, ++clusterCount)
        {
            (*i)->computeBoundary(boundingBoxes);
            cola::ConvexCluster* c=dynamic_cast<cola::ConvexCluster*>(*i);
            if(c!=NULL) {
                double idealCircumference=2.0*sqrt(M_PI*c->area(boundingBoxes));
                vector<topology::EdgePoint*> eps;
                for(unsigned j=0;j<c->hullRIDs.size();++j) {
                    const unsigned id = c->nodes[c->hullRIDs[j]];
                    const unsigned char corner = c->hullCorners[j];
                    COLA_ASSERT(id < topologyNodes.size());
                    //cout << "addToPath(vs[" << id << "],";
                    topology::Node *node= topologyNodes[id];
                    topology::EdgePoint::RectIntersect ri;
                    switch(corner) {
                        case 0: 
                            ri=topology::EdgePoint::BR; 
                            //cout << "EdgePoint::BR);" << endl;
                            break;
                        case 1: 
                            ri=topology::EdgePoint::TR; 
                            //cout << "EdgePoint::TR);" << endl;
                            break;
                        case 2: 
                            ri=topology::EdgePoint::TL;
                            //cout << "EdgePoint::TL);" << endl;
                            break;
                        default:
                            COLA_ASSERT(corner==3);
                            ri=topology::EdgePoint::BL; 
                            //cout << "EdgePoint::BL);" << endl;
                            break;
                    }
                    eps.push_back(new topology::EdgePoint(node,ri));
                }
                eps.push_back(eps[0]);
                //cout << "addToPath(vs[" << eps[0]->node->id << "],(EdgePoint::RectIntersect)"<<eps[0]->rectIntersect<<");" << endl;
                topology::Edge* e = new topology::Edge(clusterCount,idealCircumference, eps);
                topologyRoutes.push_back(e);
            }
        }
    }
    
    // Update the X and Y vectors with the new shape positions.
    for (unsigned int i = 0; i < boundingBoxes.size(); ++i)
    {
        X[i] = boundingBoxes[i]->getCentreX();
        Y[i] = boundingBoxes[i]->getCentreY();
    }
}

ConstrainedFDLayout::~ConstrainedFDLayout()
{
    for (unsigned i = 0; i < n; ++i)
    {
        delete [] G[i];
        delete [] D[i];
    }
    delete [] G;
    delete [] D;
}

void ConstrainedFDLayout::freeAssociatedObjects(void)
{
    // Free Rectangles
    for_each(boundingBoxes.begin(), boundingBoxes.end(), delete_object());
    boundingBoxes.clear();
    
    // Free compound constraints
    std::list<CompoundConstraint *> freeList(ccs.begin(), ccs.end());
    freeList.sort();
    freeList.unique();
    if (freeList.size() != ccs.size())
    {
        // The compound constraint list had repeated elements.
        fprintf(stderr, "Warning: CompoundConstraints vector contained %d "
                "duplicates.\n", (int) (ccs.size() - freeList.size()));
    }
    ccs.clear();
    for_each(freeList.begin(), freeList.end(), delete_object());

    if (clusterHierarchy)
    {
        delete clusterHierarchy;
        clusterHierarchy = NULL;
    }
}

void ConstrainedFDLayout::setTopology(std::vector<topology::Node*> *tnodes, 
        std::vector<topology::Edge*> *routes)
{
    topologyNodes = *tnodes;
    topologyRoutes = *routes;
}


void ConstrainedFDLayout::getTopology(std::vector<topology::Node*> *tnodes, 
        std::vector<topology::Edge*> *routes)
{
    *tnodes = topologyNodes;
    *routes = topologyRoutes;
}


static void setupVarsAndConstraints(unsigned n, const CompoundConstraints& ccs,
        const vpsc::Dim dim, std::vector<vpsc::Rectangle*>& boundingBoxes,
        RootCluster *clusterHierarchy,
        vpsc::Variables& vs, vpsc::Constraints& cs, 
        valarray<double> &coords) 
{
    vs.resize(n);
    for (unsigned i = 0; i < n; ++i)
    {
        vs[i] = new vpsc::Variable(i, coords[i]);
    }

    if (clusterHierarchy && !clusterHierarchy->clusters.empty())
    {
        // Create variables for clusters
        clusterHierarchy->computeBoundingRect(boundingBoxes);
        clusterHierarchy->createVars(dim,boundingBoxes,vs);
    }

    for (CompoundConstraints::const_iterator c = ccs.begin();
            c != ccs.end(); ++c) 
    {
        (*c)->generateVariables(dim, vs);
    }
    for (CompoundConstraints::const_iterator c = ccs.begin();
            c != ccs.end(); ++c) 
    {
        (*c)->generateSeparationConstraints(dim, vs, cs, boundingBoxes);
    }
}


static void setupExtraConstraints(const CompoundConstraints& ccs,
        const vpsc::Dim dim, vpsc::Variables& vs, vpsc::Constraints& cs,
        std::vector<vpsc::Rectangle*>& boundingBoxes)
{
    for (CompoundConstraints::const_iterator c = ccs.begin();
            c != ccs.end(); ++c) 
    {
        (*c)->generateVariables(dim, vs);
    }
    for (CompoundConstraints::const_iterator c = ccs.begin();
            c != ccs.end(); ++c) 
    {
        (*c)->generateSeparationConstraints(dim, vs, cs, boundingBoxes);
    }
}

void updateCompoundConstraints(const vpsc::Dim dim,
        const CompoundConstraints& ccs) 
{
    for (CompoundConstraints::const_iterator c = ccs.begin();
            c != ccs.end(); ++c) 
    {
        (*c)->updatePosition(dim);
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
void setVariableDesiredPositions(vpsc::Variables& vs, vpsc::Constraints& cs,
        const topology::DesiredPositions& des, valarray<double>& coords)
{
    COLA_UNUSED(cs);

    unsigned n=coords.size();
    COLA_ASSERT(vs.size()>=n);
    for(unsigned i=0;i<n;++i) {
        vpsc::Variable* v=vs[i];
        v->desiredPosition = coords[i];
        v->weight=1;
    }
    for(topology::DesiredPositions::const_iterator d=des.begin();
            d!=des.end();++d) {
        COLA_ASSERT(d->first<vs.size());
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
    if(topologyNodes.empty()) {
        COLA_ASSERT(topologyRoutes.empty());
        return;
    }
    // all shapes to be resized are wrapped in a ResizeInfo and
    // placed in a lookup table, resizes, indexed by id
    topology::ResizeMap resizes;
    for(Resizes::const_iterator r=resizeList.begin();r!=resizeList.end();++r) {
        topology::ResizeInfo ri(topologyNodes[r->getID()],r->getTarget());
        resizes.insert(make_pair(r->getID(),ri));
    }
    vpsc::Variables xvs, yvs;
    vpsc::Constraints xcs, ycs;
    setupVarsAndConstraints(n, ccs, vpsc::HORIZONTAL, boundingBoxes,
            clusterHierarchy, xvs, xcs, X);
    setupVarsAndConstraints(n, ccs, vpsc::VERTICAL, boundingBoxes,
            clusterHierarchy, yvs, ycs, Y);
    topology::applyResizes(topologyNodes, topologyRoutes, clusterHierarchy,
            resizes, xvs, xcs, yvs, ycs);
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
    COLA_ASSERT(target.size()==2*n);
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::moveTo(): dim="<<dim;
    valarray<double> &coords = (dim==vpsc::HORIZONTAL)?X:Y;
    vpsc::Variables vs;
    vpsc::Constraints cs;
    setupVarsAndConstraints(n, ccs, dim, boundingBoxes,
            clusterHierarchy, vs, cs, coords);
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
    if (!topologyNodes.empty() && !topologyRoutes.empty())
    {
        topology::setNodeVariables(topologyNodes,vs);
        topology::TopologyConstraints t(dim, topologyNodes, topologyRoutes,
                clusterHierarchy, vs, cs);
        bool interrupted;
        int loopBreaker=100;
        do {
            interrupted=t.solve();
            loopBreaker--;
        } while(interrupted&&loopBreaker>0);
        for(topology::Nodes::iterator i=topologyNodes.begin();
                i!=topologyNodes.end();++i) {
            topology::Node* v=*i;
            coords[v->id]=v->rect->getCentreD(dim);
        }
    } else {
        // Add non-overlap constraints, but not variables again.
        setupExtraConstraints(extraConstraints, dim, vs, cs, boundingBoxes);
        // Projection.
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
    setupVarsAndConstraints(n, ccs, dim, boundingBoxes,
            clusterHierarchy, vs, cs, coords);

    if (!topologyNodes.empty() && !topologyRoutes.empty())
    {
        FILE_LOG(logDEBUG1) << "applying topology preserving layout...";
        vpsc::Rectangle::setXBorder(0);
        vpsc::Rectangle::setYBorder(0);
        if(dim==vpsc::HORIZONTAL) {
            vpsc::Rectangle::setXBorder(0);
        }
        topology::setNodeVariables(topologyNodes,vs);
        topology::TopologyConstraints t(dim, topologyNodes, topologyRoutes,
                clusterHierarchy, vs, cs);
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
            for(topology::Nodes::iterator i=topologyNodes.begin();
                    i!=topologyNodes.end();++i,++vptr) {
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
        // Add non-overlap constraints, but not variables again.
        setupExtraConstraints(extraConstraints, dim, vs, cs, boundingBoxes);
        // Projection.
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
        )
{
    COLA_UNUSED(oldStress);

    COLA_ASSERT(d.size()==oldCoords.size());
    COLA_ASSERT(d.size()==coords.size());
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
    COLA_ASSERT(g.size()==d.size());
    COLA_ASSERT(g.size()==H.rowSize());
    // stepsize = g'd / (d' H d)
    double numerator = dotProd(g,d);
    valarray<double> Hd(d.size());
    H.rightMultiply(d,Hd);
    double denominator = dotProd(d,Hd);
    //COLA_ASSERT(numerator>=0);
    //COLA_ASSERT(denominator>=0);
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
    for(unsigned u=0;(u + 1)<n;u++) {
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
    if(!topologyRoutes.empty()) {
        double s=topology::computeStress(topologyRoutes);
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


static const double LIMIT = 100000000;

static void reduceRange(double& val)
{
    val = std::min(val, LIMIT);
    val = std::max(val, -LIMIT);
}

void ConstrainedFDLayout::outputInstanceToSVG(std::string instanceName)
{
    std::string filename;
    if (!instanceName.empty())
    {
        filename = instanceName;
    }
    else
    {
        filename = "libcola-debug";
    }
    filename += ".svg";
    FILE *fp = fopen(filename.c_str(), "w");

    if (fp == NULL)
    {
        return;
    }


    double minX = LIMIT;
    double minY = LIMIT;
    double maxX = -LIMIT;
    double maxY = -LIMIT;

    // Find the bounds of the diagram.
    for (size_t i = 0; i < boundingBoxes.size(); ++i)
    {
        double rMinX = boundingBoxes[i]->getMinX();
        double rMaxX = boundingBoxes[i]->getMaxX();
        double rMinY = boundingBoxes[i]->getMinY();
        double rMaxY = boundingBoxes[i]->getMaxY();
   
        reduceRange(rMinX);
        reduceRange(rMaxX);
        reduceRange(rMinY);
        reduceRange(rMaxY);
        
        if (rMinX > -LIMIT)
        {
            minX = std::min(minX, rMinX);
        }
        if (rMaxX < LIMIT)
        {
            maxX = std::max(maxX,rMaxX);
        }
        if (rMinY > -LIMIT)
        {
            minY = std::min(minY, rMinY);
        }
        if (rMaxY < LIMIT)
        {
            maxY = std::max(maxY, rMaxY);
        }
    }
 
    minX -= 50;
    minY -= 50;
    maxX += 50;
    maxY += 50;

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<svg xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\" width=\"100%%\" height=\"100%%\" viewBox=\"%g %g %g %g\">\n", minX, minY, maxX - minX, maxY - minY);

    // Output source code to generate this ConstrainedFDLayout instance.
    // XXX: This is not yet complete... it does not output everything.
    fprintf(fp, "<!-- Source code to generate this instance:\n");
    fprintf(fp, "#include <vector>\n");
    fprintf(fp, "#include \"libcola/cola.h\"\n");
    fprintf(fp, "using namespace cola;\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    CompoundConstraints ccs;\n");
    fprintf(fp, "    std::vector<Edge> es;\n");
    fprintf(fp, "    double defaultEdgeLength=40;\n");
    fprintf(fp, "    std::vector<vpsc::Rectangle*> rs;\n");
    fprintf(fp, "    vpsc::Rectangle *rect = NULL;\n\n");
    for (size_t i = 0; i < boundingBoxes.size(); ++i)
    {
        fprintf(fp, "    rect = new vpsc::Rectangle(%g, %g, %g, %g);\n",
               boundingBoxes[i]->getMinX(), boundingBoxes[i]->getMaxX(),
               boundingBoxes[i]->getMinY(), boundingBoxes[i]->getMaxY());
        fprintf(fp, "    rs.push_back(rect);\n\n");
    }

    for (cola::CompoundConstraints::iterator c = ccs.begin(); 
            c != ccs.end(); ++c)
    {
        (*c)->printCreationCode(fp);
    }

    fprintf(fp, "    ConstrainedFDLayout alg(rs, es, defaultEdgeLength, %s);\n",
            (m_generateNonOverlapConstraints) ? "true" : "false");
    if (clusterHierarchy)
    {
        clusterHierarchy->printCreationCode(fp);
    }
    fprintf(fp, "    alg.setConstraints(ccs);\n");
    fprintf(fp, "    alg.makeFeasible();\n");
    fprintf(fp, "    alg.run();\n");
    fprintf(fp, "};\n");
    fprintf(fp, "-->\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"Rects\">\n");
    for (size_t i = 0; i < boundingBoxes.size(); ++i)
    {
        double minX = boundingBoxes[i]->getMinX();
        double maxX = boundingBoxes[i]->getMaxX();
        double minY = boundingBoxes[i]->getMinY();
        double maxY = boundingBoxes[i]->getMaxY();
    
        fprintf(fp, "<rect id=\"rect-%u\" x=\"%g\" y=\"%g\" width=\"%g\" "
                "height=\"%g\" style=\"stroke-width: 1px; stroke: black; "
                "fill: blue; fill-opacity: 0.3;\" />\n",
                (unsigned) i, minX, minY, maxX - minX, maxY - minY);
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "</svg>\n");
    fclose(fp);
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

