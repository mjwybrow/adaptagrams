// #00010
//
// Regression test to catch a crossing that could be intriduced during
// nudging when the centring step could previously change the ordering
// of some segments.  It now unifies them, rather than centring.
//
#include "libavoid/libavoid.h"
#include "libcola/cola.h"
using namespace cola;
using namespace Avoid;
int main(void) {
    Router *router = new Router(
            PolyLineRouting | OrthogonalRouting);
    router->setRoutingParameter((RoutingParameter)0, 50);
    router->setRoutingParameter((RoutingParameter)1, 0);
    router->setRoutingParameter((RoutingParameter)2, 0);
    router->setRoutingParameter((RoutingParameter)3, 4000);
    router->setRoutingParameter((RoutingParameter)4, 0);
    router->setRoutingParameter((RoutingParameter)5, 100);
    router->setRoutingOption((RoutingOption)0, true);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingOption((RoutingOption)2, false);
    router->setRoutingOption((RoutingOption)3, false);

#if 0
    Polygon poly1(4);
    poly1.ps[0] = Point(1299.5, 855.5);
    poly1.ps[1] = Point(1299.5, 887.5);
    poly1.ps[2] = Point(1279.5, 887.5);
    poly1.ps[3] = Point(1279.5, 855.5);
    new ShapeRef(router, poly1, 1);

    Polygon poly2(4);
    poly2.ps[0] = Point(647.5, 2048.54);
    poly2.ps[1] = Point(647.5, 2068.54);
    poly2.ps[2] = Point(615.5, 2068.54);
    poly2.ps[3] = Point(615.5, 2048.54);
    new ShapeRef(router, poly2, 2);

    Polygon poly3(4);
    poly3.ps[0] = Point(282.5, 1914.17);
    poly3.ps[1] = Point(282.5, 1946.17);
    poly3.ps[2] = Point(262.5, 1946.17);
    poly3.ps[3] = Point(262.5, 1914.17);
    new ShapeRef(router, poly3, 3);

    Polygon poly4(4);
    poly4.ps[0] = Point(647.5, 2090.54);
    poly4.ps[1] = Point(647.5, 2110.54);
    poly4.ps[2] = Point(615.5, 2110.54);
    poly4.ps[3] = Point(615.5, 2090.54);
    new ShapeRef(router, poly4, 4);

    Polygon poly5(4);
    poly5.ps[0] = Point(282.5, 2048.54);
    poly5.ps[1] = Point(282.5, 2110.54);
    poly5.ps[2] = Point(220.5, 2110.54);
    poly5.ps[3] = Point(220.5, 2048.54);
    new ShapeRef(router, poly5, 5);

    Polygon poly6(4);
    poly6.ps[0] = Point(1731.5, 492);
    poly6.ps[1] = Point(1731.5, 869);
    poly6.ps[2] = Point(1509.5, 869);
    poly6.ps[3] = Point(1509.5, 492);
    new ShapeRef(router, poly6, 6);

    Polygon poly7(4);
    poly7.ps[0] = Point(1409.5, 45);
    poly7.ps[1] = Point(1409.5, 219);
    poly7.ps[2] = Point(1289.5, 219);
    poly7.ps[3] = Point(1289.5, 45);
    new ShapeRef(router, poly7, 7);

    Polygon poly8(4);
    poly8.ps[0] = Point(947.5, 2001.54);
    poly8.ps[1] = Point(947.5, 2033.54);
    poly8.ps[2] = Point(927.5, 2033.54);
    poly8.ps[3] = Point(927.5, 2001.54);
    new ShapeRef(router, poly8, 8);

    Polygon poly9(4);
    poly9.ps[0] = Point(1255.5, 1973.54);
    poly9.ps[1] = Point(1255.5, 1993.54);
    poly9.ps[2] = Point(1223.5, 1993.54);
    poly9.ps[3] = Point(1223.5, 1973.54);
    new ShapeRef(router, poly9, 9);

    Polygon poly10(4);
    poly10.ps[0] = Point(1255.5, 2048.54);
    poly10.ps[1] = Point(1255.5, 2068.54);
    poly10.ps[2] = Point(1223.5, 2068.54);
    poly10.ps[3] = Point(1223.5, 2048.54);
    new ShapeRef(router, poly10, 10);

    Polygon poly11(4);
    poly11.ps[0] = Point(1135.5, 1869.54);
    poly11.ps[1] = Point(1135.5, 1889.54);
    poly11.ps[2] = Point(1103.5, 1889.54);
    poly11.ps[3] = Point(1103.5, 1869.54);
    new ShapeRef(router, poly11, 11);

    Polygon poly12(4);
    poly12.ps[0] = Point(1129.5, 1782.17);
    poly12.ps[1] = Point(1129.5, 1814.17);
    poly12.ps[2] = Point(1109.5, 1814.17);
    poly12.ps[3] = Point(1109.5, 1782.17);
    new ShapeRef(router, poly12, 12);

    Polygon poly13(4);
    poly13.ps[0] = Point(829.5, 1859.17);
    poly13.ps[1] = Point(829.5, 1879.17);
    poly13.ps[2] = Point(797.5, 1879.17);
    poly13.ps[3] = Point(797.5, 1859.17);
    new ShapeRef(router, poly13, 13);

    Polygon poly14(4);
    poly14.ps[0] = Point(422.5, 1859.17);
    poly14.ps[1] = Point(422.5, 1879.17);
    poly14.ps[2] = Point(390.5, 1879.17);
    poly14.ps[3] = Point(390.5, 1859.17);
    new ShapeRef(router, poly14, 14);

    Polygon poly15(4);
    poly15.ps[0] = Point(1472.5, 1859.17);
    poly15.ps[1] = Point(1472.5, 1879.17);
    poly15.ps[2] = Point(1440.5, 1879.17);
    poly15.ps[3] = Point(1440.5, 1859.17);
    new ShapeRef(router, poly15, 15);

    Polygon poly16(4);
    poly16.ps[0] = Point(979.028, 1370.55);
    poly16.ps[1] = Point(979.028, 1414.55);
    poly16.ps[2] = Point(891.028, 1414.55);
    poly16.ps[3] = Point(891.028, 1370.55);
    new ShapeRef(router, poly16, 16);

    Polygon poly17(4);
    poly17.ps[0] = Point(1299.5, 1649.17);
    poly17.ps[1] = Point(1299.5, 1681.17);
    poly17.ps[2] = Point(1279.5, 1681.17);
    poly17.ps[3] = Point(1279.5, 1649.17);
    new ShapeRef(router, poly17, 17);

    Polygon poly18(4);
    poly18.ps[0] = Point(252, 1663.17);
    poly18.ps[1] = Point(252, 1827.17);
    poly18.ps[2] = Point(45, 1827.17);
    poly18.ps[3] = Point(45, 1663.17);
    new ShapeRef(router, poly18, 18);

    Polygon poly19(4);
    poly19.ps[0] = Point(1299.5, 1525.17);
    poly19.ps[1] = Point(1299.5, 1557.17);
    poly19.ps[2] = Point(1279.5, 1557.17);
    poly19.ps[3] = Point(1279.5, 1525.17);
    new ShapeRef(router, poly19, 19);

    Polygon poly20(4);
    poly20.ps[0] = Point(1299.5, 1401.17);
    poly20.ps[1] = Point(1299.5, 1433.17);
    poly20.ps[2] = Point(1279.5, 1433.17);
    poly20.ps[3] = Point(1279.5, 1401.17);
    new ShapeRef(router, poly20, 20);

    Polygon poly21(4);
    poly21.ps[0] = Point(1278.5, 1011.17);
    poly21.ps[1] = Point(1278.5, 1043.17);
    poly21.ps[2] = Point(1258.5, 1043.17);
    poly21.ps[3] = Point(1258.5, 1011.17);
    new ShapeRef(router, poly21, 21);

    Polygon poly22(4);
    poly22.ps[0] = Point(849, 1332.17);
    poly22.ps[1] = Point(849, 1392.17);
    poly22.ps[2] = Point(536, 1392.17);
    poly22.ps[3] = Point(536, 1332.17);
    new ShapeRef(router, poly22, 22);

    Polygon poly23(4);
    poly23.ps[0] = Point(1104.5, 956.167);
    poly23.ps[1] = Point(1104.5, 976.167);
    poly23.ps[2] = Point(1072.5, 976.167);
    poly23.ps[3] = Point(1072.5, 956.167);
    new ShapeRef(router, poly23, 23);

    Polygon poly24(4);
    poly24.ps[0] = Point(1278.5, 713.5);
    poly24.ps[1] = Point(1278.5, 745.5);
    poly24.ps[2] = Point(1258.5, 745.5);
    poly24.ps[3] = Point(1258.5, 713.5);
    new ShapeRef(router, poly24, 24);

    Polygon poly25(4);
    poly25.ps[0] = Point(1411.5, 713.5);
    poly25.ps[1] = Point(1411.5, 745.5);
    poly25.ps[2] = Point(1391.5, 745.5);
    poly25.ps[3] = Point(1391.5, 713.5);
    new ShapeRef(router, poly25, 25);

    Polygon poly26(4);
    poly26.ps[0] = Point(1441.5, 422.5);
    poly26.ps[1] = Point(1441.5, 442.5);
    poly26.ps[2] = Point(1409.5, 442.5);
    poly26.ps[3] = Point(1409.5, 422.5);
    new ShapeRef(router, poly26, 26);

    Polygon poly27(4);
    poly27.ps[0] = Point(1299.5, 535.25);
    poly27.ps[1] = Point(1299.5, 567.25);
    poly27.ps[2] = Point(1279.5, 567.25);
    poly27.ps[3] = Point(1279.5, 535.25);
    new ShapeRef(router, poly27, 27);

    Polygon poly28(4);
    poly28.ps[0] = Point(1299.5, 344);
    poly28.ps[1] = Point(1299.5, 376);
    poly28.ps[2] = Point(1279.5, 376);
    poly28.ps[3] = Point(1279.5, 344);
    new ShapeRef(router, poly28, 28);

    Polygon poly29(4);
    poly29.ps[0] = Point(1042.5, 628.75);
    poly29.ps[1] = Point(1042.5, 648.75);
    poly29.ps[2] = Point(1010.5, 648.75);
    poly29.ps[3] = Point(1010.5, 628.75);
    new ShapeRef(router, poly29, 29);

    Polygon poly30(4);
    poly30.ps[0] = Point(523.5, 532.75);
    poly30.ps[1] = Point(523.5, 552.75);
    poly30.ps[2] = Point(491.5, 552.75);
    poly30.ps[3] = Point(491.5, 532.75);
    new ShapeRef(router, poly30, 30);

    Polygon poly31(4);
    poly31.ps[0] = Point(523.5, 607.75);
    poly31.ps[1] = Point(523.5, 627.75);
    poly31.ps[2] = Point(491.5, 627.75);
    poly31.ps[3] = Point(491.5, 607.75);
    new ShapeRef(router, poly31, 31);

    Polygon poly32(4);
    poly32.ps[0] = Point(922.5, 541.25);
    poly32.ps[1] = Point(922.5, 561.25);
    poly32.ps[2] = Point(890.5, 561.25);
    poly32.ps[3] = Point(890.5, 541.25);
    new ShapeRef(router, poly32, 32);

    Polygon poly33(4);
    poly33.ps[0] = Point(761.5, 535.25);
    poly33.ps[1] = Point(761.5, 567.25);
    poly33.ps[2] = Point(741.5, 567.25);
    poly33.ps[3] = Point(741.5, 535.25);
    new ShapeRef(router, poly33, 33);

    Polygon poly34(4);
    poly34.ps[0] = Point(296.5, 535.25);
    poly34.ps[1] = Point(296.5, 567.25);
    poly34.ps[2] = Point(276.5, 567.25);
    poly34.ps[3] = Point(276.5, 535.25);
    new ShapeRef(router, poly34, 34);
#endif

    Polygon poly35(4);
    poly35.ps[0] = Point(616.5, 285);
    poly35.ps[1] = Point(616.5, 305);
    poly35.ps[2] = Point(584.5, 305);
    poly35.ps[3] = Point(584.5, 285);
    new ShapeRef(router, poly35, 35);

#if 0
    Polygon poly36(4);
    poly36.ps[0] = Point(616.5, 360);
    poly36.ps[1] = Point(616.5, 380);
    poly36.ps[2] = Point(584.5, 380);
    poly36.ps[3] = Point(584.5, 360);
    new ShapeRef(router, poly36, 36);

    Polygon poly37(4);
    poly37.ps[0] = Point(616.5, 552);
    poly37.ps[1] = Point(616.5, 572);
    poly37.ps[2] = Point(584.5, 572);
    poly37.ps[3] = Point(584.5, 552);
    new ShapeRef(router, poly37, 37);
#endif

    Polygon poly38(4);
    poly38.ps[0] = Point(891.5, 331);
    poly38.ps[1] = Point(891.5, 351);
    poly38.ps[2] = Point(859.5, 351);
    poly38.ps[3] = Point(859.5, 331);
    new ShapeRef(router, poly38, 38);

    Polygon poly39(4);
    poly39.ps[0] = Point(891.5, 229);
    poly39.ps[1] = Point(891.5, 249);
    poly39.ps[2] = Point(859.5, 249);
    poly39.ps[3] = Point(859.5, 229);
    new ShapeRef(router, poly39, 39);

#if 0
    Polygon poly40(4);
    poly40.ps[0] = Point(402, 96.5);
    poly40.ps[1] = Point(402, 131.5);
    poly40.ps[2] = Point(141, 131.5);
    poly40.ps[3] = Point(141, 96.5);
    new ShapeRef(router, poly40, 40);

    Polygon poly41(4);
    poly41.ps[0] = Point(554.5, 2002.21);
    poly41.ps[1] = Point(554.5, 2022.21);
    poly41.ps[2] = Point(522.5, 2022.21);
    poly41.ps[3] = Point(522.5, 2002.21);
    new ShapeRef(router, poly41, 41);

    Polygon poly42(4);
    poly42.ps[0] = Point(1299.5, 1247.17);
    poly42.ps[1] = Point(1299.5, 1279.17);
    poly42.ps[2] = Point(1279.5, 1279.17);
    poly42.ps[3] = Point(1279.5, 1247.17);
    new ShapeRef(router, poly42, 42);

    Polygon poly43(4);
    poly43.ps[0] = Point(616.5, 435);
    poly43.ps[1] = Point(616.5, 455);
    poly43.ps[2] = Point(584.5, 455);
    poly43.ps[3] = Point(584.5, 435);
    new ShapeRef(router, poly43, 43);

    Polygon poly44(4);
    poly44.ps[0] = Point(523.5, 649.75);
    poly44.ps[1] = Point(523.5, 669.75);
    poly44.ps[2] = Point(491.5, 669.75);
    poly44.ps[3] = Point(491.5, 649.75);
    new ShapeRef(router, poly44, 44);

    Polygon poly45(4);
    poly45.ps[0] = Point(302.5, 740.75);
    poly45.ps[1] = Point(302.5, 760.75);
    poly45.ps[2] = Point(270.5, 760.75);
    poly45.ps[3] = Point(270.5, 740.75);
    new ShapeRef(router, poly45, 45);

    Polygon poly46(4);
    poly46.ps[0] = Point(302.5, 842.75);
    poly46.ps[1] = Point(302.5, 862.75);
    poly46.ps[2] = Point(270.5, 862.75);
    poly46.ps[3] = Point(270.5, 842.75);
    new ShapeRef(router, poly46, 46);

    Polygon poly47(4);
    poly47.ps[0] = Point(678.5, 740.75);
    poly47.ps[1] = Point(678.5, 760.75);
    poly47.ps[2] = Point(646.5, 760.75);
    poly47.ps[3] = Point(646.5, 740.75);
    new ShapeRef(router, poly47, 47);

    Polygon poly48(4);
    poly48.ps[0] = Point(678.5, 842.75);
    poly48.ps[1] = Point(678.5, 862.75);
    poly48.ps[2] = Point(646.5, 862.75);
    poly48.ps[3] = Point(646.5, 842.75);
    new ShapeRef(router, poly48, 48);

    Polygon poly49(4);
    poly49.ps[0] = Point(1530.5, 401.5);
    poly49.ps[1] = Point(1530.5, 463.5);
    poly49.ps[2] = Point(1468.5, 463.5);
    poly49.ps[3] = Point(1468.5, 401.5);
    new ShapeRef(router, poly49, 49);

    Polygon poly50(4);
    poly50.ps[0] = Point(1320.5, 249);
    poly50.ps[1] = Point(1320.5, 311);
    poly50.ps[2] = Point(1258.5, 311);
    poly50.ps[3] = Point(1258.5, 249);
    new ShapeRef(router, poly50, 50);

    Polygon poly51(4);
    poly51.ps[0] = Point(1320.5, 1448.17);
    poly51.ps[1] = Point(1320.5, 1510.17);
    poly51.ps[2] = Point(1258.5, 1510.17);
    poly51.ps[3] = Point(1258.5, 1448.17);
    new ShapeRef(router, poly51, 51);
#endif
    Polygon poly52(4);
    poly52.ps[0] = Point(317.5, 435);
    poly52.ps[1] = Point(317.5, 497);
    poly52.ps[2] = Point(255.5, 497);
    poly52.ps[3] = Point(255.5, 435);
    new ShapeRef(router, poly52, 52);
#if 0
    Polygon poly53(4);
    poly53.ps[0] = Point(317.5, 607.75);
    poly53.ps[1] = Point(317.5, 669.75);
    poly53.ps[2] = Point(255.5, 669.75);
    poly53.ps[3] = Point(255.5, 607.75);
    new ShapeRef(router, poly53, 53);

    Polygon poly54(4);
    poly54.ps[0] = Point(875.5, 935.167);
    poly54.ps[1] = Point(875.5, 997.167);
    poly54.ps[2] = Point(813.5, 997.167);
    poly54.ps[3] = Point(813.5, 935.167);
    new ShapeRef(router, poly54, 54);

    Polygon poly55(4);
    poly55.ps[0] = Point(875.5, 607.75);
    poly55.ps[1] = Point(875.5, 669.75);
    poly55.ps[2] = Point(813.5, 669.75);
    poly55.ps[3] = Point(813.5, 607.75);
    new ShapeRef(router, poly55, 55);

    Polygon poly56(4);
    poly56.ps[0] = Point(1320.5, 435);
    poly56.ps[1] = Point(1320.5, 497);
    poly56.ps[2] = Point(1258.5, 497);
    poly56.ps[3] = Point(1258.5, 435);
    new ShapeRef(router, poly56, 56);

    Polygon poly57(4);
    poly57.ps[0] = Point(1320.5, 607.75);
    poly57.ps[1] = Point(1320.5, 669.75);
    poly57.ps[2] = Point(1258.5, 669.75);
    poly57.ps[3] = Point(1258.5, 607.75);
    new ShapeRef(router, poly57, 57);

    Polygon poly58(4);
    poly58.ps[0] = Point(1320.5, 1572.17);
    poly58.ps[1] = Point(1320.5, 1634.17);
    poly58.ps[2] = Point(1258.5, 1634.17);
    poly58.ps[3] = Point(1258.5, 1572.17);
    new ShapeRef(router, poly58, 58);

    Polygon poly59(4);
    poly59.ps[0] = Point(1320.5, 1324.17);
    poly59.ps[1] = Point(1320.5, 1386.17);
    poly59.ps[2] = Point(1258.5, 1386.17);
    poly59.ps[3] = Point(1258.5, 1324.17);
    new ShapeRef(router, poly59, 59);

    Polygon poly60(4);
    poly60.ps[0] = Point(1320.5, 1108.17);
    poly60.ps[1] = Point(1320.5, 1170.17);
    poly60.ps[2] = Point(1258.5, 1170.17);
    poly60.ps[3] = Point(1258.5, 1108.17);
    new ShapeRef(router, poly60, 60);

    Polygon poly61(4);
    poly61.ps[0] = Point(1561.5, 1838.17);
    poly61.ps[1] = Point(1561.5, 1900.17);
    poly61.ps[2] = Point(1499.5, 1900.17);
    poly61.ps[3] = Point(1499.5, 1838.17);
    new ShapeRef(router, poly61, 61);

    Polygon poly62(4);
    poly62.ps[0] = Point(813.5, 1981.21);
    poly62.ps[1] = Point(813.5, 2043.21);
    poly62.ps[2] = Point(751.5, 2043.21);
    poly62.ps[3] = Point(751.5, 1981.21);
    new ShapeRef(router, poly62, 62);

    Polygon poly63(4);
    poly63.ps[0] = Point(1456.5, 2048.54);
    poly63.ps[1] = Point(1456.5, 2110.54);
    poly63.ps[2] = Point(1394.5, 2110.54);
    poly63.ps[3] = Point(1394.5, 2048.54);
    new ShapeRef(router, poly63, 63);

    Polygon poly64(4);
    poly64.ps[0] = Point(1320.5, 1838.17);
    poly64.ps[1] = Point(1320.5, 1900.17);
    poly64.ps[2] = Point(1258.5, 1900.17);
    poly64.ps[3] = Point(1258.5, 1838.17);
    new ShapeRef(router, poly64, 64);

    Polygon poly65(4);
    poly65.ps[0] = Point(968.5, 1838.17);
    poly65.ps[1] = Point(968.5, 1900.17);
    poly65.ps[2] = Point(906.5, 1900.17);
    poly65.ps[3] = Point(906.5, 1838.17);
    new ShapeRef(router, poly65, 65);

    Polygon poly66(4);
    poly66.ps[0] = Point(282.5, 1838.17);
    poly66.ps[1] = Point(282.5, 1900.17);
    poly66.ps[2] = Point(220.5, 1900.17);
    poly66.ps[3] = Point(220.5, 1838.17);
    new ShapeRef(router, poly66, 66);

    Polygon poly67(4);
    poly67.ps[0] = Point(968.5, 2048.54);
    poly67.ps[1] = Point(968.5, 2110.54);
    poly67.ps[2] = Point(906.5, 2110.54);
    poly67.ps[3] = Point(906.5, 2048.54);
    new ShapeRef(router, poly67, 67);

    Polygon poly68(4);
    poly68.ps[0] = Point(600.5, 1838.17);
    poly68.ps[1] = Point(600.5, 1900.17);
    poly68.ps[2] = Point(538.5, 1900.17);
    poly68.ps[3] = Point(538.5, 1838.17);
    new ShapeRef(router, poly68, 68);

    Polygon poly69(4);
    poly69.ps[0] = Point(437.5, 1981.21);
    poly69.ps[1] = Point(437.5, 2043.21);
    poly69.ps[2] = Point(375.5, 2043.21);
    poly69.ps[3] = Point(375.5, 1981.21);
    new ShapeRef(router, poly69, 69);

    Polygon poly70(4);
    poly70.ps[0] = Point(1320.5, 759.5);
    poly70.ps[1] = Point(1320.5, 821.5);
    poly70.ps[2] = Point(1258.5, 821.5);
    poly70.ps[3] = Point(1258.5, 759.5);
    new ShapeRef(router, poly70, 70);

    Polygon poly71(4);
    poly71.ps[0] = Point(247.5, 719.75);
    poly71.ps[1] = Point(247.5, 781.75);
    poly71.ps[2] = Point(185.5, 781.75);
    poly71.ps[3] = Point(185.5, 719.75);
    new ShapeRef(router, poly71, 71);

    Polygon poly72(4);
    poly72.ps[0] = Point(468.5, 719.75);
    poly72.ps[1] = Point(468.5, 781.75);
    poly72.ps[2] = Point(406.5, 781.75);
    poly72.ps[3] = Point(406.5, 719.75);
    new ShapeRef(router, poly72, 72);

    Polygon poly73(4);
    poly73.ps[0] = Point(468.5, 821.75);
    poly73.ps[1] = Point(468.5, 883.75);
    poly73.ps[2] = Point(406.5, 883.75);
    poly73.ps[3] = Point(406.5, 821.75);
    new ShapeRef(router, poly73, 73);

    Polygon poly74(4);
    poly74.ps[0] = Point(247.5, 821.75);
    poly74.ps[1] = Point(247.5, 883.75);
    poly74.ps[2] = Point(185.5, 883.75);
    poly74.ps[3] = Point(185.5, 821.75);
    new ShapeRef(router, poly74, 74);

    Polygon poly75(4);
    poly75.ps[0] = Point(391.5, 1572.17);
    poly75.ps[1] = Point(391.5, 1592.17);
    poly75.ps[2] = Point(359.5, 1592.17);
    poly75.ps[3] = Point(359.5, 1572.17);
    new ShapeRef(router, poly75, 75);

    Polygon poly76(4);
    poly76.ps[0] = Point(282.5, 1572.17);
    poly76.ps[1] = Point(282.5, 1634.17);
    poly76.ps[2] = Point(220.5, 1634.17);
    poly76.ps[3] = Point(220.5, 1572.17);
    new ShapeRef(router, poly76, 76);

    Polygon poly77(4);
    poly77.ps[0] = Point(1255.5, 2090.54);
    poly77.ps[1] = Point(1255.5, 2110.54);
    poly77.ps[2] = Point(1223.5, 2110.54);
    poly77.ps[3] = Point(1223.5, 2090.54);
    new ShapeRef(router, poly77, 77);

    Polygon poly78(4);
    poly78.ps[0] = Point(391.5, 1614.17);
    poly78.ps[1] = Point(391.5, 1634.17);
    poly78.ps[2] = Point(359.5, 1634.17);
    poly78.ps[3] = Point(359.5, 1614.17);
    new ShapeRef(router, poly78, 78);

    Polygon poly79(4);
    poly79.ps[0] = Point(1604.5, 1943.04);
    poly79.ps[1] = Point(1604.5, 2108.04);
    poly79.ps[2] = Point(1467.5, 2108.04);
    poly79.ps[3] = Point(1467.5, 1943.04);
    new ShapeRef(router, poly79, 79);

    Polygon poly80(4);
    poly80.ps[0] = Point(1320.5, 1011.17);
    poly80.ps[1] = Point(1320.5, 1043.17);
    poly80.ps[2] = Point(1300.5, 1043.17);
    poly80.ps[3] = Point(1300.5, 1011.17);
    new ShapeRef(router, poly80, 80);

    Polygon poly81(4);
    poly81.ps[0] = Point(1406.5, 1185.17);
    poly81.ps[1] = Point(1406.5, 1217.17);
    poly81.ps[2] = Point(1386.5, 1217.17);
    poly81.ps[3] = Point(1386.5, 1185.17);
    new ShapeRef(router, poly81, 81);

    Polygon poly82(4);
    poly82.ps[0] = Point(1448.5, 1185.17);
    poly82.ps[1] = Point(1448.5, 1217.17);
    poly82.ps[2] = Point(1428.5, 1217.17);
    poly82.ps[3] = Point(1428.5, 1185.17);
    new ShapeRef(router, poly82, 82);

    Polygon poly83(4);
    poly83.ps[0] = Point(678.5, 1730.17);
    poly83.ps[1] = Point(678.5, 1750.17);
    poly83.ps[2] = Point(646.5, 1750.17);
    poly83.ps[3] = Point(646.5, 1730.17);
    new ShapeRef(router, poly83, 83);

    Polygon poly84(4);
    poly84.ps[0] = Point(1539.5, 1185.17);
    poly84.ps[1] = Point(1539.5, 1217.17);
    poly84.ps[2] = Point(1519.5, 1217.17);
    poly84.ps[3] = Point(1519.5, 1185.17);
    new ShapeRef(router, poly84, 84);

    Polygon poly85(4);
    poly85.ps[0] = Point(1088.5, 1232.17);
    poly85.ps[1] = Point(1088.5, 1294.17);
    poly85.ps[2] = Point(1026.5, 1294.17);
    poly85.ps[3] = Point(1026.5, 1232.17);
    new ShapeRef(router, poly85, 85);

    Polygon poly86(4);
    poly86.ps[0] = Point(1224.5, 1333.17);
    poly86.ps[1] = Point(1224.5, 1353.17);
    poly86.ps[2] = Point(1192.5, 1353.17);
    poly86.ps[3] = Point(1192.5, 1333.17);
    new ShapeRef(router, poly86, 86);

    Polygon poly87(4);
    poly87.ps[0] = Point(1224.5, 1129.17);
    poly87.ps[1] = Point(1224.5, 1149.17);
    poly87.ps[2] = Point(1192.5, 1149.17);
    poly87.ps[3] = Point(1192.5, 1129.17);
    new ShapeRef(router, poly87, 87);

    Polygon poly88(4);
    poly88.ps[0] = Point(1320.5, 935.167);
    poly88.ps[1] = Point(1320.5, 997.167);
    poly88.ps[2] = Point(1258.5, 997.167);
    poly88.ps[3] = Point(1258.5, 935.167);
    new ShapeRef(router, poly88, 88);

    Polygon poly89(4);
    poly89.ps[0] = Point(240.5, 1914.17);
    poly89.ps[1] = Point(240.5, 1946.17);
    poly89.ps[2] = Point(220.5, 1946.17);
    poly89.ps[3] = Point(220.5, 1914.17);
    new ShapeRef(router, poly89, 89);

    Polygon poly90(4);
    poly90.ps[0] = Point(1320.5, 713.5);
    poly90.ps[1] = Point(1320.5, 745.5);
    poly90.ps[2] = Point(1300.5, 745.5);
    poly90.ps[3] = Point(1300.5, 713.5);
    new ShapeRef(router, poly90, 90);

    Polygon poly91(4);
    poly91.ps[0] = Point(616.5, 477);
    poly91.ps[1] = Point(616.5, 497);
    poly91.ps[2] = Point(584.5, 497);
    poly91.ps[3] = Point(584.5, 477);
    new ShapeRef(router, poly91, 91);

    Polygon poly92(4);
    poly92.ps[0] = Point(523.5, 724.75);
    poly92.ps[1] = Point(523.5, 744.75);
    poly92.ps[2] = Point(491.5, 744.75);
    poly92.ps[3] = Point(491.5, 724.75);
    new ShapeRef(router, poly92, 92);

    Polygon poly93(4);
    poly93.ps[0] = Point(1255.5, 2165.54);
    poly93.ps[1] = Point(1255.5, 2185.54);
    poly93.ps[2] = Point(1223.5, 2185.54);
    poly93.ps[3] = Point(1223.5, 2165.54);
    new ShapeRef(router, poly93, 93);

    Polygon poly94(4);
    poly94.ps[0] = Point(968.5, 1734.17);
    poly94.ps[1] = Point(968.5, 1796.17);
    poly94.ps[2] = Point(906.5, 1796.17);
    poly94.ps[3] = Point(906.5, 1734.17);
    new ShapeRef(router, poly94, 94);

    Polygon poly95(4);
    poly95.ps[0] = Point(1375.5, 851.5);
    poly95.ps[1] = Point(1375.5, 891.5);
    poly95.ps[2] = Point(1299.5, 891.5);
    poly95.ps[3] = Point(1299.5, 851.5);
    new ShapeRef(router, poly95, 95);

    Polygon poly96(4);
    poly96.ps[0] = Point(659.5, 1988.54);
    poly96.ps[1] = Point(659.5, 2048.54);
    poly96.ps[2] = Point(603.5, 2048.54);
    poly96.ps[3] = Point(603.5, 1988.54);
    new ShapeRef(router, poly96, 96);

    Polygon poly97(4);
    poly97.ps[0] = Point(358.5, 1910.17);
    poly97.ps[1] = Point(358.5, 1950.17);
    poly97.ps[2] = Point(282.5, 1950.17);
    poly97.ps[3] = Point(282.5, 1910.17);
    new ShapeRef(router, poly97, 97);

    Polygon poly98(4);
    poly98.ps[0] = Point(659.5, 2110.54);
    poly98.ps[1] = Point(659.5, 2170.54);
    poly98.ps[2] = Point(603.5, 2170.54);
    poly98.ps[3] = Point(603.5, 2110.54);
    new ShapeRef(router, poly98, 98);

    Polygon poly99(4);
    poly99.ps[0] = Point(1023.5, 1997.54);
    poly99.ps[1] = Point(1023.5, 2037.54);
    poly99.ps[2] = Point(947.5, 2037.54);
    poly99.ps[3] = Point(947.5, 1997.54);
    new ShapeRef(router, poly99, 99);

    Polygon poly100(4);
    poly100.ps[0] = Point(1267.5, 1913.54);
    poly100.ps[1] = Point(1267.5, 1973.54);
    poly100.ps[2] = Point(1211.5, 1973.54);
    poly100.ps[3] = Point(1211.5, 1913.54);
    new ShapeRef(router, poly100, 100);

    Polygon poly101(4);
    poly101.ps[0] = Point(1267.5, 1988.54);
    poly101.ps[1] = Point(1267.5, 2048.54);
    poly101.ps[2] = Point(1211.5, 2048.54);
    poly101.ps[3] = Point(1211.5, 1988.54);
    new ShapeRef(router, poly101, 101);

    Polygon poly102(4);
    poly102.ps[0] = Point(1147.5, 1889.54);
    poly102.ps[1] = Point(1147.5, 1949.54);
    poly102.ps[2] = Point(1091.5, 1949.54);
    poly102.ps[3] = Point(1091.5, 1889.54);
    new ShapeRef(router, poly102, 102);

    Polygon poly103(4);
    poly103.ps[0] = Point(1205.5, 1778.17);
    poly103.ps[1] = Point(1205.5, 1818.17);
    poly103.ps[2] = Point(1129.5, 1818.17);
    poly103.ps[3] = Point(1129.5, 1778.17);
    new ShapeRef(router, poly103, 103);

    Polygon poly104(4);
    poly104.ps[0] = Point(841.5, 1799.17);
    poly104.ps[1] = Point(841.5, 1859.17);
    poly104.ps[2] = Point(785.5, 1859.17);
    poly104.ps[3] = Point(785.5, 1799.17);
    new ShapeRef(router, poly104, 104);

    Polygon poly105(4);
    poly105.ps[0] = Point(434.5, 1799.17);
    poly105.ps[1] = Point(434.5, 1859.17);
    poly105.ps[2] = Point(378.5, 1859.17);
    poly105.ps[3] = Point(378.5, 1799.17);
    new ShapeRef(router, poly105, 105);

    Polygon poly106(4);
    poly106.ps[0] = Point(1484.5, 1799.17);
    poly106.ps[1] = Point(1484.5, 1859.17);
    poly106.ps[2] = Point(1428.5, 1859.17);
    poly106.ps[3] = Point(1428.5, 1799.17);
    new ShapeRef(router, poly106, 106);

    Polygon poly107(4);
    poly107.ps[0] = Point(1375.5, 1645.17);
    poly107.ps[1] = Point(1375.5, 1685.17);
    poly107.ps[2] = Point(1299.5, 1685.17);
    poly107.ps[3] = Point(1299.5, 1645.17);
    new ShapeRef(router, poly107, 107);

    Polygon poly108(4);
    poly108.ps[0] = Point(1375.5, 1521.17);
    poly108.ps[1] = Point(1375.5, 1561.17);
    poly108.ps[2] = Point(1299.5, 1561.17);
    poly108.ps[3] = Point(1299.5, 1521.17);
    new ShapeRef(router, poly108, 108);

    Polygon poly109(4);
    poly109.ps[0] = Point(1375.5, 1397.17);
    poly109.ps[1] = Point(1375.5, 1437.17);
    poly109.ps[2] = Point(1299.5, 1437.17);
    poly109.ps[3] = Point(1299.5, 1397.17);
    new ShapeRef(router, poly109, 109);

    Polygon poly110(4);
    poly110.ps[0] = Point(1258.5, 1007.17);
    poly110.ps[1] = Point(1258.5, 1047.17);
    poly110.ps[2] = Point(1182.5, 1047.17);
    poly110.ps[3] = Point(1182.5, 1007.17);
    new ShapeRef(router, poly110, 110);

    Polygon poly111(4);
    poly111.ps[0] = Point(1116.5, 896.167);
    poly111.ps[1] = Point(1116.5, 956.167);
    poly111.ps[2] = Point(1060.5, 956.167);
    poly111.ps[3] = Point(1060.5, 896.167);
    new ShapeRef(router, poly111, 111);

    Polygon poly112(4);
    poly112.ps[0] = Point(1258.5, 709.5);
    poly112.ps[1] = Point(1258.5, 749.5);
    poly112.ps[2] = Point(1182.5, 749.5);
    poly112.ps[3] = Point(1182.5, 709.5);
    new ShapeRef(router, poly112, 112);

    Polygon poly113(4);
    poly113.ps[0] = Point(1487.5, 709.5);
    poly113.ps[1] = Point(1487.5, 749.5);
    poly113.ps[2] = Point(1411.5, 749.5);
    poly113.ps[3] = Point(1411.5, 709.5);
    new ShapeRef(router, poly113, 113);

    Polygon poly114(4);
    poly114.ps[0] = Point(1453.5, 362.5);
    poly114.ps[1] = Point(1453.5, 422.5);
    poly114.ps[2] = Point(1397.5, 422.5);
    poly114.ps[3] = Point(1397.5, 362.5);
    new ShapeRef(router, poly114, 114);

    Polygon poly115(4);
    poly115.ps[0] = Point(1375.5, 531.25);
    poly115.ps[1] = Point(1375.5, 571.25);
    poly115.ps[2] = Point(1299.5, 571.25);
    poly115.ps[3] = Point(1299.5, 531.25);
    new ShapeRef(router, poly115, 115);

    Polygon poly116(4);
    poly116.ps[0] = Point(1375.5, 340);
    poly116.ps[1] = Point(1375.5, 380);
    poly116.ps[2] = Point(1299.5, 380);
    poly116.ps[3] = Point(1299.5, 340);
    new ShapeRef(router, poly116, 116);

    Polygon poly117(4);
    poly117.ps[0] = Point(1054.5, 568.75);
    poly117.ps[1] = Point(1054.5, 628.75);
    poly117.ps[2] = Point(998.5, 628.75);
    poly117.ps[3] = Point(998.5, 568.75);
    new ShapeRef(router, poly117, 117);

    Polygon poly118(4);
    poly118.ps[0] = Point(535.5, 472.75);
    poly118.ps[1] = Point(535.5, 532.75);
    poly118.ps[2] = Point(479.5, 532.75);
    poly118.ps[3] = Point(479.5, 472.75);
    new ShapeRef(router, poly118, 118);

    Polygon poly119(4);
    poly119.ps[0] = Point(535.5, 547.75);
    poly119.ps[1] = Point(535.5, 607.75);
    poly119.ps[2] = Point(479.5, 607.75);
    poly119.ps[3] = Point(479.5, 547.75);
    new ShapeRef(router, poly119, 119);

    Polygon poly120(4);
    poly120.ps[0] = Point(934.5, 481.25);
    poly120.ps[1] = Point(934.5, 541.25);
    poly120.ps[2] = Point(878.5, 541.25);
    poly120.ps[3] = Point(878.5, 481.25);
    new ShapeRef(router, poly120, 120);

    Polygon poly121(4);
    poly121.ps[0] = Point(741.5, 531.25);
    poly121.ps[1] = Point(741.5, 571.25);
    poly121.ps[2] = Point(665.5, 571.25);
    poly121.ps[3] = Point(665.5, 531.25);
    new ShapeRef(router, poly121, 121);

    Polygon poly122(4);
    poly122.ps[0] = Point(372.5, 531.25);
    poly122.ps[1] = Point(372.5, 571.25);
    poly122.ps[2] = Point(296.5, 571.25);
    poly122.ps[3] = Point(296.5, 531.25);
    new ShapeRef(router, poly122, 122);
#endif

    Polygon poly123(4);
    poly123.ps[0] = Point(628.5, 225);
    poly123.ps[1] = Point(628.5, 285);
    poly123.ps[2] = Point(572.5, 285);
    poly123.ps[3] = Point(572.5, 225);
    new ShapeRef(router, poly123, 123);

#if 0
    Polygon poly124(4);
    poly124.ps[0] = Point(628.5, 300);
    poly124.ps[1] = Point(628.5, 360);
    poly124.ps[2] = Point(572.5, 360);
    poly124.ps[3] = Point(572.5, 300);
    new ShapeRef(router, poly124, 124);

    Polygon poly125(4);
    poly125.ps[0] = Point(628.5, 572);
    poly125.ps[1] = Point(628.5, 632);
    poly125.ps[2] = Point(572.5, 632);
    poly125.ps[3] = Point(572.5, 572);
    new ShapeRef(router, poly125, 125);

    Polygon poly126(4);
    poly126.ps[0] = Point(903.5, 271);
    poly126.ps[1] = Point(903.5, 331);
    poly126.ps[2] = Point(847.5, 331);
    poly126.ps[3] = Point(847.5, 271);
    new ShapeRef(router, poly126, 126);

    Polygon poly127(4);
    poly127.ps[0] = Point(903.5, 169);
    poly127.ps[1] = Point(903.5, 229);
    poly127.ps[2] = Point(847.5, 229);
    poly127.ps[3] = Point(847.5, 169);
    new ShapeRef(router, poly127, 127);

    Polygon poly128(4);
    poly128.ps[0] = Point(566.5, 1942.21);
    poly128.ps[1] = Point(566.5, 2002.21);
    poly128.ps[2] = Point(510.5, 2002.21);
    poly128.ps[3] = Point(510.5, 1942.21);
    new ShapeRef(router, poly128, 128);

    Polygon poly129(4);
    poly129.ps[0] = Point(1375.5, 1243.17);
    poly129.ps[1] = Point(1375.5, 1283.17);
    poly129.ps[2] = Point(1299.5, 1283.17);
    poly129.ps[3] = Point(1299.5, 1243.17);
    new ShapeRef(router, poly129, 129);

    Polygon poly130(4);
    poly130.ps[0] = Point(628.5, 375);
    poly130.ps[1] = Point(628.5, 435);
    poly130.ps[2] = Point(572.5, 435);
    poly130.ps[3] = Point(572.5, 375);
    new ShapeRef(router, poly130, 130);

    Polygon poly131(4);
    poly131.ps[0] = Point(535.5, 669.75);
    poly131.ps[1] = Point(535.5, 729.75);
    poly131.ps[2] = Point(479.5, 729.75);
    poly131.ps[3] = Point(479.5, 669.75);
    new ShapeRef(router, poly131, 131);

    Polygon poly132(4);
    poly132.ps[0] = Point(314.5, 680.75);
    poly132.ps[1] = Point(314.5, 740.75);
    poly132.ps[2] = Point(258.5, 740.75);
    poly132.ps[3] = Point(258.5, 680.75);
    new ShapeRef(router, poly132, 132);

    Polygon poly133(4);
    poly133.ps[0] = Point(314.5, 782.75);
    poly133.ps[1] = Point(314.5, 842.75);
    poly133.ps[2] = Point(258.5, 842.75);
    poly133.ps[3] = Point(258.5, 782.75);
    new ShapeRef(router, poly133, 133);

    Polygon poly134(4);
    poly134.ps[0] = Point(690.5, 680.75);
    poly134.ps[1] = Point(690.5, 740.75);
    poly134.ps[2] = Point(634.5, 740.75);
    poly134.ps[3] = Point(634.5, 680.75);
    new ShapeRef(router, poly134, 134);

    Polygon poly135(4);
    poly135.ps[0] = Point(690.5, 782.75);
    poly135.ps[1] = Point(690.5, 842.75);
    poly135.ps[2] = Point(634.5, 842.75);
    poly135.ps[3] = Point(634.5, 782.75);
    new ShapeRef(router, poly135, 135);

    Polygon poly136(4);
    poly136.ps[0] = Point(403.5, 1512.17);
    poly136.ps[1] = Point(403.5, 1572.17);
    poly136.ps[2] = Point(347.5, 1572.17);
    poly136.ps[3] = Point(347.5, 1512.17);
    new ShapeRef(router, poly136, 136);

    Polygon poly137(4);
    poly137.ps[0] = Point(1267.5, 2110.54);
    poly137.ps[1] = Point(1267.5, 2170.54);
    poly137.ps[2] = Point(1211.5, 2170.54);
    poly137.ps[3] = Point(1211.5, 2110.54);
    new ShapeRef(router, poly137, 137);

    Polygon poly138(4);
    poly138.ps[0] = Point(403.5, 1634.17);
    poly138.ps[1] = Point(403.5, 1694.17);
    poly138.ps[2] = Point(347.5, 1694.17);
    poly138.ps[3] = Point(347.5, 1634.17);
    new ShapeRef(router, poly138, 138);

    Polygon poly139(4);
    poly139.ps[0] = Point(1396.5, 1007.17);
    poly139.ps[1] = Point(1396.5, 1047.17);
    poly139.ps[2] = Point(1320.5, 1047.17);
    poly139.ps[3] = Point(1320.5, 1007.17);
    new ShapeRef(router, poly139, 139);

    Polygon poly140(4);
    poly140.ps[0] = Point(1386.5, 1181.17);
    poly140.ps[1] = Point(1386.5, 1221.17);
    poly140.ps[2] = Point(1310.5, 1221.17);
    poly140.ps[3] = Point(1310.5, 1181.17);
    new ShapeRef(router, poly140, 140);

    Polygon poly141(4);
    poly141.ps[0] = Point(1524.5, 1181.17);
    poly141.ps[1] = Point(1524.5, 1221.17);
    poly141.ps[2] = Point(1448.5, 1221.17);
    poly141.ps[3] = Point(1448.5, 1181.17);
    new ShapeRef(router, poly141, 141);

    Polygon poly142(4);
    poly142.ps[0] = Point(690.5, 1670.17);
    poly142.ps[1] = Point(690.5, 1730.17);
    poly142.ps[2] = Point(634.5, 1730.17);
    poly142.ps[3] = Point(634.5, 1670.17);
    new ShapeRef(router, poly142, 142);

    Polygon poly143(4);
    poly143.ps[0] = Point(1615.5, 1181.17);
    poly143.ps[1] = Point(1615.5, 1221.17);
    poly143.ps[2] = Point(1539.5, 1221.17);
    poly143.ps[3] = Point(1539.5, 1181.17);
    new ShapeRef(router, poly143, 143);

    Polygon poly144(4);
    poly144.ps[0] = Point(1236.5, 1273.17);
    poly144.ps[1] = Point(1236.5, 1333.17);
    poly144.ps[2] = Point(1180.5, 1333.17);
    poly144.ps[3] = Point(1180.5, 1273.17);
    new ShapeRef(router, poly144, 144);

    Polygon poly145(4);
    poly145.ps[0] = Point(1236.5, 1069.17);
    poly145.ps[1] = Point(1236.5, 1129.17);
    poly145.ps[2] = Point(1180.5, 1129.17);
    poly145.ps[3] = Point(1180.5, 1069.17);
    new ShapeRef(router, poly145, 145);

    Polygon poly146(4);
    poly146.ps[0] = Point(220.5, 1910.17);
    poly146.ps[1] = Point(220.5, 1950.17);
    poly146.ps[2] = Point(144.5, 1950.17);
    poly146.ps[3] = Point(144.5, 1910.17);
    new ShapeRef(router, poly146, 146);

    Polygon poly147(4);
    poly147.ps[0] = Point(1396.5, 709.5);
    poly147.ps[1] = Point(1396.5, 749.5);
    poly147.ps[2] = Point(1320.5, 749.5);
    poly147.ps[3] = Point(1320.5, 709.5);
    new ShapeRef(router, poly147, 147);

    Polygon poly148(4);
    poly148.ps[0] = Point(628.5, 497);
    poly148.ps[1] = Point(628.5, 557);
    poly148.ps[2] = Point(572.5, 557);
    poly148.ps[3] = Point(572.5, 497);
    new ShapeRef(router, poly148, 148);

    Polygon poly149(4);
    poly149.ps[0] = Point(535.5, 744.75);
    poly149.ps[1] = Point(535.5, 804.75);
    poly149.ps[2] = Point(479.5, 804.75);
    poly149.ps[3] = Point(479.5, 744.75);
    new ShapeRef(router, poly149, 149);

    Polygon poly150(4);
    poly150.ps[0] = Point(1267.5, 2185.54);
    poly150.ps[1] = Point(1267.5, 2245.54);
    poly150.ps[2] = Point(1211.5, 2245.54);
    poly150.ps[3] = Point(1211.5, 2185.54);
    new ShapeRef(router, poly150, 150);

    Polygon poly151(4);
    poly151.ps[0] = Point(1570, 641);
    poly151.ps[1] = Point(1570, 681);
    poly151.ps[2] = Point(1510, 681);
    poly151.ps[3] = Point(1510, 641);
    new ShapeRef(router, poly151, 151);

    Polygon poly152(4);
    poly152.ps[0] = Point(1570, 681);
    poly152.ps[1] = Point(1570, 721);
    poly152.ps[2] = Point(1510, 721);
    poly152.ps[3] = Point(1510, 681);
    new ShapeRef(router, poly152, 152);

    Polygon poly153(4);
    poly153.ps[0] = Point(1621, 809.5);
    poly153.ps[1] = Point(1621, 869.5);
    poly153.ps[2] = Point(1581, 869.5);
    poly153.ps[3] = Point(1581, 809.5);
    new ShapeRef(router, poly153, 153);

    Polygon poly154(4);
    poly154.ps[0] = Point(1661, 809.5);
    poly154.ps[1] = Point(1661, 869.5);
    poly154.ps[2] = Point(1621, 869.5);
    poly154.ps[3] = Point(1621, 809.5);
    new ShapeRef(router, poly154, 154);

    Polygon poly155(4);
    poly155.ps[0] = Point(1370, 159.5);
    poly155.ps[1] = Point(1370, 219.5);
    poly155.ps[2] = Point(1330, 219.5);
    poly155.ps[3] = Point(1330, 159.5);
    new ShapeRef(router, poly155, 155);

    Polygon poly156(4);
    poly156.ps[0] = Point(252.5, 1705.67);
    poly156.ps[1] = Point(252.5, 1745.67);
    poly156.ps[2] = Point(192.5, 1745.67);
    poly156.ps[3] = Point(192.5, 1705.67);
    new ShapeRef(router, poly156, 156);

    Polygon poly157(4);
    poly157.ps[0] = Point(252.5, 1745.67);
    poly157.ps[1] = Point(252.5, 1785.67);
    poly157.ps[2] = Point(192.5, 1785.67);
    poly157.ps[3] = Point(192.5, 1745.67);
    new ShapeRef(router, poly157, 157);

    Polygon poly158(4);
    poly158.ps[0] = Point(849.5, 1342.67);
    poly158.ps[1] = Point(849.5, 1382.67);
    poly158.ps[2] = Point(789.5, 1382.67);
    poly158.ps[3] = Point(789.5, 1342.67);
    new ShapeRef(router, poly158, 158);

    Polygon poly159(4);
    poly159.ps[0] = Point(1556.5, 1943.54);
    poly159.ps[1] = Point(1556.5, 2003.54);
    poly159.ps[2] = Point(1516.5, 2003.54);
    poly159.ps[3] = Point(1516.5, 1943.54);
    new ShapeRef(router, poly159, 159);

    Polygon poly160(4);
    poly160.ps[0] = Point(661.5, 2058.54);
    poly160.ps[1] = Point(661.5, 2100.54);
    poly160.ps[2] = Point(593.5, 2100.54);
    poly160.ps[3] = Point(593.5, 2058.54);
    new ShapeRef(router, poly160, 160);

    Polygon poly161(4);
    poly161.ps[0] = Point(272.5, 1900.17);
    poly161.ps[1] = Point(272.5, 1960.17);
    poly161.ps[2] = Point(230.5, 1960.17);
    poly161.ps[3] = Point(230.5, 1900.17);
    new ShapeRef(router, poly161, 161);
#endif

    Polygon poly162(4);
    poly162.ps[0] = Point(630.5, 295);
    poly162.ps[1] = Point(630.5, 562);
    poly162.ps[2] = Point(562.5, 562);
    poly162.ps[3] = Point(562.5, 295);
    new ShapeRef(router, poly162, 162);

    Polygon poly163(4);
    poly163.ps[0] = Point(537.5, 542.75);
    poly163.ps[1] = Point(537.5, 734.75);
    poly163.ps[2] = Point(469.5, 734.75);
    poly163.ps[3] = Point(469.5, 542.75);
    new ShapeRef(router, poly163, 163);

#if 0
    Polygon poly164(4);
    poly164.ps[0] = Point(1401.5, 699.5);
    poly164.ps[1] = Point(1401.5, 759.5);
    poly164.ps[2] = Point(1268.5, 759.5);
    poly164.ps[3] = Point(1268.5, 699.5);
    new ShapeRef(router, poly164, 164);

    Polygon poly165(4);
    poly165.ps[0] = Point(405.5, 1582.17);
    poly165.ps[1] = Point(405.5, 1624.17);
    poly165.ps[2] = Point(337.5, 1624.17);
    poly165.ps[3] = Point(337.5, 1582.17);
    new ShapeRef(router, poly165, 165);

    Polygon poly166(4);
    poly166.ps[0] = Point(1529.5, 1171.17);
    poly166.ps[1] = Point(1529.5, 1231.17);
    poly166.ps[2] = Point(1396.5, 1231.17);
    poly166.ps[3] = Point(1396.5, 1171.17);
    new ShapeRef(router, poly166, 166);

    Polygon poly167(4);
    poly167.ps[0] = Point(1310.5, 997.167);
    poly167.ps[1] = Point(1310.5, 1057.17);
    poly167.ps[2] = Point(1268.5, 1057.17);
    poly167.ps[3] = Point(1268.5, 997.167);
    new ShapeRef(router, poly167, 167);

    Polygon poly168(4);
    poly168.ps[0] = Point(1269.5, 1983.54);
    poly168.ps[1] = Point(1269.5, 2175.54);
    poly168.ps[2] = Point(1201.5, 2175.54);
    poly168.ps[3] = Point(1201.5, 1983.54);
    new ShapeRef(router, poly168, 168);

    ConnRef *connRef169 = new ConnRef(router, 169);
    ConnEnd srcPt169(Point(1289.5, 886.5), 2);
    connRef169->setSourceEndpoint(srcPt169);
    ConnEnd dstPt169(Point(844.5, 966.167), 15);
    connRef169->setDestEndpoint(dstPt169);
    connRef169->setRoutingType((ConnType)2);

    ConnRef *connRef170 = new ConnRef(router, 170);
    ConnEnd srcPt170(Point(1289.5, 856.5), 1);
    connRef170->setSourceEndpoint(srcPt170);
    ConnEnd dstPt170(Point(1289.5, 790.5), 15);
    connRef170->setDestEndpoint(dstPt170);
    connRef170->setRoutingType((ConnType)2);

    ConnRef *connRef171 = new ConnRef(router, 171);
    ConnEnd srcPt171(Point(1289.5, 886.5), 2);
    connRef171->setSourceEndpoint(srcPt171);
    ConnEnd dstPt171(Point(1289.5, 966.167), 15);
    connRef171->setDestEndpoint(dstPt171);
    connRef171->setRoutingType((ConnType)2);

    ConnRef *connRef172 = new ConnRef(router, 172);
    ConnEnd srcPt172(Point(616.5, 2058.54), 4);
    connRef172->setSourceEndpoint(srcPt172);
    ConnEnd dstPt172(Point(251.5, 2079.54), 15);
    connRef172->setDestEndpoint(dstPt172);
    connRef172->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints172(1);
    checkpoints172[0] = Point(593.5, 2079.54);
    connRef172->setRoutingCheckpoints(checkpoints172);

    ConnRef *connRef173 = new ConnRef(router, 173);
    ConnEnd srcPt173(Point(646.5, 2058.54), 8);
    connRef173->setSourceEndpoint(srcPt173);
    ConnEnd dstPt173(Point(937.5, 2079.54), 15);
    connRef173->setDestEndpoint(dstPt173);
    connRef173->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints173(1);
    checkpoints173[0] = Point(669.5, 2079.54);
    connRef173->setRoutingCheckpoints(checkpoints173);

    ConnRef *connRef174 = new ConnRef(router, 174);
    ConnEnd srcPt174(Point(272.5, 1945.17), 2);
    connRef174->setSourceEndpoint(srcPt174);
    ConnEnd dstPt174(Point(251.5, 2079.54), 15);
    connRef174->setDestEndpoint(dstPt174);
    connRef174->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints174(1);
    checkpoints174[0] = Point(251.5, 1960.17);
    connRef174->setRoutingCheckpoints(checkpoints174);

    ConnRef *connRef175 = new ConnRef(router, 175);
    ConnEnd srcPt175(Point(616.5, 2100.54), 4);
    connRef175->setSourceEndpoint(srcPt175);
    ConnEnd dstPt175(Point(251.5, 2079.54), 15);
    connRef175->setDestEndpoint(dstPt175);
    connRef175->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints175(1);
    checkpoints175[0] = Point(593.5, 2079.54);
    connRef175->setRoutingCheckpoints(checkpoints175);

    ConnRef *connRef176 = new ConnRef(router, 176);
    ConnEnd srcPt176(Point(646.5, 2100.54), 8);
    connRef176->setSourceEndpoint(srcPt176);
    ConnEnd dstPt176(Point(937.5, 2079.54), 15);
    connRef176->setDestEndpoint(dstPt176);
    connRef176->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints176(1);
    checkpoints176[0] = Point(669.5, 2079.54);
    connRef176->setRoutingCheckpoints(checkpoints176);

    ConnRef *connRef177 = new ConnRef(router, 177);
    ConnEnd srcPt177(Point(251.5, 2079.54), 15);
    connRef177->setSourceEndpoint(srcPt177);
    ConnEnd dstPt177(Point(230.5, 1945.17), 2);
    connRef177->setDestEndpoint(dstPt177);
    connRef177->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints177(1);
    checkpoints177[0] = Point(251.5, 1960.17);
    connRef177->setRoutingCheckpoints(checkpoints177);

    ConnRef *connRef178 = new ConnRef(router, 178);
    ConnEnd srcPt178(Point(937.5, 2032.54), 2);
    connRef178->setSourceEndpoint(srcPt178);
    ConnEnd dstPt178(Point(937.5, 2079.54), 15);
    connRef178->setDestEndpoint(dstPt178);
    connRef178->setRoutingType((ConnType)2);

    ConnRef *connRef179 = new ConnRef(router, 179);
    ConnEnd srcPt179(Point(1224.5, 1983.54), 4);
    connRef179->setSourceEndpoint(srcPt179);
    ConnEnd dstPt179(Point(937.5, 2079.54), 15);
    connRef179->setDestEndpoint(dstPt179);
    connRef179->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints179(1);
    checkpoints179[0] = Point(1201.5, 2079.54);
    connRef179->setRoutingCheckpoints(checkpoints179);

    ConnRef *connRef180 = new ConnRef(router, 180);
    ConnEnd srcPt180(Point(1224.5, 2058.54), 4);
    connRef180->setSourceEndpoint(srcPt180);
    ConnEnd dstPt180(Point(937.5, 2079.54), 15);
    connRef180->setDestEndpoint(dstPt180);
    connRef180->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints180(1);
    checkpoints180[0] = Point(1201.5, 2079.54);
    connRef180->setRoutingCheckpoints(checkpoints180);

    ConnRef *connRef181 = new ConnRef(router, 181);
    ConnEnd srcPt181(Point(1104.5, 1879.54), 4);
    connRef181->setSourceEndpoint(srcPt181);
    ConnEnd dstPt181(Point(937.5, 1869.17), 15);
    connRef181->setDestEndpoint(dstPt181);
    connRef181->setRoutingType((ConnType)2);

    ConnRef *connRef182 = new ConnRef(router, 182);
    ConnEnd srcPt182(Point(1119.5, 1813.17), 2);
    connRef182->setSourceEndpoint(srcPt182);
    ConnEnd dstPt182(Point(937.5, 1869.17), 15);
    connRef182->setDestEndpoint(dstPt182);
    connRef182->setRoutingType((ConnType)2);

    ConnRef *connRef183 = new ConnRef(router, 183);
    ConnEnd srcPt183(Point(798.5, 1869.17), 4);
    connRef183->setSourceEndpoint(srcPt183);
    ConnEnd dstPt183(Point(569.5, 1869.17), 15);
    connRef183->setDestEndpoint(dstPt183);
    connRef183->setRoutingType((ConnType)2);

    ConnRef *connRef184 = new ConnRef(router, 184);
    ConnEnd srcPt184(Point(798.5, 1869.17), 4);
    connRef184->setSourceEndpoint(srcPt184);
    ConnEnd dstPt184(Point(937.5, 1765.17), 15);
    connRef184->setDestEndpoint(dstPt184);
    connRef184->setRoutingType((ConnType)2);

    ConnRef *connRef185 = new ConnRef(router, 185);
    ConnEnd srcPt185(Point(391.5, 1869.17), 4);
    connRef185->setSourceEndpoint(srcPt185);
    ConnEnd dstPt185(Point(251.5, 1869.17), 15);
    connRef185->setDestEndpoint(dstPt185);
    connRef185->setRoutingType((ConnType)2);

    ConnRef *connRef186 = new ConnRef(router, 186);
    ConnEnd srcPt186(Point(421.5, 1869.17), 8);
    connRef186->setSourceEndpoint(srcPt186);
    ConnEnd dstPt186(Point(569.5, 1869.17), 15);
    connRef186->setDestEndpoint(dstPt186);
    connRef186->setRoutingType((ConnType)2);

    ConnRef *connRef187 = new ConnRef(router, 187);
    ConnEnd srcPt187(Point(391.5, 1869.17), 4);
    connRef187->setSourceEndpoint(srcPt187);
    ConnEnd dstPt187(Point(406.5, 2012.21), 15);
    connRef187->setDestEndpoint(dstPt187);
    connRef187->setRoutingType((ConnType)2);

    ConnRef *connRef188 = new ConnRef(router, 188);
    ConnEnd srcPt188(Point(1471.5, 1869.17), 8);
    connRef188->setSourceEndpoint(srcPt188);
    ConnEnd dstPt188(Point(1530.5, 1869.17), 15);
    connRef188->setDestEndpoint(dstPt188);
    connRef188->setRoutingType((ConnType)2);

    ConnRef *connRef189 = new ConnRef(router, 189);
    ConnEnd srcPt189(Point(1441.5, 1869.17), 4);
    connRef189->setSourceEndpoint(srcPt189);
    ConnEnd dstPt189(Point(1289.5, 1869.17), 15);
    connRef189->setDestEndpoint(dstPt189);
    connRef189->setRoutingType((ConnType)2);

    ConnRef *connRef190 = new ConnRef(router, 190);
    ConnEnd srcPt190(Point(1289.5, 1680.17), 2);
    connRef190->setSourceEndpoint(srcPt190);
    ConnEnd dstPt190(Point(1289.5, 1869.17), 15);
    connRef190->setDestEndpoint(dstPt190);
    connRef190->setRoutingType((ConnType)2);

    ConnRef *connRef191 = new ConnRef(router, 191);
    ConnEnd srcPt191(Point(1289.5, 1526.17), 1);
    connRef191->setSourceEndpoint(srcPt191);
    ConnEnd dstPt191(Point(1289.5, 1479.17), 15);
    connRef191->setDestEndpoint(dstPt191);
    connRef191->setRoutingType((ConnType)2);

    ConnRef *connRef192 = new ConnRef(router, 192);
    ConnEnd srcPt192(Point(1289.5, 1556.17), 2);
    connRef192->setSourceEndpoint(srcPt192);
    ConnEnd dstPt192(Point(1289.5, 1603.17), 15);
    connRef192->setDestEndpoint(dstPt192);
    connRef192->setRoutingType((ConnType)2);

    ConnRef *connRef193 = new ConnRef(router, 193);
    ConnEnd srcPt193(Point(1289.5, 1432.17), 2);
    connRef193->setSourceEndpoint(srcPt193);
    ConnEnd dstPt193(Point(1289.5, 1479.17), 15);
    connRef193->setDestEndpoint(dstPt193);
    connRef193->setRoutingType((ConnType)2);

    ConnRef *connRef194 = new ConnRef(router, 194);
    ConnEnd srcPt194(Point(1289.5, 1402.17), 1);
    connRef194->setSourceEndpoint(srcPt194);
    ConnEnd dstPt194(Point(1289.5, 1355.17), 15);
    connRef194->setDestEndpoint(dstPt194);
    connRef194->setRoutingType((ConnType)2);

    ConnRef *connRef195 = new ConnRef(router, 195);
    ConnEnd srcPt195(Point(1268.5, 1042.17), 2);
    connRef195->setSourceEndpoint(srcPt195);
    ConnEnd dstPt195(Point(1289.5, 1139.17), 15);
    connRef195->setDestEndpoint(dstPt195);
    connRef195->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints195(1);
    checkpoints195[0] = Point(1289.5, 1057.17);
    connRef195->setRoutingCheckpoints(checkpoints195);

    ConnRef *connRef196 = new ConnRef(router, 196);
    ConnEnd srcPt196(Point(1268.5, 1012.17), 1);
    connRef196->setSourceEndpoint(srcPt196);
    ConnEnd dstPt196(Point(1289.5, 966.167), 15);
    connRef196->setDestEndpoint(dstPt196);
    connRef196->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints196(1);
    checkpoints196[0] = Point(1289.5, 997.167);
    connRef196->setRoutingCheckpoints(checkpoints196);

    ConnRef *connRef197 = new ConnRef(router, 197);
    ConnEnd srcPt197(Point(1073.5, 966.167), 4);
    connRef197->setSourceEndpoint(srcPt197);
    ConnEnd dstPt197(Point(844.5, 966.167), 15);
    connRef197->setDestEndpoint(dstPt197);
    connRef197->setRoutingType((ConnType)2);

    ConnRef *connRef198 = new ConnRef(router, 198);
    ConnEnd srcPt198(Point(1103.5, 966.167), 8);
    connRef198->setSourceEndpoint(srcPt198);
    ConnEnd dstPt198(Point(1289.5, 966.167), 15);
    connRef198->setDestEndpoint(dstPt198);
    connRef198->setRoutingType((ConnType)2);

    ConnRef *connRef199 = new ConnRef(router, 199);
    ConnEnd srcPt199(Point(1268.5, 744.5), 2);
    connRef199->setSourceEndpoint(srcPt199);
    ConnEnd dstPt199(Point(1289.5, 790.5), 15);
    connRef199->setDestEndpoint(dstPt199);
    connRef199->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints199(1);
    checkpoints199[0] = Point(1289.5, 759.5);
    connRef199->setRoutingCheckpoints(checkpoints199);

    ConnRef *connRef200 = new ConnRef(router, 200);
    ConnEnd srcPt200(Point(1401.5, 714.5), 1);
    connRef200->setSourceEndpoint(srcPt200);
    ConnEnd dstPt200(Point(1289.5, 638.75), 15);
    connRef200->setDestEndpoint(dstPt200);
    connRef200->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints200(1);
    checkpoints200[0] = Point(1289.5, 699.5);
    connRef200->setRoutingCheckpoints(checkpoints200);

    ConnRef *connRef201 = new ConnRef(router, 201);
    ConnEnd srcPt201(Point(1410.5, 432.5), 4);
    connRef201->setSourceEndpoint(srcPt201);
    ConnEnd dstPt201(Point(1289.5, 466), 15);
    connRef201->setDestEndpoint(dstPt201);
    connRef201->setRoutingType((ConnType)2);

    ConnRef *connRef202 = new ConnRef(router, 202);
    ConnEnd srcPt202(Point(1289.5, 536.25), 1);
    connRef202->setSourceEndpoint(srcPt202);
    ConnEnd dstPt202(Point(1289.5, 466), 15);
    connRef202->setDestEndpoint(dstPt202);
    connRef202->setRoutingType((ConnType)2);

    ConnRef *connRef203 = new ConnRef(router, 203);
    ConnEnd srcPt203(Point(1289.5, 566.25), 2);
    connRef203->setSourceEndpoint(srcPt203);
    ConnEnd dstPt203(Point(1289.5, 638.75), 15);
    connRef203->setDestEndpoint(dstPt203);
    connRef203->setRoutingType((ConnType)2);

    ConnRef *connRef204 = new ConnRef(router, 204);
    ConnEnd srcPt204(Point(1289.5, 345), 1);
    connRef204->setSourceEndpoint(srcPt204);
    ConnEnd dstPt204(Point(1289.5, 280), 15);
    connRef204->setDestEndpoint(dstPt204);
    connRef204->setRoutingType((ConnType)2);

    ConnRef *connRef205 = new ConnRef(router, 205);
    ConnEnd srcPt205(Point(1289.5, 375), 2);
    connRef205->setSourceEndpoint(srcPt205);
    ConnEnd dstPt205(Point(1289.5, 466), 15);
    connRef205->setDestEndpoint(dstPt205);
    connRef205->setRoutingType((ConnType)2);

    ConnRef *connRef206 = new ConnRef(router, 206);
    ConnEnd srcPt206(Point(1011.5, 638.75), 4);
    connRef206->setSourceEndpoint(srcPt206);
    ConnEnd dstPt206(Point(844.5, 638.75), 15);
    connRef206->setDestEndpoint(dstPt206);
    connRef206->setRoutingType((ConnType)2);

    ConnRef *connRef207 = new ConnRef(router, 207);
    ConnEnd srcPt207(Point(1041.5, 638.75), 8);
    connRef207->setSourceEndpoint(srcPt207);
    ConnEnd dstPt207(Point(1289.5, 638.75), 15);
    connRef207->setDestEndpoint(dstPt207);
    connRef207->setRoutingType((ConnType)2);

    ConnRef *connRef208 = new ConnRef(router, 208);
    ConnEnd srcPt208(Point(522.5, 542.75), 8);
    connRef208->setSourceEndpoint(srcPt208);
    ConnEnd dstPt208(Point(844.5, 638.75), 15);
    connRef208->setDestEndpoint(dstPt208);
    connRef208->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints208(1);
    checkpoints208[0] = Point(545.5, 638.75);
    connRef208->setRoutingCheckpoints(checkpoints208);

    ConnRef *connRef209 = new ConnRef(router, 209);
    ConnEnd srcPt209(Point(522.5, 617.75), 8);
    connRef209->setSourceEndpoint(srcPt209);
    ConnEnd dstPt209(Point(844.5, 638.75), 15);
    connRef209->setDestEndpoint(dstPt209);
    connRef209->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints209(1);
    checkpoints209[0] = Point(545.5, 638.75);
    connRef209->setRoutingCheckpoints(checkpoints209);

    ConnRef *connRef210 = new ConnRef(router, 210);
    ConnEnd srcPt210(Point(891.5, 551.25), 4);
    connRef210->setSourceEndpoint(srcPt210);
    ConnEnd dstPt210(Point(844.5, 638.75), 15);
    connRef210->setDestEndpoint(dstPt210);
    connRef210->setRoutingType((ConnType)2);

    ConnRef *connRef211 = new ConnRef(router, 211);
    ConnEnd srcPt211(Point(921.5, 551.25), 8);
    connRef211->setSourceEndpoint(srcPt211);
    ConnEnd dstPt211(Point(1289.5, 466), 15);
    connRef211->setDestEndpoint(dstPt211);
    connRef211->setRoutingType((ConnType)2);

    ConnRef *connRef212 = new ConnRef(router, 212);
    ConnEnd srcPt212(Point(751.5, 566.25), 2);
    connRef212->setSourceEndpoint(srcPt212);
    ConnEnd dstPt212(Point(844.5, 638.75), 15);
    connRef212->setDestEndpoint(dstPt212);
    connRef212->setRoutingType((ConnType)2);

    ConnRef *connRef213 = new ConnRef(router, 213);
    ConnEnd srcPt213(Point(751.5, 536.25), 1);
    connRef213->setSourceEndpoint(srcPt213);
    ConnEnd dstPt213(Point(1289.5, 466), 15);
    connRef213->setDestEndpoint(dstPt213);
    connRef213->setRoutingType((ConnType)2);

    ConnRef *connRef214 = new ConnRef(router, 214);
    ConnEnd srcPt214(Point(286.5, 536.25), 1);
    connRef214->setSourceEndpoint(srcPt214);
    ConnEnd dstPt214(Point(286.5, 466), 15);
    connRef214->setDestEndpoint(dstPt214);
    connRef214->setRoutingType((ConnType)2);

    ConnRef *connRef215 = new ConnRef(router, 215);
    ConnEnd srcPt215(Point(286.5, 566.25), 2);
    connRef215->setSourceEndpoint(srcPt215);
    ConnEnd dstPt215(Point(286.5, 638.75), 15);
    connRef215->setDestEndpoint(dstPt215);
    connRef215->setRoutingType((ConnType)2);

    ConnRef *connRef216 = new ConnRef(router, 216);
    ConnEnd srcPt216(Point(615.5, 295), 8);
    connRef216->setSourceEndpoint(srcPt216);
    ConnEnd dstPt216(Point(1289.5, 466), 15);
    connRef216->setDestEndpoint(dstPt216);
    connRef216->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints216(1);
    checkpoints216[0] = Point(638.5, 466);
    connRef216->setRoutingCheckpoints(checkpoints216);

    ConnRef *connRef217 = new ConnRef(router, 217);
    ConnEnd srcPt217(Point(615.5, 370), 8);
    connRef217->setSourceEndpoint(srcPt217);
    ConnEnd dstPt217(Point(1289.5, 466), 15);
    connRef217->setDestEndpoint(dstPt217);
    connRef217->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints217(1);
    checkpoints217[0] = Point(638.5, 466);
    connRef217->setRoutingCheckpoints(checkpoints217);

    ConnRef *connRef218 = new ConnRef(router, 218);
    ConnEnd srcPt218(Point(585.5, 562), 4);
    connRef218->setSourceEndpoint(srcPt218);
    ConnEnd dstPt218(Point(286.5, 466), 15);
    connRef218->setDestEndpoint(dstPt218);
    connRef218->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints218(1);
    checkpoints218[0] = Point(562.5, 466);
    connRef218->setRoutingCheckpoints(checkpoints218);
#endif

    ConnRef *connRef219 = new ConnRef(router, 219);
    ConnEnd srcPt219(Point(860.5, 341), 4);
    connRef219->setSourceEndpoint(srcPt219);
    ConnEnd dstPt219(Point(286.5, 466), 15);
    connRef219->setDestEndpoint(dstPt219);
    connRef219->setRoutingType((ConnType)2);

    ConnRef *connRef220 = new ConnRef(router, 220);
    ConnEnd srcPt220(Point(860.5, 239), 4);
    connRef220->setSourceEndpoint(srcPt220);
    ConnEnd dstPt220(Point(286.5, 466), 15);
    connRef220->setDestEndpoint(dstPt220);
    connRef220->setRoutingType((ConnType)2);

#if 0
    ConnRef *connRef221 = new ConnRef(router, 221);
    ConnEnd srcPt221(Point(553.5, 2012.21), 8);
    connRef221->setSourceEndpoint(srcPt221);
    ConnEnd dstPt221(Point(782.5, 2012.21), 15);
    connRef221->setDestEndpoint(dstPt221);
    connRef221->setRoutingType((ConnType)2);

    ConnRef *connRef222 = new ConnRef(router, 222);
    ConnEnd srcPt222(Point(1289.5, 1278.17), 2);
    connRef222->setSourceEndpoint(srcPt222);
    ConnEnd dstPt222(Point(1289.5, 1355.17), 15);
    connRef222->setDestEndpoint(dstPt222);
    connRef222->setRoutingType((ConnType)2);

    ConnRef *connRef223 = new ConnRef(router, 223);
    ConnEnd srcPt223(Point(1289.5, 1248.17), 1);
    connRef223->setSourceEndpoint(srcPt223);
    ConnEnd dstPt223(Point(1289.5, 1139.17), 15);
    connRef223->setDestEndpoint(dstPt223);
    connRef223->setRoutingType((ConnType)2);

    ConnRef *connRef224 = new ConnRef(router, 224);
    ConnEnd srcPt224(Point(615.5, 445), 8);
    connRef224->setSourceEndpoint(srcPt224);
    ConnEnd dstPt224(Point(1289.5, 466), 15);
    connRef224->setDestEndpoint(dstPt224);
    connRef224->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints224(1);
    checkpoints224[0] = Point(638.5, 466);
    connRef224->setRoutingCheckpoints(checkpoints224);

    ConnRef *connRef225 = new ConnRef(router, 225);
    ConnEnd srcPt225(Point(522.5, 659.75), 8);
    connRef225->setSourceEndpoint(srcPt225);
    ConnEnd dstPt225(Point(844.5, 638.75), 15);
    connRef225->setDestEndpoint(dstPt225);
    connRef225->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints225(1);
    checkpoints225[0] = Point(545.5, 638.75);
    connRef225->setRoutingCheckpoints(checkpoints225);

    ConnRef *connRef226 = new ConnRef(router, 226);
    ConnEnd srcPt226(Point(301.5, 750.75), 8);
    connRef226->setSourceEndpoint(srcPt226);
    ConnEnd dstPt226(Point(437.5, 750.75), 15);
    connRef226->setDestEndpoint(dstPt226);
    connRef226->setRoutingType((ConnType)2);

    ConnRef *connRef227 = new ConnRef(router, 227);
    ConnEnd srcPt227(Point(301.5, 852.75), 8);
    connRef227->setSourceEndpoint(srcPt227);
    ConnEnd dstPt227(Point(437.5, 852.75), 15);
    connRef227->setDestEndpoint(dstPt227);
    connRef227->setRoutingType((ConnType)2);

    ConnRef *connRef228 = new ConnRef(router, 228);
    ConnEnd srcPt228(Point(677.5, 750.75), 8);
    connRef228->setSourceEndpoint(srcPt228);
    ConnEnd dstPt228(Point(844.5, 638.75), 15);
    connRef228->setDestEndpoint(dstPt228);
    connRef228->setRoutingType((ConnType)2);

    ConnRef *connRef229 = new ConnRef(router, 229);
    ConnEnd srcPt229(Point(677.5, 852.75), 8);
    connRef229->setSourceEndpoint(srcPt229);
    ConnEnd dstPt229(Point(844.5, 638.75), 15);
    connRef229->setDestEndpoint(dstPt229);
    connRef229->setRoutingType((ConnType)2);

    ConnRef *connRef230 = new ConnRef(router, 230);
    ConnEnd srcPt230(Point(1499.5, 432.5), 15);
    connRef230->setSourceEndpoint(srcPt230);
    ConnEnd dstPt230(Point(1440.5, 432.5), 8);
    connRef230->setDestEndpoint(dstPt230);
    connRef230->setRoutingType((ConnType)2);

    ConnRef *connRef231 = new ConnRef(router, 231);
    ConnEnd srcPt231(Point(286.5, 466), 15);
    connRef231->setSourceEndpoint(srcPt231);
    ConnEnd dstPt231(Point(585.5, 445), 4);
    connRef231->setDestEndpoint(dstPt231);
    connRef231->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints231(1);
    checkpoints231[0] = Point(562.5, 466);
    connRef231->setRoutingCheckpoints(checkpoints231);

    ConnRef *connRef232 = new ConnRef(router, 232);
    ConnEnd srcPt232(Point(286.5, 466), 15);
    connRef232->setSourceEndpoint(srcPt232);
    ConnEnd dstPt232(Point(585.5, 487), 4);
    connRef232->setDestEndpoint(dstPt232);
    connRef232->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints232(1);
    checkpoints232[0] = Point(562.5, 466);
    connRef232->setRoutingCheckpoints(checkpoints232);

    ConnRef *connRef233 = new ConnRef(router, 233);
    ConnEnd srcPt233(Point(286.5, 638.75), 15);
    connRef233->setSourceEndpoint(srcPt233);
    ConnEnd dstPt233(Point(492.5, 659.75), 4);
    connRef233->setDestEndpoint(dstPt233);
    connRef233->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints233(1);
    checkpoints233[0] = Point(469.5, 638.75);
    connRef233->setRoutingCheckpoints(checkpoints233);

    ConnRef *connRef234 = new ConnRef(router, 234);
    ConnEnd srcPt234(Point(492.5, 734.75), 4);
    connRef234->setSourceEndpoint(srcPt234);
    ConnEnd dstPt234(Point(286.5, 638.75), 15);
    connRef234->setDestEndpoint(dstPt234);
    connRef234->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints234(1);
    checkpoints234[0] = Point(469.5, 638.75);
    connRef234->setRoutingCheckpoints(checkpoints234);

    ConnRef *connRef235 = new ConnRef(router, 235);
    ConnEnd srcPt235(Point(522.5, 734.75), 8);
    connRef235->setSourceEndpoint(srcPt235);
    ConnEnd dstPt235(Point(844.5, 638.75), 15);
    connRef235->setDestEndpoint(dstPt235);
    connRef235->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints235(1);
    checkpoints235[0] = Point(545.5, 638.75);
    connRef235->setRoutingCheckpoints(checkpoints235);

    ConnRef *connRef236 = new ConnRef(router, 236);
    ConnEnd srcPt236(Point(1289.5, 638.75), 15);
    connRef236->setSourceEndpoint(srcPt236);
    ConnEnd dstPt236(Point(1310.5, 714.5), 1);
    connRef236->setDestEndpoint(dstPt236);
    connRef236->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints236(1);
    checkpoints236[0] = Point(1289.5, 699.5);
    connRef236->setRoutingCheckpoints(checkpoints236);

    ConnRef *connRef237 = new ConnRef(router, 237);
    ConnEnd srcPt237(Point(390.5, 1624.17), 8);
    connRef237->setSourceEndpoint(srcPt237);
    ConnEnd dstPt237(Point(1289.5, 1603.17), 15);
    connRef237->setDestEndpoint(dstPt237);
    connRef237->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints237(1);
    checkpoints237[0] = Point(413.5, 1603.17);
    connRef237->setRoutingCheckpoints(checkpoints237);

    ConnRef *connRef238 = new ConnRef(router, 238);
    ConnEnd srcPt238(Point(1396.5, 1216.17), 2);
    connRef238->setSourceEndpoint(srcPt238);
    ConnEnd dstPt238(Point(1289.5, 1355.17), 15);
    connRef238->setDestEndpoint(dstPt238);
    connRef238->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints238(1);
    checkpoints238[0] = Point(1417.5, 1231.17);
    connRef238->setRoutingCheckpoints(checkpoints238);

    ConnRef *connRef239 = new ConnRef(router, 239);
    ConnEnd srcPt239(Point(1438.5, 1216.17), 2);
    connRef239->setSourceEndpoint(srcPt239);
    ConnEnd dstPt239(Point(1289.5, 1355.17), 15);
    connRef239->setDestEndpoint(dstPt239);
    connRef239->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints239(1);
    checkpoints239[0] = Point(1417.5, 1231.17);
    connRef239->setRoutingCheckpoints(checkpoints239);

    ConnRef *connRef240 = new ConnRef(router, 240);
    ConnEnd srcPt240(Point(1529.5, 1216.17), 2);
    connRef240->setSourceEndpoint(srcPt240);
    ConnEnd dstPt240(Point(1289.5, 1355.17), 15);
    connRef240->setDestEndpoint(dstPt240);
    connRef240->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints240(1);
    checkpoints240[0] = Point(1417.5, 1231.17);
    connRef240->setRoutingCheckpoints(checkpoints240);

    ConnRef *connRef241 = new ConnRef(router, 241);
    ConnEnd srcPt241(Point(1310.5, 1042.17), 2);
    connRef241->setSourceEndpoint(srcPt241);
    ConnEnd dstPt241(Point(1289.5, 1139.17), 15);
    connRef241->setDestEndpoint(dstPt241);
    connRef241->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints241(1);
    checkpoints241[0] = Point(1289.5, 1057.17);
    connRef241->setRoutingCheckpoints(checkpoints241);

    ConnRef *connRef242 = new ConnRef(router, 242);
    ConnEnd srcPt242(Point(1223.5, 1139.17), 8);
    connRef242->setSourceEndpoint(srcPt242);
    ConnEnd dstPt242(Point(1289.5, 1139.17), 15);
    connRef242->setDestEndpoint(dstPt242);
    connRef242->setRoutingType((ConnType)2);

    ConnRef *connRef243 = new ConnRef(router, 243);
    ConnEnd srcPt243(Point(1254.5, 1983.54), 8);
    connRef243->setSourceEndpoint(srcPt243);
    ConnEnd dstPt243(Point(1425.5, 2079.54), 15);
    connRef243->setDestEndpoint(dstPt243);
    connRef243->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints243(1);
    checkpoints243[0] = Point(1277.5, 2079.54);
    connRef243->setRoutingCheckpoints(checkpoints243);

    ConnRef *connRef244 = new ConnRef(router, 244);
    ConnEnd srcPt244(Point(1254.5, 2058.54), 8);
    connRef244->setSourceEndpoint(srcPt244);
    ConnEnd dstPt244(Point(1425.5, 2079.54), 15);
    connRef244->setDestEndpoint(dstPt244);
    connRef244->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints244(1);
    checkpoints244[0] = Point(1277.5, 2079.54);
    connRef244->setRoutingCheckpoints(checkpoints244);

    ConnRef *connRef245 = new ConnRef(router, 245);
    ConnEnd srcPt245(Point(1254.5, 2100.54), 8);
    connRef245->setSourceEndpoint(srcPt245);
    ConnEnd dstPt245(Point(1425.5, 2079.54), 15);
    connRef245->setDestEndpoint(dstPt245);
    connRef245->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints245(1);
    checkpoints245[0] = Point(1277.5, 2079.54);
    connRef245->setRoutingCheckpoints(checkpoints245);

    ConnRef *connRef246 = new ConnRef(router, 246);
    ConnEnd srcPt246(Point(1254.5, 2175.54), 8);
    connRef246->setSourceEndpoint(srcPt246);
    ConnEnd dstPt246(Point(1425.5, 2079.54), 15);
    connRef246->setDestEndpoint(dstPt246);
    connRef246->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints246(1);
    checkpoints246[0] = Point(1277.5, 2079.54);
    connRef246->setRoutingCheckpoints(checkpoints246);

    ConnRef *connRef247 = new ConnRef(router, 247);
    ConnEnd srcPt247(Point(677.5, 1740.17), 8);
    connRef247->setSourceEndpoint(srcPt247);
    ConnEnd dstPt247(Point(1289.5, 1869.17), 15);
    connRef247->setDestEndpoint(dstPt247);
    connRef247->setRoutingType((ConnType)2);

    ConnRef *connRef248 = new ConnRef(router, 248);
    ConnEnd srcPt248(Point(937.5, 1869.17), 15);
    connRef248->setSourceEndpoint(srcPt248);
    ConnEnd dstPt248(Point(937.5, 2002.54), 1);
    connRef248->setDestEndpoint(dstPt248);
    connRef248->setRoutingType((ConnType)2);

    ConnRef *connRef249 = new ConnRef(router, 249);
    ConnEnd srcPt249(Point(647.5, 1740.17), 4);
    connRef249->setSourceEndpoint(srcPt249);
    ConnEnd dstPt249(Point(251.5, 1869.17), 15);
    connRef249->setDestEndpoint(dstPt249);
    connRef249->setRoutingType((ConnType)2);

    ConnRef *connRef250 = new ConnRef(router, 250);
    ConnEnd srcPt250(Point(1224.5, 2100.54), 4);
    connRef250->setSourceEndpoint(srcPt250);
    ConnEnd dstPt250(Point(937.5, 2079.54), 15);
    connRef250->setDestEndpoint(dstPt250);
    connRef250->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints250(1);
    checkpoints250[0] = Point(1201.5, 2079.54);
    connRef250->setRoutingCheckpoints(checkpoints250);

    ConnRef *connRef251 = new ConnRef(router, 251);
    ConnEnd srcPt251(Point(1224.5, 2175.54), 4);
    connRef251->setSourceEndpoint(srcPt251);
    ConnEnd dstPt251(Point(937.5, 2079.54), 15);
    connRef251->setDestEndpoint(dstPt251);
    connRef251->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints251(1);
    checkpoints251[0] = Point(1201.5, 2079.54);
    connRef251->setRoutingCheckpoints(checkpoints251);

    ConnRef *connRef252 = new ConnRef(router, 252);
    ConnEnd srcPt252(Point(523.5, 2012.21), 4);
    connRef252->setSourceEndpoint(srcPt252);
    ConnEnd dstPt252(Point(406.5, 2012.21), 15);
    connRef252->setDestEndpoint(dstPt252);
    connRef252->setRoutingType((ConnType)2);

    ConnRef *connRef253 = new ConnRef(router, 253);
    ConnEnd srcPt253(Point(1289.5, 790.5), 15);
    connRef253->setSourceEndpoint(srcPt253);
    ConnEnd dstPt253(Point(1401.5, 744.5), 2);
    connRef253->setDestEndpoint(dstPt253);
    connRef253->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints253(1);
    checkpoints253[0] = Point(1289.5, 759.5);
    connRef253->setRoutingCheckpoints(checkpoints253);

    ConnRef *connRef254 = new ConnRef(router, 254);
    ConnEnd srcPt254(Point(216.5, 750.75), 15);
    connRef254->setSourceEndpoint(srcPt254);
    ConnEnd dstPt254(Point(271.5, 750.75), 4);
    connRef254->setDestEndpoint(dstPt254);
    connRef254->setRoutingType((ConnType)2);

    ConnRef *connRef255 = new ConnRef(router, 255);
    ConnEnd srcPt255(Point(437.5, 750.75), 15);
    connRef255->setSourceEndpoint(srcPt255);
    ConnEnd dstPt255(Point(647.5, 750.75), 4);
    connRef255->setDestEndpoint(dstPt255);
    connRef255->setRoutingType((ConnType)2);

    ConnRef *connRef256 = new ConnRef(router, 256);
    ConnEnd srcPt256(Point(437.5, 852.75), 15);
    connRef256->setSourceEndpoint(srcPt256);
    ConnEnd dstPt256(Point(647.5, 852.75), 4);
    connRef256->setDestEndpoint(dstPt256);
    connRef256->setRoutingType((ConnType)2);

    ConnRef *connRef257 = new ConnRef(router, 257);
    ConnEnd srcPt257(Point(216.5, 852.75), 15);
    connRef257->setSourceEndpoint(srcPt257);
    ConnEnd dstPt257(Point(271.5, 852.75), 4);
    connRef257->setDestEndpoint(dstPt257);
    connRef257->setRoutingType((ConnType)2);

    ConnRef *connRef258 = new ConnRef(router, 258);
    ConnEnd srcPt258(Point(390.5, 1582.17), 8);
    connRef258->setSourceEndpoint(srcPt258);
    ConnEnd dstPt258(Point(1289.5, 1603.17), 15);
    connRef258->setDestEndpoint(dstPt258);
    connRef258->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints258(1);
    checkpoints258[0] = Point(413.5, 1603.17);
    connRef258->setRoutingCheckpoints(checkpoints258);

    ConnRef *connRef259 = new ConnRef(router, 259);
    ConnEnd srcPt259(Point(251.5, 1603.17), 15);
    connRef259->setSourceEndpoint(srcPt259);
    ConnEnd dstPt259(Point(360.5, 1582.17), 4);
    connRef259->setDestEndpoint(dstPt259);
    connRef259->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints259(1);
    checkpoints259[0] = Point(337.5, 1603.17);
    connRef259->setRoutingCheckpoints(checkpoints259);

    ConnRef *connRef260 = new ConnRef(router, 260);
    ConnEnd srcPt260(Point(251.5, 1603.17), 15);
    connRef260->setSourceEndpoint(srcPt260);
    ConnEnd dstPt260(Point(360.5, 1624.17), 4);
    connRef260->setDestEndpoint(dstPt260);
    connRef260->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints260(1);
    checkpoints260[0] = Point(337.5, 1603.17);
    connRef260->setRoutingCheckpoints(checkpoints260);

    ConnRef *connRef261 = new ConnRef(router, 261);
    ConnEnd srcPt261(Point(1310.5, 1012.17), 1);
    connRef261->setSourceEndpoint(srcPt261);
    ConnEnd dstPt261(Point(1289.5, 966.167), 15);
    connRef261->setDestEndpoint(dstPt261);
    connRef261->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints261(1);
    checkpoints261[0] = Point(1289.5, 997.167);
    connRef261->setRoutingCheckpoints(checkpoints261);

    ConnRef *connRef262 = new ConnRef(router, 262);
    ConnEnd srcPt262(Point(1193.5, 1343.17), 4);
    connRef262->setSourceEndpoint(srcPt262);
    ConnEnd dstPt262(Point(1057.5, 1263.17), 15);
    connRef262->setDestEndpoint(dstPt262);
    connRef262->setRoutingType((ConnType)2);

    ConnRef *connRef263 = new ConnRef(router, 263);
    ConnEnd srcPt263(Point(1223.5, 1343.17), 8);
    connRef263->setSourceEndpoint(srcPt263);
    ConnEnd dstPt263(Point(1289.5, 1355.17), 15);
    connRef263->setDestEndpoint(dstPt263);
    connRef263->setRoutingType((ConnType)2);

    ConnRef *connRef264 = new ConnRef(router, 264);
    ConnEnd srcPt264(Point(1193.5, 1139.17), 4);
    connRef264->setSourceEndpoint(srcPt264);
    ConnEnd dstPt264(Point(1057.5, 1263.17), 15);
    connRef264->setDestEndpoint(dstPt264);
    connRef264->setRoutingType((ConnType)2);

    ConnRef *connRef265 = new ConnRef(router, 265);
    ConnEnd srcPt265(Point(1289.5, 966.167), 15);
    connRef265->setSourceEndpoint(srcPt265);
    ConnEnd dstPt265(Point(1396.5, 1186.17), 1);
    connRef265->setDestEndpoint(dstPt265);
    connRef265->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints265(1);
    checkpoints265[0] = Point(1417.5, 1171.17);
    connRef265->setRoutingCheckpoints(checkpoints265);

    ConnRef *connRef266 = new ConnRef(router, 266);
    ConnEnd srcPt266(Point(1289.5, 966.167), 15);
    connRef266->setSourceEndpoint(srcPt266);
    ConnEnd dstPt266(Point(1438.5, 1186.17), 1);
    connRef266->setDestEndpoint(dstPt266);
    connRef266->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints266(1);
    checkpoints266[0] = Point(1417.5, 1171.17);
    connRef266->setRoutingCheckpoints(checkpoints266);

    ConnRef *connRef267 = new ConnRef(router, 267);
    ConnEnd srcPt267(Point(1289.5, 966.167), 15);
    connRef267->setSourceEndpoint(srcPt267);
    ConnEnd dstPt267(Point(1529.5, 1186.17), 1);
    connRef267->setDestEndpoint(dstPt267);
    connRef267->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints267(1);
    checkpoints267[0] = Point(1417.5, 1171.17);
    connRef267->setRoutingCheckpoints(checkpoints267);

    ConnRef *connRef268 = new ConnRef(router, 268);
    ConnEnd srcPt268(Point(230.5, 1915.17), 1);
    connRef268->setSourceEndpoint(srcPt268);
    ConnEnd dstPt268(Point(251.5, 1869.17), 15);
    connRef268->setDestEndpoint(dstPt268);
    connRef268->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints268(1);
    checkpoints268[0] = Point(251.5, 1900.17);
    connRef268->setRoutingCheckpoints(checkpoints268);

    ConnRef *connRef269 = new ConnRef(router, 269);
    ConnEnd srcPt269(Point(1310.5, 744.5), 2);
    connRef269->setSourceEndpoint(srcPt269);
    ConnEnd dstPt269(Point(1289.5, 790.5), 15);
    connRef269->setDestEndpoint(dstPt269);
    connRef269->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints269(1);
    checkpoints269[0] = Point(1289.5, 759.5);
    connRef269->setRoutingCheckpoints(checkpoints269);

    ConnRef *connRef270 = new ConnRef(router, 270);
    ConnEnd srcPt270(Point(615.5, 487), 8);
    connRef270->setSourceEndpoint(srcPt270);
    ConnEnd dstPt270(Point(1289.5, 466), 15);
    connRef270->setDestEndpoint(dstPt270);
    connRef270->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints270(1);
    checkpoints270[0] = Point(638.5, 466);
    connRef270->setRoutingCheckpoints(checkpoints270);

    ConnRef *connRef271 = new ConnRef(router, 271);
    ConnEnd srcPt271(Point(937.5, 1765.17), 15);
    connRef271->setSourceEndpoint(srcPt271);
    ConnEnd dstPt271(Point(1134.5, 1879.54), 8);
    connRef271->setDestEndpoint(dstPt271);
    connRef271->setRoutingType((ConnType)2);

    ConnRef *connRef272 = new ConnRef(router, 272);
    ConnEnd srcPt272(Point(937.5, 1765.17), 15);
    connRef272->setSourceEndpoint(srcPt272);
    ConnEnd dstPt272(Point(1119.5, 1783.17), 1);
    connRef272->setDestEndpoint(dstPt272);
    connRef272->setRoutingType((ConnType)2);

    ConnRef *connRef273 = new ConnRef(router, 273);
    ConnEnd srcPt273(Point(1347.5, 871.5), 4);
    connRef273->setSourceEndpoint(srcPt273);
    ConnEnd dstPt273(Point(1289.5, 871.5), 8);
    connRef273->setDestEndpoint(dstPt273);
    connRef273->setRoutingType((ConnType)2);

    ConnRef *connRef274 = new ConnRef(router, 274);
    ConnEnd srcPt274(Point(631.5, 2008.54), 2);
    connRef274->setSourceEndpoint(srcPt274);
    ConnEnd dstPt274(Point(631.5, 2058.54), 1);
    connRef274->setDestEndpoint(dstPt274);
    connRef274->setRoutingType((ConnType)2);

    ConnRef *connRef275 = new ConnRef(router, 275);
    ConnEnd srcPt275(Point(330.5, 1930.17), 4);
    connRef275->setSourceEndpoint(srcPt275);
    ConnEnd dstPt275(Point(272.5, 1930.17), 8);
    connRef275->setDestEndpoint(dstPt275);
    connRef275->setRoutingType((ConnType)2);

    ConnRef *connRef276 = new ConnRef(router, 276);
    ConnEnd srcPt276(Point(631.5, 2150.54), 1);
    connRef276->setSourceEndpoint(srcPt276);
    ConnEnd dstPt276(Point(631.5, 2100.54), 2);
    connRef276->setDestEndpoint(dstPt276);
    connRef276->setRoutingType((ConnType)2);

    ConnRef *connRef277 = new ConnRef(router, 277);
    ConnEnd srcPt277(Point(995.5, 2017.54), 4);
    connRef277->setSourceEndpoint(srcPt277);
    ConnEnd dstPt277(Point(937.5, 2017.54), 8);
    connRef277->setDestEndpoint(dstPt277);
    connRef277->setRoutingType((ConnType)2);

    ConnRef *connRef278 = new ConnRef(router, 278);
    ConnEnd srcPt278(Point(1239.5, 1933.54), 2);
    connRef278->setSourceEndpoint(srcPt278);
    ConnEnd dstPt278(Point(1239.5, 1983.54), 1);
    connRef278->setDestEndpoint(dstPt278);
    connRef278->setRoutingType((ConnType)2);

    ConnRef *connRef279 = new ConnRef(router, 279);
    ConnEnd srcPt279(Point(1239.5, 2008.54), 2);
    connRef279->setSourceEndpoint(srcPt279);
    ConnEnd dstPt279(Point(1239.5, 2058.54), 1);
    connRef279->setDestEndpoint(dstPt279);
    connRef279->setRoutingType((ConnType)2);

    ConnRef *connRef280 = new ConnRef(router, 280);
    ConnEnd srcPt280(Point(1119.5, 1929.54), 1);
    connRef280->setSourceEndpoint(srcPt280);
    ConnEnd dstPt280(Point(1119.5, 1879.54), 2);
    connRef280->setDestEndpoint(dstPt280);
    connRef280->setRoutingType((ConnType)2);

    ConnRef *connRef281 = new ConnRef(router, 281);
    ConnEnd srcPt281(Point(1177.5, 1798.17), 4);
    connRef281->setSourceEndpoint(srcPt281);
    ConnEnd dstPt281(Point(1119.5, 1798.17), 8);
    connRef281->setDestEndpoint(dstPt281);
    connRef281->setRoutingType((ConnType)2);

    ConnRef *connRef282 = new ConnRef(router, 282);
    ConnEnd srcPt282(Point(813.5, 1819.17), 2);
    connRef282->setSourceEndpoint(srcPt282);
    ConnEnd dstPt282(Point(813.5, 1869.17), 1);
    connRef282->setDestEndpoint(dstPt282);
    connRef282->setRoutingType((ConnType)2);

    ConnRef *connRef283 = new ConnRef(router, 283);
    ConnEnd srcPt283(Point(406.5, 1819.17), 2);
    connRef283->setSourceEndpoint(srcPt283);
    ConnEnd dstPt283(Point(406.5, 1869.17), 1);
    connRef283->setDestEndpoint(dstPt283);
    connRef283->setRoutingType((ConnType)2);

    ConnRef *connRef284 = new ConnRef(router, 284);
    ConnEnd srcPt284(Point(1456.5, 1819.17), 2);
    connRef284->setSourceEndpoint(srcPt284);
    ConnEnd dstPt284(Point(1456.5, 1869.17), 1);
    connRef284->setDestEndpoint(dstPt284);
    connRef284->setRoutingType((ConnType)2);

    ConnRef *connRef285 = new ConnRef(router, 285);
    ConnEnd srcPt285(Point(1347.5, 1665.17), 4);
    connRef285->setSourceEndpoint(srcPt285);
    ConnEnd dstPt285(Point(1289.5, 1665.17), 8);
    connRef285->setDestEndpoint(dstPt285);
    connRef285->setRoutingType((ConnType)2);

    ConnRef *connRef286 = new ConnRef(router, 286);
    ConnEnd srcPt286(Point(1347.5, 1541.17), 4);
    connRef286->setSourceEndpoint(srcPt286);
    ConnEnd dstPt286(Point(1289.5, 1541.17), 8);
    connRef286->setDestEndpoint(dstPt286);
    connRef286->setRoutingType((ConnType)2);

    ConnRef *connRef287 = new ConnRef(router, 287);
    ConnEnd srcPt287(Point(1347.5, 1417.17), 4);
    connRef287->setSourceEndpoint(srcPt287);
    ConnEnd dstPt287(Point(1289.5, 1417.17), 8);
    connRef287->setDestEndpoint(dstPt287);
    connRef287->setRoutingType((ConnType)2);

    ConnRef *connRef288 = new ConnRef(router, 288);
    ConnEnd srcPt288(Point(1210.5, 1027.17), 8);
    connRef288->setSourceEndpoint(srcPt288);
    ConnEnd dstPt288(Point(1268.5, 1027.17), 4);
    connRef288->setDestEndpoint(dstPt288);
    connRef288->setRoutingType((ConnType)2);

    ConnRef *connRef289 = new ConnRef(router, 289);
    ConnEnd srcPt289(Point(1088.5, 916.167), 2);
    connRef289->setSourceEndpoint(srcPt289);
    ConnEnd dstPt289(Point(1088.5, 966.167), 1);
    connRef289->setDestEndpoint(dstPt289);
    connRef289->setRoutingType((ConnType)2);

    ConnRef *connRef290 = new ConnRef(router, 290);
    ConnEnd srcPt290(Point(1210.5, 729.5), 8);
    connRef290->setSourceEndpoint(srcPt290);
    ConnEnd dstPt290(Point(1268.5, 729.5), 4);
    connRef290->setDestEndpoint(dstPt290);
    connRef290->setRoutingType((ConnType)2);

    ConnRef *connRef291 = new ConnRef(router, 291);
    ConnEnd srcPt291(Point(1459.5, 729.5), 4);
    connRef291->setSourceEndpoint(srcPt291);
    ConnEnd dstPt291(Point(1401.5, 729.5), 8);
    connRef291->setDestEndpoint(dstPt291);
    connRef291->setRoutingType((ConnType)2);

    ConnRef *connRef292 = new ConnRef(router, 292);
    ConnEnd srcPt292(Point(1425.5, 382.5), 2);
    connRef292->setSourceEndpoint(srcPt292);
    ConnEnd dstPt292(Point(1425.5, 432.5), 1);
    connRef292->setDestEndpoint(dstPt292);
    connRef292->setRoutingType((ConnType)2);

    ConnRef *connRef293 = new ConnRef(router, 293);
    ConnEnd srcPt293(Point(1347.5, 551.25), 4);
    connRef293->setSourceEndpoint(srcPt293);
    ConnEnd dstPt293(Point(1289.5, 551.25), 8);
    connRef293->setDestEndpoint(dstPt293);
    connRef293->setRoutingType((ConnType)2);

    ConnRef *connRef294 = new ConnRef(router, 294);
    ConnEnd srcPt294(Point(1347.5, 360), 4);
    connRef294->setSourceEndpoint(srcPt294);
    ConnEnd dstPt294(Point(1289.5, 360), 8);
    connRef294->setDestEndpoint(dstPt294);
    connRef294->setRoutingType((ConnType)2);

    ConnRef *connRef295 = new ConnRef(router, 295);
    ConnEnd srcPt295(Point(1026.5, 588.75), 2);
    connRef295->setSourceEndpoint(srcPt295);
    ConnEnd dstPt295(Point(1026.5, 638.75), 1);
    connRef295->setDestEndpoint(dstPt295);
    connRef295->setRoutingType((ConnType)2);

    ConnRef *connRef296 = new ConnRef(router, 296);
    ConnEnd srcPt296(Point(507.5, 492.75), 2);
    connRef296->setSourceEndpoint(srcPt296);
    ConnEnd dstPt296(Point(507.5, 542.75), 1);
    connRef296->setDestEndpoint(dstPt296);
    connRef296->setRoutingType((ConnType)2);

    ConnRef *connRef297 = new ConnRef(router, 297);
    ConnEnd srcPt297(Point(507.5, 567.75), 2);
    connRef297->setSourceEndpoint(srcPt297);
    ConnEnd dstPt297(Point(507.5, 617.75), 1);
    connRef297->setDestEndpoint(dstPt297);
    connRef297->setRoutingType((ConnType)2);

    ConnRef *connRef298 = new ConnRef(router, 298);
    ConnEnd srcPt298(Point(906.5, 501.25), 2);
    connRef298->setSourceEndpoint(srcPt298);
    ConnEnd dstPt298(Point(906.5, 551.25), 1);
    connRef298->setDestEndpoint(dstPt298);
    connRef298->setRoutingType((ConnType)2);

    ConnRef *connRef299 = new ConnRef(router, 299);
    ConnEnd srcPt299(Point(693.5, 551.25), 8);
    connRef299->setSourceEndpoint(srcPt299);
    ConnEnd dstPt299(Point(751.5, 551.25), 4);
    connRef299->setDestEndpoint(dstPt299);
    connRef299->setRoutingType((ConnType)2);

    ConnRef *connRef300 = new ConnRef(router, 300);
    ConnEnd srcPt300(Point(344.5, 551.25), 4);
    connRef300->setSourceEndpoint(srcPt300);
    ConnEnd dstPt300(Point(286.5, 551.25), 8);
    connRef300->setDestEndpoint(dstPt300);
    connRef300->setRoutingType((ConnType)2);

    ConnRef *connRef301 = new ConnRef(router, 301);
    ConnEnd srcPt301(Point(600.5, 245), 2);
    connRef301->setSourceEndpoint(srcPt301);
    ConnEnd dstPt301(Point(600.5, 295), 1);
    connRef301->setDestEndpoint(dstPt301);
    connRef301->setRoutingType((ConnType)2);

    ConnRef *connRef302 = new ConnRef(router, 302);
    ConnEnd srcPt302(Point(600.5, 320), 2);
    connRef302->setSourceEndpoint(srcPt302);
    ConnEnd dstPt302(Point(600.5, 370), 1);
    connRef302->setDestEndpoint(dstPt302);
    connRef302->setRoutingType((ConnType)2);

    ConnRef *connRef303 = new ConnRef(router, 303);
    ConnEnd srcPt303(Point(600.5, 612), 1);
    connRef303->setSourceEndpoint(srcPt303);
    ConnEnd dstPt303(Point(600.5, 562), 2);
    connRef303->setDestEndpoint(dstPt303);
    connRef303->setRoutingType((ConnType)2);

    ConnRef *connRef304 = new ConnRef(router, 304);
    ConnEnd srcPt304(Point(875.5, 291), 2);
    connRef304->setSourceEndpoint(srcPt304);
    ConnEnd dstPt304(Point(875.5, 341), 1);
    connRef304->setDestEndpoint(dstPt304);
    connRef304->setRoutingType((ConnType)2);

    ConnRef *connRef305 = new ConnRef(router, 305);
    ConnEnd srcPt305(Point(875.5, 189), 2);
    connRef305->setSourceEndpoint(srcPt305);
    ConnEnd dstPt305(Point(875.5, 239), 1);
    connRef305->setDestEndpoint(dstPt305);
    connRef305->setRoutingType((ConnType)2);

    ConnRef *connRef306 = new ConnRef(router, 306);
    ConnEnd srcPt306(Point(538.5, 1962.21), 2);
    connRef306->setSourceEndpoint(srcPt306);
    ConnEnd dstPt306(Point(538.5, 2012.21), 1);
    connRef306->setDestEndpoint(dstPt306);
    connRef306->setRoutingType((ConnType)2);

    ConnRef *connRef307 = new ConnRef(router, 307);
    ConnEnd srcPt307(Point(1347.5, 1263.17), 4);
    connRef307->setSourceEndpoint(srcPt307);
    ConnEnd dstPt307(Point(1289.5, 1263.17), 8);
    connRef307->setDestEndpoint(dstPt307);
    connRef307->setRoutingType((ConnType)2);

    ConnRef *connRef308 = new ConnRef(router, 308);
    ConnEnd srcPt308(Point(600.5, 395), 2);
    connRef308->setSourceEndpoint(srcPt308);
    ConnEnd dstPt308(Point(600.5, 445), 1);
    connRef308->setDestEndpoint(dstPt308);
    connRef308->setRoutingType((ConnType)2);

    ConnRef *connRef309 = new ConnRef(router, 309);
    ConnEnd srcPt309(Point(507.5, 709.75), 1);
    connRef309->setSourceEndpoint(srcPt309);
    ConnEnd dstPt309(Point(507.5, 659.75), 2);
    connRef309->setDestEndpoint(dstPt309);
    connRef309->setRoutingType((ConnType)2);

    ConnRef *connRef310 = new ConnRef(router, 310);
    ConnEnd srcPt310(Point(286.5, 700.75), 2);
    connRef310->setSourceEndpoint(srcPt310);
    ConnEnd dstPt310(Point(286.5, 750.75), 1);
    connRef310->setDestEndpoint(dstPt310);
    connRef310->setRoutingType((ConnType)2);

    ConnRef *connRef311 = new ConnRef(router, 311);
    ConnEnd srcPt311(Point(286.5, 802.75), 2);
    connRef311->setSourceEndpoint(srcPt311);
    ConnEnd dstPt311(Point(286.5, 852.75), 1);
    connRef311->setDestEndpoint(dstPt311);
    connRef311->setRoutingType((ConnType)2);

    ConnRef *connRef312 = new ConnRef(router, 312);
    ConnEnd srcPt312(Point(662.5, 700.75), 2);
    connRef312->setSourceEndpoint(srcPt312);
    ConnEnd dstPt312(Point(662.5, 750.75), 1);
    connRef312->setDestEndpoint(dstPt312);
    connRef312->setRoutingType((ConnType)2);

    ConnRef *connRef313 = new ConnRef(router, 313);
    ConnEnd srcPt313(Point(662.5, 802.75), 2);
    connRef313->setSourceEndpoint(srcPt313);
    ConnEnd dstPt313(Point(662.5, 852.75), 1);
    connRef313->setDestEndpoint(dstPt313);
    connRef313->setRoutingType((ConnType)2);

    ConnRef *connRef314 = new ConnRef(router, 314);
    ConnEnd srcPt314(Point(375.5, 1532.17), 2);
    connRef314->setSourceEndpoint(srcPt314);
    ConnEnd dstPt314(Point(375.5, 1582.17), 1);
    connRef314->setDestEndpoint(dstPt314);
    connRef314->setRoutingType((ConnType)2);

    ConnRef *connRef315 = new ConnRef(router, 315);
    ConnEnd srcPt315(Point(1239.5, 2150.54), 1);
    connRef315->setSourceEndpoint(srcPt315);
    ConnEnd dstPt315(Point(1239.5, 2100.54), 2);
    connRef315->setDestEndpoint(dstPt315);
    connRef315->setRoutingType((ConnType)2);

    ConnRef *connRef316 = new ConnRef(router, 316);
    ConnEnd srcPt316(Point(375.5, 1674.17), 1);
    connRef316->setSourceEndpoint(srcPt316);
    ConnEnd dstPt316(Point(375.5, 1624.17), 2);
    connRef316->setDestEndpoint(dstPt316);
    connRef316->setRoutingType((ConnType)2);

    ConnRef *connRef317 = new ConnRef(router, 317);
    ConnEnd srcPt317(Point(1368.5, 1027.17), 4);
    connRef317->setSourceEndpoint(srcPt317);
    ConnEnd dstPt317(Point(1310.5, 1027.17), 8);
    connRef317->setDestEndpoint(dstPt317);
    connRef317->setRoutingType((ConnType)2);

    ConnRef *connRef318 = new ConnRef(router, 318);
    ConnEnd srcPt318(Point(1338.5, 1201.17), 8);
    connRef318->setSourceEndpoint(srcPt318);
    ConnEnd dstPt318(Point(1396.5, 1201.17), 4);
    connRef318->setDestEndpoint(dstPt318);
    connRef318->setRoutingType((ConnType)2);

    ConnRef *connRef319 = new ConnRef(router, 319);
    ConnEnd srcPt319(Point(1496.5, 1201.17), 4);
    connRef319->setSourceEndpoint(srcPt319);
    ConnEnd dstPt319(Point(1438.5, 1201.17), 8);
    connRef319->setDestEndpoint(dstPt319);
    connRef319->setRoutingType((ConnType)2);

    ConnRef *connRef320 = new ConnRef(router, 320);
    ConnEnd srcPt320(Point(662.5, 1690.17), 2);
    connRef320->setSourceEndpoint(srcPt320);
    ConnEnd dstPt320(Point(662.5, 1740.17), 1);
    connRef320->setDestEndpoint(dstPt320);
    connRef320->setRoutingType((ConnType)2);

    ConnRef *connRef321 = new ConnRef(router, 321);
    ConnEnd srcPt321(Point(1587.5, 1201.17), 4);
    connRef321->setSourceEndpoint(srcPt321);
    ConnEnd dstPt321(Point(1529.5, 1201.17), 8);
    connRef321->setDestEndpoint(dstPt321);
    connRef321->setRoutingType((ConnType)2);

    ConnRef *connRef322 = new ConnRef(router, 322);
    ConnEnd srcPt322(Point(1208.5, 1293.17), 2);
    connRef322->setSourceEndpoint(srcPt322);
    ConnEnd dstPt322(Point(1208.5, 1343.17), 1);
    connRef322->setDestEndpoint(dstPt322);
    connRef322->setRoutingType((ConnType)2);

    ConnRef *connRef323 = new ConnRef(router, 323);
    ConnEnd srcPt323(Point(1208.5, 1089.17), 2);
    connRef323->setSourceEndpoint(srcPt323);
    ConnEnd dstPt323(Point(1208.5, 1139.17), 1);
    connRef323->setDestEndpoint(dstPt323);
    connRef323->setRoutingType((ConnType)2);

    ConnRef *connRef324 = new ConnRef(router, 324);
    ConnEnd srcPt324(Point(172.5, 1930.17), 8);
    connRef324->setSourceEndpoint(srcPt324);
    ConnEnd dstPt324(Point(230.5, 1930.17), 4);
    connRef324->setDestEndpoint(dstPt324);
    connRef324->setRoutingType((ConnType)2);

    ConnRef *connRef325 = new ConnRef(router, 325);
    ConnEnd srcPt325(Point(1368.5, 729.5), 4);
    connRef325->setSourceEndpoint(srcPt325);
    ConnEnd dstPt325(Point(1310.5, 729.5), 8);
    connRef325->setDestEndpoint(dstPt325);
    connRef325->setRoutingType((ConnType)2);

    ConnRef *connRef326 = new ConnRef(router, 326);
    ConnEnd srcPt326(Point(600.5, 537), 1);
    connRef326->setSourceEndpoint(srcPt326);
    ConnEnd dstPt326(Point(600.5, 487), 2);
    connRef326->setDestEndpoint(dstPt326);
    connRef326->setRoutingType((ConnType)2);

    ConnRef *connRef327 = new ConnRef(router, 327);
    ConnEnd srcPt327(Point(507.5, 784.75), 1);
    connRef327->setSourceEndpoint(srcPt327);
    ConnEnd dstPt327(Point(507.5, 734.75), 2);
    connRef327->setDestEndpoint(dstPt327);
    connRef327->setRoutingType((ConnType)2);

    ConnRef *connRef328 = new ConnRef(router, 328);
    ConnEnd srcPt328(Point(1239.5, 2225.54), 1);
    connRef328->setSourceEndpoint(srcPt328);
    ConnEnd dstPt328(Point(1239.5, 2175.54), 2);
    connRef328->setDestEndpoint(dstPt328);
    connRef328->setRoutingType((ConnType)2);

    ConnRef *connRef329 = new ConnRef(router, 329);
    ConnEnd srcPt329(Point(251.5, 1869.17), 15);
    connRef329->setSourceEndpoint(srcPt329);
    ConnEnd dstPt329(Point(272.5, 1915.17), 1);
    connRef329->setDestEndpoint(dstPt329);
    connRef329->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints329(1);
    checkpoints329[0] = Point(251.5, 1900.17);
    connRef329->setRoutingCheckpoints(checkpoints329);

    ConnRef *connRef330 = new ConnRef(router, 330);
    ConnEnd srcPt330(Point(1289.5, 1869.17), 15);
    connRef330->setSourceEndpoint(srcPt330);
    ConnEnd dstPt330(Point(1134.5, 1879.54), 8);
    connRef330->setDestEndpoint(dstPt330);
    connRef330->setRoutingType((ConnType)2);

    ConnRef *connRef331 = new ConnRef(router, 331);
    ConnEnd srcPt331(Point(1289.5, 1869.17), 15);
    connRef331->setSourceEndpoint(srcPt331);
    ConnEnd dstPt331(Point(1119.5, 1783.17), 1);
    connRef331->setDestEndpoint(dstPt331);
    connRef331->setRoutingType((ConnType)2);

    ConnRef *connRef332 = new ConnRef(router, 332);
    ConnEnd srcPt332(Point(782.5, 2012.21), 15);
    connRef332->setSourceEndpoint(srcPt332);
    ConnEnd dstPt332(Point(828.5, 1869.17), 8);
    connRef332->setDestEndpoint(dstPt332);
    connRef332->setRoutingType((ConnType)2);

    ConnRef *connRef333 = new ConnRef(router, 333);
    ConnEnd srcPt333(Point(937.5, 1869.17), 15);
    connRef333->setSourceEndpoint(srcPt333);
    ConnEnd dstPt333(Point(828.5, 1869.17), 8);
    connRef333->setDestEndpoint(dstPt333);
    connRef333->setRoutingType((ConnType)2);

    ConnRef *connRef334 = new ConnRef(router, 334);
    ConnEnd srcPt334(Point(1289.5, 1603.17), 15);
    connRef334->setSourceEndpoint(srcPt334);
    ConnEnd dstPt334(Point(1289.5, 1650.17), 1);
    connRef334->setDestEndpoint(dstPt334);
    connRef334->setRoutingType((ConnType)2);

    ConnRef *connRef335 = new ConnRef(router, 335);
    ConnEnd srcPt335(Point(1289.5, 638.75), 15);
    connRef335->setSourceEndpoint(srcPt335);
    ConnEnd dstPt335(Point(1268.5, 714.5), 1);
    connRef335->setDestEndpoint(dstPt335);
    connRef335->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints335(1);
    checkpoints335[0] = Point(1289.5, 699.5);
    connRef335->setRoutingCheckpoints(checkpoints335);

    ConnRef *connRef336 = new ConnRef(router, 336);
    ConnEnd srcPt336(Point(286.5, 638.75), 15);
    connRef336->setSourceEndpoint(srcPt336);
    ConnEnd dstPt336(Point(492.5, 542.75), 4);
    connRef336->setDestEndpoint(dstPt336);
    connRef336->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints336(1);
    checkpoints336[0] = Point(469.5, 638.75);
    connRef336->setRoutingCheckpoints(checkpoints336);

    ConnRef *connRef337 = new ConnRef(router, 337);
    ConnEnd srcPt337(Point(286.5, 638.75), 15);
    connRef337->setSourceEndpoint(srcPt337);
    ConnEnd dstPt337(Point(492.5, 617.75), 4);
    connRef337->setDestEndpoint(dstPt337);
    connRef337->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints337(1);
    checkpoints337[0] = Point(469.5, 638.75);
    connRef337->setRoutingCheckpoints(checkpoints337);

    ConnRef *connRef338 = new ConnRef(router, 338);
    ConnEnd srcPt338(Point(286.5, 466), 15);
    connRef338->setSourceEndpoint(srcPt338);
    ConnEnd dstPt338(Point(585.5, 295), 4);
    connRef338->setDestEndpoint(dstPt338);
    connRef338->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints338(1);
    checkpoints338[0] = Point(562.5, 466);
    connRef338->setRoutingCheckpoints(checkpoints338);

    ConnRef *connRef339 = new ConnRef(router, 339);
    ConnEnd srcPt339(Point(286.5, 466), 15);
    connRef339->setSourceEndpoint(srcPt339);
    ConnEnd dstPt339(Point(585.5, 370), 4);
    connRef339->setDestEndpoint(dstPt339);
    connRef339->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints339(1);
    checkpoints339[0] = Point(562.5, 466);
    connRef339->setRoutingCheckpoints(checkpoints339);

    ConnRef *connRef340 = new ConnRef(router, 340);
    ConnEnd srcPt340(Point(1289.5, 466), 15);
    connRef340->setSourceEndpoint(srcPt340);
    ConnEnd dstPt340(Point(615.5, 562), 8);
    connRef340->setDestEndpoint(dstPt340);
    connRef340->setRoutingType((ConnType)2);
    std::vector<Point> checkpoints340(1);
    checkpoints340[0] = Point(638.5, 466);
    connRef340->setRoutingCheckpoints(checkpoints340);

    ConnRef *connRef341 = new ConnRef(router, 341);
    ConnEnd srcPt341(Point(1289.5, 280), 15);
    connRef341->setSourceEndpoint(srcPt341);
    ConnEnd dstPt341(Point(890.5, 341), 8);
    connRef341->setDestEndpoint(dstPt341);
    connRef341->setRoutingType((ConnType)2);

    ConnRef *connRef342 = new ConnRef(router, 342);
    ConnEnd srcPt342(Point(1289.5, 280), 15);
    connRef342->setSourceEndpoint(srcPt342);
    ConnEnd dstPt342(Point(890.5, 239), 8);
    connRef342->setDestEndpoint(dstPt342);
    connRef342->setRoutingType((ConnType)2);

    ConnRef *connRef343 = new ConnRef(router, 343);
    ConnEnd srcPt343(Point(1289.5, 466), 15);
    connRef343->setSourceEndpoint(srcPt343);
    ConnEnd dstPt343(Point(1515, 661), 4);
    connRef343->setDestEndpoint(dstPt343);
    connRef343->setRoutingType((ConnType)2);

    ConnRef *connRef344 = new ConnRef(router, 344);
    ConnEnd srcPt344(Point(1289.5, 966.167), 15);
    connRef344->setSourceEndpoint(srcPt344);
    ConnEnd dstPt344(Point(1515, 701), 4);
    connRef344->setDestEndpoint(dstPt344);
    connRef344->setRoutingType((ConnType)2);

    ConnRef *connRef345 = new ConnRef(router, 345);
    ConnEnd srcPt345(Point(1289.5, 1479.17), 15);
    connRef345->setSourceEndpoint(srcPt345);
    ConnEnd dstPt345(Point(1601, 864.5), 2);
    connRef345->setDestEndpoint(dstPt345);
    connRef345->setRoutingType((ConnType)2);

    ConnRef *connRef346 = new ConnRef(router, 346);
    ConnEnd srcPt346(Point(1289.5, 1869.17), 15);
    connRef346->setSourceEndpoint(srcPt346);
    ConnEnd dstPt346(Point(1641, 864.5), 2);
    connRef346->setDestEndpoint(dstPt346);
    connRef346->setRoutingType((ConnType)2);

    ConnRef *connRef347 = new ConnRef(router, 347);
    ConnEnd srcPt347(Point(1289.5, 280), 15);
    connRef347->setSourceEndpoint(srcPt347);
    ConnEnd dstPt347(Point(1350, 214.5), 2);
    connRef347->setDestEndpoint(dstPt347);
    connRef347->setRoutingType((ConnType)2);

    ConnRef *connRef348 = new ConnRef(router, 348);
    ConnEnd srcPt348(Point(251.5, 1603.17), 15);
    connRef348->setSourceEndpoint(srcPt348);
    ConnEnd dstPt348(Point(247.5, 1725.67), 8);
    connRef348->setDestEndpoint(dstPt348);
    connRef348->setRoutingType((ConnType)2);

    ConnRef *connRef349 = new ConnRef(router, 349);
    ConnEnd srcPt349(Point(251.5, 1869.17), 15);
    connRef349->setSourceEndpoint(srcPt349);
    ConnEnd dstPt349(Point(247.5, 1765.67), 8);
    connRef349->setDestEndpoint(dstPt349);
    connRef349->setRoutingType((ConnType)2);

    ConnRef *connRef350 = new ConnRef(router, 350);
    ConnEnd srcPt350(Point(1289.5, 1355.17), 15);
    connRef350->setSourceEndpoint(srcPt350);
    ConnEnd dstPt350(Point(844.5, 1362.67), 8);
    connRef350->setDestEndpoint(dstPt350);
    connRef350->setRoutingType((ConnType)2);

    ConnRef *connRef351 = new ConnRef(router, 351);
    ConnEnd srcPt351(Point(1530.5, 1869.17), 15);
    connRef351->setSourceEndpoint(srcPt351);
    ConnEnd dstPt351(Point(1536.5, 1948.54), 1);
    connRef351->setDestEndpoint(dstPt351);
    connRef351->setRoutingType((ConnType)2);
#endif

    router->processTransaction();
    router->outputDiagram("output/nudgeCrossing01");

    bool optimisedForConnectorType = false;
    int crossings = router->existsCrossings(optimisedForConnectorType);

    delete router;
    return crossings;
};

