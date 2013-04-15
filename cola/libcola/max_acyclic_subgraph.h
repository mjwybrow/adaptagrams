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
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

#ifndef MAX_ACYCLIC_SUBGRAPH_H
#define MAX_ACYCLIC_SUBGRAPH_H

#include <iostream>
#include <map>
#include <vector>
#include <stack>
#include "libcola/cola.h"

namespace max_acyclic_subgraph  {  
  typedef std::vector<cola::Edge> Edges;

  class Node  {
    public:
      unsigned id;
      Edges incoming;
      Edges outgoing;
      Node(unsigned id)  { this->id = id; }
    
      // copy constructor
      Node(const Node &old)  {
        #ifdef COPY_CONS_DEBUG
          std::cout << "Node copy constructor(" << old.id << ")" << endl;
        #endif
  
        this->id = old.id;
        this->incoming = old.incoming;
        this->outgoing = old.outgoing;
      }
  
      ~Node()  {}
  };

  typedef std::vector<Node *> NodeList;

  class MaxAcyclicSubgraph  {
    public:
      MaxAcyclicSubgraph(unsigned numVertices, Edges *edges);
      ~MaxAcyclicSubgraph();
      Edges *find_subgraph();
      void mod_graph(unsigned numVertices, Edges *edges);
      unsigned getV()  { return this->V; }
      Edges *getEdges()  { return this->edges; }

    private:
      // attributes
      unsigned V;
      Edges *edges;

      // internally used variables.
      NodeList *nodes; // the nodes in the graph

      // internally used methods
      void make_matrix();
      bool find_node(unsigned k);
      NodeList *copy_graph();
      void printNodes(NodeList*& n);
  };
}

#endif
