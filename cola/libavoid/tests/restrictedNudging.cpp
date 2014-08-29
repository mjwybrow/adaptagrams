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
    router->setRoutingParameter(idealNudgingDistance, 25);

    Polygon poly282634758(4);
    poly282634758.ps[0] = Point(51360, 50215);
    poly282634758.ps[1] = Point(51360, 50460);
    poly282634758.ps[2] = Point(51075, 50460);
    poly282634758.ps[3] = Point(51075, 50215);
    new ShapeRef(router, poly282634758, 282634758);
    
    Polygon poly69386928(4);
    poly69386928.ps[0] = Point(51050, 50415);
    poly69386928.ps[1] = Point(51050, 50735);
    poly69386928.ps[2] = Point(50690, 50735);
    poly69386928.ps[3] = Point(50690, 50415);
    new ShapeRef(router, poly69386928, 69386928);
    
    Polygon poly11958280(4);
    poly11958280.ps[0] = Point(50460, 50490);
    poly11958280.ps[1] = Point(50460, 50660);
    poly11958280.ps[2] = Point(50290, 50660);
    poly11958280.ps[3] = Point(50290, 50490);
    new ShapeRef(router, poly11958280, 11958280);
    
    Polygon poly50591298(4);
    poly50591298.ps[0] = Point(51260, 50015);
    poly50591298.ps[1] = Point(51260, 50185);
    poly50591298.ps[2] = Point(51075, 50185);
    poly50591298.ps[3] = Point(51075, 50015);
    new ShapeRef(router, poly50591298, 50591298);
    
    ConnRef *connRef355676284 = new ConnRef(router, 355676284);
    ConnEnd srcPt355676284(Point(51040, 50575), 8);
    connRef355676284->setSourceEndpoint(srcPt355676284);
    ConnEnd dstPt355676284(Point(51085, 50300), 4);
    connRef355676284->setDestEndpoint(dstPt355676284);
    connRef355676284->setRoutingType((ConnType)2);

    ConnRef *connRef33653259 = new ConnRef(router, 33653259);
    ConnEnd srcPt33653259(Point(51040, 50650), 8);
    connRef33653259->setSourceEndpoint(srcPt33653259);
    ConnEnd dstPt33653259(Point(51085, 50375), 4);
    connRef33653259->setDestEndpoint(dstPt33653259);
    connRef33653259->setRoutingType((ConnType)2);

    ConnRef *connRef421608980 = new ConnRef(router, 421608980);
    ConnEnd srcPt421608980(Point(51040, 50500), 8);
    connRef421608980->setSourceEndpoint(srcPt421608980);
    ConnEnd dstPt421608980(Point(51085, 50100), 4);
    connRef421608980->setDestEndpoint(dstPt421608980);
    connRef421608980->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/restrictedNudging");
    
    bool overlap = router->existsOrthogonalTouchingPaths();
    delete router;
    return (overlap) ? 1 : 0;
};
