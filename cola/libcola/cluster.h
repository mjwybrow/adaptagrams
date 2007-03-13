#ifndef COLA_CLUSTER_H
#define COLA_CLUSTER_H
#include "commondefs.h"
namespace cola {
using vpsc::Rectangle;
using vpsc::Variable;
using std::vector;
using std::valarray;

class Cluster;

typedef vector<Cluster*> Clusters;

// A cluster defines a hierarchical partitioning over the nodes
// which should be kept disjoint by the layout somehow
class Cluster {
public:
    double varWeight;
    double internalEdgeWeightFactor;
    vector<unsigned> nodes;
    Clusters clusters;
    valarray<double> hullX, hullY;
    Cluster();
    void clear();    
    virtual ~Cluster();
    void updatePosition();
    virtual void computeBoundary(vector<Rectangle*> const & rs) = 0;
    vpsc::Rectangle getMinRect( const Dim dim, Rectangle const & bounds);
    vpsc::Rectangle getMaxRect( const Dim dim, Rectangle const & bounds);
    void computeBoundingRect(vector<Rectangle*> const & rs);
    Rectangle bounds;
    void createVars(const Dim dim, vector<Rectangle*> const & rs, vector<Variable*>& vars);
    Variable *vXMin, *vXMax, *vYMin, *vYMax;
    void generateNonOverlapConstraints(
            const Dim dim,
            const NonOverlapConstraints nonOverlapConstraints,
            vector<Rectangle*> const& rs,
            vector<Variable*> const& vars,
            vector<vpsc::Constraint*> & cs);
private:
    Variable *vMin, *vMax;
    double length;
};
class RectangularCluster : public Cluster {
public:
    void computeBoundary(vector<Rectangle*> const & rs);
};
class RootCluster : public Cluster {
public:
    void computeBoundary(vector<Rectangle*> const & rs);
};
class ConvexCluster : public Cluster {
public:
    void computeBoundary(vector<Rectangle*> const & rs);
};
} // namespace cola

#endif // COLA_CLUSTER_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
