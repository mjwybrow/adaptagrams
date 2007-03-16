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

namespace vpsc {
class Rectangle {	
	friend std::ostream& operator <<(std::ostream &os, const Rectangle &r);
public:
	static double xBorder,yBorder;
	Rectangle(double x, double X, double y, double Y,
			bool allowOverlap=false);
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
	double width() const { return getMaxX()-minX; }
	double height() const { return getMaxY()-minY; }
	void set_width(double w) { maxX = minX + w; }
	void set_height(double h) { maxY = minY + h; }
	static void setXBorder(double x) {xBorder=x;}
	static void setYBorder(double y) {yBorder=y;}
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
private:
	double minX,maxX,minY,maxY;
	bool overlap;
};


class Variable;
class Constraint;

// returns number of constraints generated
int generateXConstraints(const int n, std::vector<Rectangle*> const & rs, Variable** vars, Constraint** &cs, const bool useNeighbourLists);
int generateYConstraints(const int n, std::vector<Rectangle*> const & rs, Variable** vars, Constraint** &cs);

}

#endif // SEEN_LIBVPSC_RECTANGLE_H
