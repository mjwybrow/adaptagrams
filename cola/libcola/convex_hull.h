// vim: set cindent 
// vim: ts=4 sw=4 et tw=0 wm=0
#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H
#include <vector>
#include <valarray>

void convexHull(std::valarray<double> const & X, std::valarray<double> const & Y, std::vector<unsigned> & hull);

#endif // CONVEX_HULL_H
