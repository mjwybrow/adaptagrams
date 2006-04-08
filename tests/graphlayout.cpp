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
	std::cout << "*";
	return t(delta_p,p,g,global);
}
int main() {

	Graph g;

	WeightMap weightmap=get(edge_weight, g);

	Vertex a = add_vertex(g);
	Vertex b = add_vertex(g);
	Vertex c = add_vertex(g);
	Vertex d = add_vertex(g);
	Graph::edge_descriptor e; bool inserted;
	tie(e, inserted)=add_edge(a,b,g);
	weightmap[e]=1.0;
	tie(e, inserted)=add_edge(b,c,g);
	weightmap[e]=1.0;
	tie(e, inserted)=add_edge(c,d,g);
	weightmap[e]=1.0;
	tie(e, inserted)=add_edge(b,d,g);
	weightmap[e]=1.0;
	double width=100;
	double height=100;
	std::cout<<"Graph has |V|="<<num_vertices(g)<<" Width="<<width<<" Height="<<height<<std::endl;
	Position<>::Vec position_vec(num_vertices(g));
	Position<>::Map position(position_vec.begin(), get(vertex_index, g));
  	write_graphviz(std::cout, g);
	constrained_majorization_layout(g, position,weightmap);
	circle_graph_layout(g, position, width/2.0);
	kamada_kawai_spring_layout(g, position, weightmap, side_length(width),check );

	graph_traits<Graph>::vertex_iterator vi, vi_end, next;
	tie(vi, vi_end) = vertices(g);
	std::ofstream f("blah.svg");
	f.setf(ios::fixed);
	int r=10;
	f<<"<svg width=\""<<width<<"\" height=\""<<height<<"\" viewBox = \""<<(-width/2-r)<<" "<<(-height/2-r)<<" "<<(width+2*r)<<" "<<(height+2*r)<<"\">"<<std::endl;
	for (next = vi; vi != vi_end; vi = next) {
    		++next;
		f<<"<g id=\"node"<<*vi<<"\" class=\"node\"><title>"<<*vi<<"</title>"<<std::endl;
		f<<"<ellipse cx=\""<<position[*vi].x<<"\" cy=\""<<position[*vi].y<<"\" rx=\"10\" ry=\"10\" style=\"fill:none;stroke:black;\"/>"<<std::endl<<"</g>"<<std::endl;

	}
	f<<"</svg>"<<std::endl;
}
