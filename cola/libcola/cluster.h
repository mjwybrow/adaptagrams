#ifndef COLA_CLUSTER_H
#define COLA_CLUSTER_H

#include <libvpsc/rectangle.h>
#include <libvpsc/variable.h>
#include "commondefs.h"

namespace cola {


// A cluster defines a hierarchical partitioning over the nodes
// which should be kept disjoint by the layout somehow
class Cluster {
public:
    double varWeight;
    double internalEdgeWeightFactor;
	std::vector<unsigned> nodes;
    std::vector<Cluster*> clusters;
    std::valarray<double> hullX, hullY;
    Cluster();
    virtual ~Cluster() {}
	virtual void computeBoundary(const vpsc::Rectangles& rs) = 0;
	vpsc::Rectangle getMinRect( const vpsc::Dim dim, const vpsc::Rectangle& bounds);
	vpsc::Rectangle getMaxRect( const vpsc::Dim dim, const vpsc::Rectangle& bounds);
    void computeBoundingRect(const vpsc::Rectangles& rs);
	vpsc::Rectangle bounds;
	void createVars(const vpsc::Dim dim, const vpsc::Rectangles& rs, vpsc::Variables& vars);
	vpsc::Variable *vXMin, *vXMax, *vYMin, *vYMax;
    void generateNonOverlapConstraints(
			const vpsc::Dim dim,
            const NonOverlapConstraints nonOverlapConstraints,
            const vpsc::Rectangles& rs,
            const vpsc::Variables& vars,
            vpsc::Constraints & cs);
    void clear();
    /**
     * @return the total area covered by contents of this cluster (not
     * including space between nodes/clusters)
     */
    double area(const vpsc::Rectangles& rs);
private:
	vpsc::Variable *vMin, *vMax;
    double length;
    double border;
};
typedef std::vector<Cluster*> Clusters;
class RectangularCluster : public Cluster {
public:
	void computeBoundary(const vpsc::Rectangles& rs);
};
class RootCluster : public Cluster {
public:
	void computeBoundary(const vpsc::Rectangles& rs);
};
class ConvexCluster : public Cluster {
public:
    void computeBoundary(const vpsc::Rectangles& rs);
    std::valarray<unsigned> hullRIDs;
    std::valarray<unsigned char> hullCorners;
};


} // namespace cola

#endif // COLA_CLUSTER_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
