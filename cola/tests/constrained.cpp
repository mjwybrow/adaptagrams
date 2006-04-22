/** \file
 * Interface between Inkscape code (SPItem) and graphlayout functions.
 */
/*
* Authors:
*   Tim Dwyer <tgdwyer@gmail.com>
*
* Copyright (C) 2005 Authors
*
* Released under GNU GPL.  Read the file 'COPYING' for more information.
*/
#include <iostream>
#include <fstream>

#include <cola.h>
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <map>
#include <vector>
#include <algorithm>
#include <float.h>
#include <iomanip>

using namespace boost;
using namespace std;

/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
bool check (double delta_p, Vertex p, const Graph& g, bool global) {
	static layout_tolerance<double> t;
	cout << "*";
	return t(delta_p,p,g,global);
}
int main() {

	const int V = 4;
	typedef std::pair < int, int >Edge;
	Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(1, 3) };
	const std::size_t E = sizeof(edge_array) / sizeof(Edge);
	Graph g(edge_array, edge_array + E, V);
	WeightMap weightmap=get(edge_weight, g);
	graph_traits < Graph >::edge_iterator ei, ei_end;
	for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) weightmap[*ei] = 1.0;
	double width=100;
	double height=100;
	Position<>::Vec position_vec(num_vertices(g));
	IndexMap index = get(vertex_index, g);
	Position<>::Map position(position_vec.begin(), index);
  	write_graphviz(cout, g);
	circle_graph_layout(g, position, width/2.0);

	AlignmentConstraints acsx;
	AlignmentConstraint ac(1);
	acsx.push_back(&ac);
	Offset o1(0,0), o2(3,0);
	ac.offsets.push_back(&o1);
	ac.offsets.push_back(&o2);

	constrained_majorization_layout(g,position,weightmap,
			side_length(width),&acsx,NULL);

	ofstream f("constrained.svg");
	f.setf(ios::fixed);
	int r=5;
	graph_traits<Graph>::vertex_iterator vi, vi_end;
	double xmin=numeric_limits<double>::max(), ymin=xmin;
	double xmax=-numeric_limits<double>::max(), ymax=xmax;
	for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
		double x=position[*vi].x, y=position[*vi].y;
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
		f<<"<line x1=\""<<position[u].x
		 <<"\" y1=\""<<position[u].y
		 <<"\" x2=\""<<position[v].x
		 <<"\" y2=\""<<position[v].y
		 <<"\" style=\"stroke:rgb(99,99,99);stroke-width:2\"/>"<<endl;
	}
	for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
		f<<"<g id=\"node"<<*vi<<"\" class=\"node\"><title>"<<*vi<<"</title>"<<endl;
		f<<"<ellipse cx=\""<<position[*vi].x<<"\" cy=\""<<position[*vi].y
		 <<"\" rx=\""<<r<<"\" ry=\""<<r<<"\" style=\"fill:black;stroke:black;\"/>"
		 <<endl<<"</g>"<<endl;

	}
	f<<"</svg>"<<endl;
}
