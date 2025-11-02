#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

void test()
{
	ConnEnd end1;
	ConnEnd end2;
	Polygon poly;
	Router * router1 = new Router(OrthogonalRouting);
	router1->setTransactionUse(true);
	router1->setRoutingPenalty(segmentPenalty);
	router1->setRoutingPenalty(crossingPenalty);
	router1->setRoutingPenalty(fixedSharedPathPenalty, 9000);
	router1->setRoutingOption(penaliseOrthogonalSharedPathsAtConnEnds, true);
	router1->setRoutingParameter(idealNudgingDistance, 25);
	poly = Polygon(4);
	poly.setPoint(0, Point(50760, 50691));
	poly.setPoint(1, Point(50760, 50775));
	poly.setPoint(2, Point(50640, 50775));
	poly.setPoint(3, Point(50640, 50691));
	ShapeRef * shape386142480 = new ShapeRef(router1, poly, 386142480);
	ShapeConnectionPin * pin386142480_0 = new ShapeConnectionPin(shape386142480, 1,                  0.5,  0.40476190476190477,                    true, 0, 13);
	pin386142480_0->setExclusive(true);
	ShapeConnectionPin * pin386142480_1 = new ShapeConnectionPin(shape386142480, 2,                  0.5,                    1,                   true, 10, 2);
	pin386142480_1->setExclusive(true);
	poly = Polygon(4);
	poly.setPoint(0, Point(51185, 50765));
	poly.setPoint(1, Point(51185, 50935));
	poly.setPoint(2, Point(50975, 50935));
	poly.setPoint(3, Point(50975, 50765));
	ShapeRef * shape300136482 = new ShapeRef(router1, poly, 300136482);
	ShapeConnectionPin * pin300136482_0 = new ShapeConnectionPin(shape300136482, 1,                    0,                  0.5,                   true, 10, 4);
	pin300136482_0->setExclusive(true);
	ShapeConnectionPin * pin300136482_1 = new ShapeConnectionPin(shape300136482, 2,  0.95238095238095233,                  0.5,                   true, 10, 8);
	pin300136482_1->setExclusive(true);
	poly = Polygon(4);
	poly.setPoint(0, Point(50854, 50840));
	poly.setPoint(1, Point(50854, 50935));
	poly.setPoint(2, Point(50796, 50935));
	poly.setPoint(3, Point(50796, 50840));
	ShapeRef * shape51003942 = new ShapeRef(router1, poly, 51003942);
	ShapeConnectionPin * pin51003942_0 = new ShapeConnectionPin(shape51003942, 1,                  0.5,  0.10526315789473684,                   true, 10, 1);
	pin51003942_0->setExclusive(true);
	JunctionRef * junction502411221 = new JunctionRef(router1, Point(50825, 50850), 502411221);
	end1 = ConnEnd(shape386142480, 2);
	end2 = ConnEnd(junction502411221);
	new ConnRef(router1, end1, end2);
	end1 = ConnEnd(junction502411221);
	end2 = ConnEnd(shape300136482, 1);
	new ConnRef(router1, end1, end2);
	end1 = ConnEnd(shape51003942, 1);
	end2 = ConnEnd(junction502411221);
	new ConnRef(router1, end1, end2);
	router1->processTransaction();

	router1->outputDiagram("output/buildOrthogonalChannelInfo1");
    delete router1;
}

extern "C" int main(void)
{
	test();
	return 0;
}
