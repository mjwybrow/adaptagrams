#include <string>
#include <iostream>
#include <sstream>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include "output_svg.h"
#include "cola.h"
#include "straightener.h"

using namespace cola;
using vpsc::Rectangle;
using std::endl;
using std::cout;
using std::ios;
using std::max;
using std::min;
using std::ofstream;
using std::vector;

void output_svg(vector<Rectangle*> const & rs, 
		vector<Edge> const & es, 
        RootCluster const & rc,
		char const *fname, 
		const bool rects) {
	unsigned E=es.size();
	vector<straightener::Edge*> routes(E);
	for(unsigned i=0;i<E;i++) {
		straightener::Route* r=new straightener::Route(2);
		r->xs[0]=rs[es[i].first]->getCentreX();
		r->ys[0]=rs[es[i].first]->getCentreY();
		r->xs[1]=rs[es[i].second]->getCentreX();
		r->ys[1]=rs[es[i].second]->getCentreY();
		routes[i]=new straightener::Edge(i,es[i].first,es[i].second,r);
	}
	output_svg(rs,routes,rc,fname,rects);
	for(unsigned i=0;i<E;i++) {
		delete routes[i];
	}
}

void output_svg(vector<Rectangle*> const & rs, 
        vector<Edge> const & es, 
        RootCluster const & rc,
        char const *fname, 
        vector<const char*>const & labels,
        const bool rects
) {
    unsigned E=es.size();
    vector<straightener::Edge*> routes(E);
    for(unsigned i=0;i<E;i++) {
        straightener::Route* r=new straightener::Route(2);
        r->xs[0]=rs[es[i].first]->getCentreX();
        r->ys[0]=rs[es[i].first]->getCentreY();
        r->xs[1]=rs[es[i].second]->getCentreX();
        r->ys[1]=rs[es[i].second]->getCentreY();
        routes[i]=new straightener::Edge(i,es[i].first,es[i].second,r);
    }
    output_svg(rs,routes,rc,fname,labels,rects);
    for(unsigned i=0;i<E;i++) {
        delete routes[i];
    }
}

void draw_cluster_boundary(Cairo::RefPtr<Cairo::Context> const &cr,
        vector<vpsc::Rectangle*> const &rs,
        Cluster &c,
        const double xmin,
        const double ymin) {
    c.computeBoundary(rs);
    cr->save();
    // background
    cr->set_source_rgb(0.7, 0.7, 224./255.);
    cr->move_to(c.hullX[0]-xmin,c.hullY[0]-ymin);
    for(unsigned i=1;i<c.hullX.size();i++) {
        cr->line_to(c.hullX[i]-xmin,c.hullY[i]-ymin);
    }
    cr->line_to(c.hullX[0]-xmin,c.hullY[0]-ymin);
    cr->fill();
    cr->restore();
    // outline
    cr->move_to(c.hullX[0]-xmin,c.hullY[0]-ymin);
    for(unsigned i=1;i<c.hullX.size();i++) {
        cr->line_to(c.hullX[i]-xmin,c.hullY[i]-ymin);
    }
    cr->line_to(c.hullX[0]-xmin,c.hullY[0]-ymin);
    cr->stroke();
}

void output_svg(vector<vpsc::Rectangle*> const &rs, 
		vector<straightener::Edge*> const & es, 
		RootCluster const & rc, 
		char const* fname,
		const bool rects)
{
#ifdef CAIRO_HAS_SVG_SURFACE

    std::string filename(fname);
    double width,height,r=5;
	if(rects) r=rs[0]->width()/2;
	double xmin=DBL_MAX, ymin=xmin;
	double xmax=-DBL_MAX, ymax=xmax;
	for (unsigned i=0;i<rs.size();i++) {
		double x=rs[i]->getCentreX(), y=rs[i]->getCentreY();
		xmin=min(xmin,x);
		ymin=min(ymin,y);
		xmax=max(xmax,x);
		ymax=max(ymax,y);
	}
	xmax+=2*r;
	ymax+=2*r;
	xmin-=2*r;
	ymin-=2*r;
	width=xmax-xmin;
	height=ymax-ymin;
    Cairo::RefPtr<Cairo::SvgSurface> surface =
        Cairo::SvgSurface::create(filename, width, height);

    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

    /* set background colour
    cr->save(); // save the state of the context
    cr->set_source_rgb(0.86, 0.85, 0.47);
    cr->paint();    // fill image with the color
    cr->restore();  // color is back to black now
    */

    cr->set_line_width(1.);
    cr->set_font_size(8);
    cr->save();
    for(Clusters::const_iterator c=rc.clusters.begin();c!=rc.clusters.end();c++) {
        draw_cluster_boundary(cr,rs,**c,xmin,ymin);
    }
    for (unsigned i=0;i<es.size();i++) {
        cr->move_to(es[i]->route->xs[0]-xmin,es[i]->route->ys[0]-ymin);
		for (unsigned j=1;j<es[i]->route->n;j++) {
            cr->line_to(es[i]->route->xs[j]-xmin,es[i]->route->ys[j]-ymin);
		}
        cr->stroke();
	}
    Cairo::TextExtents te;
	for (unsigned i=0;i<rs.size();i++) {
        double x=rs[i]->getCentreX()-xmin, y=rs[i]->getCentreY()-ymin;
		if(!rects) {
            cr->arc(x,y,r, 0.0, 2.0 * M_PI);
            cr->stroke();
		} else {
            cr->rectangle(rs[i]->getMinX()-xmin+1,rs[i]->getMinY()-ymin+1,rs[i]->width()-2,rs[i]->height()-2);
            cr->stroke_preserve();
            cr->save();
            cr->set_source_rgb(245./255., 233./255., 177./255.);
            cr->fill();
            cr->restore();
            std::stringstream s; s<<i;
            cr->get_text_extents(s.str(),te);
            cr->move_to(x-te.x_bearing-te.width/2.,y+te.height/2.);
            cr->show_text(s.str());
            cr->stroke();
		}
	}

    cr->show_page();

    std::cout << "Wrote SVG file \"" << filename << "\"" << std::endl;

#else

    std::cout << "You must compile cairo with SVG support for this example to work."
        << std::endl;
#endif
}
void output_svg(vector<vpsc::Rectangle*> const &rs, 
        vector<straightener::Edge*> const & es, 
        RootCluster const & rc, 
        char const* fname,
        vector<const char*>const& labels,
        const bool rects
)
{
#ifdef CAIRO_HAS_SVG_SURFACE

    std::string filename(fname);
    double width,height,r=5;
    if(rects) r=rs[0]->width()/2;
    double xmin=DBL_MAX, ymin=xmin;
    double xmax=-DBL_MAX, ymax=xmax;
    for (unsigned i=0;i<rs.size();i++) {
        double x=rs[i]->getCentreX(), y=rs[i]->getCentreY();
        xmin=min(xmin,x);
        ymin=min(ymin,y);
        xmax=max(xmax,x);
        ymax=max(ymax,y);
    }
    xmax+=2*r;
    ymax+=2*r;
    xmin-=2*r;
    ymin-=2*r;
    width=xmax-xmin;
    height=ymax-ymin;
    Cairo::RefPtr<Cairo::SvgSurface> surface =
        Cairo::SvgSurface::create(filename, width, height);

    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

    /* set background colour
    cr->save(); // save the state of the context
    cr->set_source_rgb(0.86, 0.85, 0.47);
    cr->paint();    // fill image with the color
    cr->restore();  // color is back to black now
    */

    cr->set_line_width(1.);
    cr->set_font_size(8);
    cr->save();
    for(Clusters::const_iterator c=rc.clusters.begin();c!=rc.clusters.end();c++) {
        draw_cluster_boundary(cr,rs,**c,xmin,ymin);
    }
    for (unsigned i=0;i<es.size();i++) {
        cr->move_to(es[i]->route->xs[0]-xmin,es[i]->route->ys[0]-ymin);
        for (unsigned j=1;j<es[i]->route->n;j++) {
            cr->line_to(es[i]->route->xs[j]-xmin,es[i]->route->ys[j]-ymin);
        }
        cr->stroke();
    }
    Cairo::TextExtents te;
    for (unsigned i=0;i<rs.size();i++) {
        double x=rs[i]->getCentreX()-xmin, y=rs[i]->getCentreY()-ymin;
        if(!rects) {
            cr->arc(x,y,r, 0.0, 2.0 * M_PI);
            cr->stroke();
        } else {
            cr->rectangle(rs[i]->getMinX()-xmin+1,rs[i]->getMinY()-ymin+1,rs[i]->width()-2,rs[i]->height()-2);
            cr->stroke_preserve();
            cr->save();
            cr->set_source_rgb(245./255., 233./255., 177./255.);
            cr->fill();
            cr->restore();
            std::stringstream s; s<<i;
            cr->get_text_extents(labels[i],te);
            cr->move_to(x-te.x_bearing-te.width/2.,y+te.height/2.);
            cr->show_text(labels[i]);
            cr->stroke();
        }
    }

    cr->show_page();

    std::cout << "Wrote SVG file \"" << filename << "\"" << std::endl;

#else

    std::cout << "You must compile cairo with SVG support for this example to work."
        << std::endl;
#endif
}

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
