#ifndef MAX_ACYCLIC_SUBGRAPH_H
#define MAX_ACYCLIC_SUBGRAPH_H

#include <iostream>
#include <map>
#include <vector>
#include <stack>
#include "cola.h"

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
