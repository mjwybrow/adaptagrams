#include "cola.h"
#include "convex_hull.h"

namespace cola {
    using namespace std;

    Cluster::Cluster(): margin(2.), internalEdgeWeightFactor(2.) {}

    Cluster::~Cluster() {}

    void ConvexCluster::computeBoundary(vector<Rectangle*> const & rs) {
        valarray<double> X(4*nodes.size());
        valarray<double> Y(X.size());
        unsigned pctr=0;
        for(vector<unsigned>::const_iterator i=nodes.begin(); i!=nodes.end(); i++) {
            vpsc::Rectangle* r=rs[*i];
            X[pctr]=r->getMinX()-margin;
            Y[pctr++]=r->getMinY()-margin;
            X[pctr]=r->getMinX()-margin;
            Y[pctr++]=r->getMaxY()+margin;
            X[pctr]=r->getMaxX()+margin;
            Y[pctr++]=r->getMinY()-margin;
            X[pctr]=r->getMaxX()+margin;
            Y[pctr++]=r->getMaxY()+margin;
        }
        vector<unsigned> hull;
        convexHull(X,Y,hull);
        hullX.resize(hull.size());
        hullY.resize(hull.size());
        for(unsigned i=0;i<hull.size();i++) {
            hullX[i]=X[hull[i]];
            hullY[i]=Y[hull[i]];
        }
    }
    void RectangularCluster::computeBoundary(vector<Rectangle*> const & rs) {
        double minX=DBL_MAX, maxX=-DBL_MAX, minY=DBL_MAX, maxY=-DBL_MAX;
        for(vector<unsigned>::const_iterator i=nodes.begin(); i!=nodes.end(); i++) {
            vpsc::Rectangle* r=rs[*i];
            minX=min(r->getMinX()-margin,minX);
            maxX=max(r->getMaxX()+margin,maxX);
            minY=min(r->getMinY()-margin,minY);
            maxY=max(r->getMaxY()+margin,maxY);
        }
        hullX.resize(4);
        hullY.resize(4);
        hullX[0]=minX;
        hullY[0]=minY;
        hullX[1]=minX;
        hullY[1]=maxY;
        hullX[2]=maxX;
        hullY[2]=maxY;
        hullX[3]=maxX;
        hullY[3]=minY;
    }
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
