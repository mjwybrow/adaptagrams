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

#include <iostream>
#include <vector>
#include <set>
#include <libvpsc/rectangle.h>
#include <libcola/cola.h>
#include <libcola/connected_components.h>
#include "graphlayouttest.h"

using namespace std;
using namespace cola;

int main() {
	const unsigned V = 7;
	unsigned c1[]={0,1,2,3};
	set<unsigned> expected_c1(c1,c1+4);
	unsigned c2[]={4,5,6};
	set<unsigned> expected_c2(c2,c2+3);

	Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(3, 2), Edge(1, 3),
        		      Edge(4, 5), Edge(5, 6), Edge(6, 4)	};
	const std::size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(edge_array,edge_array+E);
	vector<Component*> cs;
	vpsc::Rectangles rs;
	double width=100,height=100;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+5,y,y+5));
	}
	connectedComponents(rs,es,cs);
	set<unsigned> result_c1(cs[0]->node_ids.begin(),cs[0]->node_ids.end());
	set<unsigned> result_c2(cs[1]->node_ids.begin(),cs[1]->node_ids.end());
	assert(expected_c1==result_c1);
	assert(expected_c2==result_c2);
	for(unsigned i=0;i<cs.size();i++) {
		printf("Component %d:\n",i);
		for(unsigned j=0;j<cs[i]->edges.size();j++) {
			Edge& e=cs[i]->edges[j];
			printf("(%d,%d) ",e.first,e.second);
		}
		cout << endl;
	}
	return 0;
}
