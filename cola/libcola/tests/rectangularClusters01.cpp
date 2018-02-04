#include <vector>
#include <utility>
#include "libcola/cola.h"
using namespace cola;
int main(void) {
    CompoundConstraints ccs;
    std::vector<Edge> es;
    EdgeLengths eLengths;
    double defaultEdgeLength=1;
    std::vector<vpsc::Rectangle*> rs;
    vpsc::Rectangle *rect = nullptr;

    rect = new vpsc::Rectangle(56.4457, 117.446, 954.77, 1043.77);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(119.446, 127.446, 992.02, 1000.02);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(119.446, 127.446, 1004.27, 1012.27);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(119.446, 127.446, 1016.52, 1024.52);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(46.4457, 54.4457, 1004.27, 1012.27);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(2165.45, 2239.45, 976.562, 1065.56);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(2155.45, 2163.45, 1026.06, 1034.06);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(-106.554, 6.44565, 955.683, 1044.68);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(8.44565, 16.4457, 1005.18, 1013.18);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(331.446, 412.446, 1015.55, 1104.55);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(414.446, 422.446, 1065.05, 1073.05);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(356.112, 364.112, 1106.55, 1114.55);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(339.779, 347.779, 1106.55, 1114.55);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(321.446, 329.446, 1073.22, 1081.22);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(321.446, 329.446, 1056.88, 1064.88);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(477.446, 559.446, 948.569, 1037.57);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(561.446, 569.446, 998.069, 1006.07);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(503.946, 511.946, 1039.57, 1047.57);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(467.446, 475.446, 998.069, 1006.07);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(56.4457, 127.446, 863.77, 952.77);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(129.446, 137.446, 913.27, 921.27);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(46.4457, 54.4457, 913.27, 921.27);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(177.446, 281.446, 922.444, 1011.44);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(283.446, 291.446, 971.944, 979.944);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(167.446, 175.446, 986.644, 994.644);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(167.446, 175.446, 976.844, 984.844);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(167.446, 175.446, 967.044, 975.044);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(167.446, 175.446, 957.244, 965.244);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(331.446, 427.446, 924.551, 1013.55);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(429.446, 437.446, 974.051, 982.051);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(321.446, 329.446, 974.051, 982.051);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(609.446, 730.446, 936.692, 1025.69);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(732.446, 740.446, 971.492, 979.492);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(732.446, 740.446, 981.292, 989.292);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(732.446, 740.446, 991.092, 999.092);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(732.446, 740.446, 1000.89, 1008.89);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(599.446, 607.446, 986.192, 994.192);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(780.446, 846.446, 838.172, 932.172);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(848.446, 856.446, 890.172, 898.172);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(770.446, 778.446, 890.172, 898.172);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(780.446, 846.446, 934.172, 1028.17);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(848.446, 856.446, 986.172, 994.172);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(770.446, 778.446, 986.172, 994.172);
    rs.push_back(rect);

    // rect-43
    rect = new vpsc::Rectangle(941.446, 1155.45, 922.958, 1025.96);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1147.45, 1155.45, 979.458, 987.458);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(941.446, 949.446, 998.358, 1006.36);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(941.446, 949.446, 985.757, 993.757);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(941.446, 949.446, 973.158, 981.158);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(941.446, 949.446, 960.558, 968.558);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(780.446, 846.446, 1030.17, 1124.17);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(848.446, 856.446, 1082.17, 1090.17);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(770.446, 778.446, 1082.17, 1090.17);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1195.45, 1263.45, 938.653, 1017.65);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1265.45, 1273.45, 983.153, 991.153);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1185.45, 1193.45, 989.653, 997.653);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1185.45, 1193.45, 976.653, 984.653);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1313.45, 1374.45, 934.988, 1013.99);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1376.45, 1384.45, 979.488, 987.488);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1303.45, 1311.45, 979.488, 987.488);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1586.45, 1660.45, 855.704, 944.704);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1576.45, 1584.45, 905.204, 913.204);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1424.45, 1536.45, 866.456, 945.456);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1538.45, 1546.45, 910.956, 918.956);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1414.45, 1422.45, 910.956, 918.956);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1445.45, 1515.45, 947.456, 1036.46);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1517.45, 1525.45, 996.956, 1004.96);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1471.95, 1479.95, 1038.46, 1046.46);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1435.45, 1443.45, 996.956, 1004.96);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1699.45, 1748.45, 970.163, 1059.16);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1750.45, 1758.45, 1019.66, 1027.66);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1689.45, 1697.45, 1027.83, 1035.83);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1689.45, 1697.45, 1011.5, 1019.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1811.45, 1898.45, 1010.04, 1099.04);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1900.45, 1908.45, 1059.54, 1067.54);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1801.45, 1809.45, 1067.7, 1075.7);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1801.45, 1809.45, 1051.37, 1059.37);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1699.45, 1761.45, 1061.16, 1134.16);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1763.45, 1771.45, 1102.66, 1110.66);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1689.45, 1697.45, 1102.66, 1110.66);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(2053.45, 2115.45, 985.999, 1080);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(2117.45, 2125.45, 1038, 1046);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(2080.11, 2088.11, 1082, 1090);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(2062.78, 2070.78, 1082, 1090);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(2043.45, 2051.45, 1038, 1046);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1948.45, 2003.45, 996.147, 1090.15);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(2005.45, 2013.45, 1048.15, 1056.15);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1975.11, 1983.11, 1092.15, 1100.15);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1957.78, 1965.78, 1092.15, 1100.15);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1938.45, 1946.45, 1048.15, 1056.15);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1565.45, 1649.45, 951.936, 1040.94);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1651.45, 1659.45, 1001.44, 1009.44);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(1555.45, 1563.45, 1001.44, 1009.44);
    rs.push_back(rect);

    es.push_back(std::make_pair(0, 1));
    es.push_back(std::make_pair(0, 2));
    es.push_back(std::make_pair(0, 3));
    es.push_back(std::make_pair(0, 4));
    es.push_back(std::make_pair(1, 26));
    es.push_back(std::make_pair(2, 25));
    es.push_back(std::make_pair(3, 6));
    es.push_back(std::make_pair(3, 24));
    es.push_back(std::make_pair(4, 8));
    es.push_back(std::make_pair(5, 6));
    es.push_back(std::make_pair(6, 80));
    es.push_back(std::make_pair(7, 8));
    es.push_back(std::make_pair(9, 10));
    es.push_back(std::make_pair(9, 11));
    es.push_back(std::make_pair(9, 12));
    es.push_back(std::make_pair(9, 13));
    es.push_back(std::make_pair(9, 14));
    es.push_back(std::make_pair(10, 17));
    es.push_back(std::make_pair(15, 16));
    es.push_back(std::make_pair(15, 17));
    es.push_back(std::make_pair(15, 18));
    es.push_back(std::make_pair(16, 36));
    es.push_back(std::make_pair(16, 67));
    es.push_back(std::make_pair(18, 29));
    es.push_back(std::make_pair(19, 20));
    es.push_back(std::make_pair(19, 21));
    es.push_back(std::make_pair(20, 27));
    es.push_back(std::make_pair(22, 23));
    es.push_back(std::make_pair(22, 24));
    es.push_back(std::make_pair(22, 25));
    es.push_back(std::make_pair(22, 26));
    es.push_back(std::make_pair(22, 27));
    es.push_back(std::make_pair(23, 30));
    es.push_back(std::make_pair(28, 29));
    es.push_back(std::make_pair(28, 30));
    es.push_back(std::make_pair(31, 32));
    es.push_back(std::make_pair(31, 33));
    es.push_back(std::make_pair(31, 34));
    es.push_back(std::make_pair(31, 35));
    es.push_back(std::make_pair(31, 36));
    es.push_back(std::make_pair(32, 39));
    es.push_back(std::make_pair(32, 48));
    es.push_back(std::make_pair(34, 42));
    es.push_back(std::make_pair(35, 51));
    es.push_back(std::make_pair(35, 54));
    es.push_back(std::make_pair(37, 38));
    es.push_back(std::make_pair(37, 39));
    es.push_back(std::make_pair(38, 47));
    es.push_back(std::make_pair(40, 41));
    es.push_back(std::make_pair(40, 42));
    es.push_back(std::make_pair(41, 46));
    es.push_back(std::make_pair(43, 44));
    es.push_back(std::make_pair(43, 45));
    es.push_back(std::make_pair(43, 46));
    es.push_back(std::make_pair(43, 47));
    es.push_back(std::make_pair(43, 48));
    es.push_back(std::make_pair(44, 55));
    es.push_back(std::make_pair(45, 50));
    es.push_back(std::make_pair(49, 50));
    es.push_back(std::make_pair(49, 51));
    es.push_back(std::make_pair(52, 53));
    es.push_back(std::make_pair(52, 54));
    es.push_back(std::make_pair(52, 55));
    es.push_back(std::make_pair(53, 58));
    es.push_back(std::make_pair(56, 57));
    es.push_back(std::make_pair(56, 58));
    es.push_back(std::make_pair(57, 63));
    es.push_back(std::make_pair(57, 66));
    es.push_back(std::make_pair(59, 60));
    es.push_back(std::make_pair(60, 62));
    es.push_back(std::make_pair(61, 62));
    es.push_back(std::make_pair(61, 63));
    es.push_back(std::make_pair(64, 65));
    es.push_back(std::make_pair(64, 66));
    es.push_back(std::make_pair(64, 67));
    es.push_back(std::make_pair(65, 91));
    es.push_back(std::make_pair(68, 69));
    es.push_back(std::make_pair(68, 70));
    es.push_back(std::make_pair(68, 71));
    es.push_back(std::make_pair(69, 75));
    es.push_back(std::make_pair(70, 85));
    es.push_back(std::make_pair(71, 90));
    es.push_back(std::make_pair(72, 73));
    es.push_back(std::make_pair(72, 74));
    es.push_back(std::make_pair(72, 75));
    es.push_back(std::make_pair(73, 88));
    es.push_back(std::make_pair(75, 77));
    es.push_back(std::make_pair(76, 77));
    es.push_back(std::make_pair(76, 78));
    es.push_back(std::make_pair(79, 80));
    es.push_back(std::make_pair(79, 81));
    es.push_back(std::make_pair(79, 82));
    es.push_back(std::make_pair(79, 83));
    es.push_back(std::make_pair(83, 85));
    es.push_back(std::make_pair(84, 85));
    es.push_back(std::make_pair(84, 86));
    es.push_back(std::make_pair(84, 87));
    es.push_back(std::make_pair(84, 88));
    es.push_back(std::make_pair(89, 90));
    es.push_back(std::make_pair(89, 91));

    eLengths.resize(100);
    eLengths[0] = 59.6059;
    eLengths[1] = 59.6059;
    eLengths[2] = 59.6059;
    eLengths[3] = 59.6059;
    eLengths[4] = 63.5296;
    eLengths[5] = 77.5769;
    eLengths[6] = 65.8274;
    eLengths[7] = 65.8274;
    eLengths[8] = 65.8274;
    eLengths[9] = 65.8274;
    eLengths[10] = 65.8274;
    eLengths[11] = 66.1651;
    eLengths[12] = 66.1651;
    eLengths[13] = 66.1651;
    eLengths[14] = 62.5822;
    eLengths[15] = 62.5822;
    eLengths[16] = 74.0984;
    eLengths[17] = 74.0984;
    eLengths[18] = 74.0984;
    eLengths[19] = 74.0984;
    eLengths[20] = 74.0984;
    eLengths[21] = 71.111;
    eLengths[22] = 71.111;
    eLengths[23] = 80.7601;
    eLengths[24] = 80.7601;
    eLengths[25] = 80.7601;
    eLengths[26] = 80.7601;
    eLengths[27] = 80.7601;
    eLengths[28] = 63.0851;
    eLengths[29] = 63.0851;
    eLengths[30] = 63.0851;
    eLengths[31] = 63.0851;
    eLengths[32] = 115.481;
    eLengths[33] = 115.481;
    eLengths[34] = 115.481;
    eLengths[35] = 115.481;
    eLengths[36] = 115.481;
    eLengths[37] = 63.0851;
    eLengths[38] = 63.0851;
    eLengths[39] = 57.7745;
    eLengths[40] = 57.7745;
    eLengths[41] = 57.7745;
    eLengths[42] = 55.5618;
    eLengths[43] = 55.5618;
    eLengths[44] = 63.5296;
    eLengths[45] = 74.186;
    eLengths[46] = 74.186;
    eLengths[47] = 62.2718;
    eLengths[48] = 62.2718;
    eLengths[49] = 62.2718;
    eLengths[50] = 56.4555;
    eLengths[51] = 56.4555;
    eLengths[52] = 56.4555;
    eLengths[53] = 67.8863;
    eLengths[54] = 67.8863;
    eLengths[55] = 67.8863;
    eLengths[56] = 53.5447;
    eLengths[57] = 53.5447;
    eLengths[58] = 61.9596;
    eLengths[59] = 61.9596;
    eLengths[60] = 61.9596;
    eLengths[61] = 61.9596;
    eLengths[62] = 60.111;
    eLengths[63] = 60.111;
    eLengths[64] = 60.111;
    eLengths[65] = 60.111;
    eLengths[66] = 66.8471;
    eLengths[67] = 66.8471;
    eLengths[68] = 18.3712;
    eLengths[69] = 18.3712;
    eLengths[70] = 18.3712;
    eLengths[71] = 18.3712;
    eLengths[72] = 10.6066;
    eLengths[73] = 10.6066;
    eLengths[74] = 15;
    eLengths[75] = 15;
    eLengths[76] = 15;
    eLengths[77] = 10.6066;
    eLengths[78] = 10.6066;
    eLengths[79] = 23.7171;
    eLengths[80] = 30;
    eLengths[81] = 23.7171;
    eLengths[82] = 23.7171;
    eLengths[83] = 25.9808;
    eLengths[84] = 21.2132;
    eLengths[85] = 21.2132;
    eLengths[86] = 15;
    eLengths[87] = 21.2132;
    eLengths[88] = 10.6066;
    eLengths[89] = 15;
    eLengths[90] = 15;
    eLengths[91] = 10.6066;
    eLengths[92] = 10.6066;
    eLengths[93] = 15;
    eLengths[94] = 10.6066;
    eLengths[95] = 15;
    eLengths[96] = 15;
    eLengths[97] = 15;
    eLengths[98] = 18.3712;
    eLengths[99] = 15;

    SeparationConstraint *separation479625808 = new SeparationConstraint(vpsc::XDIM, 0, 1, 36.5, true);
    ccs.push_back(separation479625808);

    SeparationConstraint *separation479626000 = new SeparationConstraint(vpsc::YDIM, 0, 1, -3.25, true);
    ccs.push_back(separation479626000);

    SeparationConstraint *separation479625936 = new SeparationConstraint(vpsc::XDIM, 0, 2, 36.5, true);
    ccs.push_back(separation479625936);

    SeparationConstraint *separation479626128 = new SeparationConstraint(vpsc::YDIM, 0, 2, 9, true);
    ccs.push_back(separation479626128);

    SeparationConstraint *separation479626064 = new SeparationConstraint(vpsc::XDIM, 0, 3, 36.5, true);
    ccs.push_back(separation479626064);

    SeparationConstraint *separation479626256 = new SeparationConstraint(vpsc::YDIM, 0, 3, 21.25, true);
    ccs.push_back(separation479626256);

    SeparationConstraint *separation479626192 = new SeparationConstraint(vpsc::XDIM, 0, 4, -36.5, true);
    ccs.push_back(separation479626192);

    SeparationConstraint *separation479626384 = new SeparationConstraint(vpsc::YDIM, 0, 4, 9, true);
    ccs.push_back(separation479626384);

    SeparationConstraint *separation479626320 = new SeparationConstraint(vpsc::XDIM, 5, 6, -43, true);
    ccs.push_back(separation479626320);

    SeparationConstraint *separation479626512 = new SeparationConstraint(vpsc::YDIM, 5, 6, 9, true);
    ccs.push_back(separation479626512);

    SeparationConstraint *separation479626448 = new SeparationConstraint(vpsc::XDIM, 7, 8, 62.5, true);
    ccs.push_back(separation479626448);

    SeparationConstraint *separation479626640 = new SeparationConstraint(vpsc::YDIM, 7, 8, 9, true);
    ccs.push_back(separation479626640);

    SeparationConstraint *separation479626576 = new SeparationConstraint(vpsc::XDIM, 9, 10, 46.5, true);
    ccs.push_back(separation479626576);

    SeparationConstraint *separation479626768 = new SeparationConstraint(vpsc::YDIM, 9, 10, 9, true);
    ccs.push_back(separation479626768);

    SeparationConstraint *separation479626704 = new SeparationConstraint(vpsc::XDIM, 9, 11, -11.8333, true);
    ccs.push_back(separation479626704);

    SeparationConstraint *separation479626896 = new SeparationConstraint(vpsc::YDIM, 9, 11, 50.5, true);
    ccs.push_back(separation479626896);

    SeparationConstraint *separation479626832 = new SeparationConstraint(vpsc::XDIM, 9, 12, -28.1667, true);
    ccs.push_back(separation479626832);

    SeparationConstraint *separation479627024 = new SeparationConstraint(vpsc::YDIM, 9, 12, 50.5, true);
    ccs.push_back(separation479627024);

    SeparationConstraint *separation479626960 = new SeparationConstraint(vpsc::XDIM, 9, 13, -46.5, true);
    ccs.push_back(separation479626960);

    SeparationConstraint *separation479627152 = new SeparationConstraint(vpsc::YDIM, 9, 13, 17.1667, true);
    ccs.push_back(separation479627152);

    SeparationConstraint *separation479627088 = new SeparationConstraint(vpsc::XDIM, 9, 14, -46.5, true);
    ccs.push_back(separation479627088);

    SeparationConstraint *separation479627280 = new SeparationConstraint(vpsc::YDIM, 9, 14, 0.833333, true);
    ccs.push_back(separation479627280);

    SeparationConstraint *separation479627216 = new SeparationConstraint(vpsc::XDIM, 15, 16, 47, true);
    ccs.push_back(separation479627216);

    SeparationConstraint *separation479627408 = new SeparationConstraint(vpsc::YDIM, 15, 16, 9, true);
    ccs.push_back(separation479627408);

    SeparationConstraint *separation479627344 = new SeparationConstraint(vpsc::XDIM, 15, 17, -10.5, true);
    ccs.push_back(separation479627344);

    SeparationConstraint *separation479627536 = new SeparationConstraint(vpsc::YDIM, 15, 17, 50.5, true);
    ccs.push_back(separation479627536);

    SeparationConstraint *separation479627472 = new SeparationConstraint(vpsc::XDIM, 15, 18, -47, true);
    ccs.push_back(separation479627472);

    SeparationConstraint *separation479627664 = new SeparationConstraint(vpsc::YDIM, 15, 18, 9, true);
    ccs.push_back(separation479627664);

    SeparationConstraint *separation479627600 = new SeparationConstraint(vpsc::XDIM, 19, 20, 41.5, true);
    ccs.push_back(separation479627600);

    SeparationConstraint *separation479627792 = new SeparationConstraint(vpsc::YDIM, 19, 20, 9, true);
    ccs.push_back(separation479627792);

    SeparationConstraint *separation479627728 = new SeparationConstraint(vpsc::XDIM, 19, 21, -41.5, true);
    ccs.push_back(separation479627728);

    SeparationConstraint *separation479627920 = new SeparationConstraint(vpsc::YDIM, 19, 21, 9, true);
    ccs.push_back(separation479627920);

    SeparationConstraint *separation479627856 = new SeparationConstraint(vpsc::XDIM, 22, 23, 58, true);
    ccs.push_back(separation479627856);

    SeparationConstraint *separation479628048 = new SeparationConstraint(vpsc::YDIM, 22, 23, 9, true);
    ccs.push_back(separation479628048);

    SeparationConstraint *separation479627984 = new SeparationConstraint(vpsc::XDIM, 22, 24, -58, true);
    ccs.push_back(separation479627984);

    SeparationConstraint *separation479628176 = new SeparationConstraint(vpsc::YDIM, 22, 24, 23.7, true);
    ccs.push_back(separation479628176);

    SeparationConstraint *separation479628112 = new SeparationConstraint(vpsc::XDIM, 22, 25, -58, true);
    ccs.push_back(separation479628112);

    SeparationConstraint *separation479628304 = new SeparationConstraint(vpsc::YDIM, 22, 25, 13.9, true);
    ccs.push_back(separation479628304);

    SeparationConstraint *separation479628240 = new SeparationConstraint(vpsc::XDIM, 22, 26, -58, true);
    ccs.push_back(separation479628240);

    SeparationConstraint *separation479628432 = new SeparationConstraint(vpsc::YDIM, 22, 26, 4.1, true);
    ccs.push_back(separation479628432);

    SeparationConstraint *separation479628368 = new SeparationConstraint(vpsc::XDIM, 22, 27, -58, true);
    ccs.push_back(separation479628368);

    SeparationConstraint *separation479628560 = new SeparationConstraint(vpsc::YDIM, 22, 27, -5.7, true);
    ccs.push_back(separation479628560);

    SeparationConstraint *separation479628496 = new SeparationConstraint(vpsc::XDIM, 28, 29, 54, true);
    ccs.push_back(separation479628496);

    SeparationConstraint *separation479628688 = new SeparationConstraint(vpsc::YDIM, 28, 29, 9, true);
    ccs.push_back(separation479628688);

    SeparationConstraint *separation479628624 = new SeparationConstraint(vpsc::XDIM, 28, 30, -54, true);
    ccs.push_back(separation479628624);

    SeparationConstraint *separation479628816 = new SeparationConstraint(vpsc::YDIM, 28, 30, 9, true);
    ccs.push_back(separation479628816);

    SeparationConstraint *separation479628752 = new SeparationConstraint(vpsc::XDIM, 31, 32, 66.5, true);
    ccs.push_back(separation479628752);

    SeparationConstraint *separation479622736 = new SeparationConstraint(vpsc::YDIM, 31, 32, -5.7, true);
    ccs.push_back(separation479622736);

    SeparationConstraint *separation479622800 = new SeparationConstraint(vpsc::XDIM, 31, 33, 66.5, true);
    ccs.push_back(separation479622800);

    SeparationConstraint *separation479622864 = new SeparationConstraint(vpsc::YDIM, 31, 33, 4.1, true);
    ccs.push_back(separation479622864);

    SeparationConstraint *separation479622928 = new SeparationConstraint(vpsc::XDIM, 31, 34, 66.5, true);
    ccs.push_back(separation479622928);

    SeparationConstraint *separation479622992 = new SeparationConstraint(vpsc::YDIM, 31, 34, 13.9, true);
    ccs.push_back(separation479622992);

    SeparationConstraint *separation479623056 = new SeparationConstraint(vpsc::XDIM, 31, 35, 66.5, true);
    ccs.push_back(separation479623056);

    SeparationConstraint *separation479623120 = new SeparationConstraint(vpsc::YDIM, 31, 35, 23.7, true);
    ccs.push_back(separation479623120);

    SeparationConstraint *separation479623184 = new SeparationConstraint(vpsc::XDIM, 31, 36, -66.5, true);
    ccs.push_back(separation479623184);

    SeparationConstraint *separation479623248 = new SeparationConstraint(vpsc::YDIM, 31, 36, 9, true);
    ccs.push_back(separation479623248);

    SeparationConstraint *separation479623312 = new SeparationConstraint(vpsc::XDIM, 37, 38, 39, true);
    ccs.push_back(separation479623312);

    SeparationConstraint *separation479623440 = new SeparationConstraint(vpsc::YDIM, 37, 38, 9, true);
    ccs.push_back(separation479623440);

    SeparationConstraint *separation479623504 = new SeparationConstraint(vpsc::XDIM, 37, 39, -39, true);
    ccs.push_back(separation479623504);

    SeparationConstraint *separation479623568 = new SeparationConstraint(vpsc::YDIM, 37, 39, 9, true);
    ccs.push_back(separation479623568);

    SeparationConstraint *separation479623632 = new SeparationConstraint(vpsc::XDIM, 40, 41, 39, true);
    ccs.push_back(separation479623632);

    SeparationConstraint *separation479623696 = new SeparationConstraint(vpsc::YDIM, 40, 41, 9, true);
    ccs.push_back(separation479623696);

    SeparationConstraint *separation479623760 = new SeparationConstraint(vpsc::XDIM, 40, 42, -39, true);
    ccs.push_back(separation479623760);

    SeparationConstraint *separation479623888 = new SeparationConstraint(vpsc::YDIM, 40, 42, 9, true);
    ccs.push_back(separation479623888);

    SeparationConstraint *separation479623952 = new SeparationConstraint(vpsc::XDIM, 43, 44, 103, true);
    ccs.push_back(separation479623952);

    SeparationConstraint *separation479624016 = new SeparationConstraint(vpsc::YDIM, 43, 44, 9, true);
    ccs.push_back(separation479624016);

    SeparationConstraint *separation479624080 = new SeparationConstraint(vpsc::XDIM, 43, 45, -103, true);
    ccs.push_back(separation479624080);

    SeparationConstraint *separation479624144 = new SeparationConstraint(vpsc::YDIM, 43, 45, 27.9, true);
    ccs.push_back(separation479624144);

    SeparationConstraint *separation479624208 = new SeparationConstraint(vpsc::XDIM, 43, 46, -103, true);
    ccs.push_back(separation479624208);

    SeparationConstraint *separation479624272 = new SeparationConstraint(vpsc::YDIM, 43, 46, 15.3, true);
    ccs.push_back(separation479624272);

    SeparationConstraint *separation479624336 = new SeparationConstraint(vpsc::XDIM, 43, 47, -103, true);
    ccs.push_back(separation479624336);

    SeparationConstraint *separation479624400 = new SeparationConstraint(vpsc::YDIM, 43, 47, 2.7, true);
    ccs.push_back(separation479624400);

    SeparationConstraint *separation479624528 = new SeparationConstraint(vpsc::XDIM, 43, 48, -103, true);
    ccs.push_back(separation479624528);

    SeparationConstraint *separation479624592 = new SeparationConstraint(vpsc::YDIM, 43, 48, -9.9, true);
    ccs.push_back(separation479624592);

    SeparationConstraint *separation479624656 = new SeparationConstraint(vpsc::XDIM, 49, 50, 39, true);
    ccs.push_back(separation479624656);

    SeparationConstraint *separation479624720 = new SeparationConstraint(vpsc::YDIM, 49, 50, 9, true);
    ccs.push_back(separation479624720);

    SeparationConstraint *separation479624784 = new SeparationConstraint(vpsc::XDIM, 49, 51, -39, true);
    ccs.push_back(separation479624784);

    SeparationConstraint *separation479624848 = new SeparationConstraint(vpsc::YDIM, 49, 51, 9, true);
    ccs.push_back(separation479624848);

    SeparationConstraint *separation479624912 = new SeparationConstraint(vpsc::XDIM, 52, 53, 40, true);
    ccs.push_back(separation479624912);

    SeparationConstraint *separation479624976 = new SeparationConstraint(vpsc::YDIM, 52, 53, 9, true);
    ccs.push_back(separation479624976);

    SeparationConstraint *separation479625040 = new SeparationConstraint(vpsc::XDIM, 52, 54, -40, true);
    ccs.push_back(separation479625040);

    SeparationConstraint *separation479625104 = new SeparationConstraint(vpsc::YDIM, 52, 54, 15.5, true);
    ccs.push_back(separation479625104);

    SeparationConstraint *separation479625168 = new SeparationConstraint(vpsc::XDIM, 52, 55, -40, true);
    ccs.push_back(separation479625168);

    SeparationConstraint *separation479625232 = new SeparationConstraint(vpsc::YDIM, 52, 55, 2.5, true);
    ccs.push_back(separation479625232);

    SeparationConstraint *separation479625296 = new SeparationConstraint(vpsc::XDIM, 56, 57, 36.5, true);
    ccs.push_back(separation479625296);

    SeparationConstraint *separation479625360 = new SeparationConstraint(vpsc::YDIM, 56, 57, 9, true);
    ccs.push_back(separation479625360);

    SeparationConstraint *separation479625424 = new SeparationConstraint(vpsc::XDIM, 56, 58, -36.5, true);
    ccs.push_back(separation479625424);

    SeparationConstraint *separation479625552 = new SeparationConstraint(vpsc::YDIM, 56, 58, 9, true);
    ccs.push_back(separation479625552);

    SeparationConstraint *separation479625616 = new SeparationConstraint(vpsc::XDIM, 59, 60, -43, true);
    ccs.push_back(separation479625616);

    SeparationConstraint *separation479625680 = new SeparationConstraint(vpsc::YDIM, 59, 60, 9, true);
    ccs.push_back(separation479625680);

    SeparationConstraint *separation479625744 = new SeparationConstraint(vpsc::XDIM, 61, 62, 62, true);
    ccs.push_back(separation479625744);

    SeparationConstraint *separation479625488 = new SeparationConstraint(vpsc::YDIM, 61, 62, 9, true);
    ccs.push_back(separation479625488);

    SeparationConstraint *separation479624464 = new SeparationConstraint(vpsc::XDIM, 61, 63, -62, true);
    ccs.push_back(separation479624464);

    SeparationConstraint *separation479623824 = new SeparationConstraint(vpsc::YDIM, 61, 63, 9, true);
    ccs.push_back(separation479623824);

    SeparationConstraint *separation479623376 = new SeparationConstraint(vpsc::XDIM, 64, 65, 41, true);
    ccs.push_back(separation479623376);

    SeparationConstraint *separation479628944 = new SeparationConstraint(vpsc::YDIM, 64, 65, 9, true);
    ccs.push_back(separation479628944);

    SeparationConstraint *separation479628880 = new SeparationConstraint(vpsc::XDIM, 64, 66, -4.5, true);
    ccs.push_back(separation479628880);

    SeparationConstraint *separation479629008 = new SeparationConstraint(vpsc::YDIM, 64, 66, 50.5, true);
    ccs.push_back(separation479629008);

    SeparationConstraint *separation479629072 = new SeparationConstraint(vpsc::XDIM, 64, 67, -41, true);
    ccs.push_back(separation479629072);

    SeparationConstraint *separation479629136 = new SeparationConstraint(vpsc::YDIM, 64, 67, 9, true);
    ccs.push_back(separation479629136);

    SeparationConstraint *separation479629200 = new SeparationConstraint(vpsc::XDIM, 68, 69, 30.5, true);
    ccs.push_back(separation479629200);

    SeparationConstraint *separation479629264 = new SeparationConstraint(vpsc::YDIM, 68, 69, 9, true);
    ccs.push_back(separation479629264);

    SeparationConstraint *separation479629328 = new SeparationConstraint(vpsc::XDIM, 68, 70, -30.5, true);
    ccs.push_back(separation479629328);

    SeparationConstraint *separation479629392 = new SeparationConstraint(vpsc::YDIM, 68, 70, 17.1667, true);
    ccs.push_back(separation479629392);

    SeparationConstraint *separation479629456 = new SeparationConstraint(vpsc::XDIM, 68, 71, -30.5, true);
    ccs.push_back(separation479629456);

    SeparationConstraint *separation479629520 = new SeparationConstraint(vpsc::YDIM, 68, 71, 0.833333, true);
    ccs.push_back(separation479629520);

    SeparationConstraint *separation479629584 = new SeparationConstraint(vpsc::XDIM, 72, 73, 49.5, true);
    ccs.push_back(separation479629584);

    SeparationConstraint *separation479629648 = new SeparationConstraint(vpsc::YDIM, 72, 73, 9, true);
    ccs.push_back(separation479629648);

    SeparationConstraint *separation479629712 = new SeparationConstraint(vpsc::XDIM, 72, 74, -49.5, true);
    ccs.push_back(separation479629712);

    SeparationConstraint *separation479629776 = new SeparationConstraint(vpsc::YDIM, 72, 74, 17.1667, true);
    ccs.push_back(separation479629776);

    SeparationConstraint *separation479629840 = new SeparationConstraint(vpsc::XDIM, 72, 75, -49.5, true);
    ccs.push_back(separation479629840);

    SeparationConstraint *separation479629904 = new SeparationConstraint(vpsc::YDIM, 72, 75, 0.833333, true);
    ccs.push_back(separation479629904);

    SeparationConstraint *separation479629968 = new SeparationConstraint(vpsc::XDIM, 76, 77, 37, true);
    ccs.push_back(separation479629968);

    SeparationConstraint *separation479630032 = new SeparationConstraint(vpsc::YDIM, 76, 77, 9, true);
    ccs.push_back(separation479630032);

    SeparationConstraint *separation479630096 = new SeparationConstraint(vpsc::XDIM, 76, 78, -37, true);
    ccs.push_back(separation479630096);

    SeparationConstraint *separation479630160 = new SeparationConstraint(vpsc::YDIM, 76, 78, 9, true);
    ccs.push_back(separation479630160);

    SeparationConstraint *separation479630224 = new SeparationConstraint(vpsc::XDIM, 79, 80, 37, true);
    ccs.push_back(separation479630224);

    SeparationConstraint *separation479630288 = new SeparationConstraint(vpsc::YDIM, 79, 80, 9, true);
    ccs.push_back(separation479630288);

    SeparationConstraint *separation479630352 = new SeparationConstraint(vpsc::XDIM, 79, 81, -0.333334, true);
    ccs.push_back(separation479630352);

    SeparationConstraint *separation479630416 = new SeparationConstraint(vpsc::YDIM, 79, 81, 53, true);
    ccs.push_back(separation479630416);

    SeparationConstraint *separation477486176 = new SeparationConstraint(vpsc::XDIM, 79, 82, -17.6667, true);
    ccs.push_back(separation477486176);

    SeparationConstraint *separation477486240 = new SeparationConstraint(vpsc::YDIM, 79, 82, 53, true);
    ccs.push_back(separation477486240);

    SeparationConstraint *separation477486304 = new SeparationConstraint(vpsc::XDIM, 79, 83, -37, true);
    ccs.push_back(separation477486304);

    SeparationConstraint *separation477486368 = new SeparationConstraint(vpsc::YDIM, 79, 83, 9, true);
    ccs.push_back(separation477486368);

    SeparationConstraint *separation477486432 = new SeparationConstraint(vpsc::XDIM, 84, 85, 33.5, true);
    ccs.push_back(separation477486432);

    SeparationConstraint *separation477486496 = new SeparationConstraint(vpsc::YDIM, 84, 85, 9, true);
    ccs.push_back(separation477486496);

    SeparationConstraint *separation477486560 = new SeparationConstraint(vpsc::XDIM, 84, 86, 3.16667, true);
    ccs.push_back(separation477486560);

    SeparationConstraint *separation477486624 = new SeparationConstraint(vpsc::YDIM, 84, 86, 53, true);
    ccs.push_back(separation477486624);

    SeparationConstraint *separation477486688 = new SeparationConstraint(vpsc::XDIM, 84, 87, -14.1667, true);
    ccs.push_back(separation477486688);

    SeparationConstraint *separation477486752 = new SeparationConstraint(vpsc::YDIM, 84, 87, 53, true);
    ccs.push_back(separation477486752);

    SeparationConstraint *separation477486816 = new SeparationConstraint(vpsc::XDIM, 84, 88, -33.5, true);
    ccs.push_back(separation477486816);

    SeparationConstraint *separation477486880 = new SeparationConstraint(vpsc::YDIM, 84, 88, 9, true);
    ccs.push_back(separation477486880);

    SeparationConstraint *separation477486944 = new SeparationConstraint(vpsc::XDIM, 89, 90, 48, true);
    ccs.push_back(separation477486944);

    SeparationConstraint *separation477487008 = new SeparationConstraint(vpsc::YDIM, 89, 90, 9, true);
    ccs.push_back(separation477487008);

    SeparationConstraint *separation477487072 = new SeparationConstraint(vpsc::XDIM, 89, 91, -48, true);
    ccs.push_back(separation477487072);

    SeparationConstraint *separation477487136 = new SeparationConstraint(vpsc::YDIM, 89, 91, 9, true);
    ccs.push_back(separation477487136);

    SeparationConstraint *separation477487200 = new SeparationConstraint(vpsc::XDIM, 7, 0, 137, false);
    ccs.push_back(separation477487200);

    SeparationConstraint *separation477487264 = new SeparationConstraint(vpsc::XDIM, 0, 5, 118.5, false);
    ccs.push_back(separation477487264);

    SeparationConstraint *separation477487328 = new SeparationConstraint(vpsc::XDIM, 79, 5, 118, false);
    ccs.push_back(separation477487328);

    SeparationConstraint *separation477487392 = new SeparationConstraint(vpsc::XDIM, 9, 15, 146.5, false);
    ccs.push_back(separation477487392);

    SeparationConstraint *separation477487456 = new SeparationConstraint(vpsc::XDIM, 28, 15, 139, false);
    ccs.push_back(separation477487456);

    SeparationConstraint *separation477487520 = new SeparationConstraint(vpsc::XDIM, 0, 22, 142.5, false);
    ccs.push_back(separation477487520);

    SeparationConstraint *separation477487584 = new SeparationConstraint(vpsc::XDIM, 0, 22, 142.5, false);
    ccs.push_back(separation477487584);

    SeparationConstraint *separation477487648 = new SeparationConstraint(vpsc::XDIM, 0, 22, 142.5, false);
    ccs.push_back(separation477487648);

    SeparationConstraint *separation477487712 = new SeparationConstraint(vpsc::XDIM, 19, 22, 137.5, false);
    ccs.push_back(separation477487712);

    SeparationConstraint *separation477487776 = new SeparationConstraint(vpsc::XDIM, 22, 28, 150, false);
    ccs.push_back(separation477487776);

    SeparationConstraint *separation477487840 = new SeparationConstraint(vpsc::XDIM, 15, 31, 151.5, false);
    ccs.push_back(separation477487840);

    SeparationConstraint *separation477487904 = new SeparationConstraint(vpsc::XDIM, 31, 37, 143.5, false);
    ccs.push_back(separation477487904);

    SeparationConstraint *separation477487968 = new SeparationConstraint(vpsc::XDIM, 31, 40, 143.5, false);
    ccs.push_back(separation477487968);

    SeparationConstraint *separation477488032 = new SeparationConstraint(vpsc::XDIM, 31, 43, 207.5, false);
    ccs.push_back(separation477488032);

    SeparationConstraint *separation477488096 = new SeparationConstraint(vpsc::XDIM, 37, 43, 235, false);
    ccs.push_back(separation477488096);

    SeparationConstraint *separation477488160 = new SeparationConstraint(vpsc::XDIM, 40, 43, 235, false);
    ccs.push_back(separation477488160);

    SeparationConstraint *separation477488224 = new SeparationConstraint(vpsc::XDIM, 49, 43, 235, false);
    ccs.push_back(separation477488224);

    SeparationConstraint *separation477488288 = new SeparationConstraint(vpsc::XDIM, 31, 49, 143.5, false);
    ccs.push_back(separation477488288);

    SeparationConstraint *separation477488352 = new SeparationConstraint(vpsc::XDIM, 31, 52, 217.5, false);
    ccs.push_back(separation477488352);

    SeparationConstraint *separation477488416 = new SeparationConstraint(vpsc::XDIM, 43, 52, 181, false);
    ccs.push_back(separation477488416);

    SeparationConstraint *separation477488480 = new SeparationConstraint(vpsc::XDIM, 52, 56, 114.5, false);
    ccs.push_back(separation477488480);

    SeparationConstraint *separation477488544 = new SeparationConstraint(vpsc::XDIM, 61, 59, 143, false);
    ccs.push_back(separation477488544);

    SeparationConstraint *separation477488608 = new SeparationConstraint(vpsc::XDIM, 56, 61, 136.5, false);
    ccs.push_back(separation477488608);

    SeparationConstraint *separation477488672 = new SeparationConstraint(vpsc::XDIM, 15, 64, 126, false);
    ccs.push_back(separation477488672);

    SeparationConstraint *separation477488736 = new SeparationConstraint(vpsc::XDIM, 56, 64, 136.5, false);
    ccs.push_back(separation477488736);

    SeparationConstraint *separation477488800 = new SeparationConstraint(vpsc::XDIM, 89, 68, 116.5, false);
    ccs.push_back(separation477488800);

    SeparationConstraint *separation477488864 = new SeparationConstraint(vpsc::XDIM, 68, 72, 131, false);
    ccs.push_back(separation477488864);

    SeparationConstraint *separation477488928 = new SeparationConstraint(vpsc::XDIM, 76, 72, 124.5, false);
    ccs.push_back(separation477488928);

    SeparationConstraint *separation477488992 = new SeparationConstraint(vpsc::XDIM, 84, 79, 108.5, false);
    ccs.push_back(separation477488992);

    SeparationConstraint *separation477489056 = new SeparationConstraint(vpsc::XDIM, 72, 84, 121, false);
    ccs.push_back(separation477489056);

    SeparationConstraint *separation477489120 = new SeparationConstraint(vpsc::XDIM, 64, 89, 127, false);
    ccs.push_back(separation477489120);

    cola::Box margin = cola::Box(10, 30, 30, 60);   //30
    cola::Box padding = cola::Box(); // cola::Box(10, 30, 30, 60);  //10
    ConstrainedFDLayout alg(rs, es, defaultEdgeLength, eLengths);
    RootCluster *cluster476902600 = new RootCluster();
    cluster476902600->addChildNode(0);
    cluster476902600->addChildNode(1);
    cluster476902600->addChildNode(2);
    cluster476902600->addChildNode(3);
    cluster476902600->addChildNode(4);
    cluster476902600->addChildNode(5);
    cluster476902600->addChildNode(6);
    cluster476902600->addChildNode(7);
    cluster476902600->addChildNode(8);
    cluster476902600->addChildNode(9);
    cluster476902600->addChildNode(10);
    cluster476902600->addChildNode(11);
    cluster476902600->addChildNode(12);
    cluster476902600->addChildNode(13);
    cluster476902600->addChildNode(14);
    cluster476902600->addChildNode(15);
    cluster476902600->addChildNode(16);
    cluster476902600->addChildNode(17);
    cluster476902600->addChildNode(18);
    cluster476902600->addChildNode(19);
    cluster476902600->addChildNode(20);
    cluster476902600->addChildNode(21);
    cluster476902600->addChildNode(22);
    cluster476902600->addChildNode(23);
    cluster476902600->addChildNode(24);
    cluster476902600->addChildNode(25);
    cluster476902600->addChildNode(26);
    cluster476902600->addChildNode(27);
    cluster476902600->addChildNode(28);
    cluster476902600->addChildNode(29);
    cluster476902600->addChildNode(30);
    RectangularCluster *cluster417213744 = new RectangularCluster();
    cluster417213744->setMargin(margin);
    cluster417213744->setPadding(padding);
    cluster417213744->addChildNode(59);
    cluster417213744->addChildNode(60);
    cluster417213744->addChildNode(61);
    cluster417213744->addChildNode(62);
    cluster417213744->addChildNode(63);
    cluster417213744->addChildNode(64);
    cluster417213744->addChildNode(65);
    cluster417213744->addChildNode(66);
    cluster417213744->addChildNode(67);
    cluster417213744->addChildNode(89);
    cluster417213744->addChildNode(90);
    cluster417213744->addChildNode(91);
    RectangularCluster *cluster417215984 = new RectangularCluster();
    cluster417215984->setMargin(margin);
    cluster417215984->setPadding(padding);
    cluster417215984->addChildNode(31);
    cluster417215984->addChildNode(32);
    cluster417215984->addChildNode(33);
    cluster417215984->addChildNode(34);
    cluster417215984->addChildNode(35);
    cluster417215984->addChildNode(36);
    cluster417215984->addChildNode(37);
    cluster417215984->addChildNode(38);
    cluster417215984->addChildNode(39);
    cluster417215984->addChildNode(40);
    cluster417215984->addChildNode(41);
    cluster417215984->addChildNode(42);
    cluster417215984->addChildNode(43);
    cluster417215984->addChildNode(44);
    cluster417215984->addChildNode(45);
    cluster417215984->addChildNode(46);
    cluster417215984->addChildNode(47);
    cluster417215984->addChildNode(48);
    cluster417215984->addChildNode(49);
    cluster417215984->addChildNode(50);
    cluster417215984->addChildNode(51);
    cluster417215984->addChildNode(52);
    cluster417215984->addChildNode(53);
    cluster417215984->addChildNode(54);
    cluster417215984->addChildNode(55);
    cluster417215984->addChildNode(56);
    cluster417215984->addChildNode(57);
    cluster417215984->addChildNode(58);
    cluster417213744->addChildCluster(cluster417215984);
    RectangularCluster *cluster417215760 = new RectangularCluster();
    cluster417215760->setMargin(margin);
    cluster417215760->setPadding(padding);
    cluster417215760->addChildNode(68);
    cluster417215760->addChildNode(69);
    cluster417215760->addChildNode(70);
    cluster417215760->addChildNode(71);
    cluster417215760->addChildNode(72);
    cluster417215760->addChildNode(73);
    cluster417215760->addChildNode(74);
    cluster417215760->addChildNode(75);
    cluster417215760->addChildNode(76);
    cluster417215760->addChildNode(77);
    cluster417215760->addChildNode(78);
    cluster417215760->addChildNode(79);
    cluster417215760->addChildNode(80);
    cluster417215760->addChildNode(81);
    cluster417215760->addChildNode(82);
    cluster417215760->addChildNode(83);
    cluster417215760->addChildNode(84);
    cluster417215760->addChildNode(85);
    cluster417215760->addChildNode(86);
    cluster417215760->addChildNode(87);
    cluster417215760->addChildNode(88);
    cluster417213744->addChildCluster(cluster417215760);
    cluster476902600->addChildCluster(cluster417213744);
    alg.setClusterHierarchy(cluster476902600);
    alg.setConstraints(ccs);

    UnsatisfiableConstraintInfos unsatisfiableX, unsatisfiableY;
    alg.setUnsatisfiableConstraintInfo(&unsatisfiableX, &unsatisfiableY);

    // rect-43 and associated port rects.
    cola::NodeIndexes nonOverlapExemptGroup;
    nonOverlapExemptGroup.push_back(44);
    nonOverlapExemptGroup.push_back(45);
    nonOverlapExemptGroup.push_back(46);
    nonOverlapExemptGroup.push_back(47);
    nonOverlapExemptGroup.push_back(48);
    nonOverlapExemptGroup.push_back(43);
    
    std::vector<cola::NodeIndexes> nonOverlapExemptGroupList;
    nonOverlapExemptGroupList.push_back(nonOverlapExemptGroup);
    alg.setAvoidNodeOverlaps(true, nonOverlapExemptGroupList);

    //alg.makeFeasible();
    alg.run();
    alg.outputInstanceToSVG("rectangularClusters01");

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
