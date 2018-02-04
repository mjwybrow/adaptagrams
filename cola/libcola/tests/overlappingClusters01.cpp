// Based on debug file from Christoph Daniel Schulze 
// cgraph_3_after_layout.svg
#include <vector>
#include <utility>
#include "libcola/cola.h"
using namespace cola;
int main(void) {
    CompoundConstraints ccs;
    std::vector<Edge> es;
    EdgeLengths eLengths;
    double defaultEdgeLength=50;
    std::vector<vpsc::Rectangle*> rs;
    vpsc::Rectangle *rect = nullptr;

    rect = new vpsc::Rectangle(245.517, 247.517, 279.574, 281.574);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(222.742, 224.742, 318.708, 320.708);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(460.345, 462.345, 716.936, 718.936);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(607.846, 609.846, 665.496, 667.496);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(640.191, 642.191, 637.391, 639.391);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(743.519, 745.519, 865.48, 867.48);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(807.112, 809.112, 759.798, 761.798);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(807.112, 809.112, 699.132, 701.132);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(121.997, 123.997, 221.049, 223.049);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(224.354, 226.354, 80, 82);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(448.583, 450.583, 517.722, 519.722);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(460.345, 462.345, 639.516, 641.516);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(607.846, 609.846, 583.991, 585.991);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(607.846, 609.846, 684.476, 686.476);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(625.623, 627.623, 602.774, 604.774);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(289.325, 291.325, 455.243, 457.243);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(207.785, 209.785, 222.762, 224.762);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(159.658, 161.658, 254.197, 256.197);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(650.816, 652.816, 809.48, 811.48);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(685.012, 687.012, 795.932, 797.932);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(751.112, 753.112, 643.132, 645.132);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(751.112, 753.112, 608.903, 610.903);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(320.184, 322.184, 377.132, 379.132);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(336.233, 338.233, 382.124, 384.124);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(153.571, 155.571, 391.657, 393.657);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(405.396, 407.396, 745.874, 747.874);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(460.345, 462.345, 727.442, 729.442);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(551.846, 553.846, 527.991, 529.991);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(669.714, 671.714, 583.991, 585.991);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(171.168, 221.168, 258.197, 308.197);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(110.328, 160.328, 266.708, 316.708);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(217.18, 267.18, 109.481, 159.481);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(632.075, 682.075, 858.531, 908.531);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(689.012, 739.012, 813.48, 863.48);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(755.112, 805.112, 647.132, 697.132);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(786.403, 836.403, 560.258, 610.258);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(261.651, 311.651, 247.06, 297.06);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(304.164, 354.164, 322.708, 372.708);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(70, 120, 325.132, 375.132);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(353.92, 403.92, 770.169, 820.169);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(464.345, 514.345, 749.874, 799.874);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(555.846, 605.846, 531.991, 581.991);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(696.038, 746.038, 531.991, 581.991);
    rs.push_back(rect);

    es.push_back(std::make_pair(0, 1));
    es.push_back(std::make_pair(0, 22));
    es.push_back(std::make_pair(1, 23));
    es.push_back(std::make_pair(1, 24));
    es.push_back(std::make_pair(2, 3));
    es.push_back(std::make_pair(2, 25));
    es.push_back(std::make_pair(3, 4));
    es.push_back(std::make_pair(3, 26));
    es.push_back(std::make_pair(3, 27));
    es.push_back(std::make_pair(4, 28));
    es.push_back(std::make_pair(5, 6));
    es.push_back(std::make_pair(5, 18));
    es.push_back(std::make_pair(6, 7));
    es.push_back(std::make_pair(6, 19));
    es.push_back(std::make_pair(6, 20));
    es.push_back(std::make_pair(7, 21));
    es.push_back(std::make_pair(8, 17));
    es.push_back(std::make_pair(9, 31));
    es.push_back(std::make_pair(10, 11));
    es.push_back(std::make_pair(10, 12));
    es.push_back(std::make_pair(10, 13));
    es.push_back(std::make_pair(10, 14));
    es.push_back(std::make_pair(10, 15));
    es.push_back(std::make_pair(10, 22));
    es.push_back(std::make_pair(10, 23));
    es.push_back(std::make_pair(11, 12));
    es.push_back(std::make_pair(11, 25));
    es.push_back(std::make_pair(11, 26));
    es.push_back(std::make_pair(12, 27));
    es.push_back(std::make_pair(12, 28));
    es.push_back(std::make_pair(13, 14));
    es.push_back(std::make_pair(13, 18));
    es.push_back(std::make_pair(13, 19));
    es.push_back(std::make_pair(14, 20));
    es.push_back(std::make_pair(14, 21));
    es.push_back(std::make_pair(15, 24));
    es.push_back(std::make_pair(16, 17));
    es.push_back(std::make_pair(16, 36));
    es.push_back(std::make_pair(17, 29));
    es.push_back(std::make_pair(17, 30));
    es.push_back(std::make_pair(18, 32));
    es.push_back(std::make_pair(19, 33));
    es.push_back(std::make_pair(20, 34));
    es.push_back(std::make_pair(21, 35));
    es.push_back(std::make_pair(22, 36));
    es.push_back(std::make_pair(23, 37));
    es.push_back(std::make_pair(24, 38));
    es.push_back(std::make_pair(25, 39));
    es.push_back(std::make_pair(26, 40));
    es.push_back(std::make_pair(27, 41));
    es.push_back(std::make_pair(28, 42));
    es.push_back(std::make_pair(31, 36));
    es.push_back(std::make_pair(32, 33));
    es.push_back(std::make_pair(34, 35));
    es.push_back(std::make_pair(36, 37));
    es.push_back(std::make_pair(39, 40));
    es.push_back(std::make_pair(41, 42));

    double padding = 6;
    ConstrainedFDLayout alg(rs, es, defaultEdgeLength, eLengths);
    alg.setAvoidNodeOverlaps(true);
    RootCluster *cluster140389150311456 = new RootCluster();
    cluster140389150311456->addChildNode(0);
    cluster140389150311456->addChildNode(1);
    cluster140389150311456->addChildNode(2);
    cluster140389150311456->addChildNode(3);
    cluster140389150311456->addChildNode(4);
    cluster140389150311456->addChildNode(5);
    cluster140389150311456->addChildNode(6);
    cluster140389150311456->addChildNode(7);
    cluster140389150311456->addChildNode(8);
    cluster140389150311456->addChildNode(9);
    cluster140389150311456->addChildNode(10);
    cluster140389150311456->addChildNode(11);
    cluster140389150311456->addChildNode(12);
    cluster140389150311456->addChildNode(13);
    cluster140389150311456->addChildNode(14);
    cluster140389150311456->addChildNode(15);
    cluster140389150311456->addChildNode(16);
    cluster140389150311456->addChildNode(17);
    cluster140389150311456->addChildNode(18);
    cluster140389150311456->addChildNode(19);
    cluster140389150311456->addChildNode(20);
    cluster140389150311456->addChildNode(21);
    cluster140389150311456->addChildNode(22);
    cluster140389150311456->addChildNode(23);
    cluster140389150311456->addChildNode(24);
    cluster140389150311456->addChildNode(25);
    cluster140389150311456->addChildNode(26);
    cluster140389150311456->addChildNode(27);
    cluster140389150311456->addChildNode(28);
    RectangularCluster *cluster140389152494880 = new RectangularCluster();
    cluster140389152494880->setPadding(padding);
    cluster140389152494880->addChildNode(36);
    cluster140389150311456->addChildCluster(cluster140389152494880);
    RectangularCluster *cluster140389150048048 = new RectangularCluster();
    cluster140389150048048->setPadding(padding);
    cluster140389150048048->addChildNode(38);
    cluster140389150048048->addChildNode(37);
    cluster140389150311456->addChildCluster(cluster140389150048048);
    RectangularCluster *cluster140389150180400 = new RectangularCluster();
    cluster140389150180400->setPadding(padding);
    cluster140389150180400->addChildNode(30);
    cluster140389150180400->addChildNode(29);
    cluster140389150311456->addChildCluster(cluster140389150180400);
    RectangularCluster *cluster140389150298288 = new RectangularCluster();
    cluster140389150298288->setPadding(padding);
    cluster140389150298288->addChildNode(39);
    cluster140389150311456->addChildCluster(cluster140389150298288);
    RectangularCluster *cluster140389150179600 = new RectangularCluster();
    cluster140389150179600->setPadding(padding);
    cluster140389150179600->addChildNode(41);
    cluster140389150179600->addChildNode(40);
    cluster140389150311456->addChildCluster(cluster140389150179600);
    RectangularCluster *cluster140389150131760 = new RectangularCluster();
    cluster140389150131760->setPadding(padding);
    cluster140389150131760->addChildNode(42);
    cluster140389150311456->addChildCluster(cluster140389150131760);
    RectangularCluster *cluster140389150091712 = new RectangularCluster();
    cluster140389150091712->setPadding(padding);
    cluster140389150091712->addChildNode(32);
    cluster140389150311456->addChildCluster(cluster140389150091712);
    RectangularCluster *cluster140389152581264 = new RectangularCluster();
    cluster140389152581264->setPadding(padding);
    cluster140389152581264->addChildNode(34);
    cluster140389152581264->addChildNode(33);
    cluster140389150311456->addChildCluster(cluster140389152581264);
    RectangularCluster *cluster140389144224352 = new RectangularCluster();
    cluster140389144224352->setPadding(padding);
    cluster140389144224352->addChildNode(35);
    cluster140389150311456->addChildCluster(cluster140389144224352);
    RectangularCluster *cluster140389144198304 = new RectangularCluster();
    cluster140389144198304->setPadding(padding);
    cluster140389144198304->addChildNode(30);
    cluster140389144198304->addChildNode(29);
    cluster140389150311456->addChildCluster(cluster140389144198304);
    RectangularCluster *cluster140389144177184 = new RectangularCluster();
    cluster140389144177184->setPadding(padding);
    cluster140389144177184->addChildNode(31);
    cluster140389150311456->addChildCluster(cluster140389144177184);
    RectangularCluster *cluster140389144168944 = new RectangularCluster();
    cluster140389144168944->setPadding(padding);
    cluster140389144168944->addChildNode(36);
    cluster140389144168944->addChildNode(37);
    cluster140389150311456->addChildCluster(cluster140389144168944);
    RectangularCluster *cluster140389144228288 = new RectangularCluster();
    cluster140389144228288->setPadding(padding);
    cluster140389144228288->addChildNode(39);
    cluster140389144228288->addChildNode(40);
    cluster140389150311456->addChildCluster(cluster140389144228288);
    RectangularCluster *cluster140389144244064 = new RectangularCluster();
    cluster140389144228288->setPadding(padding);
    cluster140389144244064->addChildNode(42);
    cluster140389144244064->addChildNode(41);
    cluster140389150311456->addChildCluster(cluster140389144244064);
    RectangularCluster *cluster140389144206000 = new RectangularCluster();
    cluster140389144206000->setPadding(padding);
    cluster140389144206000->addChildNode(32);
    cluster140389144206000->addChildNode(33);
    cluster140389150311456->addChildCluster(cluster140389144206000);
    RectangularCluster *cluster140389144175920 = new RectangularCluster();
    cluster140389144175920->setPadding(padding);
    cluster140389144175920->addChildNode(35);
    cluster140389144175920->addChildNode(34);
    cluster140389150311456->addChildCluster(cluster140389144175920);
    RectangularCluster *cluster140389144214752 = new RectangularCluster();
    cluster140389144214752->setPadding(padding);
    cluster140389144214752->addChildNode(38);
    cluster140389150311456->addChildCluster(cluster140389144214752);
    alg.setClusterHierarchy(cluster140389150311456);
    alg.setConstraints(ccs);

    UnsatisfiableConstraintInfos unsatisfiableX, unsatisfiableY;
    alg.setUnsatisfiableConstraintInfo(&unsatisfiableX, &unsatisfiableY);
    
    //alg.makeFeasible();
    alg.run();
    //alg.outputInstanceToSVG("overlappingClusters01");

    for (size_t i = 0; i < unsatisfiableX.size(); ++i)
    {
	printf("%s\n", unsatisfiableX[i]->toString().c_str());
    }
    for (size_t i = 0; i < unsatisfiableY.size(); ++i)
    {
	printf("%s\n", unsatisfiableY[i]->toString().c_str());
    }
    alg.freeAssociatedObjects();
    return (unsatisfiableX.empty() && unsatisfiableY.empty()) ? 0 : 1;
};
