#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    ConnEndList hyperedgeTerminals;
    Router *router = new Router(OrthogonalRouting);
    /*
    router->setRoutingPenalty((PenaltyType)0, 78.7234);
    router->setRoutingPenalty((PenaltyType)1, 0);
    router->setRoutingPenalty((PenaltyType)2, 0);
    router->setRoutingPenalty((PenaltyType)3, 0);
    router->setRoutingPenalty((PenaltyType)4, 0);
    router->setRoutingPenalty((PenaltyType)5, 100);
    router->setRoutingOption((RoutingOption)0, true);
    router->setOrthogonalNudgeDistance(4);
    */

    Polygon poly1(4);
    poly1.ps[0] = Point(461.05, 1.66648);
    poly1.ps[1] = Point(461.05, 65.6665);
    poly1.ps[2] = Point(376.05, 65.6665);
    poly1.ps[3] = Point(376.05, 1.66648);
    ShapeRef *shapeRef1 = new ShapeRef(router, poly1, 1);
    new ShapeConnectionPin(shapeRef1, 102, 0.5, 1, true, 10, (ConnDirFlags) 2);
    hyperedgeTerminals.push_back(ConnEnd(shapeRef1, 102));

    Polygon poly2(4);
    poly2.ps[0] = Point(404, 166);
    poly2.ps[1] = Point(404, 227);
    poly2.ps[2] = Point(320, 227);
    poly2.ps[3] = Point(320, 166);
    ShapeRef *shapeRef2 = new ShapeRef(router, poly2, 2);
    new ShapeConnectionPin(shapeRef2, 102, 0, 0.5, true, 10, (ConnDirFlags) 4);
    hyperedgeTerminals.push_back(ConnEnd(shapeRef2, 102));

    Polygon poly3(4);
    poly3.ps[0] = Point(240.001, 185);
    poly3.ps[1] = Point(240.001, 227);
    poly3.ps[2] = Point(178.001, 227);
    poly3.ps[3] = Point(178.001, 185);
    ShapeRef *shapeRef3 = new ShapeRef(router, poly3, 3);
    new ShapeConnectionPin(shapeRef3, 102, 1, 0.5, true, 10, (ConnDirFlags) 8);
    hyperedgeTerminals.push_back(ConnEnd(shapeRef3, 102));

    Polygon poly4(4);
    poly4.ps[0] = Point(274.526, 132);
    poly4.ps[1] = Point(274.526, 174);
    poly4.ps[2] = Point(90.526, 174);
    poly4.ps[3] = Point(90.526, 132);
    new ShapeRef(router, poly4, 4);

    Polygon poly5(4);
    poly5.ps[0] = Point(152.526, 1.66648);
    poly5.ps[1] = Point(152.526, 40);
    poly5.ps[2] = Point(90.526, 40);
    poly5.ps[3] = Point(90.526, 1.66648);
    ShapeRef *shapeRef5 = new ShapeRef(router, poly5, 5);
    new ShapeConnectionPin(shapeRef5, 102, 0.5, 1, true, 10, (ConnDirFlags) 2);
    hyperedgeTerminals.push_back(ConnEnd(shapeRef5, 102));

    Polygon poly6(4);
    poly6.ps[0] = Point(366.925, 1.66648);
    poly6.ps[1] = Point(366.925, 92.6665);
    poly6.ps[2] = Point(304.925, 92.6665);
    poly6.ps[3] = Point(304.925, 1.66648);
    new ShapeRef(router, poly6, 6);

    Polygon poly8(4);
    poly8.ps[0] = Point(345, 115);
    poly8.ps[1] = Point(345, 140);
    poly8.ps[2] = Point(320, 140);
    poly8.ps[3] = Point(320, 115);
    new ShapeRef(router, poly8, 8);

    Polygon poly7(4);
    poly7.ps[0] = Point(223.675, 1.66648);
    poly7.ps[1] = Point(223.675, 50.3333);
    poly7.ps[2] = Point(161.675, 50.3333);
    poly7.ps[3] = Point(161.675, 1.66648);
    ShapeRef *shapeRef7 = new ShapeRef(router, poly7, 7);
    new ShapeConnectionPin(shapeRef7, 102, 0.5, 1, true, 10, (ConnDirFlags) 2);
    hyperedgeTerminals.push_back(ConnEnd(shapeRef7, 102));

    Polygon poly10(4);
    poly10.ps[0] = Point(295.3, 1.66648);
    poly10.ps[1] = Point(295.3, 40);
    poly10.ps[2] = Point(233.3, 40);
    poly10.ps[3] = Point(233.3, 1.66648);
    ShapeRef *shapeRef10 = new ShapeRef(router, poly10, 10);
    new ShapeConnectionPin(shapeRef10, 102, 0.5, 1, true, 10, (ConnDirFlags) 2);
    hyperedgeTerminals.push_back(ConnEnd(shapeRef10, 102));

    HyperedgeRerouter *hyperedgeRerouter = router->hyperedgeRerouter();
    hyperedgeRerouter->registerHyperedgeForRerouting(hyperedgeTerminals);

    router->processTransaction();
    router->outputDiagram("output/hyperedge02");
    delete router;
    return 0;
};
