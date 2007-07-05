// vim: set cindent 
// vim: ts=4 sw=4 et tw=0 wm=0
#ifndef SHORTEST_PATHS_H
#define SHORTEST_PATHS_H
#include <vector>
#include <valarray>
template <class T>
struct PairNode;        // magmy20070405: Changed from class to struct to match actual definition

namespace shortest_paths {
using namespace std;

template <typename T>
struct Node {
    unsigned id;
    T d;
    Node* p; // predecessor    
    vector<Node*> neighbours;
    vector<T> nweights;
    PairNode<Node*>* qnode;
};
template <typename T>
struct CompareNodes {
    bool operator() (Node<T> *const &u, Node<T> *const &v) const {
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
template <typename T>
void floyd_warshall(unsigned const n, T** D,
        vector<Edge> const & es,valarray<T> const * eweights=NULL); 

/*
 * find all pairs shortest paths, faster, uses dijkstra
 * n: total number of nodes
 * D: n*n matrix of shortest paths
 * es: edge pairs
 * eweights: edge weights, if NULL then all weights will be taken as 1
 */
template <typename T>
void johnsons(unsigned const n, T** D,
        vector<Edge> const & es, valarray<T> const * eweights=NULL);
/*
 * find shortest path lengths from node s to all other nodes
 * s: starting node
 * n: total number of nodes
 * d: n vector of path lengths
 * es: edge pairs
 * eweights: edge weights, if NULL then all weights will be taken as 1
 */
template <typename T>
void dijkstra(unsigned const s, unsigned const n, T* d, 
        vector<Edge> const & es, valarray<T> const * eweights=NULL);

} //namespace shortest_paths

#include "shortest_paths.cpp"

#endif //SHORTEST_PATHS_H
