#ifndef CONNECTED_COMPONENTS_H
#define CONNECTED_COMPONENTS_H
#include "cola.h"
#include <vector>

namespace cola {
// a graph component with a list of node_ids giving indices for some larger list of nodes
// for the nodes in this component, 
// and a list of edges - node indices relative to this component
class Component {
public:
    vector<unsigned> node_ids;
    vector<Rectangle*> rects;
    vector<Edge> edges;
    CompoundConstraints cx, cy;
    ~Component();
    void moveRectangles(double x, double y);
    Rectangle* getBoundingBox();
};
// for a graph of n nodes, return connected components
void connectedComponents(
    const vector<Rectangle*> &rs,
    const vector<Edge> &es,
    //const CompoundConstraints &cx,
    //const CompoundConstraints &cy, 
    vector<Component*> &components);

// move the contents of each component so that the components do not
// overlap.
void separateComponents(const vector<Component*> &components);

} // namespace cola

#endif // CONNECTED_COMPONENTS_H
