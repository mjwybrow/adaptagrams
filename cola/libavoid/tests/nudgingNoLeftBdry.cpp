#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
    router->setRoutingParameter((RoutingParameter)0, 58.6522);
    router->setRoutingParameter((RoutingParameter)1, 0);
    router->setRoutingParameter((RoutingParameter)2, 234.609);
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
    
    // This is the `nudgeSharedPathsWithCommonEndPoint` option:
    router->setRoutingOption((RoutingOption)6, false);
    
    Polygon polygon;
    ConnRef *connRef = nullptr;
    ConnEnd srcPt;
    ConnEnd dstPt;
    ConnEnd heConnPt;
    PolyLine newRoute;
    ShapeConnectionPin *connPin = nullptr;

    // shapeRef1
    polygon = Polygon(4);
    polygon.ps[0] = Point(578.491, -431.639);
    polygon.ps[1] = Point(642.817, -431.639);
    polygon.ps[2] = Point(642.817, -368.313);
    polygon.ps[3] = Point(578.491, -368.313);
    new ShapeRef(router, polygon, 1);

    // shapeRef2
    polygon = Polygon(4);
    polygon.ps[0] = Point(22.0455, -300.876);
    polygon.ps[1] = Point(86.3716, -300.876);
    polygon.ps[2] = Point(86.3716, -237.55);
    polygon.ps[3] = Point(22.0455, -237.55);
    new ShapeRef(router, polygon, 2);

    // shapeRef3
    polygon = Polygon(4);
    polygon.ps[0] = Point(543.991, -76.2815);
    polygon.ps[1] = Point(677.317, -76.2815);
    polygon.ps[2] = Point(677.317, -12.9554);
    polygon.ps[3] = Point(543.991, -12.9554);
    new ShapeRef(router, polygon, 3);

    // shapeRef4
    polygon = Polygon(4);
    polygon.ps[0] = Point(-177.617, -394.976);
    polygon.ps[1] = Point(-102.291, -394.976);
    polygon.ps[2] = Point(-102.291, -331.65);
    polygon.ps[3] = Point(-177.617, -331.65);
    new ShapeRef(router, polygon, 4);

    // shapeRef5
    polygon = Polygon(4);
    polygon.ps[0] = Point(101.035, -241.749);
    polygon.ps[1] = Point(163.361, -241.749);
    polygon.ps[2] = Point(163.361, -178.423);
    polygon.ps[3] = Point(101.035, -178.423);
    new ShapeRef(router, polygon, 5);

    // shapeRef6
    polygon = Polygon(4);
    polygon.ps[0] = Point(178.024, -227.091);
    polygon.ps[1] = Point(275.35, -227.091);
    polygon.ps[2] = Point(275.35, -163.765);
    polygon.ps[3] = Point(178.024, -163.765);
    new ShapeRef(router, polygon, 6);

    // shapeRef7
    polygon = Polygon(4);
    polygon.ps[0] = Point(492.047, -431.639);
    polygon.ps[1] = Point(558.374, -431.639);
    polygon.ps[2] = Point(558.374, -368.313);
    polygon.ps[3] = Point(492.047, -368.313);
    new ShapeRef(router, polygon, 7);

    // shapeRef8
    polygon = Polygon(4);
    polygon.ps[0] = Point(184.524, -516.954);
    polygon.ps[1] = Point(268.85, -516.954);
    polygon.ps[2] = Point(268.85, -453.628);
    polygon.ps[3] = Point(184.524, -453.628);
    new ShapeRef(router, polygon, 8);

    // shapeRef9
    polygon = Polygon(4);
    polygon.ps[0] = Point(381.002, -264.213);
    polygon.ps[1] = Point(478.328, -264.213);
    polygon.ps[2] = Point(478.328, -200.887);
    polygon.ps[3] = Point(381.002, -200.887);
    new ShapeRef(router, polygon, 9);

    // shapeRef10
    polygon = Polygon(4);
    polygon.ps[0] = Point(372.348, -76.2815);
    polygon.ps[1] = Point(464.674, -76.2815);
    polygon.ps[2] = Point(464.674, -12.9554);
    polygon.ps[3] = Point(372.348, -12.9554);
    new ShapeRef(router, polygon, 10);

    // shapeRef11
    polygon = Polygon(4);
    polygon.ps[0] = Point(380.058, -458.302);
    polygon.ps[1] = Point(477.384, -458.302);
    polygon.ps[2] = Point(477.384, -394.976);
    polygon.ps[3] = Point(380.058, -394.976);
    new ShapeRef(router, polygon, 11);

    // shapeRef12
    polygon = Polygon(4);
    polygon.ps[0] = Point(290.013, -264.213);
    polygon.ps[1] = Point(366.339, -264.213);
    polygon.ps[2] = Point(366.339, -200.887);
    polygon.ps[3] = Point(290.013, -200.887);
    new ShapeRef(router, polygon, 12);

    // shapeRef13
    polygon = Polygon(4);
    polygon.ps[0] = Point(492.991, -300.876);
    polygon.ps[1] = Point(728.317, -300.876);
    polygon.ps[2] = Point(728.317, -237.55);
    polygon.ps[3] = Point(492.991, -237.55);
    new ShapeRef(router, polygon, 13);

    // shapeRef14
    polygon = Polygon(4);
    polygon.ps[0] = Point(-28.9762, -394.976);
    polygon.ps[1] = Point(482.35, -394.976);
    polygon.ps[2] = Point(482.35, -331.65);
    polygon.ps[3] = Point(-28.9762, -331.65);
    new ShapeRef(router, polygon, 14);

    // shapeRef15
    polygon = Polygon(4);
    polygon.ps[0] = Point(801.633, -300.876);
    polygon.ps[1] = Point(988.959, -300.876);
    polygon.ps[2] = Point(988.959, -237.55);
    polygon.ps[3] = Point(801.633, -237.55);
    new ShapeRef(router, polygon, 15);

    // shapeRef16
    polygon = Polygon(4);
    polygon.ps[0] = Point(657.48, -394.976);
    polygon.ps[1] = Point(781.806, -394.976);
    polygon.ps[2] = Point(781.806, -331.65);
    polygon.ps[3] = Point(657.48, -331.65);
    new ShapeRef(router, polygon, 16);

    // shapeRef17
    polygon = Polygon(4);
    polygon.ps[0] = Point(837.633, -76.2815);
    polygon.ps[1] = Point(952.959, -76.2815);
    polygon.ps[2] = Point(952.959, -12.9554);
    polygon.ps[3] = Point(837.633, -12.9554);
    new ShapeRef(router, polygon, 17);

    // shapeRef18
    polygon = Polygon(4);
    polygon.ps[0] = Point(292.069, -458.302);
    polygon.ps[1] = Point(365.395, -458.302);
    polygon.ps[2] = Point(365.395, -394.976);
    polygon.ps[3] = Point(292.069, -394.976);
    new ShapeRef(router, polygon, 18);

    // connRef19
    connRef = new ConnRef(router, 19);
    srcPt = ConnEnd(Point(610.654, -399.976), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef20
    connRef = new ConnRef(router, 20);
    srcPt = ConnEnd(Point(610.654, -399.976), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef21
    connRef = new ConnRef(router, 21);
    srcPt = ConnEnd(Point(54.2086, -269.213), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef22
    connRef = new ConnRef(router, 22);
    srcPt = ConnEnd(Point(54.2086, -269.213), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef23
    connRef = new ConnRef(router, 23);
    srcPt = ConnEnd(Point(610.654, -44.6185), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef24
    connRef = new ConnRef(router, 24);
    srcPt = ConnEnd(Point(610.654, -44.6185), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef25
    connRef = new ConnRef(router, 25);
    srcPt = ConnEnd(Point(610.654, -44.6185), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(895.296, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef26
    connRef = new ConnRef(router, 26);
    srcPt = ConnEnd(Point(610.654, -44.6185), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(895.296, -44.6185), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef27
    connRef = new ConnRef(router, 27);
    srcPt = ConnEnd(Point(-139.954, -363.313), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef28
    connRef = new ConnRef(router, 28);
    srcPt = ConnEnd(Point(-139.954, -363.313), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef29
    connRef = new ConnRef(router, 29);
    srcPt = ConnEnd(Point(132.198, -210.086), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef30
    connRef = new ConnRef(router, 30);
    srcPt = ConnEnd(Point(132.198, -210.086), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef31
    connRef = new ConnRef(router, 31);
    srcPt = ConnEnd(Point(226.687, -195.428), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef32
    connRef = new ConnRef(router, 32);
    srcPt = ConnEnd(Point(226.687, -195.428), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef33
    connRef = new ConnRef(router, 33);
    srcPt = ConnEnd(Point(525.211, -399.976), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef34
    connRef = new ConnRef(router, 34);
    srcPt = ConnEnd(Point(525.211, -399.976), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef35
    connRef = new ConnRef(router, 35);
    srcPt = ConnEnd(Point(226.687, -485.291), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef36
    connRef = new ConnRef(router, 36);
    srcPt = ConnEnd(Point(226.687, -485.291), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef37
    connRef = new ConnRef(router, 37);
    srcPt = ConnEnd(Point(429.665, -232.55), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef38
    connRef = new ConnRef(router, 38);
    srcPt = ConnEnd(Point(429.665, -232.55), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef39
    connRef = new ConnRef(router, 39);
    srcPt = ConnEnd(Point(418.511, -44.6185), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef40
    connRef = new ConnRef(router, 40);
    srcPt = ConnEnd(Point(418.511, -44.6185), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef41
    connRef = new ConnRef(router, 41);
    srcPt = ConnEnd(Point(428.721, -426.639), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef42
    connRef = new ConnRef(router, 42);
    srcPt = ConnEnd(Point(428.721, -426.639), 7);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef43
    connRef = new ConnRef(router, 43);
    srcPt = ConnEnd(Point(328.176, -232.55), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef44
    connRef = new ConnRef(router, 44);
    srcPt = ConnEnd(Point(328.176, -232.55), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef45
    connRef = new ConnRef(router, 45);
    srcPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef46
    connRef = new ConnRef(router, 46);
    srcPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(895.296, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef47
    connRef = new ConnRef(router, 47);
    srcPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(719.643, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef48
    connRef = new ConnRef(router, 48);
    srcPt = ConnEnd(Point(610.654, -269.213), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(328.732, -426.639), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef49
    connRef = new ConnRef(router, 49);
    srcPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(895.296, -269.213), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef50
    connRef = new ConnRef(router, 50);
    srcPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(719.643, -363.313), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef51
    connRef = new ConnRef(router, 51);
    srcPt = ConnEnd(Point(226.687, -363.313), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(328.732, -426.639), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef52
    connRef = new ConnRef(router, 52);
    srcPt = ConnEnd(Point(895.296, -269.213), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(895.296, -44.6185), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    
    router->processTransaction();
    router->outputInstanceToSVG("output/nudgingNoLeftBdry");
    delete router;
    return 0;
};
