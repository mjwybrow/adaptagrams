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
#include "graphlayouttest.h"

using namespace boost;
using namespace std;

/** 
* \brief Determines when to terminate layout of a particular graph based
* on a given relative tolerance. 
*/
int main() {

	const int V = 5;
	typedef std::pair < int, int >Edge;
	Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(3, 4) };
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

	Cluster c;
	c.push_back(0);
	c.push_back(4);
	Clusters cs;
	cs.push_back(&c);

	constrained_majorization_layout(g,position,weightmap,
			side_length(width),&cs);

	output_svg(g,position,"containment.svg");
}
