// vim: set cindent 
// vim: ts=4 sw=4 et tw=0 wm=0
#include <vector>
#include <valarray>
template <class T>
class PairNode;

namespace shortest_paths {
using namespace std;

struct Node {
    unsigned id;
    double d;
    Node* p; // predecessor    
    vector<Node*> neighbours;
    vector<double> nweights;
    PairNode<Node*>* qnode;
};
inline bool compareNodes(Node *const &u, Node *const &v) {
	return u->d < v->d;
}

typedef pair<unsigned,unsigned> Edge;
void floyd_warshall(unsigned n, double** D,
        vector<Edge>& es,valarray<double>* eweights=NULL); 

/*
 * find all pairs shortest paths
 * n: total number of nodes
 * D: n*n matrix of shortest paths
 * es: edge pairs
 * eweights: edge weights
 */
void johnsons(unsigned n, double** D,
        vector<Edge>& es, valarray<double>* eweights=NULL);
/*
 * find shortest path lengths from node s to all other nodes
 * s: starting node
 * n: total number of nodes
 * d: n vector of path lengths
 * es: edge pairs
 * eweights: weight of each edge
 */
void dijkstra(unsigned s, unsigned n, double* d, 
        vector<Edge>& es, valarray<double>* eweights=NULL);
}
