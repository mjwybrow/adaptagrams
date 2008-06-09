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
	std::vector<unsigned> node_ids;
	std::vector<vpsc::Rectangle*> rects;
	std::vector<cola::Edge> edges;
    //CompoundConstraints cx, cy;
    ~Component();
    void moveRectangles(double x, double y);
	vpsc::Rectangle* getBoundingBox();
};
// for a graph of n nodes, return connected components
void connectedComponents(
    const std::vector<vpsc::Rectangle*> &rs,
    const std::vector<cola::Edge> &es,
    //const CompoundConstraints &cx,
    //const CompoundConstraints &cy, 
    std::vector<Component*> &components);

// move the contents of each component so that the components do not
// overlap.
void separateComponents(const std::vector<Component*> &components);

} // namespace cola

#endif // CONNECTED_COMPONENTS_H
