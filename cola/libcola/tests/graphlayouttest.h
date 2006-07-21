#include <libcola/cola.h>
void output_svg(vector<vpsc::Rectangle*> &rs, vector<cola::Edge>& es, char* fname,bool rects=false);
void output_svg(vector<vpsc::Rectangle*> &rs, vector<straightener::Edge*>& es, char* fname,bool rects=false);
void output_svg(vector<vpsc::Rectangle*> &rs, vector<cola::Edge>& es, char* fname,bool rects, bool directedEdges, vector<bool> * cyclicEdges);
void output_svg(vector<vpsc::Rectangle*> &rs, vector<cola::Edge>& es, char* fname,bool rects, bool directedEdges, vector<cola::Edge>* Ea);

inline double getRand(double range) {
	return range*rand()/RAND_MAX;
}

