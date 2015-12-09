#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
    router->setRoutingPenalty((PenaltyType)0, 50);
    router->setRoutingPenalty((PenaltyType)1, 0);
    router->setRoutingPenalty((PenaltyType)2, 0);
    router->setRoutingPenalty((PenaltyType)3, 4000);
    router->setRoutingPenalty((PenaltyType)4, 0);

    Polygon poly1(4);
    poly1.ps[0] = Point(52, 188);
    poly1.ps[1] = Point(52, 292);
    poly1.ps[2] = Point(-52, 292);
    poly1.ps[3] = Point(-52, 188);
    ShapeRef *shapeRef1 = new ShapeRef(router, poly1, 1);
    new ShapeConnectionPin(shapeRef1, 544, 0.5, 0.5, true, 10, (ConnDirFlags) 0);

    Polygon poly2(4);
    poly2.ps[0] = Point(52, -52);
    poly2.ps[1] = Point(52, 52);
    poly2.ps[2] = Point(-52, 52);
    poly2.ps[3] = Point(-52, -52);
    ShapeRef *shapeRef2 = new ShapeRef(router, poly2, 2);
    new ShapeConnectionPin(shapeRef2, 544, 0.5, 0.5, true, 10, (ConnDirFlags) 0);

    Polygon poly3(4);
    poly3.ps[0] = Point(52, 428);
    poly3.ps[1] = Point(52, 532);
    poly3.ps[2] = Point(-52, 532);
    poly3.ps[3] = Point(-52, 428);
    ShapeRef *shapeRef3 = new ShapeRef(router, poly3, 3);
    new ShapeConnectionPin(shapeRef3, 544, 0.5, 0.5, true, 10, (ConnDirFlags) 0);

    Polygon poly4(4);
    poly4.ps[0] = Point(52, 68);
    poly4.ps[1] = Point(52, 172);
    poly4.ps[2] = Point(-52, 172);
    poly4.ps[3] = Point(-52, 68);
    ShapeRef *shapeRef4 = new ShapeRef(router, poly4, 4);
    new ShapeConnectionPin(shapeRef4, 544, 0.5, 0.5, true, 10, (ConnDirFlags) 0);

    Polygon poly5(4);
    poly5.ps[0] = Point(52, 308);
    poly5.ps[1] = Point(52, 412);
    poly5.ps[2] = Point(-52, 412);
    poly5.ps[3] = Point(-52, 308);
    ShapeRef *shapeRef5 = new ShapeRef(router, poly5, 5);
    new ShapeConnectionPin(shapeRef5, 544, 0.5, 0.5, true, 10, (ConnDirFlags) 0);

    ConnRef *connRef6 = new ConnRef(router, 6);
    ConnEnd srcPt6(shapeRef2, 544);
    connRef6->setSourceEndpoint(srcPt6);
    ConnEnd dstPt6(shapeRef5, 544);
    connRef6->setDestEndpoint(dstPt6);
    connRef6->setRoutingType((ConnType)2);

    ConnRef *connRef7 = new ConnRef(router, 7);
    ConnEnd srcPt7(shapeRef5, 544);
    connRef7->setSourceEndpoint(srcPt7);
    ConnEnd dstPt7(shapeRef4, 544);
    connRef7->setDestEndpoint(dstPt7);
    connRef7->setRoutingType((ConnType)2);

    ConnRef *connRef8 = new ConnRef(router, 8);
    ConnEnd srcPt8(shapeRef1, 544);
    connRef8->setSourceEndpoint(srcPt8);
    ConnEnd dstPt8(shapeRef5, 544);
    connRef8->setDestEndpoint(dstPt8);
    connRef8->setRoutingType((ConnType)2);

    ConnRef *connRef9 = new ConnRef(router, 9);
    ConnEnd srcPt9(shapeRef2, 544);
    connRef9->setSourceEndpoint(srcPt9);
    ConnEnd dstPt9(shapeRef3, 544);
    connRef9->setDestEndpoint(dstPt9);
    connRef9->setRoutingType((ConnType)2);

    ConnRef *connRef10 = new ConnRef(router, 10);
    ConnEnd srcPt10(shapeRef3, 544);
    connRef10->setSourceEndpoint(srcPt10);
    ConnEnd dstPt10(shapeRef4, 544);
    connRef10->setDestEndpoint(dstPt10);
    connRef10->setRoutingType((ConnType)2);

    ConnRef *connRef11 = new ConnRef(router, 11);
    ConnEnd srcPt11(shapeRef1, 544);
    connRef11->setSourceEndpoint(srcPt11);
    ConnEnd dstPt11(shapeRef3, 544);
    connRef11->setDestEndpoint(dstPt11);
    connRef11->setRoutingType((ConnType)2);
    
    router->processTransaction();
    router->outputDiagram("output/inlineShapes");
    delete router;
    return 0;
};
