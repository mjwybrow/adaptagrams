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

#include <cfloat>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <limits>

#include "commondefs.h"
#include <libvpsc/pairing_heap.h>
#include <libvpsc/assertions.h>

using namespace std;
namespace shortest_paths {
// O(n^3) time dynamic programming approach.  Slow, but fool proof.  Use for testing.
template <typename T>
void floyd_warshall(
        unsigned const n,
        T** D, 
        vector<Edge> const & es,
        valarray<T> const * eweights) 
{
    COLA_ASSERT(!eweights||eweights->size()==es.size());
    for(unsigned i=0;i<n;i++) {
        for(unsigned j=0;j<n;j++) {
            if(i==j) D[i][j]=0;
            else D[i][j]=numeric_limits<T>::max();
        }
    }
    for(unsigned i=0;i<es.size();i++) {
        unsigned u=es[i].first, v=es[i].second;
        COLA_ASSERT(u<n&&v<n);
        D[u][v]=D[v][u]=eweights?(*eweights)[i]:1;
    }
    for(unsigned k=0; k<n; k++) {
        for(unsigned i=0; i<n; i++) {
            for(unsigned j=0; j<n; j++) {
                D[i][j]=min(D[i][j],D[i][k]+D[k][j]);
            }
        }
    }
}
// Simply returns the adjacency graph
template <typename T>
void neighbours(
        unsigned const n,
        T** D, 
        vector<Edge> const & es,
        valarray<T> const * eweights) 
{
    COLA_ASSERT(!eweights||eweights->size()==es.size());
    for(unsigned i=0;i<n;i++) {
        for(unsigned j=0;j<n;j++) {
            D[i][j]=0;
        }
    }
    for(unsigned i=0;i<es.size();i++) {
        unsigned u=es[i].first, v=es[i].second;
        COLA_ASSERT(u<n&&v<n);
        D[u][v]=D[v][u]=eweights?(*eweights)[i]:1;
    }
}
template <typename T>
void dijkstra_init(
        vector<Node<T> > & vs, 
        vector<Edge> const& es, 
        valarray<T> const* eweights) {
    COLA_ASSERT(!eweights||eweights->size()==es.size());
#ifndef NDEBUG
    const unsigned n=vs.size();
#endif
    for(unsigned i=0;i<es.size();i++) {
        unsigned u=es[i].first, v=es[i].second;
        COLA_ASSERT(u<n);
        COLA_ASSERT(v<n);
        T w=eweights?(*eweights)[i]:1;
        vs[u].neighbours.push_back(&vs[v]);
        vs[u].nweights.push_back(w);
        vs[v].neighbours.push_back(&vs[u]);
        vs[v].nweights.push_back(w);
    }
}
template <typename T>
void dijkstra(
        unsigned const s,
        vector<Node<T> > & vs,
        T* d)
{
    const unsigned n=vs.size();
    COLA_ASSERT(s<n);
    for(unsigned i=0;i<n;i++) {
        vs[i].id=i;
        vs[i].d=numeric_limits<T>::max();
        vs[i].p=NULL;
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
            if(u->d!=numeric_limits<T>::max()
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
        vector<Edge> const & es,
        valarray<T> const * eweights)
{
    COLA_ASSERT(!eweights||es.size()==eweights->size());
    COLA_ASSERT(s<n);
    vector<Node<T> > vs(n);
    dijkstra_init(vs,es,eweights);
    dijkstra(s,vs,d);
}

template <typename T>
void johnsons(
        unsigned const n,
        T** D, 
        vector<Edge> const & es,
        valarray<T> const * eweights) 
{
    vector<Node<T> > vs(n);
    dijkstra_init(vs,es,eweights);
    for(unsigned k=0;k<n;k++) {
        dijkstra(k,vs,D[k]);
    }
}
}
