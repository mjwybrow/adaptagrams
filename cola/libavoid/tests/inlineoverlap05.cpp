// From Diplexer-reduced2_libavoid-debug.
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

    Polygon poly231469760(4);
    poly231469760.ps[0] = Point(6385, 9390);
    poly231469760.ps[1] = Point(6385, 9560);
    poly231469760.ps[2] = Point(6200, 9560);
    poly231469760.ps[3] = Point(6200, 9390);
    new ShapeRef(router, poly231469760, 231469760);

    Polygon poly7136376(4);
    poly7136376.ps[0] = Point(6560, 9290);
    poly7136376.ps[1] = Point(6560, 9460);
    poly7136376.ps[2] = Point(6375, 9460);
    poly7136376.ps[3] = Point(6375, 9290);
    new ShapeRef(router, poly7136376, 7136376);

    Polygon poly215821749(4);
    poly215821749.ps[0] = Point(6510, 9540);
    poly215821749.ps[1] = Point(6510, 9710);
    poly215821749.ps[2] = Point(6325, 9710);
    poly215821749.ps[3] = Point(6325, 9540);
    new ShapeRef(router, poly215821749, 215821749);

    Polygon poly83086994(4);
    poly83086994.ps[0] = Point(5925, 9240);
    poly83086994.ps[1] = Point(5925, 10085);
    poly83086994.ps[2] = Point(5575, 10085);
    poly83086994.ps[3] = Point(5575, 9240);
    new ShapeRef(router, poly83086994, 83086994);

    ConnRef *connRef292408794 = new ConnRef(router, 292408794);
    ConnEnd srcPt292408794(Point(5915, 9625), 8);
    connRef292408794->setSourceEndpoint(srcPt292408794);
    ConnEnd dstPt292408794(Point(6210, 9475), 4);
    connRef292408794->setDestEndpoint(dstPt292408794);
    connRef292408794->setRoutingType((ConnType)2);

    ConnRef *connRef40917776 = new ConnRef(router, 40917776);
    ConnEnd srcPt40917776(Point(5915, 9550), 8);
    connRef40917776->setSourceEndpoint(srcPt40917776);
    ConnEnd dstPt40917776(Point(6385, 9375), 4);
    connRef40917776->setDestEndpoint(dstPt40917776);
    connRef40917776->setRoutingType((ConnType)2);

    ConnRef *connRef629564142 = new ConnRef(router, 629564142);
    ConnEnd srcPt629564142(Point(5915, 9475), 8);
    connRef629564142->setSourceEndpoint(srcPt629564142);
    ConnEnd dstPt629564142(Point(6335, 9625), 4);
    connRef629564142->setDestEndpoint(dstPt629564142);
    connRef629564142->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/inlineoverlap05");
    bool overlap = router->existsOrthogonalFixedSegmentOverlap();
    delete router;
    return (overlap) ? 1 : 0;
};

