#include "cola.h"
void output_svg(std::vector<vpsc::Rectangle*> const &rs, 
		std::vector<cola::Edge> const & es, 
		cola::Clusters const & cs, 
		char const* fname,
		const bool rects=false);

void output_svg(std::vector<vpsc::Rectangle*> const &rs, 
		std::vector<straightener::Edge*> const & es, 
		cola::Clusters const & cs, 
		char const* fname,
		const bool rects);
