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

#ifndef SHORTEST_PATHS_H
#define SHORTEST_PATHS_H
#include <vector>
#include <valarray>
template <class T>
struct PairNode;

namespace shortest_paths {

template <typename T>
struct Node {
    unsigned id;
    T d;
    Node* p; // predecessor    
    std::vector<Node<T>*> neighbours;
    std::vector<T> nweights;
    PairNode<Node<T>*>* qnode;
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

typedef std::pair<unsigned,unsigned> Edge;
template <typename T>
/**
 * returns the adjacency matrix, 0 entries for non-adjacent nodes
 * @param n total number of nodes
 * @param D n*n matrix of shortest paths
 * @param es edge pairs
 * @param eweights edge weights, if NULL then all weights will be taken as 1
 */
void neighbours(unsigned const n, T** D,
        std::vector<Edge> const & es,std::valarray<T> const * eweights=NULL); 
/**
 * find all pairs shortest paths, n^3 dynamic programming approach
 * @param n total number of nodes
 * @param D n*n matrix of shortest paths
 * @param es edge pairs
 * @param eweights edge weights, if NULL then all weights will be taken as 1
 */
template <typename T>
void floyd_warshall(unsigned const n, T** D,
        std::vector<Edge> const & es,std::valarray<T> const * eweights=NULL); 

/**
 * find all pairs shortest paths, faster, uses dijkstra
 * @param n total number of nodes
 * @param D n*n matrix of shortest paths
 * @param es edge pairs
 * @param eweights edge weights, if NULL then all weights will be taken as 1
 */
template <typename T>
void johnsons(unsigned const n, T** D,
        std::vector<Edge> const & es, std::valarray<T> const * eweights=NULL);
/**
 * find shortest path lengths from node s to all other nodes
 * @param s starting node
 * @param n total number of nodes
 * @param d n vector of path lengths
 * @param es edge pairs
 * @param eweights edge weights, if NULL then all weights will be taken as 1
 */
template <typename T>
void dijkstra(unsigned const s, unsigned const n, T* d, 
        std::vector<Edge> const & es, std::valarray<T> const * eweights=NULL);

} //namespace shortest_paths

#include "shortest_paths.cpp"

#endif //SHORTEST_PATHS_H
