#include <valarray>
#include <cassert>
#include <float.h>
#include <convex_hull.h>
using namespace std;
struct CounterClockwiseOrder {
    CounterClockwiseOrder(std::valarray<double> const & X, std::valarray<double> const & Y)
        :X(X),Y(Y) {};
    bool operator() (unsigned i, unsigned j) {
        return X[i] < X[j];
    }
    std::valarray<double> const & X; 
    std::valarray<double> const & Y; 
};
/**
 * Implementation of Graham's scan convex hull finding algorithm.
 */
void convex_hull(valarray<double> const & X, valarray<double> const & Y) {
    assert(X.size()==Y.size());
    unsigned n=X.size(),p0;
    // find point p0 with min Y position, choose leftmost in case of tie
    double minY=DBL_MAX,minX=DBL_MAX;
    for(unsigned i=0;i<n;i++) {
        if(Y[i]<minY || Y[i]==minY && X[i]<minX) {
            p0=i;
            minY=Y[i];
            minX=X[i];
        }
    }
    vector<unsigned> stack(n);
    for(unsigned i=0;i<n;i++) { stack[i]=i; }
    CounterClockwiseOrder order(X,Y);
    //sort(stack.begin(),stack.end(),order());
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
