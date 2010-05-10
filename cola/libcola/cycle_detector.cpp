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

/* Cycle detector that returns a list of 
 * edges involved in cycles in a digraph.
*/

#include <iostream>
#include <stack>
#include <vector>
#include <cassert>

#include "libvpsc/assertions.h"
#include "commondefs.h"
#include "cycle_detector.h"


using namespace std;
using namespace cola;
using namespace cycle_detector;

// a global var representing time
TimeStamp Time;

CycleDetector::CycleDetector(unsigned numVertices, Edges *edges)  {
  this->V = numVertices; 
  this->edges = edges;
  nodes = NULL;

  // make the adjacency matrix
  this->make_matrix();
  COLA_ASSERT(nodes->size() == this->V);
}

CycleDetector::~CycleDetector()  {
  if (nodes != NULL)  {
    for (unsigned i = 0; i < nodes->size(); i++)  { if ((*nodes)[i] != NULL)  { delete (*nodes)[i]; } }
    delete nodes;
  }
}

void CycleDetector::make_matrix()  {
  Edges::iterator ei;
  Edge anEdge;
  Node *newNode;

  if (nodes != NULL)  {
    for (unsigned i = 0; i < nodes->size(); i++)  { if ((*nodes)[i] != NULL)  { delete (*nodes)[i]; } }
    delete nodes;
  }

  nodes = new vector<Node *>(this->V, NULL);
  traverse.clear();

  // we should not have an empty array
  COLA_ASSERT(!nodes->empty());
  COLA_ASSERT(traverse.empty());

  // from the edges passed, fill the adjacency matrix
  for (ei = edges->begin(); ei != edges->end(); ei++)  {
    anEdge = *ei;
    // the matrix is indexed by the first vertex of the edge
    // the second vertex of the edge is pushed onto another
    // vector of all vertices connected to the first vertex
    // with a directed edge
    #ifdef ADJMAKE_DEBUG
      cout << "vertex1: " << anEdge.first << ", vertex2: " << anEdge.second << endl;
    #endif
    if (!find_node(nodes, anEdge.first))  {
      #ifdef ADJMAKE_DEBUG
        cout << "Making a new vector indexed at: " << anEdge.first << endl;
      #endif

      newNode = new Node(anEdge.first);
      newNode->dests.push_back(anEdge.second);
      (*nodes)[anEdge.first] = newNode;
    }
    else  {
      (*nodes)[anEdge.first]->dests.push_back(anEdge.second);
    }

    // check if the destination vertex exists in the nodes map
    if (!find_node(nodes, anEdge.second))  {
      #ifdef ADJMAKE_DEBUG
        cerr << "Making a new vector indexed at: " << anEdge.second << endl;
      #endif

      newNode = new Node(anEdge.second);
      (*nodes)[anEdge.second] = newNode;
    }
  }

  COLA_ASSERT(!nodes->empty());

  // the following block is code to print out
  // the adjacency matrix.
  #ifdef ADJMAKE_DEBUG
    for (unsigned i = 0; i < nodes->size(); i++)  {
      Node *node = (*nodes)[i];
      cout << "nodes[" << node->id << "]: ";
      
      if (isSink(node))  { cout << "SINK"; }
      else  {
        for (unsigned j = 0; j < node->dests.size(); j++)  { cout << node->dests[j] << " "; }
      }
      cout << endl;
    }
  #endif
}

vector<bool> *CycleDetector::detect_cycles()  {
  cyclicEdgesMapping = new vector<bool>(edges->size(), false);

  COLA_ASSERT(!nodes->empty());
  COLA_ASSERT(!edges->empty());

  // make a copy of the graph to ensure that we have visited all
  // vertices
  traverse.clear(); COLA_ASSERT(traverse.empty());
  for (unsigned i = 0; i < V; i++)  { traverse.push_back(i); }
  #ifdef SETUP_DEBUG
    for (unsigned i = 0; i < traverse.size(); i++)  {
      cout << "traverse{" << i << "}: " << traverse[i] << endl;
    }
  #endif

  // find the cycles
  COLA_ASSERT(nodes->size() > 1);

  // while we still have vertices to visit, visit.
  while (!traverse.empty())  {
    #ifdef VISIT_DEBUG
      cout << "begining search at vertex(" << traverse[0] << ")" << endl;
    #endif

    Time = 0;

    // go go go
    visit(traverse[0]);
  }

  // clean up
  COLA_ASSERT(traverse.empty());

  return cyclicEdgesMapping;
}

void CycleDetector::mod_graph(unsigned numVertices, Edges *edges)  {
  this->V = numVertices;
  this->edges = edges;
  // remake the adjaceny matrix
  this->make_matrix();
  COLA_ASSERT(nodes->size() == this->V);
}

void CycleDetector::visit(unsigned k)  {
  unsigned cycleOpen;
  Node *thisNode = (*nodes)[k];
  Node *highestCA = NULL;

  // state that we have seen this vertex
  pair< bool, vector<unsigned>::iterator > haveSeen = find_node(traverse, k);
  if (haveSeen.first)  {
    // set up this node as being visited.
    thisNode->stamp = ++Time;
    thisNode->status = Node::BeingVisited;

    traverse.erase(haveSeen.second);

    #ifdef VISIT_DEBUG
      cout << "Visiting vertex(" << k << ") for the first time TS: " << thisNode->stamp << endl;
    #endif

  }

  // traverse to all the vertices adjacent to this vertex.
  for (unsigned n = 0; n < thisNode->dests.size(); n++)  {
    Node *otherNode = (*nodes)[thisNode->dests[n]];

    if (otherNode->status != Node::DoneVisiting)  {
      if (otherNode->status == Node::NotVisited)  {  
        // visit this node
    #ifdef VISIT_DEBUG
          cout << "traversing from vertex(" << k << ") to vertex(" << otherNode->id << ")" << endl;
    #endif
        visit(otherNode->id);
      }

      // if we are part of a cycle get the timestamp of the ancestor
      if (otherNode->cyclicAncestor != NULL)  { cycleOpen = otherNode->cyclicAncestor->stamp; }
      // else just get the timestamp of the node we just visited
      else  { cycleOpen = otherNode->stamp; }

      // compare the stamp of the traversal with our stamp
      if (cycleOpen <= thisNode->stamp)  {
    if (otherNode->cyclicAncestor == NULL)  { otherNode->cyclicAncestor = otherNode; }

    // store the cycle
    for (unsigned i = 0; i < edges->size(); i++)  {
      if ((*edges)[i] == Edge(k, otherNode->id))  {
        (*cyclicEdgesMapping)[i] = true;
            #ifdef VISIT_DEBUG
              cout << "Setting cyclicEdgesMapping[(" << k << ", " << otherNode->id << ")] to true" << endl;
            #endif
      }
    }

        // this node is part of a cycle
        if (thisNode->cyclicAncestor == NULL)  { 
      #ifdef VISIT_DEBUG
            cout << "setting vertex(" << thisNode->id << ") cyclic ancestor to vertex(" << otherNode->cyclicAncestor->id << ")" << endl;
      #endif
      
      thisNode->cyclicAncestor = otherNode->cyclicAncestor;
    }

    // see if we are part of a cycle with a cyclicAncestor that possesses a lower timestamp
    if (otherNode->cyclicAncestor->stamp < thisNode->cyclicAncestor->stamp)  { 
      #ifdef VISIT_DEBUG
            cout << "setting vertex(" << thisNode->id << ") cyclic ancestor to vertex(" << otherNode->cyclicAncestor->id << ")" << endl;
      #endif

      thisNode->cyclicAncestor = otherNode->cyclicAncestor;
    }
      }
    }
    else  {
      // we are at a node who has been visited before. 
      #ifdef VISIT_DEBUG
        cout << "Encountering vertex(" << otherNode->id << ") which is CLOSED" << endl;
      #endif

      if (otherNode->cyclicAncestor != NULL)  {
        // get the highest possible cyclic ancestor from this node
    highestCA = this->get_highest_ca(otherNode->cyclicAncestor);
        COLA_ASSERT(highestCA != NULL);

        #ifdef VISIT_DEBUG
          cout << "highest cyclic ancestor at vertex(" << highestCA->id << ")" << endl;
        #endif

    // compare the stamp of the traversal with our stamp
        if (highestCA->stamp <= thisNode->stamp && highestCA->status != Node::DoneVisiting)  {
      #ifdef VISIT_DEBUG
            cout << "Edge(" << k << ", " << otherNode->id << ") is part of a cycle" << endl;
          #endif 
        // store the cycle
      for (unsigned i = 0; i < edges->size(); i++)  {
        if ((*edges)[i] == Edge(k, otherNode->id))  {
          (*cyclicEdgesMapping)[i] = true;
              #ifdef VISIT_DEBUG
                cout << "Setting cyclicEdgesMapping[" << i << "] to true" << endl;
              #endif
        }
      }

          // this node is part of a cycle
          if (thisNode->cyclicAncestor == NULL)  { thisNode->cyclicAncestor = highestCA; }

      // reassign the other node to point to the highest cyclic ancestor
      otherNode->cyclicAncestor = highestCA;
    }
      }
    }
  }

  #ifdef RUN_DEBUG
    cout << "Marking vertex(" << k << ") as CLOSED" << endl;
  #endif

  thisNode->status = Node::DoneVisiting;
}

// finds the cyclic ancestor in the chain with the earliest timestamp
Node *CycleDetector::get_highest_ca(Node *n)  {
  #ifdef GET_HIGHEST_CA_DEBUG
    cout << "At vertex(" << n->id << ")" << endl;
  #endif

  COLA_ASSERT(n->cyclicAncestor != NULL);

  if (n == n->cyclicAncestor)  { 
    // we have reached the end of the chain
    #ifdef GET_HIGHEST_CA_DEBUG
      cout << "Highest CA in chain vertex(" << n->id << ")" << endl;
    #endif

    return n;
  } 
  else  {
    // recurse
    #ifdef GET_HIGHEST_CA_DEBUG
      cout << "Recusing to next chain vertex(" << n->cyclicAncestor->id << ")" << endl;
    #endif
    return (this->get_highest_ca(n->cyclicAncestor));
  }
}

// determines whether or not a vertex is a sink
bool CycleDetector::isSink(Node *node)  {
  // a vertex is a sink if it has no outgoing edges,
  // or that the adj entry is empty
  if (node->dests.empty())  { return true; }
  else  { return false; }
}

bool CycleDetector::find_node(std::vector<Node *> *& list, unsigned k)  {
  for (unsigned i = 0; i < this->V; i++)  {
    if ((*list)[i] != NULL)  {
      if ((*list)[i]->id == k)  { return true; }
    }
  }

  return false;
}

pair< bool, vector<unsigned>::iterator > CycleDetector::find_node(
        std::vector<unsigned>& list, unsigned k)
{
    COLA_UNUSED(list);

    for (vector<unsigned>::iterator ti = traverse.begin(); ti != traverse.end(); ti++)  {
         if (*ti == k)  { return pair< bool, vector<unsigned>::iterator >(true, ti); }
    }

    return pair< bool, vector<unsigned>::iterator >(false, traverse.end());
}

vector<TimeStamp> *CycleDetector::getTimeStamps()  {
  vector<TimeStamp> *times = new vector<TimeStamp>(nodes->size());

  for (unsigned i = 0; i < nodes->size(); i++)  { (*times)[(*nodes)[i]->id] = (*nodes)[i]->stamp; }

  return times;
}
