// KEGG -> SBGN example #072
//
#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(
            PolyLineRouting | OrthogonalRouting);
    router->setRoutingPenalty((PenaltyType)0, 50);
    router->setRoutingPenalty((PenaltyType)1, 0);
    router->setRoutingPenalty((PenaltyType)2, 0);
    router->setRoutingPenalty((PenaltyType)3, 4000);
    router->setRoutingPenalty((PenaltyType)4, 0);
    router->setRoutingPenalty((PenaltyType)5, 100);
    router->setRoutingOption((RoutingOption)0, true);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingOption((RoutingOption)2, false);

    /*
    Polygon poly1(4);
    poly1.ps[0] = Point(541.101, 45);
    poly1.ps[1] = Point(541.101, 89);
    poly1.ps[2] = Point(274.101, 89);
    poly1.ps[3] = Point(274.101, 45);
    new ShapeRef(router, poly1, 1);

    Polygon poly2(4);
    poly2.ps[0] = Point(445.857, 111);
    poly2.ps[1] = Point(445.857, 276);
    poly2.ps[2] = Point(309.857, 276);
    poly2.ps[3] = Point(309.857, 111);
    new ShapeRef(router, poly2, 2);
    */

    Polygon poly3(4);
    poly3.ps[0] = Point(328, 368);
    poly3.ps[1] = Point(328, 400);
    poly3.ps[2] = Point(308, 400);
    poly3.ps[3] = Point(308, 368);
    new ShapeRef(router, poly3, 3);

    Polygon poly4(4);
    poly4.ps[0] = Point(328, 492);
    poly4.ps[1] = Point(328, 524);
    poly4.ps[2] = Point(308, 524);
    poly4.ps[3] = Point(308, 492);
    new ShapeRef(router, poly4, 4);

    /*
    Polygon poly5(4);
    poly5.ps[0] = Point(328, 629);
    poly5.ps[1] = Point(328, 661);
    poly5.ps[2] = Point(308, 661);
    poly5.ps[3] = Point(308, 629);
    new ShapeRef(router, poly5, 5);

    Polygon poly6(4);
    poly6.ps[0] = Point(511.857, 308);
    poly6.ps[1] = Point(511.857, 328);
    poly6.ps[2] = Point(479.857, 328);
    poly6.ps[3] = Point(479.857, 308);
    new ShapeRef(router, poly6, 6);

    Polygon poly7(4);
    poly7.ps[0] = Point(518.357, 629);
    poly7.ps[1] = Point(518.357, 661);
    poly7.ps[2] = Point(498.357, 661);
    poly7.ps[3] = Point(498.357, 629);
    new ShapeRef(router, poly7, 7);

    Polygon poly8(4);
    poly8.ps[0] = Point(827.857, 210);
    poly8.ps[1] = Point(827.857, 270);
    poly8.ps[2] = Point(545.857, 270);
    poly8.ps[3] = Point(545.857, 210);
    new ShapeRef(router, poly8, 8);

    Polygon poly9(4);
    poly9.ps[0] = Point(805.357, 409);
    poly9.ps[1] = Point(805.357, 469);
    poly9.ps[2] = Point(572.357, 469);
    poly9.ps[3] = Point(572.357, 409);
    new ShapeRef(router, poly9, 9);

    Polygon poly10(4);
    poly10.ps[0] = Point(157.5, 426);
    poly10.ps[1] = Point(157.5, 458);
    poly10.ps[2] = Point(137.5, 458);
    poly10.ps[3] = Point(137.5, 426);
    new ShapeRef(router, poly10, 10);

    Polygon poly11(4);
    poly11.ps[0] = Point(276, 597.655);
    poly11.ps[1] = Point(276, 657.655);
    poly11.ps[2] = Point(45, 657.655);
    poly11.ps[3] = Point(45, 597.655);
    new ShapeRef(router, poly11, 11);

    Polygon poly12(4);
    poly12.ps[0] = Point(349, 287);
    poly12.ps[1] = Point(349, 349);
    poly12.ps[2] = Point(287, 349);
    poly12.ps[3] = Point(287, 287);
    new ShapeRef(router, poly12, 12);
    */

    Polygon poly13(4);
    poly13.ps[0] = Point(349, 415);
    poly13.ps[1] = Point(349, 477);
    poly13.ps[2] = Point(287, 477);
    poly13.ps[3] = Point(287, 415);
    new ShapeRef(router, poly13, 13);

    /*
    Polygon poly14(4);
    poly14.ps[0] = Point(349, 676);
    poly14.ps[1] = Point(349, 738);
    poly14.ps[2] = Point(287, 738);
    poly14.ps[3] = Point(287, 676);
    new ShapeRef(router, poly14, 14);

    Polygon poly15(4);
    poly15.ps[0] = Point(349, 552);
    poly15.ps[1] = Point(349, 614);
    poly15.ps[2] = Point(287, 614);
    poly15.ps[3] = Point(287, 552);
    new ShapeRef(router, poly15, 15);

    Polygon poly16(4);
    poly16.ps[0] = Point(539.357, 676);
    poly16.ps[1] = Point(539.357, 738);
    poly16.ps[2] = Point(477.357, 738);
    poly16.ps[3] = Point(477.357, 676);
    new ShapeRef(router, poly16, 16);
    */

    Polygon poly17(4);
    poly17.ps[0] = Point(640.857, 336);
    poly17.ps[1] = Point(640.857, 398);
    poly17.ps[2] = Point(578.857, 398);
    poly17.ps[3] = Point(578.857, 336);
    new ShapeRef(router, poly17, 17);

    Polygon poly18(4);
    poly18.ps[0] = Point(404, 364);
    poly18.ps[1] = Point(404, 404);
    poly18.ps[2] = Point(328, 404);
    poly18.ps[3] = Point(328, 364);
    new ShapeRef(router, poly18, 18);

    Polygon poly19(4);
    poly19.ps[0] = Point(404, 488);
    poly19.ps[1] = Point(404, 528);
    poly19.ps[2] = Point(328, 528);
    poly19.ps[3] = Point(328, 488);
    new ShapeRef(router, poly19, 19);

    /*
    Polygon poly20(4);
    poly20.ps[0] = Point(404, 625);
    poly20.ps[1] = Point(404, 665);
    poly20.ps[2] = Point(328, 665);
    poly20.ps[3] = Point(328, 625);
    new ShapeRef(router, poly20, 20);

    Polygon poly21(4);
    poly21.ps[0] = Point(523.857, 248);
    poly21.ps[1] = Point(523.857, 308);
    poly21.ps[2] = Point(467.857, 308);
    poly21.ps[3] = Point(467.857, 248);
    new ShapeRef(router, poly21, 21);

    Polygon poly22(4);
    poly22.ps[0] = Point(594.357, 625);
    poly22.ps[1] = Point(594.357, 665);
    poly22.ps[2] = Point(518.357, 665);
    poly22.ps[3] = Point(518.357, 625);
    new ShapeRef(router, poly22, 22);

    Polygon poly23(4);
    poly23.ps[0] = Point(137.5, 422);
    poly23.ps[1] = Point(137.5, 462);
    poly23.ps[2] = Point(61.5, 462);
    poly23.ps[3] = Point(61.5, 422);
    new ShapeRef(router, poly23, 23);

    Polygon poly24(4);
    poly24.ps[0] = Point(398.357, 216.5);
    poly24.ps[1] = Point(398.357, 276.5);
    poly24.ps[2] = Point(358.357, 276.5);
    poly24.ps[3] = Point(358.357, 216.5);
    new ShapeRef(router, poly24, 24);

    Polygon poly25(4);
    poly25.ps[0] = Point(606.357, 220.5);
    poly25.ps[1] = Point(606.357, 260.5);
    poly25.ps[2] = Point(546.357, 260.5);
    poly25.ps[3] = Point(546.357, 220.5);
    new ShapeRef(router, poly25, 25);
    */

    Polygon poly26(4);
    poly26.ps[0] = Point(632.857, 419.5);
    poly26.ps[1] = Point(632.857, 459.5);
    poly26.ps[2] = Point(572.857, 459.5);
    poly26.ps[3] = Point(572.857, 419.5);
    new ShapeRef(router, poly26, 26);

    /*
    Polygon poly27(4);
    poly27.ps[0] = Point(276.5, 608.155);
    poly27.ps[1] = Point(276.5, 648.155);
    poly27.ps[2] = Point(216.5, 648.155);
    poly27.ps[3] = Point(216.5, 608.155);
    new ShapeRef(router, poly27, 27);

    ConnRef *connRef28 = new ConnRef(router, 28);
    ConnEnd srcPt28(Point(318, 399), 2);
    connRef28->setSourceEndpoint(srcPt28);
    ConnEnd dstPt28(Point(318, 446), 15);
    connRef28->setDestEndpoint(dstPt28);
    connRef28->setRoutingType((ConnType)2);

    ConnRef *connRef29 = new ConnRef(router, 29);
    ConnEnd srcPt29(Point(318, 523), 2);
    connRef29->setSourceEndpoint(srcPt29);
    ConnEnd dstPt29(Point(318, 583), 15);
    connRef29->setDestEndpoint(dstPt29);
    connRef29->setRoutingType((ConnType)2);
    */

    ConnRef *connRef30 = new ConnRef(router, 30);
    ConnEnd srcPt30(Point(318, 523), 2);
    connRef30->setSourceEndpoint(srcPt30);
    ConnEnd dstPt30(Point(609.857, 367), 15);
    connRef30->setDestEndpoint(dstPt30);
    connRef30->setRoutingType((ConnType)2);

    /*
    ConnRef *connRef31 = new ConnRef(router, 31);
    ConnEnd srcPt31(Point(318, 660), 2);
    connRef31->setSourceEndpoint(srcPt31);
    ConnEnd dstPt31(Point(318, 707), 15);
    connRef31->setDestEndpoint(dstPt31);
    connRef31->setRoutingType((ConnType)2);

    ConnRef *connRef32 = new ConnRef(router, 32);
    ConnEnd srcPt32(Point(480.857, 318), 4);
    connRef32->setSourceEndpoint(srcPt32);
    ConnEnd dstPt32(Point(318, 318), 15);
    connRef32->setDestEndpoint(dstPt32);
    connRef32->setRoutingType((ConnType)2);

    ConnRef *connRef33 = new ConnRef(router, 33);
    ConnEnd srcPt33(Point(510.857, 318), 8);
    connRef33->setSourceEndpoint(srcPt33);
    ConnEnd dstPt33(Point(609.857, 367), 15);
    connRef33->setDestEndpoint(dstPt33);
    connRef33->setRoutingType((ConnType)2);

    ConnRef *connRef34 = new ConnRef(router, 34);
    ConnEnd srcPt34(Point(508.357, 630), 1);
    connRef34->setSourceEndpoint(srcPt34);
    ConnEnd dstPt34(Point(318, 583), 15);
    connRef34->setDestEndpoint(dstPt34);
    connRef34->setRoutingType((ConnType)2);

    ConnRef *connRef35 = new ConnRef(router, 35);
    ConnEnd srcPt35(Point(147.5, 427), 1);
    connRef35->setSourceEndpoint(srcPt35);
    ConnEnd dstPt35(Point(318, 318), 15);
    connRef35->setDestEndpoint(dstPt35);
    connRef35->setRoutingType((ConnType)2);

    ConnRef *connRef36 = new ConnRef(router, 36);
    ConnEnd srcPt36(Point(147.5, 457), 2);
    connRef36->setSourceEndpoint(srcPt36);
    ConnEnd dstPt36(Point(318, 583), 15);
    connRef36->setDestEndpoint(dstPt36);
    connRef36->setRoutingType((ConnType)2);

    ConnRef *connRef37 = new ConnRef(router, 37);
    ConnEnd srcPt37(Point(318, 446), 15);
    connRef37->setSourceEndpoint(srcPt37);
    ConnEnd dstPt37(Point(318, 493), 1);
    connRef37->setDestEndpoint(dstPt37);
    connRef37->setRoutingType((ConnType)2);

    ConnRef *connRef38 = new ConnRef(router, 38);
    ConnEnd srcPt38(Point(508.357, 660), 2);
    connRef38->setSourceEndpoint(srcPt38);
    ConnEnd dstPt38(Point(508.357, 707), 15);
    connRef38->setDestEndpoint(dstPt38);
    connRef38->setRoutingType((ConnType)2);

    ConnRef *connRef39 = new ConnRef(router, 39);
    ConnEnd srcPt39(Point(376, 384), 4);
    connRef39->setSourceEndpoint(srcPt39);
    ConnEnd dstPt39(Point(318, 384), 8);
    connRef39->setDestEndpoint(dstPt39);
    connRef39->setRoutingType((ConnType)2);

    ConnRef *connRef40 = new ConnRef(router, 40);
    ConnEnd srcPt40(Point(376, 508), 4);
    connRef40->setSourceEndpoint(srcPt40);
    ConnEnd dstPt40(Point(318, 508), 8);
    connRef40->setDestEndpoint(dstPt40);
    connRef40->setRoutingType((ConnType)2);

    ConnRef *connRef41 = new ConnRef(router, 41);
    ConnEnd srcPt41(Point(376, 645), 4);
    connRef41->setSourceEndpoint(srcPt41);
    ConnEnd dstPt41(Point(318, 645), 8);
    connRef41->setDestEndpoint(dstPt41);
    connRef41->setRoutingType((ConnType)2);

    ConnRef *connRef42 = new ConnRef(router, 42);
    ConnEnd srcPt42(Point(495.857, 268), 2);
    connRef42->setSourceEndpoint(srcPt42);
    ConnEnd dstPt42(Point(495.857, 318), 1);
    connRef42->setDestEndpoint(dstPt42);
    connRef42->setRoutingType((ConnType)2);

    ConnRef *connRef43 = new ConnRef(router, 43);
    ConnEnd srcPt43(Point(566.357, 645), 4);
    connRef43->setSourceEndpoint(srcPt43);
    ConnEnd dstPt43(Point(508.357, 645), 8);
    connRef43->setDestEndpoint(dstPt43);
    connRef43->setRoutingType((ConnType)2);

    ConnRef *connRef44 = new ConnRef(router, 44);
    ConnEnd srcPt44(Point(89.5, 442), 8);
    connRef44->setSourceEndpoint(srcPt44);
    ConnEnd dstPt44(Point(147.5, 442), 4);
    connRef44->setDestEndpoint(dstPt44);
    connRef44->setRoutingType((ConnType)2);

    ConnRef *connRef45 = new ConnRef(router, 45);
    ConnEnd srcPt45(Point(318, 318), 15);
    connRef45->setSourceEndpoint(srcPt45);
    ConnEnd dstPt45(Point(318, 369), 1);
    connRef45->setDestEndpoint(dstPt45);
    connRef45->setRoutingType((ConnType)2);
    */

    ConnRef *connRef46 = new ConnRef(router, 46);
    ConnEnd srcPt46(Point(609.857, 367), 15);
    connRef46->setSourceEndpoint(srcPt46);
    ConnEnd dstPt46(Point(318, 369), 1);
    connRef46->setDestEndpoint(dstPt46);
    connRef46->setRoutingType((ConnType)2);

    /*
    ConnRef *connRef47 = new ConnRef(router, 47);
    ConnEnd srcPt47(Point(318, 583), 15);
    connRef47->setSourceEndpoint(srcPt47);
    ConnEnd dstPt47(Point(318, 630), 1);
    connRef47->setDestEndpoint(dstPt47);
    connRef47->setRoutingType((ConnType)2);

    ConnRef *connRef48 = new ConnRef(router, 48);
    ConnEnd srcPt48(Point(318, 318), 15);
    connRef48->setSourceEndpoint(srcPt48);
    ConnEnd dstPt48(Point(378.357, 271.5), 2);
    connRef48->setDestEndpoint(dstPt48);
    connRef48->setRoutingType((ConnType)2);

    ConnRef *connRef49 = new ConnRef(router, 49);
    ConnEnd srcPt49(Point(318, 318), 15);
    connRef49->setSourceEndpoint(srcPt49);
    ConnEnd dstPt49(Point(551.357, 240.5), 4);
    connRef49->setDestEndpoint(dstPt49);
    connRef49->setRoutingType((ConnType)2);
    */

    ConnRef *connRef50 = new ConnRef(router, 50);
    ConnEnd srcPt50(Point(609.857, 367), 15);
    connRef50->setSourceEndpoint(srcPt50);
    ConnEnd dstPt50(Point(577.857, 439.5), 4);
    connRef50->setDestEndpoint(dstPt50);
    connRef50->setRoutingType((ConnType)2);

    /*
    ConnRef *connRef51 = new ConnRef(router, 51);
    ConnEnd srcPt51(Point(318, 583), 15);
    connRef51->setSourceEndpoint(srcPt51);
    ConnEnd dstPt51(Point(271.5, 628.155), 8);
    connRef51->setDestEndpoint(dstPt51);
    connRef51->setRoutingType((ConnType)2);
    */

    router->processTransaction();
    router->outputDiagram("output/finalSegmentNudging2");
    bool atEnds = true;
    bool overlap = router->existsOrthogonalFixedSegmentOverlap(atEnds);
    delete router;
    return (overlap) ? 1 : 0;
};
