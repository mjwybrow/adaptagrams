#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
    router->setRoutingParameter((RoutingParameter)0, 50);
    router->setRoutingParameter((RoutingParameter)1, 0);
    router->setRoutingParameter((RoutingParameter)2, 200);
    router->setRoutingParameter((RoutingParameter)3, 4000);
    router->setRoutingParameter((RoutingParameter)4, 20000);
    router->setRoutingParameter((RoutingParameter)5, 100);
    router->setRoutingParameter((RoutingParameter)6, 0);
    router->setRoutingParameter((RoutingParameter)7, 25);
    router->setRoutingOption((RoutingOption)0, false);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingOption((RoutingOption)2, true);
    router->setRoutingOption((RoutingOption)3, false);
    router->setRoutingOption((RoutingOption)4, true);
    router->setRoutingOption((RoutingOption)5, true);
    Polygon polygon;
    ConnRef *connRef = nullptr;
    ConnEnd srcPt;
    ConnEnd dstPt;
    PolyLine newRoute;
    // shapeRef320077872
    polygon = Polygon(4);
    polygon.ps[0] = Point(51000, 50740);
    polygon.ps[1] = Point(51000, 51135);
    polygon.ps[2] = Point(50650, 51135);
    polygon.ps[3] = Point(50650, 50740);
    ShapeRef *shapeRef320077872 = new ShapeRef(router, polygon, 320077872);
    new ShapeConnectionPin(shapeRef320077872, 1, 0, 0.21519, true, 10, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef320077872, 2, 1, 0.21519, true, 10, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef320077872, 3, 0.0714286, 0.405063, true, 10, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef320077872, 4, 1, 0.405063, true, 10, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef320077872, 5, 0.0714286, 0.594937, true, 10, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef320077872, 6, 1, 0.594937, true, 10, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef320077872, 7, 0, 0.78481, true, 10, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef320077872, 8, 1, 0.78481, true, 10, (ConnDirFlags) 8);

    // shapeRef346216920
    polygon = Polygon(4);
    polygon.ps[0] = Point(51650, 50740);
    polygon.ps[1] = Point(51650, 51135);
    polygon.ps[2] = Point(51300, 51135);
    polygon.ps[3] = Point(51300, 50740);
    ShapeRef *shapeRef346216920 = new ShapeRef(router, polygon, 346216920);
    new ShapeConnectionPin(shapeRef346216920, 1, 0, 0.21519, true, 10, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef346216920, 2, 1, 0.21519, true, 10, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef346216920, 3, 0, 0.405063, true, 10, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef346216920, 4, 0.928571, 0.405063, true, 10, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef346216920, 5, 0, 0.594937, true, 10, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef346216920, 6, 0.928571, 0.594937, true, 10, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef346216920, 7, 0, 0.78481, true, 10, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef346216920, 8, 1, 0.78481, true, 10, (ConnDirFlags) 8);

    // connRef13958908
    connRef = new ConnRef(router, 13958908);
    srcPt = ConnEnd(shapeRef346216920, 3);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef320077872, 6);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef81482716
    connRef = new ConnRef(router, 81482716);
    srcPt = ConnEnd(shapeRef320077872, 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef346216920, 7);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef180721992
    connRef = new ConnRef(router, 180721992);
    srcPt = ConnEnd(shapeRef320077872, 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef346216920, 5);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef273666240
    connRef = new ConnRef(router, 273666240);
    srcPt = ConnEnd(shapeRef320077872, 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef346216920, 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/inlineoverlap10");
    bool overlap = router->existsOrthogonalFixedSegmentOverlap();
    delete router;
    return (overlap) ? 1 : 0;
};
