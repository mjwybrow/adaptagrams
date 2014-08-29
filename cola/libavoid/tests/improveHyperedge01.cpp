#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

int test()
{
	// The contents of this function can be replaced by the automatically generated test code from the debug svg file
	Avoid::ConnEnd end1;
	Avoid::ConnEnd end2;
	Avoid::Polygon poly;
	Avoid::Router * router = new Avoid::Router(Avoid::OrthogonalRouting);
	router->setTransactionUse(true);
	router->setRoutingPenalty(Avoid::segmentPenalty);
	router->setRoutingPenalty(Avoid::crossingPenalty);
	router->setRoutingPenalty(Avoid::fixedSharedPathPenalty);
	router->setRoutingParameter(idealNudgingDistance, 25);
	router->setRoutingOption(Avoid::improveHyperedgeRoutesMovingJunctions, true);
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(51160, 50670));
	poly.setPoint(1, Avoid::Point(51160, 50730));
	poly.setPoint(2, Avoid::Point(50950, 50730));
	poly.setPoint(3, Avoid::Point(50950, 50670));
	Avoid::ShapeRef * shape341041314 = new Avoid::ShapeRef(router, poly, 341041314);
	Avoid::ShapeConnectionPin * pin341041314_0 = new Avoid::ShapeConnectionPin(shape341041314, 1, 0.952381, 0.5, true, 10, 8);
	pin341041314_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin341041314_1 = new Avoid::ShapeConnectionPin(shape341041314, 2, 0, 0.5, true, 10, 4);
	pin341041314_1->setExclusive(true);
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(51160, 50395));
	poly.setPoint(1, Avoid::Point(51160, 50455));
	poly.setPoint(2, Avoid::Point(50950, 50455));
	poly.setPoint(3, Avoid::Point(50950, 50395));
	Avoid::ShapeRef * shape47540376 = new Avoid::ShapeRef(router, poly, 47540376);
	Avoid::ShapeConnectionPin * pin47540376_0 = new Avoid::ShapeConnectionPin(shape47540376, 1, 0.952381, 0.5, true, 10, 8);
	pin47540376_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin47540376_1 = new Avoid::ShapeConnectionPin(shape47540376, 2, 0, 0.5, true, 10, 4);
	pin47540376_1->setExclusive(true);
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(50850, 50770));
	poly.setPoint(1, Avoid::Point(50850, 50830));
	poly.setPoint(2, Avoid::Point(50665, 50830));
	poly.setPoint(3, Avoid::Point(50665, 50770));
	Avoid::ShapeRef * shape81920772 = new Avoid::ShapeRef(router, poly, 81920772);
	Avoid::ShapeConnectionPin * pin81920772_0 = new Avoid::ShapeConnectionPin(shape81920772, 1, 0.047619, 0.5, true, 10, 4);
	pin81920772_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin81920772_1 = new Avoid::ShapeConnectionPin(shape81920772, 2, 1, 0.5, true, 10, 8);
	pin81920772_1->setExclusive(true);
	Avoid::JunctionRef * junction878282342 = new Avoid::JunctionRef(router, Avoid::Point(50875, 50700), 878282342);
	junction878282342->setPositionFixed(false);
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(50850, 50470));
	poly.setPoint(1, Avoid::Point(50850, 50530));
	poly.setPoint(2, Avoid::Point(50665, 50530));
	poly.setPoint(3, Avoid::Point(50665, 50470));
	Avoid::ShapeRef * shape206564314 = new Avoid::ShapeRef(router, poly, 206564314);
	Avoid::ShapeConnectionPin * pin206564314_0 = new Avoid::ShapeConnectionPin(shape206564314, 1, 0.047619, 0.5, true, 10, 4);
	pin206564314_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin206564314_1 = new Avoid::ShapeConnectionPin(shape206564314, 2, 1, 0.5, true, 10, 8);
	pin206564314_1->setExclusive(true);
	Avoid::JunctionRef * junction213455341 = new Avoid::JunctionRef(router, Avoid::Point(50875, 50500), 213455341);
	junction213455341->setPositionFixed(false);
	end1 = Avoid::ConnEnd(junction878282342);
	end2 = Avoid::ConnEnd(shape341041314, 2);
	Avoid::ConnRef * conn290033688 = new Avoid::ConnRef(router, end1, end2);
	conn290033688->makePathInvalid();
	end1 = Avoid::ConnEnd(shape81920772, 2);
	end2 = Avoid::ConnEnd(junction878282342);
	Avoid::ConnRef * conn281807178 = new Avoid::ConnRef(router, end1, end2);
	conn281807178->makePathInvalid();
	end1 = Avoid::ConnEnd(shape47540376, 2);
	end2 = Avoid::ConnEnd(junction213455341);
	Avoid::ConnRef * conn762966660 = new Avoid::ConnRef(router, end1, end2);
	conn762966660->makePathInvalid();
	end1 = Avoid::ConnEnd(junction213455341);
	end2 = Avoid::ConnEnd(junction878282342);
	Avoid::ConnRef * conn46508714 = new Avoid::ConnRef(router, end1, end2);
	conn46508714->makePathInvalid();
	end1 = Avoid::ConnEnd(shape206564314, 2);
	end2 = Avoid::ConnEnd(junction213455341);
	Avoid::ConnRef * conn385831514 = new Avoid::ConnRef(router, end1, end2);
	conn385831514->makePathInvalid();
	router->processTransaction();

	router->outputDiagram("output/improveHyperedge01-1");
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(51160, 50645));
	poly.setPoint(1, Avoid::Point(51160, 50705));
	poly.setPoint(2, Avoid::Point(50950, 50705));
	poly.setPoint(3, Avoid::Point(50950, 50645));
	router->moveShape(shape341041314, poly);
	conn290033688->makePathInvalid();
	conn281807178->makePathInvalid();
	conn46508714->makePathInvalid();
	conn762966660->makePathInvalid();
	router->processTransaction();

	router->outputDiagram("output/improveHyperedge01-2");
	Avoid::Point junctionPos = junction878282342->recommendedPosition();

	delete router;
	
	// Regression test to check that a junction is correctly moved.
	return (junctionPos.y == 50675) ? 0 : 1; 
}

int main(void)
{
	return test();
}
