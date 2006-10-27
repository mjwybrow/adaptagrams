#include "graphlayouttest.h"
#include <libvpsc/rectangle.h>
#include <fstream>

#define ARROW_HEAD_BUFFER 3

using namespace cola;
using vpsc::Rectangle;
using std::endl;
using std::ios;
using std::max;
using std::min;
using std::ofstream;
using std::vector;

void output_svg(vector<Rectangle*>& rs, vector<Edge>& es, char *fname, bool rects) {
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
	output_svg(rs,routes,fname,rects);
	for(unsigned i=0;i<E;i++) {
		delete routes[i];
	}
}

void output_svg(vector<Rectangle*>& rs, vector<straightener::Edge*>& es, char *fname, bool rects) {
	double width, height;
	ofstream f(fname);
	f.setf(ios::fixed);
	double r=5;
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
	f<<"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""<<width<<"\" height=\""<<height<<"\" viewBox = \""
	 <<xmin<<" "<<ymin<<" "<<width<<" "<<height<<"\">"<<endl;
    	for (unsigned i=0;i<es.size();i++) {
		for (unsigned j=1;j<es[i]->route->n;j++) {
			f<<"<line x1=\""<<es[i]->route->xs[j-1]
			 <<"\" y1=\""<<es[i]->route->ys[j-1]
			 <<"\" x2=\""<<es[i]->route->xs[j]
			 <<"\" y2=\""<<es[i]->route->ys[j]
			 <<"\" style=\"stroke:rgb(99,99,99);stroke-width:2\"/>"<<endl;
		}
	}
	for (unsigned i=0;i<rs.size();i++) {
		f<<"<g id=\"node"<<i
		 <<"\" class=\"node\"><title>"<<i<<"</title>"<<endl;
		if(!rects) {
			f<<"<ellipse cx=\""<<rs[i]->getCentreX()
			 <<"\" cy=\""<<rs[i]->getCentreY()
			 <<"\" rx=\""<<r<<"\" ry=\""<<r
			 <<"\" style=\"fill:black;stroke:black;\"/>";
		} else {
			f<<"<rect x=\""<<rs[i]->getMinX()
			 <<"\" y=\""<<rs[i]->getMinY()
			 <<"\" width=\""<<rs[i]->width()
			 <<"\" height=\""<<rs[i]->height()
			 <<"\" style=\"fill:white;stroke:black;\"/>";
			f<<"<text x=\""<<(rs[i]->getCentreX()-3)
			 <<"\" y=\""<<(rs[i]->getCentreY()+5)
			 <<"\">"<<i
			 <<"</text>"<<endl;

		}
		f <<endl<<"</g>"<<endl;
	}
	f<<"</svg>"<<endl;
}

void output_svg(vector<Rectangle*>& rs, vector<Edge>& es, char *fname, bool rects, bool directedEdges, vector<bool> * cyclicEdges) {
	double width, height;
	ofstream f(fname);
	f.setf(ios::fixed);
	double r=5;
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

        std::cout << "Making a picture ..." << endl;

	f << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" ";
	f << "xmlns:sodipodi=\"http://inkscape.sourceforge.net/DTD/sodipodi-0.dtd\" ";
	f << "xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" width=\""<<width<<"\" height=\""<<height<<"\" viewBox = \"";
	f << xmin<<" "<<ymin<<" "<<width<<" "<<height<<"\">"<<endl;

        // code to start to draw arrow heads
	if (directedEdges)  {
	  f << "<defs id=\"defs4\">" << endl;
          f << "  <marker inkscape:stockid=\"ConnArrowEnd\"" << endl;
          f << "    orient=\"auto\" refY=\"0.0\" refX=\"" << 4.5 + r + ARROW_HEAD_BUFFER << "\"" << endl;
          f << "    id=\"ConnArrowEnd\" style=\"overflow:visible;\">" << endl;
          f << "  <path id=\"path3466\"" << endl;
          f << "    d=\"M 0.0,0.0 L 1.0,-5.0 L -12.5,0.0 L 1.0,5.0 L 0.0,0.0 z \"" << endl;
          f << "    style=\"fill-rule:evenodd;stroke:#000000;stroke-width:1.0px;marker-start:none;\"" << endl;
          f << "    transform=\"scale(0.6) rotate(180)\" />" << endl;
          f << "  </marker>" << endl;
          f << "</defs>" << endl;
	  f << "<defs id=\"defs5\">" << endl;
          f << "  <marker inkscape:stockid=\"CyclicConnArrowEnd\"" << endl;
          f << "    orient=\"auto\" refY=\"0.0\" refX=\"" << 4.5 + r + ARROW_HEAD_BUFFER << "\"" << endl;
          f << "    id=\"CyclicConnArrowEnd\" style=\"fill:#b400ff;fill-opacity:1;overflow:visible\">" << endl;
          f << "  <path id=\"path3466\"" << endl;
          f << "    d=\"M 0.0,0.0 L 1.0,-5.0 L -12.5,0.0 L 1.0,5.0 L 0.0,0.0 z \"" << endl;
          f << "    style=\"fill-rule:evenodd;stroke:#B400FF;stroke-width:1.0px;marker-start:none;\"" << endl;
          f << "    transform=\"scale(0.6) rotate(180)\" />" << endl;
          f << "  </marker>" << endl;
          f << "</defs>" << endl;
	}

	for (unsigned i=0;i<es.size();i++) {
		unsigned u = es[i].first;
		unsigned v = es[i].second;

		if (directedEdges)  {
		  f << "<path d=\"M" << rs[u]->getCentreX() << "," << rs[u]->getCentreY() << " L" << rs[v]->getCentreX() << "," << rs[v]->getCentreY();
		  f << "\"";
                  f << " style=\"stroke:";

                  if (cyclicEdges != NULL)  {
		    // if we have a cylic edge we want to draw it a different colour
		    if ((*cyclicEdges)[i])  { f << "rgb(180,0,255);stroke-width:1px;marker-end:url(#CyclicConnArrowEnd);stroke-miterlimit:4;stroke-dasharray:1,1;stroke-dashoffset:0\""; }
		    else  { f << "rgb(99,99,99);stroke-width:1px;marker-end:url(#ConnArrowEnd)\""; }
		  }
		  else  { f << "rgb(99,99,99);stroke-width:1px;marker-end:url(#ConnArrowEnd)\""; }

		  f << "/>" << endl;
		}
		else  {
		  f<<"<line x1=\""<<rs[u]->getCentreX()
		   <<"\" y1=\""<<rs[u]->getCentreY()
		   <<"\" x2=\""<<rs[v]->getCentreX()
		   <<"\" y2=\""<<rs[v]->getCentreY()
		   <<"\" style=\"stroke:rgb(99,99,99);stroke-width:2\"/>"<<endl;
		}	
	}
	for (unsigned i=0;i<rs.size();i++) {
		f<<"<g id=\"node"<<i
		 <<"\" class=\"node\"><title>"<<i<<"</title>"<<endl;
		if(!rects) {
			f<<"<ellipse cx=\""<<rs[i]->getCentreX()
			 <<"\" cy=\""<<rs[i]->getCentreY()
			 <<"\" rx=\""<<r<<"\" ry=\""<<r
			 <<"\" style=\"fill:black;stroke:black;\"/>"
			 <<"<text x=\"";
			
			if (i < 10)  { f << (rs[i]->getCentreX()-2); }
			else  { f << (rs[i]->getCentreX()-4); }

			f <<"\" y=\""<<(rs[i]->getCentreY()+2)
			  <<"\" style=\"fill:white\" font-size=\"7\">"<< i
			  <<"</text>"<<endl;		
		} else {
			f<<"<rect x=\""<<rs[i]->getMinX()
			 <<"\" y=\""<<rs[i]->getMinY()
			 <<"\" width=\""<<rs[i]->width()
			 <<"\" height=\""<<rs[i]->height()
			 <<"\" style=\"fill:white;stroke:black;\"/>";
			f<<"<text x=\""<<(rs[i]->getCentreX()-3)
			 <<"\" y=\""<<(rs[i]->getCentreY()+5)
			 <<"\">"<<i
			 <<"</text>"<<endl;

		}
		f <<endl<<"</g>"<<endl;
	}
	f<<"</svg>"<<endl;
}

void output_svg(vector<Rectangle*>& rs, vector<Edge>& es, char *fname, bool rects, bool directedEdges, vector<Edge>* Ea) {
	double width, height;
	ofstream f(fname);
	f.setf(ios::fixed);
	double r=5;
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
	f << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" ";
	f << "xmlns:sodipodi=\"http://inkscape.sourceforge.net/DTD/sodipodi-0.dtd\" ";
	f << "xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" width=\""<<width<<"\" height=\""<<height<<"\" viewBox = \"";
	f << xmin<<" "<<ymin<<" "<<width<<" "<<height<<"\">"<<endl;

        // code to start to draw arrow heads
	if (directedEdges)  {
	  f << "<defs id=\"defs4\">" << endl;
          f << "  <marker inkscape:stockid=\"ConnArrowEnd\"" << endl;
          f << "    orient=\"auto\" refY=\"0.0\" refX=\"" << 4.5 + r + ARROW_HEAD_BUFFER << "\"" << endl;
          f << "    id=\"ConnArrowEnd\" style=\"overflow:visible;\">" << endl;
          f << "  <path id=\"path3466\"" << endl;
          f << "    d=\"M 0.0,0.0 L 1.0,-5.0 L -12.5,0.0 L 1.0,5.0 L 0.0,0.0 z \"" << endl;
          f << "    style=\"fill-rule:evenodd;stroke:#000000;stroke-width:1.0px;marker-start:none;\"" << endl;
          f << "    transform=\"scale(0.6) rotate(180)\" />" << endl;
          f << "  </marker>" << endl;
          f << "</defs>" << endl;
	  f << "<defs id=\"defs5\">" << endl;
          f << "  <marker inkscape:stockid=\"CyclicConnArrowEnd\"" << endl;
          f << "    orient=\"auto\" refY=\"0.0\" refX=\"" << 4.5 + r + ARROW_HEAD_BUFFER << "\"" << endl;
          f << "    id=\"CyclicConnArrowEnd\" style=\"overflow:visible;\">" << endl;
          f << "  <path id=\"path3466\"" << endl;
          f << "    d=\"M 0.0,0.0 L 1.0,-5.0 L -12.5,0.0 L 1.0,5.0 L 0.0,0.0 z \"" << endl;
          f << "    style=\"fill-rule:evenodd;stroke:#B400FF;stroke-width:1.0px;marker-start:none;\"" << endl;
          f << "    transform=\"scale(0.6) rotate(180)\" />" << endl;
          f << "  </marker>" << endl;
          f << "</defs>" << endl;
	}

	for (unsigned i=0;i<Ea->size();i++) {
		unsigned u = (*Ea)[i].first;
		unsigned v = (*Ea)[i].second;

		if (directedEdges)  {
		  f << "<path d=\"M" << rs[u]->getCentreX() << "," << rs[u]->getCentreY() << " L" << rs[v]->getCentreX() << "," << rs[v]->getCentreY();
		  f << "\"";
                  f << " style=\"stroke:rgb(99,99,99);stroke-width:1px;marker-end:url(#ConnArrowEnd)\"/>" << endl;
		}
		else  {
		  f<<"<line x1=\""<<rs[u]->getCentreX()
		   <<"\" y1=\""<<rs[u]->getCentreY()
		   <<"\" x2=\""<<rs[v]->getCentreX()
		   <<"\" y2=\""<<rs[v]->getCentreY()
		   <<"\" style=\"stroke:rgb(99,99,99);stroke-width:2\"/>"<<endl;
		}	
	}
	for (unsigned i=0;i<rs.size();i++) {
		f<<"<g id=\"node"<<i
		 <<"\" class=\"node\"><title>"<<i<<"</title>"<<endl;
		if(!rects) {
			f<<"<ellipse cx=\""<<rs[i]->getCentreX()
			 <<"\" cy=\""<<rs[i]->getCentreY()
			 <<"\" rx=\""<<r<<"\" ry=\""<<r
			 <<"\" style=\"fill:black;stroke:black;\"/>"
			 <<"<text x=\"";
			
			if (i < 10)  { f << (rs[i]->getCentreX()-2); }
			else  { f << (rs[i]->getCentreX()-4); }

			f <<"\" y=\""<<(rs[i]->getCentreY()+2)
			  <<"\" style=\"fill:white\" font-size=\"7\">"<< i
			  <<"</text>"<<endl;		
		} else {
			f<<"<rect x=\""<<rs[i]->getMinX()
			 <<"\" y=\""<<rs[i]->getMinY()
			 <<"\" width=\""<<rs[i]->width()
			 <<"\" height=\""<<rs[i]->height()
			 <<"\" style=\"fill:white;stroke:black;\"/>";
			f<<"<text x=\""<<(rs[i]->getCentreX()-3)
			 <<"\" y=\""<<(rs[i]->getCentreY()+5)
			 <<"\">"<<i
			 <<"</text>"<<endl;

		}
		f <<endl<<"</g>"<<endl;
	}
	f<<"</svg>"<<endl;
}
