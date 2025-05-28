#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

void outputDiagram(Router * router)
{
	static int counter = 0;
	std::ostringstream os;
	os << "output/improveHyperedge05-" << (++counter);
	router->outputDiagram(os.str());
}

void test()
{
	// The contents of this function can be replaced by the automatically generated test code from the debug svg file
	ConnEnd end1_0;
	ConnEnd end2_0;
	Polygon poly0;
	std::vector<Checkpoint> checkpoints_0;
	Router * router0 = new Router(OrthogonalRouting);
	HyperedgeNewAndDeletedObjectLists newanddeletedobjects_0;
	ConnRefList::const_iterator connit_0;
	JunctionRefList::const_iterator junctionit_0;
	router0->setTransactionUse(true);
	router0->setRoutingPenalty(segmentPenalty);
	router0->setRoutingPenalty(crossingPenalty);
	router0->setRoutingPenalty(fixedSharedPathPenalty, 9000);
	router0->setRoutingOption(penaliseOrthogonalSharedPathsAtConnEnds, true);
	router0->setRoutingParameter(idealNudgingDistance, 25);
	router0->setRoutingOption(improveHyperedgeRoutesMovingAddingAndDeletingJunctions, true);
	router0->processTransaction();

	newanddeletedobjects_0 = router0->newAndDeletedObjectListsFromHyperedgeImprovement();
	outputDiagram(router0);

	poly0 = Polygon(4);
	poly0.setPoint(0, Point(51885, 50715));
	poly0.setPoint(1, Point(51885, 50935));
	poly0.setPoint(2, Point(51700, 50935));
	poly0.setPoint(3, Point(51700, 50715));
	ShapeRef * shape0_68611488 = new ShapeRef(router0, poly0, 68611488);
	ShapeConnectionPin * pin0_68611488_0 = new ShapeConnectionPin(shape0_68611488, 1, 0.810810810811, 0.0454545454545, true, 10, 1);
	pin0_68611488_0->setExclusive(true);
	ShapeConnectionPin * pin0_68611488_1 = new ShapeConnectionPin(shape0_68611488, 2, 0, 0.5, true, 10, 4);
	pin0_68611488_1->setExclusive(true);
	ShapeConnectionPin * pin0_68611488_2 = new ShapeConnectionPin(shape0_68611488, 3, 0.810810810811, 0.954545454545, true, 10, 2);
	pin0_68611488_2->setExclusive(true);
	poly0 = Polygon(4);
	poly0.setPoint(0, Point(51600, 50795));
	poly0.setPoint(1, Point(51600, 50855));
	poly0.setPoint(2, Point(51440, 50855));
	poly0.setPoint(3, Point(51440, 50795));
	ShapeRef * shape0_84988680 = new ShapeRef(router0, poly0, 84988680);
	ShapeConnectionPin * pin0_84988680_0 = new ShapeConnectionPin(shape0_84988680, 1, 0.0625, 0.5, true, 10, 4);
	pin0_84988680_0->setExclusive(true);
	ShapeConnectionPin * pin0_84988680_1 = new ShapeConnectionPin(shape0_84988680, 2, 1, 0.5, true, 10, 8);
	pin0_84988680_1->setExclusive(true);
	poly0 = Polygon(4);
	poly0.setPoint(0, Point(51705, 50640));
	poly0.setPoint(1, Point(51705, 50800));
	poly0.setPoint(2, Point(51645, 50800));
	poly0.setPoint(3, Point(51645, 50640));
	ShapeRef * shape0_170757006 = new ShapeRef(router0, poly0, 170757006);
	ShapeConnectionPin * pin0_170757006_0 = new ShapeConnectionPin(shape0_170757006, 1, 0.5, 1, true, 10, 2);
	pin0_170757006_0->setExclusive(true);
	ShapeConnectionPin * pin0_170757006_1 = new ShapeConnectionPin(shape0_170757006, 2, 0.5, 0.0625, true, 10, 1);
	pin0_170757006_1->setExclusive(true);
	JunctionRef * junction0_185122667 = new JunctionRef(router0, Point(51675, 50825), 185122667);
	poly0 = Polygon(4);
	poly0.setPoint(0, Point(51705, 50925));
	poly0.setPoint(1, Point(51705, 51085));
	poly0.setPoint(2, Point(51645, 51085));
	poly0.setPoint(3, Point(51645, 50925));
	ShapeRef * shape0_300278382 = new ShapeRef(router0, poly0, 300278382);
	ShapeConnectionPin * pin0_300278382_0 = new ShapeConnectionPin(shape0_300278382, 1, 0.5, 0.9375, true, 10, 2);
	pin0_300278382_0->setExclusive(true);
	ShapeConnectionPin * pin0_300278382_1 = new ShapeConnectionPin(shape0_300278382, 2, 0.5, 0, true, 10, 1);
	pin0_300278382_1->setExclusive(true);
	end1_0 = ConnEnd(shape0_84988680, 2);
	end2_0 = ConnEnd(junction0_185122667);
	new ConnRef(router0, end1_0, end2_0, 70922320);
	end1_0 = ConnEnd(junction0_185122667);
	end2_0 = ConnEnd(shape0_68611488, 2);
	new ConnRef(router0, end1_0, end2_0, 141698060);
	end1_0 = ConnEnd(shape0_170757006, 1);
	end2_0 = ConnEnd(junction0_185122667);
	new ConnRef(router0, end1_0, end2_0, 27983869);
	end1_0 = ConnEnd(shape0_300278382, 2);
	end2_0 = ConnEnd(junction0_185122667);
	new ConnRef(router0, end1_0, end2_0, 197910572);
	router0->processTransaction();
	//printf("%d\n", junction0_185122667->attachedConnectors().size());
	COLA_ASSERT(junction0_185122667->attachedConnectors().size() == 4);

	newanddeletedobjects_0 = router0->newAndDeletedObjectListsFromHyperedgeImprovement();
	outputDiagram(router0);
	printf("\n\n\n");
	poly0 = Polygon(4);
	poly0.setPoint(0, Point(51305, 50690));
	poly0.setPoint(1, Point(51305, 50850));
	poly0.setPoint(2, Point(51245, 50850));
	poly0.setPoint(3, Point(51245, 50690));
	router0->moveShape(shape0_170757006, poly0);
	router0->processTransaction();

	newanddeletedobjects_0 = router0->newAndDeletedObjectListsFromHyperedgeImprovement();
	junctionit_0 = newanddeletedobjects_0.newJunctionList.begin();
	JunctionRef * junction0_436653030 = *junctionit_0;
	junction0_436653030->position();
	//printf("%d %d\n", junction0_185122667->attachedConnectors().size(),
	//		junction0_436653030->attachedConnectors().size());
	COLA_ASSERT(junction0_185122667->attachedConnectors().size() == 3);
	COLA_ASSERT(junction0_436653030->attachedConnectors().size() == 3);

	junction0_436653030->attachedConnectors();
	junction0_436653030->setPositionFixed(true);
	junctionit_0++;
	COLA_ASSERT(junctionit_0 == newanddeletedobjects_0.newJunctionList.end());
	connit_0 = newanddeletedobjects_0.newConnectorList.begin();
	ConnRef * conn0_400977693 = *connit_0;
	conn0_400977693->endpointConnEnds();
	connit_0++;
	COLA_ASSERT(connit_0 == newanddeletedobjects_0.newConnectorList.end());
	outputDiagram(router0);
	
	conn0_400977693->makePathInvalid();
	router0->processTransaction();
	
	outputDiagram(router0);
	return;	
/*
        // INCORRECT
	newanddeletedobjects_0 = router0->newAndDeletedObjectListsFromHyperedgeImprovement();
	junctionit_0 = newanddeletedobjects_0.newJunctionList.begin();
	JunctionRef * junction0_112970070 = *junctionit_0;
	junction0_112970070->position();
	junction0_112970070->attachedConnectors();
	junction0_112970070->setPositionFixed(true);
	junctionit_0++;
	COLA_ASSERT(junctionit_0 == newanddeletedobjects_0.newJunctionList.end());
	connit_0 = newanddeletedobjects_0.newConnectorList.begin();
	ConnRef * conn0_152820633 = *connit_0;
	conn0_152820633->endpointConnEnds();
	connit_0++;
	COLA_ASSERT(connit_0 == newanddeletedobjects_0.newConnectorList.end());
	outputDiagram(router0);
	*/
}

extern "C" int main(void)
{
	test();
	return 0;
}
