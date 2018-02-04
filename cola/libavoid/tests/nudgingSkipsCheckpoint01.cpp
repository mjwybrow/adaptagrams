// Regression test to catch problem where orthogonal segments from the same 
// connector were being merged with others going through checkpoints and 
// being simplified so as not to pass the checkpoint anymore.
// Based on ec00232.
#include "libavoid/libavoid.h"
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
    router->setRoutingParameter((RoutingParameter)6, 0);
    router->setRoutingParameter((RoutingParameter)7, 4);
    router->setRoutingOption((RoutingOption)0, true);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingOption((RoutingOption)2, false);
    router->setRoutingOption((RoutingOption)3, false);
    Polygon polygon;
    ConnRef *connRef = nullptr;
    ConnEnd srcPt;
    ConnEnd dstPt;
#if ALL
    // shapeRef1
    polygon = Polygon(4);
    polygon.ps[0] = Point(-80.18071812011561, 825.315092940984);
    polygon.ps[1] = Point(-80.18071812011561, 887.315092940984);
    polygon.ps[2] = Point(-142.1807181201156, 887.315092940984);
    polygon.ps[3] = Point(-142.1807181201156, 825.315092940984);
    new ShapeRef(router, polygon, 1);
#endif

    // shapeRef2
    polygon = Polygon(4);
    polygon.ps[0] = Point(620.1049961655988, -86.0182403953493);
    polygon.ps[1] = Point(620.1049961655988, -24.0182403953493);
    polygon.ps[2] = Point(558.1049961655988, -24.0182403953493);
    polygon.ps[3] = Point(558.1049961655988, -86.0182403953493);
    new ShapeRef(router, polygon, 2);

#if ALL
    // shapeRef3
    polygon = Polygon(4);
    polygon.ps[0] = Point(1044.504996167599, 719.315092940984);
    polygon.ps[1] = Point(1044.504996167599, 781.315092940984);
    polygon.ps[2] = Point(982.5049961675986, 781.315092940984);
    polygon.ps[3] = Point(982.5049961675986, 719.315092940984);
    new ShapeRef(router, polygon, 3);

    // shapeRef4
    polygon = Polygon(4);
    polygon.ps[0] = Point(1153.104996167599, 759.315092940984);
    polygon.ps[1] = Point(1153.104996167599, 821.315092940984);
    polygon.ps[2] = Point(1091.104996167599, 821.315092940984);
    polygon.ps[3] = Point(1091.104996167599, 759.315092940984);
    new ShapeRef(router, polygon, 4);

    // shapeRef5
    polygon = Polygon(4);
    polygon.ps[0] = Point(1044.504996167599, 495.9817596066507);
    polygon.ps[1] = Point(1044.504996167599, 557.9817596066507);
    polygon.ps[2] = Point(982.5049961675986, 557.9817596066507);
    polygon.ps[3] = Point(982.5049961675986, 495.9817596066507);
    new ShapeRef(router, polygon, 5);

    // shapeRef6
    polygon = Polygon(4);
    polygon.ps[0] = Point(968.1049961675988, 759.315092940984);
    polygon.ps[1] = Point(968.1049961675988, 821.315092940984);
    polygon.ps[2] = Point(906.1049961675988, 821.315092940984);
    polygon.ps[3] = Point(906.1049961675988, 759.315092940984);
    new ShapeRef(router, polygon, 6);

    // shapeRef7
    polygon = Polygon(4);
    polygon.ps[0] = Point(1044.504996167599, 792.315092940984);
    polygon.ps[1] = Point(1044.504996167599, 854.315092940984);
    polygon.ps[2] = Point(982.5049961675986, 854.315092940984);
    polygon.ps[3] = Point(982.5049961675986, 792.315092940984);
    new ShapeRef(router, polygon, 7);

    // shapeRef8
    polygon = Polygon(4);
    polygon.ps[0] = Point(349.8192818798844, 1008.815092941984);
    polygon.ps[1] = Point(349.8192818798844, 1070.815092941984);
    polygon.ps[2] = Point(287.8192818798844, 1070.815092941984);
    polygon.ps[3] = Point(287.8192818798844, 1008.815092941984);
    new ShapeRef(router, polygon, 8);

    // shapeRef9
    polygon = Polygon(4);
    polygon.ps[0] = Point(349.8192818798844, 617.315092940984);
    polygon.ps[1] = Point(349.8192818798844, 679.315092940984);
    polygon.ps[2] = Point(287.8192818798844, 679.315092940984);
    polygon.ps[3] = Point(287.8192818798844, 617.315092940984);
    new ShapeRef(router, polygon, 9);

    // shapeRef10
    polygon = Polygon(4);
    polygon.ps[0] = Point(288.8192818798844, 371.9817596066507);
    polygon.ps[1] = Point(288.8192818798844, 433.9817596066507);
    polygon.ps[2] = Point(226.8192818798844, 433.9817596066507);
    polygon.ps[3] = Point(226.8192818798844, 371.9817596066507);
    new ShapeRef(router, polygon, 10);
#endif

    // shapeRef11
    polygon = Polygon(4);
    polygon.ps[0] = Point(103.8192818798844, 975.8150929419839);
    polygon.ps[1] = Point(103.8192818798844, 995.8150929419839);
    polygon.ps[2] = Point(71.81928187988439, 995.8150929419839);
    polygon.ps[3] = Point(71.81928187988439, 975.8150929419839);
    new ShapeRef(router, polygon, 11);

    // shapeRef12
    polygon = Polygon(4);
    polygon.ps[0] = Point(103.8192818798844, 1017.815092941984);
    polygon.ps[1] = Point(103.8192818798844, 1037.815092941984);
    polygon.ps[2] = Point(71.81928187988439, 1037.815092941984);
    polygon.ps[3] = Point(71.81928187988439, 1017.815092941984);
    new ShapeRef(router, polygon, 12);

#if ALL
    // shapeRef13
    polygon = Polygon(4);
    polygon.ps[0] = Point(328.8192818798844, 957.8150929419839);
    polygon.ps[1] = Point(328.8192818798844, 989.8150929419839);
    polygon.ps[2] = Point(308.8192818798844, 989.8150929419839);
    polygon.ps[3] = Point(308.8192818798844, 957.8150929419839);
    new ShapeRef(router, polygon, 13);

    // shapeRef14
    polygon = Polygon(4);
    polygon.ps[0] = Point(12.81928187988439, 901.815092940984);
    polygon.ps[1] = Point(12.81928187988439, 921.815092940984);
    polygon.ps[2] = Point(-19.18071812011561, 921.815092940984);
    polygon.ps[3] = Point(-19.18071812011561, 901.815092940984);
    new ShapeRef(router, polygon, 14);

    // shapeRef15
    polygon = Polygon(4);
    polygon.ps[0] = Point(466.8192818798844, 840.315092940984);
    polygon.ps[1] = Point(466.8192818798844, 872.315092940984);
    polygon.ps[2] = Point(446.8192818798844, 872.315092940984);
    polygon.ps[3] = Point(446.8192818798844, 840.315092940984);
    new ShapeRef(router, polygon, 15);

    // shapeRef16
    polygon = Polygon(4);
    polygon.ps[0] = Point(1023.504996167599, 572.9817596076507);
    polygon.ps[1] = Point(1023.504996167599, 604.9817596076507);
    polygon.ps[2] = Point(1003.504996167599, 604.9817596076507);
    polygon.ps[3] = Point(1003.504996167599, 572.9817596076507);
    new ShapeRef(router, polygon, 16);

    // shapeRef17
    polygon = Polygon(4);
    polygon.ps[0] = Point(887.1049961675988, 638.315092940984);
    polygon.ps[1] = Point(887.1049961675988, 658.315092940984);
    polygon.ps[2] = Point(855.1049961675988, 658.315092940984);
    polygon.ps[3] = Point(855.1049961675988, 638.315092940984);
    new ShapeRef(router, polygon, 17);

    // shapeRef18
    polygon = Polygon(4);
    polygon.ps[0] = Point(472.8192818798844, 671.315092940984);
    polygon.ps[1] = Point(472.8192818798844, 691.315092940984);
    polygon.ps[2] = Point(440.8192818798844, 691.315092940984);
    polygon.ps[3] = Point(440.8192818798844, 671.315092940984);
    new ShapeRef(router, polygon, 18);

    // shapeRef19
    polygon = Polygon(4);
    polygon.ps[0] = Point(142.8192818798844, 638.315092940984);
    polygon.ps[1] = Point(142.8192818798844, 658.315092940984);
    polygon.ps[2] = Point(110.8192818798844, 658.315092940984);
    polygon.ps[3] = Point(110.8192818798844, 638.315092940984);
    new ShapeRef(router, polygon, 19);

    // shapeRef20
    polygon = Polygon(4);
    polygon.ps[0] = Point(599.1049961655988, 419.9817596066507);
    polygon.ps[1] = Point(599.1049961655988, 451.9817596066507);
    polygon.ps[2] = Point(579.1049961655988, 451.9817596066507);
    polygon.ps[3] = Point(579.1049961655988, 419.9817596066507);
    new ShapeRef(router, polygon, 20);

    // shapeRef21
    polygon = Polygon(4);
    polygon.ps[0] = Point(472.8192818798844, 483.9817596066507);
    polygon.ps[1] = Point(472.8192818798844, 503.9817596066507);
    polygon.ps[2] = Point(440.8192818798844, 503.9817596066507);
    polygon.ps[3] = Point(440.8192818798844, 483.9817596066507);
    new ShapeRef(router, polygon, 21);

    // shapeRef22
    polygon = Polygon(4);
    polygon.ps[0] = Point(472.8192818798844, 134.9817596056507);
    polygon.ps[1] = Point(472.8192818798844, 154.9817596056507);
    polygon.ps[2] = Point(440.8192818798844, 154.9817596056507);
    polygon.ps[3] = Point(440.8192818798844, 134.9817596056507);
    new ShapeRef(router, polygon, 22);

    // shapeRef23
    polygon = Polygon(4);
    polygon.ps[0] = Point(1023.504996167599, 419.9817596066507);
    polygon.ps[1] = Point(1023.504996167599, 451.9817596066507);
    polygon.ps[2] = Point(1003.504996167599, 451.9817596066507);
    polygon.ps[3] = Point(1003.504996167599, 419.9817596066507);
    new ShapeRef(router, polygon, 23);

    // shapeRef24
    polygon = Polygon(4);
    polygon.ps[0] = Point(751.1049961665988, 276.9817596056507);
    polygon.ps[1] = Point(751.1049961665988, 296.9817596056507);
    polygon.ps[2] = Point(719.1049961665988, 296.9817596056507);
    polygon.ps[3] = Point(719.1049961665988, 276.9817596056507);
    new ShapeRef(router, polygon, 24);

    // shapeRef25
    polygon = Polygon(4);
    polygon.ps[0] = Point(472.8192818798844, 209.9817596056507);
    polygon.ps[1] = Point(472.8192818798844, 229.9817596056507);
    polygon.ps[2] = Point(440.8192818798844, 229.9817596056507);
    polygon.ps[3] = Point(440.8192818798844, 209.9817596056507);
    new ShapeRef(router, polygon, 25);

    // shapeRef26
    polygon = Polygon(4);
    polygon.ps[0] = Point(751.1049961665988, 318.9817596056507);
    polygon.ps[1] = Point(751.1049961665988, 338.9817596056507);
    polygon.ps[2] = Point(719.1049961665988, 338.9817596056507);
    polygon.ps[3] = Point(719.1049961665988, 318.9817596056507);
    new ShapeRef(router, polygon, 26);

    // shapeRef27
    polygon = Polygon(4);
    polygon.ps[0] = Point(472.8192818798844, -181.0182403963493);
    polygon.ps[1] = Point(472.8192818798844, -161.0182403963493);
    polygon.ps[2] = Point(440.8192818798844, -161.0182403963493);
    polygon.ps[3] = Point(440.8192818798844, -181.0182403963493);
    new ShapeRef(router, polygon, 27);

    // shapeRef28
    polygon = Polygon(4);
    polygon.ps[0] = Point(599.1049961655988, -199.0182403963493);
    polygon.ps[1] = Point(599.1049961655988, -167.0182403963493);
    polygon.ps[2] = Point(579.1049961655988, -167.0182403963493);
    polygon.ps[3] = Point(579.1049961655988, -199.0182403963493);
    new ShapeRef(router, polygon, 28);

    // shapeRef29
    polygon = Polygon(4);
    polygon.ps[0] = Point(472.8192818798844, 251.9817596056507);
    polygon.ps[1] = Point(472.8192818798844, 271.9817596056507);
    polygon.ps[2] = Point(440.8192818798844, 271.9817596056507);
    polygon.ps[3] = Point(440.8192818798844, 251.9817596056507);
    new ShapeRef(router, polygon, 29);

    // shapeRef30
    polygon = Polygon(4);
    polygon.ps[0] = Point(142.8192818798844, -65.0182403953493);
    polygon.ps[1] = Point(142.8192818798844, -45.0182403953493);
    polygon.ps[2] = Point(110.8192818798844, -45.0182403953493);
    polygon.ps[3] = Point(110.8192818798844, -65.0182403953493);
    new ShapeRef(router, polygon, 30);

    // shapeRef31
    polygon = Polygon(4);
    polygon.ps[0] = Point(-20.18071812011561, -65.0182403953493);
    polygon.ps[1] = Point(-20.18071812011561, -45.0182403953493);
    polygon.ps[2] = Point(-52.18071812011561, -45.0182403953493);
    polygon.ps[3] = Point(-52.18071812011561, -65.0182403953493);
    new ShapeRef(router, polygon, 31);

    // shapeRef32
    polygon = Polygon(4);
    polygon.ps[0] = Point(267.8192818798844, 324.9817596056507);
    polygon.ps[1] = Point(267.8192818798844, 356.9817596056507);
    polygon.ps[2] = Point(247.8192818798844, 356.9817596056507);
    polygon.ps[3] = Point(247.8192818798844, 324.9817596056507);
    new ShapeRef(router, polygon, 32);

    // shapeRef33
    polygon = Polygon(4);
    polygon.ps[0] = Point(136.8192818798844, 258.9817596056507);
    polygon.ps[1] = Point(136.8192818798844, 290.9817596056507);
    polygon.ps[2] = Point(116.8192818798844, 290.9817596056507);
    polygon.ps[3] = Point(116.8192818798844, 258.9817596056507);
    new ShapeRef(router, polygon, 33);

    // shapeRef34
    polygon = Polygon(4);
    polygon.ps[0] = Point(472.8192818798844, -32.0182403953493);
    polygon.ps[1] = Point(472.8192818798844, -12.0182403953493);
    polygon.ps[2] = Point(440.8192818798844, -12.0182403953493);
    polygon.ps[3] = Point(440.8192818798844, -32.0182403953493);
    new ShapeRef(router, polygon, 34);

    // shapeRef35
    polygon = Polygon(4);
    polygon.ps[0] = Point(1001.104996167599, 276.9817596056507);
    polygon.ps[1] = Point(1001.104996167599, 338.9817596056507);
    polygon.ps[2] = Point(939.1049961675988, 338.9817596056507);
    polygon.ps[3] = Point(939.1049961675988, 276.9817596056507);
    new ShapeRef(router, polygon, 35);

    // shapeRef36
    polygon = Polygon(4);
    polygon.ps[0] = Point(854.1049961675988, 144.9817596056507);
    polygon.ps[1] = Point(854.1049961675988, 164.9817596056507);
    polygon.ps[2] = Point(822.1049961675988, 164.9817596056507);
    polygon.ps[3] = Point(822.1049961675988, 144.9817596056507);
    new ShapeRef(router, polygon, 36);

    // shapeRef37
    polygon = Polygon(4);
    polygon.ps[0] = Point(854.1049961675988, 186.9817596056507);
    polygon.ps[1] = Point(854.1049961675988, 206.9817596056507);
    polygon.ps[2] = Point(822.1049961675988, 206.9817596056507);
    polygon.ps[3] = Point(822.1049961675988, 186.9817596056507);
    new ShapeRef(router, polygon, 37);

    // shapeRef38
    polygon = Polygon(4);
    polygon.ps[0] = Point(472.8192818798844, -139.0182403963493);
    polygon.ps[1] = Point(472.8192818798844, -119.0182403963493);
    polygon.ps[2] = Point(440.8192818798844, -119.0182403963493);
    polygon.ps[3] = Point(440.8192818798844, -139.0182403963493);
    new ShapeRef(router, polygon, 38);

    // shapeRef39
    polygon = Polygon(4);
    polygon.ps[0] = Point(854.1049961675988, 69.98175960565069);
    polygon.ps[1] = Point(854.1049961675988, 89.98175960565069);
    polygon.ps[2] = Point(822.1049961675988, 89.98175960565069);
    polygon.ps[3] = Point(822.1049961675988, 69.98175960565069);
    new ShapeRef(router, polygon, 39);

    // shapeRef40
    polygon = Polygon(4);
    polygon.ps[0] = Point(-51.68071812011561, 455.4817596066507);
    polygon.ps[1] = Point(-51.68071812011561, 619.4817596066507);
    polygon.ps[2] = Point(-170.6807181201156, 619.4817596066507);
    polygon.ps[3] = Point(-170.6807181201156, 455.4817596066507);
    new ShapeRef(router, polygon, 40);

    // shapeRef41
    polygon = Polygon(4);
    polygon.ps[0] = Point(31.81928187988439, -317.5182403973492);
    polygon.ps[1] = Point(31.81928187988439, -282.5182403973492);
    polygon.ps[2] = Point(-164.1807181201156, -282.5182403973492);
    polygon.ps[3] = Point(-164.1807181201156, -317.5182403973492);
    new ShapeRef(router, polygon, 41);

    // shapeRef42
    polygon = Polygon(4);
    polygon.ps[0] = Point(752.8192818798843, 768.315092940984);
    polygon.ps[1] = Point(752.8192818798843, 812.315092940984);
    polygon.ps[2] = Point(664.8192818798843, 812.315092940984);
    polygon.ps[3] = Point(664.8192818798843, 768.315092940984);
    new ShapeRef(router, polygon, 42);

    // shapeRef43
    polygon = Polygon(4);
    polygon.ps[0] = Point(349.8192818798844, 880.815092940984);
    polygon.ps[1] = Point(349.8192818798844, 942.815092940984);
    polygon.ps[2] = Point(287.8192818798844, 942.815092940984);
    polygon.ps[3] = Point(287.8192818798844, 880.815092940984);
    new ShapeRef(router, polygon, 43);

    // shapeRef44
    polygon = Polygon(4);
    polygon.ps[0] = Point(349.8192818798844, -181.0182403963493);
    polygon.ps[1] = Point(349.8192818798844, -119.0182403963493);
    polygon.ps[2] = Point(287.8192818798844, -119.0182403963493);
    polygon.ps[3] = Point(287.8192818798844, -181.0182403963493);
    new ShapeRef(router, polygon, 44);

    // shapeRef45
    polygon = Polygon(4);
    polygon.ps[0] = Point(935.1049961675988, -276.0182403973492);
    polygon.ps[1] = Point(935.1049961675988, -214.0182403973492);
    polygon.ps[2] = Point(873.1049961675988, -214.0182403973492);
    polygon.ps[3] = Point(873.1049961675988, -276.0182403973492);
    new ShapeRef(router, polygon, 45);

    // shapeRef46
    polygon = Polygon(4);
    polygon.ps[0] = Point(620.1049961655988, 276.9817596056507);
    polygon.ps[1] = Point(620.1049961655988, 338.9817596056507);
    polygon.ps[2] = Point(558.1049961655988, 338.9817596056507);
    polygon.ps[3] = Point(558.1049961655988, 276.9817596056507);
    new ShapeRef(router, polygon, 46);

    // shapeRef47
    polygon = Polygon(4);
    polygon.ps[0] = Point(620.1049961655988, 617.315092940984);
    polygon.ps[1] = Point(620.1049961655988, 679.315092940984);
    polygon.ps[2] = Point(558.1049961655988, 679.315092940984);
    polygon.ps[3] = Point(558.1049961655988, 617.315092940984);
    new ShapeRef(router, polygon, 47);
#endif

    // shapeRef48
    polygon = Polygon(4);
    polygon.ps[0] = Point(78.81928187988439, -86.0182403953493);
    polygon.ps[1] = Point(78.81928187988439, -24.0182403953493);
    polygon.ps[2] = Point(16.81928187988439, -24.0182403953493);
    polygon.ps[3] = Point(16.81928187988439, -86.0182403953493);
    new ShapeRef(router, polygon, 48);

#if ALL
    // shapeRef49
    polygon = Polygon(4);
    polygon.ps[0] = Point(-80.18071812011561, -86.0182403953493);
    polygon.ps[1] = Point(-80.18071812011561, -24.0182403953493);
    polygon.ps[2] = Point(-142.1807181201156, -24.0182403953493);
    polygon.ps[3] = Point(-142.1807181201156, -86.0182403953493);
    new ShapeRef(router, polygon, 49);

    // shapeRef50
    polygon = Polygon(4);
    polygon.ps[0] = Point(620.1049961655988, -276.0182403973492);
    polygon.ps[1] = Point(620.1049961655988, -214.0182403973492);
    polygon.ps[2] = Point(558.1049961655988, -214.0182403973492);
    polygon.ps[3] = Point(558.1049961655988, -276.0182403973492);
    new ShapeRef(router, polygon, 50);

    // shapeRef51
    polygon = Polygon(4);
    polygon.ps[0] = Point(349.8192818798844, -86.0182403953493);
    polygon.ps[1] = Point(349.8192818798844, -24.0182403953493);
    polygon.ps[2] = Point(287.8192818798844, -24.0182403953493);
    polygon.ps[3] = Point(287.8192818798844, -86.0182403953493);
    new ShapeRef(router, polygon, 51);

    // shapeRef52
    polygon = Polygon(4);
    polygon.ps[0] = Point(620.1049961655988, 495.9817596066507);
    polygon.ps[1] = Point(620.1049961655988, 557.9817596066507);
    polygon.ps[2] = Point(558.1049961655988, 557.9817596066507);
    polygon.ps[3] = Point(558.1049961655988, 495.9817596066507);
    new ShapeRef(router, polygon, 52);

    // shapeRef53
    polygon = Polygon(4);
    polygon.ps[0] = Point(115.8192818798844, 915.8150929419839);
    polygon.ps[1] = Point(115.8192818798844, 975.8150929419839);
    polygon.ps[2] = Point(59.81928187988439, 975.8150929419839);
    polygon.ps[3] = Point(59.81928187988439, 915.8150929419839);
    new ShapeRef(router, polygon, 53);

    // shapeRef54
    polygon = Polygon(4);
    polygon.ps[0] = Point(115.8192818798844, 1037.815092941984);
    polygon.ps[1] = Point(115.8192818798844, 1097.815092941984);
    polygon.ps[2] = Point(59.81928187988439, 1097.815092941984);
    polygon.ps[3] = Point(59.81928187988439, 1037.815092941984);
    new ShapeRef(router, polygon, 54);

    // shapeRef55
    polygon = Polygon(4);
    polygon.ps[0] = Point(404.8192818798844, 953.8150929419839);
    polygon.ps[1] = Point(404.8192818798844, 993.8150929419839);
    polygon.ps[2] = Point(328.8192818798844, 993.8150929419839);
    polygon.ps[3] = Point(328.8192818798844, 953.8150929419839);
    new ShapeRef(router, polygon, 55);
#endif

    // shapeRef56
    polygon = Polygon(4);
    polygon.ps[0] = Point(24.81928187988439, 841.815092940984);
    polygon.ps[1] = Point(24.81928187988439, 901.815092940984);
    polygon.ps[2] = Point(-31.18071812011561, 901.815092940984);
    polygon.ps[3] = Point(-31.18071812011561, 841.815092940984);
    new ShapeRef(router, polygon, 56);

#if ALL
    // shapeRef57
    polygon = Polygon(4);
    polygon.ps[0] = Point(542.8192818798843, 836.315092940984);
    polygon.ps[1] = Point(542.8192818798843, 876.315092940984);
    polygon.ps[2] = Point(466.8192818798844, 876.315092940984);
    polygon.ps[3] = Point(466.8192818798844, 836.315092940984);
    new ShapeRef(router, polygon, 57);

    // shapeRef58
    polygon = Polygon(4);
    polygon.ps[0] = Point(1099.504996167599, 568.9817596076507);
    polygon.ps[1] = Point(1099.504996167599, 608.9817596076507);
    polygon.ps[2] = Point(1023.504996167599, 608.9817596076507);
    polygon.ps[3] = Point(1023.504996167599, 568.9817596076507);
    new ShapeRef(router, polygon, 58);

    // shapeRef59
    polygon = Polygon(4);
    polygon.ps[0] = Point(899.1049961675988, 578.315092940984);
    polygon.ps[1] = Point(899.1049961675988, 638.315092940984);
    polygon.ps[2] = Point(843.1049961675988, 638.315092940984);
    polygon.ps[3] = Point(843.1049961675988, 578.315092940984);
    new ShapeRef(router, polygon, 59);

    // shapeRef60
    polygon = Polygon(4);
    polygon.ps[0] = Point(484.8192818798844, 691.315092940984);
    polygon.ps[1] = Point(484.8192818798844, 751.315092940984);
    polygon.ps[2] = Point(428.8192818798844, 751.315092940984);
    polygon.ps[3] = Point(428.8192818798844, 691.315092940984);
    new ShapeRef(router, polygon, 60);

    // shapeRef61
    polygon = Polygon(4);
    polygon.ps[0] = Point(154.8192818798844, 578.315092940984);
    polygon.ps[1] = Point(154.8192818798844, 638.315092940984);
    polygon.ps[2] = Point(98.81928187988439, 638.315092940984);
    polygon.ps[3] = Point(98.81928187988439, 578.315092940984);
    new ShapeRef(router, polygon, 61);

    // shapeRef62
    polygon = Polygon(4);
    polygon.ps[0] = Point(675.1049961655988, 415.9817596066507);
    polygon.ps[1] = Point(675.1049961655988, 455.9817596066507);
    polygon.ps[2] = Point(599.1049961655988, 455.9817596066507);
    polygon.ps[3] = Point(599.1049961655988, 415.9817596066507);
    new ShapeRef(router, polygon, 62);

    // shapeRef63
    polygon = Polygon(4);
    polygon.ps[0] = Point(484.8192818798844, 423.9817596066507);
    polygon.ps[1] = Point(484.8192818798844, 483.9817596066507);
    polygon.ps[2] = Point(428.8192818798844, 483.9817596066507);
    polygon.ps[3] = Point(428.8192818798844, 423.9817596066507);
    new ShapeRef(router, polygon, 63);

    // shapeRef64
    polygon = Polygon(4);
    polygon.ps[0] = Point(484.8192818798844, 74.98175960565069);
    polygon.ps[1] = Point(484.8192818798844, 134.9817596056507);
    polygon.ps[2] = Point(428.8192818798844, 134.9817596056507);
    polygon.ps[3] = Point(428.8192818798844, 74.98175960565069);
    new ShapeRef(router, polygon, 64);

    // shapeRef65
    polygon = Polygon(4);
    polygon.ps[0] = Point(1099.504996167599, 415.9817596066507);
    polygon.ps[1] = Point(1099.504996167599, 455.9817596066507);
    polygon.ps[2] = Point(1023.504996167599, 455.9817596066507);
    polygon.ps[3] = Point(1023.504996167599, 415.9817596066507);
    new ShapeRef(router, polygon, 65);

    // shapeRef66
    polygon = Polygon(4);
    polygon.ps[0] = Point(763.1049961665988, 216.9817596056507);
    polygon.ps[1] = Point(763.1049961665988, 276.9817596056507);
    polygon.ps[2] = Point(707.1049961665988, 276.9817596056507);
    polygon.ps[3] = Point(707.1049961665988, 216.9817596056507);
    new ShapeRef(router, polygon, 66);

    // shapeRef67
    polygon = Polygon(4);
    polygon.ps[0] = Point(484.8192818798844, 149.9817596056507);
    polygon.ps[1] = Point(484.8192818798844, 209.9817596056507);
    polygon.ps[2] = Point(428.8192818798844, 209.9817596056507);
    polygon.ps[3] = Point(428.8192818798844, 149.9817596056507);
    new ShapeRef(router, polygon, 67);

    // shapeRef68
    polygon = Polygon(4);
    polygon.ps[0] = Point(763.1049961665988, 338.9817596056507);
    polygon.ps[1] = Point(763.1049961665988, 398.9817596056507);
    polygon.ps[2] = Point(707.1049961665988, 398.9817596056507);
    polygon.ps[3] = Point(707.1049961665988, 338.9817596056507);
    new ShapeRef(router, polygon, 68);

    // shapeRef69
    polygon = Polygon(4);
    polygon.ps[0] = Point(484.8192818798844, -241.0182403963493);
    polygon.ps[1] = Point(484.8192818798844, -181.0182403963493);
    polygon.ps[2] = Point(428.8192818798844, -181.0182403963493);
    polygon.ps[3] = Point(428.8192818798844, -241.0182403963493);
    new ShapeRef(router, polygon, 69);

    // shapeRef70
    polygon = Polygon(4);
    polygon.ps[0] = Point(675.1049961655988, -203.0182403963493);
    polygon.ps[1] = Point(675.1049961655988, -163.0182403963493);
    polygon.ps[2] = Point(599.1049961655988, -163.0182403963493);
    polygon.ps[3] = Point(599.1049961655988, -203.0182403963493);
    new ShapeRef(router, polygon, 70);

    // shapeRef71
    polygon = Polygon(4);
    polygon.ps[0] = Point(484.8192818798844, 271.9817596056507);
    polygon.ps[1] = Point(484.8192818798844, 331.9817596056507);
    polygon.ps[2] = Point(428.8192818798844, 331.9817596056507);
    polygon.ps[3] = Point(428.8192818798844, 271.9817596056507);
    new ShapeRef(router, polygon, 71);

    // shapeRef72
    polygon = Polygon(4);
    polygon.ps[0] = Point(154.8192818798844, -125.0182403953493);
    polygon.ps[1] = Point(154.8192818798844, -65.0182403953493);
    polygon.ps[2] = Point(98.81928187988439, -65.0182403953493);
    polygon.ps[3] = Point(98.81928187988439, -125.0182403953493);
    new ShapeRef(router, polygon, 72);

    // shapeRef73
    polygon = Polygon(4);
    polygon.ps[0] = Point(-8.180718120115614, -125.0182403953493);
    polygon.ps[1] = Point(-8.180718120115614, -65.0182403953493);
    polygon.ps[2] = Point(-64.18071812011561, -65.0182403953493);
    polygon.ps[3] = Point(-64.18071812011561, -125.0182403953493);
    new ShapeRef(router, polygon, 73);

    // shapeRef74
    polygon = Polygon(4);
    polygon.ps[0] = Point(343.8192818798844, 320.9817596056507);
    polygon.ps[1] = Point(343.8192818798844, 360.9817596056507);
    polygon.ps[2] = Point(267.8192818798844, 360.9817596056507);
    polygon.ps[3] = Point(267.8192818798844, 320.9817596056507);
    new ShapeRef(router, polygon, 74);

    // shapeRef75
    polygon = Polygon(4);
    polygon.ps[0] = Point(212.8192818798844, 254.9817596056507);
    polygon.ps[1] = Point(212.8192818798844, 294.9817596056507);
    polygon.ps[2] = Point(136.8192818798844, 294.9817596056507);
    polygon.ps[3] = Point(136.8192818798844, 254.9817596056507);
    new ShapeRef(router, polygon, 75);

    // shapeRef76
    polygon = Polygon(4);
    polygon.ps[0] = Point(484.8192818798844, -12.0182403953493);
    polygon.ps[1] = Point(484.8192818798844, 47.9817596046507);
    polygon.ps[2] = Point(428.8192818798844, 47.9817596046507);
    polygon.ps[3] = Point(428.8192818798844, -12.0182403953493);
    new ShapeRef(router, polygon, 76);

    // shapeRef77
    polygon = Polygon(4);
    polygon.ps[0] = Point(866.1049961675988, 84.98175960565069);
    polygon.ps[1] = Point(866.1049961675988, 144.9817596056507);
    polygon.ps[2] = Point(810.1049961675988, 144.9817596056507);
    polygon.ps[3] = Point(810.1049961675988, 84.98175960565069);
    new ShapeRef(router, polygon, 77);

    // shapeRef78
    polygon = Polygon(4);
    polygon.ps[0] = Point(866.1049961675988, 206.9817596056507);
    polygon.ps[1] = Point(866.1049961675988, 266.9817596056507);
    polygon.ps[2] = Point(810.1049961675988, 266.9817596056507);
    polygon.ps[3] = Point(810.1049961675988, 206.9817596056507);
    new ShapeRef(router, polygon, 78);

    // shapeRef79
    polygon = Polygon(4);
    polygon.ps[0] = Point(484.8192818798844, -119.0182403963493);
    polygon.ps[1] = Point(484.8192818798844, -59.01824039634928);
    polygon.ps[2] = Point(428.8192818798844, -59.01824039634928);
    polygon.ps[3] = Point(428.8192818798844, -119.0182403963493);
    new ShapeRef(router, polygon, 79);

    // shapeRef80
    polygon = Polygon(4);
    polygon.ps[0] = Point(866.1049961675988, 9.981759605650694);
    polygon.ps[1] = Point(866.1049961675988, 69.98175960565069);
    polygon.ps[2] = Point(810.1049961675988, 69.98175960565069);
    polygon.ps[3] = Point(810.1049961675988, 9.981759605650694);
    new ShapeRef(router, polygon, 80);

    // shapeRef81
    polygon = Polygon(4);
    polygon.ps[0] = Point(-91.18071812011561, 455.9817596066507);
    polygon.ps[1] = Point(-91.18071812011561, 515.9817596066507);
    polygon.ps[2] = Point(-131.1807181201156, 515.9817596066507);
    polygon.ps[3] = Point(-131.1807181201156, 455.9817596066507);
    new ShapeRef(router, polygon, 81);

    // shapeRef82
    polygon = Polygon(4);
    polygon.ps[0] = Point(-91.18071812011561, 559.9817596066507);
    polygon.ps[1] = Point(-91.18071812011561, 619.9817596066507);
    polygon.ps[2] = Point(-131.1807181201156, 619.9817596066507);
    polygon.ps[3] = Point(-131.1807181201156, 559.9817596066507);
    new ShapeRef(router, polygon, 82);

    // shapeRef83
    polygon = Polygon(4);
    polygon.ps[0] = Point(876.0049961675987, 79.98175960565069);
    polygon.ps[1] = Point(876.0049961675987, 196.9817596056507);
    polygon.ps[2] = Point(800.2049961675988, 196.9817596056507);
    polygon.ps[3] = Point(800.2049961675988, 79.98175960565069);
    new ShapeRef(router, polygon, 83);
#endif

    // shapeRef84
    polygon = Polygon(4);
    polygon.ps[0] = Point(125.7192818798844, 985.8150929419839);
    polygon.ps[1] = Point(125.7192818798844, 1027.815092941984);
    polygon.ps[2] = Point(49.91928187988439, 1027.815092941984);
    polygon.ps[3] = Point(49.91928187988439, 985.8150929419839);
    new ShapeRef(router, polygon, 84);

#if ALL
    // shapeRef85
    polygon = Polygon(4);
    polygon.ps[0] = Point(494.7192818798844, -171.0182403963493);
    polygon.ps[1] = Point(494.7192818798844, -129.0182403963493);
    polygon.ps[2] = Point(418.9192818798844, -129.0182403963493);
    polygon.ps[3] = Point(418.9192818798844, -171.0182403963493);
    new ShapeRef(router, polygon, 85);

    // shapeRef86
    polygon = Polygon(4);
    polygon.ps[0] = Point(773.0049961665987, 286.9817596056507);
    polygon.ps[1] = Point(773.0049961665987, 328.9817596056507);
    polygon.ps[2] = Point(697.2049961665988, 328.9817596056507);
    polygon.ps[3] = Point(697.2049961665988, 286.9817596056507);
    new ShapeRef(router, polygon, 86);

    // shapeRef87
    polygon = Polygon(4);
    polygon.ps[0] = Point(494.7192818798844, 144.9817596056507);
    polygon.ps[1] = Point(494.7192818798844, 261.9817596056507);
    polygon.ps[2] = Point(418.9192818798844, 261.9817596056507);
    polygon.ps[3] = Point(418.9192818798844, 144.9817596056507);
    new ShapeRef(router, polygon, 87);

    // connRef88
    connRef = new ConnRef(router, 88);
    srcPt = ConnEnd(Point(111.8192818798844, 648.315092940984), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(-111.1807181201156, 856.315092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef89
    connRef = new ConnRef(router, 89);
    srcPt = ConnEnd(Point(126.8192818798844, 289.9817596056507), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(-111.1807181201156, 856.315092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
#endif

    // connRef90
    connRef = new ConnRef(router, 90);
    srcPt = ConnEnd(Point(72.81928187988439, 985.8150929419839), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints90(1);
    checkpoints90[0] = Checkpoint(Point(49.81928187988439, 1006.815092941984), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints90);
    Avoid::ConnRef *connRef90 = connRef;

    // connRef91
    connRef = new ConnRef(router, 91);
    srcPt = ConnEnd(Point(72.81928187988439, 1027.815092941984), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints91(1);
    checkpoints91[0] = Checkpoint(Point(49.81928187988439, 1006.815092941984), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints91);
    Avoid::ConnRef *connRef91 = connRef;

#if ALL
    // connRef92
    connRef = new ConnRef(router, 92);
    srcPt = ConnEnd(Point(471.8192818798844, -171.0182403963493), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints92(1);
    checkpoints92[0] = Checkpoint(Point(494.8192818798844, -150.0182403963493), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints92);

    // connRef93
    connRef = new ConnRef(router, 93);
    srcPt = ConnEnd(Point(589.1049961655988, -55.0182403953493), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -168.0182403963493), 2);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef94
    connRef = new ConnRef(router, 94);
    srcPt = ConnEnd(Point(589.1049961655988, -55.0182403953493), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(471.8192818798844, -22.0182403953493), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef95
    connRef = new ConnRef(router, 95);
    srcPt = ConnEnd(Point(823.1049961675988, 154.9817596056507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints95(1);
    checkpoints95[0] = Checkpoint(Point(800.1049961675988, 175.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints95);

    // connRef96
    connRef = new ConnRef(router, 96);
    srcPt = ConnEnd(Point(823.1049961675988, 196.9817596056507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints96(1);
    checkpoints96[0] = Checkpoint(Point(800.1049961675988, 175.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints96);

    // connRef97
    connRef = new ConnRef(router, 97);
    srcPt = ConnEnd(Point(471.8192818798844, -129.0182403963493), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints97(1);
    checkpoints97[0] = Checkpoint(Point(494.8192818798844, -150.0182403963493), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints97);

    // connRef98
    connRef = new ConnRef(router, 98);
    srcPt = ConnEnd(Point(823.1049961675988, 79.98175960565069), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints98(1);
    checkpoints98[0] = Checkpoint(Point(800.1049961675988, 175.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints98);

    // connRef99
    connRef = new ConnRef(router, 99);
    srcPt = ConnEnd(Point(1013.504996167599, 526.9817596066507), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(1013.504996167599, 573.9817596076507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef100
    connRef = new ConnRef(router, 100);
    srcPt = ConnEnd(Point(1013.504996167599, 450.9817596066507), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(1013.504996167599, 526.9817596066507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef101
    connRef = new ConnRef(router, 101);
    srcPt = ConnEnd(Point(318.8192818798844, 648.315092940984), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(141.8192818798844, 648.315092940984), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef102
    connRef = new ConnRef(router, 102);
    srcPt = ConnEnd(Point(441.8192818798844, 493.9817596066507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, 648.315092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef103
    connRef = new ConnRef(router, 103);
    srcPt = ConnEnd(Point(102.8192818798844, 985.8150929419839), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, 911.815092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints103(1);
    checkpoints103[0] = Checkpoint(Point(125.8192818798844, 1006.815092941984), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints103);

    // connRef104
    connRef = new ConnRef(router, 104);
    srcPt = ConnEnd(Point(102.8192818798844, 1027.815092941984), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, 911.815092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints104(1);
    checkpoints104[0] = Checkpoint(Point(125.8192818798844, 1006.815092941984), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints104);

    // connRef105
    connRef = new ConnRef(router, 105);
    srcPt = ConnEnd(Point(318.8192818798844, 988.8150929419839), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, 1039.815092941984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef106
    connRef = new ConnRef(router, 106);
    srcPt = ConnEnd(Point(318.8192818798844, 911.815092940984), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, 958.8150929419839), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef107
    connRef = new ConnRef(router, 107);
    srcPt = ConnEnd(Point(-18.18071812011561, 911.815092940984), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(-111.1807181201156, 856.315092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef108
    connRef = new ConnRef(router, 108);
    srcPt = ConnEnd(Point(318.8192818798844, 911.815092940984), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(11.81928187988439, 911.815092940984), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef109
    connRef = new ConnRef(router, 109);
    srcPt = ConnEnd(Point(456.8192818798844, 871.315092940984), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, 911.815092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef110
    connRef = new ConnRef(router, 110);
    srcPt = ConnEnd(Point(589.1049961655988, 648.315092940984), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, 841.315092940984), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef111
    connRef = new ConnRef(router, 111);
    srcPt = ConnEnd(Point(1013.504996167599, 603.9817596076507), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(1013.504996167599, 750.315092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef112
    connRef = new ConnRef(router, 112);
    srcPt = ConnEnd(Point(970.1049961675988, 307.9817596056507), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(886.1049961675988, 648.315092940984), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef113
    connRef = new ConnRef(router, 113);
    srcPt = ConnEnd(Point(856.1049961675988, 648.315092940984), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, 648.315092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef114
    connRef = new ConnRef(router, 114);
    srcPt = ConnEnd(Point(441.8192818798844, 681.315092940984), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, 648.315092940984), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef115
    connRef = new ConnRef(router, 115);
    srcPt = ConnEnd(Point(589.1049961655988, 307.9817596056507), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, 420.9817596066507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef116
    connRef = new ConnRef(router, 116);
    srcPt = ConnEnd(Point(589.1049961655988, 450.9817596066507), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, 526.9817596066507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef117
    connRef = new ConnRef(router, 117);
    srcPt = ConnEnd(Point(589.1049961655988, 307.9817596056507), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(471.8192818798844, 493.9817596066507), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef118
    connRef = new ConnRef(router, 118);
    srcPt = ConnEnd(Point(471.8192818798844, 144.9817596056507), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints118(1);
    checkpoints118[0] = Checkpoint(Point(494.8192818798844, 240.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints118);

    // connRef119
    connRef = new ConnRef(router, 119);
    srcPt = ConnEnd(Point(441.8192818798844, 144.9817596056507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints119(1);
    checkpoints119[0] = Checkpoint(Point(418.8192818798844, 240.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints119);

    // connRef120
    connRef = new ConnRef(router, 120);
    srcPt = ConnEnd(Point(970.1049961675988, 307.9817596056507), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(1013.504996167599, 420.9817596066507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef121
    connRef = new ConnRef(router, 121);
    srcPt = ConnEnd(Point(750.1049961665988, 286.9817596056507), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(970.1049961675988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints121(1);
    checkpoints121[0] = Checkpoint(Point(773.1049961665988, 307.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints121);

    // connRef122
    connRef = new ConnRef(router, 122);
    srcPt = ConnEnd(Point(720.1049961665988, 286.9817596056507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints122(1);
    checkpoints122[0] = Checkpoint(Point(697.1049961665988, 307.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints122);

    // connRef123
    connRef = new ConnRef(router, 123);
    srcPt = ConnEnd(Point(471.8192818798844, 219.9817596056507), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints123(1);
    checkpoints123[0] = Checkpoint(Point(494.8192818798844, 240.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints123);

    // connRef124
    connRef = new ConnRef(router, 124);
    srcPt = ConnEnd(Point(441.8192818798844, 219.9817596056507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints124(1);
    checkpoints124[0] = Checkpoint(Point(418.8192818798844, 240.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints124);

    // connRef125
    connRef = new ConnRef(router, 125);
    srcPt = ConnEnd(Point(750.1049961665988, 328.9817596056507), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(970.1049961675988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints125(1);
    checkpoints125[0] = Checkpoint(Point(773.1049961665988, 307.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints125);

    // connRef126
    connRef = new ConnRef(router, 126);
    srcPt = ConnEnd(Point(441.8192818798844, -171.0182403963493), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, -150.0182403963493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints126(1);
    checkpoints126[0] = Checkpoint(Point(418.8192818798844, -150.0182403963493), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints126);

    // connRef127
    connRef = new ConnRef(router, 127);
    srcPt = ConnEnd(Point(589.1049961655988, -198.0182403963493), 1);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -245.0182403973492), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef128
    connRef = new ConnRef(router, 128);
    srcPt = ConnEnd(Point(471.8192818798844, 261.9817596056507), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints128(1);
    checkpoints128[0] = Checkpoint(Point(494.8192818798844, 240.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints128);

    // connRef129
    connRef = new ConnRef(router, 129);
    srcPt = ConnEnd(Point(441.8192818798844, 261.9817596056507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints129(1);
    checkpoints129[0] = Checkpoint(Point(418.8192818798844, 240.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints129);
#endif

    // connRef130
    connRef = new ConnRef(router, 130);
    srcPt = ConnEnd(Point(47.81928187988439, -55.0182403953493), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(111.8192818798844, -55.0182403953493), 4);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

#if ALL
    // connRef131
    connRef = new ConnRef(router, 131);
    srcPt = ConnEnd(Point(141.8192818798844, -55.0182403953493), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef132
    connRef = new ConnRef(router, 132);
    srcPt = ConnEnd(Point(-21.18071812011561, -55.0182403953493), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(47.81928187988439, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef133
    connRef = new ConnRef(router, 133);
    srcPt = ConnEnd(Point(-111.1807181201156, -55.0182403953493), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(-51.18071812011561, -55.0182403953493), 4);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef134
    connRef = new ConnRef(router, 134);
    srcPt = ConnEnd(Point(257.8192818798844, 355.9817596056507), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(257.8192818798844, 402.9817596066507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef135
    connRef = new ConnRef(router, 135);
    srcPt = ConnEnd(Point(318.8192818798844, -55.0182403953493), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(257.8192818798844, 325.9817596056507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef136
    connRef = new ConnRef(router, 136);
    srcPt = ConnEnd(Point(318.8192818798844, -55.0182403953493), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(126.8192818798844, 259.9817596056507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef137
    connRef = new ConnRef(router, 137);
    srcPt = ConnEnd(Point(441.8192818798844, -22.0182403953493), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, -55.0182403953493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef138
    connRef = new ConnRef(router, 138);
    srcPt = ConnEnd(Point(853.1049961675988, 154.9817596056507), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(970.1049961675988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints138(1);
    checkpoints138[0] = Checkpoint(Point(876.1049961675988, 175.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints138);

    // connRef139
    connRef = new ConnRef(router, 139);
    srcPt = ConnEnd(Point(853.1049961675988, 196.9817596056507), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(970.1049961675988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints139(1);
    checkpoints139[0] = Checkpoint(Point(876.1049961675988, 175.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints139);

    // connRef140
    connRef = new ConnRef(router, 140);
    srcPt = ConnEnd(Point(853.1049961675988, 79.98175960565069), 8);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(970.1049961675988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints140(1);
    checkpoints140[0] = Checkpoint(Point(876.1049961675988, 175.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef->setRoutingCheckpoints(checkpoints140);

    // connRef141
    connRef = new ConnRef(router, 141);
    srcPt = ConnEnd(Point(441.8192818798844, -129.0182403963493), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, -150.0182403963493), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints141(1);
    checkpoints141[0] = Checkpoint(Point(418.8192818798844, -150.0182403963493), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints141);

    // connRef142
    connRef = new ConnRef(router, 142);
    srcPt = ConnEnd(Point(720.1049961665988, 328.9817596056507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, 307.9817596056507), 15);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints142(1);
    checkpoints142[0] = Checkpoint(Point(697.1049961665988, 307.9817596056507), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef->setRoutingCheckpoints(checkpoints142);

    // connRef143
    connRef = new ConnRef(router, 143);
    srcPt = ConnEnd(Point(589.1049961655988, 648.315092940984), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(471.8192818798844, 681.315092940984), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef144
    connRef = new ConnRef(router, 144);
    srcPt = ConnEnd(Point(87.81928187988439, 935.8150929419839), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(87.81928187988439, 985.8150929419839), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef145
    connRef = new ConnRef(router, 145);
    srcPt = ConnEnd(Point(87.81928187988439, 1077.815092941984), 1);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(87.81928187988439, 1027.815092941984), 2);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef146
    connRef = new ConnRef(router, 146);
    srcPt = ConnEnd(Point(376.8192818798844, 973.8150929419839), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(318.8192818798844, 973.8150929419839), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef147
    connRef = new ConnRef(router, 147);
    srcPt = ConnEnd(Point(-3.180718120115614, 861.815092940984), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(-3.180718120115614, 911.815092940984), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef148
    connRef = new ConnRef(router, 148);
    srcPt = ConnEnd(Point(514.8192818798843, 856.315092940984), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, 856.315092940984), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef149
    connRef = new ConnRef(router, 149);
    srcPt = ConnEnd(Point(1071.504996167599, 588.9817596076507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(1013.504996167599, 588.9817596076507), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef150
    connRef = new ConnRef(router, 150);
    srcPt = ConnEnd(Point(871.1049961675988, 598.315092940984), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(871.1049961675988, 648.315092940984), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef151
    connRef = new ConnRef(router, 151);
    srcPt = ConnEnd(Point(456.8192818798844, 731.315092940984), 1);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, 681.315092940984), 2);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef152
    connRef = new ConnRef(router, 152);
    srcPt = ConnEnd(Point(126.8192818798844, 598.315092940984), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(126.8192818798844, 648.315092940984), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef153
    connRef = new ConnRef(router, 153);
    srcPt = ConnEnd(Point(647.1049961655988, 435.9817596066507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, 435.9817596066507), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef154
    connRef = new ConnRef(router, 154);
    srcPt = ConnEnd(Point(456.8192818798844, 443.9817596066507), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, 493.9817596066507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef155
    connRef = new ConnRef(router, 155);
    srcPt = ConnEnd(Point(456.8192818798844, 94.98175960565069), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, 144.9817596056507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef156
    connRef = new ConnRef(router, 156);
    srcPt = ConnEnd(Point(1071.504996167599, 435.9817596066507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(1013.504996167599, 435.9817596066507), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef157
    connRef = new ConnRef(router, 157);
    srcPt = ConnEnd(Point(735.1049961665988, 236.9817596056507), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(735.1049961665988, 286.9817596056507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef158
    connRef = new ConnRef(router, 158);
    srcPt = ConnEnd(Point(456.8192818798844, 169.9817596056507), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, 219.9817596056507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef159
    connRef = new ConnRef(router, 159);
    srcPt = ConnEnd(Point(735.1049961665988, 378.9817596056507), 1);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(735.1049961665988, 328.9817596056507), 2);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef160
    connRef = new ConnRef(router, 160);
    srcPt = ConnEnd(Point(456.8192818798844, -221.0182403963493), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, -171.0182403963493), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef161
    connRef = new ConnRef(router, 161);
    srcPt = ConnEnd(Point(647.1049961655988, -183.0182403963493), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(589.1049961655988, -183.0182403963493), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef162
    connRef = new ConnRef(router, 162);
    srcPt = ConnEnd(Point(456.8192818798844, 311.9817596056507), 1);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, 261.9817596056507), 2);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef163
    connRef = new ConnRef(router, 163);
    srcPt = ConnEnd(Point(126.8192818798844, -105.0182403953493), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(126.8192818798844, -55.0182403953493), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef164
    connRef = new ConnRef(router, 164);
    srcPt = ConnEnd(Point(-36.18071812011561, -105.0182403953493), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(-36.18071812011561, -55.0182403953493), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef165
    connRef = new ConnRef(router, 165);
    srcPt = ConnEnd(Point(315.8192818798844, 340.9817596056507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(257.8192818798844, 340.9817596056507), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef166
    connRef = new ConnRef(router, 166);
    srcPt = ConnEnd(Point(184.8192818798844, 274.9817596056507), 4);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(126.8192818798844, 274.9817596056507), 8);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef167
    connRef = new ConnRef(router, 167);
    srcPt = ConnEnd(Point(456.8192818798844, 27.9817596046507), 1);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, -22.0182403953493), 2);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef168
    connRef = new ConnRef(router, 168);
    srcPt = ConnEnd(Point(838.1049961675988, 104.9817596056507), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(838.1049961675988, 154.9817596056507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef169
    connRef = new ConnRef(router, 169);
    srcPt = ConnEnd(Point(838.1049961675988, 246.9817596056507), 1);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(838.1049961675988, 196.9817596056507), 2);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef170
    connRef = new ConnRef(router, 170);
    srcPt = ConnEnd(Point(456.8192818798844, -79.01824039634928), 1);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(456.8192818798844, -129.0182403963493), 2);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef171
    connRef = new ConnRef(router, 171);
    srcPt = ConnEnd(Point(838.1049961675988, 29.98175960565069), 2);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(838.1049961675988, 79.98175960565069), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef172
    connRef = new ConnRef(router, 172);
    srcPt = ConnEnd(Point(-111.1807181201156, -55.0182403953493), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(-111.1807181201156, 460.9817596066507), 1);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);

    // connRef173
    connRef = new ConnRef(router, 173);
    srcPt = ConnEnd(Point(-111.1807181201156, 856.315092940984), 15);
    connRef->setSourceEndpoint(srcPt);
    dstPt = ConnEnd(Point(-111.1807181201156, 614.9817596066507), 2);
    connRef->setDestEndpoint(dstPt);
    connRef->setRoutingType((ConnType)2);
#endif

    router->processTransaction();
    router->outputDiagram("output/nudgingSkipsCheckpoint01");
    Avoid::PolyLine route90 = connRef90->displayRoute();
    Avoid::PolyLine route91 = connRef91->displayRoute();
    
    delete router;
    return (route90.size() == 7 && route91.size() == 7) ? 0 : 1;
};
