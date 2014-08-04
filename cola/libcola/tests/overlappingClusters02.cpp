// Based on Euler Diagram generation example discussed with Aidan Delaney. 
//
//              B
//             +-----------------------------+
//  A          |             g       C       |
// +-----------+----------------+   +---+    |
// |           |     f          |   | e |    |
// |           |                |   +---+    |
// |     g     +----------------+------------+
// |                            |
// |                            |
// +----------------------------+
//
#include <vector>
#include <utility>
#include "libcola/cola.h"
using namespace cola;
int main(void) {
    CompoundConstraints ccs;
    std::vector<Edge> es;
    EdgeLengths eLengths;
    double defaultEdgeLength=50;
    std::vector<vpsc::Rectangle*> rs;
    vpsc::Rectangle *rect = NULL;

    double width = 5;
    double height = 5;
    
    // e
    rect = new vpsc::Rectangle(0, width, 0, height);
    rs.push_back(rect);
    
    // f
    rect = new vpsc::Rectangle(0, width, 0, height);
    rs.push_back(rect);
    
    // g
    rect = new vpsc::Rectangle(0, width, 0, height);
    rs.push_back(rect);
    
    // h
    rect = new vpsc::Rectangle(0, width, 0, height);
    rs.push_back(rect);

    //es.push_back(std::make_pair(0, 1));

    double padding = 3;
    ConstrainedFDLayout alg(rs, es, defaultEdgeLength, true, eLengths);
    RootCluster *rootCluster = new RootCluster();

    // A contains f, h
    RectangularCluster *clusterA = new RectangularCluster();
    clusterA->setPadding(padding);
    clusterA->addChildNode(1);
    clusterA->addChildNode(3);

    // C contains e
    RectangularCluster *clusterC = new RectangularCluster();
    clusterC->setPadding(padding);
    clusterC->addChildNode(0);

    // B contains f, g, C
    RectangularCluster *clusterB = new RectangularCluster();
    clusterB->setPadding(padding);
    clusterB->addChildNode(1);
    clusterB->addChildNode(2);
    clusterB->addChildCluster(clusterC);

    rootCluster->addChildCluster(clusterA);
    rootCluster->addChildCluster(clusterB);

    alg.setClusterHierarchy(rootCluster);
    alg.setConstraints(ccs);

    UnsatisfiableConstraintInfos unsatisfiableX, unsatisfiableY;
    alg.setUnsatisfiableConstraintInfo(&unsatisfiableX, &unsatisfiableY);
    
    alg.makeFeasible();
    //alg.run();
    //alg.outputInstanceToSVG("overlappingClusters02");

    for (size_t i = 0; i < unsatisfiableX.size(); ++i)
    {
	printf("%s\n", unsatisfiableX[i]->toString().c_str());
    }
    for (size_t i = 0; i < unsatisfiableY.size(); ++i)
    {
	printf("%s\n", unsatisfiableY[i]->toString().c_str());
    }
    return (unsatisfiableX.empty() && unsatisfiableY.empty()) ? 0 : 1;
};
