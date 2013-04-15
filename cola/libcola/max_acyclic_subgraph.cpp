/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006  Kieran Simpson
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

/* Implements the maximum acyclic subgraph algorithm 
*/

#include <iostream>
#include <stack>
#include <vector>

#include "libvpsc/assertions.h"
#include "libcola/commondefs.h"
#include "libcola/max_acyclic_subgraph.h"

using namespace std;
using namespace cola;
using namespace max_acyclic_subgraph;

MaxAcyclicSubgraph::MaxAcyclicSubgraph(unsigned numVertices, Edges *edges)  {
  this->V = numVertices; 
  this->edges = edges;
  nodes = NULL;

  // make the adjacency matrix
  this->make_matrix();
  COLA_ASSERT(nodes->size() == this->V);
}

MaxAcyclicSubgraph::~MaxAcyclicSubgraph()  {
  if (nodes != NULL)  {
    for (unsigned i = 0; i < nodes->size(); i++)  { if ((*nodes)[i] != NULL)  { delete (*nodes)[i]; } }
    delete nodes;
  }
}

void MaxAcyclicSubgraph::make_matrix()  {
  Edges::iterator ei;
  Edge anEdge;

  if (nodes != NULL)  {
    for (unsigned i = 0; i < nodes->size(); i++)  { if ((*nodes)[i] != NULL)  { delete (*nodes)[i]; } }
    delete nodes;
  }

  nodes = new NodeList(this->V, NULL);
  COLA_ASSERT(!nodes->empty());

  // from the edges passed, fill the adjacency matrix
  for (ei = edges->begin(); ei != edges->end(); ei++)  {
    anEdge = *ei;
    // the matrix is indexed by the first vertex of the edge
    // the second vertex of the edge is pushed onto another
    // vector as in incoming or outgoing edge
    #ifdef ADJMAKE_DEBUG
      cout << "vertex1: " << anEdge.first << ", vertex2: " << anEdge.second << endl;
    #endif

    // see if we have found the first vertex before
    if (!find_node(anEdge.first))  {
      #ifdef ADJMAKE_DEBUG
        cout << "Making a new vector indexed at: " << anEdge.first << endl;
      #endif

      (*nodes)[anEdge.first] = new Node(anEdge.first);
    }
    
    // see if we have found the second vertex before
    if (!find_node(anEdge.second))  {
      #ifdef ADJMAKE_DEBUG
    cout << "Making a new vector indexed at: " << anEdge.second << endl;
      #endif

      (*nodes)[anEdge.second] = new Node(anEdge.second);
    }
    
    // store this edge as an outgoing edge for the first vertex
    (*nodes)[anEdge.first]->outgoing.push_back(anEdge);
      
    // store this edge as an incoming edge for the second vertex
    (*nodes)[anEdge.second]->incoming.push_back(anEdge);
  }

  // the following block is code to print out
  // the adjacency matrix.
  #ifdef ADJMAKE_DEBUG
    cout << "ADJ MATRIX: " << endl;
    printNodes(nodes);
  #endif
}

Edges *MaxAcyclicSubgraph::find_subgraph()  {
  Edges *Ea = new Edges;
  stack< NodeList::iterator > toRemove;
  NodeList *copy = copy_graph();
  Node *node = NULL;

  COLA_ASSERT(!copy->empty());
  COLA_ASSERT(!edges->empty());

  #ifdef COPY_ADJ_DEBUG
    cout << "COPY OF MATRIX: " << endl;
    printNodes(copy);
  #endif

  // while the graph is not empty
  while (!copy->empty())  {
    COLA_ASSERT(toRemove.empty());

    // do we have any sinks
    for (NodeList::iterator ni = copy->begin(); ni != copy->end(); ni++)  {
      // is is a sink if there are no outgoing edges
      node = *ni;
      if (node->outgoing.empty())  {
        #ifdef RUN_DEBUG
      cout << "vertex(" << node->id << ") is a SINK" << endl;
    #endif
    // append it's incoming edges to Ea
    for (unsigned j = 0; j < node->incoming.size(); j++)  {
      #ifdef RUN_DEBUG
        cout << "Appending to Ea: Edge(" << node->incoming[j].first << ", " << node->incoming[j].second << ")" << endl;
      #endif
      Ea->push_back(node->incoming[j]);

      // find the edge from a vertex where the edge is outgoing
      Node *out = NULL;
      for (unsigned q = 0; q < copy->size(); q++)  {
        if ((*copy)[q]->id == node->incoming[j].first)  { out = (*copy)[q]; }
          }

          COLA_ASSERT(out != NULL);

      #ifdef RUN_DEBUG
        cout << "Searching through OUTGOING list for vertex(" << out->id << ")" << endl;
      #endif

          Edges::iterator oi;
      for (oi = out->outgoing.begin(); oi != out->outgoing.end(); oi++)  {
            cola::Edge e = *oi;
        #ifdef RUN_DEBUG
          cout << "Looking at Edge(" << e.first << ", " << e.second << ")" << endl;
          #endif

        if (e == node->incoming[j])  { break; }
      }

      #ifdef RUN_DEBUG
        cout << "Erasing Edge(" << (*oi).first << ", " << (*oi).second << ") from OUTGOING list of vertex(" << out->id << ")" << endl;
      #endif          
          out->outgoing.erase(oi);
    }

    // say that we want to remove this vertex from the graph.
    toRemove.push(ni);
      }
    }

    // remove all necessary vertices
    while (!toRemove.empty())  { copy->erase(toRemove.top()); toRemove.pop(); } COLA_ASSERT(toRemove.empty());

    #ifdef EA_DEBUG
      cout << "EA: ";
      for (unsigned i = 0; i < Ea->size(); i++)  {
        cout << "(" << (*Ea)[i].first << ", " << (*Ea)[i].second << ") ";
      }
      cout << endl;
    #endif
    
    #ifdef RUN_DEBUG
      cout << "COPY OF MATRIX (after SINKS removed): " << endl;
      printNodes(copy);
    #endif

    // do we have any isolated vertices
    for (NodeList::iterator ni = copy->begin(); ni != copy->end(); ni++)  {
      // is is an isolated vertice if there are no incoming or outgoing edges
      node = *ni;
      if (node->incoming.empty() && node->outgoing.empty())  {
        #ifdef RUN_DEBUG
      cout << "vertex(" << node->id << ") is ISOLATED" << endl;
    #endif

    // say that we want to remove this vertex from the graph.
    toRemove.push(ni);
      }
    }

    // remove all necessary vertices
    while (!toRemove.empty())  { copy->erase(toRemove.top()); toRemove.pop(); } COLA_ASSERT(toRemove.empty());

    #ifdef EA_DEBUG
      cout << "EA: ";
      for (unsigned i = 0; i < Ea->size(); i++)  {
        cout << "(" << (*Ea)[i].first << ", " << (*Ea)[i].second << ") ";
      }
      cout << endl;
    #endif

    #ifdef RUN_DEBUG
      cout << "COPY OF MATRIX (after isolated vertices removed): " << endl;
      printNodes(copy);
    #endif

    // do we have any sources
    for (NodeList::iterator ni = copy->begin(); ni != copy->end(); ni++)  {
      // is is a sink if there are no incoming edges
      node = *ni;
      if (node->incoming.empty())  {
        #ifdef RUN_DEBUG
      cout << "vertex(" << node->id << ") is a SOURCE" << endl;
    #endif
    // append it's outgoing edges to Ea
    for (unsigned j = 0; j < node->outgoing.size(); j++)  {
      #ifdef RUN_DEBUG
        cout << "Appending to Ea: Edge(" << node->outgoing[j].first << ", " << node->outgoing[j].second << ")" << endl;
      #endif
      Ea->push_back(node->outgoing[j]);

      // find the edge from a vertex where the edge is incoming
      Node *in = NULL;
      for (unsigned q = 0; q < copy->size(); q++)  {
        if ((*copy)[q]->id == node->outgoing[j].second)  { in = (*copy)[q]; }
          }

          COLA_ASSERT(in != NULL);

      #ifdef RUN_DEBUG
        cout << "Searching through INCOMING list for vertex(" << in->id << ")" << endl;
      #endif

          Edges::iterator ii;
      for (ii = in->incoming.begin(); ii != in->incoming.end(); ii++)  {
            cola::Edge e = *ii;
        #ifdef RUN_DEBUG
          cout << "Looking at Edge(" << e.first << ", " << e.second << ")" << endl;
          #endif

        if (e == node->outgoing[j])  { break; }
      }

      #ifdef RUN_DEBUG
        cout << "Erasing Edge(" << (*ii).first << ", " << (*ii).second << ") from INCOMING list of vertex(" << in->id << ")" << endl;
      #endif          
          in->incoming.erase(ii);
    }

    // say that we want to remove this vertex from the graph.
    toRemove.push(ni);
      }
    }

    // remove all necessary vertices
    while (!toRemove.empty())  { copy->erase(toRemove.top()); toRemove.pop(); } COLA_ASSERT(toRemove.empty());

    #ifdef EA_DEBUG
      cout << "EA: ";
      for (unsigned i = 0; i < Ea->size(); i++)  {
        cout << "(" << (*Ea)[i].first << ", " << (*Ea)[i].second << ") ";
      }
      cout << endl;
    #endif

    #ifdef RUN_DEBUG
      cout << "COPY OF MATRIX (after SOURCES removed): " << endl;
      printNodes(copy);
    #endif

    // if the graph is not empty
    if (!copy->empty())  {
      // find the vertex with the highest degree of "source"
      int degree = -1000;
      NodeList::iterator theNode;

      for (NodeList::iterator ni = copy->begin(); ni != copy->end(); ni++)  {
        node = *ni;
    
    int t = node->outgoing.size() - node->incoming.size();

    if (t > degree)  {
      #ifdef RUN_DEBUG
        cout << "Sourceiest node: " << node->id << "(d:" << degree << ", t:" << t << ")" << endl;
      #endif

      degree = t;
      theNode = ni; 
    }
      }

      // add this node's outgoing edges to Ea
      node = *theNode;

      for (unsigned j = 0; j < node->outgoing.size(); j++)  {
    #ifdef RUN_DEBUG
      cout << "Appending to Ea: Edge(" << node->outgoing[j].first << ", " << node->outgoing[j].second << ")" << endl;
    #endif
    Ea->push_back(node->outgoing[j]);

    // find the edge from a vertex where the edge is incoming
    Node *in = NULL;
    for (unsigned q = 0; q < copy->size(); q++)  {
      if ((*copy)[q]->id == node->outgoing[j].second)  { in = (*copy)[q]; }
        }

        COLA_ASSERT(in != NULL);

    #ifdef RUN_DEBUG
      cout << "Searching through INCOMING list for vertex(" << in->id << ")" << endl;
    #endif

        Edges::iterator ii;
    for (ii = in->incoming.begin(); ii != in->incoming.end(); ii++)  {
          cola::Edge e = *ii;
      #ifdef RUN_DEBUG
        cout << "Looking at Edge(" << e.first << ", " << e.second << ")" << endl;
        #endif

      if (e == node->outgoing[j])  { break; }
    }

    #ifdef RUN_DEBUG
      cout << "Erasing Edge(" << (*ii).first << ", " << (*ii).second << ") from INCOMING list of vertex(" << in->id << ")" << endl;
    #endif          
        in->incoming.erase(ii);
      }

      // for all of the incoming edges this node possesses, delete then from other node's outgoing edge list
      for (unsigned j = 0; j < node->incoming.size(); j++)  {
    // find the edge from a vertex where the edge is outgoing
    Node *out = NULL;
    for (unsigned q = 0; q < copy->size(); q++)  {
      if ((*copy)[q]->id == node->incoming[j].first)  { out = (*copy)[q]; }
        }

        COLA_ASSERT(out != NULL);

    #ifdef RUN_DEBUG
      cout << "Searching through OUTGOING list for vertex(" << out->id << ")" << endl;
    #endif

        Edges::iterator oi;
    for (oi = out->outgoing.begin(); oi != out->outgoing.end(); oi++)  {
          cola::Edge e = *oi;
      #ifdef RUN_DEBUG
        cout << "Looking at Edge(" << e.first << ", " << e.second << ")" << endl;
        #endif

      if (e == node->incoming[j])  { break; }
    }

        #ifdef RUN_DEBUG
      cout << "Erasing Edge(" << (*oi).first << ", " << (*oi).second << ") from OUTGOING list of vertex(" << out->id << ")" << endl;
    #endif          
        out->outgoing.erase(oi);
      }

      // delete this vertex
      copy->erase(theNode);
    }

    #ifdef EA_DEBUG
      cout << "EA: ";
      for (unsigned i = 0; i < Ea->size(); i++)  {
        cout << "(" << (*Ea)[i].first << ", " << (*Ea)[i].second << ") ";
      }
      cout << endl;
    #endif

    #ifdef RUN_DEBUG
      cout << "COPY OF MATRIX (after SOURCIEST node removed): " << endl;
      printNodes(copy);
    #endif
  }

  // delete the copy
  if (copy != NULL)  {
    for (unsigned i = 0; i < copy->size(); i++)  { if ((*copy)[i] != NULL)  { delete (*copy)[i]; } }
    delete copy;
  }

  #ifdef EA_DEBUG
    cout << "Returning EA: ";
    for (unsigned i = 0; i < Ea->size(); i++)  {
      cout << "(" << (*Ea)[i].first << ", " << (*Ea)[i].second << ") ";
    }
    cout << endl;
  #endif

  return Ea;
}

void MaxAcyclicSubgraph::mod_graph(unsigned numVertices, Edges *edges)  {
  this->V = numVertices;
  this->edges = edges;
  // remake the adjaceny matrix
  this->make_matrix();
  COLA_ASSERT(nodes->size() == this->V);
}

bool MaxAcyclicSubgraph::find_node(unsigned k)  {
  for (unsigned i = 0; i < this->V; i++)  {
    if ((*nodes)[i] != NULL)  {
      if ((*nodes)[i]->id == k)  { return true; }
    }
  }

  return false;
}

NodeList *MaxAcyclicSubgraph::copy_graph()  {
  // create a new vector copied from the old one
  NodeList *copy = new NodeList((*nodes));

  // for each node pointed to be the old nodes vector
  // create a new node and store
  for (unsigned i = 0; i < nodes->size(); i++)  {
    Node *currentNode, *newNode;
 
    currentNode = (*nodes)[i];
    newNode = new Node((*currentNode));
    (*copy)[i] = newNode;
  }

  // return the copy
  return copy;
}

void MaxAcyclicSubgraph::printNodes(NodeList*& n)  {
  for (unsigned i = 0; i < n->size(); i++)  {
    Node *node = (*n)[i];
    cout << "nodes[" << node->id << "]: ";
      
    // printout the incoming edges for this node
    cout << "INCOMING{";
    for (unsigned j = 0; j < node->incoming.size(); j++)  { cout << "(" << node->incoming[j].first << ", " << node->incoming[j].second << ") "; }
      
    cout << "} OUTGOING{";
    for (unsigned j = 0; j < node->outgoing.size(); j++)  { cout << "(" << node->outgoing[j].first << ", " << node->outgoing[j].second << ") "; }
    cout << "}" << endl;
  }

  cout << endl;
}
