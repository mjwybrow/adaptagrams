#include <valarray>
#include <libcola/convex_hull.h>
#include "graphlayouttest.h"

#include <string>
#include <iostream>
#include <cairomm/context.h>
#include <cairomm/surface.h>

/* M_PI is defined in math.h in the case of Microsoft Visual C++ */
#if defined(_MSC_VER)
#define _USE_MATH_DEFINES
#include <math.h>
#endif 
void dot(Cairo::RefPtr<Cairo::Context> & cr, double x, double y) {
    cr->arc(x, y, 
            2., 0.0, 2.0 * M_PI);
    cr->stroke();
}

int main(int argc, char** argv)
{
    unsigned n=20;
	std::valarray<double> X(n);
	std::valarray<double> Y(n);
    srand(time(NULL));
	for(unsigned i=0;i<n;i++) {
		X[i]=getRand(1.);
		Y[i]=getRand(1.);
	}
	std::vector<unsigned> hull;
	convexHull(X,Y,hull);
#ifdef CAIRO_HAS_SVG_SURFACE

    std::string filename = "image.svg";
    double width = 600;
    double height = 400;
    Cairo::RefPtr<Cairo::SvgSurface> surface =
        Cairo::SvgSurface::create(filename, width, height);

    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

    cr->save(); // save the state of the context
    cr->set_source_rgba(0.0, 0.0, 0.0, 0.7);
    // draw a circle in the center of the image
    for(unsigned i=0;i<n;i++) {
        dot(cr,height*X[i],height*(1-Y[i]));
    }

    cr->set_source_rgba(0.0, 0.0, 0.0, 0.3);
    cr->move_to(height*X[hull[0]],height*(1-Y[hull[0]]));
    for(int i=1;i<hull.size();i++) {
        cr->line_to(height*X[hull[i]],height*(1-Y[hull[i]]));
    }
    cr->line_to(height*X[hull[0]],height*(1-Y[hull[0]]));
    cr->stroke();
    cr->set_source_rgba(0.0, 0.0, 0.0, 1.);
    for(int i=0;i<hull.size();i++) {
        char ch[3];
        unsigned j=hull[i];
        printf("%d,%d\n",i,j);
        sprintf(ch,"%d",i);
        std::string s(ch);
        cr->move_to(height*X[j],height*(1-Y[j]));
        cr->show_text(s);
        cr->stroke();
    }
    cr->restore();

    cr->show_page();

    std::cout << "Wrote SVG file \"" << filename << "\"" << std::endl;
    return 0;

#else

    std::cout << "You must compile cairo with SVG support for this example to work."
        << std::endl;
    return 1;

#endif

}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
