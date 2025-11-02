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
    router->setRoutingPenalty((PenaltyType)5, 100);
    router->setRoutingOption((RoutingOption)0, false);
    router->setRoutingOption((RoutingOption)1, true);
    router->setRoutingParameter(idealNudgingDistance, 25);

    Polygon poly3075375(4);
    poly3075375.ps[0] = Point(51925, 52070);
    poly3075375.ps[1] = Point(51925, 52130);
    poly3075375.ps[2] = Point(51725, 52130);
    poly3075375.ps[3] = Point(51725, 52070);
    new ShapeRef(router, poly3075375, 3075375);

    Polygon poly101599344(4);
    poly101599344.ps[0] = Point(50275, 52410);
    poly101599344.ps[1] = Point(50275, 52490);
    poly101599344.ps[2] = Point(50075, 52490);
    poly101599344.ps[3] = Point(50075, 52410);
    new ShapeRef(router, poly101599344, 101599344);

    Polygon poly127957550(4);
    poly127957550.ps[0] = Point(52010, 51617);
    poly127957550.ps[1] = Point(52010, 51683);
    poly127957550.ps[2] = Point(51890, 51683);
    poly127957550.ps[3] = Point(51890, 51617);
    ShapeRef *shapeRef127957550 = new ShapeRef(router, poly127957550, 127957550);
    new ShapeConnectionPin(shapeRef127957550, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 1);
    new ShapeConnectionPin(shapeRef127957550, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 2);
    new ShapeConnectionPin(shapeRef127957550, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef127957550, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 8);

    Polygon poly129612347(4);
    poly129612347.ps[0] = Point(51900, 51696);
    poly129612347.ps[1] = Point(51900, 51754);
    poly129612347.ps[2] = Point(51815, 51754);
    poly129612347.ps[3] = Point(51815, 51696);
    new ShapeRef(router, poly129612347, 129612347);

    Polygon poly203389050(4);
    poly203389050.ps[0] = Point(51825, 52260);
    poly203389050.ps[1] = Point(51825, 52340);
    poly203389050.ps[2] = Point(51625, 52340);
    poly203389050.ps[3] = Point(51625, 52260);
    new ShapeRef(router, poly203389050, 203389050);

    Polygon poly208676910(4);
    poly208676910.ps[0] = Point(51950, 51760);
    poly208676910.ps[1] = Point(51950, 51840);
    poly208676910.ps[2] = Point(51750, 51840);
    poly208676910.ps[3] = Point(51750, 51760);
    new ShapeRef(router, poly208676910, 208676910);

    Polygon poly219117032(4);
    poly219117032.ps[0] = Point(50225, 52070);
    poly219117032.ps[1] = Point(50225, 52130);
    poly219117032.ps[2] = Point(50025, 52130);
    poly219117032.ps[3] = Point(50025, 52070);
    new ShapeRef(router, poly219117032, 219117032);

    Polygon poly258454350(4);
    poly258454350.ps[0] = Point(50185, 51617);
    poly258454350.ps[1] = Point(50185, 51683);
    poly258454350.ps[2] = Point(50065, 51683);
    poly258454350.ps[3] = Point(50065, 51617);
    ShapeRef *shapeRef258454350 = new ShapeRef(router, poly258454350, 258454350);
    new ShapeConnectionPin(shapeRef258454350, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 1);
    new ShapeConnectionPin(shapeRef258454350, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 2);
    new ShapeConnectionPin(shapeRef258454350, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 4);
    new ShapeConnectionPin(shapeRef258454350, 2147483646, 0.5, 0.5, true, 0, (ConnDirFlags) 8);

    Polygon poly274504590(4);
    poly274504590.ps[0] = Point(51400, 52470);
    poly274504590.ps[1] = Point(51400, 52530);
    poly274504590.ps[2] = Point(51150, 52530);
    poly274504590.ps[3] = Point(51150, 52470);
    new ShapeRef(router, poly274504590, 274504590);

    Polygon poly320957826(4);
    poly320957826.ps[0] = Point(50275, 51910);
    poly320957826.ps[1] = Point(50275, 51990);
    poly320957826.ps[2] = Point(50075, 51990);
    poly320957826.ps[3] = Point(50075, 51910);
    new ShapeRef(router, poly320957826, 320957826);

    Polygon poly368436384(4);
    poly368436384.ps[0] = Point(50325, 51696);
    poly368436384.ps[1] = Point(50325, 51754);
    poly368436384.ps[2] = Point(50240, 51754);
    poly368436384.ps[3] = Point(50240, 51696);
    new ShapeRef(router, poly368436384, 368436384);

    Polygon poly381499178(4);
    poly381499178.ps[0] = Point(51850, 51345);
    poly381499178.ps[1] = Point(51850, 51405);
    poly381499178.ps[2] = Point(51650, 51405);
    poly381499178.ps[3] = Point(51650, 51345);
    new ShapeRef(router, poly381499178, 381499178);

    Polygon poly426410694(4);
    poly426410694.ps[0] = Point(50325, 52270);
    poly426410694.ps[1] = Point(50325, 52330);
    poly426410694.ps[2] = Point(50125, 52330);
    poly426410694.ps[3] = Point(50125, 52270);
    new ShapeRef(router, poly426410694, 426410694);

    Polygon poly428912890(4);
    poly428912890.ps[0] = Point(51825, 51510);
    poly428912890.ps[1] = Point(51825, 51590);
    poly428912890.ps[2] = Point(51625, 51590);
    poly428912890.ps[3] = Point(51625, 51510);
    new ShapeRef(router, poly428912890, 428912890);

    Polygon poly468897320(4);
    poly468897320.ps[0] = Point(50425, 51520);
    poly468897320.ps[1] = Point(50425, 51580);
    poly468897320.ps[2] = Point(50225, 51580);
    poly468897320.ps[3] = Point(50225, 51520);
    new ShapeRef(router, poly468897320, 468897320);

    Polygon poly517518507(4);
    poly517518507.ps[0] = Point(51925, 51910);
    poly517518507.ps[1] = Point(51925, 51990);
    poly517518507.ps[2] = Point(51725, 51990);
    poly517518507.ps[3] = Point(51725, 51910);
    new ShapeRef(router, poly517518507, 517518507);

    Polygon poly565878858(4);
    poly565878858.ps[0] = Point(51160, 52500);
    poly565878858.ps[1] = Point(51160, 52810);
    poly565878858.ps[2] = Point(50890, 52810);
    poly565878858.ps[3] = Point(50890, 52500);
    new ShapeRef(router, poly565878858, 565878858);

    Polygon poly706147100(4);
    poly706147100.ps[0] = Point(51160, 50990);
    poly706147100.ps[1] = Point(51160, 51300);
    poly706147100.ps[2] = Point(50890, 51300);
    poly706147100.ps[3] = Point(50890, 50990);
    new ShapeRef(router, poly706147100, 706147100);

    Polygon poly746254622(4);
    poly746254622.ps[0] = Point(50275, 51760);
    poly746254622.ps[1] = Point(50275, 51840);
    poly746254622.ps[2] = Point(50075, 51840);
    poly746254622.ps[3] = Point(50075, 51760);
    new ShapeRef(router, poly746254622, 746254622);

    Polygon poly829835240(4);
    poly829835240.ps[0] = Point(50425, 51360);
    poly829835240.ps[1] = Point(50425, 51440);
    poly829835240.ps[2] = Point(50225, 51440);
    poly829835240.ps[3] = Point(50225, 51360);
    new ShapeRef(router, poly829835240, 829835240);

    Polygon poly157548160(4);
    poly157548160.ps[0] = Point(51550, 51500);
    poly157548160.ps[1] = Point(51550, 52225);
    poly157548160.ps[2] = Point(50525, 52225);
    poly157548160.ps[3] = Point(50525, 51500);
    new ShapeRef(router, poly157548160, 157548160);

    ConnRef *connRef6675422 = new ConnRef(router, 6675422);
    ConnEnd srcPt6675422(Point(51100, 51290), 2);
    connRef6675422->setSourceEndpoint(srcPt6675422);
    ConnEnd dstPt6675422(Point(51150, 51510), 1);
    connRef6675422->setDestEndpoint(dstPt6675422);
    connRef6675422->setRoutingType((ConnType)2);

    ConnRef *connRef13259320 = new ConnRef(router, 13259320);
    ConnEnd srcPt13259320(Point(50535, 51675), 4);
    connRef13259320->setSourceEndpoint(srcPt13259320);
    ConnEnd dstPt13259320(Point(50315, 51725), 8);
    connRef13259320->setDestEndpoint(dstPt13259320);
    connRef13259320->setRoutingType((ConnType)2);

    ConnRef *connRef17108208 = new ConnRef(router, 17108208);
    ConnEnd srcPt17108208(Point(51540, 52050), 8);
    connRef17108208->setSourceEndpoint(srcPt17108208);
    ConnEnd dstPt17108208(Point(51735, 52100), 4);
    connRef17108208->setDestEndpoint(dstPt17108208);
    connRef17108208->setRoutingType((ConnType)2);

    ConnRef *connRef41442471 = new ConnRef(router, 41442471);
    ConnEnd srcPt41442471(Point(51075, 52215), 2);
    connRef41442471->setSourceEndpoint(srcPt41442471);
    ConnEnd dstPt41442471(Point(51050, 52510), 1);
    connRef41442471->setDestEndpoint(dstPt41442471);
    connRef41442471->setRoutingType((ConnType)2);

    ConnRef *connRef45863720 = new ConnRef(router, 45863720);
    ConnEnd srcPt45863720(Point(51660, 51375), 4);
    connRef45863720->setSourceEndpoint(srcPt45863720);
    ConnEnd dstPt45863720(Point(51300, 51510), 1);
    connRef45863720->setDestEndpoint(dstPt45863720);
    connRef45863720->setRoutingType((ConnType)2);

    ConnRef *connRef52653348 = new ConnRef(router, 52653348);
    ConnEnd srcPt52653348(Point(50535, 52050), 4);
    connRef52653348->setSourceEndpoint(srcPt52653348);
    ConnEnd dstPt52653348(Point(50215, 52100), 8);
    connRef52653348->setDestEndpoint(dstPt52653348);
    connRef52653348->setRoutingType((ConnType)2);

    ConnRef *connRef57649636 = new ConnRef(router, 57649636);
    ConnEnd srcPt57649636(Point(51540, 52125), 8);
    connRef57649636->setSourceEndpoint(srcPt57649636);
    ConnEnd dstPt57649636(Point(51915, 52100), 8);
    connRef57649636->setDestEndpoint(dstPt57649636);
    connRef57649636->setRoutingType((ConnType)2);

    ConnRef *connRef64735671 = new ConnRef(router, 64735671);
    ConnEnd srcPt64735671(Point(51375, 52215), 2);
    connRef64735671->setSourceEndpoint(srcPt64735671);
    ConnEnd dstPt64735671(Point(51815, 52300), 8);
    connRef64735671->setDestEndpoint(dstPt64735671);
    connRef64735671->setRoutingType((ConnType)2);

    ConnRef *connRef81789576 = new ConnRef(router, 81789576);
    ConnEnd srcPt81789576(Point(50950, 51290), 2);
    connRef81789576->setSourceEndpoint(srcPt81789576);
    ConnEnd dstPt81789576(Point(50925, 51510), 1);
    connRef81789576->setDestEndpoint(dstPt81789576);
    connRef81789576->setRoutingType((ConnType)2);

    ConnRef *connRef87742344 = new ConnRef(router, 87742344);
    ConnEnd srcPt87742344(Point(51540, 51600), 8);
    connRef87742344->setSourceEndpoint(srcPt87742344);
    ConnEnd dstPt87742344(shapeRef127957550, 2147483646);
    connRef87742344->setDestEndpoint(dstPt87742344);
    connRef87742344->setRoutingType((ConnType)2);

    ConnRef *connRef92624219 = new ConnRef(router, 92624219);
    ConnEnd srcPt92624219(Point(51450, 52215), 2);
    connRef92624219->setSourceEndpoint(srcPt92624219);
    ConnEnd dstPt92624219(Point(51635, 52300), 4);
    connRef92624219->setDestEndpoint(dstPt92624219);
    connRef92624219->setRoutingType((ConnType)2);

    ConnRef *connRef126568050 = new ConnRef(router, 126568050);
    ConnEnd srcPt126568050(Point(51540, 51975), 8);
    connRef126568050->setSourceEndpoint(srcPt126568050);
    ConnEnd dstPt126568050(Point(51915, 51950), 8);
    connRef126568050->setDestEndpoint(dstPt126568050);
    connRef126568050->setRoutingType((ConnType)2);

    ConnRef *connRef128113340 = new ConnRef(router, 128113340);
    ConnEnd srcPt128113340(Point(51300, 52215), 2);
    connRef128113340->setSourceEndpoint(srcPt128113340);
    ConnEnd dstPt128113340(Point(51390, 52500), 8);
    connRef128113340->setDestEndpoint(dstPt128113340);
    connRef128113340->setRoutingType((ConnType)2);

    ConnRef *connRef157642749 = new ConnRef(router, 157642749);
    ConnEnd srcPt157642749(Point(51225, 52215), 2);
    connRef157642749->setSourceEndpoint(srcPt157642749);
    // Problem point:
    ConnEnd dstPt157642749(Point(51160, 52500), 4);
    connRef157642749->setDestEndpoint(dstPt157642749);
    connRef157642749->setRoutingType((ConnType)2);

    ConnRef *connRef182970626 = new ConnRef(router, 182970626);
    ConnEnd srcPt182970626(Point(51000, 52215), 2);
    connRef182970626->setSourceEndpoint(srcPt182970626);
    ConnEnd dstPt182970626(Point(51000, 52510), 1);
    connRef182970626->setDestEndpoint(dstPt182970626);
    connRef182970626->setRoutingType((ConnType)2);

    ConnRef *connRef191618599 = new ConnRef(router, 191618599);
    ConnEnd srcPt191618599(Point(50535, 51900), 4);
    connRef191618599->setSourceEndpoint(srcPt191618599);
    ConnEnd dstPt191618599(Point(50265, 51950), 8);
    connRef191618599->setDestEndpoint(dstPt191618599);
    connRef191618599->setRoutingType((ConnType)2);

    ConnRef *connRef215646300 = new ConnRef(router, 215646300);
    ConnEnd srcPt215646300(Point(50415, 51400), 8);
    connRef215646300->setSourceEndpoint(srcPt215646300);
    ConnEnd dstPt215646300(Point(50775, 51510), 1);
    connRef215646300->setDestEndpoint(dstPt215646300);
    connRef215646300->setRoutingType((ConnType)2);

    ConnRef *connRef217181718 = new ConnRef(router, 217181718);
    ConnEnd srcPt217181718(Point(51000, 51290), 2);
    connRef217181718->setSourceEndpoint(srcPt217181718);
    ConnEnd dstPt217181718(Point(51000, 51510), 1);
    connRef217181718->setDestEndpoint(dstPt217181718);
    connRef217181718->setRoutingType((ConnType)2);

    ConnRef *connRef217236398 = new ConnRef(router, 217236398);
    ConnEnd srcPt217236398(Point(50700, 52215), 2);
    connRef217236398->setSourceEndpoint(srcPt217236398);
    ConnEnd dstPt217236398(Point(50135, 52300), 4);
    connRef217236398->setDestEndpoint(dstPt217236398);
    connRef217236398->setRoutingType((ConnType)2);

    ConnRef *connRef227676560 = new ConnRef(router, 227676560);
    ConnEnd srcPt227676560(Point(50850, 51510), 1);
    connRef227676560->setSourceEndpoint(srcPt227676560);
    ConnEnd dstPt227676560(Point(50235, 51400), 4);
    connRef227676560->setDestEndpoint(dstPt227676560);
    connRef227676560->setRoutingType((ConnType)2);

    ConnRef *connRef235740708 = new ConnRef(router, 235740708);
    ConnEnd srcPt235740708(Point(51540, 51675), 8);
    connRef235740708->setSourceEndpoint(srcPt235740708);
    ConnEnd dstPt235740708(Point(51870, 51725), 8);
    connRef235740708->setDestEndpoint(dstPt235740708);
    connRef235740708->setRoutingType((ConnType)2);

    ConnRef *connRef307166980 = new ConnRef(router, 307166980);
    ConnEnd srcPt307166980(Point(50925, 52215), 2);
    connRef307166980->setSourceEndpoint(srcPt307166980);
    ConnEnd dstPt307166980(Point(50950, 52510), 1);
    connRef307166980->setDestEndpoint(dstPt307166980);
    connRef307166980->setRoutingType((ConnType)2);

    ConnRef *connRef314353850 = new ConnRef(router, 314353850);
    ConnEnd srcPt314353850(Point(51050, 51290), 2);
    connRef314353850->setSourceEndpoint(srcPt314353850);
    ConnEnd dstPt314353850(Point(51075, 51510), 1);
    connRef314353850->setDestEndpoint(dstPt314353850);
    connRef314353850->setRoutingType((ConnType)2);

    ConnRef *connRef356343444 = new ConnRef(router, 356343444);
    ConnEnd srcPt356343444(Point(50625, 51510), 1);
    connRef356343444->setSourceEndpoint(srcPt356343444);
    ConnEnd dstPt356343444(Point(50415, 51550), 8);
    connRef356343444->setDestEndpoint(dstPt356343444);
    connRef356343444->setRoutingType((ConnType)2);

    ConnRef *connRef401560509 = new ConnRef(router, 401560509);
    ConnEnd srcPt401560509(Point(50625, 52215), 2);
    connRef401560509->setSourceEndpoint(srcPt401560509);
    ConnEnd dstPt401560509(Point(50315, 52300), 8);
    connRef401560509->setDestEndpoint(dstPt401560509);
    connRef401560509->setRoutingType((ConnType)2);

    ConnRef *connRef413072940 = new ConnRef(router, 413072940);
    ConnEnd srcPt413072940(Point(51840, 51375), 8);
    connRef413072940->setSourceEndpoint(srcPt413072940);
    ConnEnd dstPt413072940(Point(51225, 51510), 1);
    connRef413072940->setDestEndpoint(dstPt413072940);
    connRef413072940->setRoutingType((ConnType)2);

    ConnRef *connRef413766381 = new ConnRef(router, 413766381);
    ConnEnd srcPt413766381(Point(50535, 51750), 4);
    connRef413766381->setSourceEndpoint(srcPt413766381);
    ConnEnd dstPt413766381(Point(50265, 51800), 8);
    connRef413766381->setDestEndpoint(dstPt413766381);
    connRef413766381->setRoutingType((ConnType)2);

    ConnRef *connRef422359365 = new ConnRef(router, 422359365);
    ConnEnd srcPt422359365(Point(50535, 51825), 4);
    connRef422359365->setSourceEndpoint(srcPt422359365);
    ConnEnd dstPt422359365(Point(50085, 51800), 4);
    connRef422359365->setDestEndpoint(dstPt422359365);
    connRef422359365->setRoutingType((ConnType)2);

    ConnRef *connRef427415850 = new ConnRef(router, 427415850);
    ConnEnd srcPt427415850(Point(50775, 52215), 2);
    connRef427415850->setSourceEndpoint(srcPt427415850);
    ConnEnd dstPt427415850(Point(50265, 52450), 8);
    connRef427415850->setDestEndpoint(dstPt427415850);
    connRef427415850->setRoutingType((ConnType)2);

    ConnRef *connRef444649182 = new ConnRef(router, 444649182);
    ConnEnd srcPt444649182(Point(51540, 51750), 8);
    connRef444649182->setSourceEndpoint(srcPt444649182);
    ConnEnd dstPt444649182(Point(51760, 51800), 4);
    connRef444649182->setDestEndpoint(dstPt444649182);
    connRef444649182->setRoutingType((ConnType)2);

    ConnRef *connRef505297694 = new ConnRef(router, 505297694);
    ConnEnd srcPt505297694(Point(50700, 51510), 1);
    connRef505297694->setSourceEndpoint(srcPt505297694);
    ConnEnd dstPt505297694(Point(50235, 51550), 4);
    connRef505297694->setDestEndpoint(dstPt505297694);
    connRef505297694->setRoutingType((ConnType)2);

    ConnRef *connRef528809632 = new ConnRef(router, 528809632);
    ConnEnd srcPt528809632(Point(51540, 51900), 8);
    connRef528809632->setSourceEndpoint(srcPt528809632);
    ConnEnd dstPt528809632(Point(51735, 51950), 4);
    connRef528809632->setDestEndpoint(dstPt528809632);
    connRef528809632->setRoutingType((ConnType)2);

    ConnRef *connRef548127810 = new ConnRef(router, 548127810);
    ConnEnd srcPt548127810(Point(50535, 51975), 4);
    connRef548127810->setSourceEndpoint(srcPt548127810);
    ConnEnd dstPt548127810(Point(50085, 51950), 4);
    connRef548127810->setDestEndpoint(dstPt548127810);
    connRef548127810->setRoutingType((ConnType)2);

    ConnRef *connRef648819584 = new ConnRef(router, 648819584);
    ConnEnd srcPt648819584(Point(51540, 51825), 8);
    connRef648819584->setSourceEndpoint(srcPt648819584);
    ConnEnd dstPt648819584(Point(51940, 51800), 8);
    connRef648819584->setDestEndpoint(dstPt648819584);
    connRef648819584->setRoutingType((ConnType)2);

    ConnRef *connRef651183694 = new ConnRef(router, 651183694);
    ConnEnd srcPt651183694(Point(51635, 51550), 4);
    connRef651183694->setSourceEndpoint(srcPt651183694);
    ConnEnd dstPt651183694(Point(51450, 51510), 1);
    connRef651183694->setDestEndpoint(dstPt651183694);
    connRef651183694->setRoutingType((ConnType)2);

    ConnRef *connRef655025700 = new ConnRef(router, 655025700);
    ConnEnd srcPt655025700(Point(51150, 52215), 2);
    connRef655025700->setSourceEndpoint(srcPt655025700);
    ConnEnd dstPt655025700(Point(51100, 52510), 1);
    connRef655025700->setDestEndpoint(dstPt655025700);
    connRef655025700->setRoutingType((ConnType)2);

    ConnRef *connRef662151765 = new ConnRef(router, 662151765);
    ConnEnd srcPt662151765(Point(50535, 51600), 4);
    connRef662151765->setSourceEndpoint(srcPt662151765);
    ConnEnd dstPt662151765(shapeRef258454350, 2147483646);
    connRef662151765->setDestEndpoint(dstPt662151765);
    connRef662151765->setRoutingType((ConnType)2);

    ConnRef *connRef678159455 = new ConnRef(router, 678159455);
    ConnEnd srcPt678159455(Point(51815, 51550), 8);
    connRef678159455->setSourceEndpoint(srcPt678159455);
    ConnEnd dstPt678159455(Point(51375, 51510), 1);
    connRef678159455->setDestEndpoint(dstPt678159455);
    connRef678159455->setRoutingType((ConnType)2);

    ConnRef *connRef803304040 = new ConnRef(router, 803304040);
    ConnEnd srcPt803304040(Point(50850, 52215), 2);
    connRef803304040->setSourceEndpoint(srcPt803304040);
    ConnEnd dstPt803304040(Point(50085, 52450), 4);
    connRef803304040->setDestEndpoint(dstPt803304040);
    connRef803304040->setRoutingType((ConnType)2);

    ConnRef *connRef960638799 = new ConnRef(router, 960638799);
    ConnEnd srcPt960638799(Point(50535, 52125), 4);
    connRef960638799->setSourceEndpoint(srcPt960638799);
    ConnEnd dstPt960638799(Point(50035, 52100), 4);
    connRef960638799->setDestEndpoint(dstPt960638799);
    connRef960638799->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/validPaths01");
    int result = router->existsInvalidOrthogonalPaths();
    delete router;
    return result;
};
