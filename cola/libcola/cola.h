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
#include "straightener.h"
namespace vpsc { class Rectangle; }

namespace cola {
using vpsc::Rectangle;
using std::vector;
typedef std::pair<unsigned, unsigned> Edge;
typedef vector<unsigned> Cluster;
typedef vector<Cluster*> Clusters;

// a graph component with a list of node_ids giving indices for some larger list of nodes
// for the nodes in this component, and a list of edges - node indices relative to this component
class Component {
public:
    vector<unsigned> node_ids;
    vector<Rectangle*> rects;
    vector<Edge> edges;
    SimpleConstraints scx, scy;
    ~Component();
    void moveRectangles(double x, double y);
    Rectangle* getBoundingBox();
};
// for a graph of n nodes, return connected components
void connectedComponents(
    const vector<Rectangle*> &rs,
    const vector<Edge> &es,
    const SimpleConstraints &scx,
    const SimpleConstraints &scy, 
    vector<Component*> &components);

// move the contents of each component so that the components do not
// overlap.
void separateComponents(const vector<Component*> &components);

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
	
class TestConvergence {
public:
    double old_stress;
    TestConvergence(const double tolerance = 0.001, const unsigned maxiterations = 1000)
        : tolerance(tolerance),
          maxiterations(maxiterations) { reset(); }
    virtual ~TestConvergence() {}

    virtual bool operator()(const double new_stress, valarray<double> const & X, valarray<double> const & Y) {
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
        double const idealLength,
        std::valarray<double> const * eweights=NULL,
        TestConvergence& done=defaultTest);

    void moveBoundingBoxes() {
        for(unsigned i=0;i<lapSize;i++) {
            boundingBoxes[i]->moveCentreX(X[i]);
            boundingBoxes[i]->moveCentreY(Y[i]);
        }
    }

    void setupConstraints(
        AlignmentConstraints* acsx, AlignmentConstraints* acsy,
        bool avoidOverlaps, 
        PageBoundaryConstraints* pbcx = NULL,
        PageBoundaryConstraints* pbcy = NULL,
        SimpleConstraints* scx = NULL,
        SimpleConstraints* scy = NULL,
        Clusters* cs = NULL,
        vector<straightener::Edge*>* straightenEdges = NULL,
	double bendWeight = 0.01, double potBendWeight = 0.1);

    void addLinearConstraints(LinearConstraints* linearConstraints);

    void setupDummyVars();

    ~ConstrainedMajorizationLayout() {
        if(boundingBoxes) {
            delete [] boundingBoxes;
        }
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
    bool avoidOverlaps;
    bool constrainedLayout;
private:
    double euclidean_distance(unsigned i, unsigned j) {
        return sqrt(
            (X[i] - X[j]) * (X[i] - X[j]) +
            (Y[i] - Y[j]) * (Y[i] - Y[j]));
    }
    double compute_stress(valarray<double> const & Dij);
    void majlayout(valarray<double> const & Dij,GradientProjection* gp, valarray<double>& coords);
    unsigned n; // is lapSize + dummyVars
    unsigned lapSize; // lapSize is the number of variables for actual nodes
    valarray<double> lap2; // graph laplacian
    valarray<double> Q; // quadratic terms matrix used in computations
    valarray<double> Dij; // all pairs shortest path distances
    double tol;
    TestConvergence& done;
    Rectangle** boundingBoxes;
    valarray<double> X, Y;
    Clusters* clusters;
    double edge_length;
    LinearConstraints *linearConstraints;
    GradientProjection *gpX, *gpY;
    vector<straightener::Edge*>* straightenEdges;
    SimpleConstraints *scx, *scy;
	AlignmentConstraints *acsx, *acsy;
    
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