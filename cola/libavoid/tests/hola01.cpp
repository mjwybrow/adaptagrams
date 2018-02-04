// Regression test exercises a problem with nudging that would result in the 
// following assertion if a sub-problem was unsatisfiable because the right 
// channel edge but not the left had moved.
//    Assertion failed: (unsatisfiedRanges.size() > 0)
// Reported by Steve Kieffer and Tobias Czauderna.
//
// Problem happens if EDGE128 is defined.

#include "libavoid/libavoid.h"

using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
    router->setRoutingParameter((RoutingParameter)0, 30);
    router->setRoutingParameter((RoutingParameter)1, 0);
    router->setRoutingParameter((RoutingParameter)2, 120);
    router->setRoutingParameter((RoutingParameter)3, 4000);
    router->setRoutingParameter((RoutingParameter)4, 0);
    router->setRoutingParameter((RoutingParameter)5, 0);
    router->setRoutingParameter((RoutingParameter)6, 7.5);
    router->setRoutingParameter((RoutingParameter)7, 4);
    router->setRoutingParameter((RoutingParameter)8, 0);
    router->setRoutingOption((RoutingOption)0, false);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingOption((RoutingOption)2, false);
    router->setRoutingOption((RoutingOption)3, false);
    router->setRoutingOption((RoutingOption)4, true);
    router->setRoutingOption((RoutingOption)5, false);
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
    polygon.ps[0] = Point(274.03, 356.334);
    polygon.ps[1] = Point(304.03, 356.334);
    polygon.ps[2] = Point(304.03, 386.334);
    polygon.ps[3] = Point(274.03, 386.334);
    new ShapeRef(router, polygon, 1);

    // shapeRef1
    polygon = Polygon(4);
    polygon.ps[0] = Point(214.034, 356.334);
    polygon.ps[1] = Point(244.034, 356.334);
    polygon.ps[2] = Point(244.034, 386.334);
    polygon.ps[3] = Point(214.034, 386.334);
    new ShapeRef(router, polygon, 1);

    // shapeRef2
    polygon = Polygon(4);
    polygon.ps[0] = Point(214.034, 439.168);
    polygon.ps[1] = Point(244.034, 439.168);
    polygon.ps[2] = Point(244.034, 469.168);
    polygon.ps[3] = Point(214.034, 469.168);
    new ShapeRef(router, polygon, 2);

    // shapeRef3
    polygon = Polygon(4);
    polygon.ps[0] = Point(354.024, 416.334);
    polygon.ps[1] = Point(384.024, 416.334);
    polygon.ps[2] = Point(384.024, 446.334);
    polygon.ps[3] = Point(354.024, 446.334);
    new ShapeRef(router, polygon, 3);

    // shapeRef4
    polygon = Polygon(4);
    polygon.ps[0] = Point(274.03, 276.334);
    polygon.ps[1] = Point(304.03, 276.334);
    polygon.ps[2] = Point(304.03, 306.334);
    polygon.ps[3] = Point(274.03, 306.334);
    new ShapeRef(router, polygon, 4);

    // shapeRef5
    polygon = Polygon(4);
    polygon.ps[0] = Point(534.011, 276.334);
    polygon.ps[1] = Point(564.011, 276.334);
    polygon.ps[2] = Point(564.011, 306.334);
    polygon.ps[3] = Point(534.011, 306.334);
    new ShapeRef(router, polygon, 5);

    // shapeRef6
    polygon = Polygon(4);
    polygon.ps[0] = Point(414.023, 416.334);
    polygon.ps[1] = Point(444.023, 416.334);
    polygon.ps[2] = Point(444.023, 446.334);
    polygon.ps[3] = Point(414.023, 446.334);
    new ShapeRef(router, polygon, 6);

    // shapeRef7
    polygon = Polygon(4);
    polygon.ps[0] = Point(594.052, 276.334);
    polygon.ps[1] = Point(624.052, 276.334);
    polygon.ps[2] = Point(624.052, 306.334);
    polygon.ps[3] = Point(594.052, 306.334);
    new ShapeRef(router, polygon, 7);

    // shapeRef8
    polygon = Polygon(4);
    polygon.ps[0] = Point(254.052, 386.334);
    polygon.ps[1] = Point(284.052, 386.334);
    polygon.ps[2] = Point(284.052, 416.334);
    polygon.ps[3] = Point(254.052, 416.334);
    new ShapeRef(router, polygon, 8);

    // shapeRef9
    polygon = Polygon(4);
    polygon.ps[0] = Point(474.011, 416.334);
    polygon.ps[1] = Point(504.011, 416.334);
    polygon.ps[2] = Point(504.011, 446.334);
    polygon.ps[3] = Point(474.011, 446.334);
    new ShapeRef(router, polygon, 9);

    // shapeRef10
    polygon = Polygon(4);
    polygon.ps[0] = Point(274.03, 76.3341);
    polygon.ps[1] = Point(304.03, 76.3341);
    polygon.ps[2] = Point(304.03, 106.334);
    polygon.ps[3] = Point(274.03, 106.334);
    new ShapeRef(router, polygon, 10);

    // shapeRef11
    polygon = Polygon(4);
    polygon.ps[0] = Point(354.034, 536.334);
    polygon.ps[1] = Point(384.034, 536.334);
    polygon.ps[2] = Point(384.034, 566.334);
    polygon.ps[3] = Point(354.034, 566.334);
    new ShapeRef(router, polygon, 11);

    // shapeRef12
    polygon = Polygon(4);
    polygon.ps[0] = Point(414.023, 476.334);
    polygon.ps[1] = Point(444.023, 476.334);
    polygon.ps[2] = Point(444.023, 506.334);
    polygon.ps[3] = Point(414.023, 506.334);
    new ShapeRef(router, polygon, 12);

    // shapeRef13
    polygon = Polygon(4);
    polygon.ps[0] = Point(354.034, 596.334);
    polygon.ps[1] = Point(384.034, 596.334);
    polygon.ps[2] = Point(384.034, 626.334);
    polygon.ps[3] = Point(354.034, 626.334);
    new ShapeRef(router, polygon, 13);

    // shapeRef14
    polygon = Polygon(4);
    polygon.ps[0] = Point(274.03, 446.334);
    polygon.ps[1] = Point(304.03, 446.334);
    polygon.ps[2] = Point(304.03, 476.334);
    polygon.ps[3] = Point(274.03, 476.334);
    new ShapeRef(router, polygon, 14);

    // shapeRef15
    polygon = Polygon(4);
    polygon.ps[0] = Point(131.696, 276.334);
    polygon.ps[1] = Point(161.696, 276.334);
    polygon.ps[2] = Point(161.696, 306.334);
    polygon.ps[3] = Point(131.696, 306.334);
    new ShapeRef(router, polygon, 15);

    // shapeRef16
    polygon = Polygon(4);
    polygon.ps[0] = Point(564.023, 416.334);
    polygon.ps[1] = Point(594.023, 416.334);
    polygon.ps[2] = Point(594.023, 446.334);
    polygon.ps[3] = Point(564.023, 446.334);
    new ShapeRef(router, polygon, 16);

    // shapeRef17
    polygon = Polygon(4);
    polygon.ps[0] = Point(8.56882, 276.334);
    polygon.ps[1] = Point(38.5688, 276.334);
    polygon.ps[2] = Point(38.5688, 306.334);
    polygon.ps[3] = Point(8.56882, 306.334);
    new ShapeRef(router, polygon, 17);

    // shapeRef18
    polygon = Polygon(4);
    polygon.ps[0] = Point(474.011, 216.334);
    polygon.ps[1] = Point(504.011, 216.334);
    polygon.ps[2] = Point(504.011, 246.334);
    polygon.ps[3] = Point(474.011, 246.334);
    new ShapeRef(router, polygon, 18);

    // shapeRef19
    polygon = Polygon(4);
    polygon.ps[0] = Point(474.011, 156.334);
    polygon.ps[1] = Point(504.011, 156.334);
    polygon.ps[2] = Point(504.011, 186.334);
    polygon.ps[3] = Point(474.011, 186.334);
    new ShapeRef(router, polygon, 19);

    // shapeRef20
    polygon = Polygon(4);
    polygon.ps[0] = Point(534.011, 216.334);
    polygon.ps[1] = Point(564.011, 216.334);
    polygon.ps[2] = Point(564.011, 246.334);
    polygon.ps[3] = Point(534.011, 246.334);
    new ShapeRef(router, polygon, 20);

    // shapeRef21
    polygon = Polygon(4);
    polygon.ps[0] = Point(474.011, 96.3341);
    polygon.ps[1] = Point(504.011, 96.3341);
    polygon.ps[2] = Point(504.011, 126.334);
    polygon.ps[3] = Point(474.011, 126.334);
    new ShapeRef(router, polygon, 21);

    // shapeRef22
    polygon = Polygon(4);
    polygon.ps[0] = Point(334.035, 356.334);
    polygon.ps[1] = Point(364.035, 356.334);
    polygon.ps[2] = Point(364.035, 386.334);
    polygon.ps[3] = Point(334.035, 386.334);
    new ShapeRef(router, polygon, 22);

    // shapeRef23
    polygon = Polygon(4);
    polygon.ps[0] = Point(414.023, 536.334);
    polygon.ps[1] = Point(444.023, 536.334);
    polygon.ps[2] = Point(444.023, 566.334);
    polygon.ps[3] = Point(414.023, 566.334);
    new ShapeRef(router, polygon, 23);

    // shapeRef25
    polygon = Polygon(4);
    polygon.ps[0] = Point(564.052, 180.582);
    polygon.ps[1] = Point(594.052, 180.582);
    polygon.ps[2] = Point(594.052, 210.582);
    polygon.ps[3] = Point(564.052, 210.582);
    new ShapeRef(router, polygon, 25);

    // shapeRef26
    polygon = Polygon(4);
    polygon.ps[0] = Point(154.041, 476.334);
    polygon.ps[1] = Point(184.041, 476.334);
    polygon.ps[2] = Point(184.041, 506.334);
    polygon.ps[3] = Point(154.041, 506.334);
    new ShapeRef(router, polygon, 26);

    // shapeRef27
    polygon = Polygon(4);
    polygon.ps[0] = Point(474.011, 300.457);
    polygon.ps[1] = Point(504.011, 300.457);
    polygon.ps[2] = Point(504.011, 330.457);
    polygon.ps[3] = Point(474.011, 330.457);
    new ShapeRef(router, polygon, 27);

    // shapeRef28
    polygon = Polygon(4);
    polygon.ps[0] = Point(134.045, 306.334);
    polygon.ps[1] = Point(164.045, 306.334);
    polygon.ps[2] = Point(164.045, 336.334);
    polygon.ps[3] = Point(134.045, 336.334);
    new ShapeRef(router, polygon, 28);

    // shapeRef29
    polygon = Polygon(4);
    polygon.ps[0] = Point(254.039, 216.334);
    polygon.ps[1] = Point(284.039, 216.334);
    polygon.ps[2] = Point(284.039, 246.334);
    polygon.ps[3] = Point(254.039, 246.334);
    new ShapeRef(router, polygon, 29);

    // shapeRef30
    polygon = Polygon(4);
    polygon.ps[0] = Point(134.045, 386.334);
    polygon.ps[1] = Point(164.045, 386.334);
    polygon.ps[2] = Point(164.045, 416.334);
    polygon.ps[3] = Point(134.045, 416.334);
    new ShapeRef(router, polygon, 30);

    // shapeRef31
    polygon = Polygon(4);
    polygon.ps[0] = Point(74.0462, 386.334);
    polygon.ps[1] = Point(104.046, 386.334);
    polygon.ps[2] = Point(104.046, 416.334);
    polygon.ps[3] = Point(74.0462, 416.334);
    new ShapeRef(router, polygon, 31);

    // shapeRef32
    polygon = Polygon(4);
    polygon.ps[0] = Point(594.052, 596.334);
    polygon.ps[1] = Point(624.052, 596.334);
    polygon.ps[2] = Point(624.052, 626.334);
    polygon.ps[3] = Point(594.052, 626.334);
    new ShapeRef(router, polygon, 32);

    // shapeRef34
    polygon = Polygon(4);
    polygon.ps[0] = Point(154.041, 356.334);
    polygon.ps[1] = Point(184.041, 356.334);
    polygon.ps[2] = Point(184.041, 386.334);
    polygon.ps[3] = Point(154.041, 386.334);
    new ShapeRef(router, polygon, 34);

    // shapeRef35
    polygon = Polygon(4);
    polygon.ps[0] = Point(474.011, 36.3341);
    polygon.ps[1] = Point(504.011, 36.3341);
    polygon.ps[2] = Point(504.011, 66.3341);
    polygon.ps[3] = Point(474.011, 66.3341);
    new ShapeRef(router, polygon, 35);

    // shapeRef38
    polygon = Polygon(4);
    polygon.ps[0] = Point(534.011, 386.334);
    polygon.ps[1] = Point(564.011, 386.334);
    polygon.ps[2] = Point(564.011, 416.334);
    polygon.ps[3] = Point(534.011, 416.334);
    new ShapeRef(router, polygon, 38);

    // shapeRef39
    polygon = Polygon(4);
    polygon.ps[0] = Point(564.023, 343.278);
    polygon.ps[1] = Point(594.023, 343.278);
    polygon.ps[2] = Point(594.023, 373.278);
    polygon.ps[3] = Point(564.023, 373.278);
    new ShapeRef(router, polygon, 39);

    // shapeRef40
    polygon = Polygon(4);
    polygon.ps[0] = Point(74.0462, 156.334);
    polygon.ps[1] = Point(104.046, 156.334);
    polygon.ps[2] = Point(104.046, 186.334);
    polygon.ps[3] = Point(74.0462, 186.334);
    new ShapeRef(router, polygon, 40);

    // shapeRef41
    polygon = Polygon(4);
    polygon.ps[0] = Point(214.034, 76.3341);
    polygon.ps[1] = Point(244.034, 76.3341);
    polygon.ps[2] = Point(244.034, 106.334);
    polygon.ps[3] = Point(214.034, 106.334);
    new ShapeRef(router, polygon, 41);

    // shapeRef42
    polygon = Polygon(4);
    polygon.ps[0] = Point(334.035, 136.334);
    polygon.ps[1] = Point(364.035, 136.334);
    polygon.ps[2] = Point(364.035, 166.334);
    polygon.ps[3] = Point(334.035, 166.334);
    new ShapeRef(router, polygon, 42);

    // shapeRef45
    polygon = Polygon(4);
    polygon.ps[0] = Point(354.024, 300.457);
    polygon.ps[1] = Point(384.024, 300.457);
    polygon.ps[2] = Point(384.024, 330.457);
    polygon.ps[3] = Point(354.024, 330.457);
    new ShapeRef(router, polygon, 45);

    // shapeRef46
    polygon = Polygon(4);
    polygon.ps[0] = Point(154.041, 416.334);
    polygon.ps[1] = Point(184.041, 416.334);
    polygon.ps[2] = Point(184.041, 446.334);
    polygon.ps[3] = Point(154.041, 446.334);
    new ShapeRef(router, polygon, 46);

    // shapeRef47
    polygon = Polygon(4);
    polygon.ps[0] = Point(214.034, 136.334);
    polygon.ps[1] = Point(244.034, 136.334);
    polygon.ps[2] = Point(244.034, 166.334);
    polygon.ps[3] = Point(214.034, 166.334);
    new ShapeRef(router, polygon, 47);

    // shapeRef48
    polygon = Polygon(4);
    polygon.ps[0] = Point(214.034, 296.334);
    polygon.ps[1] = Point(244.034, 296.334);
    polygon.ps[2] = Point(244.034, 326.334);
    polygon.ps[3] = Point(214.034, 326.334);
    new ShapeRef(router, polygon, 48);

    // shapeRef50
    polygon = Polygon(4);
    polygon.ps[0] = Point(74.0462, 296.334);
    polygon.ps[1] = Point(104.046, 296.334);
    polygon.ps[2] = Point(104.046, 326.334);
    polygon.ps[3] = Point(74.0462, 326.334);
    new ShapeRef(router, polygon, 50);

    // shapeRef51
    polygon = Polygon(4);
    polygon.ps[0] = Point(354.024, 36.3341);
    polygon.ps[1] = Point(384.024, 36.3341);
    polygon.ps[2] = Point(384.024, 66.3341);
    polygon.ps[3] = Point(354.024, 66.3341);
    new ShapeRef(router, polygon, 51);

    // shapeRef53
    polygon = Polygon(4);
    polygon.ps[0] = Point(354.024, 180.582);
    polygon.ps[1] = Point(384.024, 180.582);
    polygon.ps[2] = Point(384.024, 210.582);
    polygon.ps[3] = Point(354.024, 210.582);
    new ShapeRef(router, polygon, 53);

    // shapeRef54
    polygon = Polygon(4);
    polygon.ps[0] = Point(534.011, 596.334);
    polygon.ps[1] = Point(564.011, 596.334);
    polygon.ps[2] = Point(564.011, 626.334);
    polygon.ps[3] = Point(534.011, 626.334);
    new ShapeRef(router, polygon, 54);

    // shapeRef55
    polygon = Polygon(4);
    polygon.ps[0] = Point(8.56882, 156.334);
    polygon.ps[1] = Point(38.5688, 156.334);
    polygon.ps[2] = Point(38.5688, 186.334);
    polygon.ps[3] = Point(8.56882, 186.334);
    new ShapeRef(router, polygon, 55);

    // shapeRef57
    polygon = Polygon(4);
    polygon.ps[0] = Point(281.314, 476.334);
    polygon.ps[1] = Point(311.314, 476.334);
    polygon.ps[2] = Point(311.314, 506.334);
    polygon.ps[3] = Point(281.314, 506.334);
    new ShapeRef(router, polygon, 57);

    // shapeRef58
    polygon = Polygon(4);
    polygon.ps[0] = Point(254.039, 312.99);
    polygon.ps[1] = Point(284.039, 312.99);
    polygon.ps[2] = Point(284.039, 342.99);
    polygon.ps[3] = Point(254.039, 342.99);
    new ShapeRef(router, polygon, 58);

    // connRef59
    connRef = new ConnRef(router, 59);
    srcPt = ConnEnd(Point(229.034, 371.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef60
    connRef = new ConnRef(router, 60);
    srcPt = ConnEnd(Point(169.041, 491.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(169.041, 431.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef61
    connRef = new ConnRef(router, 61);
    srcPt = ConnEnd(Point(369.024, 315.457), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(369.024, 431.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef62
    connRef = new ConnRef(router, 62);
    srcPt = ConnEnd(Point(23.5688, 291.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(23.5688, 171.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef63
    connRef = new ConnRef(router, 63);
    srcPt = ConnEnd(Point(429.023, 551.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(429.023, 491.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef64
    connRef = new ConnRef(router, 64);
    srcPt = ConnEnd(Point(549.011, 611.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(369.034, 611.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef65
    connRef = new ConnRef(router, 65);
    srcPt = ConnEnd(Point(369.024, 431.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(229.034, 454.168), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef66
    connRef = new ConnRef(router, 66);
    srcPt = ConnEnd(Point(429.023, 551.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(579.023, 431.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef67
    connRef = new ConnRef(router, 67);
    srcPt = ConnEnd(Point(489.011, 231.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(269.039, 231.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef68
    connRef = new ConnRef(router, 68);
    srcPt = ConnEnd(Point(369.034, 551.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(429.023, 551.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef69
    connRef = new ConnRef(router, 69);
    srcPt = ConnEnd(Point(169.041, 371.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(149.045, 401.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef70
    connRef = new ConnRef(router, 70);
    srcPt = ConnEnd(Point(169.041, 491.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(229.034, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef71
    connRef = new ConnRef(router, 71);
    srcPt = ConnEnd(Point(149.045, 401.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(149.045, 321.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef72
    connRef = new ConnRef(router, 72);
    srcPt = ConnEnd(Point(489.011, 231.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(549.011, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef73
    connRef = new ConnRef(router, 73);
    srcPt = ConnEnd(Point(229.034, 91.3341), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 91.3341), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef74
    connRef = new ConnRef(router, 74);
    srcPt = ConnEnd(Point(369.024, 315.457), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(369.024, 195.582), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef75
    connRef = new ConnRef(router, 75);
    srcPt = ConnEnd(Point(489.011, 171.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 231.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef76
    connRef = new ConnRef(router, 76);
    srcPt = ConnEnd(Point(146.696, 291.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef77
    connRef = new ConnRef(router, 77);
    srcPt = ConnEnd(Point(23.5688, 291.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(146.696, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef78
    connRef = new ConnRef(router, 78);
    srcPt = ConnEnd(Point(609.052, 611.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 461.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef79
    connRef = new ConnRef(router, 79);
    srcPt = ConnEnd(Point(489.011, 431.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(429.023, 431.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef80
    connRef = new ConnRef(router, 80);
    srcPt = ConnEnd(Point(489.011, 315.457), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 231.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef81
    connRef = new ConnRef(router, 81);
    srcPt = ConnEnd(Point(229.034, 371.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(169.041, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef82
    connRef = new ConnRef(router, 82);
    srcPt = ConnEnd(Point(489.011, 431.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 315.457), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef83
    connRef = new ConnRef(router, 83);
    srcPt = ConnEnd(Point(89.0462, 171.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 171.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef84
    connRef = new ConnRef(router, 84);
    srcPt = ConnEnd(Point(229.034, 151.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(229.034, 91.3341), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef85
    connRef = new ConnRef(router, 85);
    srcPt = ConnEnd(Point(289.03, 91.3341), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef86
    connRef = new ConnRef(router, 86);
    srcPt = ConnEnd(Point(89.0462, 311.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(89.0462, 171.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef87
    connRef = new ConnRef(router, 87);
    srcPt = ConnEnd(Point(149.045, 321.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 171.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef88
    connRef = new ConnRef(router, 88);
    srcPt = ConnEnd(Point(489.011, 315.457), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(609.052, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef89
    connRef = new ConnRef(router, 89);
    srcPt = ConnEnd(Point(369.024, 195.582), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(369.024, 51.3341), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef90
    connRef = new ConnRef(router, 90);
    srcPt = ConnEnd(Point(289.03, 461.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef91
    connRef = new ConnRef(router, 91);
    srcPt = ConnEnd(Point(296.314, 491.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(429.023, 491.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef92
    connRef = new ConnRef(router, 92);
    srcPt = ConnEnd(Point(369.034, 611.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(369.034, 551.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef93
    connRef = new ConnRef(router, 93);
    srcPt = ConnEnd(Point(609.052, 291.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(549.011, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef94
    connRef = new ConnRef(router, 94);
    srcPt = ConnEnd(Point(579.023, 431.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(579.023, 358.278), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef95
    connRef = new ConnRef(router, 95);
    srcPt = ConnEnd(Point(549.011, 231.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(549.011, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef96
    connRef = new ConnRef(router, 96);
    srcPt = ConnEnd(Point(579.052, 195.582), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(549.011, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef97
    connRef = new ConnRef(router, 97);
    srcPt = ConnEnd(Point(489.011, 231.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(549.011, 231.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef98
    connRef = new ConnRef(router, 98);
    srcPt = ConnEnd(Point(549.011, 291.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef99
    connRef = new ConnRef(router, 99);
    srcPt = ConnEnd(Point(149.045, 401.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(269.052, 401.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef100
    connRef = new ConnRef(router, 100);
    srcPt = ConnEnd(Point(579.023, 431.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 431.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef101
    connRef = new ConnRef(router, 101);
    srcPt = ConnEnd(Point(229.034, 454.168), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(229.034, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef102
    connRef = new ConnRef(router, 102);
    srcPt = ConnEnd(Point(489.011, 111.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 171.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef103
    connRef = new ConnRef(router, 103);
    srcPt = ConnEnd(Point(369.024, 315.457), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 315.457), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef104
    connRef = new ConnRef(router, 104);
    srcPt = ConnEnd(Point(349.035, 371.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 111.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef105
    connRef = new ConnRef(router, 105);
    srcPt = ConnEnd(Point(429.023, 431.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef106
    connRef = new ConnRef(router, 106);
    srcPt = ConnEnd(Point(269.039, 231.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(349.035, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef107
    connRef = new ConnRef(router, 107);
    srcPt = ConnEnd(Point(289.03, 371.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(349.035, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef108
    connRef = new ConnRef(router, 108);
    srcPt = ConnEnd(Point(579.023, 358.278), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(549.011, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef109
    connRef = new ConnRef(router, 109);
    srcPt = ConnEnd(Point(429.023, 431.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(369.024, 431.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef110
    connRef = new ConnRef(router, 110);
    srcPt = ConnEnd(Point(369.024, 51.3341), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(489.011, 51.3341), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef111
    connRef = new ConnRef(router, 111);
    srcPt = ConnEnd(Point(296.314, 491.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(169.041, 491.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef112
    connRef = new ConnRef(router, 112);
    srcPt = ConnEnd(Point(349.035, 151.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(349.035, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef113
    connRef = new ConnRef(router, 113);
    srcPt = ConnEnd(Point(229.034, 311.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(229.034, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef114
    connRef = new ConnRef(router, 114);
    srcPt = ConnEnd(Point(269.039, 327.99), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(169.041, 431.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef115
    connRef = new ConnRef(router, 115);
    srcPt = ConnEnd(Point(369.034, 551.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(229.034, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef116
    connRef = new ConnRef(router, 116);
    srcPt = ConnEnd(Point(429.023, 491.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(429.023, 431.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef117
    connRef = new ConnRef(router, 117);
    srcPt = ConnEnd(Point(89.0462, 401.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(149.045, 401.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef118
    connRef = new ConnRef(router, 118);
    srcPt = ConnEnd(Point(349.035, 151.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(229.034, 151.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef119
    connRef = new ConnRef(router, 119);
    srcPt = ConnEnd(Point(549.011, 401.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(549.011, 611.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef120
    connRef = new ConnRef(router, 120);
    srcPt = ConnEnd(Point(369.024, 195.582), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(579.052, 195.582), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef121
    connRef = new ConnRef(router, 121);
    srcPt = ConnEnd(Point(269.039, 327.99), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(269.039, 231.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef122
    connRef = new ConnRef(router, 122);
    srcPt = ConnEnd(Point(169.041, 431.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(169.041, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef123
    connRef = new ConnRef(router, 123);
    srcPt = ConnEnd(Point(369.024, 431.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(169.041, 431.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef124
    connRef = new ConnRef(router, 124);
    srcPt = ConnEnd(Point(229.034, 151.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(229.034, 311.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef125
    connRef = new ConnRef(router, 125);
    srcPt = ConnEnd(Point(549.011, 401.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(269.052, 401.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef126
    connRef = new ConnRef(router, 126);
    srcPt = ConnEnd(Point(549.011, 401.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(549.011, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef127
    connRef = new ConnRef(router, 127);
    srcPt = ConnEnd(Point(269.052, 401.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 291.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

#define EDGE128
#ifdef EDGE128
    // connRef128
    connRef = new ConnRef(router, 128);
    srcPt = ConnEnd(Point(289.03, 291.334), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(289.03, 371.334), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
#endif

    router->processTransaction();
    router->outputDiagram("output/hola01");
    delete router;
    return 0;
};

