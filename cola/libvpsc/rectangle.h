/**
 * \brief Functions to automatically generate constraints for the
 * rectangular node overlap removal problem.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
#ifndef SEEN_LIBVPSC_RECTANGLE_H
#define SEEN_LIBVPSC_RECTANGLE_H
#include <iostream>
#include <vector>
#include <cassert>

namespace vpsc {
enum Dim { HORIZONTAL, VERTICAL };
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
	friend std::ostream& operator <<(std::ostream &os, const Rectangle &r);
public:
	static double xBorder,yBorder;
	Rectangle(double x, double X, double y, double Y,
			bool allowOverlap=false);
	Rectangle(Rectangle const &Other)   // magmy20070406: Added copy constructor
   :  minX(Other.minX)
   ,  maxX(Other.maxX)
   ,  minY(Other.minY)
   ,  maxY(Other.maxY)
   ,  overlap(Other.overlap) {
   }
	double getMaxX() const { return maxX+xBorder; }
	double getMaxY() const { return maxY+yBorder; }
	double getMinX() const { return minX; }
	double getMinY() const { return minY; }
	double getMinD(unsigned const d) const {
		return ( d == 0 ? getMinX() : getMinY() );
	}
	double getMaxD(unsigned const d) const {
		return ( d == 0 ? getMaxX() : getMaxY() );
	}
	double getCentreX() const { return minX+width()/2.0; }
	double getCentreY() const { return minY+height()/2.0; }
    double getCentreD(unsigned const d) const {
        return getMinD(d)+length(d)/2.0;
    }
	double width() const { return getMaxX()-minX; }
	double height() const { return getMaxY()-minY; }
	double length(unsigned const d) const {
		return ( d == 0 ? width() : height() );
	}
	void set_width(double w) { maxX = minX + w; }
	void set_height(double h) { maxY = minY + h; }
	static void setXBorder(double x) {xBorder=x;}
	static void setYBorder(double y) {yBorder=y;}
	void moveCentreD(const unsigned d, double p) {
		if(d == 0) {
			moveCentreX(p);
		} else {
			moveCentreY(p);
		}
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
		maxX=x+width()-xBorder;
		minX=x;
	}
	void moveMinY(double y) {
		maxY=y+height()-yBorder;
		minY=y;
	}
	double overlapX(Rectangle *r) const {
		if (getCentreX() <= r->getCentreX() && r->minX < getMaxX())
			return getMaxX() - r->minX;
		if (r->getCentreX() <= getCentreX() && minX < r->getMaxX())
			return r->getMaxX() - minX;
		return 0;
	}
	double overlapY(Rectangle *r) const {
		if (getCentreY() <= r->getCentreY() && r->minY < getMaxY())
			return getMaxY() - r->minY;
		if (r->getCentreY() <= getCentreY() && minY < r->getMaxY())
			return r->getMaxY() - minY;
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
		return x>minX && x<maxX && y>minY && y<maxY;
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
			assert(1==2);
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


class Variable;
class Constraint;

// returns number of constraints generated
void generateXConstraints(std::vector<Rectangle*> const & rs, std::vector<Variable*> const & vars, std::vector<Constraint*> & cs, const bool useNeighbourLists);
void generateYConstraints(std::vector<Rectangle*> const & rs, std::vector<Variable*> const & vars, std::vector<Constraint*> & cs);

}

#endif // SEEN_LIBVPSC_RECTANGLE_H
