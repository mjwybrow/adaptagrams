// From diffport-pins-overlap-reduced_libavoid-debug_4 bug.
#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(
            PolyLineRouting | OrthogonalRouting);
    router->setRoutingPenalty((PenaltyType)0, 50);
    router->setRoutingPenalty((PenaltyType)1, 0);
    router->setRoutingPenalty((PenaltyType)2, 0);
    router->setRoutingPenalty((PenaltyType)3, 4000);
    router->setRoutingPenalty((PenaltyType)4, 110);
    router->setRoutingParameter(idealNudgingDistance, 25);

    Polygon poly421433292(4);
    poly421433292.ps[0] = Point(51335, 50215);
    poly421433292.ps[1] = Point(51335, 50385);
    poly421433292.ps[2] = Point(51065, 50385);
    poly421433292.ps[3] = Point(51065, 50215);
    new ShapeRef(router, poly421433292, 421433292);

    Polygon poly174734866(4);
    poly174734866.ps[0] = Point(51335, 50490);
    poly174734866.ps[1] = Point(51335, 50660);
    poly174734866.ps[2] = Point(51065, 50660);
    poly174734866.ps[3] = Point(51065, 50490);
    new ShapeRef(router, poly174734866, 174734866);

    Polygon poly564884274(4);
    poly564884274.ps[0] = Point(50635, 50500);
    poly564884274.ps[1] = Point(50635, 50583);
    poly564884274.ps[2] = Point(50515, 50583);
    poly564884274.ps[3] = Point(50515, 50500);
    new ShapeRef(router, poly564884274, 564884274);

    ConnRef *connRef103176525 = new ConnRef(router, 103176525);
    ConnEnd srcPt103176525(Point(51085, 50550), 4);
    connRef103176525->setSourceEndpoint(srcPt103176525);
    ConnEnd dstPt103176525(Point(50575, 50510), 1);
    connRef103176525->setDestEndpoint(dstPt103176525);
    connRef103176525->setRoutingType((ConnType)2);

    ConnRef *connRef223495632 = new ConnRef(router, 223495632);
    ConnEnd srcPt223495632(Point(51085, 50325), 4);
    connRef223495632->setSourceEndpoint(srcPt223495632);
    ConnEnd dstPt223495632(Point(50575, 50550), 14);
    connRef223495632->setDestEndpoint(dstPt223495632);
    connRef223495632->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/inlineoverlap03");
    bool overlap = router->existsOrthogonalFixedSegmentOverlap();
    delete router;
    return (overlap) ? 1 : 0;
};

