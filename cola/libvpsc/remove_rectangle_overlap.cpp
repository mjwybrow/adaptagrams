/**
 * \brief remove overlaps between a set of rectangles.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#include <iostream>
#include <cassert>
#include <vector>
#include "rectangle.h"
#include "solve_VPSC.h"
#include "variable.h"
#include "constraint.h"
#ifdef LIBVPSC_LOGGING
#include <fstream>
#include "blocks.h"
using std::ios;
using std::ofstream;
using std::endl;
#endif

#define EXTRA_GAP 0.0001
using namespace vpsc;

double Rectangle::xBorder=0;
double Rectangle::yBorder=0;
/**
 * Takes an array of n rectangles and moves them as little as possible
 * such that rectangles are separated by at least xBorder horizontally
 * and yBorder vertically
 *
 * Works in three passes: 
 * 1) removes some overlap horizontally
 * 2) removes remaining overlap vertically
 * 3) a last horizontal pass removes all overlap starting from original
 *    x-positions - this corrects the case where rectangles were moved 
 *    too much in the first pass.
 */
void removeRectangleOverlap(unsigned n, std::vector<Rectangle*> const & rs, double xBorder, double yBorder) {
	try {
	// The extra gap avoids numerical imprecision problems
	Rectangle::setXBorder(xBorder+EXTRA_GAP);
	Rectangle::setYBorder(yBorder+EXTRA_GAP);
	std::vector<Variable *> vs(n);
	for(unsigned i=0;i<n;i++) {
		vs[i]=new Variable(i,rs[i]->getCentreX(),1);
	}
	std::vector<Constraint *> cs;
	double *oldX = new double[n];
	generateXConstraints(rs,vs,cs,true);
	for(unsigned i=0;i<n;i++) {
		oldX[i]=vs[i]->desiredPosition;
	}
	Solver vpsc_x(vs,cs);
#ifdef LIBVPSC_LOGGING
	ofstream f(LOGFILE,ios::app);
	f<<"Calling VPSC: Horizontal pass 1"<<endl;
	f.close();
#endif
	vpsc_x.solve();
	for(unsigned i=0;i<n;i++) {
		rs[i]->moveCentreX(vs[i]->finalPosition);
	}
	for(unsigned i = 0; i < cs.size(); ++i) {
		delete cs[i];
	}
	cs.clear();
	// Removing the extra gap here ensures things that were moved to be adjacent to
	// one another above are not considered overlapping
	Rectangle::setXBorder(Rectangle::xBorder-EXTRA_GAP);
	for(unsigned i=0;i<n;i++) {
		vs[i]->desiredPosition=rs[i]->getCentreY();
	}
	generateYConstraints(rs,vs,cs);
	Solver vpsc_y(vs,cs);
#ifdef LIBVPSC_LOGGING
	f.open(LOGFILE,ios::app);
	f<<"Calling VPSC: Vertical pass"<<endl;
	f.close();
#endif
	vpsc_y.solve();
	for(unsigned i=0;i<n;i++) {
		rs[i]->moveCentreY(vs[i]->finalPosition);
		rs[i]->moveCentreX(oldX[i]);
	}
	delete [] oldX;
	for(unsigned i = 0; i < cs.size(); ++i) {
		delete cs[i];
	}
	cs.clear();
	Rectangle::setYBorder(Rectangle::yBorder-EXTRA_GAP);
	for(unsigned i=0;i<n;i++) {
		vs[i]->desiredPosition=rs[i]->getCentreY();
	}
	generateXConstraints(rs,vs,cs,false);
	Solver vpsc_x2(vs,cs);
#ifdef LIBVPSC_LOGGING
	f.open(LOGFILE,ios::app);
	f<<"Calling VPSC: Horizontal pass 2"<<endl;
	f.close();
#endif
	vpsc_x2.solve();
	for(unsigned i = 0; i < cs.size(); ++i) {
		delete cs[i];
	}
	for(unsigned i=0;i<n;i++) {
		rs[i]->moveCentreX(vs[i]->finalPosition);
		delete vs[i];
	}
	} catch (char const *str) {
		std::cerr<<str<<std::endl;
		for(unsigned i=0;i<n;i++) {
			std::cerr << *rs[i]<<std::endl;
		}
	}
}
