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
    router->setRoutingOption(nudgeOrthogonalSegmentsConnectedToShapes, true);

    Polygon poly57(4);
    poly57.ps[0] = Point(-66.016, 38.6079);
    poly57.ps[1] = Point(-66.016, 58.6079);
    poly57.ps[2] = Point(-86.016, 58.6079);
    poly57.ps[3] = Point(-86.016, 38.6079);
    new ShapeRef(router, poly57, 57);
    
    Polygon poly114(4);
    poly114.ps[0] = Point(886.746, 359.608);
    poly114.ps[1] = Point(886.746, 399.608);
    poly114.ps[2] = Point(830.746, 399.608);
    poly114.ps[3] = Point(830.746, 359.608);
    new ShapeRef(router, poly114, 114);
    
    Polygon poly126(4);
    poly126.ps[0] = Point(510.984, 767.608);
    poly126.ps[1] = Point(510.984, 807.608);
    poly126.ps[2] = Point(454.984, 807.608);
    poly126.ps[3] = Point(454.984, 767.608);
    new ShapeRef(router, poly126, 126);
    
    Polygon poly142(4);
    poly142.ps[0] = Point(149.984, 62.6079);
    poly142.ps[1] = Point(149.984, 102.608);
    poly142.ps[2] = Point(93.984, 102.608);
    poly142.ps[3] = Point(93.984, 62.6079);
    new ShapeRef(router, poly142, 142);
    
    Polygon poly115(4);
    poly115.ps[0] = Point(539.534, 166.608);
    poly115.ps[1] = Point(539.534, 206.608);
    poly115.ps[2] = Point(483.534, 206.608);
    poly115.ps[3] = Point(483.534, 166.608);
    new ShapeRef(router, poly115, 115);
    
    Polygon poly107(4);
    poly107.ps[0] = Point(484.984, 500.608);
    poly107.ps[1] = Point(484.984, 540.608);
    poly107.ps[2] = Point(444.984, 540.608);
    poly107.ps[3] = Point(444.984, 500.608);
    new ShapeRef(router, poly107, 107);
    
    Polygon poly164(4);
    poly164.ps[0] = Point(388.984, 1086.01);
    poly164.ps[1] = Point(388.984, 1126.01);
    poly164.ps[2] = Point(328.984, 1126.01);
    poly164.ps[3] = Point(328.984, 1086.01);
    new ShapeRef(router, poly164, 164);
    
    Polygon poly40(4);
    poly40.ps[0] = Point(131.984, -13.3921);
    poly40.ps[1] = Point(131.984, 6.60786);
    poly40.ps[2] = Point(111.984, 6.60786);
    poly40.ps[3] = Point(111.984, -13.3921);
    new ShapeRef(router, poly40, 40);
    
    Polygon poly161(4);
    poly161.ps[0] = Point(785.5, 24.6079);
    poly161.ps[1] = Point(785.5, 64.6079);
    poly161.ps[2] = Point(725.5, 64.6079);
    poly161.ps[3] = Point(725.5, 24.6079);
    new ShapeRef(router, poly161, 161);
    
    Polygon poly82(4);
    poly82.ps[0] = Point(813.984, 1031.01);
    poly82.ps[1] = Point(813.984, 1071.01);
    poly82.ps[2] = Point(773.984, 1071.01);
    poly82.ps[3] = Point(773.984, 1031.01);
    new ShapeRef(router, poly82, 82);
    
    Polygon poly146(4);
    poly146.ps[0] = Point(530.584, -65.3921);
    poly146.ps[1] = Point(530.584, -25.3921);
    poly146.ps[2] = Point(474.584, -25.3921);
    poly146.ps[3] = Point(474.584, -65.3921);
    new ShapeRef(router, poly146, 146);
    
    Polygon poly151(4);
    poly151.ps[0] = Point(97.984, -229.392);
    poly151.ps[1] = Point(97.984, -189.392);
    poly151.ps[2] = Point(41.984, -189.392);
    poly151.ps[3] = Point(41.984, -229.392);
    new ShapeRef(router, poly151, 151);
    
    Polygon poly23(4);
    poly23.ps[0] = Point(691.984, 725.608);
    poly23.ps[1] = Point(691.984, 745.608);
    poly23.ps[2] = Point(671.984, 745.608);
    poly23.ps[3] = Point(671.984, 725.608);
    new ShapeRef(router, poly23, 23);
    
    Polygon poly121(4);
    poly121.ps[0] = Point(709.984, 1063.01);
    poly121.ps[1] = Point(709.984, 1103.01);
    poly121.ps[2] = Point(653.984, 1103.01);
    poly121.ps[3] = Point(653.984, 1063.01);
    new ShapeRef(router, poly121, 121);
    
    Polygon poly68(4);
    poly68.ps[0] = Point(460.584, 164.608);
    poly68.ps[1] = Point(460.584, 204.608);
    poly68.ps[2] = Point(420.584, 204.608);
    poly68.ps[3] = Point(420.584, 164.608);
    new ShapeRef(router, poly68, 68);
    
    Polygon poly53(4);
    poly53.ps[0] = Point(450.584, -424.392);
    poly53.ps[1] = Point(450.584, -404.392);
    poly53.ps[2] = Point(430.584, -404.392);
    poly53.ps[3] = Point(430.584, -424.392);
    new ShapeRef(router, poly53, 53);
    
    Polygon poly29(4);
    poly29.ps[0] = Point(808.746, 229.608);
    poly29.ps[1] = Point(808.746, 249.608);
    poly29.ps[2] = Point(788.746, 249.608);
    poly29.ps[3] = Point(788.746, 229.608);
    new ShapeRef(router, poly29, 29);
    
    Polygon poly88(4);
    poly88.ps[0] = Point(1085.98, 883.608);
    poly88.ps[1] = Point(1085.98, 923.608);
    poly88.ps[2] = Point(1045.98, 923.608);
    poly88.ps[3] = Point(1045.98, 883.608);
    new ShapeRef(router, poly88, 88);
    
    Polygon poly141(4);
    poly141.ps[0] = Point(390.984, 62.6079);
    poly141.ps[1] = Point(390.984, 102.608);
    poly141.ps[2] = Point(334.984, 102.608);
    poly141.ps[3] = Point(334.984, 62.6079);
    new ShapeRef(router, poly141, 141);
    
    Polygon poly118(4);
    poly118.ps[0] = Point(1155.98, 739.608);
    poly118.ps[1] = Point(1155.98, 779.608);
    poly118.ps[2] = Point(1099.98, 779.608);
    poly118.ps[3] = Point(1099.98, 739.608);
    new ShapeRef(router, poly118, 118);
    
    Polygon poly31(4);
    poly31.ps[0] = Point(1.98401, 363.608);
    poly31.ps[1] = Point(1.98401, 383.608);
    poly31.ps[2] = Point(-18.016, 383.608);
    poly31.ps[3] = Point(-18.016, 363.608);
    new ShapeRef(router, poly31, 31);
    
    Polygon poly137(4);
    poly137.ps[0] = Point(408.584, 166.608);
    poly137.ps[1] = Point(408.584, 206.608);
    poly137.ps[2] = Point(352.584, 206.608);
    poly137.ps[3] = Point(352.584, 166.608);
    new ShapeRef(router, poly137, 137);
    
    Polygon poly159(4);
    poly159.ps[0] = Point(622.984, 330.5);
    poly159.ps[1] = Point(622.984, 390.5);
    poly159.ps[2] = Point(582.984, 390.5);
    poly159.ps[3] = Point(582.984, 330.5);
    new ShapeRef(router, poly159, 159);
    
    Polygon poly169(4);
    poly169.ps[0] = Point(1089.98, 243.608);
    poly169.ps[1] = Point(1089.98, 303.608);
    poly169.ps[2] = Point(1049.98, 303.608);
    poly169.ps[3] = Point(1049.98, 243.608);
    new ShapeRef(router, poly169, 169);
    
    Polygon poly13(4);
    poly13.ps[0] = Point(763.984, 1145.01);
    poly13.ps[1] = Point(763.984, 1165.01);
    poly13.ps[2] = Point(743.984, 1165.01);
    poly13.ps[3] = Point(743.984, 1145.01);
    new ShapeRef(router, poly13, 13);
    
    Polygon poly33(4);
    poly33.ps[0] = Point(327.084, 309.608);
    poly33.ps[1] = Point(327.084, 329.608);
    poly33.ps[2] = Point(307.084, 329.608);
    poly33.ps[3] = Point(307.084, 309.608);
    new ShapeRef(router, poly33, 33);
    
    Polygon poly60(4);
    poly60.ps[0] = Point(11.984, -23.3921);
    poly60.ps[1] = Point(11.984, 16.6079);
    poly60.ps[2] = Point(-28.016, 16.6079);
    poly60.ps[3] = Point(-28.016, -23.3921);
    new ShapeRef(router, poly60, 60);
    
    Polygon poly145(4);
    poly145.ps[0] = Point(149.984, -73.3921);
    poly145.ps[1] = Point(149.984, -33.3921);
    poly145.ps[2] = Point(93.984, -33.3921);
    poly145.ps[3] = Point(93.984, -73.3921);
    new ShapeRef(router, poly145, 145);
    
    Polygon poly134(4);
    poly134.ps[0] = Point(888.746, 219.608);
    poly134.ps[1] = Point(888.746, 259.608);
    poly134.ps[2] = Point(832.746, 259.608);
    poly134.ps[3] = Point(832.746, 219.608);
    new ShapeRef(router, poly134, 134);
    
    Polygon poly160(4);
    poly160.ps[0] = Point(785.5, -27.3921);
    poly160.ps[1] = Point(785.5, 12.6079);
    poly160.ps[2] = Point(725.5, 12.6079);
    poly160.ps[3] = Point(725.5, -27.3921);
    new ShapeRef(router, poly160, 160);
    
    Polygon poly117(4);
    poly117.ps[0] = Point(646.484, 1203.01);
    poly117.ps[1] = Point(646.484, 1243.01);
    poly117.ps[2] = Point(590.484, 1243.01);
    poly117.ps[3] = Point(590.484, 1203.01);
    new ShapeRef(router, poly117, 117);
    
    Polygon poly170(4);
    poly170.ps[0] = Point(322.984, 536.5);
    poly170.ps[1] = Point(322.984, 596.5);
    poly170.ps[2] = Point(282.984, 596.5);
    poly170.ps[3] = Point(282.984, 536.5);
    new ShapeRef(router, poly170, 170);
    
    Polygon poly36(4);
    poly36.ps[0] = Point(372.984, 122.608);
    poly36.ps[1] = Point(372.984, 142.608);
    poly36.ps[2] = Point(352.984, 142.608);
    poly36.ps[3] = Point(352.984, 122.608);
    new ShapeRef(router, poly36, 36);
    
    Polygon poly19(4);
    poly19.ps[0] = Point(803.984, 869.608);
    poly19.ps[1] = Point(803.984, 889.608);
    poly19.ps[2] = Point(783.984, 889.608);
    poly19.ps[3] = Point(783.984, 869.608);
    new ShapeRef(router, poly19, 19);
    
    Polygon poly116(4);
    poly116.ps[0] = Point(456.984, 1213.01);
    poly116.ps[1] = Point(456.984, 1253.01);
    poly116.ps[2] = Point(400.984, 1253.01);
    poly116.ps[3] = Point(400.984, 1213.01);
    new ShapeRef(router, poly116, 116);
    
    Polygon poly52(4);
    poly52.ps[0] = Point(450.584, -271.392);
    poly52.ps[1] = Point(450.584, -251.392);
    poly52.ps[2] = Point(430.584, -251.392);
    poly52.ps[3] = Point(430.584, -271.392);
    new ShapeRef(router, poly52, 52);
    
    Polygon poly99(4);
    poly99.ps[0] = Point(307.984, -334.392);
    poly99.ps[1] = Point(307.984, -294.392);
    poly99.ps[2] = Point(267.984, -294.392);
    poly99.ps[3] = Point(267.984, -334.392);
    new ShapeRef(router, poly99, 99);
    
    Polygon poly154(4);
    poly154.ps[0] = Point(570.984, 450.608);
    poly154.ps[1] = Point(570.984, 490.608);
    poly154.ps[2] = Point(514.984, 490.608);
    poly154.ps[3] = Point(514.984, 450.608);
    new ShapeRef(router, poly154, 154);
    
    Polygon poly150(4);
    poly150.ps[0] = Point(377.984, -281.392);
    poly150.ps[1] = Point(377.984, -241.392);
    poly150.ps[2] = Point(321.984, -241.392);
    poly150.ps[3] = Point(321.984, -281.392);
    new ShapeRef(router, poly150, 150);
    
    Polygon poly153(4);
    poly153.ps[0] = Point(767.984, 450.608);
    poly153.ps[1] = Point(767.984, 490.608);
    poly153.ps[2] = Point(711.984, 490.608);
    poly153.ps[3] = Point(711.984, 450.608);
    new ShapeRef(router, poly153, 153);
    
    Polygon poly104(4);
    poly104.ps[0] = Point(965.984, 164.608);
    poly104.ps[1] = Point(965.984, 204.608);
    poly104.ps[2] = Point(925.984, 204.608);
    poly104.ps[3] = Point(925.984, 164.608);
    new ShapeRef(router, poly104, 104);
    
    Polygon poly75(4);
    poly75.ps[0] = Point(442.984, 663.608);
    poly75.ps[1] = Point(442.984, 703.608);
    poly75.ps[2] = Point(402.984, 703.608);
    poly75.ps[3] = Point(402.984, 663.608);
    new ShapeRef(router, poly75, 75);
    
    Polygon poly51(4);
    poly51.ps[0] = Point(450.584, -55.3921);
    poly51.ps[1] = Point(450.584, -35.3921);
    poly51.ps[2] = Point(430.584, -35.3921);
    poly51.ps[3] = Point(430.584, -55.3921);
    new ShapeRef(router, poly51, 51);
    
    Polygon poly112(4);
    poly112.ps[0] = Point(539.534, -229.392);
    poly112.ps[1] = Point(539.534, -189.392);
    poly112.ps[2] = Point(483.534, -189.392);
    poly112.ps[3] = Point(483.534, -229.392);
    new ShapeRef(router, poly112, 112);
    
    Polygon poly61(4);
    poly61.ps[0] = Point(337.084, 425.608);
    poly61.ps[1] = Point(337.084, 465.608);
    poly61.ps[2] = Point(297.084, 465.608);
    poly61.ps[3] = Point(297.084, 425.608);
    new ShapeRef(router, poly61, 61);
    
    Polygon poly158(4);
    poly158.ps[0] = Point(285.084, 425.608);
    poly158.ps[1] = Point(285.084, 465.608);
    poly158.ps[2] = Point(225.084, 465.608);
    poly158.ps[3] = Point(225.084, 425.608);
    new ShapeRef(router, poly158, 158);
    
    Polygon poly162(4);
    poly162.ps[0] = Point(785.5, 76.6079);
    poly162.ps[1] = Point(785.5, 116.608);
    poly162.ps[2] = Point(725.5, 116.608);
    poly162.ps[3] = Point(725.5, 76.6079);
    new ShapeRef(router, poly162, 162);
    
    Polygon poly103(4);
    poly103.ps[0] = Point(965.984, 315.608);
    poly103.ps[1] = Point(965.984, 355.608);
    poly103.ps[2] = Point(925.984, 355.608);
    poly103.ps[3] = Point(925.984, 315.608);
    new ShapeRef(router, poly103, 103);
    
    Polygon poly90(4);
    poly90.ps[0] = Point(813.984, 817.608);
    poly90.ps[1] = Point(813.984, 857.608);
    poly90.ps[2] = Point(773.984, 857.608);
    poly90.ps[3] = Point(773.984, 817.608);
    new ShapeRef(router, poly90, 90);
    
    Polygon poly70(4);
    poly70.ps[0] = Point(340.584, 164.608);
    poly70.ps[1] = Point(340.584, 204.608);
    poly70.ps[2] = Point(300.584, 204.608);
    poly70.ps[3] = Point(300.584, 164.608);
    new ShapeRef(router, poly70, 70);
    
    Polygon poly50(4);
    poly50.ps[0] = Point(886.746, 307.608);
    poly50.ps[1] = Point(886.746, 347.608);
    poly50.ps[2] = Point(830.746, 347.608);
    poly50.ps[3] = Point(830.746, 307.608);
    new ShapeRef(router, poly50, 50);
    
    Polygon poly49(4);
    poly49.ps[0] = Point(1115.98, 1187.01);
    poly49.ps[1] = Point(1115.98, 1247.01);
    poly49.ps[2] = Point(825.984, 1247.01);
    poly49.ps[3] = Point(825.984, 1187.01);
    new ShapeRef(router, poly49, 49);
    
    Polygon poly37(4);
    poly37.ps[0] = Point(131.984, 122.608);
    poly37.ps[1] = Point(131.984, 142.608);
    poly37.ps[2] = Point(111.984, 142.608);
    poly37.ps[3] = Point(111.984, 122.608);
    new ShapeRef(router, poly37, 37);
    
    Polygon poly120(4);
    poly120.ps[0] = Point(843.984, 1135.01);
    poly120.ps[1] = Point(843.984, 1175.01);
    poly120.ps[2] = Point(787.984, 1175.01);
    poly120.ps[3] = Point(787.984, 1135.01);
    new ShapeRef(router, poly120, 120);
    
    Polygon poly28(4);
    poly28.ps[0] = Point(372.984, 673.608);
    poly28.ps[1] = Point(372.984, 693.608);
    poly28.ps[2] = Point(352.984, 693.608);
    poly28.ps[3] = Point(352.984, 673.608);
    new ShapeRef(router, poly28, 28);
    
    Polygon poly136(4);
    poly136.ps[0] = Point(81.984, 353.608);
    poly136.ps[1] = Point(81.984, 393.608);
    poly136.ps[2] = Point(25.984, 393.608);
    poly136.ps[3] = Point(25.984, 353.608);
    new ShapeRef(router, poly136, 136);
    
    Polygon poly12(4);
    poly12.ps[0] = Point(933.984, 939.008);
    poly12.ps[1] = Point(933.984, 959.008);
    poly12.ps[2] = Point(913.984, 959.008);
    poly12.ps[3] = Point(913.984, 939.008);
    new ShapeRef(router, poly12, 12);
    
    Polygon poly131(4);
    poly131.ps[0] = Point(390.984, 715.608);
    poly131.ps[1] = Point(390.984, 755.608);
    poly131.ps[2] = Point(334.984, 755.608);
    poly131.ps[3] = Point(334.984, 715.608);
    new ShapeRef(router, poly131, 131);
    
    Polygon poly78(4);
    poly78.ps[0] = Point(302.984, 1031.01);
    poly78.ps[1] = Point(302.984, 1071.01);
    poly78.ps[2] = Point(262.984, 1071.01);
    poly78.ps[3] = Point(262.984, 1031.01);
    new ShapeRef(router, poly78, 78);
    
    Polygon poly3(4);
    poly3.ps[0] = Point(944.984, 516.108);
    poly3.ps[1] = Point(944.984, 536.108);
    poly3.ps[2] = Point(924.984, 536.108);
    poly3.ps[3] = Point(924.984, 516.108);
    new ShapeRef(router, poly3, 3);
    
    Polygon poly17(4);
    poly17.ps[0] = Point(691.984, 939.008);
    poly17.ps[1] = Point(691.984, 959.008);
    poly17.ps[2] = Point(671.984, 959.008);
    poly17.ps[3] = Point(671.984, 939.008);
    new ShapeRef(router, poly17, 17);
    
    Polygon poly66(4);
    poly66.ps[0] = Point(11.984, 229.608);
    poly66.ps[1] = Point(11.984, 269.608);
    poly66.ps[2] = Point(-28.016, 269.608);
    poly66.ps[3] = Point(-28.016, 229.608);
    new ShapeRef(router, poly66, 66);
    
    Polygon poly15(4);
    poly15.ps[0] = Point(691.984, 1031.01);
    poly15.ps[1] = Point(691.984, 1051.01);
    poly15.ps[2] = Point(671.984, 1051.01);
    poly15.ps[3] = Point(671.984, 1031.01);
    new ShapeRef(router, poly15, 15);
    
    Polygon poly4(4);
    poly4.ps[0] = Point(868.746, 419.608);
    poly4.ps[1] = Point(868.746, 439.608);
    poly4.ps[2] = Point(848.746, 439.608);
    poly4.ps[3] = Point(848.746, 419.608);
    new ShapeRef(router, poly4, 4);
    
    Polygon poly22(4);
    poly22.ps[0] = Point(803.984, 725.608);
    poly22.ps[1] = Point(803.984, 745.608);
    poly22.ps[2] = Point(783.984, 745.608);
    poly22.ps[3] = Point(783.984, 725.608);
    new ShapeRef(router, poly22, 22);
    
    Polygon poly119(4);
    poly119.ps[0] = Point(951.984, 879.008);
    poly119.ps[1] = Point(951.984, 919.008);
    poly119.ps[2] = Point(895.984, 919.008);
    poly119.ps[3] = Point(895.984, 879.008);
    new ShapeRef(router, poly119, 119);
    
    Polygon poly130(4);
    poly130.ps[0] = Point(512.984, 715.608);
    poly130.ps[1] = Point(512.984, 755.608);
    poly130.ps[2] = Point(456.984, 755.608);
    poly130.ps[3] = Point(456.984, 715.608);
    new ShapeRef(router, poly130, 130);
    
    Polygon poly138(4);
    poly138.ps[0] = Point(407.084, 299.608);
    poly138.ps[1] = Point(407.084, 339.608);
    poly138.ps[2] = Point(351.084, 339.608);
    poly138.ps[3] = Point(351.084, 299.608);
    new ShapeRef(router, poly138, 138);
    
    Polygon poly97(4);
    poly97.ps[0] = Point(460.584, -334.392);
    poly97.ps[1] = Point(460.584, -294.392);
    poly97.ps[2] = Point(420.584, -294.392);
    poly97.ps[3] = Point(420.584, -334.392);
    new ShapeRef(router, poly97, 97);
    
    Polygon poly156(4);
    poly156.ps[0] = Point(190.984, 624.465);
    poly156.ps[1] = Point(190.984, 664.465);
    poly156.ps[2] = Point(134.984, 664.465);
    poly156.ps[3] = Point(134.984, 624.465);
    new ShapeRef(router, poly156, 156);
    
    Polygon poly11(4);
    poly11.ps[0] = Point(1075.98, 749.608);
    poly11.ps[1] = Point(1075.98, 769.608);
    poly11.ps[2] = Point(1055.98, 769.608);
    poly11.ps[3] = Point(1055.98, 749.608);
    new ShapeRef(router, poly11, 11);
    
    Polygon poly152(4);
    poly152.ps[0] = Point(13.984, 28.6079);
    poly152.ps[1] = Point(13.984, 68.6079);
    poly152.ps[2] = Point(-42.016, 68.6079);
    poly152.ps[3] = Point(-42.016, 28.6079);
    new ShapeRef(router, poly152, 152);
    
    Polygon poly81(4);
    poly81.ps[0] = Point(122.984, 817.608);
    poly81.ps[1] = Point(122.984, 857.608);
    poly81.ps[2] = Point(82.984, 857.608);
    poly81.ps[3] = Point(82.984, 817.608);
    new ShapeRef(router, poly81, 81);
    
    Polygon poly109(4);
    poly109.ps[0] = Point(372.984, -313.392);
    poly109.ps[1] = Point(372.984, -293.392);
    poly109.ps[2] = Point(352.984, -293.392);
    poly109.ps[3] = Point(352.984, -313.392);
    new ShapeRef(router, poly109, 109);
    
    Polygon poly101(4);
    poly101.ps[0] = Point(-81.016, -179.392);
    poly101.ps[1] = Point(-81.016, -139.392);
    poly101.ps[2] = Point(-121.016, -139.392);
    poly101.ps[3] = Point(-121.016, -179.392);
    new ShapeRef(router, poly101, 101);
    
    Polygon poly64(4);
    poly64.ps[0] = Point(307.984, 112.608);
    poly64.ps[1] = Point(307.984, 152.608);
    poly64.ps[2] = Point(267.984, 152.608);
    poly64.ps[3] = Point(267.984, 112.608);
    new ShapeRef(router, poly64, 64);
    
    Polygon poly92(4);
    poly92.ps[0] = Point(813.984, 929.008);
    poly92.ps[1] = Point(813.984, 969.008);
    poly92.ps[2] = Point(773.984, 969.008);
    poly92.ps[3] = Point(773.984, 929.008);
    new ShapeRef(router, poly92, 92);
    
    Polygon poly27(4);
    poly27.ps[0] = Point(542.984, 680.465);
    poly27.ps[1] = Point(542.984, 700.465);
    poly27.ps[2] = Point(522.984, 700.465);
    poly27.ps[3] = Point(522.984, 680.465);
    new ShapeRef(router, poly27, 27);
    
    Polygon poly9(4);
    poly9.ps[0] = Point(438.984, 1273.01);
    poly9.ps[1] = Point(438.984, 1293.01);
    poly9.ps[2] = Point(418.984, 1293.01);
    poly9.ps[3] = Point(418.984, 1273.01);
    new ShapeRef(router, poly9, 9);
    
    Polygon poly171(4);
    poly171.ps[0] = Point(885.984, 1187.01);
    poly171.ps[1] = Point(885.984, 1227.01);
    poly171.ps[2] = Point(825.984, 1227.01);
    poly171.ps[3] = Point(825.984, 1187.01);
    new ShapeRef(router, poly171, 171);
    
    Polygon poly26(4);
    poly26.ps[0] = Point(852, 549);
    poly26.ps[1] = Point(852, 589);
    poly26.ps[2] = Point(796, 589);
    poly26.ps[3] = Point(796, 549);
    new ShapeRef(router, poly26, 26);
    
    Polygon poly95(4);
    poly95.ps[0] = Point(818.746, 315.608);
    poly95.ps[1] = Point(818.746, 355.608);
    poly95.ps[2] = Point(778.746, 355.608);
    poly95.ps[3] = Point(778.746, 315.608);
    new ShapeRef(router, poly95, 95);
    
    Polygon poly72(4);
    poly72.ps[0] = Point(813.984, 663.608);
    poly72.ps[1] = Point(813.984, 703.608);
    poly72.ps[2] = Point(773.984, 703.608);
    poly72.ps[3] = Point(773.984, 663.608);
    new ShapeRef(router, poly72, 72);
    
    Polygon poly16(4);
    poly16.ps[0] = Point(561.984, 981.008);
    poly16.ps[1] = Point(561.984, 1001.01);
    poly16.ps[2] = Point(541.984, 1001.01);
    poly16.ps[3] = Point(541.984, 981.008);
    new ShapeRef(router, poly16, 16);
    
    Polygon poly35(4);
    poly35.ps[0] = Point(521.534, 122.608);
    poly35.ps[1] = Point(521.534, 142.608);
    poly35.ps[2] = Point(501.534, 142.608);
    poly35.ps[3] = Point(501.534, 122.608);
    new ShapeRef(router, poly35, 35);
    
    Polygon poly34(4);
    poly34.ps[0] = Point(1.98401, 197.608);
    poly34.ps[1] = Point(1.98401, 217.608);
    poly34.ps[2] = Point(-18.016, 217.608);
    poly34.ps[3] = Point(-18.016, 197.608);
    new ShapeRef(router, poly34, 34);
    
    Polygon poly86(4);
    poly86.ps[0] = Point(571.984, 1031.01);
    poly86.ps[1] = Point(571.984, 1071.01);
    poly86.ps[2] = Point(531.984, 1071.01);
    poly86.ps[3] = Point(531.984, 1031.01);
    new ShapeRef(router, poly86, 86);
    
    Polygon poly140(4);
    poly140.ps[0] = Point(539.534, 62.6079);
    poly140.ps[1] = Point(539.534, 102.608);
    poly140.ps[2] = Point(483.534, 102.608);
    poly140.ps[3] = Point(483.534, 62.6079);
    new ShapeRef(router, poly140, 140);
    
    Polygon poly69(4);
    poly69.ps[0] = Point(591.534, 164.608);
    poly69.ps[1] = Point(591.534, 204.608);
    poly69.ps[2] = Point(551.534, 204.608);
    poly69.ps[3] = Point(551.534, 164.608);
    new ShapeRef(router, poly69, 69);
    
    Polygon poly48(4);
    poly48.ps[0] = Point(322.984, 477.608);
    poly48.ps[1] = Point(322.984, 651.608);
    poly48.ps[2] = Point(202.984, 651.608);
    poly48.ps[3] = Point(202.984, 477.608);
    new ShapeRef(router, poly48, 48);
    
    Polygon poly100(4);
    poly100.ps[0] = Point(307.984, -179.392);
    poly100.ps[1] = Point(307.984, -139.392);
    poly100.ps[2] = Point(267.984, -139.392);
    poly100.ps[3] = Point(267.984, -179.392);
    new ShapeRef(router, poly100, 100);
    
    Polygon poly54(4);
    poly54.ps[0] = Point(372.984, -405.392);
    poly54.ps[1] = Point(372.984, -385.392);
    poly54.ps[2] = Point(352.984, -385.392);
    poly54.ps[3] = Point(352.984, -405.392);
    new ShapeRef(router, poly54, 54);
    
    Polygon poly113(4);
    poly113.ps[0] = Point(1024.98, 506.108);
    poly113.ps[1] = Point(1024.98, 546.108);
    poly113.ps[2] = Point(968.984, 546.108);
    poly113.ps[3] = Point(968.984, 506.108);
    new ShapeRef(router, poly113, 113);
    
    Polygon poly155(4);
    poly155.ps[0] = Point(390.984, -373.392);
    poly155.ps[1] = Point(390.984, -333.392);
    poly155.ps[2] = Point(334.984, -333.392);
    poly155.ps[3] = Point(334.984, -373.392);
    new ShapeRef(router, poly155, 155);
    
    Polygon poly87(4);
    poly87.ps[0] = Point(376.184, 939.758);
    poly87.ps[1] = Point(376.184, 979.758);
    poly87.ps[2] = Point(336.184, 979.758);
    poly87.ps[3] = Point(336.184, 939.758);
    new ShapeRef(router, poly87, 87);
    
    Polygon poly63(4);
    poly63.ps[0] = Point(460.584, 112.608);
    poly63.ps[1] = Point(460.584, 152.608);
    poly63.ps[2] = Point(420.584, 152.608);
    poly63.ps[3] = Point(420.584, 112.608);
    new ShapeRef(router, poly63, 63);
    
    Polygon poly129(4);
    poly129.ps[0] = Point(771.984, 715.608);
    poly129.ps[1] = Point(771.984, 755.608);
    poly129.ps[2] = Point(715.984, 755.608);
    poly129.ps[3] = Point(715.984, 715.608);
    new ShapeRef(router, poly129, 129);
    
    Polygon poly76(4);
    poly76.ps[0] = Point(320.984, 663.608);
    poly76.ps[1] = Point(320.984, 703.608);
    poly76.ps[2] = Point(280.984, 703.608);
    poly76.ps[3] = Point(280.984, 663.608);
    new ShapeRef(router, poly76, 76);
    
    Polygon poly93(4);
    poly93.ps[0] = Point(1085.98, 663.608);
    poly93.ps[1] = Point(1085.98, 703.608);
    poly93.ps[2] = Point(1045.98, 703.608);
    poly93.ps[3] = Point(1045.98, 663.608);
    new ShapeRef(router, poly93, 93);
    
    Polygon poly157(4);
    poly157.ps[0] = Point(213, 425.608);
    poly157.ps[1] = Point(213, 465.608);
    poly157.ps[2] = Point(153, 465.608);
    poly157.ps[3] = Point(153, 425.608);
    new ShapeRef(router, poly157, 157);
    
    Polygon poly147(4);
    poly147.ps[0] = Point(530.584, -281.392);
    poly147.ps[1] = Point(530.584, -241.392);
    poly147.ps[2] = Point(474.584, -241.392);
    poly147.ps[3] = Point(474.584, -281.392);
    new ShapeRef(router, poly147, 147);
    
    Polygon poly80(4);
    poly80.ps[0] = Point(122.984, 933.758);
    poly80.ps[1] = Point(122.984, 973.758);
    poly80.ps[2] = Point(82.984, 973.758);
    poly80.ps[3] = Point(82.984, 933.758);
    new ShapeRef(router, poly80, 80);
    
    Polygon poly7(4);
    poly7.ps[0] = Point(326.584, 1263.01);
    poly7.ps[1] = Point(326.584, 1303.01);
    poly7.ps[2] = Point(286.584, 1303.01);
    poly7.ps[3] = Point(286.584, 1263.01);
    new ShapeRef(router, poly7, 7);
    
    Polygon poly5(4);
    poly5.ps[0] = Point(460.584, 216.608);
    poly5.ps[1] = Point(460.584, 256.608);
    poly5.ps[2] = Point(420.584, 256.608);
    poly5.ps[3] = Point(420.584, 216.608);
    new ShapeRef(router, poly5, 5);
    
    Polygon poly148(4);
    poly148.ps[0] = Point(530.584, -434.392);
    poly148.ps[1] = Point(530.584, -394.392);
    poly148.ps[2] = Point(474.584, -394.392);
    poly148.ps[3] = Point(474.584, -434.392);
    new ShapeRef(router, poly148, 148);
    
    Polygon poly166(4);
    poly166.ps[0] = Point(316.984, 1158.01);
    poly166.ps[1] = Point(316.984, 1218.01);
    poly166.ps[2] = Point(276.984, 1218.01);
    poly166.ps[3] = Point(276.984, 1158.01);
    new ShapeRef(router, poly166, 166);
    
    Polygon poly139(4);
    poly139.ps[0] = Point(81.984, 187.608);
    poly139.ps[1] = Point(81.984, 227.608);
    poly139.ps[2] = Point(25.984, 227.608);
    poly139.ps[3] = Point(25.984, 187.608);
    new ShapeRef(router, poly139, 139);
    
    Polygon poly91(4);
    poly91.ps[0] = Point(701.984, 827.008);
    poly91.ps[1] = Point(701.984, 867.008);
    poly91.ps[2] = Point(661.984, 867.008);
    poly91.ps[3] = Point(661.984, 827.008);
    new ShapeRef(router, poly91, 91);
    
    Polygon poly122(4);
    poly122.ps[0] = Point(709.984, 971.008);
    poly122.ps[1] = Point(709.984, 1011.01);
    poly122.ps[2] = Point(653.984, 1011.01);
    poly122.ps[3] = Point(653.984, 971.008);
    new ShapeRef(router, poly122, 122);
    
    Polygon poly6(4);
    poly6.ps[0] = Point(521.534, 226.608);
    poly6.ps[1] = Point(521.534, 246.608);
    poly6.ps[2] = Point(501.534, 246.608);
    poly6.ps[3] = Point(501.534, 226.608);
    new ShapeRef(router, poly6, 6);
    
    Polygon poly18(4);
    poly18.ps[0] = Point(517, 916.608);
    poly18.ps[1] = Point(517, 956.608);
    poly18.ps[2] = Point(461, 956.608);
    poly18.ps[3] = Point(461, 916.608);
    new ShapeRef(router, poly18, 18);
    
    Polygon poly39(4);
    poly39.ps[0] = Point(372.984, -13.3921);
    poly39.ps[1] = Point(372.984, 6.60786);
    poly39.ps[2] = Point(352.984, 6.60786);
    poly39.ps[3] = Point(352.984, -13.3921);
    new ShapeRef(router, poly39, 39);
    
    Polygon poly25(4);
    poly25.ps[0] = Point(310.984, 725.608);
    poly25.ps[1] = Point(310.984, 745.608);
    poly25.ps[2] = Point(290.984, 745.608);
    poly25.ps[3] = Point(290.984, 725.608);
    new ShapeRef(router, poly25, 25);
    
    Polygon poly56(4);
    poly56.ps[0] = Point(79.984, -169.392);
    poly56.ps[1] = Point(79.984, -149.392);
    poly56.ps[2] = Point(59.984, -149.392);
    poly56.ps[3] = Point(59.984, -169.392);
    new ShapeRef(router, poly56, 56);
    
    Polygon poly21(4);
    poly21.ps[0] = Point(432.984, 874.608);
    poly21.ps[1] = Point(432.984, 894.608);
    poly21.ps[2] = Point(412.984, 894.608);
    poly21.ps[3] = Point(412.984, 874.608);
    new ShapeRef(router, poly21, 21);
    
    Polygon poly47(4);
    poly47.ps[0] = Point(1291.98, 128.608);
    poly47.ps[1] = Point(1291.98, 303.608);
    poly47.ps[2] = Point(977.984, 303.608);
    poly47.ps[3] = Point(977.984, 128.608);
    new ShapeRef(router, poly47, 47);
    
    Polygon poly62(4);
    poly62.ps[0] = Point(460.584, -23.3921);
    poly62.ps[1] = Point(460.584, 16.6079);
    poly62.ps[2] = Point(420.584, 16.6079);
    poly62.ps[3] = Point(420.584, -23.3921);
    new ShapeRef(router, poly62, 62);
    
    Polygon poly74(4);
    poly74.ps[0] = Point(11.984, 425.608);
    poly74.ps[1] = Point(11.984, 465.608);
    poly74.ps[2] = Point(-28.016, 465.608);
    poly74.ps[3] = Point(-28.016, 425.608);
    new ShapeRef(router, poly74, 74);
    
    Polygon poly89(4);
    poly89.ps[0] = Point(979.984, 663.608);
    poly89.ps[1] = Point(979.984, 703.608);
    poly89.ps[2] = Point(939.984, 703.608);
    poly89.ps[3] = Point(939.984, 663.608);
    new ShapeRef(router, poly89, 89);
    
    Polygon poly143(4);
    poly143.ps[0] = Point(81.984, 70.6079);
    poly143.ps[1] = Point(81.984, 110.608);
    poly143.ps[2] = Point(25.984, 110.608);
    poly143.ps[3] = Point(25.984, 70.6079);
    new ShapeRef(router, poly143, 143);
    
    Polygon poly135(4);
    poly135.ps[0] = Point(407.084, 383.608);
    poly135.ps[1] = Point(407.084, 423.608);
    poly135.ps[2] = Point(351.084, 423.608);
    poly135.ps[3] = Point(351.084, 383.608);
    new ShapeRef(router, poly135, 135);
    
    Polygon poly46(4);
    poly46.ps[0] = Point(285.084, 281.608);
    poly46.ps[1] = Point(285.084, 341.608);
    poly46.ps[2] = Point(49.084, 341.608);
    poly46.ps[3] = Point(49.084, 281.608);
    new ShapeRef(router, poly46, 46);
    
    Polygon poly144(4);
    poly144.ps[0] = Point(390.984, -73.3921);
    poly144.ps[1] = Point(390.984, -33.3921);
    poly144.ps[2] = Point(334.984, -33.3921);
    poly144.ps[3] = Point(334.984, -73.3921);
    new ShapeRef(router, poly144, 144);
    
    Polygon poly128(4);
    poly128.ps[0] = Point(883.984, 715.608);
    poly128.ps[1] = Point(883.984, 755.608);
    poly128.ps[2] = Point(827.984, 755.608);
    poly128.ps[3] = Point(827.984, 715.608);
    new ShapeRef(router, poly128, 128);
    
    Polygon poly111(4);
    poly111.ps[0] = Point(172.984, 684.465);
    poly111.ps[1] = Point(172.984, 704.465);
    poly111.ps[2] = Point(152.984, 704.465);
    poly111.ps[3] = Point(152.984, 684.465);
    new ShapeRef(router, poly111, 111);
    
    Polygon poly45(4);
    poly45.ps[0] = Point(388.984, 1086.01);
    poly45.ps[1] = Point(388.984, 1251.01);
    poly45.ps[2] = Point(73.984, 1251.01);
    poly45.ps[3] = Point(73.984, 1086.01);
    new ShapeRef(router, poly45, 45);
    
    Polygon poly172(4);
    poly172.ps[0] = Point(532.584, -117.392);
    poly172.ps[1] = Point(532.584, -77.3921);
    poly172.ps[2] = Point(472.584, -77.3921);
    poly172.ps[3] = Point(472.584, -117.392);
    new ShapeRef(router, poly172, 172);
    
    Polygon poly83(4);
    poly83.ps[0] = Point(813.984, 1187.01);
    poly83.ps[1] = Point(813.984, 1227.01);
    poly83.ps[2] = Point(773.984, 1227.01);
    poly83.ps[3] = Point(773.984, 1187.01);
    new ShapeRef(router, poly83, 83);
    
    Polygon poly58(4);
    poly58.ps[0] = Point(749.984, 510.608);
    poly58.ps[1] = Point(749.984, 530.608);
    poly58.ps[2] = Point(729.984, 530.608);
    poly58.ps[3] = Point(729.984, 510.608);
    new ShapeRef(router, poly58, 58);
    
    Polygon poly106(4);
    poly106.ps[0] = Point(502.984, 448.608);
    poly106.ps[1] = Point(502.984, 488.608);
    poly106.ps[2] = Point(462.984, 488.608);
    poly106.ps[3] = Point(462.984, 448.608);
    new ShapeRef(router, poly106, 106);
    
    Polygon poly41(4);
    poly41.ps[0] = Point(285.084, 405.608);
    poly41.ps[1] = Point(285.084, 465.608);
    poly41.ps[2] = Point(52.084, 465.608);
    poly41.ps[3] = Point(52.084, 405.608);
    new ShapeRef(router, poly41, 41);
    
    Polygon poly132(4);
    poly132.ps[0] = Point(560.984, 620.465);
    poly132.ps[1] = Point(560.984, 660.465);
    poly132.ps[2] = Point(504.984, 660.465);
    poly132.ps[3] = Point(504.984, 620.465);
    new ShapeRef(router, poly132, 132);
    
    Polygon poly127(4);
    poly127.ps[0] = Point(512.984, 864.608);
    poly127.ps[1] = Point(512.984, 904.608);
    poly127.ps[2] = Point(456.984, 904.608);
    poly127.ps[3] = Point(456.984, 864.608);
    new ShapeRef(router, poly127, 127);
    
    Polygon poly124(4);
    poly124.ps[0] = Point(709.984, 879.008);
    poly124.ps[1] = Point(709.984, 919.008);
    poly124.ps[2] = Point(653.984, 919.008);
    poly124.ps[3] = Point(653.984, 879.008);
    new ShapeRef(router, poly124, 124);
    
    Polygon poly168(4);
    poly168.ps[0] = Point(1037.98, 263.5);
    poly168.ps[1] = Point(1037.98, 303.5);
    poly168.ps[2] = Point(977.984, 303.5);
    poly168.ps[3] = Point(977.984, 263.5);
    new ShapeRef(router, poly168, 168);
    
    Polygon poly85(4);
    poly85.ps[0] = Point(442.984, 817.608);
    poly85.ps[1] = Point(442.984, 857.608);
    poly85.ps[2] = Point(402.984, 857.608);
    poly85.ps[3] = Point(402.984, 817.608);
    new ShapeRef(router, poly85, 85);
    
    Polygon poly149(4);
    poly149.ps[0] = Point(390.984, -465.392);
    poly149.ps[1] = Point(390.984, -425.392);
    poly149.ps[2] = Point(334.984, -425.392);
    poly149.ps[3] = Point(334.984, -465.392);
    new ShapeRef(router, poly149, 149);
    
    Polygon poly79(4);
    poly79.ps[0] = Point(122.984, 1034.01);
    poly79.ps[1] = Point(122.984, 1074.01);
    poly79.ps[2] = Point(82.984, 1074.01);
    poly79.ps[3] = Point(82.984, 1034.01);
    new ShapeRef(router, poly79, 79);
    
    Polygon poly133(4);
    poly133.ps[0] = Point(390.984, 613.608);
    poly133.ps[1] = Point(390.984, 653.608);
    poly133.ps[2] = Point(334.984, 653.608);
    poly133.ps[3] = Point(334.984, 613.608);
    new ShapeRef(router, poly133, 133);
    
    Polygon poly14(4);
    poly14.ps[0] = Point(691.984, 1123.01);
    poly14.ps[1] = Point(691.984, 1143.01);
    poly14.ps[2] = Point(671.984, 1143.01);
    poly14.ps[3] = Point(671.984, 1123.01);
    new ShapeRef(router, poly14, 14);
    
    Polygon poly30(4);
    poly30.ps[0] = Point(327.084, 393.608);
    poly30.ps[1] = Point(327.084, 413.608);
    poly30.ps[2] = Point(307.084, 413.608);
    poly30.ps[3] = Point(307.084, 393.608);
    new ShapeRef(router, poly30, 30);
    
    Polygon poly20(4);
    poly20.ps[0] = Point(492.984, 827.608);
    poly20.ps[1] = Point(492.984, 847.608);
    poly20.ps[2] = Point(472.984, 847.608);
    poly20.ps[3] = Point(472.984, 827.608);
    new ShapeRef(router, poly20, 20);
    
    Polygon poly38(4);
    poly38.ps[0] = Point(1.98401, 80.6079);
    poly38.ps[1] = Point(1.98401, 100.608);
    poly38.ps[2] = Point(-18.016, 100.608);
    poly38.ps[3] = Point(-18.016, 80.6079);
    new ShapeRef(router, poly38, 38);
    
    Polygon poly44(4);
    poly44.ps[0] = Point(408.584, -512.392);
    poly44.ps[1] = Point(408.584, -477.392);
    poly44.ps[2] = Point(20.584, -477.392);
    poly44.ps[3] = Point(20.584, -512.392);
    new ShapeRef(router, poly44, 44);
    
    Polygon poly94(4);
    poly94.ps[0] = Point(585.984, 817.608);
    poly94.ps[1] = Point(585.984, 857.608);
    poly94.ps[2] = Point(545.984, 857.608);
    poly94.ps[3] = Point(545.984, 817.608);
    new ShapeRef(router, poly94, 94);
    
    Polygon poly32(4);
    poly32.ps[0] = Point(390.584, 226.608);
    poly32.ps[1] = Point(390.584, 246.608);
    poly32.ps[2] = Point(370.584, 246.608);
    poly32.ps[3] = Point(370.584, 226.608);
    new ShapeRef(router, poly32, 32);
    
    Polygon poly8(4);
    poly8.ps[0] = Point(576.484, 1263.01);
    poly8.ps[1] = Point(576.484, 1303.01);
    poly8.ps[2] = Point(536.484, 1303.01);
    poly8.ps[3] = Point(536.484, 1263.01);
    new ShapeRef(router, poly8, 8);
    
    Polygon poly59(4);
    poly59.ps[0] = Point(552.984, 510.608);
    poly59.ps[1] = Point(552.984, 530.608);
    poly59.ps[2] = Point(532.984, 530.608);
    poly59.ps[3] = Point(532.984, 510.608);
    new ShapeRef(router, poly59, 59);
    
    Polygon poly102(4);
    poly102.ps[0] = Point(1047.98, 315.608);
    poly102.ps[1] = Point(1047.98, 355.608);
    poly102.ps[2] = Point(1007.98, 355.608);
    poly102.ps[3] = Point(1007.98, 315.608);
    new ShapeRef(router, poly102, 102);
    
    Polygon poly67(4);
    poly67.ps[0] = Point(818.746, 164.608);
    poly67.ps[1] = Point(818.746, 204.608);
    poly67.ps[2] = Point(778.746, 204.608);
    poly67.ps[3] = Point(778.746, 164.608);
    new ShapeRef(router, poly67, 67);
    
    Polygon poly77(4);
    poly77.ps[0] = Point(429.484, 1031.01);
    poly77.ps[1] = Point(429.484, 1071.01);
    poly77.ps[2] = Point(389.484, 1071.01);
    poly77.ps[3] = Point(389.484, 1031.01);
    new ShapeRef(router, poly77, 77);
    
    Polygon poly163(4);
    poly163.ps[0] = Point(886.5, 56.6079);
    poly163.ps[1] = Point(886.5, 116.608);
    poly163.ps[2] = Point(846.5, 116.608);
    poly163.ps[3] = Point(846.5, 56.6079);
    new ShapeRef(router, poly163, 163);
    
    Polygon poly2(4);
    poly2.ps[0] = Point(954.984, 442.608);
    poly2.ps[1] = Point(954.984, 482.608);
    poly2.ps[2] = Point(914.984, 482.608);
    poly2.ps[3] = Point(914.984, 442.608);
    new ShapeRef(router, poly2, 2);
    
    Polygon poly73(4);
    poly73.ps[0] = Point(122.984, 663.608);
    poly73.ps[1] = Point(122.984, 703.608);
    poly73.ps[2] = Point(82.984, 703.608);
    poly73.ps[3] = Point(82.984, 663.608);
    new ShapeRef(router, poly73, 73);
    
    Polygon poly65(4);
    poly65.ps[0] = Point(307.984, -23.3921);
    poly65.ps[1] = Point(307.984, 16.6079);
    poly65.ps[2] = Point(267.984, 16.6079);
    poly65.ps[3] = Point(267.984, -23.3921);
    new ShapeRef(router, poly65, 65);
    
    Polygon poly71(4);
    poly71.ps[0] = Point(337.084, 341.608);
    poly71.ps[1] = Point(337.084, 381.608);
    poly71.ps[2] = Point(297.084, 381.608);
    poly71.ps[3] = Point(297.084, 341.608);
    new ShapeRef(router, poly71, 71);
    
    Polygon poly110(4);
    poly110.ps[0] = Point(786.584, -137.392);
    poly110.ps[1] = Point(786.584, -77.3921);
    poly110.ps[2] = Point(472.584, -77.3921);
    poly110.ps[3] = Point(472.584, -137.392);
    new ShapeRef(router, poly110, 110);
    
    Polygon poly84(4);
    poly84.ps[0] = Point(571.984, 929.008);
    poly84.ps[1] = Point(571.984, 969.008);
    poly84.ps[2] = Point(531.984, 969.008);
    poly84.ps[3] = Point(531.984, 929.008);
    new ShapeRef(router, poly84, 84);
    
    Polygon poly125(4);
    poly125.ps[0] = Point(883.984, 859.608);
    poly125.ps[1] = Point(883.984, 899.608);
    poly125.ps[2] = Point(827.984, 899.608);
    poly125.ps[3] = Point(827.984, 859.608);
    new ShapeRef(router, poly125, 125);
    
    Polygon poly96(4);
    poly96.ps[0] = Point(460.584, -476.392);
    poly96.ps[1] = Point(460.584, -436.392);
    poly96.ps[2] = Point(420.584, -436.392);
    poly96.ps[3] = Point(420.584, -476.392);
    new ShapeRef(router, poly96, 96);
    
    Polygon poly167(4);
    poly167.ps[0] = Point(285.084, 301.608);
    poly167.ps[1] = Point(285.084, 341.608);
    poly167.ps[2] = Point(225.084, 341.608);
    poly167.ps[3] = Point(225.084, 301.608);
    new ShapeRef(router, poly167, 167);
    
    Polygon poly43(4);
    poly43.ps[0] = Point(1007, -57.3921);
    poly43.ps[1] = Point(1007, 116.608);
    poly43.ps[2] = Point(725, 116.608);
    poly43.ps[3] = Point(725, -57.3921);
    new ShapeRef(router, poly43, 43);
    
    Polygon poly55(4);
    poly55.ps[0] = Point(297.984, -271.392);
    poly55.ps[1] = Point(297.984, -251.392);
    poly55.ps[2] = Point(277.984, -251.392);
    poly55.ps[3] = Point(277.984, -271.392);
    new ShapeRef(router, poly55, 55);
    
    Polygon poly98(4);
    poly98.ps[0] = Point(460.584, -179.392);
    poly98.ps[1] = Point(460.584, -139.392);
    poly98.ps[2] = Point(420.584, -139.392);
    poly98.ps[3] = Point(420.584, -179.392);
    new ShapeRef(router, poly98, 98);
    
    Polygon poly42(4);
    poly42.ps[0] = Point(699.984, 314.608);
    poly42.ps[1] = Point(699.984, 488.608);
    poly42.ps[2] = Point(582.984, 488.608);
    poly42.ps[3] = Point(582.984, 314.608);
    new ShapeRef(router, poly42, 42);
    
    Polygon poly105(4);
    poly105.ps[0] = Point(622.984, 500.608);
    poly105.ps[1] = Point(622.984, 540.608);
    poly105.ps[2] = Point(582.984, 540.608);
    poly105.ps[3] = Point(582.984, 500.608);
    new ShapeRef(router, poly105, 105);
    
    Polygon poly1(4);
    poly1.ps[0] = Point(521.534, -169.392);
    poly1.ps[1] = Point(521.534, -149.392);
    poly1.ps[2] = Point(501.534, -149.392);
    poly1.ps[3] = Point(501.534, -169.392);
    new ShapeRef(router, poly1, 1);
    
    Polygon poly165(4);
    poly165.ps[0] = Point(316.984, 1086.01);
    poly165.ps[1] = Point(316.984, 1146.01);
    poly165.ps[2] = Point(276.984, 1146.01);
    poly165.ps[3] = Point(276.984, 1086.01);
    new ShapeRef(router, poly165, 165);
    
    Polygon poly10(4);
    poly10.ps[0] = Point(566.484, 1213.01);
    poly10.ps[1] = Point(566.484, 1233.01);
    poly10.ps[2] = Point(546.484, 1233.01);
    poly10.ps[3] = Point(546.484, 1213.01);
    new ShapeRef(router, poly10, 10);
    
    Polygon poly24(4);
    poly24.ps[0] = Point(432.984, 725.608);
    poly24.ps[1] = Point(432.984, 745.608);
    poly24.ps[2] = Point(412.984, 745.608);
    poly24.ps[3] = Point(412.984, 725.608);
    new ShapeRef(router, poly24, 24);
    
    Polygon poly108(4);
    poly108.ps[0] = Point(11.984, 112.608);
    poly108.ps[1] = Point(11.984, 152.608);
    poly108.ps[2] = Point(-28.016, 152.608);
    poly108.ps[3] = Point(-28.016, 112.608);
    new ShapeRef(router, poly108, 108);
    
    Polygon poly123(4);
    poly123.ps[0] = Point(641.984, 971.008);
    poly123.ps[1] = Point(641.984, 1011.01);
    poly123.ps[2] = Point(585.984, 1011.01);
    poly123.ps[3] = Point(585.984, 971.008);
    new ShapeRef(router, poly123, 123);
    
    ConnRef *connRef279 = new ConnRef(router, 279);
    ConnEnd srcPt279(Point(743.984, 735.608), 15);
    connRef279->setSourceEndpoint(srcPt279);
    ConnEnd dstPt279(Point(681.984, 735.608), 15);
    connRef279->setDestEndpoint(dstPt279);
    connRef279->setRoutingType((ConnType)2);

    ConnRef *connRef280 = new ConnRef(router, 280);
    ConnEnd srcPt280(Point(484.984, 735.608), 15);
    connRef280->setSourceEndpoint(srcPt280);
    ConnEnd dstPt280(Point(422.984, 735.608), 15);
    connRef280->setDestEndpoint(dstPt280);
    connRef280->setRoutingType((ConnType)2);

    ConnRef *connRef327 = new ConnRef(router, 327);
    ConnEnd srcPt327(Point(1065.98, 683.608), 15);
    connRef327->setSourceEndpoint(srcPt327);
    ConnEnd dstPt327(Point(1069.98, 298.608), 2);
    connRef327->setDestEndpoint(dstPt327);
    connRef327->setRoutingType((ConnType)2);

    ConnRef *connRef330 = new ConnRef(router, 330);
    ConnEnd srcPt330(Point(440.584, -3.39214), 15);
    connRef330->setSourceEndpoint(srcPt330);
    ConnEnd dstPt330(Point(477.584, -97.3921), 4);
    connRef330->setDestEndpoint(dstPt330);
    connRef330->setRoutingType((ConnType)2);

    ConnRef *connRef176 = new ConnRef(router, 176);
    ConnEnd srcPt176(Point(934.984, 536.108), 15);
    connRef176->setSourceEndpoint(srcPt176);
    ConnEnd dstPt176(Point(793.984, 837.608), 15);
    connRef176->setDestEndpoint(dstPt176);
    connRef176->setRoutingType((ConnType)2);

    ConnRef *connRef177 = new ConnRef(router, 177);
    ConnEnd srcPt177(Point(440.584, 236.608), 15);
    connRef177->setSourceEndpoint(srcPt177);
    ConnEnd dstPt177(Point(501.534, 236.608), 15);
    connRef177->setDestEndpoint(dstPt177);
    connRef177->setRoutingType((ConnType)2);

    ConnRef *connRef312 = new ConnRef(router, 312);
    ConnEnd srcPt312(Point(440.584, -159.392), 15);
    connRef312->setSourceEndpoint(srcPt312);
    ConnEnd dstPt312(Point(440.584, -55.3921), 15);
    connRef312->setDestEndpoint(dstPt312);
    connRef312->setRoutingType((ConnType)2);

    ConnRef *connRef306 = new ConnRef(router, 306);
    ConnEnd srcPt306(Point(162.984, 644.465), 15);
    connRef306->setSourceEndpoint(srcPt306);
    ConnEnd dstPt306(Point(162.984, 694.465), 15);
    connRef306->setDestEndpoint(dstPt306);
    connRef306->setRoutingType((ConnType)2);

    ConnRef *connRef198 = new ConnRef(router, 198);
    ConnEnd srcPt198(Point(422.984, 894.608), 15);
    connRef198->setSourceEndpoint(srcPt198);
    ConnEnd dstPt198(Point(551.984, 949.008), 15);
    connRef198->setDestEndpoint(dstPt198);
    connRef198->setRoutingType((ConnType)2);

    ConnRef *connRef199 = new ConnRef(router, 199);
    ConnEnd srcPt199(Point(422.984, 874.608), 15);
    connRef199->setSourceEndpoint(srcPt199);
    ConnEnd dstPt199(Point(422.984, 837.608), 15);
    connRef199->setDestEndpoint(dstPt199);
    connRef199->setRoutingType((ConnType)2);

    ConnRef *connRef282 = new ConnRef(router, 282);
    ConnEnd srcPt282(Point(532.984, 640.465), 15);
    connRef282->setSourceEndpoint(srcPt282);
    ConnEnd dstPt282(Point(532.984, 690.465), 15);
    connRef282->setDestEndpoint(dstPt282);
    connRef282->setRoutingType((ConnType)2);

    ConnRef *connRef283 = new ConnRef(router, 283);
    ConnEnd srcPt283(Point(362.984, 633.608), 15);
    connRef283->setSourceEndpoint(srcPt283);
    ConnEnd dstPt283(Point(362.984, 683.608), 15);
    connRef283->setDestEndpoint(dstPt283);
    connRef283->setRoutingType((ConnType)2);

    ConnRef *connRef186 = new ConnRef(router, 186);
    ConnEnd srcPt186(Point(913.984, 949.008), 15);
    connRef186->setSourceEndpoint(srcPt186);
    ConnEnd dstPt186(Point(793.984, 949.008), 15);
    connRef186->setDestEndpoint(dstPt186);
    connRef186->setRoutingType((ConnType)2);

    ConnRef *connRef187 = new ConnRef(router, 187);
    ConnEnd srcPt187(Point(753.984, 1145.01), 15);
    connRef187->setSourceEndpoint(srcPt187);
    ConnEnd dstPt187(Point(793.984, 1051.01), 15);
    connRef187->setDestEndpoint(dstPt187);
    connRef187->setRoutingType((ConnType)2);

    ConnRef *connRef222 = new ConnRef(router, 222);
    ConnEnd srcPt222(Point(501.534, 132.608), 15);
    connRef222->setSourceEndpoint(srcPt222);
    ConnEnd dstPt222(Point(440.584, 132.608), 15);
    connRef222->setDestEndpoint(dstPt222);
    connRef222->setRoutingType((ConnType)2);

    ConnRef *connRef223 = new ConnRef(router, 223);
    ConnEnd srcPt223(Point(521.534, 132.608), 15);
    connRef223->setSourceEndpoint(srcPt223);
    ConnEnd dstPt223(Point(571.534, 184.608), 15);
    connRef223->setDestEndpoint(dstPt223);
    connRef223->setRoutingType((ConnType)2);

    ConnRef *connRef284 = new ConnRef(router, 284);
    ConnEnd srcPt284(Point(860.746, 239.608), 15);
    connRef284->setSourceEndpoint(srcPt284);
    ConnEnd dstPt284(Point(798.746, 239.608), 15);
    connRef284->setDestEndpoint(dstPt284);
    connRef284->setRoutingType((ConnType)2);

    ConnRef *connRef174 = new ConnRef(router, 174);
    ConnEnd srcPt174(Point(934.984, 462.608), 15);
    connRef174->setSourceEndpoint(srcPt174);
    ConnEnd dstPt174(Point(934.984, 516.108), 15);
    connRef174->setDestEndpoint(dstPt174);
    connRef174->setRoutingType((ConnType)2);

    ConnRef *connRef175 = new ConnRef(router, 175);
    ConnEnd srcPt175(Point(868.746, 429.608), 15);
    connRef175->setSourceEndpoint(srcPt175);
    ConnEnd dstPt175(Point(934.984, 462.608), 15);
    connRef175->setDestEndpoint(dstPt175);
    connRef175->setRoutingType((ConnType)2);

    ConnRef *connRef188 = new ConnRef(router, 188);
    ConnEnd srcPt188(Point(753.984, 1165.01), 15);
    connRef188->setSourceEndpoint(srcPt188);
    ConnEnd dstPt188(Point(793.984, 1207.01), 15);
    connRef188->setDestEndpoint(dstPt188);
    connRef188->setRoutingType((ConnType)2);

    ConnRef *connRef189 = new ConnRef(router, 189);
    ConnEnd srcPt189(Point(671.984, 1133.01), 15);
    connRef189->setSourceEndpoint(srcPt189);
    ConnEnd dstPt189(Point(551.984, 1051.01), 15);
    connRef189->setDestEndpoint(dstPt189);
    connRef189->setRoutingType((ConnType)2);

    ConnRef *connRef252 = new ConnRef(router, 252);
    ConnEnd srcPt252(Point(492.984, 837.608), 15);
    connRef252->setSourceEndpoint(srcPt252);
    ConnEnd dstPt252(Point(565.984, 837.608), 15);
    connRef252->setDestEndpoint(dstPt252);
    connRef252->setRoutingType((ConnType)2);

    ConnRef *connRef253 = new ConnRef(router, 253);
    ConnEnd srcPt253(Point(798.746, 335.608), 15);
    connRef253->setSourceEndpoint(srcPt253);
    ConnEnd dstPt253(Point(858.746, 327.608), 15);
    connRef253->setDestEndpoint(dstPt253);
    connRef253->setRoutingType((ConnType)2);

    ConnRef *connRef315 = new ConnRef(router, 315);
    ConnEnd srcPt315(Point(-8.01599, 445.608), 15);
    connRef315->setSourceEndpoint(srcPt315);
    ConnEnd dstPt315(Point(158, 445.608), 4);
    connRef315->setDestEndpoint(dstPt315);
    connRef315->setRoutingType((ConnType)2);

    ConnRef *connRef321 = new ConnRef(router, 321);
    ConnEnd srcPt321(Point(1065.98, 683.608), 15);
    connRef321->setSourceEndpoint(srcPt321);
    ConnEnd dstPt321(Point(866.5, 111.608), 2);
    connRef321->setDestEndpoint(dstPt321);
    connRef321->setRoutingType((ConnType)2);

    ConnRef *connRef180 = new ConnRef(router, 180);
    ConnEnd srcPt180(Point(438.984, 1283.01), 15);
    connRef180->setSourceEndpoint(srcPt180);
    ConnEnd dstPt180(Point(556.484, 1283.01), 15);
    connRef180->setDestEndpoint(dstPt180);
    connRef180->setRoutingType((ConnType)2);

    ConnRef *connRef181 = new ConnRef(router, 181);
    ConnEnd srcPt181(Point(556.484, 1233.01), 15);
    connRef181->setSourceEndpoint(srcPt181);
    ConnEnd dstPt181(Point(556.484, 1283.01), 15);
    connRef181->setDestEndpoint(dstPt181);
    connRef181->setRoutingType((ConnType)2);

    ConnRef *connRef288 = new ConnRef(router, 288);
    ConnEnd srcPt288(Point(379.084, 319.608), 15);
    connRef288->setSourceEndpoint(srcPt288);
    ConnEnd dstPt288(Point(317.084, 319.608), 15);
    connRef288->setDestEndpoint(dstPt288);
    connRef288->setRoutingType((ConnType)2);

    ConnRef *connRef289 = new ConnRef(router, 289);
    ConnEnd srcPt289(Point(53.984, 207.608), 15);
    connRef289->setSourceEndpoint(srcPt289);
    ConnEnd dstPt289(Point(-8.01599, 207.608), 15);
    connRef289->setDestEndpoint(dstPt289);
    connRef289->setRoutingType((ConnType)2);

    ConnRef *connRef328 = new ConnRef(router, 328);
    ConnEnd srcPt328(Point(300.984, 683.608), 15);
    connRef328->setSourceEndpoint(srcPt328);
    ConnEnd dstPt328(Point(302.984, 591.5), 2);
    connRef328->setDestEndpoint(dstPt328);
    connRef328->setRoutingType((ConnType)2);

    ConnRef *connRef329 = new ConnRef(router, 329);
    ConnEnd srcPt329(Point(793.984, 1207.01), 15);
    connRef329->setSourceEndpoint(srcPt329);
    ConnEnd dstPt329(Point(830.984, 1207.01), 4);
    connRef329->setDestEndpoint(dstPt329);
    connRef329->setRoutingType((ConnType)2);

    ConnRef *connRef192 = new ConnRef(router, 192);
    ConnEnd srcPt192(Point(551.984, 1001.01), 15);
    connRef192->setSourceEndpoint(srcPt192);
    ConnEnd dstPt192(Point(551.984, 1051.01), 15);
    connRef192->setDestEndpoint(dstPt192);
    connRef192->setRoutingType((ConnType)2);

    ConnRef *connRef193 = new ConnRef(router, 193);
    ConnEnd srcPt193(Point(671.984, 949.008), 15);
    connRef193->setSourceEndpoint(srcPt193);
    ConnEnd dstPt193(Point(551.984, 949.008), 15);
    connRef193->setDestEndpoint(dstPt193);
    connRef193->setRoutingType((ConnType)2);

    ConnRef *connRef215 = new ConnRef(router, 215);
    ConnEnd srcPt215(Point(-8.01599, 363.608), 15);
    connRef215->setSourceEndpoint(srcPt215);
    ConnEnd dstPt215(Point(-8.01599, 249.608), 15);
    connRef215->setDestEndpoint(dstPt215);
    connRef215->setRoutingType((ConnType)2);

    ConnRef *connRef276 = new ConnRef(router, 276);
    ConnEnd srcPt276(Point(482.984, 787.608), 15);
    connRef276->setSourceEndpoint(srcPt276);
    ConnEnd dstPt276(Point(482.984, 837.608), 15);
    connRef276->setDestEndpoint(dstPt276);
    connRef276->setRoutingType((ConnType)2);

    ConnRef *connRef277 = new ConnRef(router, 277);
    ConnEnd srcPt277(Point(484.984, 884.608), 15);
    connRef277->setSourceEndpoint(srcPt277);
    ConnEnd dstPt277(Point(422.984, 884.608), 15);
    connRef277->setDestEndpoint(dstPt277);
    connRef277->setRoutingType((ConnType)2);

    ConnRef *connRef273 = new ConnRef(router, 273);
    ConnEnd srcPt273(Point(613.984, 991.008), 15);
    connRef273->setSourceEndpoint(srcPt273);
    ConnEnd dstPt273(Point(551.984, 991.008), 15);
    connRef273->setDestEndpoint(dstPt273);
    connRef273->setRoutingType((ConnType)2);

    ConnRef *connRef274 = new ConnRef(router, 274);
    ConnEnd srcPt274(Point(681.984, 899.008), 15);
    connRef274->setSourceEndpoint(srcPt274);
    ConnEnd dstPt274(Point(681.984, 949.008), 15);
    connRef274->setDestEndpoint(dstPt274);
    connRef274->setRoutingType((ConnType)2);

    ConnRef *connRef313 = new ConnRef(router, 313);
    ConnEnd srcPt313(Point(440.584, -314.392), 15);
    connRef313->setSourceEndpoint(srcPt313);
    ConnEnd dstPt313(Point(440.584, -271.392), 15);
    connRef313->setDestEndpoint(dstPt313);
    connRef313->setRoutingType((ConnType)2);

    ConnRef *connRef314 = new ConnRef(router, 314);
    ConnEnd srcPt314(Point(287.984, -314.392), 15);
    connRef314->setSourceEndpoint(srcPt314);
    ConnEnd dstPt314(Point(287.984, -271.392), 15);
    connRef314->setDestEndpoint(dstPt314);
    connRef314->setRoutingType((ConnType)2);

    ConnRef *connRef248 = new ConnRef(router, 248);
    ConnEnd srcPt248(Point(691.984, 1041.01), 15);
    connRef248->setSourceEndpoint(srcPt248);
    ConnEnd dstPt248(Point(793.984, 1051.01), 15);
    connRef248->setDestEndpoint(dstPt248);
    connRef248->setRoutingType((ConnType)2);

    ConnRef *connRef309 = new ConnRef(router, 309);
    ConnEnd srcPt309(Point(317.084, 445.608), 15);
    connRef309->setSourceEndpoint(srcPt309);
    ConnEnd dstPt309(Point(317.084, 413.608), 15);
    connRef309->setDestEndpoint(dstPt309);
    connRef309->setRoutingType((ConnType)2);

    ConnRef *connRef203 = new ConnRef(router, 203);
    ConnEnd srcPt203(Point(681.984, 745.608), 15);
    connRef203->setSourceEndpoint(srcPt203);
    ConnEnd dstPt203(Point(681.984, 847.008), 15);
    connRef203->setDestEndpoint(dstPt203);
    connRef203->setRoutingType((ConnType)2);

    ConnRef *connRef239 = new ConnRef(router, 239);
    ConnEnd srcPt239(Point(532.984, 520.608), 15);
    connRef239->setSourceEndpoint(srcPt239);
    ConnEnd dstPt239(Point(482.984, 468.608), 15);
    connRef239->setDestEndpoint(dstPt239);
    connRef239->setRoutingType((ConnType)2);

    ConnRef *connRef316 = new ConnRef(router, 316);
    ConnEnd srcPt316(Point(317.084, 445.608), 15);
    connRef316->setSourceEndpoint(srcPt316);
    ConnEnd dstPt316(Point(280.084, 445.608), 8);
    connRef316->setDestEndpoint(dstPt316);
    connRef316->setRoutingType((ConnType)2);

    ConnRef *connRef317 = new ConnRef(router, 317);
    ConnEnd srcPt317(Point(571.534, 184.608), 15);
    connRef317->setSourceEndpoint(srcPt317);
    ConnEnd dstPt317(Point(602.984, 335.5), 1);
    connRef317->setDestEndpoint(dstPt317);
    connRef317->setRoutingType((ConnType)2);

    ConnRef *connRef234 = new ConnRef(router, 234);
    ConnEnd srcPt234(Point(287.984, -251.392), 15);
    connRef234->setSourceEndpoint(srcPt234);
    ConnEnd dstPt234(Point(287.984, -159.392), 15);
    connRef234->setDestEndpoint(dstPt234);
    connRef234->setRoutingType((ConnType)2);

    ConnRef *connRef235 = new ConnRef(router, 235);
    ConnEnd srcPt235(Point(59.984, -159.392), 15);
    connRef235->setSourceEndpoint(srcPt235);
    ConnEnd dstPt235(Point(-101.016, -159.392), 15);
    connRef235->setDestEndpoint(dstPt235);
    connRef235->setRoutingType((ConnType)2);

    ConnRef *connRef322 = new ConnRef(router, 322);
    ConnEnd srcPt322(Point(551.984, 1051.01), 15);
    connRef322->setSourceEndpoint(srcPt322);
    ConnEnd dstPt322(Point(383.984, 1106.01), 8);
    connRef322->setDestEndpoint(dstPt322);
    connRef322->setRoutingType((ConnType)2);

    ConnRef *connRef323 = new ConnRef(router, 323);
    ConnEnd srcPt323(Point(-8.01599, 132.608), 15);
    connRef323->setSourceEndpoint(srcPt323);
    ConnEnd dstPt323(Point(296.984, 1091.01), 1);
    connRef323->setDestEndpoint(dstPt323);
    connRef323->setRoutingType((ConnType)2);

    ConnRef *connRef213 = new ConnRef(router, 213);
    ConnEnd srcPt213(Point(798.746, 249.608), 15);
    connRef213->setSourceEndpoint(srcPt213);
    ConnEnd dstPt213(Point(798.746, 335.608), 15);
    connRef213->setDestEndpoint(dstPt213);
    connRef213->setRoutingType((ConnType)2);

    ConnRef *connRef214 = new ConnRef(router, 214);
    ConnEnd srcPt214(Point(317.084, 393.608), 15);
    connRef214->setSourceEndpoint(srcPt214);
    ConnEnd dstPt214(Point(317.084, 361.608), 15);
    connRef214->setDestEndpoint(dstPt214);
    connRef214->setRoutingType((ConnType)2);

    ConnRef *connRef243 = new ConnRef(router, 243);
    ConnEnd srcPt243(Point(749.984, 520.608), 15);
    connRef243->setSourceEndpoint(srcPt243);
    ConnEnd dstPt243(Point(793.984, 683.608), 15);
    connRef243->setDestEndpoint(dstPt243);
    connRef243->setRoutingType((ConnType)2);

    ConnRef *connRef244 = new ConnRef(router, 244);
    ConnEnd srcPt244(Point(372.984, 683.608), 15);
    connRef244->setSourceEndpoint(srcPt244);
    ConnEnd dstPt244(Point(422.984, 683.608), 15);
    connRef244->setDestEndpoint(dstPt244);
    connRef244->setRoutingType((ConnType)2);

    ConnRef *connRef272 = new ConnRef(router, 272);
    ConnEnd srcPt272(Point(681.984, 991.008), 15);
    connRef272->setSourceEndpoint(srcPt272);
    ConnEnd dstPt272(Point(681.984, 1041.01), 15);
    connRef272->setDestEndpoint(dstPt272);
    connRef272->setRoutingType((ConnType)2);

    ConnRef *connRef210 = new ConnRef(router, 210);
    ConnEnd srcPt210(Point(522.984, 690.465), 15);
    connRef210->setSourceEndpoint(srcPt210);
    ConnEnd dstPt210(Point(422.984, 683.608), 15);
    connRef210->setDestEndpoint(dstPt210);
    connRef210->setRoutingType((ConnType)2);

    ConnRef *connRef211 = new ConnRef(router, 211);
    ConnEnd srcPt211(Point(352.984, 683.608), 15);
    connRef211->setSourceEndpoint(srcPt211);
    ConnEnd dstPt211(Point(300.984, 683.608), 15);
    connRef211->setDestEndpoint(dstPt211);
    connRef211->setRoutingType((ConnType)2);

    ConnRef *connRef245 = new ConnRef(router, 245);
    ConnEnd srcPt245(Point(300.984, 683.608), 15);
    connRef245->setSourceEndpoint(srcPt245);
    ConnEnd dstPt245(Point(172.984, 694.465), 15);
    connRef245->setDestEndpoint(dstPt245);
    connRef245->setRoutingType((ConnType)2);

    ConnRef *connRef178 = new ConnRef(router, 178);
    ConnEnd srcPt178(Point(390.584, 236.608), 15);
    connRef178->setSourceEndpoint(srcPt178);
    ConnEnd dstPt178(Point(440.584, 236.608), 15);
    connRef178->setDestEndpoint(dstPt178);
    connRef178->setRoutingType((ConnType)2);

    ConnRef *connRef179 = new ConnRef(router, 179);
    ConnEnd srcPt179(Point(521.534, 236.608), 15);
    connRef179->setSourceEndpoint(srcPt179);
    ConnEnd dstPt179(Point(571.534, 184.608), 15);
    connRef179->setDestEndpoint(dstPt179);
    connRef179->setRoutingType((ConnType)2);

    ConnRef *connRef194 = new ConnRef(router, 194);
    ConnEnd srcPt194(Point(691.984, 949.008), 15);
    connRef194->setSourceEndpoint(srcPt194);
    ConnEnd dstPt194(Point(793.984, 949.008), 15);
    connRef194->setDestEndpoint(dstPt194);
    connRef194->setRoutingType((ConnType)2);

    ConnRef *connRef296 = new ConnRef(router, 296);
    ConnEnd srcPt296(Point(502.584, -45.3921), 15);
    connRef296->setSourceEndpoint(srcPt296);
    ConnEnd dstPt296(Point(440.584, -45.3921), 15);
    connRef296->setDestEndpoint(dstPt296);
    connRef296->setRoutingType((ConnType)2);

    ConnRef *connRef230 = new ConnRef(router, 230);
    ConnEnd srcPt230(Point(440.584, -35.3921), 15);
    connRef230->setSourceEndpoint(srcPt230);
    ConnEnd dstPt230(Point(440.584, -3.39214), 15);
    connRef230->setDestEndpoint(dstPt230);
    connRef230->setRoutingType((ConnType)2);

    ConnRef *connRef246 = new ConnRef(router, 246);
    ConnEnd srcPt246(Point(409.484, 1051.01), 15);
    connRef246->setSourceEndpoint(srcPt246);
    ConnEnd dstPt246(Point(551.984, 1051.01), 15);
    connRef246->setDestEndpoint(dstPt246);
    connRef246->setRoutingType((ConnType)2);

    ConnRef *connRef247 = new ConnRef(router, 247);
    ConnEnd srcPt247(Point(691.984, 1133.01), 15);
    connRef247->setSourceEndpoint(srcPt247);
    ConnEnd dstPt247(Point(793.984, 1051.01), 15);
    connRef247->setDestEndpoint(dstPt247);
    connRef247->setRoutingType((ConnType)2);

    ConnRef *connRef269 = new ConnRef(router, 269);
    ConnEnd srcPt269(Point(923.984, 899.008), 15);
    connRef269->setSourceEndpoint(srcPt269);
    ConnEnd dstPt269(Point(923.984, 949.008), 15);
    connRef269->setDestEndpoint(dstPt269);
    connRef269->setRoutingType((ConnType)2);

    ConnRef *connRef240 = new ConnRef(router, 240);
    ConnEnd srcPt240(Point(532.984, 520.608), 15);
    connRef240->setSourceEndpoint(srcPt240);
    ConnEnd dstPt240(Point(464.984, 520.608), 15);
    connRef240->setDestEndpoint(dstPt240);
    connRef240->setRoutingType((ConnType)2);

    ConnRef *connRef241 = new ConnRef(router, 241);
    ConnEnd srcPt241(Point(131.984, 132.608), 15);
    connRef241->setSourceEndpoint(srcPt241);
    ConnEnd dstPt241(Point(287.984, 132.608), 15);
    connRef241->setDestEndpoint(dstPt241);
    connRef241->setRoutingType((ConnType)2);

    ConnRef *connRef201 = new ConnRef(router, 201);
    ConnEnd srcPt201(Point(793.984, 745.608), 15);
    connRef201->setSourceEndpoint(srcPt201);
    ConnEnd dstPt201(Point(793.984, 837.608), 15);
    connRef201->setDestEndpoint(dstPt201);
    connRef201->setRoutingType((ConnType)2);

    ConnRef *connRef202 = new ConnRef(router, 202);
    ConnEnd srcPt202(Point(681.984, 725.608), 15);
    connRef202->setSourceEndpoint(srcPt202);
    ConnEnd dstPt202(Point(793.984, 683.608), 15);
    connRef202->setDestEndpoint(dstPt202);
    connRef202->setRoutingType((ConnType)2);

    ConnRef *connRef324 = new ConnRef(router, 324);
    ConnEnd srcPt324(Point(306.584, 1283.01), 15);
    connRef324->setSourceEndpoint(srcPt324);
    ConnEnd dstPt324(Point(296.984, 1213.01), 2);
    connRef324->setDestEndpoint(dstPt324);
    connRef324->setRoutingType((ConnType)2);

    ConnRef *connRef182 = new ConnRef(router, 182);
    ConnEnd srcPt182(Point(418.984, 1283.01), 15);
    connRef182->setSourceEndpoint(srcPt182);
    ConnEnd dstPt182(Point(306.584, 1283.01), 15);
    connRef182->setDestEndpoint(dstPt182);
    connRef182->setRoutingType((ConnType)2);

    ConnRef *connRef183 = new ConnRef(router, 183);
    ConnEnd srcPt183(Point(556.484, 1213.01), 15);
    connRef183->setSourceEndpoint(srcPt183);
    ConnEnd dstPt183(Point(551.984, 1051.01), 15);
    connRef183->setDestEndpoint(dstPt183);
    connRef183->setRoutingType((ConnType)2);

    ConnRef *connRef190 = new ConnRef(router, 190);
    ConnEnd srcPt190(Point(671.984, 1041.01), 15);
    connRef190->setSourceEndpoint(srcPt190);
    ConnEnd dstPt190(Point(551.984, 1051.01), 15);
    connRef190->setDestEndpoint(dstPt190);
    connRef190->setRoutingType((ConnType)2);

    ConnRef *connRef191 = new ConnRef(router, 191);
    ConnEnd srcPt191(Point(551.984, 981.008), 15);
    connRef191->setSourceEndpoint(srcPt191);
    ConnEnd dstPt191(Point(551.984, 949.008), 15);
    connRef191->setDestEndpoint(dstPt191);
    connRef191->setRoutingType((ConnType)2);

    ConnRef *connRef285 = new ConnRef(router, 285);
    ConnEnd srcPt285(Point(379.084, 403.608), 15);
    connRef285->setSourceEndpoint(srcPt285);
    ConnEnd dstPt285(Point(317.084, 403.608), 15);
    connRef285->setDestEndpoint(dstPt285);
    connRef285->setRoutingType((ConnType)2);

    ConnRef *connRef286 = new ConnRef(router, 286);
    ConnEnd srcPt286(Point(53.984, 373.608), 15);
    connRef286->setSourceEndpoint(srcPt286);
    ConnEnd dstPt286(Point(-8.01599, 373.608), 15);
    connRef286->setDestEndpoint(dstPt286);
    connRef286->setRoutingType((ConnType)2);

    ConnRef *connRef291 = new ConnRef(router, 291);
    ConnEnd srcPt291(Point(362.984, 82.6079), 15);
    connRef291->setSourceEndpoint(srcPt291);
    ConnEnd dstPt291(Point(362.984, 132.608), 15);
    connRef291->setDestEndpoint(dstPt291);
    connRef291->setRoutingType((ConnType)2);

    ConnRef *connRef292 = new ConnRef(router, 292);
    ConnEnd srcPt292(Point(121.984, 82.6079), 15);
    connRef292->setSourceEndpoint(srcPt292);
    ConnEnd dstPt292(Point(121.984, 132.608), 15);
    connRef292->setDestEndpoint(dstPt292);
    connRef292->setRoutingType((ConnType)2);

    ConnRef *connRef325 = new ConnRef(router, 325);
    ConnEnd srcPt325(Point(317.084, 361.608), 15);
    connRef325->setSourceEndpoint(srcPt325);
    ConnEnd dstPt325(Point(280.084, 321.608), 8);
    connRef325->setDestEndpoint(dstPt325);
    connRef325->setRoutingType((ConnType)2);

    ConnRef *connRef326 = new ConnRef(router, 326);
    ConnEnd srcPt326(Point(571.534, 184.608), 15);
    connRef326->setSourceEndpoint(srcPt326);
    ConnEnd dstPt326(Point(982.984, 283.5), 4);
    connRef326->setDestEndpoint(dstPt326);
    connRef326->setRoutingType((ConnType)2);

    ConnRef *connRef219 = new ConnRef(router, 219);
    ConnEnd srcPt219(Point(317.084, 329.608), 15);
    connRef219->setSourceEndpoint(srcPt219);
    ConnEnd dstPt219(Point(317.084, 361.608), 15);
    connRef219->setDestEndpoint(dstPt219);
    connRef219->setRoutingType((ConnType)2);

    ConnRef *connRef220 = new ConnRef(router, 220);
    ConnEnd srcPt220(Point(-8.01599, 217.608), 15);
    connRef220->setSourceEndpoint(srcPt220);
    ConnEnd dstPt220(Point(-8.01599, 249.608), 15);
    connRef220->setDestEndpoint(dstPt220);
    connRef220->setRoutingType((ConnType)2);

    ConnRef *connRef224 = new ConnRef(router, 224);
    ConnEnd srcPt224(Point(372.984, 132.608), 15);
    connRef224->setSourceEndpoint(srcPt224);
    ConnEnd dstPt224(Point(440.584, 132.608), 15);
    connRef224->setDestEndpoint(dstPt224);
    connRef224->setRoutingType((ConnType)2);

    ConnRef *connRef173 = new ConnRef(router, 173);
    ConnEnd srcPt173(Point(511.534, -149.392), 15);
    connRef173->setSourceEndpoint(srcPt173);
    ConnEnd dstPt173(Point(440.584, -3.39214), 15);
    connRef173->setDestEndpoint(dstPt173);
    connRef173->setRoutingType((ConnType)2);

    ConnRef *connRef260 = new ConnRef(router, 260);
    ConnEnd srcPt260(Point(552.984, 520.608), 15);
    connRef260->setSourceEndpoint(srcPt260);
    ConnEnd dstPt260(Point(602.984, 520.608), 15);
    connRef260->setDestEndpoint(dstPt260);
    connRef260->setRoutingType((ConnType)2);

    ConnRef *connRef197 = new ConnRef(router, 197);
    ConnEnd srcPt197(Point(472.984, 837.608), 15);
    connRef197->setSourceEndpoint(srcPt197);
    ConnEnd dstPt197(Point(422.984, 837.608), 15);
    connRef197->setDestEndpoint(dstPt197);
    connRef197->setRoutingType((ConnType)2);

    ConnRef *connRef206 = new ConnRef(router, 206);
    ConnEnd srcPt206(Point(300.984, 725.608), 15);
    connRef206->setSourceEndpoint(srcPt206);
    ConnEnd dstPt206(Point(300.984, 683.608), 15);
    connRef206->setDestEndpoint(dstPt206);
    connRef206->setRoutingType((ConnType)2);

    ConnRef *connRef255 = new ConnRef(router, 255);
    ConnEnd srcPt255(Point(440.584, -456.392), 15);
    connRef255->setSourceEndpoint(srcPt255);
    ConnEnd dstPt255(Point(440.584, -424.392), 15);
    connRef255->setDestEndpoint(dstPt255);
    connRef255->setRoutingType((ConnType)2);

    ConnRef *connRef256 = new ConnRef(router, 256);
    ConnEnd srcPt256(Point(440.584, -314.392), 15);
    connRef256->setSourceEndpoint(srcPt256);
    ConnEnd dstPt256(Point(362.984, -405.392), 15);
    connRef256->setDestEndpoint(dstPt256);
    connRef256->setRoutingType((ConnType)2);

    ConnRef *connRef225 = new ConnRef(router, 225);
    ConnEnd srcPt225(Point(352.984, 132.608), 15);
    connRef225->setSourceEndpoint(srcPt225);
    ConnEnd dstPt225(Point(287.984, 132.608), 15);
    connRef225->setDestEndpoint(dstPt225);
    connRef225->setRoutingType((ConnType)2);

    ConnRef *connRef226 = new ConnRef(router, 226);
    ConnEnd srcPt226(Point(111.984, 132.608), 15);
    connRef226->setSourceEndpoint(srcPt226);
    ConnEnd dstPt226(Point(-8.01599, 132.608), 15);
    connRef226->setDestEndpoint(dstPt226);
    connRef226->setRoutingType((ConnType)2);

    ConnRef *connRef236 = new ConnRef(router, 236);
    ConnEnd srcPt236(Point(-76.016, 38.6079), 15);
    connRef236->setSourceEndpoint(srcPt236);
    ConnEnd dstPt236(Point(-101.016, -159.392), 15);
    connRef236->setDestEndpoint(dstPt236);
    connRef236->setRoutingType((ConnType)2);

    ConnRef *connRef184 = new ConnRef(router, 184);
    ConnEnd srcPt184(Point(1065.98, 769.608), 15);
    connRef184->setSourceEndpoint(srcPt184);
    ConnEnd dstPt184(Point(1065.98, 903.608), 15);
    connRef184->setDestEndpoint(dstPt184);
    connRef184->setRoutingType((ConnType)2);

    ConnRef *connRef185 = new ConnRef(router, 185);
    ConnEnd srcPt185(Point(1065.98, 749.608), 15);
    connRef185->setSourceEndpoint(srcPt185);
    ConnEnd dstPt185(Point(1065.98, 683.608), 15);
    connRef185->setDestEndpoint(dstPt185);
    connRef185->setRoutingType((ConnType)2);

    ConnRef *connRef212 = new ConnRef(router, 212);
    ConnEnd srcPt212(Point(798.746, 229.608), 15);
    connRef212->setSourceEndpoint(srcPt212);
    ConnEnd dstPt212(Point(798.746, 184.608), 15);
    connRef212->setDestEndpoint(dstPt212);
    connRef212->setRoutingType((ConnType)2);

    ConnRef *connRef195 = new ConnRef(router, 195);
    ConnEnd srcPt195(Point(489, 936.608), 15);
    connRef195->setSourceEndpoint(srcPt195);
    ConnEnd dstPt195(Point(551.984, 1051.01), 15);
    connRef195->setDestEndpoint(dstPt195);
    connRef195->setRoutingType((ConnType)2);

    ConnRef *connRef196 = new ConnRef(router, 196);
    ConnEnd srcPt196(Point(793.984, 869.608), 15);
    connRef196->setSourceEndpoint(srcPt196);
    ConnEnd dstPt196(Point(793.984, 837.608), 15);
    connRef196->setDestEndpoint(dstPt196);
    connRef196->setRoutingType((ConnType)2);

    ConnRef *connRef294 = new ConnRef(router, 294);
    ConnEnd srcPt294(Point(362.984, -53.3921), 15);
    connRef294->setSourceEndpoint(srcPt294);
    ConnEnd dstPt294(Point(362.984, -3.39214), 15);
    connRef294->setDestEndpoint(dstPt294);
    connRef294->setRoutingType((ConnType)2);

    ConnRef *connRef295 = new ConnRef(router, 295);
    ConnEnd srcPt295(Point(121.984, -53.3921), 15);
    connRef295->setSourceEndpoint(srcPt295);
    ConnEnd dstPt295(Point(121.984, -3.39214), 15);
    connRef295->setDestEndpoint(dstPt295);
    connRef295->setRoutingType((ConnType)2);

    ConnRef *connRef254 = new ConnRef(router, 254);
    ConnEnd srcPt254(Point(798.746, 335.608), 15);
    connRef254->setSourceEndpoint(srcPt254);
    ConnEnd dstPt254(Point(945.984, 335.608), 15);
    connRef254->setDestEndpoint(dstPt254);
    connRef254->setRoutingType((ConnType)2);

    ConnRef *connRef264 = new ConnRef(router, 264);
    ConnEnd srcPt264(Point(858.746, 379.608), 15);
    connRef264->setSourceEndpoint(srcPt264);
    ConnEnd dstPt264(Point(858.746, 429.608), 15);
    connRef264->setDestEndpoint(dstPt264);
    connRef264->setRoutingType((ConnType)2);

    ConnRef *connRef265 = new ConnRef(router, 265);
    ConnEnd srcPt265(Point(511.534, 186.608), 15);
    connRef265->setSourceEndpoint(srcPt265);
    ConnEnd dstPt265(Point(511.534, 236.608), 15);
    connRef265->setDestEndpoint(dstPt265);
    connRef265->setRoutingType((ConnType)2);

    ConnRef *connRef266 = new ConnRef(router, 266);
    ConnEnd srcPt266(Point(428.984, 1233.01), 15);
    connRef266->setSourceEndpoint(srcPt266);
    ConnEnd dstPt266(Point(428.984, 1283.01), 15);
    connRef266->setDestEndpoint(dstPt266);
    connRef266->setRoutingType((ConnType)2);

    ConnRef *connRef290 = new ConnRef(router, 290);
    ConnEnd srcPt290(Point(511.534, 82.6079), 15);
    connRef290->setSourceEndpoint(srcPt290);
    ConnEnd dstPt290(Point(511.534, 132.608), 15);
    connRef290->setDestEndpoint(dstPt290);
    connRef290->setRoutingType((ConnType)2);

    ConnRef *connRef270 = new ConnRef(router, 270);
    ConnEnd srcPt270(Point(815.984, 1155.01), 15);
    connRef270->setSourceEndpoint(srcPt270);
    ConnEnd dstPt270(Point(753.984, 1155.01), 15);
    connRef270->setDestEndpoint(dstPt270);
    connRef270->setRoutingType((ConnType)2);

    ConnRef *connRef271 = new ConnRef(router, 271);
    ConnEnd srcPt271(Point(681.984, 1083.01), 15);
    connRef271->setSourceEndpoint(srcPt271);
    ConnEnd dstPt271(Point(681.984, 1133.01), 15);
    connRef271->setDestEndpoint(dstPt271);
    connRef271->setRoutingType((ConnType)2);

    ConnRef *connRef301 = new ConnRef(router, 301);
    ConnEnd srcPt301(Point(69.984, -209.392), 15);
    connRef301->setSourceEndpoint(srcPt301);
    ConnEnd dstPt301(Point(69.984, -159.392), 15);
    connRef301->setDestEndpoint(dstPt301);
    connRef301->setRoutingType((ConnType)2);

    ConnRef *connRef228 = new ConnRef(router, 228);
    ConnEnd srcPt228(Point(352.984, -3.39214), 15);
    connRef228->setSourceEndpoint(srcPt228);
    ConnEnd dstPt228(Point(287.984, -3.39214), 15);
    connRef228->setDestEndpoint(dstPt228);
    connRef228->setRoutingType((ConnType)2);

    ConnRef *connRef229 = new ConnRef(router, 229);
    ConnEnd srcPt229(Point(111.984, -3.39214), 15);
    connRef229->setSourceEndpoint(srcPt229);
    ConnEnd dstPt229(Point(-8.01599, -3.39214), 15);
    connRef229->setDestEndpoint(dstPt229);
    connRef229->setRoutingType((ConnType)2);

    ConnRef *connRef237 = new ConnRef(router, 237);
    ConnEnd srcPt237(Point(-76.016, 58.6079), 15);
    connRef237->setSourceEndpoint(srcPt237);
    ConnEnd dstPt237(Point(-8.01599, 132.608), 15);
    connRef237->setDestEndpoint(dstPt237);
    connRef237->setRoutingType((ConnType)2);

    ConnRef *connRef238 = new ConnRef(router, 238);
    ConnEnd srcPt238(Point(729.984, 520.608), 15);
    connRef238->setSourceEndpoint(srcPt238);
    ConnEnd dstPt238(Point(602.984, 520.608), 15);
    connRef238->setDestEndpoint(dstPt238);
    connRef238->setRoutingType((ConnType)2);

    ConnRef *connRef227 = new ConnRef(router, 227);
    ConnEnd srcPt227(Point(-8.01599, 100.608), 15);
    connRef227->setSourceEndpoint(srcPt227);
    ConnEnd dstPt227(Point(-8.01599, 132.608), 15);
    connRef227->setDestEndpoint(dstPt227);
    connRef227->setRoutingType((ConnType)2);

    ConnRef *connRef281 = new ConnRef(router, 281);
    ConnEnd srcPt281(Point(362.984, 735.608), 15);
    connRef281->setSourceEndpoint(srcPt281);
    ConnEnd dstPt281(Point(300.984, 735.608), 15);
    connRef281->setDestEndpoint(dstPt281);
    connRef281->setRoutingType((ConnType)2);

    ConnRef *connRef221 = new ConnRef(router, 221);
    ConnEnd srcPt221(Point(-8.01599, 197.608), 15);
    connRef221->setSourceEndpoint(srcPt221);
    ConnEnd dstPt221(Point(-8.01599, 132.608), 15);
    connRef221->setDestEndpoint(dstPt221);
    connRef221->setRoutingType((ConnType)2);

    ConnRef *connRef304 = new ConnRef(router, 304);
    ConnEnd srcPt304(Point(542.984, 470.608), 15);
    connRef304->setSourceEndpoint(srcPt304);
    ConnEnd dstPt304(Point(542.984, 520.608), 15);
    connRef304->setDestEndpoint(dstPt304);
    connRef304->setRoutingType((ConnType)2);

    ConnRef *connRef305 = new ConnRef(router, 305);
    ConnEnd srcPt305(Point(362.984, -353.392), 15);
    connRef305->setSourceEndpoint(srcPt305);
    ConnEnd dstPt305(Point(362.984, -303.392), 15);
    connRef305->setDestEndpoint(dstPt305);
    connRef305->setRoutingType((ConnType)2);

    ConnRef *connRef319 = new ConnRef(router, 319);
    ConnEnd srcPt319(Point(571.534, 184.608), 15);
    connRef319->setSourceEndpoint(srcPt319);
    ConnEnd dstPt319(Point(730.5, 44.6079), 4);
    connRef319->setDestEndpoint(dstPt319);
    connRef319->setRoutingType((ConnType)2);

    ConnRef *connRef320 = new ConnRef(router, 320);
    ConnEnd srcPt320(Point(798.746, 184.608), 15);
    connRef320->setSourceEndpoint(srcPt320);
    ConnEnd dstPt320(Point(730.5, 96.6079), 4);
    connRef320->setDestEndpoint(dstPt320);
    connRef320->setRoutingType((ConnType)2);

    ConnRef *connRef251 = new ConnRef(router, 251);
    ConnEnd srcPt251(Point(793.984, 889.608), 15);
    connRef251->setSourceEndpoint(srcPt251);
    ConnEnd dstPt251(Point(793.984, 949.008), 15);
    connRef251->setDestEndpoint(dstPt251);
    connRef251->setRoutingType((ConnType)2);

    ConnRef *connRef261 = new ConnRef(router, 261);
    ConnEnd srcPt261(Point(152.984, 694.465), 15);
    connRef261->setSourceEndpoint(srcPt261);
    ConnEnd dstPt261(Point(102.984, 683.608), 15);
    connRef261->setDestEndpoint(dstPt261);
    connRef261->setRoutingType((ConnType)2);

    ConnRef *connRef262 = new ConnRef(router, 262);
    ConnEnd srcPt262(Point(511.534, -209.392), 15);
    connRef262->setSourceEndpoint(srcPt262);
    ConnEnd dstPt262(Point(511.534, -159.392), 15);
    connRef262->setDestEndpoint(dstPt262);
    connRef262->setRoutingType((ConnType)2);

    ConnRef *connRef257 = new ConnRef(router, 257);
    ConnEnd srcPt257(Point(440.584, -314.392), 15);
    connRef257->setSourceEndpoint(srcPt257);
    ConnEnd dstPt257(Point(372.984, -303.392), 15);
    connRef257->setDestEndpoint(dstPt257);
    connRef257->setRoutingType((ConnType)2);

    ConnRef *connRef233 = new ConnRef(router, 233);
    ConnEnd srcPt233(Point(362.984, -385.392), 15);
    connRef233->setSourceEndpoint(srcPt233);
    ConnEnd dstPt233(Point(287.984, -314.392), 15);
    connRef233->setDestEndpoint(dstPt233);
    connRef233->setRoutingType((ConnType)2);

    ConnRef *connRef263 = new ConnRef(router, 263);
    ConnEnd srcPt263(Point(996.984, 526.108), 15);
    connRef263->setSourceEndpoint(srcPt263);
    ConnEnd dstPt263(Point(934.984, 526.108), 15);
    connRef263->setDestEndpoint(dstPt263);
    connRef263->setRoutingType((ConnType)2);

    ConnRef *connRef287 = new ConnRef(router, 287);
    ConnEnd srcPt287(Point(380.584, 186.608), 15);
    connRef287->setSourceEndpoint(srcPt287);
    ConnEnd dstPt287(Point(380.584, 236.608), 15);
    connRef287->setDestEndpoint(dstPt287);
    connRef287->setRoutingType((ConnType)2);

    ConnRef *connRef218 = new ConnRef(router, 218);
    ConnEnd srcPt218(Point(317.084, 309.608), 15);
    connRef218->setSourceEndpoint(srcPt218);
    ConnEnd dstPt218(Point(320.584, 184.608), 15);
    connRef218->setDestEndpoint(dstPt218);
    connRef218->setRoutingType((ConnType)2);

    ConnRef *connRef267 = new ConnRef(router, 267);
    ConnEnd srcPt267(Point(618.484, 1223.01), 15);
    connRef267->setSourceEndpoint(srcPt267);
    ConnEnd dstPt267(Point(556.484, 1223.01), 15);
    connRef267->setDestEndpoint(dstPt267);
    connRef267->setRoutingType((ConnType)2);

    ConnRef *connRef268 = new ConnRef(router, 268);
    ConnEnd srcPt268(Point(1127.98, 759.608), 15);
    connRef268->setSourceEndpoint(srcPt268);
    ConnEnd dstPt268(Point(1065.98, 759.608), 15);
    connRef268->setDestEndpoint(dstPt268);
    connRef268->setRoutingType((ConnType)2);

    ConnRef *connRef278 = new ConnRef(router, 278);
    ConnEnd srcPt278(Point(855.984, 735.608), 15);
    connRef278->setSourceEndpoint(srcPt278);
    ConnEnd dstPt278(Point(793.984, 735.608), 15);
    connRef278->setDestEndpoint(dstPt278);
    connRef278->setRoutingType((ConnType)2);

    ConnRef *connRef299 = new ConnRef(router, 299);
    ConnEnd srcPt299(Point(362.984, -445.392), 15);
    connRef299->setSourceEndpoint(srcPt299);
    ConnEnd dstPt299(Point(362.984, -395.392), 15);
    connRef299->setDestEndpoint(dstPt299);
    connRef299->setRoutingType((ConnType)2);

    ConnRef *connRef300 = new ConnRef(router, 300);
    ConnEnd srcPt300(Point(349.984, -261.392), 15);
    connRef300->setSourceEndpoint(srcPt300);
    ConnEnd dstPt300(Point(287.984, -261.392), 15);
    connRef300->setDestEndpoint(dstPt300);
    connRef300->setRoutingType((ConnType)2);

    ConnRef *connRef302 = new ConnRef(router, 302);
    ConnEnd srcPt302(Point(-14.016, 48.6079), 15);
    connRef302->setSourceEndpoint(srcPt302);
    ConnEnd dstPt302(Point(-76.016, 48.6079), 15);
    connRef302->setDestEndpoint(dstPt302);
    connRef302->setRoutingType((ConnType)2);

    ConnRef *connRef303 = new ConnRef(router, 303);
    ConnEnd srcPt303(Point(739.984, 470.608), 15);
    connRef303->setSourceEndpoint(srcPt303);
    ConnEnd dstPt303(Point(739.984, 520.608), 15);
    connRef303->setDestEndpoint(dstPt303);
    connRef303->setRoutingType((ConnType)2);

    ConnRef *connRef204 = new ConnRef(router, 204);
    ConnEnd srcPt204(Point(422.984, 725.608), 15);
    connRef204->setSourceEndpoint(srcPt204);
    ConnEnd dstPt204(Point(422.984, 683.608), 15);
    connRef204->setDestEndpoint(dstPt204);
    connRef204->setRoutingType((ConnType)2);

    ConnRef *connRef205 = new ConnRef(router, 205);
    ConnEnd srcPt205(Point(422.984, 745.608), 15);
    connRef205->setSourceEndpoint(srcPt205);
    ConnEnd dstPt205(Point(422.984, 837.608), 15);
    connRef205->setDestEndpoint(dstPt205);
    connRef205->setRoutingType((ConnType)2);

    ConnRef *connRef207 = new ConnRef(router, 207);
    ConnEnd srcPt207(Point(300.984, 745.608), 15);
    connRef207->setSourceEndpoint(srcPt207);
    ConnEnd dstPt207(Point(422.984, 837.608), 15);
    connRef207->setDestEndpoint(dstPt207);
    connRef207->setRoutingType((ConnType)2);

    ConnRef *connRef208 = new ConnRef(router, 208);
    ConnEnd srcPt208(Point(824, 569), 15);
    connRef208->setSourceEndpoint(srcPt208);
    ConnEnd dstPt208(Point(1065.98, 683.608), 15);
    connRef208->setDestEndpoint(dstPt208);
    connRef208->setRoutingType((ConnType)2);

    ConnRef *connRef258 = new ConnRef(router, 258);
    ConnEnd srcPt258(Point(352.984, -303.392), 15);
    connRef258->setSourceEndpoint(srcPt258);
    ConnEnd dstPt258(Point(287.984, -314.392), 15);
    connRef258->setDestEndpoint(dstPt258);
    connRef258->setRoutingType((ConnType)2);

    ConnRef *connRef259 = new ConnRef(router, 259);
    ConnEnd srcPt259(Point(79.984, -159.392), 15);
    connRef259->setSourceEndpoint(srcPt259);
    ConnEnd dstPt259(Point(287.984, -159.392), 15);
    connRef259->setDestEndpoint(dstPt259);
    connRef259->setRoutingType((ConnType)2);

    ConnRef *connRef200 = new ConnRef(router, 200);
    ConnEnd srcPt200(Point(793.984, 725.608), 15);
    connRef200->setSourceEndpoint(srcPt200);
    ConnEnd dstPt200(Point(793.984, 683.608), 15);
    connRef200->setDestEndpoint(dstPt200);
    connRef200->setRoutingType((ConnType)2);

    ConnRef *connRef231 = new ConnRef(router, 231);
    ConnEnd srcPt231(Point(440.584, -251.392), 15);
    connRef231->setSourceEndpoint(srcPt231);
    ConnEnd dstPt231(Point(440.584, -159.392), 15);
    connRef231->setDestEndpoint(dstPt231);
    connRef231->setRoutingType((ConnType)2);

    ConnRef *connRef232 = new ConnRef(router, 232);
    ConnEnd srcPt232(Point(440.584, -404.392), 15);
    connRef232->setSourceEndpoint(srcPt232);
    ConnEnd dstPt232(Point(440.584, -314.392), 15);
    connRef232->setDestEndpoint(dstPt232);
    connRef232->setRoutingType((ConnType)2);

    ConnRef *connRef318 = new ConnRef(router, 318);
    ConnEnd srcPt318(Point(440.584, -3.39214), 15);
    connRef318->setSourceEndpoint(srcPt318);
    ConnEnd dstPt318(Point(730.5, -7.39214), 4);
    connRef318->setDestEndpoint(dstPt318);
    connRef318->setRoutingType((ConnType)2);

    ConnRef *connRef293 = new ConnRef(router, 293);
    ConnEnd srcPt293(Point(53.984, 90.6079), 15);
    connRef293->setSourceEndpoint(srcPt293);
    ConnEnd dstPt293(Point(-8.01599, 90.6079), 15);
    connRef293->setDestEndpoint(dstPt293);
    connRef293->setRoutingType((ConnType)2);

    ConnRef *connRef310 = new ConnRef(router, 310);
    ConnEnd srcPt310(Point(-8.01599, -3.39214), 15);
    connRef310->setSourceEndpoint(srcPt310);
    ConnEnd dstPt310(Point(-8.01599, 80.6079), 15);
    connRef310->setDestEndpoint(dstPt310);
    connRef310->setRoutingType((ConnType)2);

    ConnRef *connRef311 = new ConnRef(router, 311);
    ConnEnd srcPt311(Point(440.584, -3.39214), 15);
    connRef311->setSourceEndpoint(srcPt311);
    ConnEnd dstPt311(Point(372.984, -3.39214), 15);
    connRef311->setDestEndpoint(dstPt311);
    connRef311->setRoutingType((ConnType)2);

    ConnRef *connRef249 = new ConnRef(router, 249);
    ConnEnd srcPt249(Point(933.984, 949.008), 15);
    connRef249->setSourceEndpoint(srcPt249);
    ConnEnd dstPt249(Point(1065.98, 903.608), 15);
    connRef249->setDestEndpoint(dstPt249);
    connRef249->setRoutingType((ConnType)2);

    ConnRef *connRef250 = new ConnRef(router, 250);
    ConnEnd srcPt250(Point(959.984, 683.608), 15);
    connRef250->setSourceEndpoint(srcPt250);
    ConnEnd dstPt250(Point(1065.98, 683.608), 15);
    connRef250->setDestEndpoint(dstPt250);
    connRef250->setRoutingType((ConnType)2);

    ConnRef *connRef242 = new ConnRef(router, 242);
    ConnEnd srcPt242(Point(287.984, -3.39214), 15);
    connRef242->setSourceEndpoint(srcPt242);
    ConnEnd dstPt242(Point(131.984, -3.39214), 15);
    connRef242->setDestEndpoint(dstPt242);
    connRef242->setRoutingType((ConnType)2);

    ConnRef *connRef275 = new ConnRef(router, 275);
    ConnEnd srcPt275(Point(855.984, 879.608), 15);
    connRef275->setSourceEndpoint(srcPt275);
    ConnEnd dstPt275(Point(793.984, 879.608), 15);
    connRef275->setDestEndpoint(dstPt275);
    connRef275->setRoutingType((ConnType)2);

    ConnRef *connRef307 = new ConnRef(router, 307);
    ConnEnd srcPt307(Point(440.584, -314.392), 15);
    connRef307->setSourceEndpoint(srcPt307);
    ConnEnd dstPt307(Point(511.534, -169.392), 15);
    connRef307->setDestEndpoint(dstPt307);
    connRef307->setRoutingType((ConnType)2);

    ConnRef *connRef308 = new ConnRef(router, 308);
    ConnEnd srcPt308(Point(798.746, 335.608), 15);
    connRef308->setSourceEndpoint(srcPt308);
    ConnEnd dstPt308(Point(848.746, 429.608), 15);
    connRef308->setDestEndpoint(dstPt308);
    connRef308->setRoutingType((ConnType)2);

    ConnRef *connRef297 = new ConnRef(router, 297);
    ConnEnd srcPt297(Point(502.584, -261.392), 15);
    connRef297->setSourceEndpoint(srcPt297);
    ConnEnd dstPt297(Point(440.584, -261.392), 15);
    connRef297->setDestEndpoint(dstPt297);
    connRef297->setRoutingType((ConnType)2);

    ConnRef *connRef298 = new ConnRef(router, 298);
    ConnEnd srcPt298(Point(502.584, -414.392), 15);
    connRef298->setSourceEndpoint(srcPt298);
    ConnEnd dstPt298(Point(440.584, -414.392), 15);
    connRef298->setDestEndpoint(dstPt298);
    connRef298->setRoutingType((ConnType)2);

    ConnRef *connRef216 = new ConnRef(router, 216);
    ConnEnd srcPt216(Point(-8.01599, 383.608), 15);
    connRef216->setSourceEndpoint(srcPt216);
    ConnEnd dstPt216(Point(-8.01599, 445.608), 15);
    connRef216->setDestEndpoint(dstPt216);
    connRef216->setRoutingType((ConnType)2);

    ConnRef *connRef217 = new ConnRef(router, 217);
    ConnEnd srcPt217(Point(370.584, 236.608), 15);
    connRef217->setSourceEndpoint(srcPt217);
    ConnEnd dstPt217(Point(320.584, 184.608), 15);
    connRef217->setDestEndpoint(dstPt217);
    connRef217->setRoutingType((ConnType)2);

    ConnRef *connRef209 = new ConnRef(router, 209);
    ConnEnd srcPt209(Point(542.984, 690.465), 15);
    connRef209->setSourceEndpoint(srcPt209);
    ConnEnd dstPt209(Point(793.984, 683.608), 15);
    connRef209->setDestEndpoint(dstPt209);
    connRef209->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/overlappingRects");
    delete router;
    return 0;
};
