#ifndef _OUTPUT_SVG_H
#define _OUTPUT_SVG_H
#include "cola.h"
#include <cairomm/context.h>
#include <cairomm/surface.h>
struct ColourRGBA {
    double r, g, b, a;
    ColourRGBA() : r(0), g(0), b(0), a(1) {}
    ColourRGBA(double r, double g, double b, double a)
        : r(r), g(g), b(b), a(a) {}
    ColourRGBA(unsigned r, unsigned g, unsigned b, unsigned a)
        : r((double)r/255.), g((double)g/255.), b((double)b/255.), a((double)a/255.) {}
};
class OutputFile {
public:
	std::vector<vpsc::Rectangle*> const &rs;
	std::vector<cola::Edge> const &es;
	std::vector<straightener::Edge*> *routes;
	cola::RootCluster const * rc;
	std::string const fname;
	bool rects;
	bool curvedEdges;
    std::auto_ptr<std::vector<const char*> > labels;
    std::vector<ColourRGBA> *colours;
    ColourRGBA edgeColour;
	OutputFile(std::vector<vpsc::Rectangle*> const &rs, 
		std::vector<cola::Edge> const &es, 
		cola::RootCluster const * rc, 
		std::string const fname,
		const bool rects=false,
		const bool curvedEdges=false)
		: rs(rs),
		  es(es),
		  routes(NULL),
		  rc(rc),
		  fname(fname),
		  rects(rects),
		  curvedEdges(curvedEdges),
       	  labels(NULL),
          colours(NULL),
          edgeColour(0,0,0,0.1) {
      }
	void generate();
	void setLabels(const unsigned n, const char *ls[]) {
		labels.reset(new std::vector<const char *>(n));
		for(unsigned i=0;i<n;i++) {
			(*labels)[i]=ls[i];
		}
	}
private:
	void draw_cluster_boundary(Cairo::RefPtr<Cairo::Context> const &cr, 
        cola::Cluster &c, const double xmin, const double ymin);
	void draw_edges(Cairo::RefPtr<Cairo::Context> &cr, 
        std::vector<straightener::Edge*> const & es, 
        double const xmin, double const ymin);
	void draw_curved_edges(Cairo::RefPtr<Cairo::Context> &cr,
		std::vector<straightener::Edge*> const & es, 
		const double xmin, 
		const double ymin);
	void openCairo(Cairo::RefPtr<Cairo::Context> &cr, double width, double height);
};
#endif // _OUTPUT_SVG_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
