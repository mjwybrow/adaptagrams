// From Aidan Delaney: a_b_ab_abc.json.svg 
// Regression test for SEGFAULT caused by specifing an invalid node index 7
// in cluster description.
#include <vector>
#include <utility>
#include "libcola/cola.h"
using namespace cola;
int main(void) {
    CompoundConstraints ccs;
    std::vector<Edge> es;
    EdgeLengths eLengths;
    double defaultEdgeLength=10;
    std::vector<vpsc::Rectangle*> rs;
    vpsc::Rectangle *rect = nullptr;

    rect = new vpsc::Rectangle(0, 5, 0, 5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(-2, 3, -2, 3);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(-1, 4, -1, 4);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1, 6, 1, 6);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1, 6, 1, 6);
    rs.push_back(rect);


    ConstrainedFDLayout alg(rs, es, defaultEdgeLength, eLengths);
    alg.setAvoidNodeOverlaps(true);

    RootCluster *cluster33771480 = new RootCluster();
    RectangularCluster *cluster33771952 = new RectangularCluster();
    cluster33771952->setPadding(Box(2));
    cluster33771952->addChildNode(1);
    cluster33771952->addChildNode(3);
    cluster33771952->addChildNode(7);
    cluster33771480->addChildCluster(cluster33771952);
    
    RectangularCluster *cluster33772496 = new RectangularCluster();
    cluster33772496->setPadding(Box(2));
    cluster33772496->addChildNode(2);
    cluster33772496->addChildNode(3);
    cluster33772496->addChildNode(7);
    cluster33771480->addChildCluster(cluster33772496);
    
    RectangularCluster *cluster33773040 = new RectangularCluster();
    cluster33773040->setPadding(Box(2));
    cluster33773040->addChildNode(7);
    cluster33771480->addChildCluster(cluster33773040);
    
    alg.setClusterHierarchy(cluster33771480);
    
    alg.setConstraints(ccs);
    alg.makeFeasible();
    //alg.outputInstanceToSVG("overlappingCLusters04");
    alg.freeAssociatedObjects();
};
