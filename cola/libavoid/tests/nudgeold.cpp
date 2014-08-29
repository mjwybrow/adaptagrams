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

    Polygon poly426345871(4);
    poly426345871.ps[0] = Point(7200, 5240);
    poly426345871.ps[1] = Point(7200, 5660);
    poly426345871.ps[2] = Point(6690, 5660);
    poly426345871.ps[3] = Point(6690, 5240);
    new ShapeRef(router, poly426345871, 426345871);
    
    Polygon poly596896212(4);
    poly596896212.ps[0] = Point(7200, 5740);
    poly596896212.ps[1] = Point(7200, 6160);
    poly596896212.ps[2] = Point(6690, 6160);
    poly596896212.ps[3] = Point(6690, 5740);
    new ShapeRef(router, poly596896212, 596896212);
    
    ConnRef *connRef20026701 = new ConnRef(router, 20026701);
    ConnEnd srcPt20026701(Point(7190, 5600), 8);
    connRef20026701->setSourceEndpoint(srcPt20026701);
    ConnEnd dstPt20026701(Point(7190, 5800), 8);
    connRef20026701->setDestEndpoint(dstPt20026701);
    connRef20026701->setRoutingType((ConnType)2);

    ConnRef *connRef124776000 = new ConnRef(router, 124776000);
    ConnEnd srcPt124776000(Point(7190, 5550), 8);
    connRef124776000->setSourceEndpoint(srcPt124776000);
    ConnEnd dstPt124776000(Point(7190, 5850), 8);
    connRef124776000->setDestEndpoint(dstPt124776000);
    connRef124776000->setRoutingType((ConnType)2);

    ConnRef *connRef162425002 = new ConnRef(router, 162425002);
    ConnEnd srcPt162425002(Point(7190, 5500), 8);
    connRef162425002->setSourceEndpoint(srcPt162425002);
    ConnEnd dstPt162425002(Point(7190, 5900), 8);
    connRef162425002->setDestEndpoint(dstPt162425002);
    connRef162425002->setRoutingType((ConnType)2);

    ConnRef *connRef445102866 = new ConnRef(router, 445102866);
    ConnEnd srcPt445102866(Point(7190, 5450), 8);
    connRef445102866->setSourceEndpoint(srcPt445102866);
    ConnEnd dstPt445102866(Point(7190, 5950), 8);
    connRef445102866->setDestEndpoint(dstPt445102866);
    connRef445102866->setRoutingType((ConnType)2);

    ConnRef *connRef472520508 = new ConnRef(router, 472520508);
    ConnEnd srcPt472520508(Point(7190, 5400), 8);
    connRef472520508->setSourceEndpoint(srcPt472520508);
    ConnEnd dstPt472520508(Point(7190, 6000), 8);
    connRef472520508->setDestEndpoint(dstPt472520508);
    connRef472520508->setRoutingType((ConnType)2);

    ConnRef *connRef406587600 = new ConnRef(router, 406587600);
    ConnEnd srcPt406587600(Point(7190, 5350), 8);
    connRef406587600->setSourceEndpoint(srcPt406587600);
    ConnEnd dstPt406587600(Point(7190, 6050), 8);
    connRef406587600->setDestEndpoint(dstPt406587600);
    connRef406587600->setRoutingType((ConnType)2);

    ConnRef *connRef41657796 = new ConnRef(router, 41657796);
    ConnEnd srcPt41657796(Point(7190, 5300), 8);
    connRef41657796->setSourceEndpoint(srcPt41657796);
    ConnEnd dstPt41657796(Point(7190, 6100), 8);
    connRef41657796->setDestEndpoint(dstPt41657796);
    connRef41657796->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/nudgeold");
    delete router;
    return 0;
};
