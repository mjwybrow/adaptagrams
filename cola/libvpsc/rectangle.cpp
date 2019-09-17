/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2005-2008  Monash University
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
 * Author(s):  Tim Dwyer
*/

/*
 * @brief Functions to automatically generate constraints for the
 * rectangular node overlap removal problem.
 *
 */

#include <cmath>
#include <set>
#include <cstdlib>
#include <algorithm>
#include <cstdio>

#include "libvpsc/assertions.h"
#include "libvpsc/solve_VPSC.h"
#include "libvpsc/rectangle.h"
#include "libvpsc/constraint.h"
#include "libvpsc/variable.h"

using std::set;
using std::vector;

namespace vpsc {

double Rectangle::xBorder = 0;
double Rectangle::yBorder = 0;

std::ostream& operator <<(std::ostream &os, const Rectangle &r) {
    os << "Hue[0.17],Rectangle[{"<<r.getMinX()<<","<<r.getMinY()<<"},{"<<r.getMaxX()<<","<<r.getMaxY()<<"}]";
    return os;
}

Rectangle::Rectangle(double x, double X, double y, double Y,bool allowOverlap) 
    : minX(x),
      maxX(X),
      minY(y),
      maxY(Y),
      overlap(allowOverlap) 
{
    COLA_ASSERT(x<X);
    COLA_ASSERT(y<Y);
    COLA_ASSERT(getMinX()<getMaxX());
    COLA_ASSERT(getMinY()<getMaxY());
}

Rectangle::Rectangle()
    : minX(1),
      maxX(-1),
      minY(1),
      maxY(-1),
      overlap(false) 
{
    // Creates an invalid Rectangle
}

bool Rectangle::isValid(void) const
{
    return ((minX <= maxX) && (minY <= maxY));
}

Rectangle Rectangle::unionWith(const Rectangle& rhs) const
{
    if (!isValid())
    {
        return Rectangle(rhs);
    }
    else if (!rhs.isValid())
    {
        return Rectangle(*this);
    }
    
    double newMaxY = std::max(rhs.getMaxY(),maxY);
    double newMinY = std::min(rhs.getMinY(),minY);
    double newMinX = std::min(rhs.getMinX(),minX);
    double newMaxX = std::max(rhs.getMaxX(),maxX);

    return Rectangle(newMinX, newMaxX, newMinY, newMaxY);
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
    Node(Variable *v, Rectangle *r, double p) 
        : v(v),r(r),pos(p),
          firstAbove(nullptr), firstBelow(nullptr),
          leftNeighbours(nullptr), rightNeighbours(nullptr)
     
    {
        COLA_ASSERT(r->width()<1e40);
    }
    ~Node() {
        delete leftNeighbours;
        delete rightNeighbours;
    }
    void addLeftNeighbour(Node *u) {
        COLA_ASSERT(leftNeighbours!=nullptr);
        leftNeighbours->insert(u);
    }
    void addRightNeighbour(Node *u) {
        COLA_ASSERT(rightNeighbours!=nullptr);
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
    COLA_ASSERT(!std::isnan(u->pos));
    COLA_ASSERT(!std::isnan(v->pos));
    if (u->pos < v->pos) {
        return true;
    }
    if (v->pos < u->pos) {
        return false;
    }
    return u < v;
}

NodeSet* getLeftNeighbours(NodeSet &scanline,Node *v) {
    NodeSet *leftv = new NodeSet;
    NodeSet::iterator i=scanline.find(v);
    while(i!=scanline.begin()) {
        Node *u=*(--i);
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
    } else if(std::isnan(ea->pos) != std::isnan(ea->pos)) {
        /* See comment in CmpNodePos. */
        return ( std::isnan(ea->pos)
             ? -1
             : 1 );
    }
    return 0;
}

/*
 * Prepares constraints in order to apply VPSC horizontally.  Assumes 
 * variables have already been created.
 * useNeighbourLists determines whether or not a heuristic is used to 
 * deciding whether to resolve all overlap in the x pass, or leave some
 * overlaps for the y pass.
 */
void generateXConstraints(const Rectangles& rs, const Variables& vars,
        Constraints& cs, const bool useNeighbourLists)
{
    const unsigned n = rs.size();
    COLA_ASSERT(vars.size()>=n);
    Event **events=new Event*[2*n];
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
                if(it!=scanline.begin()) {
                    Node *u=*(--it);
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
            size_t result;
            // Close event
            if(useNeighbourLists) {
                for(NodeSet::iterator i=v->leftNeighbours->begin();
                    i!=v->leftNeighbours->end();i++
                ) {
                    Node *u=*i;
                    double sep = (v->r->width()+u->r->width())/2.0;
                    cs.push_back(new Constraint(u->v,v->v,sep));
                    result=u->rightNeighbours->erase(v);
                    COLA_ASSERT(result==1);
                }
                
                for(NodeSet::iterator i=v->rightNeighbours->begin();
                    i!=v->rightNeighbours->end();i++
                ) {
                    Node *u=*i;
                    double sep = (v->r->width()+u->r->width())/2.0;
                    cs.push_back(new Constraint(v->v,u->v,sep));
                    result=u->leftNeighbours->erase(v);
                    COLA_ASSERT(result==1);
                }
            } else {
                Node *l=v->firstAbove, *r=v->firstBelow;
                if(l!=nullptr) {
                    double sep = (v->r->width()+l->r->width())/2.0;
                    cs.push_back(new Constraint(l->v,v->v,sep));
                    l->firstBelow=v->firstBelow;
                }
                if(r!=nullptr) {
                    double sep = (v->r->width()+r->r->width())/2.0;
                    cs.push_back(new Constraint(v->v,r->v,sep));
                    r->firstAbove=v->firstAbove;
                }
            }
            result=scanline.erase(v);
            COLA_ASSERT(result==1);
            delete v;
        }
        delete e;
    }
    COLA_ASSERT(scanline.size()==0);
    delete [] events;
}

/*
 * Prepares constraints in order to apply VPSC vertically to remove ALL
 * overlap.
 */
void generateYConstraints(const Rectangles& rs, const Variables& vars,
        Constraints& cs)
{
    const unsigned n = rs.size();
    COLA_ASSERT(vars.size()>=n);
    Event **events=new Event*[2*n];
    unsigned ctr=0;
    Rectangles::const_iterator ri=rs.begin(), re=rs.end();
    Variables::const_iterator vi=vars.begin(), ve=vars.end();
    for(;ri!=re&&vi!=ve;++ri,++vi) {
        Rectangle* r=*ri;
        Variable* v=*vi;
        v->desiredPosition=r->getCentreY();
        Node *node = new Node(v,r,r->getCentreY());
        COLA_ASSERT(r->getMinX()<r->getMaxX());
        events[ctr++]=new Event(Open,node,r->getMinX());
        events[ctr++]=new Event(Close,node,r->getMaxX());
    }
    COLA_ASSERT(ri==rs.end());
    qsort((Event*)events, (size_t)2*n, sizeof(Event*), compare_events );
    NodeSet scanline;
#ifndef NDEBUG
    size_t deletes=0;
#endif
    for(unsigned i=0;i<2*n;i++) {
        Event *e=events[i];
        Node *v=e->v;
        if(e->type==Open) {
            scanline.insert(v);
            NodeSet::iterator it=scanline.find(v);
            if(it!=scanline.begin()) {
                Node *u=*(--it);
                v->firstAbove=u;
                u->firstBelow=v;
            }
            it=scanline.find(v);
            if(++it!=scanline.end())     {
                Node *u=*it;
                v->firstBelow=u;
                u->firstAbove=v;
            }
        } else {
            // Close event
            Node *l=v->firstAbove, *r=v->firstBelow;
            if(l!=nullptr) {
                double sep = (v->r->height()+l->r->height())/2.0;
                cs.push_back(new Constraint(l->v,v->v,sep));
                l->firstBelow=v->firstBelow;
            }
            if(r!=nullptr) {
                double sep = (v->r->height()+r->r->height())/2.0;
                cs.push_back(new Constraint(v->v,r->v,sep));
                r->firstAbove=v->firstAbove;
            }
#ifndef NDEBUG
            deletes++;
            size_t erased=
#endif
            scanline.erase(v);
            COLA_ASSERT(erased==1);
            delete v;
        }
        delete e;
    }
    COLA_ASSERT(scanline.size()==0);
    COLA_ASSERT(deletes==n);
    delete [] events;
}
#include "libvpsc/linesegment.h"
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
    COLA_ASSERT(eq(x1,minX) || eq(x1,maxX) || eq(y1,minY) || eq(y1,maxY));
    COLA_ASSERT(eq(x2,minX) || eq(x2,maxX) || eq(y2,minY) || eq(y2,maxY));
    xs.push_back(x1);
    ys.push_back(y1);
    bool top1=eq(y1,maxY), top2=eq(y2,maxY),
         bottom1=eq(y1,minY), bottom2=eq(y2,minY);
    bool left1=eq(x1,minX), left2=eq(x2,minX), 
         right1=eq(x1,maxX), right2=eq(x2,maxX);
    bool leftright = (left1 && right2) || (right1 && left2);
    bool topbottom = (top1 && bottom2) || (bottom1 && top2);
    bool lefttop = (left1 && top2) || (top1 && left2);
    bool righttop = (right1 && top2) || (top1 && right2);
    bool leftbottom = (left1 && bottom2) || (bottom1 && left2);
    bool rightbottom = (right1 && bottom2) || (bottom1 && right2);
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
    const set<unsigned> fixed = set<unsigned>();
    removeoverlaps(rs,fixed);
}
#define ISNOTNAN(d) (d)==(d)
/*
 * Moves rectangles to remove all overlaps.  A heuristic
 * attempts to move by as little as possible.  The heuristic is
 * that the overlaps are removed horizontally and then vertically,
 * each pass being a quadratic program in which the total squared movement
 * is minimised subject to non-overlap constraints.  An optional third
 * horizontal pass (in addition to the first horizontal pass and the second
 * vertical pass) can be applied wherein the x-positions of rectangles are reset to their
 * original positions and overlap removal repeated.  This may avoid some
 * unnecessary movement. 
 * @param rs the rectangles which will be moved to remove overlap
 * @param fixed a set of indices to rectangles which should not be moved
 * @param thirdPass optionally run the third horizontal pass described above.
 */
void removeoverlaps(Rectangles& rs, const set<unsigned>& fixed, bool thirdPass) {
    const double xBorder=Rectangle::xBorder, yBorder=Rectangle::yBorder;
    static const double EXTRA_GAP=1e-3;
    static const size_t ARRAY_UNUSED=1;
    unsigned n=rs.size();
    try {
        // The extra gap avoids numerical imprecision problems
        Rectangle::setXBorder(xBorder+EXTRA_GAP);
        Rectangle::setYBorder(yBorder+EXTRA_GAP);
        Variables vs(n);
        Variables::iterator v;
        unsigned i=0;
        vector<double> initX(thirdPass?n:ARRAY_UNUSED);
        for(v=vs.begin();v!=vs.end();++v,++i) {
            double weight=1;
            if(fixed.find(i)!=fixed.end()) {
                weight=10000;
            }
            *v=new Variable(i,0,weight);
            if(thirdPass) {
                initX[i]=rs[i]->getCentreX();
            }
        }
        Constraints cs;
        generateXConstraints(rs,vs,cs,true);
        Solver vpsc_x(vs,cs);
        vpsc_x.solve();
        Rectangles::iterator r=rs.begin();
        for(v=vs.begin();v!=vs.end();++v,++r) {
            COLA_ASSERT(ISNOTNAN((*v)->finalPosition));
            (*r)->moveCentreX((*v)->finalPosition);
        }
        COLA_ASSERT(r==rs.end());
        for_each(cs.begin(),cs.end(),delete_object());
        cs.clear();
        // Removing the extra gap here ensures things that were moved to be
        // adjacent to one another above are not considered overlapping
        Rectangle::setXBorder(xBorder);
        generateYConstraints(rs,vs,cs);
        Solver vpsc_y(vs,cs);
        vpsc_y.solve();
        r=rs.begin();
        for(v=vs.begin();v!=vs.end();++v,++r) {
            COLA_ASSERT(ISNOTNAN((*v)->finalPosition));
            (*r)->moveCentreY((*v)->finalPosition);
        }
        for_each(cs.begin(),cs.end(),delete_object());
        cs.clear();
        Rectangle::setYBorder(yBorder);
        if(thirdPass) {
            // we reset x positions to their original values
            // and apply a third pass horizontally so that
            // rectangles which were moved unnecessarily in the
            // first horizontal pass (i.e. their overlap
            // was later resolved vertically) have an
            // opportunity now to stay put.
            Rectangle::setXBorder(xBorder+EXTRA_GAP);
            r=rs.begin();
            for(v=vs.begin();v!=vs.end();++v,++r) {
                (*r)->moveCentreX(initX[(*v)->id]);
            }
            generateXConstraints(rs,vs,cs,false);
            Solver vpsc_x2(vs,cs);
            vpsc_x2.solve();
            r=rs.begin();
            for(v=vs.begin();v!=vs.end();++v,++r) {
                COLA_ASSERT(ISNOTNAN((*v)->finalPosition));
                (*r)->moveCentreX((*v)->finalPosition);
            }
        }
        Rectangle::setXBorder(xBorder);
        for_each(cs.begin(),cs.end(),delete_object());
        for_each(vs.begin(),vs.end(),delete_object());
    } catch (char *str) {
        std::cerr<<str<<std::endl;
        for(Rectangles::iterator r=rs.begin();r!=rs.end();++r) {
            std::cerr << **r <<std::endl;
        }
    }
    COLA_ASSERT(noRectangleOverlaps(rs));
}


bool noRectangleOverlaps(const Rectangles& rs) 
{
    Rectangle *u, *v;
    Rectangles::const_iterator i=rs.begin(), j, e=rs.end();
    for (;i!=e;++i) 
    {
        u=*i;
        for (j=i+1;j!=e;++j) 
        {
            v=*j;
            if (u->overlapX(v)>0) 
            {
                COLA_ASSERT(u->overlapY(v)==0);
            }
        }
    }
    return true;
}

// checks if line segment is strictly overlapping.
// That is, if any point on the line is inside the rectangle.
bool Rectangle::overlaps(double x1, double y1, double x2, double y2) 
{
    RectangleIntersections ri;
    lineIntersections(x1,y1,x2,y2,ri);
    if(ri.intersects) {
        if(ri.countIntersections()==1) {
            // special case when one point is touching
            // the boundary of the rectangle but no part
            // of the line is interior
            if(!inside(x1,y1)&&!inside(x2,y2)) {
                return false;
            }
        }
        printf("Rectangle/Segment intersection (SVG):\n");
        printf("<svg style=\"stroke: black; fill: none;\">\n");
        printf("<polyline points=\"%f,%f %f,%f\" />\n",x1,y1,x2,y2);
        printf("<rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" />\n",
                getMinX(),getMinY(),width(),height());
        printf("</svg>\n");
        ri.printIntersections();
        return true;
    }
    return false;
}


void RectangleIntersections::printIntersections() 
{
    printf("intersections:\n");
    if(top) printf("  top=%d:(%f,%f)\n",top,topX,topY);
    if(bottom) printf("  bottom=%d:(%f,%f)\n",bottom,bottomX,bottomY);
    if(left) printf("  left=%d:(%f,%f)\n",left,leftX,leftY);
    if(right) printf("  right=%d:(%f,%f)\n",right,rightX,rightY);
}

// Of the stored intersections, this returns the one closest to the
// specified point
void RectangleIntersections::nearest(double x, double y, double &xi, double &yi) {
    bool is[]={top, right, bottom, left};
    double xs[]={topX, rightX, bottomX, leftX};
    double ys[]={topY, rightY, bottomY, leftY};
    double dx, dy, l, minl = 999999999999999.0;
    for(unsigned i=0;i<4;i++) 
    {
        if(is[i]) 
        {
            dx=xs[i]-x;
            dy=ys[i]-y;
            l=dx*dx + dy*dy;
            if(l<minl) 
            {
                minl=l;
                xi=xs[i];
                yi=ys[i];
            }
        }
    }
}

}
