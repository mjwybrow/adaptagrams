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

#include "libcola/gradient_projection.h"
#include "libcola/cluster.h"
#include "libcola/straightener.h"
#include "libcola/exceptions.h"
#include "libcola/pseudorandom.h"

namespace vpsc { class Rectangle; }
namespace topology { 
    class ColaTopologyAddon;
}
namespace dialect {
    class Graph;
}


/**
 * @namespace cola
 * @brief libcola: Force-directed network layout subject to 
 *       separation constraints library.
 *
 * You should use COLA via an instance of the ConstrainedFDLayout class.
*/
namespace cola {

class NonOverlapConstraints;
class NonOverlapConstraintExemptions;

//! @brief A vector of node Indexes.
typedef std::vector<unsigned> NodeIndexes;

//! @brief A vector of NodeIndexes.
typedef std::vector<NodeIndexes> ListOfNodeIndexes;

//! Edges are simply a pair of indices to entries in the Node vector
typedef std::pair<unsigned, unsigned> Edge;

//! EdgeLengths is a vector of ideal lengths for edges corresponding to
//! edges in the edge list.
typedef std::vector<double> EdgeLengths;
#define StandardEdgeLengths EdgeLengths()

/**
 * @brief A Lock specifies a required position for a node.
 */
class Lock {
public:
    Lock() {}
    /**
     * @brief  Constructs a Lock object for a given node and position.
     *
     * @param[in] id  The index of the node in the Rectangles vector.
     * @param[in] X   The node's fixed position in the x-dimension.
     * @param[in] Y   The node's fixed position in the y-dimension.
     */
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
//! @brief A vector of Lock objects.
typedef std::vector<cola::Lock> Locks;

/**
 * @brief A Resize specifies a new required bounding box for a node.
 */
class Resize {
public:
    Resize() {}
    /**
     * @brief  Constructs a Resize object for a given node and it's new 
     *         bounding box.
     *
     * @param[in] id  The index of the node in the Rectangles vector.
     * @param[in] x   The minimum horizontal value for the node's new 
     *                bounding box.
     * @param[in] y   The minimum vertical value for the node's new 
     *                bounding box.
     * @param[in] w   The width value for the node's new bounding box.
     * @param[in] h   The height value for the node's new bounding box.
     */
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
//! @brief A vector of Resize objects.
typedef std::vector<cola::Resize> Resizes;

/*
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

/*
 * desired positions which should override those computed by applying forces
 * are passed in for a set of nodes.  The first entry is the Node->id, the
 * second is the desired position.
 */
typedef std::pair<unsigned,double> DesiredPositionInDim;
typedef std::vector<DesiredPositionInDim> DesiredPositionsInDim;

/** 
 * @brief A default functor that is called before each iteration in the 
 *        main loop of the ConstrainedFDLayout::run() method.
 *
 * Override the operator() for things like locking the position of nodes
 * for the duration of the iteration.
 *
 * If the operator() returns false the subsequent iterations are
 * abandoned, i.e., layout ends immediately.  You can make it return true
 * when a user-interrupt is detected, for instance.
 */
class PreIteration {
public:
    /**
     * @brief  Constructs a PreIteration object that handles locking and 
     *         resizing of nodes.
     *
     * @param[in] locks    A list of nodes (by index) and positions at which
     *                     they should be locked.
     * @param[in] resizes  A list of nodes (by index) and required dimensions
     *                     for their bounding rects to be resized to.
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

// To prevent C++ objects from being destroyed in garbage collected languages
// when the libraries are called from SWIG, we hide the declarations of the
// destructors and prevent generation of default destructors.
#ifndef SWIG
    virtual ~PreIteration() {}
#endif
    virtual bool operator()() { return true; }
    Locks& locks;
    Resizes& resizes;
    bool changed;
private:
    static Locks __locksNotUsed;
    static Resizes __resizesNotUsed;
};

/** 
 * @brief  A default functor that is called after each iteration of the layout
 *         algorithm.
 *
 * You can either instantiate ConstrainedFDLayout with an instance of this
 * class setting the tolerance and maxiterations as desired, or create a
 * derived class implementing the operator() to do your own convergence test,
 * or create your own operator() that calls the TestConvergence::operator() in
 * order to do any other post processing you might need, e.g., to animate
 * changes.
 */
class TestConvergence {
public:
    double old_stress;
    TestConvergence(const double tol = 1e-4, const unsigned maxiterations = 100)
        : tolerance(tol),
          maxiterations(maxiterations)
    { reset(); }
    virtual ~TestConvergence() {}

public:
    virtual bool operator()(const double new_stress, std::valarray<double> & X, std::valarray<double> & Y)
    {
        COLA_UNUSED(X);
        COLA_UNUSED(Y);

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

/**
 * @brief  Implements the Constrained Majorization graph layout algorithm 
 *         (deprecated).
 *
 * The optimisation method used is "stress majorization", where a sequence of 
 * quadratic functions which strictly bound the stress from above, is solved 
 * to monotonically reduce the stress (by iteratively changing the 
 * configuration of nodes).
 *
 * Once the problem has been set up, call run() or runOnce() to run the 
 * layout algorithm.
 *
 * @note  We recommend the use of ConstrainedFDLayout instead of this class. 
 *        ConstrainedFDLayout tends to produce better results and has more 
 *        features.  We are no longer working on ConstrainedMajorizationLayout.
 */
class ConstrainedMajorizationLayout {
public:
   /**
     * @brief Constructs a constrained majorization layout instance.
     *
     * @param[in] rs  Bounding boxes of nodes at their initial positions.
     * @param[in] es  Simple pair edges, giving indices of the start and end 
     *                nodes in rs.
     * @param[in] clusterHierarchy  A pointer to a RootCluster object defining
     *                              the cluster hierarchy (optional).
     * @param[in] idealLength  Aa scalar modifier of ideal edge lengths in 
     *                         eLengths.
     * @param[in] eLengths  Individual ideal lengths for edges.
     *                      The actual ideal length used for the ith edge is 
     *                      idealLength*eLengths[i], or if eLengths is empty 
     *                      then just idealLength is used (i.e., eLengths[i] 
     *                      is assumed to be 1).
     * @param[in] done  A test of convergence operation called at the end of 
     *                  each iteration (optional).
     * @param[in] preIteration  An operation called before each iteration
     *                          (optional).
     */
    ConstrainedMajorizationLayout(
        vpsc::Rectangles& rs,
        std::vector<Edge> const & es,
        RootCluster* clusterHierarchy,
        const double idealLength,
        EdgeLengths eLengths = StandardEdgeLengths,
        TestConvergence *doneTest = nullptr,
        PreIteration* preIteration=nullptr,
        bool useNeighbourStress = false);
    /**
     * @brief  Specify a set of compound constraints to apply to the layout.
     *
     * @param[in] ccs  The compound constraints.
     */
    void setConstraints(cola::CompoundConstraints* ccs) {
        constrainedLayout = true;
        this->ccs=ccs;
    }

    void setConstraintsVector(cola::CompoundConstraints& ccs) {
        constrainedLayout = true;
        cola::CompoundConstraints *ccsp = new cola::CompoundConstraints;
        for (size_t i = 0; i < ccs.size(); ++i) {
            ccsp->push_back(ccs.at(i));
        }
        this->ccs=ccsp;
    }

    /**
     * @brief Register to receive information about unsatisfiable constraints.
     *
     * In the case of unsatisifiable constraints, the solver will drop 
     * unsatisfiable constraints of lowest priority.  Information about these
     * will be written to these lists after each iteration of constrained 
     * layout.
     *
     * param[out] unsatisfiableX A pointer to an UnsatisfiableConstraintInfos 
     *                           object that will be used to record 
     *                           unsatisfiable constraints in the x-dimension.
     * param[out] unsatisfiableY A pointer to an UnsatisfiableConstraintInfos 
     *                           object that will be used to record 
     *                           unsatisfiable constraints in the y-dimension.
     */
    void setUnsatisfiableConstraintInfo(
            UnsatisfiableConstraintInfos *unsatisfiableX,
            UnsatisfiableConstraintInfos *unsatisfiableY) {
        this->unsatisfiableX = unsatisfiableX;
        this->unsatisfiableY = unsatisfiableY;
    }
    /**
     * Sticky nodes causes nodes to spring back to (startX,startY) when 
     * unconstrained.
     */
    void setStickyNodes(const double stickyWeight, 
            std::valarray<double> const & startX,
            std::valarray<double> const & startY);

    /**
     * Scaling speeds up the solver by conditioning the quadratic terms matrix.
     */
    void setScaling(bool scaling) {
        this->scaling=scaling;
    }
    /**
     * Says that the Mosek optimisation library should be used to solve the 
     * quadratic programs rather than the libvpsc solver.
     */
    void setExternalSolver(bool externalSolver) {
        this->externalSolver=externalSolver;
    }
    /**
     * At each iteration of layout, generate constraints to avoid overlaps.
     * If bool horizontal is true, all overlaps will be resolved horizontally, 
     * otherwise some overlaps will be left to be resolved vertically where 
     * doing so leads to less displacement
     */
    void setAvoidOverlaps(bool horizontal = false) {
        constrainedLayout = true;
        this->avoidOverlaps = horizontal ? Horizontal : Both;
    }
    /**
     * Add constraints to prevent clusters overlapping.
     */
    void setNonOverlappingClusters() {
        constrainedLayout = true;
        nonOverlappingClusters = true;
    }
    /**
     * For the specified edges (with routings), generate dummy vars and
     * constraints to try and straighten them.  bendWeight controls how hard we
     * try to straighten existing bends potBendWeight controls how much we try
     * to keep straight edges straight
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
    /** 
     * Update position of bounding boxes.
     */
    void moveBoundingBoxes() {
        for(unsigned i=0;i<n;i++) {
            boundingBoxes[i]->moveCentre(X[i],Y[i]);
        }
    }

    ~ConstrainedMajorizationLayout() {
        if (using_default_done)
        {
            delete done;
        }

        if(constrainedLayout) {
            delete gpX;
            delete gpY;
        }
    }
    /**
     * @brief  Implements the main layout loop, taking descent steps until
     *         stress is no-longer significantly reduced.
     *
     * @param[in] x  If true, layout will be performed in x-dimension
     *               (default: true).
     * @param[in] y  If true, layout will be performed in y-dimension
     *               (default: true).
     */
    void run(bool x=true, bool y=true);
    /**
     * @brief  Same as run(), but only applies one iteration.  
     *
     * This may be useful here it's too hard to implement a call-back 
     * (e.g., in java apps).
     *
     * @param[in] x  If true, layout will be performed in x-dimension
     *               (default: true).
     * @param[in] y  If true, layout will be performed in y-dimension
     *               (default: true).
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
    unsigned n; //< number of nodes
    //std::valarray<double> degrees;
    std::valarray<double> lap2; //< graph laplacian
    std::valarray<double> Q; //< quadratic terms matrix used in computations
    std::valarray<double> Dij; //< all pairs shortest path distances
    double tol; //< convergence tolerance
    TestConvergence *done; //< functor used to determine if layout is finished
    bool using_default_done; // Whether we allocated a default TestConvergence object.
    PreIteration* preIteration; //< client can use this to create locks on nodes
    vpsc::Rectangles boundingBoxes; //< node bounding boxes
    /*
     * stickyNodes controls whether nodes are attracted to their starting
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
    /*
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
    GradientProjection *gpX, *gpY;
    cola::CompoundConstraints *ccs;
    UnsatisfiableConstraintInfos *unsatisfiableX, *unsatisfiableY;
    NonOverlapConstraintsMode avoidOverlaps;
    std::vector<straightener::Edge*>* straightenEdges;
    
    double bendWeight, potBendWeight;
    /* 
     * determines whether we should leave some overlaps to be resolved
     * vertically when generating straightening constraints in the x-dim
     */
    bool xSkipping;
    /* 
     * when using the gradient projection optimisation method, the following
     * controls whether the problem should be preconditioned by affine scaling
     */
    bool scaling;
    /* 
     * if the Mosek quadratic programming environment is available it may be
     * used to solve each iteration of stress majorization... slow but useful
     * for testing 
     */
    bool externalSolver;
    bool majorization;
};

vpsc::Rectangle bounds(vpsc::Rectangles& rs);

class ConstrainedFDLayout;

/**
 * @brief  Interface for writing COLA addons to handle topology preserving 
 *         layout.
 */
class TopologyAddonInterface 
{
    public:
        TopologyAddonInterface()
        {
        }

        virtual ~TopologyAddonInterface()
        {
        }

        virtual TopologyAddonInterface *clone(void) const
        {
            return new TopologyAddonInterface(*this);
        }
        
        virtual void freeAssociatedObjects(void)
        {
        }

        virtual void handleResizes(const Resizes& resizeList, unsigned n,
                std::valarray<double>& X, std::valarray<double>& Y, 
                cola::CompoundConstraints& ccs, 
                vpsc::Rectangles& boundingBoxes,
                cola::RootCluster* clusterHierarchy)
        {
            COLA_UNUSED(resizeList);
            COLA_UNUSED(n);
            COLA_UNUSED(X);
            COLA_UNUSED(Y);
            COLA_UNUSED(ccs);
            COLA_UNUSED(boundingBoxes);
            COLA_UNUSED(clusterHierarchy);
        }
        virtual void computePathLengths(unsigned short** G)
        {
            COLA_UNUSED(G);
        }
        virtual double computeStress(void) const
        {
            return 0;
        }
        virtual bool useTopologySolver(void) const
        {
            return false;
        }
        virtual void makeFeasible(bool generateNonOverlapConstraints, 
                vpsc::Rectangles& boundingBoxes, 
                cola::RootCluster* clusterHierarchy)
        {
            COLA_UNUSED(generateNonOverlapConstraints);
            COLA_UNUSED(boundingBoxes);
            COLA_UNUSED(clusterHierarchy);
        }
        virtual void moveTo(const vpsc::Dim dim, vpsc::Variables& vs, 
                vpsc::Constraints& cs, std::valarray<double> &coords,
                cola::RootCluster* clusterHierarchy)
        {
            COLA_UNUSED(dim);
            COLA_UNUSED(vs);
            COLA_UNUSED(cs);
            COLA_UNUSED(coords);
            COLA_UNUSED(clusterHierarchy);
        }
        virtual double applyForcesAndConstraints(ConstrainedFDLayout *layout, 
                const vpsc::Dim dim, std::valarray<double>& g, 
                vpsc::Variables& vs, vpsc::Constraints& cs, 
                std::valarray<double> &coords,
                DesiredPositionsInDim& des, double oldStress)
        {
            COLA_UNUSED(layout);
            COLA_UNUSED(dim);
            COLA_UNUSED(g);
            COLA_UNUSED(vs);
            COLA_UNUSED(cs);
            COLA_UNUSED(coords);
            COLA_UNUSED(des);
            COLA_UNUSED(oldStress);
            return 0;
        }
};


/**
 * @brief Implements a constrained force-directed layout algorithm.
 *
 * This method is based on a non-linear gradient projection technique.
 * Conceptually it's similar to a force directed method like
 * Fruchterman-Reingold---but using a more principled goal function and
 * optimisation techniques.
 */
class ConstrainedFDLayout {
public:
    /**
     * @brief Constructs a constrained force-directed layout instance.
     *
     * If an empty edges (es) vector is passed to the constructor, then 
     * constraint satisfaction will be performed, but no force-directed
     * layout.  In this case, idealLength and eLengths have no effect.
     *
     * Conversely, if no constraints or clusters are specified and no overlap
     * prevention is enabled, but edge info is given, then pure force-directed 
     * layout will be performed.
     *
     * @param[in] rs  Bounding boxes of nodes at their initial positions.
     * @param[in] es  Simple pair edges, giving indices of the start and end 
     *                nodes in rs.
     * @param[in] idealLength  A scalar modifier of ideal edge lengths in 
     *                         eLengths or of 1 if no ideal lengths are 
     *                         specified.
     * @param[in] eLengths  Individual ideal lengths for edges.
     *                      The actual ideal length used for the ith edge is 
     *                      idealLength*eLengths[i], or if eLengths is nullptr a
     *                      then just idealLength is used (i.e., eLengths[i] 
     *                      is assumed to be 1).
     * @param[in] done  A test of convergence operation called at the end of 
     *                  each iteration (optional).  If not given, uses a
     *                  default TestConvergence object.
     * @param[in] preIteration  An operation called before each iteration
     *                          (optional).
     */
    ConstrainedFDLayout(
        const vpsc::Rectangles& rs,
        const std::vector<cola::Edge>& es,
        const double idealLength,
        const EdgeLengths& eLengths = StandardEdgeLengths, 
        TestConvergence* doneTest = nullptr,
        PreIteration* preIteration = nullptr);
    ~ConstrainedFDLayout();
  
    /**
     * @brief  Implements the main layout loop, taking descent steps until
     *         stress is no-longer significantly reduced.
     *
     * @param[in] x  If true, layout will be performed in x-dimension
     *               (default: true).
     * @param[in] y  If true, layout will be performed in y-dimension
     *               (default: true).
     */
    void run(bool x=true, bool y=true);

    /**
     * @brief  Same as run(), but only applies one iteration.  
     *
     * This may be useful here it's too hard to implement a call-back 
     * (e.g., in java apps).
     *
     * @param[in] x  If true, layout will be performed in x-dimension
     *               (default: true).
     * @param[in] y  If true, layout will be performed in y-dimension
     *               (default: true).
     */
    void runOnce(bool x=true, bool y=true);

    /**
     * @brief  Specify a set of compound constraints to apply to the layout.
     *
     * @param[in] ccs  The compound constraints.
     */
    void setConstraints(const cola::CompoundConstraints& ccs);

    /**
     * @brief  Specifies whether non-overlap constraints should be
     *         automatically generated between all nodes, as well as any
     *         exemptions to this.
     *
     * The optional second parameter indicates groups of nodes that should be
     * exempt from having non-overlap constraints generated between each other.
     * For example, you might want to do this for nodes representing ports, or
     * the child nodes in a particular cluster.
     *
     * @param[in] avoidOverlaps     New boolean value for this option.
     * @param[in] listOfNodeGroups  A list of groups of node indexes which will
     *                              not have non-overlap constraints generated
     *                              between each other.
     */
    void setAvoidNodeOverlaps(bool avoidOverlaps,
            ListOfNodeIndexes listOfNodeGroups =
            ListOfNodeIndexes());

    /** 
     *  @brief  Set an addon for doing topology preserving layout.
     *
     *  It is expected that you would use the topology::ColaTopologyAddon() 
     *  from libtopology rather than write your own.  This is done so that 
     *  libcola does not have to depend on libtopology.
     *
     *  @param[in] topology  Instance of a class implementing the
     *                       TopologyAddonInterface.
     *  @sa topology::ColaTopologyAddon
     */
    void setTopology(TopologyAddonInterface *topology); 
    TopologyAddonInterface *getTopology(void);
    
    void setDesiredPositions(DesiredPositions *desiredPositions);

    /**
     * @brief  Specifies an optional hierarchy for clustering nodes.
     *
     * @param[in] hierarchy  A pointer to a RootCluster object defining the
     *                      the cluster hierarchy (optional).
     */
    void setClusterHierarchy(RootCluster *hierarchy)
    {
        clusterHierarchy = hierarchy;
    }
    /**
     * @brief Register to receive information about unsatisfiable constraints.
     *
     * In the case of unsatisifiable constraints, the solver will drop 
     * unsatisfiable constraints of lowest priority.  Information about these
     * will be written to these lists after each iteration of constrained 
     * layout.
     *
     * param[out] unsatisfiableX A pointer to a UnsatisfiableConstraintInfos 
     *                           object that will be used to record 
     *                           unsatisfiable constraints in the x-dimension.
     * param[out] unsatisfiableY A pointer to a UnsatisfiableConstraintInfos 
     *                           object that will be used to record 
     *                           unsatisfiable constraints in the y-dimension.
     */
    void setUnsatisfiableConstraintInfo(
            UnsatisfiableConstraintInfos *unsatisfiableX,
            UnsatisfiableConstraintInfos *unsatisfiableY)
    {
        unsatisfiable.resize(2);
        unsatisfiable[0]=unsatisfiableX;
        unsatisfiable[1]=unsatisfiableY;
    }
    /**
     * @brief Finds a feasible starting position for nodes that satisfies the
     *        given constraints.
     *
     * Starts with an initial position (x, y) for the nodes.  This position 
     * is then iteratively updated with a greedy heuristic that tries adding
     * additional constraints based on compound constraint priority to the
     * satisfiable set, so as to satisfy as many of the placement constraints
     * as possible.  This includes automatically generated constraints for
     * non-overlap and cluster containment.
     *
     * @param[in] xBorder  Optional border width to add to left and right
     *                     sides of rectangles. Defaults to 1.
     * @param[in] yBorder  Optional border width to add to top and bottom
     *                     sides of rectangles. Defaults to 1.
     *
     * @note This method doesn't do force-directed layout.  All forces are 
     *       ignored and it merely satisfies the constraints with minimal 
     *       movement to nodes.
     */
    void makeFeasible(double xBorder=1, double yBorder=1);

    /**
     * @brief  A convenience method that can be called from Java to free
     *         the memory of nodes (Rectangles), CompoundConstraints, etc.
     * 
     * This assumes that the ConstrainedFDLayout instance takes ownership
     * of all the objects passed to it.
     *
     * This is useful because in SWIG we have problems with Java wrapper
     * classes going out of scope and causing objects like Rectanges to 
     * sometimes be freed when the layout instance still needs them.  For 
     * this reason we prevent the Java wrappers from deleting the internal
     * C++ instances, and let them be cleaned up later via this method.
     */
    void freeAssociatedObjects(void);
    
    //! @brief  Generates an SVG file containing debug output and code that
    //!         can be used to regenerate the instance.
    //!
    //! This method can be called before or after run() or makeFeasible()
    //! have been called.
    //!
    //! @param[in] filename  A string indicating the filename (without 
    //!                      extension) for the output file.  Defaults to
    //!                      "libcola-debug.svg" if no filename is given.
    //!
    void outputInstanceToSVG(std::string filename = std::string());

    /**
     * @brief  Specifies whether neighbour stress should be used.
     *
     * Under neighbour stress, only the terms representing neighbouring
     * nodes contribute to the stress function. This can help to distribute
     * nodes more evenly, eliminating long-range forces.
     *
     * Default value is false.
     *
     * @param[in] useNeighbourStress  New boolean value for this option.
     */
    void setUseNeighbourStress(bool useNeighbourStress);

    /**
     * @brief  Retrieve a copy of the "D matrix" computed by the computePathLengths
     * method, linearised as a vector.
     *
     * This is especially useful for projects in SWIG target languages that want to
     * do their own computations with stress.
     *
     * D is the required euclidean distances between pairs of nodes
     * based on the shortest paths between them (using
     * m_idealEdgeLength*eLengths[edge] as the edge length, if eLengths array
     * is provided otherwise just m_idealEdgeLength).
     *
     * @return  vector representing the D matrix.
     */
    std::vector<double> readLinearD(void);

    /**
     * @brief  Retrieve a copy of the "G matrix" computed by the computePathLengths
     * method, linearised as a vector.
     *
     * * This is especially useful for projects in SWIG target languages that want to
     * do their own computations with stress.
     *
     * G is a matrix of unsigned ints such that G[u][v]=
     *   0 if there are no forces required between u and v
     *     (for example, if u and v are in unconnected components)
     *   1 if attractive forces are required between u and v
     *     (i.e. if u and v are immediately connected by an edge and there is
     *      no topology route between u and v (for which an attractive force
     *      is computed elsewhere))
     *   2 if no attractive force is required between u and v but there is
     *     a connected path between them.
     *
     * @return  vector representing the G matrix.
     */
    std::vector<unsigned> readLinearG(void);

    double computeStress() const;

private:
    unsigned n; // number of nodes
    std::valarray<double> X, Y;
    vpsc::Rectangles boundingBoxes;
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
            /*,topology::TopologyConstraints *s=nullptr*/);
    void computePathLengths(
            const std::vector<Edge>& es, std::valarray<double> eLengths);
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
    std::vector<double> offsetDir(double minD);

    void computeNeighbours(std::vector<Edge> es);
    std::vector<std::vector<unsigned> > neighbours;
    std::vector<std::vector<double> > neighbourLengths;
    TestConvergence *done;
    bool using_default_done; // Whether we allocated a default TestConvergence object.
    PreIteration* preIteration;
    cola::CompoundConstraints ccs;
    double** D;
    unsigned short** G;
    double minD;
    PseudoRandom random;

    TopologyAddonInterface *topologyAddon;
    std::vector<UnsatisfiableConstraintInfos*> unsatisfiable;
    bool rungekutta;
    DesiredPositions *desiredPositions;
    cola::CompoundConstraints extraConstraints;
    
    RootCluster *clusterHierarchy;
    double rectClusterBuffer;
    double m_idealEdgeLength;
    bool m_generateNonOverlapConstraints;
    bool m_useNeighbourStress;
    const std::valarray<double> m_edge_lengths;

    NonOverlapConstraintExemptions *m_nonoverlap_exemptions;

    friend class topology::ColaTopologyAddon;
    friend class dialect::Graph;
};

struct ProjectionResult {
    int errorLevel;
    std::string unsatinfo;
};

/**
 * @brief Attempt to do a projection onto a vector of cola CompoundConstraints.
 * @param dim the dimension in which to perform the projection
 * @param rs the rectangles representing the nodes
 * @param ccs the constraints
 * @param preventOverlaps boolean saying whether you want overlap prevention
 *                        constraints to be automatically generated
 * @param accept  an integer indicating which types of infeasibilities you will accept.
 *                The default value of 0 means you accept no infeasibility.
 *                For other values, see the description of the "errorLevel" in the
 *                doctext for the solve function below.
 * @param debugLevel see solve function below
 * @note          Rectangle positions are updated if and only if the error level is less
 *                than or equal to the accept level.
 * @return a ProjectionResult indicating whether the projection was feasible or not.
 * @sa solve
 */
ProjectionResult projectOntoCCs(vpsc::Dim dim, vpsc::Rectangles &rs, cola::CompoundConstraints ccs,
                                bool preventOverlaps, int accept=0, unsigned debugLevel=0);

/**
 * @brief Constructs a solver and attempts to solve the passed constraints on the passed vars.
 * @param debugLevel: controls how much information comes back when the projection fails. See below.
 * @return a ProjectionResult, containing:
 *  errorLevel:
 *   0: all constraints were satisfiable.
 *   1: some constraints were unsatisfiable, but they were all nonoverlap constraints.
 *   2: some constraints were unsatisfiable which were /not/ nonoverlap constraints.
 *  unsatinfo:
 *   The amount of information reported depends on the debugLevel:
 *   0: nothing reported (empty string)
 *   1: description of the unsatisfied constraints
 *   2: the info from level 1, plus a description of all "related" constraints (those sharing a variable).
 *      This is useful for understanding the conflicts.
 */
ProjectionResult solve(vpsc::Variables &vs, vpsc::Constraints &cs, vpsc::Rectangles &rs,
                        unsigned debugLevel=0);


ConstrainedMajorizationLayout* simpleCMLFactory(
        vpsc::Rectangles& rs,
        std::vector<Edge> const & es,
        RootCluster* clusterHierarchy,
        const double idealLength,
        bool useNeighbourStress = false
    );

/*
 * find shortest path lengths from node s to all other nodes.
 * @param s starting node
 * @param n total number of nodes
 * @param d n vector of path lengths
 * @param es edge pairs
 * @param eLengths edge weights
 */
void dijkstra(const unsigned s, const unsigned n, double* d, 
              const std::vector<cola::Edge>& es,
              const std::valarray<double> & eLengths);

#if 0
void removeClusterOverlapFast(RootCluster& clusterHierarchy, vpsc::Rectangles& rs, Locks& locks);
#endif

void setupVarsAndConstraints(unsigned n, const CompoundConstraints& ccs,
        const vpsc::Dim dim, vpsc::Rectangles& boundingBoxes,
        RootCluster *clusterHierarchy,
        vpsc::Variables& vs, vpsc::Constraints& cs, 
        std::valarray<double> &coords);
void setVariableDesiredPositions(vpsc::Variables& vs, vpsc::Constraints& cs,
        const DesiredPositionsInDim& des, std::valarray<double>& coords);

}
#endif // COLA_H
