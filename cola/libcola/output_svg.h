#include "cola.h"
void output_svg(std::vector<vpsc::Rectangle*> const &rs, 
		std::vector<cola::Edge> const & es, 
		cola::RootCluster const & rc, 
		char const* fname,
		const bool rects=false);

void output_svg(std::vector<vpsc::Rectangle*> const &rs, 
		std::vector<straightener::Edge*> const & es, 
		cola::RootCluster const & rc, 
		char const* fname,
		const bool rects);

void output_svg(std::vector<vpsc::Rectangle*> const &rs, 
		std::vector<cola::Edge> const & es, 
		cola::RootCluster const & rc, 
		char const* fname,
		std::vector<const char*>const&labels,
		const bool rects=false);

void output_svg(std::vector<vpsc::Rectangle*> const &rs, 
		std::vector<straightener::Edge*> const & es, 
		cola::RootCluster const & rc, 
		char const* fname,
		std::vector<const char*>const&labels,
		const bool rects);
