#include "libavoid/libavoid.h"
#include <sstream>

void test()
{
	Avoid::ConnEnd end1_0;
	Avoid::ConnEnd end2_0;
	Avoid::Polygon poly0;
	Avoid::Router * router0 = new Avoid::Router(Avoid::OrthogonalRouting);
	Avoid::HyperedgeNewAndDeletedObjectLists newanddeletedobjects_0;
	Avoid::ConnRefList::const_iterator connit_0;
	Avoid::JunctionRefList::const_iterator junctionit_0;
	router0->setTransactionUse(true);
	router0->setRoutingPenalty(Avoid::segmentPenalty);
	router0->setRoutingParameter(Avoid::portDirectionPenalty, 100);
	router0->setRoutingParameter(Avoid::idealNudgingDistance, 25);
	router0->setRoutingOption(Avoid::improveHyperedgeRoutesMovingAddingAndDeletingJunctions, true);
	router0->setRoutingPenalty(Avoid::fixedSharedPathPenalty, 9000);
	router0->setRoutingOption(Avoid::penaliseOrthogonalSharedPathsAtConnEnds, true);
	router0->setRoutingParameter(Avoid::crossingPenalty);
	router0->setRoutingOption(Avoid::performUnifyingNudgingPreprocessingStep, true);
	router0->setRoutingPenalty(Avoid::fixedSharedPathPenalty, 9000);
	router0->setRoutingOption(Avoid::penaliseOrthogonalSharedPathsAtConnEnds, true);
	router0->setRoutingParameter(Avoid::crossingPenalty);
	router0->setRoutingOption(Avoid::performUnifyingNudgingPreprocessingStep, true);
	poly0 = Avoid::Polygon(4);
	poly0.setPoint(0, Avoid::Point(50410, 49965));
	poly0.setPoint(1, Avoid::Point(50410, 50710));
	poly0.setPoint(2, Avoid::Point(49975, 50710));
	poly0.setPoint(3, Avoid::Point(49975, 49965));
	Avoid::ShapeRef * shape0_1023802233 = new Avoid::ShapeRef(router0, poly0, 1023802233);
	Avoid::ShapeConnectionPin * pin0_1023802233_2 = new Avoid::ShapeConnectionPin(shape0_1023802233, 3, 0, 560, false, 10, 4);
	pin0_1023802233_2->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_1023802233_12 = new Avoid::ShapeConnectionPin(shape0_1023802233, 13, 0, 635, false, 10, 4);
	pin0_1023802233_12->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_1023802233_19 = new Avoid::ShapeConnectionPin(shape0_1023802233, 20, 0, 485, false, 10, 4);
	pin0_1023802233_19->setExclusive(true);
	poly0 = Avoid::Polygon(4);
	poly0.setPoint(0, Avoid::Point(49504, 50775));
	poly0.setPoint(1, Avoid::Point(49504, 50860));
	poly0.setPoint(2, Avoid::Point(49446, 50860));
	poly0.setPoint(3, Avoid::Point(49446, 50775));
	Avoid::ShapeRef * shape0_368159034 = new Avoid::ShapeRef(router0, poly0, 368159034);
	Avoid::ShapeConnectionPin * pin0_368159034_0 = new Avoid::ShapeConnectionPin(shape0_368159034, 1, 29, 0, false, 10, 1);
	pin0_368159034_0->setExclusive(true);
	
	Avoid::JunctionRef * junction0_526491597 = new Avoid::JunctionRef(router0, Avoid::Point(49600, 50525), 526491597);
	junction0_526491597->setPositionFixed(false);
	
	Avoid::JunctionRef * junction0_30828132 = new Avoid::JunctionRef(router0, Avoid::Point(49600, 50650), 30828132);
	junction0_30828132->setPositionFixed(false);
	
	// Right, top - to top junction
	end1_0 = Avoid::ConnEnd(shape0_1023802233, 20);
	end2_0 = Avoid::ConnEnd(junction0_526491597);
	Avoid::ConnRef * conn0_117136925 = new Avoid::ConnRef(router0, end1_0, end2_0, 117136925);
	poly0 = Avoid::PolyLine(3);
	poly0.setPoint(0, Avoid::Point(49975, 50450));
	poly0.setPoint(1, Avoid::Point(49600, 50450));
	poly0.setPoint(2, Avoid::Point(49600, 50525));
	conn0_117136925->setFixedRoute(poly0);
	conn0_117136925->setSourceEndpoint(end1_0);
	conn0_117136925->setDestEndpoint(end2_0);
	
	// Right, middle - to top junction.
	end1_0 = Avoid::ConnEnd(shape0_1023802233, 3);
	end2_0 = Avoid::ConnEnd(junction0_526491597);
	Avoid::ConnRef * conn0_224626788 = new Avoid::ConnRef(router0, end1_0, end2_0, 224626788);
	
	// Middle vertical from top junction point to lower junction.
#ifdef CORRECTED
	end1_0 = Avoid::ConnEnd(junction0_526491597);
#else
	end1_0 = Avoid::ConnEnd(Avoid::Point(49600, 50525), 15);
#endif
	end2_0 = Avoid::ConnEnd(junction0_30828132);
	Avoid::ConnRef * conn0_446423927 = new Avoid::ConnRef(router0, end1_0, end2_0, 446423927);
	poly0 = Avoid::PolyLine(3);
	poly0.setPoint(0, Avoid::Point(49600, 50525));
	poly0.setPoint(1, Avoid::Point(49600, 50587));
	poly0.setPoint(2, Avoid::Point(49600, 50650));
	conn0_446423927->setFixedRoute(poly0);
	conn0_446423927->setSourceEndpoint(end1_0);
	conn0_446423927->setDestEndpoint(end2_0);
	
	// Bottom, right - to lower junction point.
	end1_0 = Avoid::ConnEnd(shape0_1023802233, 13);
#ifdef CORRECTED
	end2_0 = Avoid::ConnEnd(junction0_30828132);
#else
	end2_0 = Avoid::ConnEnd(Avoid::Point(49600, 50650), 15);
#endif
	Avoid::ConnRef * conn0_11189244 = new Avoid::ConnRef(router0, end1_0, end2_0, 11189244);
	poly0 = Avoid::PolyLine(3);
	poly0.setPoint(0, Avoid::Point(49975, 50600));
	poly0.setPoint(1, Avoid::Point(49600, 50600));
	poly0.setPoint(2, Avoid::Point(49600, 50650));
	conn0_11189244->setFixedRoute(poly0);
	conn0_11189244->setSourceEndpoint(end1_0);
	conn0_11189244->setDestEndpoint(end2_0);

	// Left bottom - to lower junction.
	end1_0 = Avoid::ConnEnd(shape0_368159034, 1);
	end2_0 = Avoid::ConnEnd(junction0_30828132);
	Avoid::ConnRef * conn0_280909026 = new Avoid::ConnRef(router0, end1_0, end2_0, 280909026);
	
	router0->processTransaction();

	newanddeletedobjects_0 = router0->newAndDeletedObjectListsFromHyperedgeImprovement();
	router0->outputDiagram("output/treeRootCrash01-1");
	
	conn0_117136925->clearFixedRoute();
	conn0_224626788->makePathInvalid();
	conn0_446423927->clearFixedRoute();
	conn0_11189244->clearFixedRoute();
	conn0_280909026->makePathInvalid();
	junction0_526491597->setPositionFixed(false);
	junction0_30828132->setPositionFixed(false);
	router0->processTransaction();

	newanddeletedobjects_0 = router0->newAndDeletedObjectListsFromHyperedgeImprovement();
	router0->outputDiagram("output/treeRootCrash01-2");
	router0->processTransaction();

	newanddeletedobjects_0 = router0->newAndDeletedObjectListsFromHyperedgeImprovement();
	router0->outputDiagram("output/treeRootCrash01-3");
	
	router0->hyperedgeRerouter()->registerHyperedgeForRerouting(junction0_526491597);
	router0->processTransaction();
	router0->outputDiagram("output/treeRootCrash01-4");

	delete router0;
}


extern "C" int main(int argc, char* argv[])
{
	test();
	return 0;
}
