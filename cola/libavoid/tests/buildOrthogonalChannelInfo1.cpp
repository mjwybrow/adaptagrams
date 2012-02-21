#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

void outputInstanceToSVG(Avoid::Router * router)
{
	static int counter = 0;
	std::ostringstream os;
	os << (++counter);
	router->outputInstanceToSVG(os.str());
}

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
	router1->setRoutingOption(Avoid::penaliseOrthogonalSharedPathsAtConnEnds, true);
	router1->setOrthogonalNudgeDistance(25);
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(50760, 50691));
	poly.setPoint(1, Avoid::Point(50760, 50775));
	poly.setPoint(2, Avoid::Point(50640, 50775));
	poly.setPoint(3, Avoid::Point(50640, 50691));
	Avoid::ShapeRef * shape386142480 = new Avoid::ShapeRef(router1, poly, 386142480);
	Avoid::ShapeConnectionPin * pin386142480_0 = new Avoid::ShapeConnectionPin(shape386142480, 1,                  0.5,  0.40476190476190477,                    0, 13);
	pin386142480_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin386142480_1 = new Avoid::ShapeConnectionPin(shape386142480, 2,                  0.5,                    1,                   10, 2);
	pin386142480_1->setExclusive(true);
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(51185, 50765));
	poly.setPoint(1, Avoid::Point(51185, 50935));
	poly.setPoint(2, Avoid::Point(50975, 50935));
	poly.setPoint(3, Avoid::Point(50975, 50765));
	Avoid::ShapeRef * shape300136482 = new Avoid::ShapeRef(router1, poly, 300136482);
	Avoid::ShapeConnectionPin * pin300136482_0 = new Avoid::ShapeConnectionPin(shape300136482, 1,                    0,                  0.5,                   10, 4);
	pin300136482_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin300136482_1 = new Avoid::ShapeConnectionPin(shape300136482, 2,  0.95238095238095233,                  0.5,                   10, 8);
	pin300136482_1->setExclusive(true);
	poly = Avoid::Polygon(4);
	poly.setPoint(0, Avoid::Point(50854, 50840));
	poly.setPoint(1, Avoid::Point(50854, 50935));
	poly.setPoint(2, Avoid::Point(50796, 50935));
	poly.setPoint(3, Avoid::Point(50796, 50840));
	Avoid::ShapeRef * shape51003942 = new Avoid::ShapeRef(router1, poly, 51003942);
	Avoid::ShapeConnectionPin * pin51003942_0 = new Avoid::ShapeConnectionPin(shape51003942, 1,                  0.5,  0.10526315789473684,                   10, 1);
	pin51003942_0->setExclusive(true);
	Avoid::JunctionRef * junction502411221 = new Avoid::JunctionRef(router1, Avoid::Point(50825, 50850), 502411221);
	end1 = Avoid::ConnEnd(shape386142480, 2);
	end2 = Avoid::ConnEnd(junction502411221);
	Avoid::ConnRef * conn153915090 = new Avoid::ConnRef(router1, end1, end2);
	end1 = Avoid::ConnEnd(junction502411221);
	end2 = Avoid::ConnEnd(shape300136482, 1);
	Avoid::ConnRef * conn114162789 = new Avoid::ConnRef(router1, end1, end2);
	end1 = Avoid::ConnEnd(shape51003942, 1);
	end2 = Avoid::ConnEnd(junction502411221);
	Avoid::ConnRef * conn80701218 = new Avoid::ConnRef(router1, end1, end2);
	router1->processTransaction();

	outputInstanceToSVG(router1);
    delete router1;
}

extern "C" int main(int argc, char* argv[])
{
	test();
	return 0;
}
