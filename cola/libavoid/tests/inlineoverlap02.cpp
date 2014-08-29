// From p-overlap bug.
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

    Polygon poly548374163(4);
    poly548374163.ps[0] = Point(52285, 50415);
    poly548374163.ps[1] = Point(52285, 50885);
    poly548374163.ps[2] = Point(51915, 50885);
    poly548374163.ps[3] = Point(51915, 50415);
    new ShapeRef(router, poly548374163, 548374163);

    Polygon poly942733064(4);
    poly942733064.ps[0] = Point(51829, 50800);
    poly942733064.ps[1] = Point(51829, 50885);
    poly942733064.ps[2] = Point(51771, 50885);
    poly942733064.ps[3] = Point(51771, 50800);
    new ShapeRef(router, poly942733064, 942733064);

    Polygon poly335855988(4);
    poly335855988.ps[0] = Point(51510, 50190);
    poly335855988.ps[1] = Point(51510, 50885);
    poly335855988.ps[2] = Point(51140, 50885);
    poly335855988.ps[3] = Point(51140, 50190);
    new ShapeRef(router, poly335855988, 335855988);

    ConnRef *connRef240732432 = new ConnRef(router, 240732432);
    ConnEnd srcPt240732432(Point(51935, 50650), 4);
    connRef240732432->setSourceEndpoint(srcPt240732432);
    ConnEnd dstPt240732432(Point(51800, 50810), 1);
    connRef240732432->setDestEndpoint(dstPt240732432);
    connRef240732432->setRoutingType((ConnType)2);

    ConnRef *connRef148374725 = new ConnRef(router, 148374725);
    ConnEnd srcPt148374725(Point(51935, 50500), 4);
    connRef148374725->setSourceEndpoint(srcPt148374725);
    ConnEnd dstPt148374725(Point(51490, 50800), 8);
    connRef148374725->setDestEndpoint(dstPt148374725);
    connRef148374725->setRoutingType((ConnType)2);

    ConnRef *connRef100850179 = new ConnRef(router, 100850179);
    ConnEnd srcPt100850179(Point(51935, 50800), 4);
    connRef100850179->setSourceEndpoint(srcPt100850179);
    ConnEnd dstPt100850179(Point(51490, 50650), 8);
    connRef100850179->setDestEndpoint(dstPt100850179);
    connRef100850179->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/inlineoverlap02");
    bool overlap = router->existsOrthogonalFixedSegmentOverlap();
    delete router;
    return (overlap) ? 1 : 0;
};

