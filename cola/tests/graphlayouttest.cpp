#include <boost/graph/adjacency_list.hpp>
#include <cola.h>
#include "graphlayouttest.h"
#include <generate-constraints.h>
#include <fstream>
using namespace cola;

void output_svg(vector<Rectangle*>& rs, vector<Edge>& es, char *fname, bool rects) {
	double width, height;
	ofstream f(fname);
	f.setf(ios::fixed);
	double r=5;
	if(rects) r=rs[0]->width()/2;
	double xmin=DBL_MAX, ymin=xmin;
	double xmax=-DBL_MAX, ymax=xmax;
	for (unsigned i=0;i<rs.size();i++) {
		double x=rs[i]->getCentreX(), y=rs[i]->getCentreY();
		xmin=min(xmin,x);
		ymin=min(ymin,y);
		xmax=max(xmax,x);
		ymax=max(ymax,y);
	}
	xmax+=2*r;
	ymax+=2*r;
	xmin-=2*r;
	ymin-=2*r;
	width=xmax-xmin;
	height=ymax-ymin;
	f<<"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""<<width<<"\" height=\""<<height<<"\" viewBox = \""
	 <<xmin<<" "<<ymin<<" "<<width<<" "<<height<<"\">"<<endl;
    	for (unsigned i=0;i<es.size();i++) {
		unsigned u = es[i].first;
		unsigned v = es[i].second;
		f<<"<line x1=\""<<rs[u]->getCentreX()
		 <<"\" y1=\""<<rs[u]->getCentreY()
		 <<"\" x2=\""<<rs[v]->getCentreX()
		 <<"\" y2=\""<<rs[v]->getCentreY()
		 <<"\" style=\"stroke:rgb(99,99,99);stroke-width:2\"/>"<<endl;
	}
	for (unsigned i=0;i<rs.size();i++) {
		f<<"<g id=\"node"<<i
		 <<"\" class=\"node\"><title>"<<i<<"</title>"<<endl;
		if(!rects) {
			f<<"<ellipse cx=\""<<rs[i]->getCentreX()
			 <<"\" cy=\""<<rs[i]->getCentreY()
			 <<"\" rx=\""<<r<<"\" ry=\""<<r
			 <<"\" style=\"fill:black;stroke:black;\"/>";
		} else {
			f<<"<rect x=\""<<rs[i]->getMinX()
			 <<"\" y=\""<<rs[i]->getMinY()
			 <<"\" width=\""<<rs[i]->width()
			 <<"\" height=\""<<rs[i]->height()
			 <<"\" style=\"fill:white;stroke:black;\"/>";
			f<<"<text x=\""<<(rs[i]->getCentreX()-3)
			 <<"\" y=\""<<(rs[i]->getCentreY()+5)
			 <<"\">"<<i
			 <<"</text>"<<endl;

		}
		f <<endl<<"</g>"<<endl;
	}
	f<<"</svg>"<<endl;
}
