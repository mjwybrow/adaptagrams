/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2015  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author(s):  Tim Dwyer
 *             Michael Wybrow
 *
*/

// cmath needs ::strcpy_s under MinGW so include cstring.
#include <cstring>

#include <vector>
#include <cmath>
#include <limits>

#include "libvpsc/solve_VPSC.h"
#include "libvpsc/variable.h"
#include "libvpsc/constraint.h"
#include "libvpsc/rectangle.h"
#include "libvpsc/exceptions.h"

#include "libcola/commondefs.h"
#include "libcola/cola.h"
#include "libcola/shortest_paths.h"
#include "libcola/straightener.h"
#include "libcola/cc_clustercontainmentconstraints.h"
#include "libcola/cc_nonoverlapconstraints.h"

#ifdef MAKEFEASIBLE_DEBUG
  #include "libcola/output_svg.h"
#endif

// Needs to come last since it will include windows.h on WIN32 and
// may mess up C++ std library include on GCC 4.4
#include "libcola/cola_log.h"

using namespace std;
using vpsc::Dim;
using vpsc::XDIM;
using vpsc::YDIM;
using vpsc::IncSolver;
using vpsc::Variable;
using vpsc::Variables;
using vpsc::Constraint;
using vpsc::Constraints;
using vpsc::Rectangle;
using vpsc::Rectangles;

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
        const EdgeLengths& eLengths,
        TestConvergence *doneTest, PreIteration* preIteration)
    : n(rs.size()),
      X(valarray<double>(n)),
      Y(valarray<double>(n)),
      done(doneTest),
      using_default_done(false),
      preIteration(preIteration),
      topologyAddon(new TopologyAddonInterface()),
      rungekutta(true),
      desiredPositions(nullptr),
      clusterHierarchy(nullptr),
      rectClusterBuffer(0),
      m_idealEdgeLength(idealLength),
      m_generateNonOverlapConstraints(false),
      m_useNeighbourStress(false),
      m_edge_lengths(eLengths.data(), eLengths.size()),
      m_nonoverlap_exemptions(new NonOverlapConstraintExemptions())
{
    minD = DBL_MAX;

    if (done == nullptr)
    {
        done = new TestConvergence();
        using_default_done = true;
    }

    computeNeighbours(es);

    //FILELog::ReportingLevel() = logDEBUG1;
    FILELog::ReportingLevel() = logERROR;
    boundingBoxes = rs;
    done->reset();
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

    computePathLengths(es,m_edge_lengths);
}

std::vector<double> ConstrainedFDLayout::readLinearD(void)
{
    std::vector<double> d;
    d.resize(n*n);
    for (unsigned i = 0; i < n; ++i) {
        for (unsigned j = 0; j < n; ++j) {
            d[n*i + j] = D[i][j];
        }
    }
    return d;
}

std::vector<unsigned> ConstrainedFDLayout::readLinearG(void)
{
    std::vector<unsigned> g;
    g.resize(n*n);
    for (unsigned i = 0; i < n; ++i) {
        for (unsigned j = 0; j < n; ++j) {
            g[n*i + j] = G[i][j];
        }
    }
    return g;
}

void ConstrainedFDLayout::computeNeighbours(vector<Edge> es) {
    for (unsigned i = 0; i < n; ++i) {
        neighbours.push_back(vector<unsigned>(n));
    }
    for (vector<Edge>::iterator it = es.begin(); it!=es.end(); ++it) {
        Edge e = *it;
        unsigned s = e.first, t = e.second;
        neighbours[s][t] = 1;
        neighbours[t][s] = 1;
    }
}

void dijkstra(const unsigned s, const unsigned n, double* d,
        const vector<Edge>& es, const std::valarray<double> & eLengths)
{
    shortest_paths::dijkstra(s,n,d,es,eLengths);
}

void ConstrainedFDLayout::setConstraints(const cola::CompoundConstraints& ccs)
{
    this->ccs = ccs;
}

void ConstrainedFDLayout::setAvoidNodeOverlaps(bool avoidOverlaps,
        std::vector<std::vector<unsigned> > listOfNodeGroups)
{
    m_generateNonOverlapConstraints = avoidOverlaps;
    m_nonoverlap_exemptions->addExemptGroupOfNodes(listOfNodeGroups);
}

void ConstrainedFDLayout::setUseNeighbourStress(bool useNeighbourStress)
{
    m_useNeighbourStress = useNeighbourStress;
}

void ConstrainedFDLayout::setDesiredPositions(DesiredPositions *desiredPositions)
{
    this->desiredPositions = desiredPositions;
}


/*
 * Sets up the D and G matrices.  D is the required euclidean distances
 * between pairs of nodes based on the shortest paths between them (using
 * m_idealEdgeLength*eLengths[edge] as the edge length, if eLengths array
 * is provided otherwise just m_idealEdgeLength).  G is a matrix of
 * unsigned ints such that G[u][v]=
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
        const vector<Edge>& es, std::valarray<double> eLengths)
{
    // Correct zero or negative entries in eLengths array.
    for (size_t i = 0; i < eLengths.size(); ++i)
    {
        if (eLengths[i] <= 0)
        {
            fprintf(stderr, "Warning: ignoring non-positive length at index %d "
                    "in ideal edge length array.\n", (int) i);
            eLengths[i] = 1;
        }
    }

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
            } else {
                d*=m_idealEdgeLength;
            }

            if ((d > 0) && (d < minD)) {
                minD = d;
            }
        }
    }
    if (minD == DBL_MAX) minD = 1;

    for(vector<Edge>::const_iterator e=es.begin();e!=es.end();++e) {
        unsigned u=e->first, v=e->second;
        G[u][v]=G[v][u]=1;
    }
    topologyAddon->computePathLengths(G);
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
/*
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
/*
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

/*
 * run() implements the main layout loop, taking descent steps until
 * stress is no-longer significantly reduced.
 * done is a callback used to check stress but also to report updated
 * positions.
 */
void ConstrainedFDLayout::run(const bool xAxis, const bool yAxis)
{
    // This generates constraints for non-overlap inside and outside
    // of clusters.  To assign correct variable indexes it requires
    // that vs[] contains elements equal to the number of rectangles.
    vpsc::Variables vs[2];
    vs[0].resize(n);
    vs[1].resize(n);
    generateNonOverlapAndClusterCompoundConstraints(vs);

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
    } while(!(*done)(stress,X,Y));
    for(unsigned i=0;i<n;i++) {
        vpsc::Rectangle *r=boundingBoxes[i];
    FILE_LOG(logDEBUG) << *r;
    }
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::run done.";

    // Clear extra constraints.
    for_each(extraConstraints.begin(), extraConstraints.end(), delete_object());
    extraConstraints.clear();

    // Free extra variables used for cluster containment.
    for (size_t dim = 0; dim < 2; ++dim)
    {
        for (size_t i = n; i < vs[dim].size(); ++i)
        {
            delete vs[dim][i];
        }
    }
}

/*
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

    if ( (noc == nullptr) && (dynamic_cast<RootCluster *> (cluster) == nullptr) )
    {
        double freeWeight = 0.00000000001;
        // Then create left and right variables for the boundary of this
        // cluster.
        vpsc::Variable *variable = nullptr;
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
        //printf("Cluster #%d non-overlap constraints - nodes %d clusters %d\n",
        //        (int) cluster->clusterVarId, (int) cluster->nodes.size(),
        //        (int) cluster->clusters.size());
        unsigned int group = cluster->clusterVarId;
        // The set of clusters to put non-overlap constraints between is the
        // child clusters of this cluster.  We will also add any overlapping
        // clusters (due to multiple inheritence) to this set.
        std::set<Cluster *> expandedClusterSet(cluster->clusters.begin(),
                cluster->clusters.end());
        for (std::set<unsigned>::iterator curr = cluster->nodes.begin();
                curr != cluster->nodes.end(); ++curr)
        {
            unsigned id = *curr;

            if (cluster->m_overlap_replacement_map.count(id) > 0)
            {
                // This shape is child of another cluster also, so replace
                // this node with the other cluster for the purpose of
                // non-overlap with other children of the current cluster.
                expandedClusterSet.insert(
                        cluster->m_overlap_replacement_map[id]);
            }
            // Normal case: Add shape for generation of non-overlap
            // constraints.
            noc->addShape(id, boundingBoxes[id]->width() / 2,
                    boundingBoxes[id]->height() / 2, group);
        }
        for (std::set<Cluster*>::iterator curr = expandedClusterSet.begin();
                curr != expandedClusterSet.end(); ++curr)
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

        // For the set of shapes that have been replaced due to multiple
        // inheritance, still generate overlap constraints between them.
        // (The group uses the ID of the right side variable of the cluster
        // so it is not the same group as the cluster itself.)
        for (std::set<unsigned>::iterator curr =
                cluster->m_nodes_replaced_with_clusters.begin();
                curr != cluster->m_nodes_replaced_with_clusters.end(); ++curr)
        {
            unsigned id = *curr;
            noc->addShape(id, boundingBoxes[id]->width() / 2,
                    boundingBoxes[id]->height() / 2, group + 1);
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
        std::vector<unsigned> nodesInClusterCounts(boundingBoxes.size(), 0);
        clusterHierarchy->countContainedNodes(nodesInClusterCounts);

        for (unsigned int i = 0; i < nodesInClusterCounts.size(); ++i)
        {
            unsigned count = nodesInClusterCounts[i];
            if (!clusterHierarchy->allowsMultipleParents() &&
                    count > 1)
            {
                fprintf(stderr, "Warning: node %u is contained in %d "
                        "clusters.\n", i, count);
            }

            if (count == 0)
            {
                // Not present in hierarchy, so add to root cluster.
                clusterHierarchy->nodes.insert(i);
            }
        }

        // Add non-overlap and containment constraints for all clusters
        // and nodes.
        unsigned int priority = PRIORITY_NONOVERLAP;
        clusterHierarchy->computeBoundingRect(boundingBoxes);

        // Generate the containment constraints
        recGenerateClusterVariablesAndConstraints(vs, priority,
                nullptr, clusterHierarchy, extraConstraints);

        // Compute overlapping clusters.
        clusterHierarchy->calculateClusterPathsToEachNode(boundingBoxes.size());

        // Generate non-overlap constraints between all clusters and
        // all contained nodes.
        if (m_generateNonOverlapConstraints)
        {
            priority--;
            cola::NonOverlapConstraints *noc =
                    new cola::NonOverlapConstraints(m_nonoverlap_exemptions,
                            priority);
            noc->setClusterClusterExemptions(
                    clusterHierarchy->m_cluster_cluster_overlap_exceptions);
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
                new cola::NonOverlapConstraints(m_nonoverlap_exemptions);
        for (unsigned int i = 0; i < boundingBoxes.size(); ++i)
        {
            noc->addShape(i, boundingBoxes[i]->width() / 2,
                    boundingBoxes[i]->height() / 2);
        }
        extraConstraints.push_back(noc);
    }
}

void ConstrainedFDLayout::makeFeasible(double xBorder, double yBorder)
{
    vpsc::Variables vs[2];
    vpsc::Constraints valid[2];

    vpsc::Rectangle::setXBorder(xBorder);
    vpsc::Rectangle::setYBorder(yBorder);

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
    int iteration = 0;
    vector<string> labels(boundingBoxes.size());
    for(unsigned i=0;i<boundingBoxes.size();++i)
    {
        stringstream ss;
        ss << i;
        labels[i]=ss.str();
    }
#endif

    // We can keep adding new constraints to the existing VPSC instances so
    // long as everything is satisfiable.  Only when it's not do we discard
    // the existing VPSC instance for that dimension and create a new one.
    vpsc::IncSolver *solver[2] = { nullptr };

    // Main makeFeasible loop.
    while (!idleConstraints.empty())
    {
        // idleConstraints is sorted lowest to highest priority, so the
        // highest priority constraint will be at the back of the vector.
        cola::CompoundConstraint *cc = idleConstraints.back();
        idleConstraints.pop_back();

#ifdef MAKEFEASIBLE_DEBUG
        {
            // Debugging SVG time slice output.
            std::vector<cola::Edge> es;
            for (unsigned int i = 0; i < boundingBoxes.size(); ++i)
            {
                boundingBoxes[i]->moveCentreX(vs[0][i]->finalPosition);
                boundingBoxes[i]->moveCentreY(vs[1][i]->finalPosition);
            }
            iteration++;
            std::sstream filename;
            filename << "out/file-" << std::setfill('0') << std::setw(5) << iteration << ".pdf";

            OutputFile of(boundingBoxes,es,clusterHierarchy,filename.str().c_str(),true,false);
            of.setLabels(labels);
            of.generate();
        }
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
                vpsc::Constraint *newConstraint =
                        new vpsc::Constraint(constraint);
                valid[dim].push_back(newConstraint);
                if (solver[dim])
                {
                    // If we have an existing valid solver instance, add the
                    // constraint to that.
                    solver[dim]->addConstraint(newConstraint);
                }
                cc->markCurrSubConstraintAsActive(subConstraintSatisfiable);
            }
            // Satisfy the constraints in each dimension.
            for (size_t dim = 0; dim < 2; ++dim)
            {
                if (solver[dim] == nullptr)
                {
                    // Create a new VPSC solver if necessary.
                    solver[dim] = new vpsc::IncSolver(vs[dim], valid[dim]);
                }
                solver[dim]->satisfy();
            }
            continue;
        }

        while (cc->subConstraintsRemaining())
        {
            cola::SubConstraintAlternatives alternatives =
                    cc->getCurrSubConstraintAlternatives(vs);
            alternatives.sort();

            if (alternatives.empty())
            {
                continue;
            }

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
                    vpsc::Constraint *newConstraint =
                            new vpsc::Constraint(constraint);
                    valid[dim].push_back(newConstraint);

                    //fprintf(stderr, ".%d %3d - ", dim, valid[dim].size());

                    // Try to satisfy this set of constraints..
                    if (solver[dim] == nullptr)
                    {
                        // Create a new VPSC solver if necessary.
                        solver[dim] = new vpsc::IncSolver(vs[dim], valid[dim]);
                    }
                    else
                    {
                        // Or just add the constraint to the existing solver.
                        solver[dim]->addConstraint(newConstraint);
                    }
                    solver[dim]->satisfy();
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
                    // Since we had unsatisfiable constraints we must
                    // discard this solver instance.
                    delete solver[dim];
                    solver[dim] = nullptr;

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
                    // Satisfied, so don't need to consider other alternatives.
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
                std::sstream filename;
                filename << "out/file-" << std::setfill('0') << std::setw(5) << iteration << ".pdf";

                OutputFile of(boundingBoxes,es,clusterHierarchy,filename.str().c_str(),
                        true,false);
                of.setLabels(labels);
                of.generate();
            }
#endif
            cc->markCurrSubConstraintAsActive(subConstraintSatisfiable);
        }
    }

    // Delete the persistent VPSC solver instances.
    for (size_t dim = 0; dim < 2; ++dim)
    {
        if (solver[dim])
        {
            delete solver[dim];
            solver[dim] = nullptr;
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

    // Cleanup.
    for (unsigned int dim = 0; dim < 2; ++dim)
    {
        for_each(valid[dim].begin(), valid[dim].end(), delete_object());
        for_each(vs[dim].begin(), vs[dim].end(), delete_object());
    }

    topologyAddon->makeFeasible(m_generateNonOverlapConstraints,
            boundingBoxes, clusterHierarchy);

    // Update the X and Y vectors with the new shape positions.
    for (unsigned int i = 0; i < boundingBoxes.size(); ++i)
    {
        X[i] = boundingBoxes[i]->getCentreX();
        Y[i] = boundingBoxes[i]->getCentreY();
    }

    // Clear extra constraints for cluster containment and non-overlap.
    for_each(extraConstraints.begin(), extraConstraints.end(), delete_object());
    extraConstraints.clear();
}

ConstrainedFDLayout::~ConstrainedFDLayout()
{
    if (using_default_done)
    {
        delete done;
    }

    for (unsigned i = 0; i < n; ++i)
    {
        delete [] G[i];
        delete [] D[i];
    }
    delete [] G;
    delete [] D;
    delete topologyAddon;
    delete m_nonoverlap_exemptions;
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
        clusterHierarchy = nullptr;
    }

    topologyAddon->freeAssociatedObjects();
}

void ConstrainedFDLayout::setTopology(TopologyAddonInterface *newTopology)
{
    COLA_ASSERT(topologyAddon);
    delete topologyAddon;
    topologyAddon = newTopology->clone();
}

TopologyAddonInterface *ConstrainedFDLayout::getTopology(void)
{
    return topologyAddon->clone();
}


void setupVarsAndConstraints(unsigned n, const CompoundConstraints& ccs,
        const vpsc::Dim dim, vpsc::Rectangles& boundingBoxes,
        RootCluster *clusterHierarchy,
        vpsc::Variables& vs, vpsc::Constraints& cs,
        valarray<double> &coords)
{
    vs.resize(n);
    for (unsigned i = 0; i < n; ++i)
    {
        vs[i] = new vpsc::Variable(i, coords[i]);
    }

    if (clusterHierarchy && !clusterHierarchy->flat())
    {
        // Create variables for clusters
        clusterHierarchy->computeBoundingRect(boundingBoxes);
        clusterHierarchy->createVars(dim, boundingBoxes, vs);
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
        vpsc::Rectangles& boundingBoxes)
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
        const DesiredPositionsInDim& des, valarray<double>& coords)
{
    COLA_UNUSED(cs);

    unsigned n=coords.size();
    COLA_ASSERT(vs.size()>=n);
    for(unsigned i=0;i<n;++i) {
        vpsc::Variable* v=vs[i];
        v->desiredPosition = coords[i];
        v->weight=1;
    }
    for (DesiredPositionsInDim::const_iterator d=des.begin();
            d!=des.end(); ++d) {
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

void ConstrainedFDLayout::handleResizes(const Resizes& resizeList)
{
    topologyAddon->handleResizes(resizeList, n, X, Y, ccs, boundingBoxes,
            clusterHierarchy);
}

/*
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
    DesiredPositionsInDim des;
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
    if (topologyAddon->useTopologySolver())
    {
        topologyAddon->moveTo(dim, vs, cs, coords, clusterHierarchy);
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
/*
 * The following computes an unconstrained solution then uses Projection to
 * make this solution feasible with respect to constraints by moving things as
 * little as possible.  If "meta-constraints" such as avoidOverlaps or edge
 * straightening are required then dummy variables will be generated.
 */
double ConstrainedFDLayout::applyForcesAndConstraints(const vpsc::Dim dim, const double oldStress) {
    FILE_LOG(logDEBUG) << "ConstrainedFDLayout::applyForcesAndConstraints(): dim="<<dim;
    valarray<double> g(n);
    valarray<double> &coords = (dim==vpsc::HORIZONTAL)?X:Y;
    DesiredPositionsInDim des;
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

    if (topologyAddon->useTopologySolver())
    {
        stress = topologyAddon->applyForcesAndConstraints(this, dim, g, vs, cs,
                coords, des, oldStress);
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
    if (clusterHierarchy)
    {
        clusterHierarchy->computeVarRect(vs, dim);
        clusterHierarchy->computeBoundingRect(boundingBoxes);
    }

    for_each(vs.begin(),vs.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
    return stress;
}
/*
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


// Computes X and Y offsets for nodes that are at the same position.
std::vector<double> ConstrainedFDLayout::offsetDir(double minD)
{
    std::vector<double> u(2);
    double l = 0;
    for (size_t i = 0; i < 2; ++i)
    {
        double x = u[i] = random.getNextBetween(0.01, 1) - 0.5;
        l += x * x;
    }
    l = sqrt(l);

    for (size_t i = 0; i < 2; ++i)
    {
        u[i] *= (minD / l);
    }

    return u;
}


/*
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
            if (m_useNeighbourStress && neighbours[u][v]!=1) continue;

            // The following loop randomly displaces nodes that are at identical positions
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double sd2 = rx*rx+ry*ry;
            unsigned maxDisplaces = n;  // avoid infinite loop in the case of numerical issues, such as huge values

            while (maxDisplaces--)
            {
                if ((sd2) > 1e-3)
                {
                    break;
                }

                std::vector<double> rd = offsetDir(minD);
                X[v] += rd[0];
                Y[v] += rd[1];
                rx=X[u]-X[v], ry=Y[u]-Y[v];
                sd2 = rx*rx+ry*ry;
            }

            unsigned short p = G[u][v];
            // no forces between disconnected parts of the graph
            if(p==0) continue;
            double l=sqrt(sd2);
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
/*
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
/*
 * Just computes the cost (Stress) at the current X,Y position
 * used to test termination.
 * This method will call preIteration if one is set.
 */
double ConstrainedFDLayout::computeStress() const {
    FILE_LOG(logDEBUG)<<"ConstrainedFDLayout::computeStress()";
    double stress=0;
    for(unsigned u=0;(u + 1)<n;u++) {
        for(unsigned v=u+1;v<n;v++) {
            if (m_useNeighbourStress && neighbours[u][v]!=1) continue;
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
    stress += topologyAddon->computeStress();
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

    if (fp == nullptr)
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
    fprintf(fp, "<!-- Source code to generate this instance:\n");
    fprintf(fp, "#include <vector>\n");
    fprintf(fp, "#include <utility>\n");
    fprintf(fp, "#include \"libcola/cola.h\"\n");
    fprintf(fp, "using namespace cola;\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    CompoundConstraints ccs;\n");
    fprintf(fp, "    std::vector<Edge> es;\n");
    fprintf(fp, "    EdgeLengths eLengths;\n");
    fprintf(fp, "    double defaultEdgeLength=%g;\n", m_idealEdgeLength);
    fprintf(fp, "    std::vector<vpsc::Rectangle*> rs;\n");
    fprintf(fp, "    vpsc::Rectangle *rect = nullptr;\n\n");
    for (size_t i = 0; i < boundingBoxes.size(); ++i)
    {
        fprintf(fp, "    rect = new vpsc::Rectangle(%g, %g, %g, %g);\n",
               boundingBoxes[i]->getMinX(), boundingBoxes[i]->getMaxX(),
               boundingBoxes[i]->getMinY(), boundingBoxes[i]->getMaxY());
        fprintf(fp, "    rs.push_back(rect);\n\n");
    }

    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j =  i + 1; j < n; ++j)
        {
            if (G[i][j] == 1)
            {
                fprintf(fp, "    es.push_back(std::make_pair(%lu, %lu));\n", i, j);
            }
        }
    }
    fprintf(fp, "\n");

    if (m_edge_lengths.size() > 0)
    {
        fprintf(fp, "    eLengths.resize(%d);\n", (int) m_edge_lengths.size());
        for (size_t i = 0; i < m_edge_lengths.size(); ++i)
        {
            fprintf(fp, "    eLengths[%d] = %g;\n", (int) i, m_edge_lengths[i]);
        }
        fprintf(fp, "\n");
    }

    for (cola::CompoundConstraints::iterator c = ccs.begin();
            c != ccs.end(); ++c)
    {
        (*c)->printCreationCode(fp);
    }

    fprintf(fp, "    ConstrainedFDLayout alg(rs, es, defaultEdgeLength, eLengths);\n");
    if (clusterHierarchy)
    {
        clusterHierarchy->printCreationCode(fp);
        fprintf(fp, "    alg.setClusterHierarchy(cluster%llu);\n",
                (unsigned long long) clusterHierarchy);
    }
    fprintf(fp, "    alg.setConstraints(ccs);\n");
    fprintf(fp, "    alg.setAvoidNodeOverlaps(%s);\n",
            (m_generateNonOverlapConstraints) ? "true" : "false");
    fprintf(fp, "    alg.makeFeasible();\n");
    fprintf(fp, "    alg.run();\n");
    fprintf(fp, "    alg.freeAssociatedObjects();\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "};\n");
    fprintf(fp, "-->\n");

    if (clusterHierarchy)
    {
        clusterHierarchy->computeBoundingRect(boundingBoxes);
        fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"Clusters\">\n");
        clusterHierarchy->outputToSVG(fp);
        fprintf(fp, "</g>\n");
    }

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

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"Edges\">\n");
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j =  i + 1; j < n; ++j)
        {
            if (G[i][j] == 1)
            {
                fprintf(fp, "<path d=\"M %g %g L %g %g\" "
                        "style=\"stroke-width: 1px; stroke: black;\" />\n",
                        boundingBoxes[i]->getCentreX(),
                        boundingBoxes[i]->getCentreY(),
                        boundingBoxes[j]->getCentreX(),
                        boundingBoxes[j]->getCentreY());
            }
        }
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "</svg>\n");
    fclose(fp);
}

ProjectionResult projectOntoCCs(Dim dim, Rectangles &rs, CompoundConstraints ccs,
                                bool preventOverlaps, int accept, unsigned debugLevel)
{
    size_t n = rs.size();
    // Set up nonoverlap constraints if desired.
    NonOverlapConstraintExemptions *nocexemps = nullptr;
    NonOverlapConstraints *noc = nullptr;
    if (preventOverlaps) {
        nocexemps = new NonOverlapConstraintExemptions();
        noc = new NonOverlapConstraints(nocexemps);
        for (size_t i = 0; i < n; ++i) {
            noc->addShape(i, rs[i]->width()/2.0, rs[i]->height()/2.0);
        }
        ccs.push_back(noc);
    }
    // Set up vars and constraints.
    Variables vs;
    Constraints cs;
    vs.resize(n);
    for (size_t i = 0; i < n; ++i) {
        vs[i] = new Variable(i, rs[i]->getCentreD(dim));
    }
    for (CompoundConstraints::iterator it=ccs.begin(); it!=ccs.end(); ++it) {
        CompoundConstraint *cc = *it;
        cc->generateVariables(dim, vs);
        cc->generateSeparationConstraints(dim, vs, cs, rs);
    }
    // Solve, if possible.
    ProjectionResult result = solve(vs, cs, rs, debugLevel);
    // If good enough, accept positions.
    if (result.errorLevel <= accept) {
        for (size_t i = 0; i < n; ++i) {
            rs[i]->moveCentreD(dim, vs[i]->finalPosition);
        }
    }
    // Clean up
    for (Variables::iterator it=vs.begin(); it!=vs.end(); ++it) delete *it;
    for (Constraints::iterator it=cs.begin(); it!=cs.end(); ++it) delete *it;
    delete noc;
    delete nocexemps;
    // Return
    return result;
}

ProjectionResult solve(Variables &vs, Constraints &cs, Rectangles &rs, unsigned debugLevel)
{
    int result = 0;
    IncSolver solv(vs,cs);
    try {
        solv.solve();
    } catch (vpsc::UnsatisfiedConstraint uc) {
    }
    for (Constraints::iterator it=cs.begin(); it!=cs.end(); it++) {
        Constraint *c = *it;
        if (c->unsatisfiable) {
            CompoundConstraint *cc = (CompoundConstraint*)(c->creator);
            if (cc->toString() == "NonOverlapConstraints()") {
                result = 1;
            } else {
                result = 2;
                break;
            }
        }
    }
    std::string unsatinfo;
    if (debugLevel>0) {
        std::set<Variable*> varsInvolved;
        unsatinfo += "===================================================\n";
        unsatinfo += "UNSATISFIED CONSTRAINTS:\n";
        char buf [1000];
        for (Constraints::iterator it=cs.begin(); it!=cs.end(); it++) {
            Constraint *c = *it;
            if (c->unsatisfiable) {
                varsInvolved.insert(c->left);
                varsInvolved.insert(c->right);
                sprintf(buf, "v_%d + %f", c->left->id, c->gap);
                unsatinfo += buf;
                unsatinfo += c->equality ? " == " : " <= ";
                sprintf(buf, "v_%d\n", c->right->id);
                unsatinfo += buf;
                if ((unsigned) c->left->id < rs.size()) {
                    Rectangle *r = rs[c->left->id];
                    sprintf(buf, "    v_%d rect: [%f, %f] x [%f, %f]\n", c->left->id,
                            r->getMinX(), r->getMaxX(), r->getMinY(), r->getMaxY());
                    unsatinfo += buf;
                }
                if ((unsigned) c->right->id < rs.size()) {
                    Rectangle *r = rs[c->right->id];
                    sprintf(buf, "    v_%d rect: [%f, %f] x [%f, %f]\n", c->right->id,
                            r->getMinX(), r->getMaxX(), r->getMinY(), r->getMaxY());
                    unsatinfo += buf;
                }
                CompoundConstraint *cc = (CompoundConstraint*)(c->creator);
                unsatinfo += "    Creator: " + cc->toString() + "\n";
            }
        }
        if (debugLevel>1) {
            unsatinfo += "--------------------------------------------------\n";
            unsatinfo += "RELATED CONSTRAINTS:\n";
            std::set<Variable*>::iterator lit, rit, eit = varsInvolved.end();
            for (Constraints::iterator it=cs.begin(); it!=cs.end(); it++) {
                Constraint *c = *it;
                lit = varsInvolved.find(c->left);
                rit = varsInvolved.find(c->right);
                if (lit != eit || rit != eit) {
                    sprintf(buf, "v_%d + %f", c->left->id, c->gap);
                    unsatinfo += buf;
                    unsatinfo += c->equality ? " == " : " <= ";
                    sprintf(buf, "v_%d\n", c->right->id);
                    unsatinfo += buf;
                    if ((unsigned) c->left->id < rs.size()) {
                        Rectangle *r = rs[c->left->id];
                        sprintf(buf, "    v_%d rect: [%f, %f] x [%f, %f]\n", c->left->id,
                                r->getMinX(), r->getMaxX(), r->getMinY(), r->getMaxY());
                        unsatinfo += buf;
                    }
                    if ((unsigned) c->right->id < rs.size()) {
                        Rectangle *r = rs[c->right->id];
                        sprintf(buf, "    v_%d rect: [%f, %f] x [%f, %f]\n", c->right->id,
                                r->getMinX(), r->getMaxX(), r->getMinY(), r->getMaxY());
                        unsatinfo += buf;
                    }
                    CompoundConstraint *cc = (CompoundConstraint*)(c->creator);
                    unsatinfo += "    Creator: " + cc->toString() + "\n";
                }
            }
        }
    }
    ProjectionResult pr;
    pr.errorLevel = result;
    pr.unsatinfo = unsatinfo;
    return pr;
}

} // namespace cola
