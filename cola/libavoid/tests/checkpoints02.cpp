// This is a regression test checking for a problem where orthogonal c-bends 
// due to checkpoints were being incorrectly limited in one dimension as if 
// they were bending around an obstacle. 

#include <cmath>

#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
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
    
    Polygon poly5(4);
    poly5.ps[0] = Point(365.3425124847556, 152.0752314071785);
    poly5.ps[1] = Point(365.3425124847556, 214.0752314071785);
    poly5.ps[2] = Point(303.3425124847556, 214.0752314071785);
    poly5.ps[3] = Point(303.3425124847556, 152.0752314071785);
    new ShapeRef(router, poly5, 5);
    
    Polygon poly7(4);
    poly7.ps[0] = Point(365.3425124847556, 24.07523140617849);
    poly7.ps[1] = Point(365.3425124847556, 86.07523140617849);
    poly7.ps[2] = Point(303.3425124847556, 86.07523140617849);
    poly7.ps[3] = Point(303.3425124847556, 24.07523140617849);
    new ShapeRef(router, poly7, 7);

    Polygon poly8(4);
    poly8.ps[0] = Point(541.6758458190889, 24.07523140617849);
    poly8.ps[1] = Point(541.6758458190889, 44.07523140617849);
    poly8.ps[2] = Point(509.6758458190889, 44.07523140617849);
    poly8.ps[3] = Point(509.6758458190889, 24.07523140617849);
    new ShapeRef(router, poly8, 8);

    Polygon poly10(4);
    poly10.ps[0] = Point(541.6758458190889, 66.07523140617849);
    poly10.ps[1] = Point(541.6758458190889, 86.07523140617849);
    poly10.ps[2] = Point(509.6758458190889, 86.07523140617849);
    poly10.ps[3] = Point(509.6758458190889, 66.07523140617849);
    new ShapeRef(router, poly10, 10);

    Polygon poly61(4);
    poly61.ps[0] = Point(420.3425124847556, 101.0752314061785);
    poly61.ps[1] = Point(420.3425124847556, 141.0752314061785);
    poly61.ps[2] = Point(344.3425124847556, 141.0752314061785);
    poly61.ps[3] = Point(344.3425124847556, 101.0752314061785);
    new ShapeRef(router, poly61, 61);

    Polygon poly92(4);
    poly92.ps[0] = Point(563.5758458190888, 34.07523140617849);
    poly92.ps[1] = Point(563.5758458190888, 76.07523140617849);
    poly92.ps[2] = Point(487.7758458190889, 76.07523140617849);
    poly92.ps[3] = Point(487.7758458190889, 34.07523140617849);
    new ShapeRef(router, poly92, 92);

    ConnRef *connRef109 = new ConnRef(router, 109);
    ConnEnd srcPt109(Point(510.6758458190889, 34.07523140617849), 4);
    connRef109->setSourceEndpoint(srcPt109);
    ConnEnd dstPt109(Point(334.3425124847556, 183.0752314071785), 15);
    connRef109->setDestEndpoint(dstPt109);
    connRef109->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints109(1);
    checkpoints109[0] = Checkpoint(Point(487.6758458190889, 55.07523140617849), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef109->setRoutingCheckpoints(checkpoints109);

    ConnRef *connRef110 = new ConnRef(router, 110);
    ConnEnd srcPt110(Point(510.6758458190889, 76.07523140617849), 4);
    connRef110->setSourceEndpoint(srcPt110);
    ConnEnd dstPt110(Point(334.3425124847556, 183.0752314071785), 15);
    connRef110->setDestEndpoint(dstPt110);
    connRef110->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints110(1);
    checkpoints110[0] = Checkpoint(Point(487.6758458190889, 55.07523140617849), (ConnDirFlags) 15, (ConnDirFlags) 4);
    connRef110->setRoutingCheckpoints(checkpoints110);

    router->processTransaction();
    router->outputDiagram("output/checkpoints02");

    bool succeeded = true;
    double checkpointY = 55.07523140617849;
    Avoid::PolyLine route109 = connRef109->displayRoute();
    if (fabs(route109.ps[route109.size() - 3].y - checkpointY) < 2)
    {
        succeeded = false;
    }

    Avoid::PolyLine route110 = connRef110->displayRoute();
    if (fabs(route110.ps[route110.size() - 3].y - checkpointY) < 2)
    {
        succeeded = false;
    }

    delete router;
    return (succeeded) ? 0 : 1;
};
