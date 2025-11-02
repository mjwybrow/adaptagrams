#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

int test()
{
	ConnEnd end1;
	ConnEnd end2;
	Polygon poly;
	Router * router1 = new Router(OrthogonalRouting);
	router1->setTransactionUse(true);
	router1->setRoutingPenalty(segmentPenalty);
	router1->setRoutingPenalty(crossingPenalty);
	router1->setRoutingPenalty(fixedSharedPathPenalty);
	router1->setRoutingOption(penaliseOrthogonalSharedPathsAtConnEnds, true);
	router1->setRoutingParameter(idealNudgingDistance, 25);
	poly = Polygon(4);
	poly.setPoint(0, Point(51410, 50640));
	poly.setPoint(1, Point(51410, 50960));
	poly.setPoint(2, Point(51050, 50960));
	poly.setPoint(3, Point(51050, 50640));
	ShapeRef * shape365249936 = new ShapeRef(router1, poly, 365249936);
	ShapeConnectionPin * pin365249936_2 = new ShapeConnectionPin(shape365249936, 3, 0, 0.5, true, 10, 4);
	pin365249936_2->setExclusive(true);
	ShapeConnectionPin * pin365249936_4 = new ShapeConnectionPin(shape365249936, 5, 0, 0.734375, true, 10, 4);
	pin365249936_4->setExclusive(true);
	poly = Polygon(4);
	poly.setPoint(0, Point(50760, 50691));
	poly.setPoint(1, Point(50760, 50775));
	poly.setPoint(2, Point(50640, 50775));
	poly.setPoint(3, Point(50640, 50691));
	ShapeRef * shape386142480 = new ShapeRef(router1, poly, 386142480);
	ShapeConnectionPin * pin386142480_1 = new ShapeConnectionPin(shape386142480, 2, 0.5, 1, true, 10, 2);
	pin386142480_1->setExclusive(true);
	JunctionRef * junction328922310 = new JunctionRef(router1, Point(50700, 50800), 328922310);
	junction328922310->setPositionFixed(true);
	end1 = ConnEnd(shape365249936, 3);
	end2 = ConnEnd(junction328922310);
	/*ConnRef * conn152169328 = */ new ConnRef(router1, end1, end2);
	end1 = ConnEnd(junction328922310);
	end2 = ConnEnd(shape386142480, 2);
	/*ConnRef * conn684888074 = */ new ConnRef(router1, end1, end2);
	end1 = ConnEnd(shape365249936, 5);
	end2 = ConnEnd(junction328922310);
	ConnRef * conn18168360 = new ConnRef(router1, end1, end2);
	router1->processTransaction();

	router1->outputDiagram("output/2junctions-1");
	poly = Polygon(4);
	poly.setPoint(0, Point(50879, 50885));
	poly.setPoint(1, Point(50879, 50960));
	poly.setPoint(2, Point(50821, 50960));
	poly.setPoint(3, Point(50821, 50885));
	ShapeRef * shape150429385 = new ShapeRef(router1, poly, 150429385);
	ShapeConnectionPin * pin150429385_0 = new ShapeConnectionPin(shape150429385, 1, 0.5, 0, true, 10, 1);
	pin150429385_0->setExclusive(true);
	JunctionRef * junction550292508 = new JunctionRef(router1, Point(50850, 50875), 550292508);
	router1->deleteConnector(conn18168360);
	conn18168360 = nullptr;
	end1 = ConnEnd(shape365249936, 5);
	end2 = ConnEnd(junction550292508);
	ConnRef * conn795337150 = new ConnRef(router1, end1, end2);
	conn795337150->makePathInvalid();
	end1 = ConnEnd(junction550292508);
	end2 = ConnEnd(junction328922310);
	ConnRef * conn151961380 = new ConnRef(router1, end1, end2);
	conn151961380->makePathInvalid();
	end1 = ConnEnd(shape150429385, 1);
	end2 = ConnEnd(junction550292508);
	ConnRef * conn149180423 = new ConnRef(router1, end1, end2);
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
