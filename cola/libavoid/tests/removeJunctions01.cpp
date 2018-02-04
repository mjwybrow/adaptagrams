#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
    router->setRoutingParameter((RoutingParameter)0, 2);
    router->setRoutingParameter((RoutingParameter)1, 0);
    router->setRoutingParameter((RoutingParameter)2, 0);
    router->setRoutingParameter((RoutingParameter)3, 4000);
    router->setRoutingParameter((RoutingParameter)4, 9000);
    router->setRoutingParameter((RoutingParameter)5, 0);
    router->setRoutingParameter((RoutingParameter)6, 0);
    router->setRoutingParameter((RoutingParameter)7, 16);
    router->setRoutingParameter((RoutingParameter)8, 0);
    router->setRoutingOption((RoutingOption)0, false);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingOption((RoutingOption)2, false);
    router->setRoutingOption((RoutingOption)3, false);
    router->setRoutingOption((RoutingOption)4, true);
    router->setRoutingOption((RoutingOption)5, false);
    router->setRoutingOption((RoutingOption)6, true);
    Polygon polygon;
    ConnEnd srcPt;
    ConnEnd dstPt;
    ConnEnd heConnPt;
    PolyLine newRoute;
    ShapeConnectionPin *connPin = nullptr;

    // shapeRef1
    polygon = Polygon(4);
    polygon.ps[0] = Point(450, 330);
    polygon.ps[1] = Point(490, 330);
    polygon.ps[2] = Point(490, 450);
    polygon.ps[3] = Point(450, 450);
    ShapeRef *shapeRef1 = new ShapeRef(router, polygon, 1);
    connPin = new ShapeConnectionPin(shapeRef1, 1, 0, 0.333333, true, 10, (ConnDirFlags) 0);
    connPin->setExclusive(false);
    connPin = new ShapeConnectionPin(shapeRef1, 2, 0, 0.666667, true, 10, (ConnDirFlags) 0);
    connPin->setExclusive(false);
    connPin = new ShapeConnectionPin(shapeRef1, 3, 1, 0.5, true, 10, (ConnDirFlags) 0);
    connPin->setExclusive(false);
    connPin = new ShapeConnectionPin(shapeRef1, 4, 0.5, 0, true, 10, (ConnDirFlags) 0);
    connPin->setExclusive(false);

    // shapeRef8
    polygon = Polygon(4);
    polygon.ps[0] = Point(502, 237.5);
    polygon.ps[1] = Point(502, 262.5);
    polygon.ps[2] = Point(438, 262.5);
    polygon.ps[3] = Point(438, 237.5);
    ShapeRef *shapeRef8 = new ShapeRef(router, polygon, 8);
    connPin = new ShapeConnectionPin(shapeRef8, 1, 0, 0.5, true, 1, (ConnDirFlags) 0);
    connPin = new ShapeConnectionPin(shapeRef8, 2, 1, 0.5, true, 1, (ConnDirFlags) 0);
    connPin = new ShapeConnectionPin(shapeRef8, 3, 0.5, 1, true, 1, (ConnDirFlags) 0);
    connPin = new ShapeConnectionPin(shapeRef8, 4, 0.5, 0, true, 1, (ConnDirFlags) 0);

    JunctionRef *junctionRef6 = new JunctionRef(router, Point(470, 300), 6);
    /*
    // This may be useful if junction pins are modified.
    connPin = new ShapeConnectionPin(junctionRef6, 2147483646, (ConnDirFlags) 15);
    */

    // connRef9
    ConnRef *connRef9 = new ConnRef(router, 9);
    srcPt = ConnEnd(junctionRef6);
    connRef9->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef8, 3);
    connRef9->setDestEndpoint(dstPt);
    connRef9->setRoutingType((ConnType)2);

#if 1
    // connRef10 - WITH BUG
    ConnRef *connRef10 = new ConnRef(router, 10);
    srcPt = ConnEnd(junctionRef6);
    connRef10->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef1, 4);
    connRef10->setDestEndpoint(dstPt);
    connRef10->setRoutingType((ConnType)2);
#else
    // connRef10 - WITHOUT BUG
    ConnRef *connRef10 = new ConnRef(router, 10);
    srcPt = ConnEnd(shapeRef1, 4);
    connRef10->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(junctionRef6);
    connRef10->setDestEndpoint(dstPt);
    connRef10->setRoutingType((ConnType)2);
#endif

    //    router->deleteConnector(connRef5);
    router->processTransaction();

    ConnRef *mergedConn = junctionRef6->removeJunctionAndMergeConnectors();
    router->processTransaction();
    router->outputDiagram("output/removeJunctions01");

    std::pair<ConnEnd, ConnEnd> ends = mergedConn->endpointConnEnds();

    // Expected result: output images should be virtually identical.
    // Result: An endpoint seems to be disconnected from shapeRef8.
    bool isConnected = (ends.first.shape() == shapeRef8) || (ends.second.shape() == shapeRef8);
    
    delete router;
    return isConnected ? 0 : 1;
};
