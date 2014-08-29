#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
    router->setRoutingPenalty(segmentPenalty, 50);
    router->setRoutingParameter(idealNudgingDistance, 10);
    Rectangle rect47(Point(51145, 50195), Point(51145+360, 50195+685));
    new ShapeRef(router, rect47);
    Rectangle rect46(Point(51920, 49770), Point(51920+360, 49770+310));
    new ShapeRef(router, rect46);
    Rectangle rect45(Point(51151, 49770), Point(51151+360, 49770+310));
    new ShapeRef(router, rect45);
    ConnEnd srcPt53(Point(51150,49850), 4);
    ConnEnd dstPt53(Point(51500,50350), 8);
    new ConnRef(router, srcPt53, dstPt53);
    ConnEnd srcPt52(Point(52275,49850), 8);
    ConnEnd dstPt52(Point(51150,49925), 4);
    new ConnRef(router, srcPt52, dstPt52);
    ConnEnd srcPt6(Point(51150,50500), 4);
    ConnEnd dstPt6(Point(51925,50000), 4);
    new ConnRef(router, srcPt6, dstPt6, 6);
    router->processTransaction();
    router->outputDiagram("output/infinity");
    delete router;
    return 0;
};
