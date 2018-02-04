/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

#ifndef _OUTPUT_SVG_H
#define _OUTPUT_SVG_H

#include "libcola/config.h"
#include "libcola/cola.h"

#ifdef HAVE_CAIROMM
#include <cairomm/context.h>
#include <cairomm/surface.h>
#endif

class OutputFile {
public:
	std::vector<vpsc::Rectangle*> const &rs;
	std::vector<cola::Edge> const &es;
	std::vector<straightener::Route*> *routes;
	cola::RootCluster const * rc;
	std::string const fname;
	bool rects;
	bool curvedEdges;
	OutputFile(std::vector<vpsc::Rectangle*> const &rs, 
		std::vector<cola::Edge> const &es, 
		cola::RootCluster const * rc, 
		std::string const fname,
		const bool rects=false,
		const bool curvedEdges=false)
		: rs(rs),
		  es(es),
		  routes(nullptr),
		  rc(rc),
		  fname(fname),
		  rects(rects),
		  curvedEdges(curvedEdges) {}
	void generate();
    void setLabels(std::vector<std::string> ls) {
        labels.resize(ls.size());
        std::copy(ls.begin(),ls.end(),labels.begin());
    }
	void setLabels(const unsigned n, const char **ls) {
        labels.resize(n);
        for(unsigned i=0;i<n;i++) {
            labels[i]=ls[i];
        }
	}
private:
#ifdef HAVE_CAIROMM
	void draw_cluster_boundary(Cairo::RefPtr<Cairo::Context> const &cr, 
        cola::Cluster &c, const double xmin, const double ymin);
	void draw_edges(Cairo::RefPtr<Cairo::Context> &cr, 
        std::vector<straightener::Route*> const & es, 
        double const xmin, double const ymin);
	void draw_curved_edges(Cairo::RefPtr<Cairo::Context> &cr,
		std::vector<cola::Edge> const & es, 
		const double xmin, 
		const double ymin);
	void openCairo(Cairo::RefPtr<Cairo::Context> &cr, double width, double height);
#endif // HAVE_CAIROMM
    std::vector<std::string> labels;
};
#endif // _OUTPUT_SVG_H
