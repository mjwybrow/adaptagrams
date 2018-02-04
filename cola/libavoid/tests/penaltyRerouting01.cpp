#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
    router->setRoutingParameter((RoutingParameter)0, 10);
    router->setRoutingParameter((RoutingParameter)1, 0);
    router->setRoutingParameter((RoutingParameter)2, 1e+09);
    router->setRoutingParameter((RoutingParameter)3, 4000);
    router->setRoutingParameter((RoutingParameter)4, 0);
    router->setRoutingParameter((RoutingParameter)5, 0);
    router->setRoutingParameter((RoutingParameter)6, 0);
    router->setRoutingParameter((RoutingParameter)7, 4);
    router->setRoutingParameter((RoutingParameter)8, 0);
    router->setRoutingOption((RoutingOption)0, false);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingOption((RoutingOption)2, false);
    router->setRoutingOption((RoutingOption)3, false);
    router->setRoutingOption((RoutingOption)4, true);
    router->setRoutingOption((RoutingOption)5, false);
    router->setRoutingOption((RoutingOption)6, true);
    Polygon polygon;
    ConnRef *connRef = nullptr;
    ConnEnd srcPt;
    ConnEnd dstPt;
    ConnEnd heConnPt;
    PolyLine newRoute;

    // shapeRef1
    polygon = Polygon(4);
    polygon.ps[0] = Point(449, 491);
    polygon.ps[1] = Point(479, 491);
    polygon.ps[2] = Point(479, 521);
    polygon.ps[3] = Point(449, 521);
    new ShapeRef(router, polygon, 1);

    // shapeRef1
    polygon = Polygon(4);
    polygon.ps[0] = Point(449, 583);
    polygon.ps[1] = Point(479, 583);
    polygon.ps[2] = Point(479, 613);
    polygon.ps[3] = Point(449, 613);
    new ShapeRef(router, polygon, 1);

    // shapeRef2
    polygon = Polygon(4);
    polygon.ps[0] = Point(354, 583);
    polygon.ps[1] = Point(384, 583);
    polygon.ps[2] = Point(384, 613);
    polygon.ps[3] = Point(354, 613);
    new ShapeRef(router, polygon, 2);

    // shapeRef3
    polygon = Polygon(4);
    polygon.ps[0] = Point(354, 354);
    polygon.ps[1] = Point(384, 354);
    polygon.ps[2] = Point(384, 384);
    polygon.ps[3] = Point(354, 384);
    new ShapeRef(router, polygon, 3);

    // shapeRef4
    polygon = Polygon(4);
    polygon.ps[0] = Point(252.5, 354);
    polygon.ps[1] = Point(282.5, 354);
    polygon.ps[2] = Point(282.5, 384);
    polygon.ps[3] = Point(252.5, 384);
    new ShapeRef(router, polygon, 4);

    // shapeRef5
    polygon = Polygon(4);
    polygon.ps[0] = Point(252.5, 583);
    polygon.ps[1] = Point(282.5, 583);
    polygon.ps[2] = Point(282.5, 613);
    polygon.ps[3] = Point(252.5, 613);
    new ShapeRef(router, polygon, 5);

    // shapeRef6
    polygon = Polygon(4);
    polygon.ps[0] = Point(151, 583);
    polygon.ps[1] = Point(181, 583);
    polygon.ps[2] = Point(181, 613);
    polygon.ps[3] = Point(151, 613);
    new ShapeRef(router, polygon, 6);

    // shapeRef7
    polygon = Polygon(4);
    polygon.ps[0] = Point(151, 130);
    polygon.ps[1] = Point(181, 130);
    polygon.ps[2] = Point(181, 160);
    polygon.ps[3] = Point(151, 160);
    new ShapeRef(router, polygon, 7);

    // shapeRef8
    polygon = Polygon(4);
    polygon.ps[0] = Point(520, 491);
    polygon.ps[1] = Point(550, 491);
    polygon.ps[2] = Point(550, 521);
    polygon.ps[3] = Point(520, 521);
    new ShapeRef(router, polygon, 8);

    // shapeRef9
    polygon = Polygon(4);
    polygon.ps[0] = Point(520, 354);
    polygon.ps[1] = Point(550, 354);
    polygon.ps[2] = Point(550, 384);
    polygon.ps[3] = Point(520, 384);
    new ShapeRef(router, polygon, 9);

    // shapeRef10
    polygon = Polygon(4);
    polygon.ps[0] = Point(633, 354);
    polygon.ps[1] = Point(663, 354);
    polygon.ps[2] = Point(663, 384);
    polygon.ps[3] = Point(633, 384);
    new ShapeRef(router, polygon, 10);

    // shapeRef11
    polygon = Polygon(4);
    polygon.ps[0] = Point(633, 583);
    polygon.ps[1] = Point(663, 583);
    polygon.ps[2] = Point(663, 613);
    polygon.ps[3] = Point(633, 613);
    new ShapeRef(router, polygon, 11);

    // shapeRef12
    polygon = Polygon(4);
    polygon.ps[0] = Point(741, 583);
    polygon.ps[1] = Point(771, 583);
    polygon.ps[2] = Point(771, 613);
    polygon.ps[3] = Point(741, 613);
    new ShapeRef(router, polygon, 12);

    // shapeRef13
    polygon = Polygon(4);
    polygon.ps[0] = Point(741, 130);
    polygon.ps[1] = Point(771, 130);
    polygon.ps[2] = Point(771, 160);
    polygon.ps[3] = Point(741, 160);
    new ShapeRef(router, polygon, 13);

    // shapeRef14
    polygon = Polygon(4);
    polygon.ps[0] = Point(459.495, 481.99);
    polygon.ps[1] = Point(468.505, 481.99);
    polygon.ps[2] = Point(468.505, 491);
    polygon.ps[3] = Point(459.495, 491);
    new ShapeRef(router, polygon, 14);

    // shapeRef15
    polygon = Polygon(4);
    polygon.ps[0] = Point(212.495, 481.99);
    polygon.ps[1] = Point(221.505, 481.99);
    polygon.ps[2] = Point(221.505, 491);
    polygon.ps[3] = Point(212.495, 491);
    new ShapeRef(router, polygon, 15);

    // shapeRef16
    polygon = Polygon(4);
    polygon.ps[0] = Point(701.99, 481.99);
    polygon.ps[1] = Point(711, 481.99);
    polygon.ps[2] = Point(711, 491);
    polygon.ps[3] = Point(701.99, 491);
    new ShapeRef(router, polygon, 16);

    // connRef17
    connRef = new ConnRef(router, 17);
    srcPt = ConnEnd(Point(464, 598), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(369, 598), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 17;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(464, 598);
    newRoute.ps[1] = Point(369, 598);
    connRef->setFixedRoute(newRoute);

    // connRef18
    connRef = new ConnRef(router, 18);
    srcPt = ConnEnd(Point(267.5, 598), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(166, 598), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 18;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(267.5, 598);
    newRoute.ps[1] = Point(166, 598);
    connRef->setFixedRoute(newRoute);

    // connRef19
    connRef = new ConnRef(router, 19);
    srcPt = ConnEnd(Point(369, 598), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(369, 369), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 19;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(369, 598);
    newRoute.ps[1] = Point(369, 369);
    connRef->setFixedRoute(newRoute);

    // connRef20
    connRef = new ConnRef(router, 20);
    srcPt = ConnEnd(Point(464, 506), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(464, 598), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 20;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(464, 506);
    newRoute.ps[1] = Point(464, 598);
    connRef->setFixedRoute(newRoute);

    // connRef21
    connRef = new ConnRef(router, 21);
    srcPt = ConnEnd(Point(756, 598), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(756, 145), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 21;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(756, 598);
    newRoute.ps[1] = Point(756, 145);
    connRef->setFixedRoute(newRoute);

    // connRef22
    connRef = new ConnRef(router, 22);
    srcPt = ConnEnd(Point(369, 369), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(267.5, 369), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 22;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(369, 369);
    newRoute.ps[1] = Point(267.5, 369);
    connRef->setFixedRoute(newRoute);

    // connRef23
    connRef = new ConnRef(router, 23);
    srcPt = ConnEnd(Point(166, 145), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(756, 145), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 23;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(166, 145);
    newRoute.ps[1] = Point(756, 145);
    connRef->setFixedRoute(newRoute);

    // connRef24
    connRef = new ConnRef(router, 24);
    srcPt = ConnEnd(Point(464, 486.495), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(706.495, 486.495), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef25
    connRef = new ConnRef(router, 25);
    srcPt = ConnEnd(Point(535, 369), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(535, 506), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 25;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(535, 369);
    newRoute.ps[1] = Point(535, 506);
    connRef->setFixedRoute(newRoute);

    // connRef26
    connRef = new ConnRef(router, 26);
    srcPt = ConnEnd(Point(648, 598), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(648, 369), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 26;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(648, 598);
    newRoute.ps[1] = Point(648, 369);
    connRef->setFixedRoute(newRoute);

    // connRef27
    connRef = new ConnRef(router, 27);
    srcPt = ConnEnd(Point(464, 486.495), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(217, 486.495), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef28
    connRef = new ConnRef(router, 28);
    srcPt = ConnEnd(Point(648, 598), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(756, 598), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 28;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(648, 598);
    newRoute.ps[1] = Point(756, 598);
    connRef->setFixedRoute(newRoute);

    // connRef29
    connRef = new ConnRef(router, 29);
    srcPt = ConnEnd(Point(166, 598), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(166, 145), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 29;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(166, 598);
    newRoute.ps[1] = Point(166, 145);
    connRef->setFixedRoute(newRoute);

    // connRef30
    connRef = new ConnRef(router, 30);
    srcPt = ConnEnd(Point(648, 369), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(535, 369), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 30;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(648, 369);
    newRoute.ps[1] = Point(535, 369);
    connRef->setFixedRoute(newRoute);

    // connRef31
    connRef = new ConnRef(router, 31);
    srcPt = ConnEnd(Point(267.5, 369), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(267.5, 598), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 31;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(267.5, 369);
    newRoute.ps[1] = Point(267.5, 598);
    connRef->setFixedRoute(newRoute);

    // connRef32
    connRef = new ConnRef(router, 32);
    srcPt = ConnEnd(Point(535, 506), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(464, 506), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    newRoute._id = 32;
    newRoute.ps.resize(2);
    newRoute.ps[0] = Point(535, 506);
    newRoute.ps[1] = Point(464, 506);
    connRef->setFixedRoute(newRoute);

    router->processTransaction();
    router->outputDiagram("output/penaltyRerouting01");

    int crossings = router->existsCrossings();

    delete router;
    return (crossings > 0) ? 1 : 0;
};
