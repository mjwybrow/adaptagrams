// vim: set cindent 
// vim: ts=4 sw=4 et tw=0 wm=0

#include <boost/config.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
//#define TEST_AGAINST_BOOST
#ifdef TEST_AGAINST_BOOST
#include <boost/property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
using namespace boost;
#endif // TEST_AGAINST_BOOST
#include <shortest_paths.h>
#include <cmath>
#include <time.h>

using namespace std;
// creates a (not necessarily connected random graph) with n nodes.
vector<shortest_paths::Edge> random_graph(unsigned n) {
	vector<shortest_paths::Edge> edges;
	for(unsigned i=0;i<n;i++) {
		for(unsigned j=i+1;j<n;j++) {
			double r=(double)rand()/(double)RAND_MAX;
			if(r < 0.1) {
				edges.push_back(make_pair(i,j));
			}
		}
	}

	return edges;
}
clock_t lastTime;
static void resetClock() {
    lastTime=clock();
}
static double getRunTime() {
    clock_t time = clock()-lastTime;
    return (double)time/(double)CLOCKS_PER_SEC;
}
int
main()
{
    bool dump=false;
    srand(time(0));
#ifdef TEST_AGAINST_BOOST
    typedef adjacency_list<vecS, vecS, undirectedS, no_property,
      property< edge_weight_t, double, property< edge_weight2_t, double > > > Graph;
    Graph g;
#endif
    unsigned V = 400;
    vector<shortest_paths::Edge> es = random_graph(V);
    unsigned E=es.size();
    cout << "  Test graph |V|="<<V<<",|E|="<<E<<endl;
    double weights[E];
    for(unsigned i=0;i<E;i++) {
	    weights[i]=round(((double)rand()/(double)RAND_MAX)*10);
#ifdef TEST_AGAINST_BOOST
	    add_edge(es[i].first,es[i].second,weights[i],g);
#endif
    }

#ifdef TEST_AGAINST_BOOST
    vector < double >d(V, (numeric_limits < double >::max)());
    typedef vector<double> weight_vec;
    vector<weight_vec> D(V,weight_vec(V));
    cout<<"Running boost::johnson_all_pairs_shortest_paths..."<<endl;
    resetClock();
    johnson_all_pairs_shortest_paths(g, D, distance_map(&d[0]));
    cout<<"  ...done, time="<<getRunTime()<<endl;
#endif
    double** D1=new double*[V];
    for(unsigned i=0;i<V;i++) {
	    D1[i]=new double[V];
    }
    cout<<"Running shortest_paths::johnsons..."<<endl;
    resetClock();
    shortest_paths::johnsons(V,D1,es,weights);
    cout<<"  ...done, time="<<getRunTime()<<endl;
    double** D2=new double*[V];
    for(unsigned i=0;i<V;i++) {
	    D2[i]=new double[V];
    }
    cout<<"Running shortest_paths::floyd_warshall..."<<endl;
    resetClock();
    shortest_paths::floyd_warshall(V,D2,es,weights);
    cout<<"  ...done, time="<<getRunTime()<<endl;

    for (int i = 0; i < V; ++i) {
        if(dump) cout << i << " -> ";
        for (int j = 0; j < V; ++j) {
	        if(dump) cout << setw(5) << D1[i][j];
	        assert(D1[i][j]==D2[i][j]);
#ifdef TEST_AGAINST_BOOST
	        assert(D[i][j]==D2[i][j]);
#endif
        }
        if(dump) cout << endl;
    }
#ifdef TEST_AGAINST_BOOST
    if(dump) {
        ofstream fout("figs/johnson-eg.dot");
        fout << "graph A {\n" << "node[shape=\"circle\"]\n";

        graph_traits < Graph >::edge_iterator ei, ei_end;
        for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
            fout << source(*ei, g) << " -- " << target(*ei, g)
                 << "[label=" << get(edge_weight, g)[*ei] << "]\n";

        fout << "}\n";
    }
#endif
    return 0;
}
