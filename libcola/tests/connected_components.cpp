#include <iostream>
#include <vector>
#include <set>
#include <libcola/cola.h>

using namespace std;
using cola::Edge;

int main() {
	const unsigned V = 7;
	unsigned c1[]={0,1,2,3};
	set<unsigned> expected_c1(c1,c1+4);
	unsigned c2[]={4,5,6};
	set<unsigned> expected_c2(c2,c2+3);

	Edge edge_array[] = { Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(1, 3),
        		      Edge(4, 5), Edge(5, 6), Edge(6, 4)	};
	const std::size_t E = sizeof(edge_array) / sizeof(Edge);
	vector<Edge> es(E);
	copy(edge_array,edge_array+E,es.begin());
	vector< vector<unsigned>* > cs;
	cola::connectedComponents(V,es,cs);
	set<unsigned> result_c1(cs[0]->begin(),cs[0]->end());
	set<unsigned> result_c2(cs[1]->begin(),cs[1]->end());
	assert(expected_c1==result_c1);
	assert(expected_c2==result_c2);
	return 0;
}
