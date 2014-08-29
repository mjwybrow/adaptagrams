#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(
            PolyLineRouting | OrthogonalRouting);
    router->setRoutingPenalty((PenaltyType)0, 50);
    router->setRoutingPenalty((PenaltyType)1, 0);
    router->setRoutingPenalty((PenaltyType)2, 200);
    router->setRoutingPenalty((PenaltyType)3, 4000);
    //router->setRoutingPenalty((PenaltyType)4, 110);
    router->setRoutingParameter(idealNudgingDistance, 25);

    double buffer = 4;
    Polygon poly342721632(4);
    poly342721632.ps[0] = Point(51910, 50946);
    poly342721632.ps[1] = Point(51910, 51004);
    poly342721632.ps[2] = Point(51825, 51004);
    poly342721632.ps[3] = Point(51825, 50946);
    ShapeRef *shapeRef342721632 = new ShapeRef(router, poly342721632, 342721632);
    new ShapeConnectionPin(shapeRef342721632, 1, 
            ATTACH_POS_LEFT, ATTACH_POS_CENTRE, true, buffer, ConnDirLeft);

    Polygon poly807558175(4);
    poly807558175.ps[0] = Point(51604, 51000);
    poly807558175.ps[1] = Point(51604, 51085);
    poly807558175.ps[2] = Point(51546, 51085);
    poly807558175.ps[3] = Point(51546, 51000);
    ShapeRef *shapeRef807558175 = new ShapeRef(router, poly807558175, 807558175);
    new ShapeConnectionPin(shapeRef807558175, 1, 
	    ATTACH_POS_CENTRE, ATTACH_POS_TOP, true, buffer, ConnDirNone);

    Polygon poly404900496(4);
    poly404900496.ps[0] = Point(51525, 50946);
    poly404900496.ps[1] = Point(51525, 51004);
    poly404900496.ps[2] = Point(51440, 51004);
    poly404900496.ps[3] = Point(51440, 50946);
    ShapeRef *shapeRef404900496 = new ShapeRef(router, poly404900496, 404900496);
    new ShapeConnectionPin(shapeRef404900496, 1, 
	    ATTACH_POS_RIGHT, ATTACH_POS_CENTRE, true, buffer, (ConnDirFlags) 8);

    JunctionRef *junctionRef265539540 = new JunctionRef(router, Point(51625, 50950), 265539540);

    ConnRef *connRef985644192 = new ConnRef(router, 985644192);
    ConnEnd srcPt985644192(shapeRef404900496, 1);
    connRef985644192->setSourceEndpoint(srcPt985644192);
    ConnEnd dstPt985644192(junctionRef265539540);
    connRef985644192->setDestEndpoint(dstPt985644192);
    connRef985644192->setRoutingType((ConnType)2);

    ConnRef *connRef467016913 = new ConnRef(router, 467016913);
    ConnEnd srcPt467016913(junctionRef265539540);
    connRef467016913->setSourceEndpoint(srcPt467016913);
    ConnEnd dstPt467016913(shapeRef807558175, 1);
    connRef467016913->setDestEndpoint(dstPt467016913);
    connRef467016913->setRoutingType((ConnType)2);

    ConnRef *connRef45740440 = new ConnRef(router, 45740440);
    ConnEnd srcPt45740440(shapeRef342721632, 1);
    connRef45740440->setSourceEndpoint(srcPt45740440);
    ConnEnd dstPt45740440(junctionRef265539540);
    connRef45740440->setDestEndpoint(dstPt45740440);
    connRef45740440->setRoutingType((ConnType)2);

    router->processTransaction();
    router->outputDiagram("output/node1");
    delete router;
    return 0;
};
