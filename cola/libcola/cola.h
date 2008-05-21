#ifndef COLA_H
#define COLA_H

#include <utility>
#include <iterator>
#include <vector>
#include <valarray>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cassert>
#include "gradient_projection.h"
#include "cluster.h"
#include "straightener.h"

namespace vpsc { class Rectangle; }
namespace topology { 
    class Node; 
    class Edge;
    class TopologyConstraints;
}

/**
 * The cola namespace delineates the interface to the libcola constraint layout library
 */
namespace cola {
using vpsc::Rectangle;

//! Edges are simply a pair of indices to entries in the Node vector
typedef std::pair<unsigned, unsigned> Edge;

/**
 * A Lock specifies a required position for a Node
 */
class Lock {
public:
    Lock(unsigned id, double X, double Y) : id(id), x(X), y(Y) {
    }
    unsigned getID() const {
        return id;
    }
    double pos(cola::Dim dim) const {
        return dim==HORIZONTAL?x:y;
    }
private:
    unsigned id;
    double x;
    double y;
};
typedef std::vector<Lock> Locks;

/**
 * A Resize specifies a required bounding box for a node
 */
class Resize {
public:
    Resize(unsigned id, double x, double y, double w, double h)
        : id(id), target(x,x+w,y,y+h) {}
    unsigned getID() const {
        return id;
    }
    const Rectangle* getTarget() const {
        return &target;
    }
private:
    unsigned id;
    Rectangle target;
};
typedef std::vector<Resize> Resizes;

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
    virtual bool operator()(const double new_stress, std::valarray<double> & X, std::valarray<double> & Y) {
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
        std::vector<Rectangle*>& rs,
        std::vector<Edge> const & es,
        RootCluster* clusterHierarchy,
        double const idealLength,
        std::valarray<double> const * eweights=NULL,
        TestConvergence& done=defaultTest,
        PreIteration* preIteration=NULL);
    /**
     * Horizontal alignment constraints
     */
    void setXConstraints(CompoundConstraints* ccsx) {
        constrainedLayout = true;
        this->ccsx=ccsx;
    }
    /**
     * Vertical alignment constraints
     */
    void setYConstraints(CompoundConstraints* ccsy) {
        constrainedLayout = true;
        this->ccsy=ccsy;
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
        this->avoidOverlaps = horizontal?Horizontal:Both;
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
    void straighten(std::vector<straightener::Edge*>&, cola::Dim);
    void setConstrainedLayout(bool c) {
        constrainedLayout=c;
    }
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
    std::vector<Rectangle*> boundingBoxes; //!< node bounding boxes
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
    CompoundConstraints *ccsx, *ccsy;
    UnsatisfiableConstraintInfos *unsatisfiableX, *unsatisfiableY;
    NonOverlapConstraints avoidOverlaps;
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

Rectangle bounds(std::vector<Rectangle*>& rs);

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
     * @param idealLength is a scalar modifier of ideal edge lengths in eweights
     * @param eweights individual ideal lengths for edges, actual ideal length of the
     * ith edge is idealLength*eweights[i], if eweights is NULL then just idealLength
     * is used (ie eweights[i] is assumed to be 1).
     * @param done a test of convergence operation called at the end of each iteration
     * @param preIteration an operation called before each iteration
     */
    ConstrainedFDLayout(
        const std::vector<Rectangle*>& rs,
        const std::vector<Edge>& es,
        const double idealLength,
        const std::valarray<double>* eweights=NULL,
        TestConvergence& done=defaultTest,
        PreIteration* preIteration=NULL);
    void run(bool x=true, bool y=true);
    /**
     * Horizontal alignment constraints
     */
    void setXConstraints(CompoundConstraints* ccsx) {
        if(ccsx->size()>0) {
            this->ccsx=ccsx;
        }
    }
    /**
     * Vertical alignment constraints
     */
    void setYConstraints(CompoundConstraints* ccsy) {
        if(ccsy->size()>0) {
            this->ccsy=ccsy;
        }
    }
    void setTopology(std::vector<topology::Node*>* tnodes, std::vector<topology::Edge*>* routes) {
        //printf("setTopology...\n");
        topologyNodes=tnodes;
        topologyRoutes=routes;
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
    ~ConstrainedFDLayout() {
        for(unsigned i=0;i<n;i++) {
            delete [] G[i];
            delete [] D[i];
        }
        delete [] G;
        delete [] D;
    }
    double computeStress() const;
private:
    unsigned n; // number of nodes
    std::valarray<double> X, Y;
    std::vector<Rectangle*> boundingBoxes;
    double applyForcesAndConstraints(const Dim dim,const double oldStress);
    double computeStepSize(const SparseMatrix& H, const std::valarray<double>& g,
            const std::valarray<double>& d) const;
    void computeDescentVectorOnBothAxes(const bool xaxis, const bool yaxis,
            double stress, std::valarray<double>& x0, std::valarray<double>& x1);
    void moveTo(const Dim dim, std::valarray<double>& target);
    double applyDescentVector(
            const std::valarray<double>& d,
            const std::valarray<double>& oldCoords,
            std::valarray<double> &coords, 
            const double oldStress, 
            double stepsize
            /*,topology::TopologyConstraints *s=NULL*/);
    void computePathLengths(
            const std::vector<Edge>& es,
            const double idealLength,
            const std::valarray<double> * eweights);
    void handleResizes(const Resizes&);
    void setPosition(std::valarray<double>& pos);
    void moveBoundingBoxes();
    bool noForces(double, double, unsigned) const;
    void computeForces(const Dim dim, SparseMap &H, std::valarray<double> &g);
    std::vector<std::vector<unsigned> > neighbours;
    std::vector<std::vector<double> > neighbourLengths;
    TestConvergence& done;
    PreIteration* preIteration;
    CompoundConstraints *ccsx, *ccsy;
    double** D;
    unsigned short** G;
    std::vector<topology::Node*>* topologyNodes;
    std::vector<topology::Edge*>* topologyRoutes;
    std::vector<UnsatisfiableConstraintInfos*> unsatisfiable;
    bool rungekutta;
};

}
#endif				// COLA_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
