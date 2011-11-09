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
 * Author(s):  Tim Dwyer
 *             Michael Wybrow
*/

#ifndef COLA_H
#define COLA_H

#include <utility>
#include <iterator>
#include <vector>
#include <valarray>
#include <algorithm>
#include <cmath>
#include <iostream>

#include "gradient_projection.h"
#include "cluster.h"
#include "straightener.h"
#include "exceptions.h"

namespace vpsc { class Rectangle; }
namespace topology { 
    class Node; 
    class Edge;
    class TopologyConstraints;
    typedef std::vector<Node*> Nodes;
}

/**
 * The cola namespace delineates the interface to the libcola constraint layout library
 */
namespace cola {

class NonOverlapConstraints;

//! Edges are simply a pair of indices to entries in the Node vector
typedef std::pair<unsigned, unsigned> Edge;

/**
 * A Lock specifies a required position for a Node
 */
class Lock {
public:
    Lock() {}
    Lock(unsigned id, double X, double Y) : id(id), x(X), y(Y) {
    }
    unsigned getID() const {
        return id;
    }
    double pos(vpsc::Dim dim) const {
        return dim==vpsc::HORIZONTAL?x:y;
    }
private:
    unsigned id;
    double x;
    double y;
};
typedef std::vector<cola::Lock> Locks;

/**
 * A Resize specifies a required bounding box for a node
 */
class Resize {
public:
    Resize() {}
    Resize(unsigned id, double x, double y, double w, double h)
        : id(id), target(x,x+w,y,y+h) {}
    unsigned getID() const {
        return id;
    }
    const vpsc::Rectangle* getTarget() const {
        return &target;
    }
private:
    unsigned id;
    vpsc::Rectangle target;
};
typedef std::vector<cola::Resize> Resizes;

/**
 * Setting a desired position for a node adds a term to the goal function
 * drawing the node towards that desired position
 */
struct DesiredPosition {
    unsigned id;
    double x;
    double y;
    double weight;
};
typedef std::vector<cola::DesiredPosition> DesiredPositions;
/** 
 * provides a functor that is called before each iteration in the main loop of
 * the ConstrainedMajorizationLayout::run() method.
 * Keeps a local copy of the x and y GradientProjection instances.
 * Override the operator() for things like locking the position of nodes
 * for the duration of the iteration.
 * If the operator() returns false the subsequent iterations are
 * abandoned... ie layout ends immediately.  You can make it return true
 * e.g. when a user-interrupt is detected.
 */
class PreIteration {
public:
    /**
     * @param locks a list of nodes (by id) and positions at which
     * they should be locked.
     * @param resizes a list of nodes (by id) and required dimensions
     * for their bounding rects.
     */
    PreIteration(
            Locks& locks=__locksNotUsed,
            Resizes& resizes=__resizesNotUsed) 
        : locks(locks)
        , resizes(resizes) 
        , changed(true) {}
    PreIteration(Resizes& resizes) 
        : locks(__locksNotUsed)
        , resizes(resizes)
        , changed(true) {}
    virtual ~PreIteration() {}
    virtual bool operator()() { return true; }
    Locks& locks;
    Resizes& resizes;
    bool changed;
private:
    static Locks __locksNotUsed;
    static Resizes __resizesNotUsed;
};

/** 
 * provides a functor for callback after each iteration
 * You can either call ConstrainedMajorizationLayout with an instance of this class setting the
 * tolerance and maxiterations as desired, or create a derived class implementing the operator() to
 * do your own convergence test, or create your own operator() that calls the
 * TestConvergence::operator() in order to do any other post processing you might need... e.g. to
 * animate changes.
 */
class TestConvergence {
public:
    double old_stress;
    TestConvergence(const double tol = 1e-4, const unsigned maxiterations = 100)
        : tolerance(tol),
          maxiterations(maxiterations)
    { reset(); }
    virtual ~TestConvergence() {}

private:
    TestConvergence(TestConvergence const &);
    TestConvergence &operator=(TestConvergence const &);

public:
    virtual bool operator()(const double new_stress, std::valarray<double> & X, std::valarray<double> & Y)
    {
        // Suppress unused parameter warning.
        (void) X;
        (void) Y;

        iterations++;
        //std::cout<<"iteration="<<iterations<<", old_stress="<<old_stress<<", new_stress="<<new_stress<<std::endl;
        if (old_stress == DBL_MAX) {
            old_stress = new_stress;
            return iterations >= maxiterations;
        }
        // converged if relative decrease in stress falls below threshold
        // or if stress increases (shouldn't happen for straight majorization)
        bool converged = 
            (old_stress - new_stress) / (new_stress + 1e-10) < tolerance
             || iterations > maxiterations;
        old_stress = new_stress;
        return converged;
    }
    void reset() {
        old_stress = DBL_MAX;
        iterations = 0;
    }
    const double tolerance;
    const unsigned maxiterations;
    unsigned iterations;
};

//! default instance of TestConvergence used if no other is specified
extern TestConvergence defaultTest;

/**
 * The following class implements the Constrained Majorization graph Layout algorithm.
 * The optimisation method used is "stress majorization", where a sequence of quadratic
 * functions which strictly bound the stress from above, is solved to monotonically reduce
 * the stress (by iteratively changing the configuration of nodes).
 */
class ConstrainedMajorizationLayout {
public:
    ConstrainedMajorizationLayout(
        std::vector<vpsc::Rectangle*>& rs,
        std::vector<Edge> const & es,
        RootCluster* clusterHierarchy,
        const double idealLength,
        const double* eLengths=NULL,
        TestConvergence& done=defaultTest,
        PreIteration* preIteration=NULL);
    /**
     * Horizontal and vertical compound constraints
     */
    void setConstraints(cola::CompoundConstraints* ccs) {
        constrainedLayout = true;
        this->ccs=ccs;
    }
    /**
     * These lists will have info about unsatisfiable constraints
     * after each iteration of constrained layout
     */
    void setUnsatisfiableConstraintInfo(
            UnsatisfiableConstraintInfos *unsatisfiableX,
            UnsatisfiableConstraintInfos *unsatisfiableY) {
        this->unsatisfiableX = unsatisfiableX;
        this->unsatisfiableY = unsatisfiableY;
    }
    /**
     * sticky nodes causes nodes to spring back to (startX,startY) when unconstrained
     */
    void setStickyNodes(const double stickyWeight, 
            std::valarray<double> const & startX,
            std::valarray<double> const & startY);

    /**
     * scaling speeds up the solver by conditioning the quadratic terms matrix.
     */
    void setScaling(bool scaling) {
        this->scaling=scaling;
    }
    /**
     * says that the Mosek optimisation library should be used to solve the quadratic
     * programs rather than the libvpsc solver
     */
    void setExternalSolver(bool externalSolver) {
        this->externalSolver=externalSolver;
    }
    /**
     * At each iteration of layout, generate constraints to avoid overlaps.
     * If bool horizontal is true, all overlaps will be resolved horizontally, otherwise
     * some overlaps will be left to be resolved vertically where doing so 
     * leads to less displacement
     */
    void setAvoidOverlaps(bool horizontal = false) {
        constrainedLayout = true;
        this->avoidOverlaps = horizontal ? Horizontal : Both;
    }
    /**
     * Add constraints to prevent clusters overlapping
     */
    void setNonOverlappingClusters() {
        constrainedLayout = true;
        nonOverlappingClusters = true;
    }
    /**
     * For the specified edges (with routings), generate dummy vars and constraints
     * to try and straighten them.
     * bendWeight controls how hard we try to straighten existing bends
     * potBendWeight controls how much we try to keep straight edges straight
     */
    void setStraightenEdges(std::vector<straightener::Edge*>* straightenEdges, 
            double bendWeight = 0.01, double potBendWeight = 0.1,
            bool xSkipping = true) {
        for(std::vector<straightener::Edge*>::const_iterator e=straightenEdges->begin();
                e!=straightenEdges->end();e++) {
            (*e)->rerouteAround(boundingBoxes);
        }
        constrainedLayout = true;
        this->xSkipping = xSkipping;
        this->straightenEdges = straightenEdges;
        this->bendWeight = bendWeight;
        this->potBendWeight = potBendWeight;
    }
    /** update position of bounding boxes
     */
    void moveBoundingBoxes() {
        for(unsigned i=0;i<n;i++) {
            boundingBoxes[i]->moveCentre(X[i],Y[i]);
        }
    }

    ~ConstrainedMajorizationLayout() {
        if(constrainedLayout) {
            delete gpX;
            delete gpY;
        }
    }
    /**
     * run the layout algorithm in either the x-dim the y-dim or both
     */
    void run(bool x=true, bool y=true);
    /**
     * run one iteration only
     */
    void runOnce(bool x=true, bool y=true);
    void straighten(std::vector<straightener::Edge*>&, vpsc::Dim);
    void setConstrainedLayout(bool c) {
        constrainedLayout=c;
    }
    double computeStress();
private:
    double euclidean_distance(unsigned i, unsigned j) {
        return sqrt(
            (X[i] - X[j]) * (X[i] - X[j]) +
            (Y[i] - Y[j]) * (Y[i] - Y[j]));
    }
    double compute_stress(std::valarray<double> const & Dij);
    void majorize(std::valarray<double> const & Dij,GradientProjection* gp, std::valarray<double>& coords, std::valarray<double> const & startCoords);
    void newton(std::valarray<double> const & Dij,GradientProjection* gp, std::valarray<double>& coords, std::valarray<double> const & startCoords);
    unsigned n; //!< number of nodes
    //std::valarray<double> degrees;
    std::valarray<double> lap2; //!< graph laplacian
    std::valarray<double> Q; //!< quadratic terms matrix used in computations
    std::valarray<double> Dij; //!< all pairs shortest path distances
    double tol; //!< convergence tolerance
    TestConvergence& done; //!< functor used to determine if layout is finished
    PreIteration* preIteration; //!< client can use this to create locks on nodes
    std::vector<vpsc::Rectangle*> boundingBoxes; //!< node bounding boxes
    /** stickyNodes controls whether nodes are attracted to their starting
     * positions (at time of ConstrainedMajorizationLayout instantiation)
     * stored in startX, startY
     */
    std::valarray<double> X, Y;
    bool stickyNodes;
    double stickyWeight;
    std::valarray<double> startX;
    std::valarray<double> startY;
    double edge_length;
    bool constrainedLayout;
    bool nonOverlappingClusters;
    /**
     * A cluster is a set of nodes that are somehow semantically grouped
     * and should therefore be kept together a bit more tightly than, and
     * preferably without overlapping, the rest of the graph.
     *
     * We achieve this by augmenting the L matrix with stronger attractive
     * forces between all members of a cluster (other than the root)
     * and by maintaining a (preferably convex) hull around those 
     * constituents which, using constraints and dummy variables, is 
     * prevented from overlapping other parts of the graph.
     *
     * Clusters are defined over the graph in a hierarchy starting with
     * a single root cluster.
     *
     * Need to:
     *  - augment Lap matrix with intra cluster forces
     *  - compute convex hull of each cluster
     *  - from convex hull generate "StraightenEdges"
     */
    RootCluster *clusterHierarchy;
    straightener::LinearConstraints *linearConstraints;
    GradientProjection *gpX, *gpY;
    cola::CompoundConstraints *ccs;
    UnsatisfiableConstraintInfos *unsatisfiableX, *unsatisfiableY;
    NonOverlapConstraintsMode avoidOverlaps;
    std::vector<straightener::Edge*>* straightenEdges;
    
    double bendWeight, potBendWeight;
    /** determines whether we should leave some overlaps to be resolved
     * vertically when generating straightening constraints in the x-dim
     */
    bool xSkipping;
    /** when using the gradient projection optimisation method, the following controls
     * whether the problem should be preconditioned by affine scaling
     */
    bool scaling;
    /** if the Mosek quadratic programming environment is available it may be used
       to solve each iteration of stress majorization... slow but useful for testing */
    bool externalSolver;
    bool majorization;
};

vpsc::Rectangle bounds(std::vector<vpsc::Rectangle*>& rs);

/**
 * This class implements a constrained layout method based on a non-linear
 * gradient projection technique, conceptually it's similar to a force
 * directed method like Fruchterman-Reingold-but using a more principled
 * goal function and optimisation techniques.
 */
class ConstrainedFDLayout {
public:
    /**
     * @param rs bounding boxes of nodes passed in at their initial positions
     * @param es simple pair edges, giving indices of the start and end nodes
     * @param idealLength is a scalar modifier of ideal edge lengths in eLengths
     * @param preventOverlaps causes non-overlap constraints to be generated 
     *        for all rectangles, if it is set to true.
     * @param eLengths individual ideal lengths for edges, actual ideal length 
     *        of the ith edge is idealLength*eLengths[i], if eLengths is NULL 
     *        then just idealLength is used (ie eLengths[i] is assumed to be 1).
     * @param done a test of convergence operation called at the end of each 
     *        iteration
     * @param preIteration an operation called before each iteration
     */
    ConstrainedFDLayout(
        const vpsc::Rectangles& rs,
        const std::vector<cola::Edge>& es,
        const double idealLength,
        const bool preventOverlaps,
        const double* eLengths=NULL,
        TestConvergence& done=defaultTest,
        PreIteration* preIteration=NULL);
    ~ConstrainedFDLayout();
    
    void run(bool x=true, bool y=true);
    void runOnce(bool x=true, bool y=true);
    void outputInstanceToSVG(std::string filename = std::string());
    /**
     *  Allow the user to specify compound constraints.
     */
    void setConstraints(const cola::CompoundConstraints& ccs)
    {
        this->ccs = ccs;
    }
    void setTopology(std::vector<topology::Node*> *tnodes, 
            std::vector<topology::Edge*> *routes);
    void getTopology(std::vector<topology::Node*> *tnodes, 
            std::vector<topology::Edge*> *routes);
    void setDesiredPositions(std::vector<DesiredPosition>* desiredPositions) {
        this->desiredPositions = desiredPositions;
    }
    void setClusterHierarchy(RootCluster *hierarchy)
    {
        clusterHierarchy = hierarchy;
    }
    /**
     * These lists will have info about unsatisfiable constraints
     * after each iteration of constrained layout
     */
    void setUnsatisfiableConstraintInfo(
            UnsatisfiableConstraintInfos* unsatisfiableX,
            UnsatisfiableConstraintInfos* unsatisfiableY) {
        unsatisfiable.resize(2);
        unsatisfiable[0]=unsatisfiableX;
        unsatisfiable[1]=unsatisfiableY;
    }

    void makeFeasible(void);
    double computeStress() const;

    //! @brief  A convenience method that can be called from Java to free
    //!         the memory of Rectangles, CompoundConstraints, etc.
    //! 
    //! This is useful because in SWIG we have problems with Java wrapper
    //! classes going out of scope and causing objects like Rectanges to 
    //! sometimes be freed when the layout instance still needs them.  For 
    //! this reason we prevent the Java wrappers from deleting the internal
    //! C++ instances, and let them be cleaned up later via this method.
    void freeAssociatedObjects(void);

private:
    unsigned n; // number of nodes
    std::valarray<double> X, Y;
    std::vector<vpsc::Rectangle*> boundingBoxes;
    double applyForcesAndConstraints(const vpsc::Dim dim,const double oldStress);
    double computeStepSize(const SparseMatrix& H, const std::valarray<double>& g,
            const std::valarray<double>& d) const;
    void computeDescentVectorOnBothAxes(const bool xaxis, const bool yaxis,
            double stress, std::valarray<double>& x0, std::valarray<double>& x1);
    void moveTo(const vpsc::Dim dim, std::valarray<double>& target);
    double applyDescentVector(
            const std::valarray<double>& d,
            const std::valarray<double>& oldCoords,
            std::valarray<double> &coords, 
            const double oldStress, 
            double stepsize
            /*,topology::TopologyConstraints *s=NULL*/);
    void computePathLengths(
            const std::vector<Edge>& es,
            const std::valarray<double> * eLengths);
    void generateNonOverlapAndClusterCompoundConstraints(
            vpsc::Variables (&vs)[2]);
    void handleResizes(const Resizes&);
    void setPosition(std::valarray<double>& pos);
    void moveBoundingBoxes();
    bool noForces(double, double, unsigned) const;
    void computeForces(const vpsc::Dim dim, SparseMap &H, 
            std::valarray<double> &g);
    void recGenerateClusterVariablesAndConstraints(
            vpsc::Variables (&vars)[2], unsigned int& priority, 
            cola::NonOverlapConstraints *noc, Cluster *cluster, 
            cola::CompoundConstraints& idleConstraints);

    std::vector<std::vector<unsigned> > neighbours;
    std::vector<std::vector<double> > neighbourLengths;
    TestConvergence& done;
    PreIteration* preIteration;
    cola::CompoundConstraints ccs;
    double** D;
    unsigned short** G;
    std::vector<topology::Node*> topologyNodes;
    std::vector<topology::Edge*> topologyRoutes;
    std::vector<UnsatisfiableConstraintInfos*> unsatisfiable;
    bool rungekutta;
    std::vector<DesiredPosition>* desiredPositions;
    cola::CompoundConstraints extraConstraints;
    
    RootCluster *clusterHierarchy;
    double rectClusterBuffer;
    double m_idealEdgeLength;
    bool m_generateNonOverlapConstraints;
};

/**
 * find shortest path lengths from node s to all other nodes.
 * @param s starting node
 * @param n total number of nodes
 * @param d n vector of path lengths
 * @param es edge pairs
 * @param eLengths edge weights
 */
void dijkstra(const unsigned s, const unsigned n, double* d, 
              const std::vector<cola::Edge>& es, const double* eLengths);

#if 0
void removeClusterOverlapFast(RootCluster& clusterHierarchy, vpsc::Rectangles& rs, Locks& locks);
#endif


}

#endif // COLA_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:cindent:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
