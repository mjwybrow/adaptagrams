#include <vector>
#include "libcola/cola.h"
using namespace cola;
int main(void) {
    CompoundConstraints ccs;
    std::vector<Edge> es;
    double defaultEdgeLength=40;
    std::vector<vpsc::Rectangle*> rs;
    vpsc::Rectangle *rect = nullptr;

    rect = new vpsc::Rectangle(252, 272, 930, 950);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(109, 129, 898, 918);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(252, 272, 951, 971);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(99, 139, 931, 971);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(593, 665, 212.5, 459.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(398, 518, 52, 96);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(341, 361, 899, 919);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(450, 470, 930, 950);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(402, 422, 930, 950);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(394, 414, 861, 881);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(394, 414, 840, 860);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(282, 302, 852, 872);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(173, 193, 852, 872);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(521, 541, 851, 871);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(313, 401, 739, 783);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(446, 466, 757, 777);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(446, 466, 688, 708);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(447, 467, 616, 636);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(421, 441, 471, 491);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(196.5, 359.5, 575, 619);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(366, 386, 437, 457);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(446, 466, 391, 411);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(446, 466, 325, 345);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(393, 413, 325, 345);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(515, 535, 182, 202);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(446, 466, 255, 275);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(446, 466, 160, 180);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(366, 386, 289, 309);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(211, 231, 270, 290);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(211, 231, 289, 309);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(320, 340, 256, 276);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(270, 290, 256, 276);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(144, 164, 251, 271);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(211, 231, 214, 234);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(211, 231, 195, 215);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(279, 299, 173, 193);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(306, 326, 141, 161);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(306, 326, 120, 140);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(34.5, 295.5, 39.5, 74.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(225, 245, 895, 915);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(446, 466, 543, 563);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(211, 231, 233, 253);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(211, 231, 308, 328);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(144, 164, 347, 367);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(144, 164, 373, 393);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(254, 274, 346, 366);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(254, 274, 372, 392);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(554, 594, 173, 213);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 121, 161);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 642, 682);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(134, 174, 206, 246);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(134, 174, 279, 319);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(285, 325, 426, 466);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(285, 325, 279, 319);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 208, 248);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 281, 321);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 714, 754);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 570, 610);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 498, 538);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(580, 620, 841, 881);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(267, 307, 884, 924);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(503, 543, 931, 971);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 841, 881);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(331, 371, 841, 881);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(331, 371, 798, 838);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(99, 139, 841, 881);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(331, 371, 931, 971);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(218, 258, 841, 881);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(165, 205, 884, 924);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 351, 391);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(85, 125, 336, 376);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(192, 232, 336, 376);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(192, 232, 362, 402);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(85, 125, 362, 402);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(20.5, 77.5, 716, 880);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(167, 187, 713, 733);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(99, 139, 714, 754);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(402, 422, 951, 971);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(167, 187, 734, 754);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(450, 470, 951, 971);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(532.5, 669.5, 899.5, 934.5);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(471, 491, 471, 491);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(545, 565, 512, 532);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(518, 538, 531, 551);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(252, 272, 782, 802);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(495, 515, 512, 532);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(351, 391, 534, 574);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(400, 420, 573, 593);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(401, 421, 509, 529);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(436, 476, 426, 466);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(234, 290, 970, 1010);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(29, 85, 888, 928);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(234, 290, 991, 1031);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(261, 317, 889, 929);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(432, 488, 870, 910);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(384, 440, 970, 1010);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(314, 370, 851, 891);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(438, 494, 830, 870);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(202, 258, 842, 882);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(155, 211, 792, 832);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(503, 559, 791, 831);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(490, 546, 747, 787);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(366, 422, 678, 718);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(367, 423, 606, 646);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(341, 397, 461, 501);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(348, 404, 377, 417);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(490, 546, 381, 421);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(490, 546, 315, 355);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(375, 431, 365, 405);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(497, 553, 122, 162);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(490, 546, 245, 285);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(366, 422, 150, 190);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(348, 404, 229, 269);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(193, 249, 210, 250);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(193, 249, 229, 269);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(364, 420, 246, 286);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(190, 246, 246, 286);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(64, 120, 241, 281);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(193, 249, 154, 194);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(193, 249, 135, 175);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(261, 317, 213, 253);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(288, 344, 81, 121);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(288, 344, 160, 200);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(207, 263, 835, 875);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(366, 422, 533, 573);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(193, 249, 173, 213);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(193, 249, 248, 288);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(126, 182, 287, 327);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(126, 182, 413, 453);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(236, 292, 286, 326);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(236, 292, 412, 452);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(149, 205, 653, 693);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(384, 440, 891, 931);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(149, 205, 674, 714);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(432, 488, 991, 1031);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(515, 571, 461, 501);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(527, 583, 452, 492);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(500, 556, 571, 611);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(234, 290, 722, 762);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(539, 595, 502, 542);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(382, 438, 613, 653);
    rs.push_back(rect);

    rect = new vpsc::Rectangle(383, 439, 449, 489);
    rs.push_back(rect);

#if 1
    AlignmentConstraint *alignment2424608 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment2424608->addShape(0, 0);
    alignment2424608->addShape(2, 0);
    alignment2424608->addShape(45, 0);
    alignment2424608->addShape(46, 0);
    alignment2424608->addShape(84, 0);
    alignment2424608->addShape(90, 0);
    alignment2424608->addShape(92, 0);
    alignment2424608->addShape(129, 0);
    alignment2424608->addShape(130, 0);
    alignment2424608->addShape(138, 0);
    ccs.push_back(alignment2424608);
    AlignmentConstraint *alignment2424744 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment2424744->addShape(1, 0);
    alignment2424744->addShape(3, 0);
    alignment2424744->addShape(65, 0);
    alignment2424744->addShape(76, 0);
    ccs.push_back(alignment2424744);
    AlignmentConstraint *alignment1216038856 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216038856->addShape(4, 0);
    ccs.push_back(alignment1216038856);
    AlignmentConstraint *alignment1216038920 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216038920->addShape(5, 0);
    alignment1216038920->addShape(7, 0);
    alignment1216038920->addShape(15, 0);
    alignment1216038920->addShape(16, 0);
    alignment1216038920->addShape(17, 0);
    alignment1216038920->addShape(21, 0);
    alignment1216038920->addShape(22, 0);
    alignment1216038920->addShape(25, 0);
    alignment1216038920->addShape(26, 0);
    alignment1216038920->addShape(40, 0);
    alignment1216038920->addShape(48, 0);
    alignment1216038920->addShape(49, 0);
    alignment1216038920->addShape(54, 0);
    alignment1216038920->addShape(55, 0);
    alignment1216038920->addShape(56, 0);
    alignment1216038920->addShape(57, 0);
    alignment1216038920->addShape(58, 0);
    alignment1216038920->addShape(62, 0);
    alignment1216038920->addShape(69, 0);
    alignment1216038920->addShape(79, 0);
    alignment1216038920->addShape(89, 0);
    alignment1216038920->addShape(94, 0);
    alignment1216038920->addShape(134, 0);
    ccs.push_back(alignment1216038920);
    AlignmentConstraint *alignment1216038984 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216038984->addShape(6, 0);
    alignment1216038984->addShape(63, 0);
    alignment1216038984->addShape(64, 0);
    alignment1216038984->addShape(66, 0);
    ccs.push_back(alignment1216038984);
    AlignmentConstraint *alignment1216039048 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216039048->addShape(8, 0);
    alignment1216039048->addShape(77, 0);
    alignment1216039048->addShape(87, 0);
    alignment1216039048->addShape(88, 0);
    alignment1216039048->addShape(95, 0);
    alignment1216039048->addShape(132, 0);
    alignment1216039048->addShape(140, 0);
    alignment1216039048->addShape(141, 0);
    ccs.push_back(alignment1216039048);
    AlignmentConstraint *alignment1216039112 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216039112->addShape(9, 0);
    alignment1216039112->addShape(10, 0);
    alignment1216039112->addShape(23, 0);
    alignment1216039112->addShape(108, 0);
    ccs.push_back(alignment1216039112);
    AlignmentConstraint *alignment1216039176 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216039176->addShape(11, 0);
    alignment1216039176->addShape(35, 0);
    alignment1216039176->addShape(60, 0);
    alignment1216039176->addShape(93, 0);
    alignment1216039176->addShape(120, 0);
    ccs.push_back(alignment1216039176);
    AlignmentConstraint *alignment1216039240 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216039240->addShape(12, 0);
    alignment1216039240->addShape(68, 0);
    alignment1216039240->addShape(99, 0);
    ccs.push_back(alignment1216039240);
    AlignmentConstraint *alignment1216039304 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216039304->addShape(13, 0);
    alignment1216039304->addShape(83, 0);
    alignment1216039304->addShape(100, 0);
    alignment1216039304->addShape(137, 0);
    ccs.push_back(alignment1216039304);
    AlignmentConstraint *alignment1216040464 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216040464->addShape(14, 0);
    ccs.push_back(alignment1216040464);
    AlignmentConstraint *alignment1216040528 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216040528->addShape(18, 0);
    ccs.push_back(alignment1216040528);
    AlignmentConstraint *alignment1216040592 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216040592->addShape(19, 0);
    alignment1216040592->addShape(31, 0);
    ccs.push_back(alignment1216040592);
    AlignmentConstraint *alignment1216040656 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216040656->addShape(20, 0);
    alignment1216040656->addShape(27, 0);
    alignment1216040656->addShape(86, 0);
    alignment1216040656->addShape(105, 0);
    alignment1216040656->addShape(112, 0);
    ccs.push_back(alignment1216040656);
    AlignmentConstraint *alignment1216040720 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216040720->addShape(24, 0);
    alignment1216040720->addShape(61, 0);
    alignment1216040720->addShape(109, 0);
    ccs.push_back(alignment1216040720);
    AlignmentConstraint *alignment1216040784 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216040784->addShape(28, 0);
    alignment1216040784->addShape(29, 0);
    alignment1216040784->addShape(33, 0);
    alignment1216040784->addShape(34, 0);
    alignment1216040784->addShape(41, 0);
    alignment1216040784->addShape(42, 0);
    alignment1216040784->addShape(113, 0);
    alignment1216040784->addShape(114, 0);
    alignment1216040784->addShape(116, 0);
    alignment1216040784->addShape(118, 0);
    alignment1216040784->addShape(119, 0);
    alignment1216040784->addShape(125, 0);
    alignment1216040784->addShape(126, 0);
    ccs.push_back(alignment1216040784);
    AlignmentConstraint *alignment1216034032 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216034032->addShape(30, 0);
    ccs.push_back(alignment1216034032);
    AlignmentConstraint *alignment1216035736 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216035736->addShape(32, 0);
    alignment1216035736->addShape(43, 0);
    alignment1216035736->addShape(44, 0);
    alignment1216035736->addShape(50, 0);
    alignment1216035736->addShape(51, 0);
    alignment1216035736->addShape(127, 0);
    alignment1216035736->addShape(128, 0);
    ccs.push_back(alignment1216035736);
    AlignmentConstraint *alignment1216035800 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216035800->addShape(36, 0);
    alignment1216035800->addShape(37, 0);
    alignment1216035800->addShape(121, 0);
    alignment1216035800->addShape(122, 0);
    ccs.push_back(alignment1216035800);
    AlignmentConstraint *alignment1216035864 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216035864->addShape(38, 0);
    ccs.push_back(alignment1216035864);
    AlignmentConstraint *alignment1216035928 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216035928->addShape(39, 0);
    alignment1216035928->addShape(67, 0);
    alignment1216035928->addShape(98, 0);
    alignment1216035928->addShape(123, 0);
    ccs.push_back(alignment1216035928);
    AlignmentConstraint *alignment1216035992 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216035992->addShape(47, 0);
    ccs.push_back(alignment1216035992);
    AlignmentConstraint *alignment1216036056 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036056->addShape(52, 0);
    alignment1216036056->addShape(53, 0);
    ccs.push_back(alignment1216036056);
    AlignmentConstraint *alignment1216036120 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036120->addShape(59, 0);
    alignment1216036120->addShape(80, 0);
    ccs.push_back(alignment1216036120);
    AlignmentConstraint *alignment1216036184 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036184->addShape(70, 0);
    alignment1216036184->addShape(73, 0);
    ccs.push_back(alignment1216036184);
    AlignmentConstraint *alignment1216036248 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036248->addShape(71, 0);
    alignment1216036248->addShape(72, 0);
    ccs.push_back(alignment1216036248);
    AlignmentConstraint *alignment1216036312 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036312->addShape(74, 0);
    ccs.push_back(alignment1216036312);
    AlignmentConstraint *alignment1216036376 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036376->addShape(75, 0);
    alignment1216036376->addShape(78, 0);
    alignment1216036376->addShape(131, 0);
    alignment1216036376->addShape(133, 0);
    ccs.push_back(alignment1216036376);
    AlignmentConstraint *alignment1216036440 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036440->addShape(81, 0);
    ccs.push_back(alignment1216036440);
    AlignmentConstraint *alignment1216036504 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036504->addShape(82, 0);
    alignment1216036504->addShape(136, 0);
    ccs.push_back(alignment1216036504);
    AlignmentConstraint *alignment1216036568 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036568->addShape(85, 0);
    ccs.push_back(alignment1216036568);
    AlignmentConstraint *alignment1216036632 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216036632->addShape(91, 0);
    ccs.push_back(alignment1216036632);
    AlignmentConstraint *alignment1216034712 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216034712->addShape(96, 0);
    ccs.push_back(alignment1216034712);
    AlignmentConstraint *alignment1216034776 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216034776->addShape(97, 0);
    ccs.push_back(alignment1216034776);
    AlignmentConstraint *alignment1216034840 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216034840->addShape(101, 0);
    alignment1216034840->addShape(106, 0);
    alignment1216034840->addShape(107, 0);
    alignment1216034840->addShape(110, 0);
    ccs.push_back(alignment1216034840);
    AlignmentConstraint *alignment1216034904 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216034904->addShape(102, 0);
    alignment1216034904->addShape(103, 0);
    alignment1216034904->addShape(111, 0);
    alignment1216034904->addShape(115, 0);
    alignment1216034904->addShape(124, 0);
    ccs.push_back(alignment1216034904);
    AlignmentConstraint *alignment1216034968 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216034968->addShape(104, 0);
    ccs.push_back(alignment1216034968);
    AlignmentConstraint *alignment1216035032 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216035032->addShape(117, 0);
    ccs.push_back(alignment1216035032);
    AlignmentConstraint *alignment1216035096 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216035096->addShape(135, 0);
    ccs.push_back(alignment1216035096);
    AlignmentConstraint *alignment1216035160 = new AlignmentConstraint((vpsc::Dim) 0, 0);
    alignment1216035160->addShape(139, 0);
    ccs.push_back(alignment1216035160);
#endif

#if 1
    AlignmentConstraint *alignment1216035224 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216035224->addShape(0, 0);
    alignment1216035224->addShape(7, 0);
    alignment1216035224->addShape(8, 0);
    ccs.push_back(alignment1216035224);
    AlignmentConstraint *alignment1216035288 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216035288->addShape(1, 0);
    alignment1216035288->addShape(6, 0);
    alignment1216035288->addShape(39, 0);
    alignment1216035288->addShape(60, 0);
    alignment1216035288->addShape(68, 0);
    alignment1216035288->addShape(91, 0);
    alignment1216035288->addShape(93, 0);
    alignment1216035288->addShape(132, 0);
    ccs.push_back(alignment1216035288);
    AlignmentConstraint *alignment1216035352 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216035352->addShape(2, 0);
    alignment1216035352->addShape(77, 0);
    alignment1216035352->addShape(79, 0);
    ccs.push_back(alignment1216035352);
    AlignmentConstraint *alignment1216035416 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216035416->addShape(3, 0);
    alignment1216035416->addShape(61, 0);
    alignment1216035416->addShape(66, 0);
    ccs.push_back(alignment1216035416);
    AlignmentConstraint *alignment1216035480 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216035480->addShape(4, 0);
    alignment1216035480->addShape(22, 0);
    alignment1216035480->addShape(23, 0);
    alignment1216035480->addShape(107, 0);
    ccs.push_back(alignment1216035480);
    AlignmentConstraint *alignment1216035544 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216035544->addShape(5, 0);
    ccs.push_back(alignment1216035544);
    AlignmentConstraint *alignment1216035608 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216035608->addShape(9, 0);
    alignment1216035608->addShape(96, 0);
    ccs.push_back(alignment1216035608);
    AlignmentConstraint *alignment1216042672 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216042672->addShape(10, 0);
    alignment1216042672->addShape(97, 0);
    alignment1216042672->addShape(123, 0);
    ccs.push_back(alignment1216042672);
    AlignmentConstraint *alignment1216042736 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216042736->addShape(11, 0);
    alignment1216042736->addShape(12, 0);
    alignment1216042736->addShape(13, 0);
    alignment1216042736->addShape(59, 0);
    alignment1216042736->addShape(62, 0);
    alignment1216042736->addShape(63, 0);
    alignment1216042736->addShape(65, 0);
    alignment1216042736->addShape(67, 0);
    alignment1216042736->addShape(98, 0);
    ccs.push_back(alignment1216042736);
    AlignmentConstraint *alignment1216042800 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216042800->addShape(14, 0);
    ccs.push_back(alignment1216042800);
    AlignmentConstraint *alignment1216042864 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216042864->addShape(15, 0);
    alignment1216042864->addShape(101, 0);
    ccs.push_back(alignment1216042864);
    AlignmentConstraint *alignment1216042928 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216042928->addShape(16, 0);
    alignment1216042928->addShape(102, 0);
    alignment1216042928->addShape(133, 0);
    ccs.push_back(alignment1216042928);
    AlignmentConstraint *alignment1216042992 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216042992->addShape(17, 0);
    alignment1216042992->addShape(103, 0);
    ccs.push_back(alignment1216042992);
    AlignmentConstraint *alignment1216043056 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043056->addShape(18, 0);
    alignment1216043056->addShape(81, 0);
    alignment1216043056->addShape(104, 0);
    alignment1216043056->addShape(135, 0);
    ccs.push_back(alignment1216043056);
    AlignmentConstraint *alignment1216043120 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043120->addShape(19, 0);
    ccs.push_back(alignment1216043120);
    AlignmentConstraint *alignment1216043184 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043184->addShape(20, 0);
    alignment1216043184->addShape(52, 0);
    alignment1216043184->addShape(89, 0);
    ccs.push_back(alignment1216043184);
    AlignmentConstraint *alignment1216043248 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043248->addShape(21, 0);
    alignment1216043248->addShape(105, 0);
    alignment1216043248->addShape(106, 0);
    ccs.push_back(alignment1216043248);
    AlignmentConstraint *alignment1216043312 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043312->addShape(24, 0);
    alignment1216043312->addShape(47, 0);
    alignment1216043312->addShape(125, 0);
    ccs.push_back(alignment1216043312);
    AlignmentConstraint *alignment1216043376 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043376->addShape(25, 0);
    alignment1216043376->addShape(30, 0);
    alignment1216043376->addShape(31, 0);
    alignment1216043376->addShape(32, 0);
    alignment1216043376->addShape(110, 0);
    alignment1216043376->addShape(115, 0);
    alignment1216043376->addShape(116, 0);
    alignment1216043376->addShape(117, 0);
    alignment1216043376->addShape(126, 0);
    ccs.push_back(alignment1216043376);
    AlignmentConstraint *alignment1216043440 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043440->addShape(26, 0);
    alignment1216043440->addShape(111, 0);
    alignment1216043440->addShape(118, 0);
    ccs.push_back(alignment1216043440);
    AlignmentConstraint *alignment1216043504 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043504->addShape(27, 0);
    alignment1216043504->addShape(29, 0);
    alignment1216043504->addShape(51, 0);
    alignment1216043504->addShape(53, 0);
    alignment1216043504->addShape(55, 0);
    ccs.push_back(alignment1216043504);
    AlignmentConstraint *alignment1216043568 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043568->addShape(28, 0);
    ccs.push_back(alignment1216043568);
    AlignmentConstraint *alignment1216043632 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043632->addShape(33, 0);
    alignment1216043632->addShape(50, 0);
    alignment1216043632->addShape(54, 0);
    ccs.push_back(alignment1216043632);
    AlignmentConstraint *alignment1216043696 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043696->addShape(34, 0);
    ccs.push_back(alignment1216043696);
    AlignmentConstraint *alignment1216043760 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043760->addShape(35, 0);
    alignment1216043760->addShape(122, 0);
    ccs.push_back(alignment1216043760);
    AlignmentConstraint *alignment1216043824 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043824->addShape(36, 0);
    alignment1216043824->addShape(119, 0);
    ccs.push_back(alignment1216043824);
    AlignmentConstraint *alignment1216043888 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043888->addShape(37, 0);
    ccs.push_back(alignment1216043888);
    AlignmentConstraint *alignment1216043952 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216043952->addShape(38, 0);
    ccs.push_back(alignment1216043952);
    AlignmentConstraint *alignment1216044016 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044016->addShape(40, 0);
    alignment1216044016->addShape(86, 0);
    alignment1216044016->addShape(124, 0);
    ccs.push_back(alignment1216044016);
    AlignmentConstraint *alignment1216044080 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044080->addShape(41, 0);
    ccs.push_back(alignment1216044080);
    AlignmentConstraint *alignment1216044144 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044144->addShape(42, 0);
    ccs.push_back(alignment1216044144);
    AlignmentConstraint *alignment1216044208 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044208->addShape(43, 0);
    alignment1216044208->addShape(45, 0);
    alignment1216044208->addShape(70, 0);
    alignment1216044208->addShape(71, 0);
    ccs.push_back(alignment1216044208);
    AlignmentConstraint *alignment1216044272 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044272->addShape(44, 0);
    alignment1216044272->addShape(46, 0);
    alignment1216044272->addShape(72, 0);
    alignment1216044272->addShape(73, 0);
    alignment1216044272->addShape(108, 0);
    ccs.push_back(alignment1216044272);
    AlignmentConstraint *alignment1216044336 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044336->addShape(48, 0);
    alignment1216044336->addShape(109, 0);
    ccs.push_back(alignment1216044336);
    AlignmentConstraint *alignment1216044400 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044400->addShape(49, 0);
    ccs.push_back(alignment1216044400);
    AlignmentConstraint *alignment1216044464 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044464->addShape(56, 0);
    alignment1216044464->addShape(76, 0);
    ccs.push_back(alignment1216044464);
    AlignmentConstraint *alignment1216044528 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044528->addShape(57, 0);
    alignment1216044528->addShape(137, 0);
    ccs.push_back(alignment1216044528);
    AlignmentConstraint *alignment1216044592 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216044592->addShape(58, 0);
    alignment1216044592->addShape(82, 0);
    alignment1216044592->addShape(85, 0);
    alignment1216044592->addShape(88, 0);
    alignment1216044592->addShape(139, 0);
    ccs.push_back(alignment1216044592);
    AlignmentConstraint *alignment1216021552 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216021552->addShape(64, 0);
    ccs.push_back(alignment1216021552);
    AlignmentConstraint *alignment1216021616 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216021616->addShape(69, 0);
    ccs.push_back(alignment1216021616);
    AlignmentConstraint *alignment1216021680 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216021680->addShape(74, 0);
    ccs.push_back(alignment1216021680);
    AlignmentConstraint *alignment1216021744 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216021744->addShape(75, 0);
    ccs.push_back(alignment1216021744);
    AlignmentConstraint *alignment1216021808 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216021808->addShape(78, 0);
    alignment1216021808->addShape(138, 0);
    ccs.push_back(alignment1216021808);
    AlignmentConstraint *alignment1216021872 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216021872->addShape(80, 0);
    ccs.push_back(alignment1216021872);
    AlignmentConstraint *alignment1216021936 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216021936->addShape(83, 0);
    ccs.push_back(alignment1216021936);
    AlignmentConstraint *alignment1216022000 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022000->addShape(84, 0);
    ccs.push_back(alignment1216022000);
    AlignmentConstraint *alignment1216022064 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022064->addShape(87, 0);
    ccs.push_back(alignment1216022064);
    AlignmentConstraint *alignment1216022128 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022128->addShape(90, 0);
    alignment1216022128->addShape(95, 0);
    ccs.push_back(alignment1216022128);
    AlignmentConstraint *alignment1216022192 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022192->addShape(92, 0);
    alignment1216022192->addShape(134, 0);
    ccs.push_back(alignment1216022192);
    AlignmentConstraint *alignment1216022256 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022256->addShape(94, 0);
    ccs.push_back(alignment1216022256);
    AlignmentConstraint *alignment1216022320 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022320->addShape(99, 0);
    alignment1216022320->addShape(100, 0);
    ccs.push_back(alignment1216022320);
    AlignmentConstraint *alignment1216022384 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022384->addShape(112, 0);
    alignment1216022384->addShape(114, 0);
    ccs.push_back(alignment1216022384);
    AlignmentConstraint *alignment1216022448 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022448->addShape(113, 0);
    alignment1216022448->addShape(120, 0);
    ccs.push_back(alignment1216022448);
    AlignmentConstraint *alignment1216022512 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022512->addShape(121, 0);
    ccs.push_back(alignment1216022512);
    AlignmentConstraint *alignment1216022576 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022576->addShape(127, 0);
    alignment1216022576->addShape(129, 0);
    ccs.push_back(alignment1216022576);
    AlignmentConstraint *alignment1216022640 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022640->addShape(128, 0);
    alignment1216022640->addShape(130, 0);
    ccs.push_back(alignment1216022640);
    AlignmentConstraint *alignment1216022704 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022704->addShape(131, 0);
    ccs.push_back(alignment1216022704);
    AlignmentConstraint *alignment1216022768 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022768->addShape(136, 0);
    alignment1216022768->addShape(141, 0);
    ccs.push_back(alignment1216022768);
    AlignmentConstraint *alignment1216022832 = new AlignmentConstraint((vpsc::Dim) 1, 0);
    alignment1216022832->addShape(140, 0);
    ccs.push_back(alignment1216022832);
#endif

    ConstrainedFDLayout alg(rs, es, defaultEdgeLength);
    alg.setAvoidNodeOverlaps(true);
    alg.setConstraints(ccs);
    alg.makeFeasible();
    alg.outputInstanceToSVG();
    //alg.run();
    return 0;
};
