#include <boost/graph/adjacency_list.hpp>
#include <cola.h>
#include "graphlayouttest.h"
#include <generate-constraints.h>
#include <fstream>

void output_svg(Graph g, vector<Rectangle*>& rs, char *fname) {
	double width, height;
	ofstream f(fname);
	f.setf(ios::fixed);
	int r=5;
	graph_traits<Graph>::vertex_iterator vi, vi_end;
	graph_traits < Graph >::edge_iterator ei, ei_end;
	IndexMap index = get(vertex_index, g);
	double xmin=numeric_limits<double>::max(), ymin=xmin;
	double xmax=-numeric_limits<double>::max(), ymax=xmax;
	for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
		double x=rs[*vi]->getCentreX(), y=rs[*vi]->getCentreY();
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
	f<<"<svg width=\""<<width<<"\" height=\""<<height<<"\" viewBox = \""
	 <<xmin<<" "<<ymin<<" "<<width<<" "<<height<<"\">"<<endl;
    	for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
		Vertex u = index[source(*ei, g)];
		Vertex v = index[target(*ei, g)];
		f<<"<line x1=\""<<rs[u]->getCentreX()
		 <<"\" y1=\""<<rs[u]->getCentreY()
		 <<"\" x2=\""<<rs[v]->getCentreX()
		 <<"\" y2=\""<<rs[v]->getCentreY()
		 <<"\" style=\"stroke:rgb(99,99,99);stroke-width:2\"/>"<<endl;
	}
	for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
		f<<"<g id=\"node"<<*vi<<"\" class=\"node\"><title>"<<*vi<<"</title>"<<endl;
		f<<"<ellipse cx=\""<<rs[*vi]->getCentreX()<<"\" cy=\""<<rs[*vi]->getCentreY()
		 <<"\" rx=\""<<r<<"\" ry=\""<<r<<"\" style=\"fill:black;stroke:black;\"/>"
		 <<endl<<"</g>"<<endl;
	}
	f<<"</svg>"<<endl;
}
