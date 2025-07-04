#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

int test()
{
	// The contents of this function can be replaced by the automatically generated test code from the debug svg file
	ConnEnd end1;
	ConnEnd end2;
	Polygon poly;
	Router * router = new Router(OrthogonalRouting);
	router->setTransactionUse(true);
	router->setRoutingPenalty(segmentPenalty);
	router->setRoutingPenalty(crossingPenalty);
	router->setRoutingPenalty(fixedSharedPathPenalty);
	router->setRoutingParameter(idealNudgingDistance, 25);
	router->setRoutingOption(improveHyperedgeRoutesMovingJunctions, true);
	poly = Polygon(4);
	poly.setPoint(0, Point(51160, 50670));
	poly.setPoint(1, Point(51160, 50730));
	poly.setPoint(2, Point(50950, 50730));
	poly.setPoint(3, Point(50950, 50670));
	ShapeRef * shape341041314 = new ShapeRef(router, poly, 341041314);
	ShapeConnectionPin * pin341041314_0 = new ShapeConnectionPin(shape341041314, 1, 0.952381, 0.5, true, 10, 8);
	pin341041314_0->setExclusive(true);
	ShapeConnectionPin * pin341041314_1 = new ShapeConnectionPin(shape341041314, 2, 0, 0.5, true, 10, 4);
	pin341041314_1->setExclusive(true);
	poly = Polygon(4);
	poly.setPoint(0, Point(51160, 50395));
	poly.setPoint(1, Point(51160, 50455));
	poly.setPoint(2, Point(50950, 50455));
	poly.setPoint(3, Point(50950, 50395));
	ShapeRef * shape47540376 = new ShapeRef(router, poly, 47540376);
	ShapeConnectionPin * pin47540376_0 = new ShapeConnectionPin(shape47540376, 1, 0.952381, 0.5, true, 10, 8);
	pin47540376_0->setExclusive(true);
	ShapeConnectionPin * pin47540376_1 = new ShapeConnectionPin(shape47540376, 2, 0, 0.5, true, 10, 4);
	pin47540376_1->setExclusive(true);
	poly = Polygon(4);
	poly.setPoint(0, Point(50850, 50770));
	poly.setPoint(1, Point(50850, 50830));
	poly.setPoint(2, Point(50665, 50830));
	poly.setPoint(3, Point(50665, 50770));
	ShapeRef * shape81920772 = new ShapeRef(router, poly, 81920772);
	ShapeConnectionPin * pin81920772_0 = new ShapeConnectionPin(shape81920772, 1, 0.047619, 0.5, true, 10, 4);
	pin81920772_0->setExclusive(true);
	ShapeConnectionPin * pin81920772_1 = new ShapeConnectionPin(shape81920772, 2, 1, 0.5, true, 10, 8);
	pin81920772_1->setExclusive(true);
	JunctionRef * junction878282342 = new JunctionRef(router, Point(50875, 50700), 878282342);
	junction878282342->setPositionFixed(false);
	poly = Polygon(4);
	poly.setPoint(0, Point(50850, 50470));
	poly.setPoint(1, Point(50850, 50530));
	poly.setPoint(2, Point(50665, 50530));
	poly.setPoint(3, Point(50665, 50470));
	ShapeRef * shape206564314 = new ShapeRef(router, poly, 206564314);
	ShapeConnectionPin * pin206564314_0 = new ShapeConnectionPin(shape206564314, 1, 0.047619, 0.5, true, 10, 4);
	pin206564314_0->setExclusive(true);
	ShapeConnectionPin * pin206564314_1 = new ShapeConnectionPin(shape206564314, 2, 1, 0.5, true, 10, 8);
	pin206564314_1->setExclusive(true);
	JunctionRef * junction213455341 = new JunctionRef(router, Point(50875, 50500), 213455341);
	junction213455341->setPositionFixed(false);
	end1 = ConnEnd(junction878282342);
	end2 = ConnEnd(shape341041314, 2);
	ConnRef * conn290033688 = new ConnRef(router, end1, end2);
	conn290033688->makePathInvalid();
	end1 = ConnEnd(shape81920772, 2);
	end2 = ConnEnd(junction878282342);
	ConnRef * conn281807178 = new ConnRef(router, end1, end2);
	conn281807178->makePathInvalid();
	end1 = ConnEnd(shape47540376, 2);
	end2 = ConnEnd(junction213455341);
	ConnRef * conn762966660 = new ConnRef(router, end1, end2);
	conn762966660->makePathInvalid();
	end1 = ConnEnd(junction213455341);
	end2 = ConnEnd(junction878282342);
	ConnRef * conn46508714 = new ConnRef(router, end1, end2);
	conn46508714->makePathInvalid();
	end1 = ConnEnd(shape206564314, 2);
	end2 = ConnEnd(junction213455341);
	ConnRef * conn385831514 = new ConnRef(router, end1, end2);
	conn385831514->makePathInvalid();
	router->processTransaction();

	router->outputDiagram("output/improveHyperedge01-1");
	poly = Polygon(4);
	poly.setPoint(0, Point(51160, 50645));
	poly.setPoint(1, Point(51160, 50705));
	poly.setPoint(2, Point(50950, 50705));
	poly.setPoint(3, Point(50950, 50645));
	router->moveShape(shape341041314, poly);
	conn290033688->makePathInvalid();
	conn281807178->makePathInvalid();
	conn46508714->makePathInvalid();
	conn762966660->makePathInvalid();
	router->processTransaction();

	router->outputDiagram("output/improveHyperedge01-2");
	Point junctionPos = junction878282342->recommendedPosition();

	delete router;
	
	// Regression test to check that a junction is correctly moved.
	return (junctionPos.y == 50675) ? 0 : 1; 
}

int main(void)
{
	return test();
}
