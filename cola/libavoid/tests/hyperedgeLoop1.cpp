// THis is a regression test for an infinite loop in the hyperedge improvement.

#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

void test()
{
	Avoid::ConnEnd end1;
	Avoid::ConnEnd end2;
	Avoid::Polygon poly;
	Avoid::Router * router1 = new Avoid::Router(Avoid::OrthogonalRouting);
	router1->setTransactionUse(true);
	router1->setRoutingPenalty(Avoid::segmentPenalty);
	router1->setRoutingPenalty(Avoid::crossingPenalty);
	router1->setRoutingPenalty(Avoid::fixedSharedPathPenalty, 9000);
	router1->setRoutingParameter(idealNudgingDistance, 25);

	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(50760, 50620));
	poly.setPoint(1, Avoid::Point(50760, 50680));
	poly.setPoint(2, Avoid::Point(50590, 50680));
	poly.setPoint(3, Avoid::Point(50590, 50620));
	Avoid::ShapeRef * shape147006780 = new Avoid::ShapeRef(router1, poly, 147006780);
	Avoid::ShapeConnectionPin * pin147006780_1 = new Avoid::ShapeConnectionPin(shape147006780, 2, 0.941176, 0.5, true, 10, 8);
	pin147006780_1->setExclusive(true);

	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(50585, 50765));
	poly.setPoint(1, Avoid::Point(50585, 50935));
	poly.setPoint(2, Avoid::Point(50365, 50935));
	poly.setPoint(3, Avoid::Point(50365, 50765));
	Avoid::ShapeRef * shape69758810 = new Avoid::ShapeRef(router1, poly, 69758810);
	Avoid::ShapeConnectionPin * pin69758810_1 = new Avoid::ShapeConnectionPin(shape69758810, 2, 0.954545, 0.5, true, 10, 8);
	pin69758810_1->setExclusive(true);

	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(51060, 51020));
	poly.setPoint(1, Avoid::Point(51060, 51080));
	poly.setPoint(2, Avoid::Point(50890, 51080));
	poly.setPoint(3, Avoid::Point(50890, 51020));
	Avoid::ShapeRef * shape149922619 = new Avoid::ShapeRef(router1, poly, 149922619);
	Avoid::ShapeConnectionPin * pin149922619_1 = new Avoid::ShapeConnectionPin(shape149922619, 2, 0.941176, 0.5, true, 10, 8);
	pin149922619_1->setExclusive(true);

	Avoid::JunctionRef * junction228834480 = new Avoid::JunctionRef(router1, Avoid::Point(51050, 51050), 228834480);
	end1 = Avoid::ConnEnd(shape69758810, 2);
	end2 = Avoid::ConnEnd(junction228834480);
	Avoid::ConnRef * conn159270000 = new Avoid::ConnRef(router1, end1, end2);
	conn159270000->makePathInvalid();
	end1 = Avoid::ConnEnd(junction228834480);
	end2 = Avoid::ConnEnd(shape149922619, 2);
	Avoid::ConnRef * conn199495942 = new Avoid::ConnRef(router1, end1, end2);
	router1->processTransaction();

	router1->outputDiagram("output/hyperedgeLoop1-1");
	end1 = Avoid::ConnEnd(shape147006780, 2);
	end2 = Avoid::ConnEnd(junction228834480);
	Avoid::ConnRef * conn8326760 = new Avoid::ConnRef(router1, end1, end2);
	conn8326760->makePathInvalid();
	router1->processTransaction(); // infinite

	router1->outputDiagram("output/hyperedgeLoop1-2");
	router1->deleteShape(shape147006780);
	shape147006780 = nullptr;
	router1->deleteShape(shape69758810);
	shape69758810 = nullptr;
	router1->deleteShape(shape149922619);
	shape149922619 = nullptr;
	router1->deleteJunction(junction228834480);
	junction228834480 = nullptr;
	router1->deleteConnector(conn159270000);
	conn159270000 = nullptr;
	router1->deleteConnector(conn199495942);
	conn199495942 = nullptr;
	router1->deleteConnector(conn8326760);
	conn8326760 = nullptr;
	router1->processTransaction();

	delete router1;
}

extern "C" int main(void)
{
	test();
	return 0;
}
