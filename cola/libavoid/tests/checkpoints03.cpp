// This is a regression test for a problem where free segments slightly offset
// from effectively fixed segments going through a checkpoint did not get 
// aligned.
#include <cmath>
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

    Polygon poly41(4);
    poly41.ps[0] = Point(918.6758458220888, 441.0752314081785);
    poly41.ps[1] = Point(918.6758458220888, 503.0752314081785);
    poly41.ps[2] = Point(856.6758458220888, 503.0752314081785);
    poly41.ps[3] = Point(856.6758458220888, 441.0752314081785);
    new ShapeRef(router, poly41, 41);


    Polygon poly43(4);
    poly43.ps[0] = Point(710.6758458200889, 441.0752314081785);
    poly43.ps[1] = Point(710.6758458200889, 461.0752314081785);
    poly43.ps[2] = Point(678.6758458200889, 461.0752314081785);
    poly43.ps[3] = Point(678.6758458200889, 441.0752314081785);
    new ShapeRef(router, poly43, 43);

    Polygon poly44(4);
    poly44.ps[0] = Point(710.6758458200889, 483.0752314081785);
    poly44.ps[1] = Point(710.6758458200889, 503.0752314081785);
    poly44.ps[2] = Point(678.6758458200889, 503.0752314081785);
    poly44.ps[3] = Point(678.6758458200889, 483.0752314081785);
    new ShapeRef(router, poly44, 44);

    ConnRef *connRef149 = new ConnRef(router, 149);
    ConnEnd srcPt149(Point(709.6758458200889, 451.0752314081785), 8);
    connRef149->setSourceEndpoint(srcPt149);
    ConnEnd dstPt149(Point(887.6758458220888, 472.0752314081786), 15);
    connRef149->setDestEndpoint(dstPt149);
    connRef149->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints149(1);
    checkpoints149[0] = Checkpoint(Point(732.6758458200889, 472.0752314081785), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef149->setRoutingCheckpoints(checkpoints149);

    ConnRef *connRef150 = new ConnRef(router, 150);
    ConnEnd srcPt150(Point(709.6758458200889, 493.0752314081785), 8);
    connRef150->setSourceEndpoint(srcPt150);
    ConnEnd dstPt150(Point(887.6758458220888, 472.0752314081786), 15);
    connRef150->setDestEndpoint(dstPt150);
    connRef150->setRoutingType((ConnType)2);
    std::vector<Checkpoint> checkpoints150(1);
    checkpoints150[0] = Checkpoint(Point(732.6758458200889, 472.0752314081785), (ConnDirFlags) 15, (ConnDirFlags) 8);
    connRef150->setRoutingCheckpoints(checkpoints150);

    router->processTransaction();
    router->outputDiagram("output/checkpoints03");

    bool succeeded = true;
    Avoid::PolyLine route149 = connRef149->displayRoute();
    Avoid::PolyLine route150 = connRef150->displayRoute();
    if (route149.size() > 4 || route150.size() > 4)
    {
	// Five segments weren't merged into three.  The segments in the shape
	// on right weren't shifted to align with the segments going through
	// the checkpoint.
	succeeded = false;
    }
    if (succeeded == true)
    {
	    // The segments were merged by were not centred around the 
	    // checkpoint.
	    double checkpointY = 472.0752314081785;
	    if (fabs(route149.ps[route149.size() - 1].y - checkpointY) < 2)
	    {
		succeeded = false;
	    }

	    if (fabs(route150.ps[route150.size() - 1].y - checkpointY) < 2)
	    {
		succeeded = false;
	    }
    }
    delete router;
    return (succeeded) ? 0 : 1;
};

