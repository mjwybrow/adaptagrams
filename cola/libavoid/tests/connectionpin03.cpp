#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(
            PolyLineRouting | OrthogonalRouting);
    router->setRoutingPenalty((PenaltyType)0, 50);
    router->setRoutingPenalty((PenaltyType)1, 0);
    router->setRoutingPenalty((PenaltyType)2, 200);
    router->setRoutingPenalty((PenaltyType)3, 4000);
    router->setRoutingPenalty((PenaltyType)4, 110);
    router->setRoutingParameter(idealNudgingDistance, 25);

    Polygon poly219926511(4);
    poly219926511.ps[0] = Point(50760, 51240);
    poly219926511.ps[1] = Point(50760, 51260);
    poly219926511.ps[2] = Point(50740, 51260);
    poly219926511.ps[3] = Point(50740, 51240);
    ShapeRef *shapeRef219926511 = new ShapeRef(router, poly219926511, 219926511);
    new ShapeConnectionPin(shapeRef219926511, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef219926511, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 1);
    new ShapeConnectionPin(shapeRef219926511, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef219926511, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 2);

    JunctionRef *shapeRef171026375 = new JunctionRef(router, Point(51075, 51225), 171026375);

    Polygon poly878674(4);
    poly878674.ps[0] = Point(51454, 51200);
    poly878674.ps[1] = Point(51454, 51285);
    poly878674.ps[2] = Point(51396, 51285);
    poly878674.ps[3] = Point(51396, 51200);
    new ShapeRef(router, poly878674, 878674);

    Polygon poly286906544(4);
    poly286906544.ps[0] = Point(50704, 51250);
    poly286906544.ps[1] = Point(50704, 51335);
    poly286906544.ps[2] = Point(50646, 51335);
    poly286906544.ps[3] = Point(50646, 51250);
    new ShapeRef(router, poly286906544, 286906544);

    Polygon poly170813081(4);
    poly170813081.ps[0] = Point(51104, 51090);
    poly170813081.ps[1] = Point(51104, 51175);
    poly170813081.ps[2] = Point(51046, 51175);
    poly170813081.ps[3] = Point(51046, 51090);
    new ShapeRef(router, poly170813081, 170813081);

    Polygon poly342721632(4);
    poly342721632.ps[0] = Point(51910, 50946);
    poly342721632.ps[1] = Point(51910, 51004);
    poly342721632.ps[2] = Point(51825, 51004);
    poly342721632.ps[3] = Point(51825, 50946);
    new ShapeRef(router, poly342721632, 342721632);

    Polygon poly223927132(4);
    poly223927132.ps[0] = Point(51510, 51190);
    poly223927132.ps[1] = Point(51510, 51210);
    poly223927132.ps[2] = Point(51490, 51210);
    poly223927132.ps[3] = Point(51490, 51190);
    ShapeRef *shapeRef223927132 = new ShapeRef(router, poly223927132, 223927132);
    new ShapeConnectionPin(shapeRef223927132, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef223927132, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 1);
    new ShapeConnectionPin(shapeRef223927132, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 8);
    new ShapeConnectionPin(shapeRef223927132, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 2);

    JunctionRef *shapeRef102578723 = new JunctionRef(router, Point(51275, 50725), 102578723);

    JunctionRef *shapeRef17922268 = new JunctionRef(router, Point(50850, 50975), 17922268);

    Polygon poly11057158(4);
    poly11057158.ps[0] = Point(51679, 51200);
    poly11057158.ps[1] = Point(51679, 51285);
    poly11057158.ps[2] = Point(51621, 51285);
    poly11057158.ps[3] = Point(51621, 51200);
    new ShapeRef(router, poly11057158, 11057158);

    Polygon poly322446045(4);
    poly322446045.ps[0] = Point(50779, 51240);
    poly322446045.ps[1] = Point(50779, 51335);
    poly322446045.ps[2] = Point(50721, 51335);
    poly322446045.ps[3] = Point(50721, 51240);
    new ShapeRef(router, poly322446045, 322446045);

    Polygon poly767565928(4);
    poly767565928.ps[0] = Point(51004, 50741);
    poly767565928.ps[1] = Point(51004, 50826);
    poly767565928.ps[2] = Point(50946, 50826);
    poly767565928.ps[3] = Point(50946, 50741);
    new ShapeRef(router, poly767565928, 767565928);

    Polygon poly280939374(4);
    poly280939374.ps[0] = Point(51085, 50971);
    poly280939374.ps[1] = Point(51085, 51029);
    poly280939374.ps[2] = Point(51000, 51029);
    poly280939374.ps[3] = Point(51000, 50971);
    new ShapeRef(router, poly280939374, 280939374);

    Polygon poly630623707(4);
    poly630623707.ps[0] = Point(51279, 50925);
    poly630623707.ps[1] = Point(51279, 51010);
    poly630623707.ps[2] = Point(51221, 51010);
    poly630623707.ps[3] = Point(51221, 50925);
    new ShapeRef(router, poly630623707, 630623707);

    Polygon poly28160455(4);
    poly28160455.ps[0] = Point(50879, 51250);
    poly28160455.ps[1] = Point(50879, 51335);
    poly28160455.ps[2] = Point(50821, 51335);
    poly28160455.ps[3] = Point(50821, 51250);
    new ShapeRef(router, poly28160455, 28160455);

    Polygon poly525890448(4);
    poly525890448.ps[0] = Point(51174, 50721);
    poly525890448.ps[1] = Point(51174, 50779);
    poly525890448.ps[2] = Point(51089, 50779);
    poly525890448.ps[3] = Point(51089, 50721);
    new ShapeRef(router, poly525890448, 525890448);

    Polygon poly157182552(4);
    poly157182552.ps[0] = Point(51529, 51190);
    poly157182552.ps[1] = Point(51529, 51285);
    poly157182552.ps[2] = Point(51471, 51285);
    poly157182552.ps[3] = Point(51471, 51190);
    new ShapeRef(router, poly157182552, 157182552);

    Polygon poly799637520(4);
    poly799637520.ps[0] = Point(50625, 50896);
    poly799637520.ps[1] = Point(50625, 50954);
    poly799637520.ps[2] = Point(50540, 50954);
    poly799637520.ps[3] = Point(50540, 50896);
    new ShapeRef(router, poly799637520, 799637520);

    Polygon poly60349320(4);
    poly60349320.ps[0] = Point(50904, 51025);
    poly60349320.ps[1] = Point(50904, 51110);
    poly60349320.ps[2] = Point(50846, 51110);
    poly60349320.ps[3] = Point(50846, 51025);
    new ShapeRef(router, poly60349320, 60349320);
    
    Polygon poly119851894(4);
    poly119851894.ps[0] = Point(51179, 51225);
    poly119851894.ps[1] = Point(51179, 51310);
    poly119851894.ps[2] = Point(51121, 51310);
    poly119851894.ps[3] = Point(51121, 51225);
    new ShapeRef(router, poly119851894, 119851894);
    
    Polygon poly233164275(4);
    poly233164275.ps[0] = Point(51604, 51200);
    poly233164275.ps[1] = Point(51604, 51285);
    poly233164275.ps[2] = Point(51546, 51285);
    poly233164275.ps[3] = Point(51546, 51200);
    new ShapeRef(router, poly233164275, 233164275);
    
    Polygon poly807558175(4);
    poly807558175.ps[0] = Point(51604, 51000);
    poly807558175.ps[1] = Point(51604, 51085);
    poly807558175.ps[2] = Point(51546, 51085);
    poly807558175.ps[3] = Point(51546, 51000);
    new ShapeRef(router, poly807558175, 807558175);
    
    JunctionRef *shapeRef265539540 = new JunctionRef(router, Point(51625, 50950), 265539540);

    Polygon poly108859552(4);
    poly108859552.ps[0] = Point(51560, 50721);
    poly108859552.ps[1] = Point(51560, 50779);
    poly108859552.ps[2] = Point(51475, 50779);
    poly108859552.ps[3] = Point(51475, 50721);
    new ShapeRef(router, poly108859552, 108859552);
    
    Polygon poly404900496(4);
    poly404900496.ps[0] = Point(51525, 50946);
    poly404900496.ps[1] = Point(51525, 51004);
    poly404900496.ps[2] = Point(51440, 51004);
    poly404900496.ps[3] = Point(51440, 50946);
    new ShapeRef(router, poly404900496, 404900496);
    
    Polygon poly620445000(4);
    poly620445000.ps[0] = Point(51029, 51225);
    poly620445000.ps[1] = Point(51029, 51310);
    poly620445000.ps[2] = Point(50971, 51310);
    poly620445000.ps[3] = Point(50971, 51225);
    new ShapeRef(router, poly620445000, 620445000);
    
    ConnRef *connRef450684531 = new ConnRef(router, 450684531);
    ConnEnd srcPt450684531(Point(51165, 50750), 8);
    connRef450684531->setSourceEndpoint(srcPt450684531);
    ConnEnd dstPt450684531(shapeRef102578723);
    connRef450684531->setDestEndpoint(dstPt450684531);
    connRef450684531->setRoutingType((ConnType)2);

    ConnRef *connRef135371350 = new ConnRef(router, 135371350);
    ConnEnd srcPt135371350(shapeRef102578723);
    connRef135371350->setSourceEndpoint(srcPt135371350);
    ConnEnd dstPt135371350(Point(51250, 50935), 1);
    connRef135371350->setDestEndpoint(dstPt135371350);
    connRef135371350->setRoutingType((ConnType)2);

    ConnRef *connRef143333770 = new ConnRef(router, 143333770);
    ConnEnd srcPt143333770(Point(51485, 50750), 4);
    connRef143333770->setSourceEndpoint(srcPt143333770);
    ConnEnd dstPt143333770(shapeRef102578723);
    connRef143333770->setDestEndpoint(dstPt143333770);
    connRef143333770->setRoutingType((ConnType)2);

    ConnRef *connRef102966575 = new ConnRef(router, 102966575);
    ConnEnd srcPt102966575(Point(50615, 50925), 8);
    connRef102966575->setSourceEndpoint(srcPt102966575);
    ConnEnd dstPt102966575(shapeRef17922268);
    connRef102966575->setDestEndpoint(dstPt102966575);
    connRef102966575->setRoutingType((ConnType)2);

    ConnRef *connRef68998920 = new ConnRef(router, 68998920);
    ConnEnd srcPt68998920(shapeRef17922268);
    connRef68998920->setSourceEndpoint(srcPt68998920);
    ConnEnd dstPt68998920(Point(50975, 50815), 2);
    connRef68998920->setDestEndpoint(dstPt68998920);
    connRef68998920->setRoutingType((ConnType)2);

    ConnRef *connRef1625709 = new ConnRef(router, 1625709);
    ConnEnd srcPt1625709(shapeRef17922268);
    connRef1625709->setSourceEndpoint(srcPt1625709);
    ConnEnd dstPt1625709(Point(50875, 51035), 1);
    connRef1625709->setDestEndpoint(dstPt1625709);
    connRef1625709->setRoutingType((ConnType)2);

    ConnRef *connRef34178450 = new ConnRef(router, 34178450);
    ConnEnd srcPt34178450(shapeRef17922268);
    connRef34178450->setSourceEndpoint(srcPt34178450);
    ConnEnd dstPt34178450(Point(51010, 51000), 4);
    connRef34178450->setDestEndpoint(dstPt34178450);
    connRef34178450->setRoutingType((ConnType)2);

    ConnRef *connRef7982176 = new ConnRef(router, 7982176);
    ConnEnd srcPt7982176(Point(50675, 51260), 1);
    connRef7982176->setSourceEndpoint(srcPt7982176);
    ConnEnd dstPt7982176(shapeRef219926511, 2147483646);
    connRef7982176->setDestEndpoint(dstPt7982176);
    connRef7982176->setRoutingType((ConnType)2);

    ConnRef *connRef197177982 = new ConnRef(router, 197177982);
    ConnEnd srcPt197177982(Point(50750, 51250), 15);
    connRef197177982->setSourceEndpoint(srcPt197177982);
    ConnEnd dstPt197177982(Point(50750, 51260), 1);
    connRef197177982->setDestEndpoint(dstPt197177982);
    connRef197177982->setRoutingType((ConnType)2);

    ConnRef *connRef398066432 = new ConnRef(router, 398066432);
    ConnEnd srcPt398066432(shapeRef219926511, 2147483646);
    connRef398066432->setSourceEndpoint(srcPt398066432);
    ConnEnd dstPt398066432(Point(50850, 51260), 1);
    connRef398066432->setDestEndpoint(dstPt398066432);
    connRef398066432->setRoutingType((ConnType)2);

    ConnRef *connRef314512308 = new ConnRef(router, 314512308);
    ConnEnd srcPt314512308(Point(51000, 51235), 1);
    connRef314512308->setSourceEndpoint(srcPt314512308);
    ConnEnd dstPt314512308(shapeRef171026375);
    connRef314512308->setDestEndpoint(dstPt314512308);
    connRef314512308->setRoutingType((ConnType)2);

    ConnRef *connRef182776308 = new ConnRef(router, 182776308);
    ConnEnd srcPt182776308(Point(51075, 51225), 15);
    connRef182776308->setSourceEndpoint(srcPt182776308);
    ConnEnd dstPt182776308(Point(51075, 51165), 2);
    connRef182776308->setDestEndpoint(dstPt182776308);
    connRef182776308->setRoutingType((ConnType)2);

    ConnRef *connRef96504835 = new ConnRef(router, 96504835);
    ConnEnd srcPt96504835(Point(51150, 51235), 1);
    connRef96504835->setSourceEndpoint(srcPt96504835);
    ConnEnd dstPt96504835(shapeRef171026375);
    connRef96504835->setDestEndpoint(dstPt96504835);
    connRef96504835->setRoutingType((ConnType)2);

    ConnRef *connRef118099300 = new ConnRef(router, 118099300);
    ConnEnd srcPt118099300(Point(51425, 51210), 1);
    connRef118099300->setSourceEndpoint(srcPt118099300);
    ConnEnd dstPt118099300(shapeRef223927132, 2147483646);
    connRef118099300->setDestEndpoint(dstPt118099300);
    connRef118099300->setRoutingType((ConnType)2);

    ConnRef *connRef176080410 = new ConnRef(router, 176080410);
    ConnEnd srcPt176080410(shapeRef223927132, 2147483646);
    connRef176080410->setSourceEndpoint(srcPt176080410);
    ConnEnd dstPt176080410(Point(51575, 51210), 1);
    connRef176080410->setDestEndpoint(dstPt176080410);
    connRef176080410->setRoutingType((ConnType)2);

    ConnRef *connRef426666001 = new ConnRef(router, 426666001);
    ConnEnd srcPt426666001(Point(51500, 51210), 1);
    connRef426666001->setSourceEndpoint(srcPt426666001);
    ConnEnd dstPt426666001(Point(51500, 51200), 15);
    connRef426666001->setDestEndpoint(dstPt426666001);
    connRef426666001->setRoutingType((ConnType)2);

    ConnRef *connRef795762792 = new ConnRef(router, 795762792);
    ConnEnd srcPt795762792(Point(51650, 51210), 1);
    connRef795762792->setSourceEndpoint(srcPt795762792);
    ConnEnd dstPt795762792(shapeRef223927132, 2147483646);
    connRef795762792->setDestEndpoint(dstPt795762792);
    connRef795762792->setRoutingType((ConnType)2);

    ConnRef *connRef467016913 = new ConnRef(router, 467016913);
    ConnEnd srcPt467016913(shapeRef265539540);
    connRef467016913->setSourceEndpoint(srcPt467016913);
    ConnEnd dstPt467016913(Point(51575, 51010), 1);
    connRef467016913->setDestEndpoint(dstPt467016913);
    connRef467016913->setRoutingType((ConnType)2);

    ConnRef *connRef45740440 = new ConnRef(router, 45740440);
    ConnEnd srcPt45740440(Point(51835, 50975), 4);
    connRef45740440->setSourceEndpoint(srcPt45740440);
    ConnEnd dstPt45740440(shapeRef265539540);
    connRef45740440->setDestEndpoint(dstPt45740440);
    connRef45740440->setRoutingType((ConnType)2);

    ConnRef *connRef985644192 = new ConnRef(router, 985644192);
    ConnEnd srcPt985644192(Point(51515, 50975), 8);
    connRef985644192->setSourceEndpoint(srcPt985644192);
    ConnEnd dstPt985644192(shapeRef265539540);
    connRef985644192->setDestEndpoint(dstPt985644192);
    connRef985644192->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/connectionpin03");
    delete router;
    return 0;
}

