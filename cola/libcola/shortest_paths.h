/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2014  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author(s):  Tim Dwyer
*/

#ifndef SHORTEST_PATHS_H
#define SHORTEST_PATHS_H

#include <vector>
#include <valarray>
#include <cfloat>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <limits>

#include "libcola/commondefs.h"
#include <libvpsc/pairing_heap.h>
#include <libvpsc/assertions.h>

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
 * @param eweights edge weights, if empty then all weights will be taken as 1
 */
void neighbours(unsigned const n, T** D, std::vector<Edge> const & es,
        std::valarray<T> const & eweights = std::valarray<T>()); 
/**
 * find all pairs shortest paths, n^3 dynamic programming approach
 * @param n total number of nodes
 * @param D n*n matrix of shortest paths
 * @param es edge pairs
 * @param eweights edge weights, if empty then all weights will be taken as 1
 */
template <typename T>
void floyd_warshall(unsigned const n, T** D, std::vector<Edge> const & es,
        std::valarray<T> const & eweights = std::valarray<T>()); 

/**
 * find all pairs shortest paths, faster, uses dijkstra
 * @param n total number of nodes
 * @param D n*n matrix of shortest paths
 * @param es edge pairs
 * @param eweights edge weights, if empty then all weights will be taken as 1
 */
template <typename T>
void johnsons(unsigned const n, T** D, std::vector<Edge> const & es,
        std::valarray<T> const & eweights = std::valarray<T>());
/**
 * find shortest path lengths from node s to all other nodes
 * @param s starting node
 * @param n total number of nodes
 * @param d n vector of path lengths
 * @param es edge pairs
 * @param eweights edge weights, if empty then all weights will be taken as 1
 */
template <typename T>
void dijkstra(unsigned const s, unsigned const n, T* d, 
        std::vector<Edge> const & es, 
        std::valarray<T> const & eweights = std::valarray<T>());


//-----------------------------------------------------------------------------
// Implementation:

// O(n^3) time dynamic programming approach.  Slow, but fool proof.  
// Use for testing.
template <typename T>
void floyd_warshall(
        unsigned const n,
        T** D, 
        std::vector<Edge> const & es,
        std::valarray<T> const & eweights) 
{
    COLA_ASSERT((eweights.size() == 0) || (eweights.size() == es.size()));
    for(unsigned i=0;i<n;i++) {
        for(unsigned j=0;j<n;j++) {
            if(i==j) D[i][j]=0;
            else D[i][j]=std::numeric_limits<T>::max();
        }
    }
    for(unsigned i=0;i<es.size();i++) {
        unsigned u=es[i].first, v=es[i].second;
        COLA_ASSERT(u<n&&v<n);
        D[u][v] = D[v][u] = (eweights.size() > 0) ? eweights[i] : 1;
    }
    for(unsigned k=0; k<n; k++) {
        for(unsigned i=0; i<n; i++) {
            for(unsigned j=0; j<n; j++) {
                D[i][j]=std::min(D[i][j],D[i][k]+D[k][j]);
            }
        }
    }
}
// Simply returns the adjacency graph
template <typename T>
void neighbours(
        unsigned const n,
        T** D, 
        std::vector<Edge> const & es,
        std::valarray<T> const & eweights) 
{
    COLA_ASSERT((eweights.size() == 0) || (eweights.size() == es.size()));
    for(unsigned i=0;i<n;i++) {
        for(unsigned j=0;j<n;j++) {
            D[i][j]=0;
        }
    }
    for(unsigned i=0;i<es.size();i++) {
        unsigned u=es[i].first, v=es[i].second;
        COLA_ASSERT(u<n&&v<n);
        D[u][v] = D[v][u] = (eweights.size() > 0) ? eweights[i] : 1;
    }
}
template <typename T>
void dijkstra_init(
        std::vector<Node<T> > & vs, 
        std::vector<Edge> const& es, 
        std::valarray<T> const & eweights) {
    COLA_ASSERT((eweights.size() == 0) || (eweights.size() == es.size()));
#ifndef NDEBUG
    const unsigned n=vs.size();
#endif
    for(unsigned i=0;i<es.size();i++) {
        unsigned u=es[i].first, v=es[i].second;
        COLA_ASSERT(u<n);
        COLA_ASSERT(v<n);
        T w = (eweights.size() > 0) ? eweights[i] : 1;
        vs[u].neighbours.push_back(&vs[v]);
        vs[u].nweights.push_back(w);
        vs[v].neighbours.push_back(&vs[u]);
        vs[v].nweights.push_back(w);
    }
}
template <typename T>
void dijkstra(
        unsigned const s,
        std::vector<Node<T> > & vs,
        T* d)
{
    const unsigned n=vs.size();
    COLA_ASSERT(s<n);
    for(unsigned i=0;i<n;i++) {
        vs[i].id=i;
        vs[i].d=std::numeric_limits<T>::max();
        vs[i].p=nullptr;
    }
    vs[s].d=0;
    PairingHeap<Node<T>*,CompareNodes<T> > Q;
    for(unsigned i=0;i<n;i++) {
        vs[i].qnode = Q.insert(&vs[i]);
    }
    while(!Q.isEmpty()) {
        Node<T> *u=Q.extractMin();
        d[u->id]=u->d;
        for(unsigned i=0;i<u->neighbours.size();i++) {
            Node<T> *v=u->neighbours[i];
            T w=u->nweights[i];
            if(u->d!=std::numeric_limits<T>::max()
               && v->d > u->d+w) {
                v->p=u;
                v->d=u->d+w;
                Q.decreaseKey(v->qnode,v);
            }
        }
    }
}
template <typename T>
void dijkstra(
        unsigned const s,
        unsigned const n,
        T* d,
        std::vector<Edge> const & es,
        std::valarray<T> const & eweights)
{
    COLA_ASSERT((eweights.size() == 0) || (eweights.size() == es.size()));
    COLA_ASSERT(s<n);
    std::vector<Node<T> > vs(n);
    dijkstra_init(vs,es,eweights);
    dijkstra(s,vs,d);
}

template <typename T>
void johnsons(
        unsigned const n,
        T** D, 
        std::vector<Edge> const & es,
        std::valarray<T> const & eweights) 
{
    std::vector<Node<T> > vs(n);
    dijkstra_init(vs,es,eweights);
    for(unsigned k=0;k<n;k++) {
        dijkstra(k,vs,D[k]);
    }
}

} //namespace shortest_paths
#endif //SHORTEST_PATHS_H
