/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2005-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
*/

/**
 * \brief Functions to automatically generate constraints for the
 * rectangular node overlap removal problem.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 */
#ifndef SEEN_LIBVPSC_RECTANGLE_H
#define SEEN_LIBVPSC_RECTANGLE_H
#include <iostream>
#include <vector>
#include <set>
#include <cassert>
#include <cmath>

namespace vpsc {
enum Dim { 
    HORIZONTAL = 0,
    XDIM = 0,
    VERTICAL = 1,
    YDIM = 1
};
inline Dim conjugate(Dim d) {
	return static_cast<Dim>(!d);
}
/* records the positions and sides through which a particular line intersects with a rectangle
 */
struct RectangleIntersections {
    bool intersects, top, bottom, left, right;
    double topX, topY, bottomX, bottomY, leftX, leftY, rightX, rightY;
    RectangleIntersections()
        : intersects(false),top(false),bottom(false),left(false),right(false),
      topX(0),topY(0),bottomX(0),bottomY(0),leftX(0),leftY(0),rightX(0),rightY(0) {} 
    int countIntersections() {
        return left+right+top+bottom;
    }
    void print() {
        printf("intersections:\n");
        if(top) printf("  top=%d:(%f,%f)\n",top,topX,topY);
        if(bottom) printf("  bottom=%d:(%f,%f)\n",bottom,bottomX,bottomY);
        if(left) printf("  left=%d:(%f,%f)\n",left,leftX,leftY);
        if(right) printf("  right=%d:(%f,%f)\n",right,rightX,rightY);
    }
    // of the stored intersections, this returns the one closest to the
    // specified point
    void nearest(double x, double y, double & xi, double & yi) {
        bool is[]={top, right, bottom, left};
        double xs[]={topX, rightX, bottomX, leftX};
        double ys[]={topY, rightY, bottomY, leftY};
        double dx, dy, l, minl = 999999999999999.0;
        for(unsigned i=0;i<4;i++) {
        if(is[i]) {
            dx=xs[i]-x;
            dy=ys[i]-y;
            l=dx*dx + dy*dy;
            if(l<minl) {
            minl=l;
            xi=xs[i];
            yi=ys[i];
            }
        }
        }
    }
};
class Rectangle {   
public:
    /**
     * xBorder and yBorder can be set to add a border to the boundary of the
     * rectangle.  In other words, the size of the rectangle returned by the
     * getters (getMinX, getMaxX, etc) will be slightly larger than the
     * internal representation.  This is useful in situations where we need the
     * size considered in one axis to be slightly different to that considered
     * in the other axis for example, to avoid numerical precision problems in
     * the axis-by-axis overlap removal process.
     */
    static double xBorder,yBorder;
    /**
     * @param x minimum horizontal value
     * @param X maximum horizontal value
     * @param y minimum vertical value
     * @param Y maximum vertical value
     * @param allowOverlap not used currently
     */
    Rectangle(double x, double X, double y, double Y,
            bool allowOverlap=false);
    Rectangle(Rectangle const &Other)
        :  minX(Other.minX)
        ,  maxX(Other.maxX)
        ,  minY(Other.minY)
        ,  maxY(Other.maxY)
        ,  overlap(Other.overlap) { }
    Rectangle() {}
    /**
     * reset the dimensions in one axis
     * @param d axis (0==X, 1==Y)
     * @param x min value
     * @param X max value
     */
    void reset(const unsigned d, double x, double X);
    double getMaxX() const { return maxX+xBorder; }
    double getMaxY() const { return maxY+yBorder; }
    double getMinX() const { return minX-xBorder; }
    double getMinY() const { return minY-yBorder; }
    /**
     * @param d axis: 0=horizontal 1=vertical
     */
    double getMinD(unsigned const d) const {
        assert(d==0||d==1);
        return ( d == 0 ? getMinX() : getMinY() );
    }
    /**
     * @param d axis: 0=horizontal 1=vertical
     */
    double getMaxD(unsigned const d) const {
        assert(d==0||d==1);
        return ( d == 0 ? getMaxX() : getMaxY() );
    }
    double getCentreX() const { return getMinX()+width()/2.0; }
    double getCentreY() const { return getMinY()+height()/2.0; }
    /**
     * @param d axis: 0=horizontal 1=vertical
     */
    double getCentreD(unsigned const d) const {
        assert(d==0||d==1);
        return getMinD(d)+length(d)/2.0;
    }
    double width() const { return getMaxX()-getMinX(); }
    double height() const { return getMaxY()-getMinY(); }
    /**
     * @param d axis: 0=width 1=height
     * @return width or height
     */
    double length(unsigned const d) const {
        assert(d==0||d==1);
        return ( d == 0 ? width() : height() );
    }
    void set_width(double w) { maxX = minX + w - 2.0*xBorder; }
    void set_height(double h) { maxY = minY + h - 2.0*yBorder; }
    static void setXBorder(double x) {xBorder=x;}
    static void setYBorder(double y) {yBorder=y;}
    void moveCentreD(const unsigned d, double p) {
        assert(d==0||d==1);
        if(d == 0) { moveCentreX(p);
        } else { moveCentreY(p); }
    }
    void moveCentreX(double x) {
        moveMinX(x-width()/2.0);
    }
    void moveCentreY(double y) {
        moveMinY(y-height()/2.0);
    }
    void moveCentre(double x, double y) {
        moveCentreX(x);
        moveCentreY(y);
    }
    void moveMinX(double x) {
        double w=width();
        minX=x+xBorder;
        maxX=x+w-xBorder;
        assert(fabs(width()-w)<1e-9);
    }
    void moveMinY(double y) {
        double h=height();
        maxY=y+h-yBorder;
        minY=y+yBorder;
        assert(fabs(height()-h)<1e-9);
    }
    double overlapD(const unsigned d, Rectangle* r) {
        if(d==0) {
            return overlapX(r);
        } else {
            return overlapY(r);
        }
    }
    double overlapX(Rectangle *r) const {
        double ux=getCentreX(), vx=r->getCentreX();
        if (ux <= vx && r->getMinX() < getMaxX())
            return getMaxX() - r->getMinX();
        if (vx <= ux && getMinX() < r->getMaxX())
            return r->getMaxX() - getMinX();
        return 0;
    }
    double overlapY(Rectangle *r) const {
        double uy=getCentreY(), vy=r->getCentreY();
        if (uy <= vy && r->getMinY() < getMaxY()) {
            return getMaxY() - r->getMinY();
        }
        if (vy <= uy && getMinY() < r->getMaxY()) {
            return r->getMaxY() - getMinY();
        }
        return 0;
    }
    bool allowOverlap() {
        return overlap;
    }
    void offset(double dx, double dy) {
        minX += dx;
        maxX += dx;
        minY += dy;
        maxY += dy;
    }
    // returns the intersections between the line segment from (x1,y1) 
    // to (x2,y2) and this rectangle.  Any intersections points with
    // sides are reported, lines coincident with a side are considered not
    // to intersect.
    void lineIntersections(double x1, double y1, double x2, double y2, RectangleIntersections &ri) const;
    bool inside(double x, double y) const {
        return x>getMinX() && x<getMaxX() && y>getMinY() && y<getMaxY();
    }
    // checks if line segment is strictly overlapping.
    // That is, if any point on the line is inside the rectangle.
    bool overlaps(double x1, double y1, double x2, double y2) {
        RectangleIntersections ri;
        lineIntersections(x1,y1,x2,y2,ri);
        if(ri.intersects) {
            if(ri.countIntersections()==1) {
                // special case when one point is touching
                // the boundary of the rectangle but no part
                // of the line is interior
                if(!inside(x1,y1)&&!inside(x2,y2)) {
                    return false;
                }
            }
            printf("Rectangle/Segment intersection!\n");
            printf("Line[{{%f,%f},{%f,%f}}],\n",x1,y1,x2,y2);
            printf("Rectangle[{%f,%f},{%f,%f}]\n",getMinX(),getMinY(),getMaxX(),getMaxY());
            ri.print();
            return true;
        }
        return false;
    }
    // p1=(x1,y1),p2=(x2,y2) are points on the boundary.  Puts the shortest
    // path round the outside of the rectangle  from p1 to p2 into xs, ys.
    void routeAround(double x1, double y1, double x2, double y2,
            std::vector<double> &xs, std::vector<double> &ys);
private:
    double minX,maxX,minY,maxY;
    bool overlap;
};

typedef std::vector<Rectangle*> Rectangles;
std::ostream& operator<<(std::ostream& os, vpsc::Rectangle const &r);

class Variable;
class Constraint;

void generateXConstraints(const Rectangles& rs, std::vector<Variable*> const & vars, std::vector<Constraint*> & cs, const bool useNeighbourLists);
void generateYConstraints(std::vector<Rectangle*> const & rs, std::vector<Variable*> const & vars, std::vector<Constraint*> & cs);

/** 
 * Moves all the rectangles to remove all overlaps.  Heuristic
 * attempts to move by as little as possible.
 * no overlaps guaranteed.
 * @param rs the rectangles which will be moved to remove overlap
 */
void removeoverlaps(Rectangles& rs);
/** 
 * Moves rectangles to remove all overlaps.  A heuristic
 * attempts to move by as little as possible.  The heuristic is
 * that the overlaps are removed horizontally and then vertically,
 * each pass being a quadratic program in which the total squared movement
 * is minimised subject to non-overlap constraints.  An optional third
 * horizontal pass (in addition to the first horizontal pass and the second
 * vertical pass) can be applied wherein the x-positions of rectangles are reset to their
 * original positions and overlap removal repeated.  This may avoid some
 * unnecessary movement. 
 * @param rs the rectangles which will be moved to remove overlap
 * @param fixed a set of indices to rectangles which should not be moved
 * @param thirdPass optionally run the third horizontal pass described above.
 */
void removeoverlaps(Rectangles& rs, const std::set<unsigned>& fixed, bool thirdPass=true);

#ifndef NDEBUG
bool assertNoOverlaps(const Rectangles& rs);
#endif

struct delete_object
{
    template <typename T>
    void operator()(T *ptr){ delete ptr;}
};

} // namespace vpsc

#endif // SEEN_LIBVPSC_RECTANGLE_H
// vim: cindent ts=4 sw=4 et tw=0 wm=0
