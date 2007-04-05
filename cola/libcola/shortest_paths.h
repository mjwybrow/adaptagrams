// vim: set cindent 
// vim: ts=4 sw=4 et tw=0 wm=0
#include <vector>
#include <valarray>
template <class T>
struct PairNode;        // magmy20070405: Changed from class to struct to match actual definition

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
struct CompareNodes {
    bool operator() (Node *const &u, Node *const &v) const {
        if(u==v) return false; // with g++ 4.1.2 unless I have this explicit check
                               // it returns true for this case when using -O3 optimization
                               // CRAZY!
        if(u->d < v->d) {
            return true;
        } 
        return false;
    }
};

typedef pair<unsigned,unsigned> Edge;
/*
 * find all pairs shortest paths, n^3 dynamic programming approach
 * n: total number of nodes
 * D: n*n matrix of shortest paths
 * es: edge pairs
 * eweights: edge weights, if NULL then all weights will be taken as 1
 */
void floyd_warshall(unsigned const n, double** D,
        vector<Edge> const & es,valarray<double> const * eweights=NULL); 

/*
 * find all pairs shortest paths, faster, uses dijkstra
 * n: total number of nodes
 * D: n*n matrix of shortest paths
 * es: edge pairs
 * eweights: edge weights, if NULL then all weights will be taken as 1
 */
void johnsons(unsigned const n, double** D,
        vector<Edge> const & es, valarray<double> const * eweights=NULL);
/*
 * find shortest path lengths from node s to all other nodes
 * s: starting node
 * n: total number of nodes
 * d: n vector of path lengths
 * es: edge pairs
 * eweights: edge weights, if NULL then all weights will be taken as 1
 */
void dijkstra(unsigned const s, unsigned const n, double* d, 
        vector<Edge> const & es, valarray<double> const * eweights=NULL);
}
