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
namespace vpsc { class Rectangle; }
namespace straightener { 
    class Edge;
    class StraightenEdges; 
}

namespace cola {
using vpsc::Rectangle;
using std::vector;
using std::valarray;

// Edges are simply a pair of indices to entries in the Node vector
typedef std::pair<unsigned, unsigned> Edge;

// A cluster is a set of nodes to be kept together somehow
class Cluster {
public:
    double margin;
    double internalEdgeWeightFactor;
    vector<unsigned> nodes;
    valarray<double> hullX, hullY;
    Cluster();
    virtual ~Cluster();
    virtual void computeBoundary(vector<Rectangle*> const & rs) = 0;
};
class RectangularCluster : public Cluster {
public:
    void computeBoundary(vector<Rectangle*> const & rs);
};
class ConvexCluster : public Cluster {
public:
    void computeBoundary(vector<Rectangle*> const & rs);
};
typedef vector<Cluster*> Clusters;

// defines references to three variables for which the goal function
// will be altered to prefer points u-b-v are in a linear arrangement
// such that b is placed at u+t(v-u).
struct LinearConstraint {
    LinearConstraint(unsigned u, unsigned v, unsigned b, double w, 
                     valarray<double> const & X, valarray<double> const & Y) 
        : u(u),v(v),b(b),w(w)
    {
        // from cosine rule: ub.uv/|uv|=|ub|cos(theta)
        double uvx = X[v] - X[u],
               uvy = Y[v] - Y[u],
               ubx = X[b] - X[u],
               uby = Y[b] - Y[u],
               duv2 = uvx * uvx + uvy * uvy;
        if(duv2 < 0.0001) {
            t=0;
        } else {
            t = (uvx * ubx + uvy * uby)/duv2;
        }
        duu=(1-t)*(1-t);
        duv=t*(1-t);
        dub=t-1;
        dvv=t*t;
        dvb=-t;
        dbb=1;
        //printf("New LC: t=%f\n",t); 
    }
    unsigned u;
    unsigned v;
    unsigned b;
    double w; // weight
    double t;
    // 2nd partial derivatives of the goal function
    //   (X[b] - (1-t) X[u] - t X[v])^2
    double duu;
    double duv;
    double dub;
    double dvv;
    double dvb;
    double dbb;
};
typedef vector<LinearConstraint*> LinearConstraints;
	
/**
 * provides a functor that is called before each iteration in the main loop of
 * the ConstrainedMajorizationLayout::run() method.
 * Keeps a local copy of the x and y GradientProjection instances.
 * Use it for things like locking the position of nodes just for the duration of the iteration.
 */ 
struct Lock {
    unsigned id;
    double x,y;
    Lock(unsigned id, double x, double y) : id(id), x(x), y(y) {}
};
typedef vector<Lock> Locks;
class PreIteration {
public:
    PreIteration(Locks& locks) : locks(locks) {}
    virtual ~PreIteration() {}
    virtual void operator()(GradientProjection* gpX, GradientProjection* gpY) {}
    Locks& locks;
private:
};
/** 
 * The following class provides a functor for callback after each iteration
 * You can either call ConstrainedMajorizationLayout with an instance of this class setting the
 * tolerance and maxiterations as desired, or create a derived class implementing the operator() to
 * do your own convergence test, or create your own operator() that calls the
 * TestConvergence::operator() in order to do any other post processing you might need... e.g. to
 * animate changes.
 */
class TestConvergence {
public:
    double old_stress;
    TestConvergence(const double tolerance = 0.001, const unsigned maxiterations = 1000)
        : tolerance(tolerance),
          maxiterations(maxiterations)
    { reset(); }
    virtual ~TestConvergence() {}

    virtual bool operator()(const double new_stress, valarray<double> & X, valarray<double> & Y) {
        //std::cout<<"iteration="<<iterations<<", new_stress="<<new_stress<<std::endl;
        if (old_stress == DBL_MAX) {
            old_stress = new_stress;
            if(++iterations>=maxiterations) {;
            return true;
            } else {
                return false;
            }
        }
        bool converged = 
            fabs(new_stress - old_stress) / (new_stress + 1e-10) < tolerance
            || ++iterations > maxiterations;
        old_stress = new_stress;
        return converged;
    }
    void reset() {
        old_stress = DBL_MAX;
        iterations = 0;
    }
private:
    const double tolerance;
    const unsigned maxiterations;
    unsigned iterations;
};

// The following instance of TestConvergence is used if no other is
// specified
static TestConvergence defaultTest(0.0001,100);

class ConstrainedMajorizationLayout {
public:
    ConstrainedMajorizationLayout(
        vector<Rectangle*>& rs,
        vector<Edge> const & es,
        Clusters* cs,
        double const idealLength,
        std::valarray<double> const * startX=NULL,
        std::valarray<double> const * startY=NULL,
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
    void setStraightenEdges(vector<straightener::Edge*>* straightenEdges, 
            double bendWeight = 0.01, double potBendWeight = 0.1) {
        constrainedLayout = true;
        this->straightenEdges = straightenEdges;
        this->bendWeight = bendWeight;
        this->potBendWeight = potBendWeight;
    }
    void moveBoundingBoxes() {
        for(unsigned i=0;i<n;i++) {
            boundingBoxes[i]->moveCentre(X[i],Y[i]);
        }
    }

    void addLinearConstraints(LinearConstraints* linearConstraints);

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
    void straighten(vector<straightener::Edge*>&, Dim);
private:
    double euclidean_distance(unsigned i, unsigned j) {
        return sqrt(
            (X[i] - X[j]) * (X[i] - X[j]) +
            (Y[i] - Y[j]) * (Y[i] - Y[j]));
    }
    double compute_stress(valarray<double> const & Dij);
    void majlayout(valarray<double> const & Dij,GradientProjection* gp, valarray<double>& coords, valarray<double> const * startCoords);
    unsigned n; // number of nodes
    valarray<double> lap2; // graph laplacian
    valarray<double> Q; // quadratic terms matrix used in computations
    valarray<double> Dij; // all pairs shortest path distances
    double tol;
    TestConvergence& done;
    PreIteration* preIteration;
    vector<Rectangle*> boundingBoxes;
    // stickyNodes controls whether nodes are attracted to their starting
    // positions (at time of ConstrainedMajorizationLayout instantiation)
    // stored in startX, startY
    bool stickyNodes;
    double stickyWeight;
    valarray<double> const *startX, *startY;

    valarray<double> X, Y;
    double edge_length;
    bool constrainedLayout;
    bool nonOverlappingClusters;
    /*
     * Clusters are a set of nodes that are somehow semantically grouped
     * and should therefore be kept together a bit more tightly than, and
     * preferably without overlapping, the rest of the graph.
     *
     * We achieve this by augmenting the L matrix with stronger attractive
     * forces between all members of the cluster and by maintaining a
     * (preferably convex) hull around those constituents which, using
     * constraints and dummy variables, is prevented from overlapping
     * other parts of the graph.
     *
     * Need to:
     *  - augment Lap matrix with intra cluster forces
     *  - compute convex hull of each cluster
     *  - from convex hull generate "StraightenEdges"
     */
    Clusters *clusters;
    LinearConstraints *linearConstraints;
    GradientProjection *gpX, *gpY;
    CompoundConstraints *ccsx, *ccsy;
    NonOverlapConstraints avoidOverlaps;
    vector<straightener::Edge*>* straightenEdges;
    
    double bendWeight, potBendWeight;
};

Rectangle bounds(vector<Rectangle*>& rs);


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
