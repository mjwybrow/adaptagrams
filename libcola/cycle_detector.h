#ifndef CYCLE_DETECTOR_H
#define CYCLE_DETECTOR_H

#include <map>
#include <vector>
#include <cola.h>

using namespace std;
using namespace cola;

typedef unsigned TimeStamp;
typedef vector<cola::Edge> Edges;
typedef vector<bool> CyclicEdges;
class Node;

class CycleDetector  {
  public:
    CycleDetector(unsigned numVertices, Edges *edges);
    ~CycleDetector();
    vector<bool> *detect_cycles();
    void mod_graph(unsigned numVertices, Edges *edges);
    unsigned getV()  { return this->V; }
    Edges *getEdges()  { return this->edges; }

  private:
    // attributes
    unsigned V;
    Edges *edges;

    // internally used variables.
    map<unsigned, Node *> nodes; // the nodes in the graph
    map<cola::Edge, bool> cyclicEdges; // the cyclic edges in the graph.
    map<unsigned, bool> traverse; // nodes still left to visit in the graph

    // internally used methods
    void make_matrix();
    unsigned visit(unsigned k);
    bool isSink(Node *node);
};

class Node  {
  public:
    enum StatusType { NotVisited, BeingVisited, DoneVisiting };

    unsigned id;
    TimeStamp stamp;
    Node *cyclicAncestor;
    vector<unsigned> dests;
    StatusType status;

    Node(unsigned id)  { this->id = id; cyclicAncestor = NULL; status = NotVisited; }
    ~Node()  {}
};

#endif
