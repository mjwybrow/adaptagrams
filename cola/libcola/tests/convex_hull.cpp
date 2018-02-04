/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
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
 * Really basic regression test of convex hull implementation
 * declared in libcola/convex_hull.h
 */
#include <valarray>
#include <algorithm>
#include <libcola/convex_hull.h>
#include "graphlayouttest.h"

#include <string>
#include <iostream>
#include <cairomm/context.h>
#include <cairomm/surface.h>

/* M_PI is defined in math.h in the case of Microsoft Visual C++ */
#if defined(_MSC_VER)
#define _USE_MATH_DEFINES
#include <math.h>
#endif 
using namespace std;

typedef vector<unsigned> Hull;
/**
 * generates a random set of n points in X and Y.
 */
void randTest(unsigned n, valarray<double>& X, valarray<double>& Y) {
    X.resize(n);
    Y.resize(n);
    srand(time(nullptr));
	for(unsigned i=0;i<n;i++) {
		X[i]=getRand(1.);
		Y[i]=getRand(1.);
	}
}
/**
 * generates a set of 8 points (actually the vertices of two rectangles)
 * which lineup horizontally.  The expected hull are the lower-left and
 * top-left corners of the left rectangle and the top-right/lower-right
 * corners of the right rectangle.
 */
void tworects(valarray<double>& X, valarray<double>& Y, Hull& expectedHull) {
    const unsigned n=8;
    X.resize(n);
    Y.resize(n);
    X[0]=330.011898, Y[0]=203.250425;
    X[1]=330.011898, Y[1]=237.250425;
    X[2]=276.011898, Y[2]=237.250425;
    X[3]=276.011898, Y[3]=203.250425;
    X[4]=459.998300, Y[4]=203.250425;
    X[5]=459.998300, Y[5]=237.250425;
    X[6]=405.998300, Y[6]=237.250425;
    X[7]=405.998300, Y[7]=203.250425;
    unsigned hull[]={3,4,5,2};
    unsigned m=sizeof(hull)/sizeof(unsigned);
    expectedHull.resize(m);
    copy(hull,hull+m,expectedHull.begin());
}

int drawCairo(const string& fname,
        const valarray<double>& X, const valarray<double>& Y, 
        const Hull& hull);

int main(int argc, char** argv) {
	valarray<double> X, Y;
	Hull h,expectedHull;
    tworects(X,Y,expectedHull);
    hull::convex(X,Y,h);
    printf("hull size=%d\n",h.size());
    pair<Hull::iterator,Hull::iterator> r
        =mismatch(h.begin(),h.end(),expectedHull.begin());
    assert(r.first==h.end());
    drawCairo("convex_tworects.svg",X,Y,h);

    randTest(20,X,Y);
    hull::convex(X,Y,h);
    drawCairo("convex_hull_random.svg",X,Y,h);
    return 0;
}

/***********CAIRO CODE***************************************************/
double width = 600;
double height = 400;
double border=10;
void dot(Cairo::RefPtr<Cairo::Context> & cr, double x, double y) {
    cr->arc(x, y, 
            2., 0.0, 2.0 * M_PI);
    cr->stroke();
}

double xcoord(double x) {
    return border+x*width;
}
double ycoord(double y) {
    return border+y*height;
}
int drawCairo(const string& fname,
        const valarray<double>& Xin, const valarray<double>& Yin, 
        const Hull& hull) {
#ifdef CAIRO_HAS_SVG_SURFACE
    unsigned n=Xin.size();
    assert(Yin.size()==n);

    // normalise coords to range 0-1
    valarray<double> X=Xin, Y=Yin;
    X-=X.min();
    Y-=Y.min();
    X/=X.max();
    Y/=Y.max();

    Cairo::RefPtr<Cairo::SvgSurface> surface =
        Cairo::SvgSurface::create(fname, width+2*border, height+2*border);

    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

    cr->save(); // save the state of the context
    cr->set_source_rgba(0.0, 0.0, 0.0, 0.7);
    // draw a circle at each coordinate
    for(unsigned i=0;i<n;i++) {
        dot(cr,xcoord(X[i]),ycoord(Y[i]));
    }

    cr->set_source_rgba(0.0, 0.0, 0.0, 0.3);
    cr->move_to(xcoord(X[hull[0]]),ycoord(Y[hull[0]]));
    for(unsigned i=1;i<hull.size();i++) {
        cr->line_to(xcoord(X[hull[i]]),ycoord(Y[hull[i]]));
    }
    cr->line_to(xcoord(X[hull[0]]),ycoord(Y[hull[0]]));
    cr->stroke();
    cr->set_source_rgba(0.0, 0.0, 0.0, 1.);
    for(vector<unsigned>::const_iterator i=hull.begin();i!=hull.end();++i) {
        unsigned j=*i;
        stringstream ss;
        ss<<j;
        printf("p[%d]=(%f,%f)\n",j,X[j],Y[j]);
        cr->move_to(xcoord(X[j]),ycoord(Y[j]));
        cr->show_text(ss.str());
        cr->stroke();
    }
    cr->restore();

    cr->show_page();

    cout << "Wrote SVG file \"" << fname << "\"" << endl;
    return 0;

#else

    cout << "You must compile cairo with SVG support for this example to work."
        << endl;
    return 1;

#endif

}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
