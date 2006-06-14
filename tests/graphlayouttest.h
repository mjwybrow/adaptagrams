#include <cola.h>
void output_svg(vector<Rectangle*> &rs, vector<cola::Edge>& es, char* fname,bool rects=false);
void output_svg(vector<Rectangle*> &rs, vector<cola::Route*>& es, char* fname,bool rects=false);
inline double getRand(double range) {
	return range*rand()/RAND_MAX;
}

