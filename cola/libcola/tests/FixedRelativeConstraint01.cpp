#include <vector>
#include "libcola/cola.h"
using namespace cola;
int main(void) {
    CompoundConstraints ccs;
    std::vector<Edge> es;
    double defaultEdgeLength=40;
    std::vector<vpsc::Rectangle*> rs;
    vpsc::Rectangle *rect = nullptr;
    std::vector<unsigned> fixedShapes;
    
    double textW = 100;
    double textH = 10;
    double circleW = 5;
    double circleH = 5;

    rect = new vpsc::Rectangle(303-circleW, 303+circleW, 300-circleH, 300+circleH);
    rs.push_back(rect);
    fixedShapes.push_back(rs.size() - 1);
    
    rect = new vpsc::Rectangle(310-circleW, 310+circleW, 302-circleH, 302+circleH);
    rs.push_back(rect);
    fixedShapes.push_back(rs.size() - 1);
    
    rect = new vpsc::Rectangle(313-circleW, 313+circleW, 297-circleH, 297+circleH);
    rs.push_back(rect);
    fixedShapes.push_back(rs.size() - 1);

    rect = new vpsc::Rectangle(300-textW, 300+textW, 300-textH, 300+textH);
    rs.push_back(rect);
    
    rect = new vpsc::Rectangle(310-textW, 310+textW, 305-textH, 305+textH);
    rs.push_back(rect);
    
    rect = new vpsc::Rectangle(317-textW, 317+textW, 295-textH, 295+textH);
    rs.push_back(rect);

    ConstrainedFDLayout alg(rs, es, defaultEdgeLength);
    alg.setAvoidNodeOverlaps(true);
    
    ccs.push_back(new FixedRelativeConstraint(rs, fixedShapes, true));
    alg.setConstraints(ccs);
    
    alg.makeFeasible();

    // Can be used to output a "libcola-debug.svg" file:
    //alg.outputInstanceToSVG();
    alg.freeAssociatedObjects();

    return 0;
};
