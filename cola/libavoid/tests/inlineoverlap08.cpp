#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(
            PolyLineRouting | OrthogonalRouting);
    router->setRoutingParameter((RoutingParameter)0, 10);
    router->setRoutingParameter((RoutingParameter)1, 0);
    router->setRoutingParameter((RoutingParameter)2, 0);
    router->setRoutingParameter((RoutingParameter)3, 4000);
    router->setRoutingParameter((RoutingParameter)4, 400);
    router->setRoutingParameter((RoutingParameter)5, 100);
    router->setRoutingOption((RoutingOption)0, false);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingOption((RoutingOption)2, false);
    router->setRoutingOption((RoutingOption)3, false);

    Polygon poly1(4);
    poly1.ps[0] = Point(99, -20);
    poly1.ps[1] = Point(99, 0);
    poly1.ps[2] = Point(55, 0);
    poly1.ps[3] = Point(55, -20);
    new ShapeRef(router, poly1, 1);

    Polygon poly2(4);
    poly2.ps[0] = Point(20, -65);
    poly2.ps[1] = Point(20, -45);
    poly2.ps[2] = Point(0, -45);
    poly2.ps[3] = Point(0, -65);
    new ShapeRef(router, poly2, 2);

    Polygon poly3(4);
    poly3.ps[0] = Point(20, -90);
    poly3.ps[1] = Point(20, -70);
    poly3.ps[2] = Point(0, -70);
    poly3.ps[3] = Point(0, -90);
    new ShapeRef(router, poly3, 3);

    Polygon poly4(4);
    poly4.ps[0] = Point(20, -115);
    poly4.ps[1] = Point(20, -95);
    poly4.ps[2] = Point(0, -95);
    poly4.ps[3] = Point(0, -115);
    new ShapeRef(router, poly4, 4);

    Polygon poly5(4);
    poly5.ps[0] = Point(20, -140);
    poly5.ps[1] = Point(20, -120);
    poly5.ps[2] = Point(0, -120);
    poly5.ps[3] = Point(0, -140);
    new ShapeRef(router, poly5, 5);

    Polygon poly6(4);
    poly6.ps[0] = Point(75, -185);
    poly6.ps[1] = Point(75, -165);
    poly6.ps[2] = Point(55, -165);
    poly6.ps[3] = Point(55, -185);
    new ShapeRef(router, poly6, 6);

    Polygon poly7(4);
    poly7.ps[0] = Point(100, -185);
    poly7.ps[1] = Point(100, -165);
    poly7.ps[2] = Point(80, -165);
    poly7.ps[3] = Point(80, -185);
    new ShapeRef(router, poly7, 7);

    Polygon poly8(4);
    poly8.ps[0] = Point(125, -185);
    poly8.ps[1] = Point(125, -165);
    poly8.ps[2] = Point(105, -165);
    poly8.ps[3] = Point(105, -185);
    new ShapeRef(router, poly8, 8);

    Polygon poly9(4);
    poly9.ps[0] = Point(150, -185);
    poly9.ps[1] = Point(150, -165);
    poly9.ps[2] = Point(130, -165);
    poly9.ps[3] = Point(130, -185);
    new ShapeRef(router, poly9, 9);

    Polygon poly10(4);
    poly10.ps[0] = Point(175, -185);
    poly10.ps[1] = Point(175, -165);
    poly10.ps[2] = Point(155, -165);
    poly10.ps[3] = Point(155, -185);
    new ShapeRef(router, poly10, 10);

    ConnRef *connRef11 = new ConnRef(router, 11);
    ConnEnd srcPt11(Point(20, -47), 15);
    connRef11->setSourceEndpoint(srcPt11);
    ConnEnd dstPt11(Point(57, -165), 15);
    connRef11->setDestEndpoint(dstPt11);
    connRef11->setRoutingType((ConnType)2);

    ConnRef *connRef12 = new ConnRef(router, 12);
    ConnEnd srcPt12(Point(61, -165), 15);
    connRef12->setSourceEndpoint(srcPt12);
    ConnEnd dstPt12(Point(57, -20), 15);
    connRef12->setDestEndpoint(dstPt12);
    connRef12->setRoutingType((ConnType)2);

    ConnRef *connRef13 = new ConnRef(router, 13);
    ConnEnd srcPt13(Point(15, -74), 15);
    connRef13->setSourceEndpoint(srcPt13);
    ConnEnd dstPt13(Point(65, -20), 15);
    connRef13->setDestEndpoint(dstPt13);
    connRef13->setRoutingType((ConnType)2);

    ConnRef *connRef14 = new ConnRef(router, 14);
    ConnEnd srcPt14(Point(61, -20), 15);
    connRef14->setSourceEndpoint(srcPt14);
    ConnEnd dstPt14(Point(61, -24), 15);
    connRef14->setDestEndpoint(dstPt14);
    connRef14->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/inlineoverlap08");
    bool overlap = router->existsOrthogonalFixedSegmentOverlap();
    delete router;
    return (overlap) ? 1 : 0;
};
