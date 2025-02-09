// Based on 33004heap_corruption.cpp
//
#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

void outputDiagram(Router * router)
{
	static int counter = 0;
	std::ostringstream os;
	os << (++counter);
	//router->outputDiagram(os.str());
}

int test()
{
	// The contents of this function can be replaced by the automatically generated test code from the debug svg file
	ConnEnd end1_0;
	ConnEnd end2_0;
	Polygon poly0;
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

	poly0 = Polygon(4);
	poly0.setPoint(0, Point(27350, 26040));
	poly0.setPoint(1, Point(27350, 26450));
	poly0.setPoint(2, Point(26925, 26450));
	poly0.setPoint(3, Point(26925, 26040));
	ShapeRef * shape0_478041183 = new ShapeRef(router0, poly0, 478041183);
	ShapeConnectionPin * pin0_478041183_0 = new ShapeConnectionPin(shape0_478041183, 1, 0, 85, false, 10, 4);
	pin0_478041183_0->setExclusive(true);
	ShapeConnectionPin * pin0_478041183_1 = new ShapeConnectionPin(shape0_478041183, 2, 100, -1, false, 10, 2);
	pin0_478041183_1->setExclusive(true);
	ShapeConnectionPin * pin0_478041183_2 = new ShapeConnectionPin(shape0_478041183, 3, -1, 85, false, 10, 8);
	pin0_478041183_2->setExclusive(true);
	ShapeConnectionPin * pin0_478041183_3 = new ShapeConnectionPin(shape0_478041183, 4, 250, -1, false, 10, 2);
	pin0_478041183_3->setExclusive(true);
	ShapeConnectionPin * pin0_478041183_4 = new ShapeConnectionPin(shape0_478041183, 5, 0, 235, false, 10, 4);
	pin0_478041183_4->setExclusive(true);
	ShapeConnectionPin * pin0_478041183_5 = new ShapeConnectionPin(shape0_478041183, 6, 0, 160, false, 10, 4);
	pin0_478041183_5->setExclusive(true);
	ShapeConnectionPin * pin0_478041183_6 = new ShapeConnectionPin(shape0_478041183, 7, 175, -1, false, 10, 2);
	pin0_478041183_6->setExclusive(true);
	ShapeConnectionPin * pin0_478041183_7 = new ShapeConnectionPin(shape0_478041183, 8, -1, 160, false, 10, 8);
	pin0_478041183_7->setExclusive(true);
	ShapeConnectionPin * pin0_478041183_8 = new ShapeConnectionPin(shape0_478041183, 9, 325, -1, false, 10, 2);
	pin0_478041183_8->setExclusive(true);
	ShapeConnectionPin * pin0_478041183_9 = new ShapeConnectionPin(shape0_478041183, 10, 0, 310, false, 10, 4);
	pin0_478041183_9->setExclusive(true);
	poly0 = Polygon(4);
	poly0.setPoint(0, Point(27030, 26900));
	poly0.setPoint(1, Point(27030, 27050));
	poly0.setPoint(2, Point(26970, 27050));
	poly0.setPoint(3, Point(26970, 26900));
	ShapeRef * shape0_646972128 = new ShapeRef(router0, poly0, 646972128);
	ShapeConnectionPin * pin0_646972128_0 = new ShapeConnectionPin(shape0_646972128, 1, 25, -1, false, 10, 2);
	pin0_646972128_0->setExclusive(true);
	ShapeConnectionPin * pin0_646972128_1 = new ShapeConnectionPin(shape0_646972128, 2, 25, 0, false, 10, 1);
	pin0_646972128_1->setExclusive(true);
	poly0 = Polygon(4);
	poly0.setPoint(0, Point(26480, 26350));
	poly0.setPoint(1, Point(26480, 26500));
	poly0.setPoint(2, Point(26420, 26500));
	poly0.setPoint(3, Point(26420, 26350));
	ShapeRef * shape0_53041512 = new ShapeRef(router0, poly0, 53041512);
	ShapeConnectionPin * pin0_53041512_0 = new ShapeConnectionPin(shape0_53041512, 1, 30, 0, false, 10, 1);
	pin0_53041512_0->setExclusive(true);
	ShapeConnectionPin * pin0_53041512_1 = new ShapeConnectionPin(shape0_53041512, 2, 30, -1, false, 10, 2);
	pin0_53041512_1->setExclusive(true);
	poly0 = Polygon(4);
	poly0.setPoint(0, Point(26860, 26900));
	poly0.setPoint(1, Point(26860, 27050));
	poly0.setPoint(2, Point(26790, 27050));
	poly0.setPoint(3, Point(26790, 26900));
	ShapeRef * shape0_241539284 = new ShapeRef(router0, poly0, 241539284);
	ShapeConnectionPin * pin0_241539284_0 = new ShapeConnectionPin(shape0_241539284, 1, 35, -1, false, 10, 2);
	pin0_241539284_0->setExclusive(true);
	ShapeConnectionPin * pin0_241539284_1 = new ShapeConnectionPin(shape0_241539284, 2, 35, 0, false, 10, 1);
	pin0_241539284_1->setExclusive(true);
	JunctionRef * junction0_16268769 = new JunctionRef(router0, Point(27000, 27075), 16268769);
	junction0_16268769->setPositionFixed(true);
	JunctionRef * junction0_142222080 = new JunctionRef(router0, Point(27200, 26450), 142222080);
	junction0_142222080->setPositionFixed(true);
	JunctionRef * junction0_471289251 = new JunctionRef(router0, Point(26350, 26325), 471289251);
	junction0_471289251->setPositionFixed(true);
	
	
	end1_0 = ConnEnd(shape0_646972128, 1);
	end2_0 = ConnEnd(junction0_16268769);
	ConnRef * conn0_64950216 = new ConnRef(router0, end1_0, end2_0, 64950216);
	end1_0 = ConnEnd(junction0_16268769);
	end2_0 = ConnEnd(shape0_478041183, 7);
	ConnRef * conn0_278089510 = new ConnRef(router0, end1_0, end2_0, 278089510);
	end1_0 = ConnEnd(shape0_241539284, 1);
	end2_0 = ConnEnd(junction0_16268769);
	ConnRef * conn0_41268690 = new ConnRef(router0, end1_0, end2_0, 41268690);

	end1_0 = ConnEnd(shape0_478041183, 9);
	end2_0 = ConnEnd(junction0_142222080);
	ConnRef * conn0_60256130 = new ConnRef(router0, end1_0, end2_0, 60256130);
	end1_0 = ConnEnd(shape0_478041183, 3);
	end2_0 = ConnEnd(junction0_142222080);
	ConnRef * conn0_73835204 = new ConnRef(router0, end1_0, end2_0, 73835204);
	end1_0 = ConnEnd(shape0_478041183, 4);
	end2_0 = ConnEnd(junction0_142222080);
	ConnRef * conn0_93872527 = new ConnRef(router0, end1_0, end2_0, 93872527);

	end1_0 = ConnEnd(shape0_53041512, 1);
	end2_0 = ConnEnd(junction0_471289251);
	ConnRef * conn0_448284384 = new ConnRef(router0, end1_0, end2_0, 448284384);
	end1_0 = ConnEnd(shape0_478041183, 1);
	end2_0 = ConnEnd(junction0_471289251);
	ConnRef * conn0_392836745 = new ConnRef(router0, end1_0, end2_0, 392836745);
	end1_0 = ConnEnd(shape0_241539284, 2);
	end2_0 = ConnEnd(junction0_471289251);
	ConnRef * conn0_836242673 = new ConnRef(router0, end1_0, end2_0, 836242673);

	router0->processTransaction();

	newanddeletedobjects_0 = router0->newAndDeletedObjectListsFromHyperedgeImprovement();
	outputDiagram(router0);
	
	junction0_16268769->setPositionFixed(false);
	junction0_142222080->setPositionFixed(false);
	junction0_471289251->setPositionFixed(false);
	
	router0->hyperedgeRerouter()->registerHyperedgeForRerouting(junction0_16268769);
	router0->hyperedgeRerouter()->registerHyperedgeForRerouting(junction0_142222080);
	router0->hyperedgeRerouter()->registerHyperedgeForRerouting(junction0_471289251);
	
	router0->processTransaction();
	outputDiagram(router0);

    int result = router0->existsInvalidOrthogonalPaths();
    delete router0;
    return result;
}

extern "C" int main(int argc, char* argv[])
{
	return test();
}
