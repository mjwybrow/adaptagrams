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
    router->setRoutingParameter(idealNudgingDistance, 25);

    Polygon poly186982048(4);
    poly186982048.ps[0] = Point(52660, 50490);
    poly186982048.ps[1] = Point(52660, 50960);
    poly186982048.ps[2] = Point(52100, 50960);
    poly186982048.ps[3] = Point(52100, 50490);
    new ShapeRef(router, poly186982048, 186982048);
    
    Polygon poly193989760(4);
    poly193989760.ps[0] = Point(51850, 50665);
    poly193989760.ps[1] = Point(51850, 50835);
    poly193989760.ps[2] = Point(51590, 50835);
    poly193989760.ps[3] = Point(51590, 50665);
    new ShapeRef(router, poly193989760, 193989760);
    
    Polygon poly276825384(4);
    poly276825384.ps[0] = Point(51825, 51040);
    poly276825384.ps[1] = Point(51825, 51210);
    poly276825384.ps[2] = Point(51565, 51210);
    poly276825384.ps[3] = Point(51565, 51040);
    new ShapeRef(router, poly276825384, 276825384);
    
    Polygon poly190150804(4);
    poly190150804.ps[0] = Point(50460, 51340);
    poly190150804.ps[1] = Point(50460, 51510);
    poly190150804.ps[2] = Point(50290, 51510);
    poly190150804.ps[3] = Point(50290, 51340);
    new ShapeRef(router, poly190150804, 190150804);
    
    ConnRef *connRef562170592 = new ConnRef(router, 562170592);
    ConnEnd srcPt562170592(Point(52110, 50800), 4);
    connRef562170592->setSourceEndpoint(srcPt562170592);
    ConnEnd dstPt562170592(Point(51815, 51100), 8);
    connRef562170592->setDestEndpoint(dstPt562170592);
    connRef562170592->setRoutingType((ConnType)2);

    ConnRef *connRef171593006 = new ConnRef(router, 171593006);
    ConnEnd srcPt171593006(Point(52110, 50750), 4);
    connRef171593006->setSourceEndpoint(srcPt171593006);
    ConnEnd dstPt171593006(Point(51840, 50725), 8);
    connRef171593006->setDestEndpoint(dstPt171593006);
    connRef171593006->setRoutingType((ConnType)2);

    ConnRef *connRef172593160 = new ConnRef(router, 172593160);
    ConnEnd srcPt172593160(Point(51815, 51150), 8);
    connRef172593160->setSourceEndpoint(srcPt172593160);
    ConnEnd dstPt172593160(Point(52110, 50900), 4);
    connRef172593160->setDestEndpoint(dstPt172593160);
    connRef172593160->setRoutingType((ConnType)2);

    ConnRef *connRef116944564 = new ConnRef(router, 116944564);
    ConnEnd srcPt116944564(Point(51840, 50775), 8);
    connRef116944564->setSourceEndpoint(srcPt116944564);
    ConnEnd dstPt116944564(Point(52110, 50850), 4);
    connRef116944564->setDestEndpoint(dstPt116944564);
    connRef116944564->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/inlineoverlap07");
    delete router;
    return 0;
};

