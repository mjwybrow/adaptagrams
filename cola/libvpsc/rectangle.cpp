/**
 * @file rectangle.cpp
 * @brief Functions to automatically generate constraints for the
 * rectangular node overlap removal problem.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#include <set>
#include <cassert>
#include "solve_VPSC.h"
#include "rectangle.h"
#include "constraint.h"

#include "isnan.h" /* Include last */

using std::set;
using std::vector;

namespace vpsc {
std::ostream& operator <<(std::ostream &os, const Rectangle &r) {
	os << "{"<<r.minX<<","<<r.maxX<<","<<r.minY<<","<<r.maxY<<"},";
	return os;
}

Rectangle::Rectangle(double x, double X, double y, double Y,bool allowOverlap) 
: minX(x),maxX(X),minY(y),maxY(Y),overlap(allowOverlap) {
	assert(x<=X);
	assert(y<=Y);
}
void Rectangle::reset(unsigned d, double x, double X) {
    if(d==0) {
        minX=x;
        maxX=X;
    } else {
        minY=x;
        maxY=X;
    }
}

struct Node;
struct CmpNodePos { bool operator()(const Node* u, const Node* v) const; };

typedef set<Node*,CmpNodePos> NodeSet;

struct Node {
	Variable *v;
	Rectangle *r;
	double pos;
	Node *firstAbove, *firstBelow;
	NodeSet *leftNeighbours, *rightNeighbours;
	Node(Variable *v, Rectangle *r, double p) : v(v),r(r),pos(p) {
		firstAbove=firstBelow=NULL;
		leftNeighbours=rightNeighbours=NULL;
		assert(r->width()<1e40);
	}
	~Node() {
		delete leftNeighbours;
		delete rightNeighbours;
	}
	void addLeftNeighbour(Node *u) {
		leftNeighbours->insert(u);
	}
	void addRightNeighbour(Node *u) {
		rightNeighbours->insert(u);
	}
	void setNeighbours(NodeSet *left, NodeSet *right) {
		leftNeighbours=left;
		rightNeighbours=right;
		for(NodeSet::iterator i=left->begin();i!=left->end();++i) {
			Node *v=*(i);
			v->addRightNeighbour(this);
		}
		for(NodeSet::iterator i=right->begin();i!=right->end();++i) {
			Node *v=*(i);
			v->addLeftNeighbour(this);
		}
	}
};
bool CmpNodePos::operator() (const Node* u, const Node* v) const {
	if (u->pos < v->pos) {
		return true;
	}
	if (v->pos < u->pos) {
		return false;
	}
	if (isNaN(u->pos) != isNaN(v->pos)) {
		return isNaN(u->pos) != 0; // magmy20070424: Avoid warning "forcing value to bool' under MS VC++
	}
	return u < v;
}

NodeSet* getLeftNeighbours(NodeSet &scanline,Node *v) {
	NodeSet *leftv = new NodeSet;
	NodeSet::iterator i=scanline.find(v);
	while(i--!=scanline.begin()) {
		Node *u=*(i);
		if(u->r->overlapX(v->r)<=0) {
			leftv->insert(u);
			return leftv;
		}
		if(u->r->overlapX(v->r)<=u->r->overlapY(v->r)) {
			leftv->insert(u);
		}
	}
	return leftv;
}
NodeSet* getRightNeighbours(NodeSet &scanline,Node *v) {
	NodeSet *rightv = new NodeSet;
	NodeSet::iterator i=scanline.find(v);
	for(++i;i!=scanline.end(); ++i) {
		Node *u=*(i);
		if(u->r->overlapX(v->r)<=0) {
			rightv->insert(u);
			return rightv;
		}
		if(u->r->overlapX(v->r)<=u->r->overlapY(v->r)) {
			rightv->insert(u);
		}
	}
	return rightv;
}

typedef enum {Open, Close} EventType;
struct Event {
	EventType type;
	Node *v;
	double pos;
	Event(EventType t, Node *v, double p) : type(t),v(v),pos(p) {};
};
Event **events;
int compare_events(const void *a, const void *b) {
	Event *ea=*(Event**)a;
	Event *eb=*(Event**)b;
	if(ea->pos==eb->pos) {
		// when comparing opening and closing
		// open must come first
		if(ea->type==Open) return -1;
		return 1;
	} else if(ea->pos > eb->pos) {
		return 1;
	} else if(ea->pos < eb->pos) {
		return -1;
	} else if(isNaN(ea->pos) != isNaN(ea->pos)) {
		/* See comment in CmpNodePos. */
		return ( isNaN(ea->pos)
			 ? -1
			 : 1 );
	}
	return 0;
}

/**
 * Prepares constraints in order to apply VPSC horizontally.  Assumes variables have already been created.
 * useNeighbourLists determines whether or not a heuristic is used to deciding whether to resolve
 * all overlap in the x pass, or leave some overlaps for the y pass.
 */
void generateXConstraints(vector<Rectangle*> const & rs, vector<Variable*> const &vars, vector<Constraint*> &cs, const bool useNeighbourLists) {
	const unsigned n = rs.size();
	events=new Event*[2*n];
	unsigned i,ctr=0;
	for(i=0;i<n;i++) {
		vars[i]->desiredPosition=rs[i]->getCentreX();
		Node *v = new Node(vars[i],rs[i],rs[i]->getCentreX());
		events[ctr++]=new Event(Open,v,rs[i]->getMinY());
		events[ctr++]=new Event(Close,v,rs[i]->getMaxY());
	}
	qsort((Event*)events, (size_t)2*n, sizeof(Event*), compare_events );

	NodeSet scanline;
	for(i=0;i<2*n;i++) {
		Event *e=events[i];
		Node *v=e->v;
		if(e->type==Open) {
			scanline.insert(v);
			if(useNeighbourLists) {
				v->setNeighbours(
					getLeftNeighbours(scanline,v),
					getRightNeighbours(scanline,v)
				);
			} else {
				NodeSet::iterator it=scanline.find(v);
				if(it--!=scanline.begin()) {
					Node *u=*it;
					v->firstAbove=u;
					u->firstBelow=v;
				}
				it=scanline.find(v);
				if(++it!=scanline.end()) {
					Node *u=*it;
					v->firstBelow=u;
					u->firstAbove=v;
				}
			}
		} else {
			// Close event
			int r;
			if(useNeighbourLists) {
				for(NodeSet::iterator i=v->leftNeighbours->begin();
					i!=v->leftNeighbours->end();i++
				) {
					Node *u=*i;
					double sep = (v->r->width()+u->r->width())/2.0;
					cs.push_back(new Constraint(u->v,v->v,sep));
					r=u->rightNeighbours->erase(v);
				}
				
				for(NodeSet::iterator i=v->rightNeighbours->begin();
					i!=v->rightNeighbours->end();i++
				) {
					Node *u=*i;
					double sep = (v->r->width()+u->r->width())/2.0;
					cs.push_back(new Constraint(v->v,u->v,sep));
					r=u->leftNeighbours->erase(v);
				}
			} else {
				Node *l=v->firstAbove, *r=v->firstBelow;
				if(l!=NULL) {
					double sep = (v->r->width()+l->r->width())/2.0;
					cs.push_back(new Constraint(l->v,v->v,sep));
					l->firstBelow=v->firstBelow;
				}
				if(r!=NULL) {
					double sep = (v->r->width()+r->r->width())/2.0;
					cs.push_back(new Constraint(v->v,r->v,sep));
					r->firstAbove=v->firstAbove;
				}
			}
			r=scanline.erase(v);
			delete v;
		}
		delete e;
	}
	delete [] events;
}

/**
 * Prepares constraints in order to apply VPSC vertically to remove ALL overlap.
 */
void generateYConstraints(const Rectangles& rs, const Variables& vars, Constraints& cs) {
	const unsigned n = rs.size();
    assert(vars.size()==n);
	events=new Event*[2*n];
	unsigned ctr=0;
    Rectangles::const_iterator ri=rs.begin();
    Variables::const_iterator vi=vars.begin();
    for(;ri!=rs.end()&&vi!=vars.end();++ri,++vi) {
        Rectangle* r=*ri;
        Variable* v=*vi;
		v->desiredPosition=r->getCentreY();
		Node *node = new Node(v,r,r->getCentreY());
		events[ctr++]=new Event(Open,node,r->getMinX());
		events[ctr++]=new Event(Close,node,r->getMaxX());
	}
    assert(ri==rs.end()&&vi==vars.end());
	qsort((Event*)events, (size_t)2*n, sizeof(Event*), compare_events );
	NodeSet scanline;
	for(unsigned i=0;i<2*n;i++) {
		Event *e=events[i];
		Node *v=e->v;
		if(e->type==Open) {
			scanline.insert(v);
			NodeSet::iterator it=scanline.find(v);
			if(it--!=scanline.begin()) {
				Node *u=*it;
				v->firstAbove=u;
				u->firstBelow=v;
			}
			it=scanline.find(v);
			if(++it!=scanline.end())	 {
				Node *u=*it;
				v->firstBelow=u;
				u->firstAbove=v;
			}
		} else {
			// Close event
			Node *l=v->firstAbove, *r=v->firstBelow;
			if(l!=NULL) {
				double sep = (v->r->height()+l->r->height())/2.0;
				cs.push_back(new Constraint(l->v,v->v,sep));
				l->firstBelow=v->firstBelow;
			}
			if(r!=NULL) {
				double sep = (v->r->height()+r->r->height())/2.0;
				cs.push_back(new Constraint(v->v,r->v,sep));
				r->firstAbove=v->firstAbove;
			}
			scanline.erase(v);
			delete v;
		}
		delete e;
	}
	delete [] events;
}
#include "linesegment.h"
using namespace linesegment;
inline bool checkIntersection(
        const LineSegment::IntersectResult result, 
        Vector const &intersection,
        RectangleIntersections &ri, 
        bool &side, double &sideX, double &sideY) {
    switch(result) {
    case LineSegment::INTERSECTING:
        ri.intersects=side=true;
        sideX=intersection.x_;
        sideY=intersection.y_;
    case LineSegment::PARALLEL:
    case LineSegment::NOT_INTERSECTING:
        return true;
    case LineSegment::COINCIDENT:
        ri.intersects=ri.top=ri.bottom=ri.left=ri.right=false;
        return false;
    }
    return false;
}
void Rectangle::
lineIntersections(double x1, double y1, double x2, double y2, RectangleIntersections &ri) const {
    Vector intersection;
    LineSegment l(Vector(x1,y1),Vector(x2,y2));
    LineSegment top(Vector(getMinX(),getMaxY()),Vector(getMaxX(),getMaxY()));
    if(!checkIntersection(
            l.Intersect(top,intersection),intersection,
            ri,ri.top,ri.topX,ri.topY)) {
        return;
    }
    LineSegment bottom(Vector(getMinX(),getMinY()),Vector(getMaxX(),getMinY()));
    if(!checkIntersection(
            l.Intersect(bottom,intersection),intersection,
            ri,ri.bottom,ri.bottomX,ri.bottomY)) {
        return;
    }
    LineSegment left(Vector(getMinX(),getMinY()),Vector(getMinX(),getMaxY()));
    if(!checkIntersection(
            l.Intersect(left,intersection),intersection,
            ri,ri.left,ri.leftX,ri.leftY)) {
        return;
    }
    LineSegment right(Vector(getMaxX(),getMinY()),Vector(getMaxX(),getMaxY()));
    if(!checkIntersection(
            l.Intersect(right,intersection),intersection,
            ri,ri.right,ri.rightX,ri.rightY)) {
        return;
    }
}
static const double ERROR_MARGIN = 1e-4;
inline bool eq(double a, double b) {
    return fabs(a-b)<ERROR_MARGIN;
    //return a==b;
}
/*
bool Rectangle::inside(double x, double y) const {
    return x>(minX+ERROR_MARGIN) && x<(maxX-ERROR_MARGIN) 
        && y>(minY+ERROR_MARGIN) && y<(maxY-ERROR_MARGIN);
}
*/
// p1=(x1,y1),p2=(x2,y2) are points on the boundary.  Puts the shortest
// path round the outside of the rectangle  from p1 to p2 into xs, ys.
void Rectangle::routeAround(double x1, double y1, double x2, double y2,
        std::vector<double> &xs, std::vector<double> &ys) {
    assert(eq(x1,minX) || eq(x1,maxX) || eq(y1,minY) || eq(y1,maxY));
    assert(eq(x2,minX) || eq(x2,maxX) || eq(y2,minY) || eq(y2,maxY));
    xs.push_back(x1);
    ys.push_back(y1);
    bool top1=eq(y1,maxY), top2=eq(y2,maxY),
         bottom1=eq(y1,minY), bottom2=eq(y2,minY);
    bool left1=eq(x1,minX), left2=eq(x2,minX), 
         right1=eq(x1,maxX), right2=eq(x2,maxX);
    bool leftright = left1 && right2 || right1 && left2;
    bool topbottom = top1 && bottom2 || bottom1 && top2;
    bool lefttop = left1 && top2 || top1 && left2;
    bool righttop = right1 && top2 || top1 && right2;
    bool leftbottom = left1 && bottom2 || bottom1 && left2;
    bool rightbottom = right1 && bottom2 || bottom1 && right2;
    if(lefttop) {
        xs.push_back(minX);
        ys.push_back(maxY);
    } else if(righttop) {
        xs.push_back(maxX);
        ys.push_back(maxY);
    } else if(leftbottom) {
        xs.push_back(minX);
        ys.push_back(minY);
    } else if(rightbottom) {
        xs.push_back(maxX);
        ys.push_back(minY);
    } else if(leftright) {
        double midY = y1+(y2-y1)/2.0;
        if(left1) { // left to right
            if(midY<getCentreY()) { // route below
                // bottom left
                xs.push_back(getMinX());
                ys.push_back(getMinY());
                // bottom right
                xs.push_back(getMaxX());
                ys.push_back(getMinY());
            } else { // route above
                // top left
                xs.push_back(getMinX());
                ys.push_back(getMaxY());
                // top right
                xs.push_back(getMaxX());
                ys.push_back(getMaxY());
            }
        } else { // right to left
            if(midY<getCentreY()) { // route below
                // bottom right
                xs.push_back(getMaxX());
                ys.push_back(getMinY());
                // bottom left
                xs.push_back(getMinX());
                ys.push_back(getMinY());
            } else { // route above
                // top right
                xs.push_back(getMaxX());
                ys.push_back(getMaxY());
                // top left
                xs.push_back(getMinX());
                ys.push_back(getMaxY());
            }
        }
    } else if(topbottom) {
        double midX = x1+(x2-x1)/2.0;
        if(top1) {
            if(midX<getCentreX()) { // route left
                // top left
                xs.push_back(getMinX());
                ys.push_back(getMaxY());
                // bottom left
                xs.push_back(getMinX());
                ys.push_back(getMinY());
            } else { // route right
                // top right
                xs.push_back(getMaxX());
                ys.push_back(getMaxY());
                // bottom right
                xs.push_back(getMaxX());
                ys.push_back(getMinY());
            }
        } else { // bottom to top
            if(midX<getCentreX()) { // route left
                // bottom left
                xs.push_back(getMinX());
                ys.push_back(getMinY());
                // top left
                xs.push_back(getMinX());
                ys.push_back(getMaxY());
            } else { // route right
                // bottom right
                xs.push_back(getMaxX());
                ys.push_back(getMinY());
                // top right
                xs.push_back(getMaxX());
                ys.push_back(getMaxY());
            }
        }
    }
    xs.push_back(x2);
    ys.push_back(y2);
}
/* 
 * moves all the rectangles to remove all overlaps.  Heuristic
 * attempts to move by as little as possible.
 * no overlaps guaranteed.
 * @param rs the rectangles which will be moved to remove overlap
 */
void removeoverlaps(Rectangles& rs) {
    const set<unsigned> fixed;
    removeoverlaps(rs,fixed);
}
/* 
 * moves all the rectangles to remove all overlaps.  Heuristic
 * attempts to move by as little as possible.
 * no overlaps guaranteed.
 * @param rs the rectangles which will be moved to remove overlap
 * @param fixed a set of indices to rectangles which should not be moved
 */
void removeoverlaps(Rectangles& rs, const set<unsigned>& fixed) {
	const double xBorder=Rectangle::xBorder, yBorder=Rectangle::yBorder;
    const double EXTRA_GAP=1e-7;
	unsigned n=rs.size();
	try {
		// The extra gap avoids numerical imprecision problems
		Rectangle::setXBorder(xBorder+EXTRA_GAP);
		Rectangle::setYBorder(yBorder+EXTRA_GAP);
		Variables vs(n);
		unsigned i=0;
		for(Variables::iterator v=vs.begin();v!=vs.end();++v,++i) {
            double weight=1;
            if(fixed.find(i)!=fixed.end()) {
                weight=10000;
            }
			*v=new Variable(i,0,weight);
		}
		Constraints cs;
		generateXConstraints(rs,vs,cs,true);
		Solver vpsc_x(vs,cs);
		vpsc_x.solve();
		Rectangles::iterator r=rs.begin();
		for(Variables::iterator v=vs.begin();v!=vs.end();++v,++r) {
			assert((*v)->finalPosition==(*v)->finalPosition);
			(*r)->moveCentreX((*v)->finalPosition);
		}
		assert(r==rs.end());
		for_each(cs.begin(),cs.end(),delete_object());
		cs.clear();
        // Removing the extra gap here ensures things that were moved to be
        // adjacent to one another above are not considered overlapping
		Rectangle::setXBorder(xBorder);
		generateYConstraints(rs,vs,cs);
		Solver vpsc_y(vs,cs);
		vpsc_y.solve();
		r=rs.begin();
		for(Variables::iterator v=vs.begin();v!=vs.end();++v,++r) {
			(*r)->moveCentreY((*v)->finalPosition);
		}
		for_each(cs.begin(),cs.end(),delete_object());
		cs.clear();
		Rectangle::setYBorder(yBorder);
		generateXConstraints(rs,vs,cs,false);
		Solver vpsc_x2(vs,cs);
		vpsc_x2.solve();
		r=rs.begin();
		for(Variables::iterator v=vs.begin();v!=vs.end();++v,++r) {
			(*r)->moveCentreX((*v)->finalPosition);
		}
		for_each(cs.begin(),cs.end(),delete_object());
		for_each(vs.begin(),vs.end(),delete_object());
	} catch (char *str) {
		std::cerr<<str<<std::endl;
		for(Rectangles::iterator r=rs.begin();r!=rs.end();++r) {
			std::cerr << **r <<std::endl;
		}
	}
}
void assertNoOverlaps(const Rectangles& rs) {
	for(Rectangles::const_iterator i=rs.begin();i!=rs.end();++i) {
		for(Rectangles::const_iterator j=i+1;j!=rs.end();++j) {
			Rectangle *u=*i, *v=*j;
			if(u->overlapX(v)>0) {
				assert(u->overlapY(v)==0);
			}
		}
	}
}

}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:cindent:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
