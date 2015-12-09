// Based on 33004heap_corruption.cpp
//
#include "libavoid/libavoid.h"
#include <sstream>

void outputDiagram(Avoid::Router * router)
{
	static int counter = 0;
	std::ostringstream os;
	os << (++counter);
	//router->outputDiagram(os.str());
}

int test()
{
	// The contents of this function can be replaced by the automatically generated test code from the debug svg file
	Avoid::ConnEnd end1_0;
	Avoid::ConnEnd end2_0;
	Avoid::Polygon poly0;
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
	poly0.setPoint(0, Avoid::Point(27350, 26040));
	poly0.setPoint(1, Avoid::Point(27350, 26450));
	poly0.setPoint(2, Avoid::Point(26925, 26450));
	poly0.setPoint(3, Avoid::Point(26925, 26040));
	Avoid::ShapeRef * shape0_478041183 = new Avoid::ShapeRef(router0, poly0, 478041183);
	Avoid::ShapeConnectionPin * pin0_478041183_0 = new Avoid::ShapeConnectionPin(shape0_478041183, 1, 0, 85, false, 10, 4);
	pin0_478041183_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_478041183_1 = new Avoid::ShapeConnectionPin(shape0_478041183, 2, 100, -1, false, 10, 2);
	pin0_478041183_1->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_478041183_2 = new Avoid::ShapeConnectionPin(shape0_478041183, 3, -1, 85, false, 10, 8);
	pin0_478041183_2->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_478041183_3 = new Avoid::ShapeConnectionPin(shape0_478041183, 4, 250, -1, false, 10, 2);
	pin0_478041183_3->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_478041183_4 = new Avoid::ShapeConnectionPin(shape0_478041183, 5, 0, 235, false, 10, 4);
	pin0_478041183_4->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_478041183_5 = new Avoid::ShapeConnectionPin(shape0_478041183, 6, 0, 160, false, 10, 4);
	pin0_478041183_5->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_478041183_6 = new Avoid::ShapeConnectionPin(shape0_478041183, 7, 175, -1, false, 10, 2);
	pin0_478041183_6->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_478041183_7 = new Avoid::ShapeConnectionPin(shape0_478041183, 8, -1, 160, false, 10, 8);
	pin0_478041183_7->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_478041183_8 = new Avoid::ShapeConnectionPin(shape0_478041183, 9, 325, -1, false, 10, 2);
	pin0_478041183_8->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_478041183_9 = new Avoid::ShapeConnectionPin(shape0_478041183, 10, 0, 310, false, 10, 4);
	pin0_478041183_9->setExclusive(true);
	poly0 = Avoid::Polygon(4);
	poly0.setPoint(0, Avoid::Point(27030, 26900));
	poly0.setPoint(1, Avoid::Point(27030, 27050));
	poly0.setPoint(2, Avoid::Point(26970, 27050));
	poly0.setPoint(3, Avoid::Point(26970, 26900));
	Avoid::ShapeRef * shape0_646972128 = new Avoid::ShapeRef(router0, poly0, 646972128);
	Avoid::ShapeConnectionPin * pin0_646972128_0 = new Avoid::ShapeConnectionPin(shape0_646972128, 1, 25, -1, false, 10, 2);
	pin0_646972128_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_646972128_1 = new Avoid::ShapeConnectionPin(shape0_646972128, 2, 25, 0, false, 10, 1);
	pin0_646972128_1->setExclusive(true);
	poly0 = Avoid::Polygon(4);
	poly0.setPoint(0, Avoid::Point(26480, 26350));
	poly0.setPoint(1, Avoid::Point(26480, 26500));
	poly0.setPoint(2, Avoid::Point(26420, 26500));
	poly0.setPoint(3, Avoid::Point(26420, 26350));
	Avoid::ShapeRef * shape0_53041512 = new Avoid::ShapeRef(router0, poly0, 53041512);
	Avoid::ShapeConnectionPin * pin0_53041512_0 = new Avoid::ShapeConnectionPin(shape0_53041512, 1, 30, 0, false, 10, 1);
	pin0_53041512_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_53041512_1 = new Avoid::ShapeConnectionPin(shape0_53041512, 2, 30, -1, false, 10, 2);
	pin0_53041512_1->setExclusive(true);
	poly0 = Avoid::Polygon(4);
	poly0.setPoint(0, Avoid::Point(26860, 26900));
	poly0.setPoint(1, Avoid::Point(26860, 27050));
	poly0.setPoint(2, Avoid::Point(26790, 27050));
	poly0.setPoint(3, Avoid::Point(26790, 26900));
	Avoid::ShapeRef * shape0_241539284 = new Avoid::ShapeRef(router0, poly0, 241539284);
	Avoid::ShapeConnectionPin * pin0_241539284_0 = new Avoid::ShapeConnectionPin(shape0_241539284, 1, 35, -1, false, 10, 2);
	pin0_241539284_0->setExclusive(true);
	Avoid::ShapeConnectionPin * pin0_241539284_1 = new Avoid::ShapeConnectionPin(shape0_241539284, 2, 35, 0, false, 10, 1);
	pin0_241539284_1->setExclusive(true);
	Avoid::JunctionRef * junction0_16268769 = new Avoid::JunctionRef(router0, Avoid::Point(27000, 27075), 16268769);
	junction0_16268769->setPositionFixed(true);
	Avoid::JunctionRef * junction0_142222080 = new Avoid::JunctionRef(router0, Avoid::Point(27200, 26450), 142222080);
	junction0_142222080->setPositionFixed(true);
	Avoid::JunctionRef * junction0_471289251 = new Avoid::JunctionRef(router0, Avoid::Point(26350, 26325), 471289251);
	junction0_471289251->setPositionFixed(true);
	
	
	end1_0 = Avoid::ConnEnd(shape0_646972128, 1);
	end2_0 = Avoid::ConnEnd(junction0_16268769);
	Avoid::ConnRef * conn0_64950216 = new Avoid::ConnRef(router0, end1_0, end2_0, 64950216);
	end1_0 = Avoid::ConnEnd(junction0_16268769);
	end2_0 = Avoid::ConnEnd(shape0_478041183, 7);
	Avoid::ConnRef * conn0_278089510 = new Avoid::ConnRef(router0, end1_0, end2_0, 278089510);
	end1_0 = Avoid::ConnEnd(shape0_241539284, 1);
	end2_0 = Avoid::ConnEnd(junction0_16268769);
	Avoid::ConnRef * conn0_41268690 = new Avoid::ConnRef(router0, end1_0, end2_0, 41268690);

	end1_0 = Avoid::ConnEnd(shape0_478041183, 9);
	end2_0 = Avoid::ConnEnd(junction0_142222080);
	Avoid::ConnRef * conn0_60256130 = new Avoid::ConnRef(router0, end1_0, end2_0, 60256130);
	end1_0 = Avoid::ConnEnd(shape0_478041183, 3);
	end2_0 = Avoid::ConnEnd(junction0_142222080);
	Avoid::ConnRef * conn0_73835204 = new Avoid::ConnRef(router0, end1_0, end2_0, 73835204);
	end1_0 = Avoid::ConnEnd(shape0_478041183, 4);
	end2_0 = Avoid::ConnEnd(junction0_142222080);
	Avoid::ConnRef * conn0_93872527 = new Avoid::ConnRef(router0, end1_0, end2_0, 93872527);

	end1_0 = Avoid::ConnEnd(shape0_53041512, 1);
	end2_0 = Avoid::ConnEnd(junction0_471289251);
	Avoid::ConnRef * conn0_448284384 = new Avoid::ConnRef(router0, end1_0, end2_0, 448284384);
	end1_0 = Avoid::ConnEnd(shape0_478041183, 1);
	end2_0 = Avoid::ConnEnd(junction0_471289251);
	Avoid::ConnRef * conn0_392836745 = new Avoid::ConnRef(router0, end1_0, end2_0, 392836745);
	end1_0 = Avoid::ConnEnd(shape0_241539284, 2);
	end2_0 = Avoid::ConnEnd(junction0_471289251);
	Avoid::ConnRef * conn0_836242673 = new Avoid::ConnRef(router0, end1_0, end2_0, 836242673);

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
