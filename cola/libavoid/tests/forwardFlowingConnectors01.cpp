#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
    router->setRoutingParameter((RoutingParameter)0, 10);
    router->setRoutingParameter((RoutingParameter)1, 0);
    router->setRoutingParameter((RoutingParameter)2, 1000);
    router->setRoutingParameter((RoutingParameter)3, 4000);
    router->setRoutingParameter((RoutingParameter)4, 0);
    router->setRoutingParameter((RoutingParameter)5, 100);
    router->setRoutingParameter((RoutingParameter)6, 1);
    router->setRoutingParameter((RoutingParameter)7, 10);
    router->setRoutingParameter(reverseDirectionPenalty, 500);
    router->setRoutingOption((RoutingOption)0, false);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingOption((RoutingOption)2, false);
    router->setRoutingOption((RoutingOption)3, false);
    router->setRoutingOption((RoutingOption)4, true);
    router->setRoutingOption((RoutingOption)5, true);
    Polygon polygon;
    ConnRef *connRef = nullptr;
    ConnEnd srcPt;
    ConnEnd dstPt;
    PolyLine newRoute;
    // shapeRef1
    polygon = Polygon(4);
    polygon.ps[0] = Point(0, 0);
    polygon.ps[1] = Point(0, 0);
    polygon.ps[2] = Point(0, 0);
    polygon.ps[3] = Point(0, 0);
    new ShapeRef(router, polygon, 1);

    // shapeRef2
    polygon = Polygon(4);
    polygon.ps[0] = Point(0, 0);
    polygon.ps[1] = Point(0, 0);
    polygon.ps[2] = Point(0, 0);
    polygon.ps[3] = Point(0, 0);
    new ShapeRef(router, polygon, 2);

    // shapeRef3
    polygon = Polygon(4);
    polygon.ps[0] = Point(0, 0);
    polygon.ps[1] = Point(0, 0);
    polygon.ps[2] = Point(0, 0);
    polygon.ps[3] = Point(0, 0);
    new ShapeRef(router, polygon, 3);

    // shapeRef4
    polygon = Polygon(4);
    polygon.ps[0] = Point(0, 0);
    polygon.ps[1] = Point(0, 0);
    polygon.ps[2] = Point(0, 0);
    polygon.ps[3] = Point(0, 0);
    new ShapeRef(router, polygon, 4);

    // shapeRef5
    polygon = Polygon(4);
    polygon.ps[0] = Point(501, 345);
    polygon.ps[1] = Point(501, 404);
    polygon.ps[2] = Point(421, 404);
    polygon.ps[3] = Point(421, 345);
    ShapeRef *shapeRef5 = new ShapeRef(router, polygon, 5);
    new ShapeConnectionPin(shapeRef5, 5, 1, 0.652542, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef5, 6, 0, 0.79096, true, 0, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef5, 7, 0, 0.514124, true, 0, (ConnDirFlags) 4);

    // shapeRef6
    polygon = Polygon(4);
    polygon.ps[0] = Point(94, 251.5);
    polygon.ps[1] = Point(94, 315.5);
    polygon.ps[2] = Point(12, 315.5);
    polygon.ps[3] = Point(12, 251.5);
    ShapeRef *shapeRef6 = new ShapeRef(router, polygon, 6);
    new ShapeConnectionPin(shapeRef6, 8, 1, 0.640625, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef6, 9, 0, 0.640625, true, 0, (ConnDirFlags) 4);

    // shapeRef7
    polygon = Polygon(4);
    polygon.ps[0] = Point(634.366, 262);
    polygon.ps[1] = Point(634.366, 305);
    polygon.ps[2] = Point(416.366, 305);
    polygon.ps[3] = Point(416.366, 262);
    ShapeRef *shapeRef7 = new ShapeRef(router, polygon, 7);
    new ShapeConnectionPin(shapeRef7, 10, 1, 0.709302, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef7, 11, 0, 0.709302, true, 0, (ConnDirFlags) 4);

    // shapeRef8
    polygon = Polygon(4);
    polygon.ps[0] = Point(324, 147.167);
    polygon.ps[1] = Point(324, 206.167);
    polygon.ps[2] = Point(236, 206.167);
    polygon.ps[3] = Point(236, 147.167);
    ShapeRef *shapeRef8 = new ShapeRef(router, polygon, 8);
    new ShapeConnectionPin(shapeRef8, 12, 1, 0.652542, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef8, 13, 0, 0.79096, true, 0, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef8, 14, 0, 0.514124, true, 0, (ConnDirFlags) 4);

    // shapeRef9
    polygon = Polygon(4);
    polygon.ps[0] = Point(816, 353.167);
    polygon.ps[1] = Point(816, 412.167);
    polygon.ps[2] = Point(735, 412.167);
    polygon.ps[3] = Point(735, 353.167);
    ShapeRef *shapeRef9 = new ShapeRef(router, polygon, 9);
    new ShapeConnectionPin(shapeRef9, 15, 1, 0.514124, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef9, 16, 1, 0.79096, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef9, 17, 0, 0.79096, true, 0, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef9, 18, 0, 0.514124, true, 0, (ConnDirFlags) 4);

    // shapeRef10
    polygon = Polygon(4);
    polygon.ps[0] = Point(981, 263.833);
    polygon.ps[1] = Point(981, 321.833);
    polygon.ps[2] = Point(828, 321.833);
    polygon.ps[3] = Point(828, 263.833);
    ShapeRef *shapeRef10 = new ShapeRef(router, polygon, 10);
    new ShapeConnectionPin(shapeRef10, 19, 0, 0.655172, true, 0, (ConnDirFlags) 4);

    // shapeRef11
    polygon = Polygon(4);
    polygon.ps[0] = Point(1011.49, 361.833);
    polygon.ps[1] = Point(1011.49, 419.833);
    polygon.ps[2] = Point(834.489, 419.833);
    polygon.ps[3] = Point(834.489, 361.833);
    ShapeRef *shapeRef11 = new ShapeRef(router, polygon, 11);
    new ShapeConnectionPin(shapeRef11, 20, 0, 0.655172, true, 0, (ConnDirFlags) 4);

    // shapeRef12
    polygon = Polygon(4);
    polygon.ps[0] = Point(511, 155.333);
    polygon.ps[1] = Point(511, 214.333);
    polygon.ps[2] = Point(422, 214.333);
    polygon.ps[3] = Point(422, 155.333);
    ShapeRef *shapeRef12 = new ShapeRef(router, polygon, 12);
    new ShapeConnectionPin(shapeRef12, 21, 1, 0.514124, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef12, 22, 1, 0.79096, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef12, 23, 0, 0.79096, true, 0, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef12, 24, 0, 0.514124, true, 0, (ConnDirFlags) 4);

    // shapeRef13
    polygon = Polygon(4);
    polygon.ps[0] = Point(690, 66);
    polygon.ps[1] = Point(690, 124);
    polygon.ps[2] = Point(523, 124);
    polygon.ps[3] = Point(523, 66);
    ShapeRef *shapeRef13 = new ShapeRef(router, polygon, 13);
    new ShapeConnectionPin(shapeRef13, 25, 0, 0.655172, true, 0, (ConnDirFlags) 4);

    // shapeRef14
    polygon = Polygon(4);
    polygon.ps[0] = Point(720.212, 164);
    polygon.ps[1] = Point(720.212, 222);
    polygon.ps[2] = Point(529.212, 222);
    polygon.ps[3] = Point(529.212, 164);
    ShapeRef *shapeRef14 = new ShapeRef(router, polygon, 14);
    new ShapeConnectionPin(shapeRef14, 26, 0, 0.655172, true, 0, (ConnDirFlags) 4);

    // shapeRef15
    polygon = Polygon(4);
    polygon.ps[0] = Point(217, 336.833);
    polygon.ps[1] = Point(217, 395.833);
    polygon.ps[2] = Point(98, 395.833);
    polygon.ps[3] = Point(98, 336.833);
    ShapeRef *shapeRef15 = new ShapeRef(router, polygon, 15);
    new ShapeConnectionPin(shapeRef15, 27, 1, 0.652542, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef15, 28, 0, 0.652542, true, 0, (ConnDirFlags) 4);

    // shapeRef16
    polygon = Polygon(4);
    polygon.ps[0] = Point(413, 147.167);
    polygon.ps[1] = Point(413, 206.167);
    polygon.ps[2] = Point(336, 206.167);
    polygon.ps[3] = Point(336, 147.167);
    ShapeRef *shapeRef16 = new ShapeRef(router, polygon, 16);
    new ShapeConnectionPin(shapeRef16, 29, 1, 0.652542, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef16, 30, 0, 0.652542, true, 0, (ConnDirFlags) 4);

    // shapeRef17
    polygon = Polygon(4);
    polygon.ps[0] = Point(306, 336.833);
    polygon.ps[1] = Point(306, 395.833);
    polygon.ps[2] = Point(229, 395.833);
    polygon.ps[3] = Point(229, 336.833);
    ShapeRef *shapeRef17 = new ShapeRef(router, polygon, 17);
    new ShapeConnectionPin(shapeRef17, 31, 1, 0.652542, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef17, 32, 0, 0.652542, true, 0, (ConnDirFlags) 4);

    // shapeRef18
    polygon = Polygon(4);
    polygon.ps[0] = Point(175, 139);
    polygon.ps[1] = Point(175, 198);
    polygon.ps[2] = Point(98, 198);
    polygon.ps[3] = Point(98, 139);
    ShapeRef *shapeRef18 = new ShapeRef(router, polygon, 18);
    new ShapeConnectionPin(shapeRef18, 33, 1, 0.652542, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef18, 34, 0, 0.652542, true, 0, (ConnDirFlags) 4);

    // shapeRef19
    polygon = Polygon(4);
    polygon.ps[0] = Point(409, 399.333);
    polygon.ps[1] = Point(409, 458.333);
    polygon.ps[2] = Point(298, 458.333);
    polygon.ps[3] = Point(298, 399.333);
    ShapeRef *shapeRef19 = new ShapeRef(router, polygon, 19);
    new ShapeConnectionPin(shapeRef19, 35, 1, 0.652542, true, 0, (ConnDirFlags) 8);

    // shapeRef20
    polygon = Polygon(4);
    polygon.ps[0] = Point(224, 40);
    polygon.ps[1] = Point(224, 99);
    polygon.ps[2] = Point(106, 99);
    polygon.ps[3] = Point(106, 40);
    ShapeRef *shapeRef20 = new ShapeRef(router, polygon, 20);
    new ShapeConnectionPin(shapeRef20, 36, 1, 0.652542, true, 0, (ConnDirFlags) 8);

    // shapeRef21
    polygon = Polygon(4);
    polygon.ps[0] = Point(604, 345);
    polygon.ps[1] = Point(604, 404);
    polygon.ps[2] = Point(513, 404);
    polygon.ps[3] = Point(513, 345);
    ShapeRef *shapeRef21 = new ShapeRef(router, polygon, 21);
    new ShapeConnectionPin(shapeRef21, 37, 1, 0.652542, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef21, 38, 0, 0.652542, true, 0, (ConnDirFlags) 4);

    // connRef1
    connRef = new ConnRef(router, 1);
    srcPt = ConnEnd(shapeRef5, 5);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef21, 38);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef2
    connRef = new ConnRef(router, 2);
    srcPt = ConnEnd(shapeRef6, 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef18, 34);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef3
    connRef = new ConnRef(router, 3);
    srcPt = ConnEnd(shapeRef6, 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef15, 28);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef4
    connRef = new ConnRef(router, 4);
    srcPt = ConnEnd(shapeRef6, 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef12, 23);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef5
    connRef = new ConnRef(router, 5);
    srcPt = ConnEnd(shapeRef6, 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef7, 11);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef6
    connRef = new ConnRef(router, 6);
    srcPt = ConnEnd(shapeRef7, 10);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef9, 17);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    ConnRef *connector6 = connRef;

    // connRef7
    connRef = new ConnRef(router, 7);
    srcPt = ConnEnd(shapeRef8, 12);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef16, 30);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef8
    connRef = new ConnRef(router, 8);
    srcPt = ConnEnd(shapeRef9, 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef10, 19);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef9
    connRef = new ConnRef(router, 9);
    srcPt = ConnEnd(shapeRef9, 16);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef11, 20);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef10
    connRef = new ConnRef(router, 10);
    srcPt = ConnEnd(shapeRef12, 21);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef13, 25);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef11
    connRef = new ConnRef(router, 11);
    srcPt = ConnEnd(shapeRef12, 22);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef14, 26);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef12
    connRef = new ConnRef(router, 12);
    srcPt = ConnEnd(shapeRef15, 27);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef17, 32);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef13
    connRef = new ConnRef(router, 13);
    srcPt = ConnEnd(shapeRef16, 29);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef12, 24);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef14
    connRef = new ConnRef(router, 14);
    srcPt = ConnEnd(shapeRef17, 31);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef5, 7);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef15
    connRef = new ConnRef(router, 15);
    srcPt = ConnEnd(shapeRef18, 33);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef8, 14);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef16
    connRef = new ConnRef(router, 16);
    srcPt = ConnEnd(shapeRef19, 35);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef5, 7);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef17
    connRef = new ConnRef(router, 17);
    srcPt = ConnEnd(shapeRef20, 36);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef8, 14);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef18
    connRef = new ConnRef(router, 18);
    srcPt = ConnEnd(shapeRef21, 37);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(shapeRef9, 18);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    router->processTransaction();
    // Test that connector 6 has three segments and doesnt loop right
    // around the shapes on the right due to the crossing penalty.
    bool suceeds = (connector6->displayRoute().size() == 4);

    //router->outputDiagram("output/forwardFlowingConnectors01");
    delete router;
    return (suceeds ? 0 : 1);
};
