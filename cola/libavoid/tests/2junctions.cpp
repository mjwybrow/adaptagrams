#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

int test()
{
	Avoid::ConnEnd end1;
	Avoid::ConnEnd end2;
	Avoid::Polygon poly;
	Avoid::Router * router1 = new Avoid::Router(Avoid::OrthogonalRouting);
	router1->setTransactionUse(true);
	router1->setRoutingPenalty(Avoid::segmentPenalty);
	router1->setRoutingPenalty(Avoid::crossingPenalty);
	router1->setRoutingPenalty(Avoid::fixedSharedPathPenalty);
	router1->setRoutingOption(Avoid::penaliseOrthogonalSharedPathsAtConnEnds, true);
	router1->setRoutingParameter(idealNudgingDistance, 25);
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(51410, 50640));
	poly.setPoint(1, Avoid::Point(51410, 50960));
	poly.setPoint(2, Avoid::Point(51050, 50960));
	poly.setPoint(3, Avoid::Point(51050, 50640));
	Avoid::ShapeRef * shape365249936 = new Avoid::ShapeRef(router1, poly, 365249936);
	Avoid::ShapeConnectionPin * pin365249936_2 = new Avoid::ShapeConnectionPin(shape365249936, 3, 0, 0.5, true, 10, 4);
	pin365249936_2->setExclusive(true);
	Avoid::ShapeConnectionPin * pin365249936_4 = new Avoid::ShapeConnectionPin(shape365249936, 5, 0, 0.734375, true, 10, 4);
	pin365249936_4->setExclusive(true);
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(50760, 50691));
	poly.setPoint(1, Avoid::Point(50760, 50775));
	poly.setPoint(2, Avoid::Point(50640, 50775));
	poly.setPoint(3, Avoid::Point(50640, 50691));
	Avoid::ShapeRef * shape386142480 = new Avoid::ShapeRef(router1, poly, 386142480);
	Avoid::ShapeConnectionPin * pin386142480_1 = new Avoid::ShapeConnectionPin(shape386142480, 2, 0.5, 1, true, 10, 2);
	pin386142480_1->setExclusive(true);
	Avoid::JunctionRef * junction328922310 = new Avoid::JunctionRef(router1, Avoid::Point(50700, 50800), 328922310);
	junction328922310->setPositionFixed(true);
	end1 = Avoid::ConnEnd(shape365249936, 3);
	end2 = Avoid::ConnEnd(junction328922310);
	/*Avoid::ConnRef * conn152169328 = */ new Avoid::ConnRef(router1, end1, end2);
	end1 = Avoid::ConnEnd(junction328922310);
	end2 = Avoid::ConnEnd(shape386142480, 2);
	/*Avoid::ConnRef * conn684888074 = */ new Avoid::ConnRef(router1, end1, end2);
	end1 = Avoid::ConnEnd(shape365249936, 5);
	end2 = Avoid::ConnEnd(junction328922310);
	Avoid::ConnRef * conn18168360 = new Avoid::ConnRef(router1, end1, end2);
	router1->processTransaction();

	router1->outputDiagram("output/2junctions-1");
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(50879, 50885));
	poly.setPoint(1, Avoid::Point(50879, 50960));
	poly.setPoint(2, Avoid::Point(50821, 50960));
	poly.setPoint(3, Avoid::Point(50821, 50885));
	Avoid::ShapeRef * shape150429385 = new Avoid::ShapeRef(router1, poly, 150429385);
	Avoid::ShapeConnectionPin * pin150429385_0 = new Avoid::ShapeConnectionPin(shape150429385, 1, 0.5, 0, true, 10, 1);
	pin150429385_0->setExclusive(true);
	Avoid::JunctionRef * junction550292508 = new Avoid::JunctionRef(router1, Avoid::Point(50850, 50875), 550292508);
	router1->deleteConnector(conn18168360);
	conn18168360 = nullptr;
	end1 = Avoid::ConnEnd(shape365249936, 5);
	end2 = Avoid::ConnEnd(junction550292508);
	Avoid::ConnRef * conn795337150 = new Avoid::ConnRef(router1, end1, end2);
	conn795337150->makePathInvalid();
	end1 = Avoid::ConnEnd(junction550292508);
	end2 = Avoid::ConnEnd(junction328922310);
	Avoid::ConnRef * conn151961380 = new Avoid::ConnRef(router1, end1, end2);
	conn151961380->makePathInvalid();
	end1 = Avoid::ConnEnd(shape150429385, 1);
	end2 = Avoid::ConnEnd(junction550292508);
	Avoid::ConnRef * conn149180423 = new Avoid::ConnRef(router1, end1, end2);
	conn149180423->makePathInvalid();
	router1->processTransaction();

	router1->outputDiagram("output/2junctions-2");
	/*
	junction550292508->setPositionFixed(true);
	router1->deleteShape(shape365249936);
	shape365249936 = nullptr;
	router1->deleteShape(shape386142480);
	shape386142480 = nullptr;
	router1->deleteJunction(junction328922310);
	junction328922310 = nullptr;
	router1->deleteConnector(conn152169328);
	conn152169328 = nullptr;
	router1->deleteConnector(conn684888074);
	conn684888074 = nullptr;
	router1->deleteShape(shape150429385);
	shape150429385 = nullptr;
	router1->deleteJunction(junction550292508);
	junction550292508 = nullptr;
	router1->deleteConnector(conn795337150);
	conn795337150 = nullptr;
	router1->deleteConnector(conn151961380);
	conn151961380 = nullptr;
	router1->deleteConnector(conn149180423);
	conn149180423 = nullptr;
	router1->processTransaction();
*/
	bool atEnds = true;
        bool overlap = router1->existsOrthogonalFixedSegmentOverlap(atEnds);
        delete router1;
        return (overlap) ? 1 : 0;
}

extern "C" int main(void)
{
	return test();
}
