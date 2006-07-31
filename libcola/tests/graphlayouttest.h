#include <libcola/cola.h>
void output_svg(std::vector<vpsc::Rectangle*> &rs, std::vector<cola::Edge>& es, char* fname,bool rects=false);
void output_svg(std::vector<vpsc::Rectangle*> &rs, std::vector<straightener::Edge*>& es, char* fname,bool rects=false);
void output_svg(std::vector<vpsc::Rectangle*> &rs, std::vector<cola::Edge>& es, char* fname,bool rects, bool directedEdges, std::vector<bool> * cyclicEdges);
void output_svg(std::vector<vpsc::Rectangle*> &rs, std::vector<cola::Edge>& es, char* fname,bool rects, bool directedEdges, std::vector<cola::Edge>* Ea);

inline double getRand(double range) {
	return range*rand()/RAND_MAX;
}

