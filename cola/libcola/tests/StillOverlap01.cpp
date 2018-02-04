#include <vector>
#include "libcola/cola.h"
using namespace cola;
int main(void) {
    CompoundConstraints ccs;
    std::vector<Edge> es;
    double defaultEdgeLength=40;
    std::vector<vpsc::Rectangle*> rs;
    vpsc::Rectangle *rect = nullptr;

    rect = new vpsc::Rectangle(210, 240, 83.5, 113.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(210, 240, 125.5, 155.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(377, 407, 43.5, 73.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(251, 281, 43.5, 73.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(110, 140, 43.5, 73.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(29, 79, 55.5, 105.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(427, 477, 32.5, 82.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(300, 350, 32.5, 82.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(171, 221, 32.5, 82.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(33, 83, 115.5, 165.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(522, 572, 115.5, 165.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(519, 569, 35.5, 85.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(192, 258, 123.5, 173.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(192, 258, 65.5, 115.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(359, 425, -16.5, 33.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(233, 299, -16.5, 33.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(154, 220, 33.5, 83.5);
    rs.push_back(rect);

    AlignmentConstraint *alignment140600592030544 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592030544->addShape(1, 0);
    alignment140600592030544->addShape(0, 0);
    ccs.push_back(alignment140600592030544);

    AlignmentConstraint *alignment140600592030848 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592030848->addShape(2, 0);
    ccs.push_back(alignment140600592030848);

    AlignmentConstraint *alignment140600592031040 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592031040->addShape(3, 0);
    ccs.push_back(alignment140600592031040);

    AlignmentConstraint *alignment140600592031168 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592031168->addShape(4, 0);
    ccs.push_back(alignment140600592031168);

    AlignmentConstraint *alignment140600592031296 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592031296->addShape(5, 0);
    ccs.push_back(alignment140600592031296);

    AlignmentConstraint *alignment140600592031456 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592031456->addShape(6, 0);
    ccs.push_back(alignment140600592031456);

    AlignmentConstraint *alignment140600592031616 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592031616->addShape(7, 0);
    ccs.push_back(alignment140600592031616);

    AlignmentConstraint *alignment140600592031776 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592031776->addShape(8, 0);
    ccs.push_back(alignment140600592031776);

    AlignmentConstraint *alignment140600592031936 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592031936->addShape(9, 0);
    ccs.push_back(alignment140600592031936);

    AlignmentConstraint *alignment140600592032240 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592032240->addShape(10, 0);
    ccs.push_back(alignment140600592032240);

    AlignmentConstraint *alignment140600592032400 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592032400->addShape(11, 0);
    ccs.push_back(alignment140600592032400);

    SeparationConstraint *separation140600592044112 = new SeparationConstraint(vpsc::XDIM, alignment140600592031296, alignment140600592031936, 4, false);
    ccs.push_back(separation140600592044112);

    SeparationConstraint *separation140600592045280 = new SeparationConstraint(vpsc::XDIM, alignment140600592031936, alignment140600592031168, 67, false);
    ccs.push_back(separation140600592045280);

    SeparationConstraint *separation140600592045376 = new SeparationConstraint(vpsc::XDIM, alignment140600592031168, alignment140600592031776, 71, false);
    ccs.push_back(separation140600592045376);

    SeparationConstraint *separation140600592045520 = new SeparationConstraint(vpsc::XDIM, alignment140600592031776, alignment140600592030544, 29, false);
    ccs.push_back(separation140600592045520);

    SeparationConstraint *separation140600592045696 = new SeparationConstraint(vpsc::XDIM, alignment140600592030544, alignment140600592031040, 41, false);
    ccs.push_back(separation140600592045696);

    SeparationConstraint *separation140600592045872 = new SeparationConstraint(vpsc::XDIM, alignment140600592031040, alignment140600592031616, 59, false);
    ccs.push_back(separation140600592045872);

    SeparationConstraint *separation140600592043472 = new SeparationConstraint(vpsc::XDIM, alignment140600592031616, alignment140600592030848, 67, false);
    ccs.push_back(separation140600592043472);

    SeparationConstraint *separation140600592043648 = new SeparationConstraint(vpsc::XDIM, alignment140600592030848, alignment140600592031456, 60, false);
    ccs.push_back(separation140600592043648);

    SeparationConstraint *separation140600592043824 = new SeparationConstraint(vpsc::XDIM, alignment140600592031456, alignment140600592032400, 92, false);
    ccs.push_back(separation140600592043824);

    SeparationConstraint *separation140600592046048 = new SeparationConstraint(vpsc::XDIM, alignment140600592032400, alignment140600592032240, 3, false);
    ccs.push_back(separation140600592046048);

    AlignmentConstraint *alignment140600592044208 = new AlignmentConstraint(vpsc::YDIM, 0);
    alignment140600592044208->addShape(0, 0);
    ccs.push_back(alignment140600592044208);

    AlignmentConstraint *alignment140600592044368 = new AlignmentConstraint(vpsc::YDIM, 0);
    alignment140600592044368->addShape(5, 0);
    ccs.push_back(alignment140600592044368);

    AlignmentConstraint *alignment140600592044528 = new AlignmentConstraint(vpsc::YDIM, 0);
    alignment140600592044528->addShape(1, 0);
    alignment140600592044528->addShape(9, 0);
    alignment140600592044528->addShape(10, 0);
    ccs.push_back(alignment140600592044528);

    AlignmentConstraint *alignment140600592044752 = new AlignmentConstraint(vpsc::YDIM, 0);
    alignment140600592044752->addShape(6, 0);
    alignment140600592044752->addShape(4, 0);
    alignment140600592044752->addShape(7, 0);
    alignment140600592044752->addShape(8, 0);
    alignment140600592044752->addShape(3, 0);
    alignment140600592044752->addShape(11, 0);
    alignment140600592044752->addShape(2, 0);
    ccs.push_back(alignment140600592044752);

    SeparationConstraint *separation140600592045088 = new SeparationConstraint(vpsc::YDIM, alignment140600592044752, alignment140600592044368, 22.1429, false);
    ccs.push_back(separation140600592045088);

    SeparationConstraint *separation140600590330720 = new SeparationConstraint(vpsc::YDIM, alignment140600592044368, alignment140600592044208, 18, false);
    ccs.push_back(separation140600590330720);

    SeparationConstraint *separation140600590330896 = new SeparationConstraint(vpsc::YDIM, alignment140600592044208, alignment140600592044528, 42, false);
    ccs.push_back(separation140600590330896);

    AlignmentConstraint *alignment140600590331072 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600590331072->addShape(0, 0);
    alignment140600590331072->addShape(12, 0);
    ccs.push_back(alignment140600590331072);

    SeparationConstraint *separation140600590331376 = new SeparationConstraint(vpsc::YDIM, 0, 12, 50, true);
    ccs.push_back(separation140600590331376);

    AlignmentConstraint *alignment140600590331520 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600590331520->addShape(1, 0);
    alignment140600590331520->addShape(13, 0);
    ccs.push_back(alignment140600590331520);

    SeparationConstraint *separation140600590331744 = new SeparationConstraint(vpsc::YDIM, 1, 13, -50, true);
    ccs.push_back(separation140600590331744);

    AlignmentConstraint *alignment140600590331888 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600590331888->addShape(2, 0);
    alignment140600590331888->addShape(14, 0);
    ccs.push_back(alignment140600590331888);

    SeparationConstraint *separation140600590332112 = new SeparationConstraint(vpsc::YDIM, 2, 14, -50, true);
    ccs.push_back(separation140600590332112);

    AlignmentConstraint *alignment140600592043200 = new AlignmentConstraint(vpsc::XDIM, 0);
    alignment140600592043200->addShape(3, 0);
    alignment140600592043200->addShape(15, 0);
    ccs.push_back(alignment140600592043200);

    SeparationConstraint *separation140600590332784 = new SeparationConstraint(vpsc::YDIM, 3, 15, -50, true);
    ccs.push_back(separation140600590332784);

    AlignmentConstraint *alignment140600590332880 = new AlignmentConstraint(vpsc::YDIM, 0);
    alignment140600590332880->addShape(4, 0);
    alignment140600590332880->addShape(16, 0);
    ccs.push_back(alignment140600590332880);

    SeparationConstraint *separation140600590333104 = new SeparationConstraint(vpsc::XDIM, 4, 16, 62, true);
    ccs.push_back(separation140600590333104);

    ConstrainedFDLayout alg(rs, es, defaultEdgeLength);
    alg.setAvoidNodeOverlaps(true);
    RootCluster *cluster140600590333360 = new RootCluster();
    alg.setClusterHierarchy(cluster140600590333360);
    alg.setConstraints(ccs);
    alg.makeFeasible();
    alg.run();
    alg.outputInstanceToSVG("test-StillOverlap01");
    alg.freeAssociatedObjects();
    return 0;
};

