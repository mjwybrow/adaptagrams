#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

void outputDiagram(Avoid::Router * router)
{
	static int counter = 0;
	std::ostringstream os;
	os << (++counter);
	router->outputDiagram(os.str());
}

void test()
{
	// The contents of this function can be replaced by the automatically generated test code from the debug svg file
	Avoid::ConnEnd end1_0;
	Avoid::ConnEnd end2_0;
	Avoid::Polygon poly0;
	std::vector<Avoid::Checkpoint> checkpoints_0;
	Avoid::Router * router0 = new Avoid::Router(Avoid::OrthogonalRouting);
	Avoid::HyperedgeNewAndDeletedObjectLists newanddeletedobjects_0;
	Avoid::ConnRefList::const_iterator connit_0;
	Avoid::JunctionRefList::const_iterator junctionit_0;
	router0->setTransactionUse(true);
	router0->setRoutingPenalty(Avoid::segmentPenalty);
	router0->setRoutingPenalty(Avoid::crossingPenalty);
	router0->setRoutingPenalty(Avoid::fixedSharedPathPenalty, 9000);
	router0->setRoutingOption(Avoid::penaliseOrthogonalSharedPathsAtConnEnds, true);
	router0->setRoutingParameter(Avoid::idealNudgingDistance, 25);
	router0->setRoutingOption(Avoid::improveHyperedgeRoutesMovingAddingAndDeletingJunctions, true);

	poly0 = Avoid::Polygon(4);
	poly0.setPoint(0, Avoid::Point(50700, 50615));
	poly0.setPoint(1, Avoid::Point(50700, 51185));
	poly0.setPoint(2, Avoid::Point(50365, 51185));
	poly0.setPoint(3, Avoid::Point(50365, 50615));
	Avoid::ShapeRef * shape0_355243155 = new Avoid::ShapeRef(router0, poly0, 355243155);
	Avoid::ShapeConnectionPin * pin0_355243155_0 = new Avoid::ShapeConnectionPin(shape0_355243155, 1, 1, 0.149122807018, true, 10, 8);
	pin0_355243155_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_355243155_1 = new Avoid::ShapeConnectionPin(shape0_355243155, 2, 1, 0.675438596491, true, 10, 8);
	pin0_355243155_1->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_355243155_2 = new Avoid::ShapeConnectionPin(shape0_355243155, 3, 1, 0.80701754386, true, 10, 8);
	pin0_355243155_2->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_355243155_3 = new Avoid::ShapeConnectionPin(shape0_355243155, 4, 1, 0.543859649123, true, 10, 8);
	pin0_355243155_3->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_355243155_4 = new Avoid::ShapeConnectionPin(shape0_355243155, 5, 1, 0.280701754386, true, 10, 8);
	pin0_355243155_4->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_355243155_5 = new Avoid::ShapeConnectionPin(shape0_355243155, 6, 1, 0.412280701754, true, 10, 8);
	pin0_355243155_5->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_355243155_6 = new Avoid::ShapeConnectionPin(shape0_355243155, 7, 0.477611940299, 0.982456140351, true, 10, 2);
	pin0_355243155_6->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_355243155_7 = new Avoid::ShapeConnectionPin(shape0_355243155, 8, 0.253731343284, 0.982456140351, true, 10, 2);
	pin0_355243155_7->setExclusive(true);
	poly0 = Avoid::Polygon(4);
	poly0.setPoint(0, Avoid::Point(51060, 50816));
	poly0.setPoint(1, Avoid::Point(51060, 50884));
	poly0.setPoint(2, Avoid::Point(50940, 50884));
	poly0.setPoint(3, Avoid::Point(50940, 50816));
	Avoid::ShapeRef * shape0_35177560 = new Avoid::ShapeRef(router0, poly0, 35177560);
	Avoid::ShapeConnectionPin * pin0_35177560_0 = new Avoid::ShapeConnectionPin(shape0_35177560, 1, 0.5, 0.5, true, 0, 15);
	pin0_35177560_0->setExclusive(true);
	poly0 = Avoid::Polygon(4);
	poly0.setPoint(0, Avoid::Point(51060, 50966));
	poly0.setPoint(1, Avoid::Point(51060, 51034));
	poly0.setPoint(2, Avoid::Point(50940, 51034));
	poly0.setPoint(3, Avoid::Point(50940, 50966));
	Avoid::ShapeRef * shape0_214253640 = new Avoid::ShapeRef(router0, poly0, 214253640);
	Avoid::ShapeConnectionPin * pin0_214253640_0 = new Avoid::ShapeConnectionPin(shape0_214253640, 1, 0.5, 0.5, true, 0, 15);
	pin0_214253640_0->setExclusive(true);
	Avoid::JunctionRef * junction0_636368600 = new Avoid::JunctionRef(router0, Avoid::Point(50875, 50850), 636368600);
	Avoid::JunctionRef * junction0_503331836 = new Avoid::JunctionRef(router0, Avoid::Point(50875, 51000), 503331836);
	Avoid::JunctionRef * junction0_46074711 = new Avoid::JunctionRef(router0, Avoid::Point(50775, 51000), 46074711);
	Avoid::JunctionRef * junction0_390996481 = new Avoid::JunctionRef(router0, Avoid::Point(50775, 50850), 390996481);
	end1_0 = Avoid::ConnEnd(junction0_636368600);
	end2_0 = Avoid::ConnEnd(shape0_35177560, 1);
	Avoid::ConnRef * conn0_929108568 = new Avoid::ConnRef(router0, end1_0, end2_0, 929108568);
	end1_0 = Avoid::ConnEnd(junction0_503331836);
	end2_0 = Avoid::ConnEnd(shape0_214253640, 1);
	Avoid::ConnRef * conn0_627671070 = new Avoid::ConnRef(router0, end1_0, end2_0, 627671070);
	end1_0 = Avoid::ConnEnd(junction0_636368600);
	end2_0 = Avoid::ConnEnd(junction0_503331836);
	Avoid::ConnRef * conn0_510680660 = new Avoid::ConnRef(router0, end1_0, end2_0, 510680660);
	end1_0 = Avoid::ConnEnd(shape0_355243155, 2);
	end2_0 = Avoid::ConnEnd(junction0_46074711);
	Avoid::ConnRef * conn0_175131537 = new Avoid::ConnRef(router0, end1_0, end2_0, 175131537);
	end1_0 = Avoid::ConnEnd(junction0_46074711);
	end2_0 = Avoid::ConnEnd(junction0_503331836);
	Avoid::ConnRef * conn0_545568780 = new Avoid::ConnRef(router0, end1_0, end2_0, 545568780);
	end1_0 = Avoid::ConnEnd(shape0_355243155, 6);
	end2_0 = Avoid::ConnEnd(junction0_390996481);
	Avoid::ConnRef * conn0_17788857 = new Avoid::ConnRef(router0, end1_0, end2_0, 17788857);
	end1_0 = Avoid::ConnEnd(junction0_390996481);
	end2_0 = Avoid::ConnEnd(junction0_636368600);
	Avoid::ConnRef * conn0_160581511 = new Avoid::ConnRef(router0, end1_0, end2_0, 160581511);
	end1_0 = Avoid::ConnEnd(junction0_46074711);
	end2_0 = Avoid::ConnEnd(junction0_390996481);
	Avoid::ConnRef * conn0_92116728 = new Avoid::ConnRef(router0, end1_0, end2_0, 92116728);
	router0->processTransaction();

	newanddeletedobjects_0 = router0->newAndDeletedObjectListsFromHyperedgeImprovement();
	outputDiagram(router0);
	router0->deleteConnector(conn0_92116728);
	conn0_92116728 = nullptr;
	router0->deleteConnector(conn0_160581511);
	conn0_160581511 = nullptr;
	router0->deleteConnector(conn0_17788857);
	conn0_17788857 = nullptr;
	router0->deleteJunction(junction0_390996481);
	junction0_390996481 = nullptr;
	router0->deleteConnector(conn0_545568780);
	conn0_545568780 = nullptr;
	router0->deleteConnector(conn0_175131537);
	conn0_175131537 = nullptr;
	router0->deleteJunction(junction0_46074711);
	junction0_46074711 = nullptr;
	router0->deleteConnector(conn0_510680660);
	conn0_510680660 = nullptr;
	router0->deleteConnector(conn0_627671070);
	conn0_627671070 = nullptr;
	router0->deleteJunction(junction0_503331836);
	junction0_503331836 = nullptr;
	router0->deleteConnector(conn0_929108568);
	conn0_929108568 = nullptr;
	router0->deleteJunction(junction0_636368600);
	junction0_636368600 = nullptr;
	router0->deleteShape(shape0_214253640);
	shape0_214253640 = nullptr;
	router0->deleteShape(shape0_35177560);
	shape0_35177560 = nullptr;
	router0->deleteShape(shape0_355243155);
	shape0_355243155 = nullptr;

	router0->processTransaction();

	delete router0;
}

extern "C" int main(void)
{
	test();
	return 0;
}
