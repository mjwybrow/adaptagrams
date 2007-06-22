#ifndef _OUTPUT_SVG_H
#define _OUTPUT_SVG_H
#include "cola.h"
#include <cairomm/context.h>
#include <cairomm/surface.h>
class OutputFile {
public:
	std::vector<vpsc::Rectangle*> const &rs;
	std::vector<cola::Edge> const &es;
	std::vector<straightener::Edge*> *routes;
	cola::RootCluster const * rc;
	std::string const fname;
	bool rects;
	bool curvedEdges;
	std::vector<const char*>* labels;
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
		  deleteLabels(false)	{}
	~OutputFile() {
		if(deleteLabels) {
			delete labels;
		}
	}
	void generate();
	void setLabels(const unsigned n, const char *ls[]) {
		deleteLabels=true;
		labels=new std::vector<const char *>(n);
		for(unsigned i=0;i<n;i++) {
			(*labels)[i]=ls[i];
		}
	}
private:
	bool deleteLabels;
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
