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
using std::valarray;
typedef std::pair<unsigned, unsigned> Edge;
class Cluster {
public:
    double margin;
    vector<unsigned> nodes;
    Cluster(const std::size_t n, const unsigned ns[]);
    void computeBoundary(vector<Rectangle*> const & rs,
            valarray<double> & X, valarray<double> & Y) const;
};
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
        Clusters const * cs,
        double const idealLength,
        std::valarray<double> const * eweights=NULL,
        TestConvergence& done=defaultTest);
    /**
     * Horizontal alignment constraints
     */
    void setXAlignmentConstraints(AlignmentConstraints* acsx) {
        constrainedLayout = true;
        this->acsx=acsx;
    }
    /**
     * Vertical alignment constraints
     */
    void setYAlignmentConstraints(AlignmentConstraints* acsy) {
        constrainedLayout = true;
        this->acsy=acsy;
    }
    /**
     * Horizontal distribution constraints
     */
    void setXDistributionConstraints(DistributionConstraints* dcsx) {
        constrainedLayout = true;
        this->dcsx=dcsx;
    }
    /**
     * Vertical distribution constraints
     */
    void setYDistributionConstraints(DistributionConstraints* dcsy) {
        constrainedLayout = true;
        this->dcsy=dcsy;
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
    void setXPageBoundaryConstraints(PageBoundaryConstraints* pbcx) {
        constrainedLayout = true;
        this->pbcx = pbcx;
    }
    void setYPageBoundaryConstraints(PageBoundaryConstraints* pbcy) {
        constrainedLayout = true;
        this->pbcy = pbcy;
    }
    void setXSimpleConstraints(SimpleConstraints* scx) {
        constrainedLayout = true;
        this->scx = scx;
    }
    void setYSimpleConstraints(SimpleConstraints* scy) {
        constrainedLayout = true;
        this->scy = scy;
    }
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
    /**
     * one almighty function call to setup all of the above in one go
     */
    void setupConstraints(
        AlignmentConstraints* acsx, AlignmentConstraints* acsy,
        NonOverlapConstraints avoidOverlaps, 
        PageBoundaryConstraints* pbcx = NULL,
        PageBoundaryConstraints* pbcy = NULL,
        SimpleConstraints* scx = NULL,
        SimpleConstraints* scy = NULL,
        vector<straightener::Edge*>* straightenEdges = NULL,
	double bendWeight = 0.01, double potBendWeight = 0.1);

    void moveBoundingBoxes() {
        for(unsigned i=0;i<n;i++) {
            boundingBoxes[i]->moveCentreX(X[i]);
            boundingBoxes[i]->moveCentreY(Y[i]);
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
    void majlayout(valarray<double> const & Dij,GradientProjection* gp, valarray<double>& coords, int N = -1);
    unsigned n; // number of nodes
    valarray<double> lap2; // graph laplacian
    valarray<double> Q; // quadratic terms matrix used in computations
    valarray<double> Dij; // all pairs shortest path distances
    double tol;
    TestConvergence& done;
    vector<Rectangle*> boundingBoxes;
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
    Clusters const * clusters;
    LinearConstraints *linearConstraints;
    GradientProjection *gpX, *gpY;
    NonOverlapConstraints avoidOverlaps;
    vector<straightener::Edge*>* straightenEdges;
    PageBoundaryConstraints *pbcx, *pbcy;
    SimpleConstraints *scx, *scy;
    AlignmentConstraints *acsx, *acsy;
    DistributionConstraints *dcsx, *dcsy;
    
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
