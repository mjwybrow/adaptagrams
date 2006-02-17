#include <iostream>
#include <cassert>
#include <math.h>
#include <time.h>
#include <generate-constraints.h>
#include <variable.h>
#include <constraint.h>
#include <solve_VPSC.h>
#define EXTRA_GAP 0.0001
using namespace std;
void removeoverlaps(Rectangle **rs, int n) {
	double xBorder=0, yBorder=0;
	assert(0 <= n);
	try {
		// The extra gap avoids numerical imprecision problems
		Rectangle::setXBorder(xBorder+EXTRA_GAP);
		Rectangle::setYBorder(yBorder+EXTRA_GAP);
		double *ws=new double[n];
		for(int i=0;i<n;i++) {
			ws[i]=1;
		}
		Variable **vs;
		Constraint **cs;
		double *oldX = new double[n];
		int m=generateXConstraints(rs,ws,n,vs,cs,true);
		for(int i=0;i<n;i++) {
			oldX[i]=vs[i]->desiredPosition;
		}
		VPSC vpsc_x(vs,n,cs,m);
		vpsc_x.satisfy();
		for(int i=0;i<n;i++) {
			rs[i]->moveCentreX(vs[i]->position());
			delete vs[i];
		}
		delete [] vs;
		for(int i = 0; i < m; ++i) {
			delete cs[i];
		}
		delete [] cs;
		// Removing the extra gap here ensures things that were moved to be adjacent to
		// one another above are not considered overlapping
		Rectangle::setXBorder(Rectangle::xBorder-EXTRA_GAP);
		m=generateYConstraints(rs,ws,n,vs,cs);
		VPSC vpsc_y(vs,n,cs,m);
		vpsc_y.satisfy();
		for(int i=0;i<n;i++) {
			rs[i]->moveCentreY(vs[i]->position());
			rs[i]->moveCentreX(oldX[i]);
			delete vs[i];
		}
		delete [] vs;
		delete [] oldX;
		for(int i = 0; i < m; ++i) {
			delete cs[i];
		}
		delete [] cs;
		Rectangle::setYBorder(Rectangle::yBorder-EXTRA_GAP);
		m=generateXConstraints(rs,ws,n,vs,cs,false);
		VPSC vpsc_x2(vs,n,cs,m);
		vpsc_x2.satisfy();
		for(int i=0;i<n;i++) {
			rs[i]->moveCentreX(vs[i]->position());
			delete vs[i];
		}
		delete [] vs;
		for(int i = 0; i < m; ++i) {
			delete cs[i];
		}
		delete [] cs;
		delete [] ws;
	} catch (char *str) {
		std::cerr<<str<<std::endl;
		for(int i=0;i<n;i++) {
			std::cerr << *rs[i]<<std::endl;
		}
	}
}
unsigned countOverlaps(Rectangle **rs, unsigned n) {
	unsigned overlaps=0;
	for(unsigned i=0;i<n-1;i++) {
		for(unsigned j=i+1;j<n;j++) {
			if(rs[i]->overlapX(rs[j])>EXTRA_GAP&&rs[i]->overlapY(rs[j])>EXTRA_GAP) {
				/*
				cout << "Overlap found between: " << endl;
				cout << "    " << *rs[i] << endl;
				cout << "    " << *rs[j] << endl;
				*/
				overlaps++;
			}
		}
	}
	return overlaps;
}
inline int getRand(int range) {
	return (int)(range*rand()/(RAND_MAX+1.0));
}
inline double getRand(double range) {
	return range*rand()/RAND_MAX;
}
Rectangle **generateRandomRects(unsigned n) {
	Rectangle **rs=new Rectangle*[n];
	double const rect_size = 5;
	double const fld_size = sqrt(rect_size * n / 2.0);
	double coords[4];
	for (unsigned i = 0; i < n; ++i) {
		for (unsigned d = 0; d < 2; ++d) {
			//unsigned const end = 1 + (rand() % (fld_size - 1));
			//unsigned const start = rand() % end;
			double const start = getRand(fld_size);
			double const end = start + getRand(rect_size);
			coords[2 * d] = start;
			coords[2 * d + 1] = end;
		} 
		rs[i]=new Rectangle(coords[0],coords[1],coords[2],coords[3]);
	}
	//double k = (double)countOverlaps(rs,n)/n;
	//cout << "    k="<< k << endl;
	return rs;
}
Rectangle **generateRects(double coords[][4], unsigned n) {
	Rectangle **rs=new Rectangle*[n];
	for (unsigned i = 0; i < n; ++i) {
		rs[i]=new Rectangle(coords[i][0],coords[i][1],coords[i][2],coords[i][3]);
	}
	return rs;
}
void test(Rectangle **rs, unsigned n, double &cost, double &duration) {
	Rectangle **ors=new Rectangle*[n];
	double const rect_size = 5;
	double const fld_size = sqrt(rect_size * n / 2.0);
	for (unsigned i = 0; i < n; ++i) {
		ors[i]=new Rectangle(rs[i]->getMinX(),rs[i]->getMaxX(),rs[i]->getMinY(),rs[i]->getMaxY());
	}

	clock_t starttime = clock();
	removeoverlaps(rs,n);
	duration = (double)(clock() - starttime)/CLOCKS_PER_SEC;
	/*
	if(countOverlaps(rs,n)!=0){
		std::cerr<<"************FAIL**************"<<std::endl;
		for(unsigned i=0;i<n;i++) {
			std::cerr << *ors[i]<<std::endl;
		}
		exit(1);
	}
	*/
	cost = 0;
	for(unsigned i=0;i<n;i++) {
		double dx=rs[i]->getCentreX()-ors[i]->getCentreX();
		double dy=rs[i]->getCentreY()-ors[i]->getCentreY();
		cost+=sqrt(dx*dx+dy*dy);
		delete rs[i];
		delete ors[i];
	}
	delete [] rs;
	delete [] ors;
}
double test1[][4]={ { 0, 50, 0, 30 }, { 10, 20, 10, 29 },
{ 30, 70, 39, 70 }, { 0, 90, 40, 50 }, { 30, 70, 1, 29 } };
unsigned n1=5;
double test2[][4]={ { 7, 22, 39, 54 }, { 7, 33, 0, 59 },
{ 3, 26, 16, 56 }, { 7, 17, 18, 20 }, { 1, 59, 11, 26 },
{ 19, 20, 13, 49 }, { 1, 10, 0, 4 }, { 47, 52, 1, 3 } };
unsigned n2=8;
double test3[][4]={ { 8, 32, 29, 36 }, { 19, 24, 2, 27 },
{ 4, 5, 27, 55 }, { 6, 7, 13, 26 }, { 3, 39, 46, 62 },
{ 6, 23, 2, 19 }, { 18, 39, 5, 23 }, { 35, 63, 42, 78 },
{ 16, 18, 14, 72 }, { 12, 32, 10, 58 } };
unsigned n3=10;
double test4[][4]={ { 315.755, 355.288, 353.595, 449.627 },
{ 395.048, 395.635, 253.943, 362.228 },
{ 254.439, 393.289, 278.708, 286.346 },
{ 209.852, 370.831, 326.496, 507.255 },
{ 271.947, 415.74, 362.228, 450.318 },
{ 293.408, 405.197, 220.61, 244.119 },
{ 276.482, 386.472, 286.346, 435.767 },
{ 268.211, 436.23, 192.807, 220.61 },
{ 378.008, 502.118, 358.437, 475.587 },
{ 340.68, 472.597, 249.492, 335.448 } };
unsigned n4=10;
double test5[][4]={ { 7, 22, 39, 54 }, { 7, 33, 0, 59 },
{ 3, 26, 16, 56 }, { 7, 17, 18, 20 }, { 1, 59, 11, 26 },
{ 19, 20, 13, 49 }, { 1, 10, 0, 4 }, { 47, 52, 1, 3 } };
unsigned n5=8;
double test6[][4]={ { 40, 69, 63, 69 }, { 1, 5, 27, 64 },
{ 34, 66, 20, 22 }, { 1, 24, 10, 25 }, { 1, 19, 9, 61 },
{ 0, 56, 8, 70 }, { 33, 35, 13, 28 }, { 11, 31, 33, 35 },
{ 12, 22, 3, 23 } };
unsigned n6=9;
double test7[][4]={	{ 341.594, 388.459, 373.491, 518.168 },
{ 271.214, 324.782, 311.332, 409.166 },
{ 293.848, 475.064, 305.194, 391.162 },
{ 255.317, 447.738, 342.671, 489.923 },
{ 228.375, 261.057, 206.422, 327.794 },
{ 383.552, 462.834, 363.132, 412.843 },
{ 288.859, 481.054, 351.895, 497.728 },
{ 201.307, 368.511, 387.02, 394.95 },
{ 257.961, 259.673, 386.503, 518.403 },
{ 200.178, 275.606, 364.968, 466.787 } };	
unsigned n7=10;
double test8[][4]={{12.807,15.7566,14.9478,16.7924},
{7.76228,11.6532,4.75249,4.75249},
{7.84596,10.1387,15.465,16.7709},
{1.80748,3.0357,5.9983,6.16279},
{6.46447,7.47249,12.8694,13.4378},
{14.0026,17.3342,5.10141,9.81088},
{6.84223,6.85932,6.40395,9.21135},
{7.63462,10.3552,6.78124,8.59953},
{0.26429,2.80847,14.5724,17.7455},
{14.7686,15.7148,3.46036,5.66776},
{10.635,11.4893,12.5044,16.941},
{6.32027,10.7117,14.2953,15.6276},
{11.9942,13.1118,10.6893,11.4477},
{11.9384,15.1357,2.20982,6.92982},
{2.89395,4.29002,11.7058,16.2896},
{9.44116,12.7547,9.75556,11.1811},
{5.2475,8.00607,15.3652,17.026},
{2.09541,3.76981,2.5526,3.16739},
{3.14595,6.66351,10.3007,14.4881},
{4.88109,9.38044,9.02416,10.2954},
{7.55378,9.14715,13.9686,16.0468},
{1.70299,5.42198,14.1913,14.2191},
{14.4877,14.4897,6.14013,8.50074},
{12.9909,14.5163,10.322,12.4457},
{11.616,12.0848,3.7601,5.45419},
{10.3087,13.358,3.04666,3.53389},
{2.28263,2.44881,13.806,15.8206},
{3.31805,5.47662,3.91187,6.85355},
{0.484138,2.06164,3.57335,7.87753},
{4.73784,5.12359,9.383,11.9217},
{12.2921,12.7769,12.329,12.8139},
{12.7351,16.7141,12.7658,13.78},
{3.71614,5.79872,1.53137,4.97126},
{10.7423,14.8183,2.57104,2.94168},
{9.93995,10.1557,1.3432,1.499},
{0.198099,0.204966,9.29459,11.8795},
{14.1043,18.8473,11.2028,14.1971},
{4.23857,4.95743,14.7047,17.1439},
{13.936,15.9612,10.7744,14.8598},
{11.355,15.6824,2.49113,5.96963},
{12.8528,16.3913,3.82582,6.67259},
{13.9445,14.7354,10.8576,12.9503},
{13.0041,15.6166,2.07035,3.70034},
{2.31809,5.03195,1.13659,5.8604},
{10.2454,14.5396,10.9442,15.4321},
{7.12259,11.3929,7.20864,10.4059},
{6.54862,9.88399,1.82828,5.89899},
{4.27072,7.52613,7.99016,11.1703},
{8.84828,9.15453,10.1489,11.7934},
{7.71027,8.97206,3.26462,4.02636},
{14.3383,15.1727,3.02586,3.26268},
{9.21233,11.1919,13.9814,16.1433},
{13.1946,16.1613,12.2268,13.1319},
{13.0462,15.6689,7.03513,8.59752},
{8.72724,11.3859,5.69193,7.8238},
{10.6241,14.1033,7.88946,9.95327},
{8.48376,10.8714,8.86955,11.2137},
{3.55633,4.14351,13.7308,18.4988},
{1.79802,4.68843,11.0964,11.6543},
{13.1535,14.6895,1.52144,2.50643},
{1.24533,4.27261,13.345,13.4925},
{14.4031,18.7944,15.4447,18.0096},
{8.56933,11.3392,0.446787,1.18762},
{11.311,16.0574,8.41,8.76508},
{13.5332,17.8479,8.2067,11.4446},
{5.73826,6.63581,10.8364,11.6186},
{10.1995,11.5202,6.54248,9.49804},
{14.6456,16.7915,7.01621,7.57531},
{15.1444,16.4184,1.45761,2.35577},
{11.8169,12.0382,14.8149,19.3316},
{10.148,11.1227,15.0087,18.1595},
{10.9498,14.7849,12.3852,13.9467},
{10.7631,10.9141,5.24419,8.64319},
{11.2486,12.6029,6.25029,10.0076},
{12.8731,16.0703,4.96619,8.54006},
{6.92828,8.53172,8.36509,11.8971},
{3.1677,7.20423,14.5582,17.1794},
{14.1426,16.9327,15.1198,19.1306},
{2.52943,6.64027,5.18935,8.58545},
{6.89613,11.4515,3.7672,6.23813},
{2.91428,6.40636,1.4231,4.10552},
{9.81892,12.9687,8.25114,10.1775},
{14.6192,14.7412,10.8349,15.1725},
{6.96657,11.7009,11.0638,11.8966},
{7.03182,9.20687,2.04293,2.60462},
{7.47955,9.28533,7.45213,8.37318},
{6.24651,9.79246,8.61803,9.91034},
{4.73642,7.48461,9.59481,12.748},
{8.70644,11.1702,2.54172,4.26587},
{14.9028,19.4109,6.238,8.04256},
{5.22907,8.45899,1.98714,4.94042},
{8.96884,12.3636,1.72663,3.39844},
{6.96563,8.04735,10.6198,11.3663},
{1.65099,5.65883,10.5002,13.9039},
{11.3337,11.5138,5.31369,7.75029},
{2.79561,4.08151,7.04269,9.00167}};

unsigned n8=96;
double test9[][4]={{2.67865,4.81342,8.67025,11.5025},
{5.77912,6.94355,9.80043,12.2904},
{11.9459,13.8675,3.66967,7.38408},
{6.72663,8.30474,3.37566,7.24571},
{9.2081,10.9827,10.9557,11.8868},
{0.161903,3.31202,3.35371,3.84933},
{4.46508,9.18934,9.43037,10.9125},
{3.84059,5.2565,8.66868,8.70713},
{4.67598,5.15207,5.2252,8.92099},
{1.19252,1.31215,7.97481,8.27191},
{4.57641,5.71063,0.440628,4.17365},
{6.80268,11.5523,3.78375,4.79819},
{11.2172,13.7652,8.44876,11.1649},
{11.4673,13.1644,6.09156,7.41026},
{12.2995,13.585,0.155239,4.63887},
{12.3513,15.3947,8.3437,11.1337},
{3.92566,6.15611,12.432,16.3993},
{12.6351,14.6039,9.96391,13.1152},
{0.682894,2.48867,7.66904,8.86704},
{3.50189,7.28939,8.37741,8.96276},
{2.37719,4.19762,11.1129,12.0465},
{5.53568,8.1444,11.9056,15.5818},
{7.32876,11.2713,12.1655,15.0785},
{2.534,6.15563,9.30845,11.2833},
{8.00852,12.8051,2.16786,4.90262},
{11.8095,12.814,11.5818,13.7731},
{7.54398,9.23043,7.36953,10.7315},
{8.36918,9.83071,11.8668,11.8776},
{5.72267,9.66642,1.46379,3.71362},
{11.2725,14.6538,4.76772,7.30304},
{5.8638,10.7623,5.99865,10.0994},
{11.0224,12.6455,0.852638,2.20201},
{4.03151,8.51056,6.12449,10.3672},
{7.75136,10.6782,10.648,11.8606},
{11.9663,14.3993,2.83154,5.46437},
{6.05275,8.00044,10.9675,15.3426},
{10.9702,13.146,8.98465,12.4718},
{4.99665,7.91376,9.19241,9.71153},
{3.7422,4.00542,2.00556,2.53918},
{9.01679,10.9439,12.8107,15.1367},
{10.0117,11.5009,0.194049,3.08568},
{8.29117,13.1323,6.99045,8.65707},
{5.97395,10.5816,5.77559,7.35126},
{5.68856,6.51699,4.40392,5.82242},
{0.209729,3.89072,7.36013,8.98447},
{0.360264,1.88558,0.319886,3.22738},
{2.02163,2.07428,3.2753,8.12958},
{5.05075,5.86987,12.7958,16.7689},
{10.6484,15.5931,7.23546,10.4741},
{7.20998,7.46939,2.44384,5.10459},
{12.5022,15.1819,10.6593,13.5708},
{2.3619,2.45407,5.75168,7.28966},
{12.131,16.661,4.39608,8.19289},
{9.81023,10.731,6.67919,9.31523},
{6.97712,10.166,8.4813,13.3432},
{6.45378,7.44457,8.96504,11.9806},
{8.70396,11.5729,7.15588,12.1524},
{9.93058,14.0306,0.849502,1.80855},
{8.32174,8.34616,7.52595,12.0431},
{2.79979,5.93358,4.90296,7.6893},
{0.72484,5.42439,3.48229,5.77743},
{6.05275,10.2779,7.4252,7.72184},
{4.41176,6.91184,11.8809,15.3923},
{6.50435,8.04432,11.4826,15.3172},
{12.16,14.3518,1.13763,4.17255},
{7.82506,10.5124,8.05713,9.03876}};
unsigned n9=66;
double test10[][4]={{15.9875,18.9768,15.3193,17.9811},
{16.5669,19.7216,1.05824,1.58667},
{8.80629,9.88527,9.71101,13.2142},
{19.568,20.1767,3.28922,8.03561},
{13.6363,15.6827,12.9329,16.6542},
{2.32913,3.58085,8.28597,12.6503},
{6.86597,11.2847,5.9172,10.236},
{8.57413,11.0048,18.0203,18.4095},
{7.68828,8.89193,9.52038,12.3945},
{13.5784,15.499,0.824193,4.7533},
{5.70392,8.26946,10.1955,12.8184},
{17.9857,22.6483,14.2013,16.0377},
{14.7965,16.4222,11.9332,16.5984},
{20.5111,23.1383,0.355473,0.834461},
{5.88763,9.2761,8.31995,11.6173},
{13.8307,16.6334,17.2376,19.8715},
{7.20005,8.99179,19.0137,23.483},
{3.67112,6.97933,18.6142,21.1208},
{0.936183,5.81945,5.11063,7.47703},
{9.85383,12.4861,7.80027,11.677},
{8.07584,11.1964,13.8571,17.9736},
{8.07017,10.0661,8.42816,9.79417},
{10.8259,11.6232,16.7028,19.3248},
{18.55,19.3911,17.174,22.0082},
{1.34073,1.93538,17.9341,20.4509},
{11.7558,12.1994,0.295703,2.19289},
{12.4063,13.662,0.965124,5.21391},
{2.2033,2.85518,16.4455,20.0191},
{6.3853,9.77239,0.892142,2.17773},
{0.546736,5.29679,12.1949,15.2469},
{14.1365,17.5213,14.7027,15.1512},
{9.46879,11.9145,11.8539,14.4408},
{8.49611,8.98075,17.1388,17.5066},
{1.53577,2.52014,0.615943,4.13396},
{7.12078,9.05901,20.3739,23.44},
{2.7104,7.09713,16.4442,20.8587},
{12.9707,14.2845,12.5642,16.2281},
{13.5086,14.3207,17.7611,20.1056},
{19.2264,24.1481,2.063,5.27645},
{15.2016,18.2491,1.10228,3.37286},
{16.7733,18.1385,2.35807,7.0123},
{19.8272,21.9464,18.27,22.8376},
{4.27133,8.48869,19.5925,21.488},
{19.6951,23.7284,10.5207,11.1889},
{0.799027,3.61863,1.42504,3.44902},
{1.31871,5.95584,16.4147,18.2479},
{19.8398,21.0495,15.2495,15.8842},
{3.15773,5.76599,11.1562,12.2007},
{4.61422,4.78559,16.6607,16.9497},
{11.0064,11.5745,16.521,19.5463},
{13.3865,15.7969,12.215,14.3112},
{0.272424,4.59201,7.19502,9.58004},
{15.6377,16.4926,18.5978,19.0178},
{1.44957,1.65985,5.24023,10.1284},
{0.0559948,0.138395,3.75354,4.46554},
{3.24015,3.86196,12.205,14.0543},
{2.64686,3.51587,16.1429,16.6103},
{2.56003,5.14236,5.6624,8.31491},
{13.7143,16.5806,7.32777,10.6354},
{15.8994,20.4036,0.171759,0.567584},
{11.1247,11.5263,7.61341,8.80042},
{10.7869,11.8353,7.73861,8.39933},
{16.7563,18.1604,15.6377,20.3633},
{4.7627,5.87556,0.850618,4.25755},
{11.2178,15.6734,14.2378,15.9026},
{2.00323,5.94744,1.78428,4.77571},
{16.0705,20.0359,9.50528,12.9563},
{14.9808,18.3586,9.56819,10.8817},
{13.7005,17.173,10.5755,15.5172},
{6.24185,6.44174,11.9766,16.0609},
{14.0767,16.1476,11.9904,14.2274},
{18.3009,20.9378,0.109473,0.330122},
{0.395109,4.35458,19.8694,23.5078},
{17.4949,22.4626,6.16132,10.5268},
{0.992178,2.11099,18.813,22.5419},
{8.41369,12.6754,20.0682,22.6},
{19.5239,23.6359,3.22945,3.81068},
{6.60361,6.84761,11.8174,12.6899},
{8.95351,9.96275,11.2373,12.3269},
{13.8414,17.1229,18.182,20.7568},
{13.0468,17.4872,16.8349,21.2615},
{16.6053,17.1026,5.24778,8.1272},
{1.20043,2.96974,10.1904,13.8153},
{7.73924,7.74809,1.53703,3.76274},
{15.0915,18.4866,18.6576,18.9426},
{10.69,13.9811,6.92511,9.44625},
{8.66032,12.8382,6.67974,9.08735},
{3.24707,3.91176,7.21641,8.75134},
{20.5281,23.518,5.04016,5.38868},
{8.78238,13.6922,11.9149,12.334},
{19.1358,19.5264,15.9157,16.8192},
{4.52551,7.52884,12.1049,16.4566},
{18.467,22.8957,2.97024,7.48362},
{17.9618,18.9449,2.7601,4.06645},
{10.8057,15.3718,7.7254,8.32173},
{19.6416,21.5781,7.62473,11.5731},
{5.14712,8.87694,0.495145,1.50088},
{4.11719,6.22953,3.59814,8.25023},
{6.34314,9.83141,1.90886,3.52039},
{11.6041,11.7642,12.6177,16.2647},
{17.4351,19.786,0.846214,2.85357},
{7.90974,12.4031,10.5818,10.868},
{3.33578,5.53021,19.271,22.6137},
{14.3296,18.1074,3.76864,6.21134},
{1.39798,4.21194,14.7015,18.9814},
{13.3463,13.89,4.76333,8.78979},
{7.78517,12.3834,12.0445,12.1204},
{1.21553,4.14149,10.9523,11.5827},
{10.7328,15.0635,11.2248,16.0148},
{1.89816,6.42421,6.21479,9.51491},
{5.76369,9.87896,0.471237,2.08597},
{1.10543,3.50571,6.4243,6.85217},
{13.4916,16.9187,13.4167,14.751},
{6.61683,9.79075,4.44435,7.2165},
{17.5276,21.8159,3.22316,7.12862},
{8.09408,10.3727,0.167984,1.77021},
{8.04815,12.6168,7.91666,11.0816},
{1.32248,2.63905,12.5164,17.4845},
{7.324,11.7358,9.05354,13.7152},
{15.0274,18.5751,11.55,13.6444},
{15.3627,16.1478,19.7391,21.3528},
{16.5701,19.0741,18.0876,18.9877},
{19.9719,20.1289,10.2999,14.2899},
{5.77124,9.49847,20.3551,23.9988},
{9.4065,13.7029,1.45775,2.17524},
{8.22117,12.8533,16.9079,17.5996},
{8.20293,12.4012,15.7691,15.783},
{9.33037,11.8253,0.286895,2.70091},
{12.5114,14.6471,11.0241,13.8678},
{14.6423,15.2091,5.08987,9.29257},
{10.4729,12.308,1.7717,6.27227},
{9.65187,11.9638,14.3196,18.937},
{5.1704,6.94718,18.4884,22.8028},
{4.66015,8.43071,12.6095,14.2635},
{19.5384,21.1589,7.47562,10.9592},
{20.3595,24.407,0.804689,2.94465},
{0.775119,2.53222,1.35017,2.38185},
{14.1459,18.0717,13.111,17.3643},
{15.9535,16.9371,3.47482,6.36462},
{15.1727,17.7769,13.1324,16.2993},
{17.2678,19.5719,19.2912,23.3942},
{14.5832,17.1443,19.2559,24.0751},
{1.57289,3.42857,7.75497,11.9183},
{2.63994,5.18274,5.84108,8.64802},
{7.56056,8.51991,0.284378,0.750091},
{8.4225,12.0985,13.1443,16.5509},
{12.4397,13.7349,16.1177,21.0089},
{18.411,22.1179,12.4302,16.1478},
{16.7343,19.9381,0.87138,4.38391},
{0.729191,5.0941,0.4316,3.36382},
{7.96699,10.079,15.0613,19.489},
{14.4215,16.2705,13.932,15.6623},
{9.37756,12.4608,12.8857,16.6014},
{12.0729,12.5163,2.59841,7.39714},
{7.52658,11.0203,15.2853,19.805},
{16.0919,20.3462,13.6464,15.1218},
{6.17075,10.0034,17.5729,21.8258},
{8.94533,11.0185,7.98461,12.5047},
{0.249775,1.8787,2.72361,7.6554},
{11.5167,11.8826,8.7937,10.8008},
{14.3038,17.8052,19.0873,20.7193},
{19.7272,24.1131,5.29434,7.64106},
{19.7624,20.0966,13.2827,17.354},
{5.58186,7.99298,3.3987,5.8269},
{19.7479,20.3262,16.1492,17.465},
{3.25902,6.3684,17.0948,20.9851},
{12.7549,14.4322,1.40931,6.02218},
{12.232,13.2567,0.559319,3.07481},
{0.138414,4.75571,14.9273,18.2745},
{6.04241,6.23422,9.62104,10.3701}};
unsigned n10=170;
double test11[][4]={{2.20847,5.99613,25.7684,29.5065},
{4.38169,7.14163,-7.79499,-7.42678},
{17.1816,19.4476,-5.03151,-3.38153},
{10.6383,11.0169,-0.356332,3.85003},
{13.3309,15.7634,21.4146,24.0122},
{17.1354,20.5384,27.9053,29.8839},
{4.82128,7.88931,4.90792,5.71239},
{13.1802,14.7891,24.0123,26.5035},
{9.1468,9.95493,-3.73661,-1.29955},
{14.4021,17.8989,19.2669,23.5805},
{3.53936,4.95206,17.2632,19.2913},
{11.2732,15.3951,10.4298,11.6976},
{9.82902,13.9892,17.9523,19.9811},
{10.0265,11.0793,16.4791,17.5196},
{1.27427,5.64116,7.85316,8.33764},
{3.70992,7.16293,5.06311,6.98578},
{9.95501,13.9876,-3.13412,-1.29965},
{17.6086,21.8963,15.3022,17.9743},
{2.05607,2.66477,-0.552748,0.286511},
{12.6971,13.3533,-4.23798,-3.73671},
{7.7961,11.69,7.35371,11.9837},
{7.68167,9.93012,1.7933,3.98514},
{1.2176,2.27018,0.286611,3.17168},
{4.98578,7.91342,12.8464,16.1781},
{16.5918,21.3527,4.59687,5.72758},
{9.12809,9.56481,1.38197,2.061},
{14.6904,19.1649,-3.38143,-1.21492},
{11.3046,15.5357,14.6315,17.2819},
{3.69836,4.52282,5.06311,5.67974},
{12.2713,13.7058,11.6977,14.6314},
{0.130993,3.26738,-4.32728,-3.7221},
{11.4317,12.3544,19.9812,22.2083},
{10.0298,10.1041,11.1483,12.7055},
{16.2023,16.7313,1.49517,4.59677},
{12.4149,15.8265,4.33334,6.45789},
{17.575,19.9519,12.3124,15.3021},
{17.0991,22.0233,3.78959,6.02599},
{3.99216,5.32933,0.583351,0.882127},
{16.5929,18.003,14.3823,17.6574},
{13.5273,15.1372,16.4791,20.1285},
{12.3687,15.2044,6.45799,7.83514},
{8.04258,9.47375,17.3137,21.5515},
{7.329,10.9322,7.65703,11.5161},
{1.98895,2.89459,1.84656,3.17168},
{0.520521,2.46608,6.71895,10.4866},
{13.184,16.1474,6.45799,7.30869},
{2.13915,4.58689,17.2632,19.8192},
{15.8375,18.2786,15.9558,18.1255},
{14.7223,16.4109,16.4791,17.3636},
{4.00206,7.29287,-4.32728,-1.41062},
{16.6028,18.6157,14.6315,17.1786},
{3.19495,7.10528,-1.41052,0.583251},
{1.69626,5.11997,0.326338,1.84646},
{12.1579,12.759,-0.356332,0.328198},
{3.51735,7.12098,5.71249,7.85306},
{0.953512,5.10372,3.17178,5.06301},
{13.2737,14.3319,18.7924,21.1975},
{1.46188,5.99876,10.3471,12.8463},
{8.39195,9.77825,1.66426,2.061},
{1.81344,5.57851,-3.53802,0.326238},
{1.31773,2.99152,29.5066,33.3419},
{5.33625,7.15652,19.4265,19.8527},
{0.432492,3.44635,7.60608,10.3589},
{1.60933,2.36115,-1.16144,0.286511},
{9.27774,10.4284,-1.86525,-1.56998},
{11.8878,13.7671,-5.11518,-4.23808},
{10.0073,10.2574,11.282,12.9297},
{10.5778,11.703,0.629406,0.965262},
{3.17349,7.96657,16.1782,17.2631},
{11.2281,13.6998,12.2002,12.4339},
{13.7166,18.3124,19.2669,21.4145},
{10.5388,15.0813,0.328298,4.33324},
{9.05987,12.0136,-1.29945,-0.356432},
{6.74526,7.05518,-8.26859,-7.79509},
{17.7736,20.6859,5.72768,8.29825},
{2.22113,7.1491,21.4284,25.7683},
{16.1247,17.7842,2.70437,3.3773},
{8.34023,10.0276,13.9827,18.7779},
{15.1872,18.2348,17.1787,17.2902},
{16.5308,17.6299,6.45799,8.71682},
{5.98601,9.22204,23.2905,26.0581},
{7.64095,10.7424,18.778,22.0208},
{15.0585,19.52,17.7993,19.2668},
{17.646,20.037,0.451228,3.78949},
{0.0160052,1.94859,4.68464,6.71885},
{7.29434,9.17489,13.9827,17.3136},
{17.7896,21.639,6.02609,9.48842},
{4.43065,8.71164,19.8528,23.2904},
{16.5231,21.0708,3.3774,4.59677},
{14.2096,16.0172,17.282,17.9522},
{8.32648,9.09478,3.98524,7.65693},
{3.6703,7.6841,0.583351,4.90782},
{9.90605,10.9591,11.5162,13.1508},
{14.7669,18.5129,-1.21482,3.3773},
{3.48104,6.74301,29.5066,30.5551},
{9.22657,10.4539,-5.59946,-1.29955},
{8.12181,8.34154,-2.11601,1.66416},
{4.11375,5.37752,30.5552,34.3283},
{9.11434,12.5991,8.88212,10.4297},
{11.3365,14.9133,-8.77796,-4.9549},
{15.7809,16.0235,-2.30357,0.667869},
{15.4629,16.7224,4.59687,6.45789},
{13.3766,16.7042,21.4146,25.1652},
{7.87643,10.2361,12.8464,13.9826},
{7.38842,11.7536,11.5162,16.479},
{5.15799,9.74446,-7.42668,-4.32738},
{16.0009,16.044,-3.82308,0.667869},
{15.6318,18.7733,8.71692,11.607},
{10.7258,12.2305,7.30879,8.88202},
{8.17133,9.84923,-1.56988,1.38187},
{3.5179,5.38762,0.583351,5.06301},
{9.4516,10.8319,22.0209,26.7891},
{9.94456,11.8221,19.9812,21.2926},
{5.14479,8.31779,3.92878,7.65693},
{14.6057,18.2633,0.667969,4.59677},
{5.62784,8.67939,17.3137,19.4264},
{10.9244,13.5481,-1.29955,-1.29955},
{5.76759,6.30746,-3.722,-3.5131},
{7.13259,9.6258,8.33774,11.5161},
{1.05144,1.32443,10.359,10.9074},
{16.4807,19.9127,26.5036,27.9052},
{5.9629,7.66553,-1.41052,-1.14028},
{13.6489,16.9728,22.2084,25.5279},
{2.50777,4.43837,19.8193,21.4283},
{1.57577,3.7409,15.383,18.2258},
{17.1816,18.8466,17.2903,17.7992},
{8.43156,13.0154,-4.9548,-3.13422},
{1.88827,6.19077,6.98588,10.347},
{14.73,15.2147,17.9523,18.7923},
{5.55742,8.69305,2.0611,3.92868}};	
unsigned n11=130;
int main() {
	double c,t;
	/*
	cout << "test1" << endl;
	test(generateRects(test1,n1),n1,c,t);
	cout << "test2" << endl;
	test(generateRects(test2,n2),n2,c,t);
	cout << "test3" << endl;
	test(generateRects(test3,n3),n3,c,t);
	cout << "test4" << endl;
	test(generateRects(test4,n4),n4,c,t);
	cout << "test5" << endl;
	test(generateRects(test5,n5),n5,c,t);
	cout << "test6" << endl;
	test(generateRects(test6,n6),n6,c,t);
	cout << "test7" << endl;
	test(generateRects(test7,n7),n7,c,t);
	cout << "test8" << endl;
	test(generateRects(test8,n8),n8,c,t);
	cout << "test9" << endl;
	test(generateRects(test9,n9),n9,c,t);
	cout << "test10" << endl;
	test(generateRects(test10,n10),n10,c,t);
	
	cout << "test11" << endl;
	test(generateRects(test11,n11),n11,c,t);
	*/
	//*/
	int max_size=100, repeats=100,step=10; 
	srand(time(NULL));
	for(int i=10;i<=max_size;i+=step) {
		//if(i%5==0) cout << i << endl;
		double disp=0, time=0;
		for(int repeat=repeats;repeat--;) {
			test(generateRandomRects(i),i,c,t);
			disp+=c;
			time+=t;
		}   
		disp/=repeats;
		time/=repeats;
		cout << i << "," << time << "," << disp << endl;
	}
	//*/
}
