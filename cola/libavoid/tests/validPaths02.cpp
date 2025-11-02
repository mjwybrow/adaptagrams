#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(
            PolyLineRouting | OrthogonalRouting);
    router->setRoutingPenalty((PenaltyType)0, 50);
    router->setRoutingPenalty((PenaltyType)1, 0);
    router->setRoutingPenalty((PenaltyType)2, 200);
    router->setRoutingPenalty((PenaltyType)3, 4000);
    router->setRoutingPenalty((PenaltyType)4, 110);
    router->setRoutingPenalty((PenaltyType)5, 100);
    router->setRoutingOption((RoutingOption)0, false);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingParameter(idealNudgingDistance, 25);

    Polygon poly186982048(4);
    poly186982048.ps[0] = Point(52660, 50490);
    poly186982048.ps[1] = Point(52660, 50960);
    poly186982048.ps[2] = Point(52090, 50960);
    poly186982048.ps[3] = Point(52090, 50490);
    ShapeRef *shapeRef186982048 = new ShapeRef(router, poly186982048, 186982048);
    new ShapeConnectionPin(shapeRef186982048, 5, 0.982456, 0.12766, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef186982048, 13, 0.982456, 0.234043, true, 0, (ConnDirFlags) 8);

    Polygon poly197692264(4);
    poly197692264.ps[0] = Point(52560, 50342);
    poly197692264.ps[1] = Point(52560, 50425);
    poly197692264.ps[2] = Point(52440, 50425);
    poly197692264.ps[3] = Point(52440, 50342);
    ShapeRef *shapeRef197692264 = new ShapeRef(router, poly197692264, 197692264);
    new ShapeConnectionPin(shapeRef197692264, 1, 0.5, 0.39759, true, 0, (ConnDirFlags) 13);
    new ShapeConnectionPin(shapeRef197692264, 2, 0.5, 1, true, 10, (ConnDirFlags) 2);

    ConnRef *connRef111820287 = new ConnRef(router, 111820287);
    ConnEnd srcPt111820287(shapeRef186982048, 13);
    connRef111820287->setSourceEndpoint(srcPt111820287);
    ConnEnd dstPt111820287(shapeRef197692264, 2);
    connRef111820287->setDestEndpoint(dstPt111820287);
    connRef111820287->setRoutingType((ConnType)2);

    ConnRef *connRef744282799 = new ConnRef(router, 744282799);
    ConnEnd srcPt744282799(shapeRef186982048, 5);
    connRef744282799->setSourceEndpoint(srcPt744282799);
    ConnEnd dstPt744282799(shapeRef197692264, 1);
    connRef744282799->setDestEndpoint(dstPt744282799);
    connRef744282799->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/validPaths02");
    int result = router->existsInvalidOrthogonalPaths();
    delete router;
    return result;
};
