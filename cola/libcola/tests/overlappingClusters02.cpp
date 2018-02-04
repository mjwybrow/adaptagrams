// Based on Euler Diagram generation example discussed with Aidan Delaney. 
//
//              B
//             +-----------------------------+
//  A          |             2       C       |
// +-----------+----------------+   +---+    |
// |           |     1          |   | 0 |    |
// |           |                |   +---+    |
// |     3     +----------------+------------+
// |                            |
// |                            |    4
// +----------------------------+
//

#include <vector>
#include <utility>
#include <cstdlib>
#include "libcola/cola.h"
using namespace cola;
int main(void) {
    CompoundConstraints ccs;
    std::vector<Edge> es;
    EdgeLengths eLengths;
    double defaultEdgeLength=10;
    std::vector<vpsc::Rectangle*> rs;
    vpsc::Rectangle *rect = nullptr;

    double width = 5;
    double height = 5;
    double pos = 0;

    size_t nodes = 5;
    for (size_t i = 0; i < nodes; ++i)
    {
        rect = new vpsc::Rectangle(pos, pos +width, pos, pos +height);
        rs.push_back(rect);

	// XXX randomness is needed because COLA doesn't currently untangle
	// the graph properly if all the nodes begin at the same position.
        pos += (rand() % 10) - 5;
    }

    // Euler dual graph (optional)
    es.push_back(std::make_pair(2, 4));
    es.push_back(std::make_pair(3, 4));
    es.push_back(std::make_pair(1, 3));
    es.push_back(std::make_pair(2, 1));
    es.push_back(std::make_pair(2, 0));
    
    // Padding around the inside of clusters.
    double padding = 3;

    ConstrainedFDLayout alg(rs, es, defaultEdgeLength, eLengths);
    alg.setAvoidNodeOverlaps(true);
    RootCluster *rootCluster = new RootCluster();

    // A contains 1, 3
    RectangularCluster *clusterA = new RectangularCluster();
    clusterA->setPadding(padding);
    clusterA->addChildNode(1);
    clusterA->addChildNode(3);

    // C contains 0
    RectangularCluster *clusterC = new RectangularCluster();
    clusterC->setPadding(padding);
    clusterC->addChildNode(0);

    // B contains 1, 2, C
    RectangularCluster *clusterB = new RectangularCluster();
    clusterB->setPadding(padding);
    clusterB->addChildNode(1);
    clusterB->addChildNode(2);
    clusterB->addChildCluster(clusterC);

    // node 4 is in the empty set.

    rootCluster->addChildCluster(clusterA);
    rootCluster->addChildCluster(clusterB);

    alg.setConstraints(ccs);

    UnsatisfiableConstraintInfos unsatisfiableX, unsatisfiableY;
    alg.setUnsatisfiableConstraintInfo(&unsatisfiableX, &unsatisfiableY);
    
    alg.setClusterHierarchy(rootCluster);
    //alg.makeFeasible();
    alg.run();
    alg.outputInstanceToSVG("overlappingClusters02");
    
    for (size_t i = 0; i < unsatisfiableX.size(); ++i)
    {
	printf("%s\n", unsatisfiableX[i]->toString().c_str());
    }
    for (size_t i = 0; i < unsatisfiableY.size(); ++i)
    {
	printf("%s\n", unsatisfiableY[i]->toString().c_str());
    }
    alg.freeAssociatedObjects();
    return (unsatisfiableX.empty() && unsatisfiableY.empty()) ? 0 : 1;
};
