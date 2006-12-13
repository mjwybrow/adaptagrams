#include "cola.h"
#include "convex_hull.h"

namespace cola {
    using namespace std;

    Cluster::Cluster(const size_t n, const unsigned nodeList[]) : margin(2.) {
        for(unsigned i=0;i<n;i++) {
            nodes.push_back(nodeList[i]);
        }
    }
    void Cluster::computeBoundary(vector<Rectangle*> const & rs) {
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
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
