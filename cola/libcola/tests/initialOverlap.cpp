// Check that rectangles that start at same position don't get stuck on top of each other
// during force-directed layout.
#include <vector>
#include <utility>
#include "libcola/cola.h"
#include "libcola/pseudorandom.h"

using namespace cola;

int main(void) {
    CompoundConstraints ccs;
    std::vector<Edge> es;
    EdgeLengths eLengths;
    double defaultEdgeLength=1;
    std::vector<vpsc::Rectangle*> rs;
    vpsc::Rectangle *rect = nullptr;

    rect = new vpsc::Rectangle(0, 100, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 100, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 100, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 20, 0, 20);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 20, 0, 20);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 20, 0, 20);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 100, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 20, 0, 20);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 20, 0, 20);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 60, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 100, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 100, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 100, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 100, 0, 60);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(0, 100, 0, 60);
    rs.push_back(rect);
	
    es.push_back(std::make_pair(0, 3));
    es.push_back(std::make_pair(1, 3));
    es.push_back(std::make_pair(2, 3));
    es.push_back(std::make_pair(2, 8));
    es.push_back(std::make_pair(2, 11));
    es.push_back(std::make_pair(3, 4));
    es.push_back(std::make_pair(3, 5));
    es.push_back(std::make_pair(6, 8));
    es.push_back(std::make_pair(7, 8));
    es.push_back(std::make_pair(8, 22));
    es.push_back(std::make_pair(8, 23));
    es.push_back(std::make_pair(9, 11));
    es.push_back(std::make_pair(10, 11));
    es.push_back(std::make_pair(11, 12));
    es.push_back(std::make_pair(11, 23));
    es.push_back(std::make_pair(12, 13));
    es.push_back(std::make_pair(12, 14));
    es.push_back(std::make_pair(13, 17));
    es.push_back(std::make_pair(13, 18));
    es.push_back(std::make_pair(13, 19));
    es.push_back(std::make_pair(13, 20));
    es.push_back(std::make_pair(14, 15));
    es.push_back(std::make_pair(14, 16));
    es.push_back(std::make_pair(14, 20));
    es.push_back(std::make_pair(14, 21));

    eLengths.resize(25);
    eLengths[0] = 100;
    eLengths[1] = 100;
    eLengths[2] = 100;
    eLengths[3] = 100;
    eLengths[4] = 100;
    eLengths[5] = 100;
    eLengths[6] = 100;
    eLengths[7] = 100;
    eLengths[8] = 100;
    eLengths[9] = 100;
    eLengths[10] = 100;
    eLengths[11] = 100;
    eLengths[12] = 100;
    eLengths[13] = 100;
    eLengths[14] = 100;
    eLengths[15] = 100;
    eLengths[16] = 100;
    eLengths[17] = 100;
    eLengths[18] = 100;
    eLengths[19] = 100;
    eLengths[20] = 100;
    eLengths[21] = 100;
    eLengths[22] = 100;
    eLengths[23] = 100;
    eLengths[24] = 100;

    ConstrainedFDLayout alg(rs, es, defaultEdgeLength, eLengths);
    alg.run();
    //alg.outputInstanceToSVG("initialOverlap");

    bool overlaps = false;

    for (size_t i = 0; i < rs.size(); ++i)
    {
	for (size_t j = i + 1; j < rs.size(); ++j)
	{
	    overlaps |= ((rs[i]->overlapD(0, rs[j]) > 0) && (rs[i]->overlapD(1, rs[j]) > 0));
	    if (overlaps)
	    {
		break;
	    }
	}
        if (overlaps)
        {
	    break;
        }
    }
    
    alg.freeAssociatedObjects();

    return (overlaps) ? 1 : 0;
};
