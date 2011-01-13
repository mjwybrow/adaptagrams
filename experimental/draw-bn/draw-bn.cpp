/*
 * Authors:
 *   Nathan Hurst
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */
#include <iostream>

#include <vector>
#include <algorithm>
#include <float.h>
#include <libcola/cola.h>
#include "graphlayouttest.h"

#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>
#include <cairo/cairo.h>
#include <boost/spirit/core.hpp>
#include <iostream>
#include <complex>
#include <string>
#include <pango/pangocairo.h>

using namespace std;
using namespace cola;

/** Convenience/readability wrapper for strcmp(a,b)==0. */
inline bool
streq(char const *a, char const *b)
{
    return std::strcmp(a, b) == 0;
}

class Node{
public:
    vector<char const*> states;
    vector<Node*> parents;
    vector<double> probs;
    string title;
    vector<double> belief;
    vpsc::Rectangle* position;
    int idx;
    
    void draw(cairo_t* cr, vpsc::Rectangle* r) {
        double x = position->getMinX()+10;
        double y = position->getMinY()+10;
        cout << *position << endl;
        cout << position->width() << endl;
        cairo_rectangle(cr, x, y, position->width(), position->height());
        cairo_stroke(cr);
        std::ostringstream notify;
        notify << title;// << " + " << p->heu() << " = " << p->goal_heu();
    
        PangoLayout* layout = pango_cairo_create_layout (cr);
        pango_layout_set_text(layout, 
                              notify.str().c_str(), -1);

        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, "Sans");
        const int size_px = 12;
        pango_font_description_set_absolute_size(font_desc, size_px * PANGO_SCALE);
        pango_layout_set_font_description(layout, font_desc);
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        cairo_move_to(cr, x+3, y+3);
        pango_cairo_show_layout(cr, layout);
    }
    
    void set_minimum_bounds(cairo_t* cr) {
        std::ostringstream notify;
        notify << title;// << " + " << p->heu() << " = " << p->goal_heu();
    
        PangoLayout* layout = pango_cairo_create_layout (cr);
        pango_layout_set_text(layout, 
                              notify.str().c_str(), -1);

        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, "Sans");
        const int size_px = 12;
        pango_font_description_set_absolute_size(font_desc, size_px * PANGO_SCALE);
        pango_layout_set_font_description(layout, font_desc);
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        position->set_width(logical_extent.width+20);
        position->set_height(logical_extent.height+20);
    }

    Node(char const* name) : title(name) {
        double x = getRand(1000);
        double y = getRand(1000);
        position = new vpsc::Rectangle(x, x + 1, y, y + 1);
    }
    Node(vpsc::Rectangle* r) : position(r) {}
};

#include "asia.cpp"

static cairo_surface_t *
cairo_surface_for_filename(char const *const filename, double const width, double const height)
{
    cairo_surface_t *cr_s = NULL;
    size_t const namelen = strlen(filename);
    if (CAIRO_HAS_SVG_SURFACE
        ? ((namelen >= 4) && streq(filename + namelen - 4, ".pdf"))
        : !((namelen >= 4) && streq(filename + namelen - 4, ".svg")))
    {
#if CAIRO_HAS_PDF_SURFACE
        cr_s = cairo_pdf_surface_create(filename, width, height);
#else
        fprintf("Sorry, cairo not built with PDF support, not saving.\n");
#endif
    } else {
#if CAIRO_HAS_SVG_SURFACE
        cr_s = cairo_svg_surface_create(filename, width, height);
# else
        fprintf("Sorry, cairo not built with SVG support, not saving.\n");
#endif
    }
    return cr_s;
}

/** 
 * \brief Determines when to terminate layout of a particular graph based
 * on a given relative tolerance. 
 */
int main(int argc, char** argv) {
    typedef pair < unsigned, unsigned >Edge;
    vector<Edge> es;
    double width=100;
    vector<vpsc::Rectangle*> rs;
    vector<Node*> network;
    
    cairo_surface_t * cr_s = 
        cairo_surface_for_filename("bn.pdf", 1, 1);

    if (!cr_s) {
        return 1;
    }
    cairo_t *cr = cairo_create(cr_s);
    
    SimpleConstraints  scy;
    construct_graph(network);
    for(unsigned i = 0; i < network.size(); i++) {
        Node* n = network[i];
        n->idx = i;
        n->set_minimum_bounds(cr);
        rs.push_back(n->position);
        
        for(unsigned j = 0; j < n->parents.size(); j++) {
            es.push_back(Edge(n->parents[j]->idx, i));

            scy.push_back(new SimpleConstraint(n->parents[j]->idx, i, 100));
        }
    }
    
    ConstrainedMajorizationLayout alg(rs,es,NULL,width/2);
    
    alg.setYSimpleConstraints(&scy);
    alg.run();
    
    double page_width, page_height;
    
    {
    vpsc::Rectangle display_window = cola::bounds(rs);
    double dx = -display_window.getMinX();
    double dy = -display_window.getMinY();
    page_width = display_window.width();
    page_height = display_window.height();
    
    cout << "shifting: (" << dx << ", " << dy << ")\n";
    cout << "size: (" << page_width << ", " << page_height << ")\n";
    for(unsigned i = 0; i < network.size(); i++) {
	rs[i]->offset(dx, dy);
    }
    }
/*
    PageBoundaryConstraints pbcx(0, page_width, 10000);
    PageBoundaryConstraints pbcy(0, page_height, 10000);
    for(unsigned i = 0; i < network.size(); i++) {
        pbcx.offsets.push_back(make_pair(i, rs[i]->width()/2));
        pbcy.offsets.push_back(make_pair(i, rs[i]->height()/2));
    }
    alg.setXPageBoundaryConstraints(&pbcx);
    alg.setYPageBoundaryConstraints(&pbcx);
    alg.run();
*/
    alg.setAvoidOverlaps();
    alg.run();
    {
    vpsc::Rectangle display_window = cola::bounds(rs);
    double dx = 10-display_window.getMinX();
    double dy = 10-display_window.getMinY();
    page_width = display_window.width();
    page_height = display_window.height();
    
    cout << "shifting: (" << dx << ", " << dy << ")\n";
    cout << "size: (" << page_width << ", " << page_height << ")\n";
    for(unsigned i = 0; i < network.size(); i++) {
	rs[i]->offset(dx, dy);
    }
    display_window = cola::bounds(rs);
    cout << display_window << endl;
    }
    cr_s = 
        cairo_surface_for_filename("bn.pdf", page_width+100, page_height+20);

    if (!cr_s) {
        return 1;
    }
    cr = cairo_create(cr_s);
    
    for(unsigned i = 0; i < es.size(); i++) {
        unsigned st = es[i].first;
        unsigned en = es[i].second;
        cairo_move_to(cr, 
                      network[st]->position->getCentreX(),
                      network[st]->position->getCentreY());
        cairo_line_to(cr, 
                      network[en]->position->getCentreX(),
                      network[en]->position->getCentreY());
        cairo_stroke(cr);
    }

    for(unsigned i = 0; i < network.size(); i++)
        network[i]->draw(cr, rs[i]);
    
    
    cairo_show_page(cr);
    cairo_destroy(cr);
    cairo_surface_destroy(cr_s);
    
    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
