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

#ifndef STRAIGHTENER_H
#define STRAIGHTENER_H

#include <set>
#include <cfloat>
#include <iostream>
#include <iterator>

#include "libvpsc/rectangle.h"
#include "libcola/commondefs.h"

namespace cola {
    class Cluster;
    class ConvexCluster;
    class SeparationConstraint;
}
namespace straightener {

struct Route {
    Route(unsigned n) : n(n), xs(new double[n]), ys(new double[n]) {}
    ~Route() {
        delete [] xs;
        delete [] ys;
    }
    void print() {
        std::cout << "double xs[]={";
        std::copy(xs,xs+n-1,std::ostream_iterator<double>(std::cout,","));
        std::cout << xs[n-1] << "};" << std::endl << "double ys[]={";
        std::copy(ys,ys+n-1,std::ostream_iterator<double>(std::cout,","));
        std::cout << ys[n-1] << "};" << std::endl;
    }
    void boundingBox(double &xmin,double &ymin,double &xmax,double &ymax) {
        xmin=ymin=DBL_MAX;
        xmax=ymax=-DBL_MAX;
        for(unsigned i=0;i<n;i++) {
            xmin=std::min(xmin,xs[i]);
            xmax=std::max(xmax,xs[i]);
            ymin=std::min(ymin,ys[i]);
            ymax=std::max(ymax,ys[i]);
        } 
    }
    double routeLength () const {
        double length=0;
        for(unsigned i=1;i<n;i++) {
            double dx=xs[i-1]-xs[i];
            double dy=ys[i-1]-ys[i];
            length+=sqrt(dx*dx+dy*dy);
        }
        return length;
    }
    void rerouteAround(vpsc::Rectangle *rect);
    unsigned n;
    double *xs;
    double *ys;
};
class Node;
struct DebugPoint {
    double x,y;
};
struct DebugLine {
    DebugLine(double x0,double y0,double x1,double y1,unsigned colour) 
        : x0(x0),y0(y0),x1(x1),y1(y1),colour(colour) {}
    double x0,y0,x1,y1;
    unsigned colour;
};
class ScanObject {
public:
    const unsigned id;
    double getMin(vpsc::Dim d) const {
        return min[d];
    }
    double getMax(vpsc::Dim d) const {
        return max[d];
    }
    ScanObject(unsigned id) : id(id) {}
protected:
    double min[2], max[2];
};
class Edge : public ScanObject {
public:
    unsigned openInd; // position in openEdges
    unsigned startNode, endNode;
    double idealLength;
    std::vector<unsigned> dummyNodes;
    std::vector<unsigned> path;
    std::vector<unsigned> activePath;
    std::vector<DebugPoint> debugPoints;
    std::vector<DebugLine> debugLines;
    void print() {
        printf("Edge[%d]=(%d,%d)\n",id,startNode,endNode);
        route->print();
    }
    // if the edge route intersects with any of the rectangles in rects then reroute
    // those parts of the route around the rectangle boundaries
    void rerouteAround(std::vector<vpsc::Rectangle*> const &rects) {
        unsigned rid=0;
        for(std::vector<vpsc::Rectangle*>::const_iterator r=rects.begin();r!=rects.end();r++,rid++) {
            if(rid!=startNode && rid!=endNode) {
                route->rerouteAround(*r);
            }
        }
        updateBoundingBox();
    }
    // Edge with a non-trivial route
    Edge(unsigned id, unsigned start, unsigned end, Route* route)
    : ScanObject(id), startNode(start), endNode(end), route(route)
    {
        updateBoundingBox();
    }
    // Edge with a trivial route
    Edge(unsigned id, unsigned start, unsigned end,
            double x1, double y1, double x2, double y2) 
    : ScanObject(id), startNode(start), endNode(end) {
        route = new Route(2);
        route->xs[0]=x1; route->ys[0]=y1;
        route->xs[1]=x2; route->ys[1]=y2;
        updateBoundingBox();
    }
    ~Edge() {
        delete route;
    }
    bool isEnd(unsigned n) const {
        if(startNode==n||endNode==n) return true;
        return false;
    }
    void nodePath(std::vector<Node*>& nodes, bool allActive);
    void createRouteFromPath(std::vector<Node *> const & nodes);
    void xpos(double y, std::vector<double>& xs) const {
        // search line segments for intersection points with y pos
        for(unsigned i=1;i<route->n;i++) {
            double ax=route->xs[i-1], bx=route->xs[i], ay=route->ys[i-1], by=route->ys[i];
            double r=(y-ay)/(by-ay);
            // as long as y is between ay and by then r>0
            if(r>=0&&r<=1) {
                xs.push_back(ax+(bx-ax)*r);
            }
        }
    }
    void ypos(double x, std::vector<double>& ys) const {
        // search line segments for intersection points with x pos
        for(unsigned i=1;i<route->n;i++) {
            double ax=route->xs[i-1], bx=route->xs[i], ay=route->ys[i-1], by=route->ys[i];
            double r=(x-ax)/(bx-ax);
            // as long as y is between ax and bx then r>0
            if(r>0&&r<=1) {
                ys.push_back(ay+(by-ay)*r);
            }
        }
    }
    Route const * getRoute() const {
        return route;
    }
    void setRoute(Route * r) {
        delete route;
        route=r;
        updateBoundingBox();
    }
private:
    void updateBoundingBox() {
        route->boundingBox(min[0],min[1],max[0],max[1]);
    }
    Route* route;
};
class Straightener {
public:
    Straightener(
            const double strength,
            const vpsc::Dim dim,
            std::vector<vpsc::Rectangle*> const & rs,
            cola::FixedList const & fixed,
            std::vector<Edge*> const & edges, 
            vpsc::Variables const & vs,
            vpsc::Variables & lvs,
            vpsc::Constraints & lcs,
            std::valarray<double> & oldCoords,
            std::valarray<double> & oldG);
    ~Straightener();
    void updateNodePositions();
    void finalizeRoutes();
    void computeForces(cola::SparseMap &H);
    void computeForces2(cola::SparseMap &H);
    double computeStress(std::valarray<double> const &coords);
    double computeStress2(std::valarray<double> const &coords);
    std::valarray<double> dummyNodesX;
    std::valarray<double> dummyNodesY;
    std::valarray<double> g;
    std::valarray<double> coords;
    unsigned N;
private:
    double strength;
    const vpsc::Dim dim;
    cola::FixedList const & fixed;
    std::vector<Edge*> const & edges;
    vpsc::Variables const & vs;
    vpsc::Variables & lvs;
    std::vector<Node*> nodes;
    double len(const unsigned u, const unsigned v, 
            double& dx, double& dy,
            double& dx2, double& dy2);
    double gRule1(const unsigned a, const unsigned b);
    double gRule2(const unsigned a, const unsigned b, const unsigned c);
    double hRuleD1(const unsigned u, const unsigned v, const double sqrtf);
    double hRuleD2(const unsigned u, const unsigned v, const unsigned w, const double sqrtf);
    double hRule2(const unsigned u, const unsigned v, const unsigned w, const double sqrtf);
    double hRule3(const unsigned u, const unsigned v, const unsigned w, const double sqrtf);
    double hRule4(const unsigned a, const unsigned b, const unsigned c, const unsigned d);
    double hRule56(const unsigned u, const unsigned v, 
            const unsigned a, const unsigned b, const unsigned c);
    double hRule7(const unsigned a, const unsigned b, 
            const unsigned c, const unsigned d, const double sqrtf);
    double hRule8(const unsigned u, const unsigned v, const unsigned w,
            const unsigned a, const unsigned b, const unsigned c);
};
class Cluster {
public:
    cola::ConvexCluster* colaCluster;
    Cluster(cola::ConvexCluster* c) : colaCluster(c) {}
    double scanpos;
    std::vector<Edge*> boundary;
    void updateActualBoundary();
};
class Node : public ScanObject {
public:
    Cluster* cluster;
    // Nodes may optionally belong to a cluster.  
    // Neg cluster_id means no cluster - i.e. top-level membership.
    double pos[2];
    double scanpos;
    double length[2];
    Edge* edge;
    bool dummy; // nodes on edge paths (but not ends) are dummy
    bool scan; // triggers scan events
    bool active; // node is active if it is not dummy or is dummy and involved in
                 // a violated constraint
    bool open; // a node is opened (if scan is true) when the scanline first reaches
               // its boundary and closed when the scanline leaves it.
    Node(unsigned id, vpsc::Rectangle const * r) :
        ScanObject(id),cluster(nullptr),
        edge(nullptr),dummy(false),scan(true),active(true),open(false) { 
            for(unsigned i=0;i<2;i++) {
                pos[i]=r->getCentreD(i);
                min[i]=r->getMinD(i);
                max[i]=r->getMaxD(i);
                length[i]=r->length(i);
            }
    }
    Node(unsigned id, const double x, const double y) :
        ScanObject(id),cluster(nullptr),
        edge(nullptr),dummy(false),scan(false),active(true),open(false) {
            pos[vpsc::HORIZONTAL]=x;
            pos[vpsc::VERTICAL]=y;
            for(unsigned i=0;i<2;i++) {
                length[i]=4;
                min[i]=pos[i]-length[i]/2.0;
                max[i]=pos[i]+length[i]/2.0;
            }
    }
    double euclidean_distance(Node const * v) const {
        double dx=pos[0]-v->pos[0];
        double dy=pos[1]-v->pos[1];
        return sqrt(dx*dx+dy*dy);
    }

private:
    friend void sortNeighbours(const vpsc::Dim dim, Node * v, Node * l, Node * r, 
        const double conjpos, std::vector<Edge*> const & openEdges, 
        std::vector<Node *>& L, std::vector<Node *>& nodes);
    Node(const unsigned id, const double x, const double y, Edge* e) : 
        ScanObject(id),cluster(nullptr),
        edge(e),dummy(true),scan(false),active(false)  {
            pos[vpsc::HORIZONTAL]=x;
            pos[vpsc::VERTICAL]=y;
            for(unsigned i=0;i<2;i++) {
                length[i]=4;
                min[i]=pos[i]-length[i]/2.0;
                max[i]=pos[i]+length[i]/2.0;
            }
            e->dummyNodes.push_back(id);
        }
};
struct CmpNodePos {
    bool operator() (const Node* u, const Node* v) const {
        double upos = u->scanpos;
        double vpos = v->scanpos;
        bool tiebreaker = u < v;
        if (u->cluster != v->cluster) {
            if(u->cluster!=nullptr) {
                upos = u->cluster->scanpos;
            }
            if(v->cluster!=nullptr) {
                vpos = v->cluster->scanpos;
            }
            tiebreaker = u->cluster < v->cluster;
        }
        if (upos < vpos) {
            return true;
        }
        if (vpos < upos) {
            return false;
        }
        return tiebreaker;
    }
};
typedef std::set<Node*,CmpNodePos> NodeSet;
// defines references to three variables for which the goal function
// will be altered to prefer points u-b-v are in a linear arrangement
// such that b is placed at u+t(v-u).
struct LinearConstraint {
    LinearConstraint(
            Node const & u, 
            Node const & v,
            Node const & b,
            double w)
        : u(u.id),v(v.id),b(b.id),w(w)
    {
        // from cosine rule: ub.uv/|uv|=|ub|cos(theta)
        double uvx = v.pos[0] - u.pos[0],
               uvy = v.pos[1] - u.pos[1],
               ubx = b.pos[0] - u.pos[0],
               uby = b.pos[1] - u.pos[1],
               duv2 = uvx * uvx + uvy * uvy;
        if(duv2 < 0.0001) {
            t=0;
        } else {
            t = (uvx * ubx + uvy * uby)/duv2;
        }
        duu=(1-t)*(1-t);
        duv=t*(1-t);
        dub=t-1;
        dvv=t*t;
        dvb=-t;
        dbb=1;
        //printf("New LC: t=%f\n",t); 
    }
    unsigned u;
    unsigned v;
    unsigned b;
    double w; // weight
    double t;
    // 2nd partial derivatives of the goal function
    //   (X[b] - (1-t) X[u] - t X[v])^2
    double duu;
    double duv;
    double dub;
    double dvv;
    double dvb;
    double dbb;
};
void setEdgeLengths(double **D, std::vector<Edge*> & edges);
double pathLength(Edge const * e, std::vector<Node*> const & nodes);
double computeStressFromRoutes(double strength, std::vector<Edge*> & edges);
typedef std::vector<LinearConstraint*> LinearConstraints;
void generateConstraints(
        const vpsc::Dim dim, 
        std::vector<Node*> & nodes, 
        std::vector<Edge*> const & edges, 
        std::vector<cola::SeparationConstraint*>& cs,
        bool xSkipping);
void nodePath(Edge& e, std::vector<Node*>& nodes, std::vector<unsigned>& path);
void generateClusterBoundaries(
        const vpsc::Dim dim,
        std::vector<Node*> & nodes,
        std::vector<Edge*> & edges,
        std::vector<vpsc::Rectangle*> const & rs,
        cola::Cluster const & clusterHierarchy,
        std::vector<straightener::Cluster*>& sclusters);

} // namespace straightener
#endif
