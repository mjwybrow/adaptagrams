// From Routing_2a_libavoid-debug.
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

    Polygon poly170253204(4);
    poly170253204.ps[0] = Point(50450, 50190);
    poly170253204.ps[1] = Point(50450, 50360);
    poly170253204.ps[2] = Point(50265, 50360);
    poly170253204.ps[3] = Point(50265, 50190);
    new ShapeRef(router, poly170253204, 170253204);

    Polygon poly914700248(4);
    poly914700248.ps[0] = Point(50800, 50390);
    poly914700248.ps[1] = Point(50800, 50560);
    poly914700248.ps[2] = Point(50615, 50560);
    poly914700248.ps[3] = Point(50615, 50390);
    new ShapeRef(router, poly914700248, 914700248);

    Polygon poly285103539(4);
    poly285103539.ps[0] = Point(51385, 50390);
    poly285103539.ps[1] = Point(51385, 50560);
    poly285103539.ps[2] = Point(51200, 50560);
    poly285103539.ps[3] = Point(51200, 50390);
    new ShapeRef(router, poly285103539, 285103539);

    Polygon poly252459412(4);
    poly252459412.ps[0] = Point(51360, 50190);
    poly252459412.ps[1] = Point(51360, 50360);
    poly252459412.ps[2] = Point(51175, 50360);
    poly252459412.ps[3] = Point(51175, 50190);
    new ShapeRef(router, poly252459412, 252459412);

    ConnRef *connRef573390636 = new ConnRef(router, 573390636);
    ConnEnd srcPt573390636(Point(50790, 50475), 8);
    connRef573390636->setSourceEndpoint(srcPt573390636);
    ConnEnd dstPt573390636(Point(51185, 50275), 4);
    connRef573390636->setDestEndpoint(dstPt573390636);
    connRef573390636->setRoutingType((ConnType)2);

    ConnRef *connRef214207623 = new ConnRef(router, 214207623);
    ConnEnd srcPt214207623(Point(51210, 50475), 4);
    connRef214207623->setSourceEndpoint(srcPt214207623);
    ConnEnd dstPt214207623(Point(50440, 50275), 8);
    connRef214207623->setDestEndpoint(dstPt214207623);
    connRef214207623->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/inlineoverlap04");
    bool overlap = router->existsOrthogonalFixedSegmentOverlap();
    delete router;
    return (overlap) ? 1 : 0;
};

