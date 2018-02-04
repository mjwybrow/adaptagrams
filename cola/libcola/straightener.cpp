/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
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
 *
*/

/*
 * Functions to automatically generate constraints for the
 * rectangular node overlap removal problem.
 */

#include <set>
#include <list>
#include <cassert>
#include <iostream>
#include <cmath>

#include "libvpsc/assertions.h"
#include "libcola/commondefs.h"
#include "libcola/cola.h"
#include "libcola/compound_constraints.h"
#include "libcola/straightener.h"

//#define STRAIGHTENER_DEBUG 1

using std::list;
using std::make_pair;
using std::pair;
using std::set;
using std::vector;
using std::copy;

namespace straightener {

    // is point p on line a-b?
    static bool pointOnLine(double px,double py, double ax, double ay, double bx, double by, double& tx) {
        double dx=bx-ax;
        double dy=by-ay;
        double ty=0;
        if(fabs(dx)<0.0001&&fabs(dy)<0.0001) {
            // runty line!
            tx=px-ax;
            ty=py-ay;
        } else {
            if(fabs(dx)<0.0001) {
                //vertical line
                if(fabs(px-ax)<0.01) {
                   tx=(py-ay)/dy;
                } 
            } else {
                tx=(px-ax)/dx;
            } 
            if(fabs(dy)<0.0001) {
                //horizontal line
                if(fabs(py-ay)<0.01) {
                   ty=tx;
                } 
            } else {
                ty=(py-ay)/dy;
            }
        }
        //printf("      tx=%f,ty=%f\n",tx,ty);
        if(fabs(tx-ty)<0.001 && tx>=0 && tx<=1) {
            return true;
        }
        return false;
    }
    void Route::rerouteAround(vpsc::Rectangle* rect) {
        // the first and last points should not be inside this
        // rectangle - note that we should not be routing around
        // rectangles directly connected to this route
        COLA_ASSERT(!rect->inside(xs[0],ys[0]));
        COLA_ASSERT(!rect->inside(xs[n-1],ys[n-1]));
        // first, we examine each point and if it is inside the rectangle, we
        // project to the nearest edge of the rectangle
        for(unsigned i=1;i<n-1;i++) {
            double &x=xs[i], &y=ys[i];
            if(rect->inside(x,y)) {
                enum ProjectSide {LEFT, BOTTOM, RIGHT, TOP};
                unsigned projectSide = LEFT;
                double minDist = x - rect->getMinX();
                double dist = y - rect->getMinY();
                if(dist<minDist) {
                    projectSide = BOTTOM;
                    minDist = dist;
                }
                dist = rect->getMaxX() - x;
                if(dist<minDist) {
                    projectSide = RIGHT;
                    minDist = dist;
                }
                dist = rect->getMaxY() - y;
                if(dist<minDist) {
                    projectSide = TOP;
                    minDist = dist;
                }
                switch(projectSide) {
                    case LEFT:
                        x=rect->getMinX();
                        break;
                    case BOTTOM:
                        y=rect->getMinY();
                        break;
                    case RIGHT:
                        x=rect->getMaxX();
                        break;
                    case TOP:
                        y=rect->getMaxY();
                        break;
                }

            }
        }
        // the new route is copied into rxs, rys.
        vector<double> rxs, rys;
        double prevX=xs[0], prevY=ys[0];
        rxs.push_back(prevX);
        rys.push_back(prevY);
    
        // check each segment in turn to see if it intersects
        // with the rectangle.
        // If an intersecting segment is found:
        //   1) the segment passes right through the rectangle
        //      - we insert new segments routed around the rectangle
        //   2) the segment terminates inside the rectangle
        //      - we follow connected segments until we find the exit
        //        point, then we insert a route around the rectangle
        //   3) the segment just touches one side
        //
        for(unsigned i=1;i<n;i++) {
            // we have projected all points to the boundary already so we shouldn't find any inside
            COLA_ASSERT(!rect->inside(xs[i],ys[i]));
            vpsc::RectangleIntersections ri;
            rect->lineIntersections(prevX,prevY,
                    xs[i],ys[i],ri);
            if(ri.intersects) {
                int count=ri.countIntersections();
                COLA_ASSERT(count>0); // can't be 0 because we have detected an intersection
                COLA_ASSERT(count<4); // assumes no zero width or height rects which would be
                                 // the only way for a line segment to touch all 4 sides at once
                if(count==3) { // runs along one side
                    COLA_ASSERT(!rect->inside(xs[i],ys[i]));
                } else if(count==2) { // passes right through
                    COLA_ASSERT(!rect->inside(xs[i],ys[i]));
                    double x1=0, y1=0, x2=0, y2=0;
                    ri.nearest(prevX, prevY, x1, y1);
                    ri.nearest(xs[i], ys[i], x2, y2);
                    rect->routeAround(x1, y1, x2, y2, rxs, rys); 
                } else if(count==1) { 
                    // single intersection, earlier projection step ensures it is on the
                    // perimeter, so nothing to do
                }
            }
            prevX=xs[i];
            prevY=ys[i];
            COLA_ASSERT(!rect->inside(prevX,prevY));
            rxs.push_back(prevX);
            rys.push_back(prevY);
        }
        delete [] xs;
        delete [] ys;
        n=rxs.size();
        COLA_ASSERT(rys.size()==n);
        xs = new double[n];
        ys = new double[n];
        copy(rxs.begin(),rxs.end(),xs);
        copy(rys.begin(),rys.end(),ys);
    }
    /**
     * sets up the path information for an edge,
     * i.e. nodes are added to the path list in the order they appear on the
     * edge, from startNode to endNode.
     * activePath contains at least the first and last node in the edge.
     * If allActive is true then
     * activePath list is also set up with a subset of nodes from path, each of
     * which is active (a start/end node or involved in a violated constraint).
     */
    void Edge::nodePath(vector<Node*>& nodes, bool allActive = true) {
        list<unsigned> ds(dummyNodes.size());
        copy(dummyNodes.begin(),dummyNodes.end(),ds.begin());
        //printf("Edge::nodePath: (%d,%d) dummyNodes:%d\n",startNode,endNode,ds.size());
        path.clear();
        activePath.clear();
        path.push_back(startNode);
        activePath.push_back(0);
        for(unsigned i=1;i<route->n;i++) {
            //printf("  checking segment %d-%d\n",i-1,i);
            set<pair<double,unsigned> > pntsOnLineSegment;
            for(list<unsigned>::iterator j=ds.begin();j!=ds.end();) {
                double px=nodes[*j]->pos[0];
                double py=nodes[*j]->pos[1];
                double ax=route->xs[i-1];
                double ay=route->ys[i-1];
                double bx=route->xs[i];
                double by=route->ys[i];
                double t=0;
                list<unsigned>::iterator copyit=j++;
                //printf("     px=%f, py=%f, ax=%f, ay=%f, bx=%f, by=%f\n",px,py,ax,ay,bx,by);
                if(pointOnLine(px,py,ax,ay,bx,by,t)) {
                    //printf(" got node %d\n",*copyit);
                    pntsOnLineSegment.insert(make_pair(t,*copyit));
                    ds.erase(copyit);
                }
            }
            for(set<pair<double,unsigned> >::iterator j=pntsOnLineSegment.begin();j!=pntsOnLineSegment.end();j++) {
                if(allActive && nodes[j->second]->active) {
                    activePath.push_back(path.size());
                }
                path.push_back(j->second);
            }
            //printf("\n");
        }
        activePath.push_back(path.size());
        path.push_back(endNode);
        COLA_ASSERT(ds.empty());
    }
    void Edge::createRouteFromPath(std::vector<Node *> const & nodes) {
        Route* r=new Route(path.size());
#ifdef STRAIGHTENER_DEBUG
        //printf("Route:");
#endif
        for(unsigned i=0;i<path.size();i++) {
            r->xs[i]=nodes[path[i]]->pos[0];
            r->ys[i]=nodes[path[i]]->pos[1];
#ifdef STRAIGHTENER_DEBUG
            //printf("(%f,%f)",r->xs[i],r->ys[i]);
#endif
        }
#ifdef STRAIGHTENER_DEBUG
        //printf("\n");
#endif
        setRoute(r);
    }

    typedef enum {Open, Close} EventType;
    struct Event {
        EventType type;
        Node *v;
        Edge *e;
        double pos;
        Event(EventType t, Node *v, double p) : type(t),v(v),e(nullptr),pos(p) {};
        Event(EventType t, Edge *e, double p) : type(t),v(nullptr),e(e),pos(p) {};
    };
    /*
     * the following relation defines a strict weak ordering over events, i.e.:
     *   irreflexivity: CompareEvents(e,e) == false
     *   antisymetry:   CompareEvents(a,b) => !CompareEvents(b,a)
     *   transitivity:  CompareEvents(a,b) && CompareEvents(b,c) => CompareEvents(a,c)
     *   transitivity of equivalence:
     *                  !CompareEvents(a,b) && !CompareEvents(b,a) && !CompareEvents(b,c) && !CompareEvents(c,b)
     *               => !CompareEvents(a,c) && !CompareEvents(c,a)
     */
    struct CompareEvents {
        bool operator() (Event *const &a, Event *const &b) const {
            if(a->pos < b->pos) {
                return true;
            } else if(a->pos==b->pos) {
                // All opens should come before closes when at the same position
                if(a->type==Open && b->type==Close) return true;
                if(a->type==Close && b->type==Open) return false;
                // Edge opens at the same position as node opens, edge comes first
                if(a->type==Open && b->type==Open) {
                    if(a->e && b->v) return true;
                    if(b->e && a->v) return false;
                }
                // Edge closes at the same position as node closes, node comes first
                if(a->type==Close && b->type==Close) {
                    if(a->e && b->v) return false;
                    if(b->e && a->v) return true;
                }
            }
            return false;
        }
    };

    /**
     * Search along scan line at conjpos for open edges to the left of v
     * as far as l, and to the right of v as far as r.
     * The result is a list of nodes L (including l,v,r and a bunch of
     * new dummy nodes for each edge intersected - excluding edges
     * connected to v).
     * The new dummy nodes are also added to the end of the canonical
     * node list: nodes.
     */
    void sortNeighbours(const vpsc::Dim dim, Node * v, Node * l, Node * r, 
            const double conjpos, vector<Edge*> const & openEdges, 
            vector<Node *>& L,vector<Node *>& nodes) {
        double minpos=-DBL_MAX, maxpos=DBL_MAX;
        if(l!=nullptr) {
            L.push_back(l);
            minpos=l->scanpos;
        }
        typedef pair<double,Edge*> PosEdgePair;
        set<PosEdgePair> sortedEdges;
        for(unsigned i=0;i<openEdges.size();i++) {
            Edge *e=openEdges[i];
            vector<double> bs;
            if(dim==vpsc::HORIZONTAL) {
                e->xpos(conjpos,bs);
            } else {
                e->ypos(conjpos,bs);
            }
            //std::cerr << "edge(intersections="<<bs.size()<<":("<<e->startNode<<","<<e->endNode<<"))"<<std::endl;
            for(vector<double>::iterator it=bs.begin();it!=bs.end();it++) {
                sortedEdges.insert(make_pair(*it,e));
            }
        }
        for(set<PosEdgePair>::iterator i=sortedEdges.begin();i!=sortedEdges.end();i++) {
            double pos=i->first;
            if(pos < minpos) continue;
            if(pos > v->scanpos) break;
            // if edge is connected (start or end) to v then skip
            // need to record start and end positions of edge segment!
            Edge* e=i->second; 
            if(e->startNode==v->id||e->endNode==v->id) continue;
            //if(l!=nullptr&&(e->startNode==l->id||e->endNode==l->id)) continue;
            //cerr << "edge("<<e->startNode<<","<<e->endNode<<",pts="<<e->pts<<")"<<endl;
            // here, we probably want to search for existing dummy 
            // nodes associated with the same edge within some
            // range of (pos,conjpos), rather than creating new ones.
            // Would require some sort of quad-tree structure
            Node* d=dim==vpsc::HORIZONTAL?
                new Node(nodes.size(),pos,conjpos,e):
                new Node(nodes.size(),conjpos,pos,e);
            L.push_back(d);
            nodes.push_back(d);
        }
        L.push_back(v);

        if(r!=nullptr) {
            maxpos=r->scanpos;
        }
        for(set<PosEdgePair>::iterator i=sortedEdges.begin();i!=sortedEdges.end();i++) {
            if(i->first < v->scanpos) continue;
            if(i->first > maxpos) break;
            double pos=i->first;
            // if edge is connected (start or end) to v then skip
            // need to record start and end positions of edge segment!
            Edge* e=i->second; 
            if(e->startNode==v->id||e->endNode==v->id) continue;
            //if(r!=nullptr&&(e->startNode==r->id||e->endNode==r->id)) continue;
            //cerr << "edge("<<e->startNode<<","<<e->endNode<<",pts="<<e->pts<<")"<<endl;
            Node* d=dim==vpsc::HORIZONTAL?
                new Node(nodes.size(),pos,conjpos,e):
                new Node(nodes.size(),conjpos,pos,e);
            L.push_back(d);
            nodes.push_back(d);
        }
        if(r!=nullptr) {
            L.push_back(r);
        }
    }
    static double overlap(vpsc::Dim k, Node const *u, Node const *v) {
        if (u->pos[k] <= v->pos[k] && v->getMin(k) < u->getMax(k))
            return u->getMax(k) - v->getMin(k);
        if (v->pos[k] <= u->pos[k] && u->getMin(k) < v->getMax(k))
            return v->getMax(k) - u->getMin(k);
        return 0;
    }

    static cola::SeparationConstraint* createConstraint(
            Node* u, Node* v, vpsc::Dim dim) {
        double g=u->length[dim]+v->length[dim];
        g/=2;
        double sep=v->pos[dim]-u->pos[dim];
        if(sep < g) {
            u->active = true;
            v->active = true;
        }
        //cerr << "Constraint: "<< u->id << "+"<<g<<"<="<<v->id<<endl;
        return new cola::SeparationConstraint(dim, u->id, v->id, g);
    }

    template <typename T>
    Event* createEvent(
            const vpsc::Dim dim, 
            const EventType type,
            T *v,
            double border) {
        double pos = (type==Open) ? v->getMin((vpsc::Dim)!dim)-border 
                                  : v->getMax((vpsc::Dim)!dim)+border ;
        return new Event(type,v,pos);
    }
    /**
     * Generates constraints to prevent node/edge and edge/edge intersections.
     * Can be invoked to generate either horizontal or vertical constraints
     * depending on dim parameter.
     * For horizontal constraints, a vertical scan (from top to bottom) is
     * conducted, looking for node/edge boundaries, and then searching along
     * the horizontal limit of that boundary for intersections with other
     * nodes/edges.
     */
    void generateConstraints(
            const vpsc::Dim dim, 
            vector<Node*> & nodes, 
            vector<Edge*> const & edges, 
            vector<cola::SeparationConstraint*>& cs,
            bool xSkipping = true) {
        vector<Event*> events;
        double nodeFudge=-0.01, edgeFudge=0;
#ifdef STRAIGHTENER_DEBUG
        cout << (dim==vpsc::HORIZONTAL
            ?"scanning top to bottom..."
            :"scanning left to right...")
            << endl;
#endif
        for(unsigned i=0;i<nodes.size();i++) {
            Node *v=nodes[i];
            if(v->scan) {
                v->scanpos=v->pos[dim];
                events.push_back(createEvent(dim,Open,v,nodeFudge));
                events.push_back(createEvent(dim,Close,v,nodeFudge));
            }
        }
        for(unsigned i=0;i<edges.size();i++) {
            Edge *e=edges[i];
            events.push_back(createEvent(dim,Open,e,edgeFudge));
            events.push_back(createEvent(dim,Close,e,edgeFudge));
        }
        std::sort(events.begin(),events.end(),CompareEvents());

        NodeSet openNodes;
        vector<Edge*> openEdges;
        // scan opening and closing events in order
        for(unsigned i=0;i<events.size();i++) {
            Event *e=events[i];
            Node *v=e->v;
            if(v!=nullptr) {
                v->open = true;
#ifdef STRAIGHTENER_DEBUG
                printf("NEvent@%f,nid=%d,(%f,%f),w=%f,h=%f,openn=%d,opene=%d\n",e->pos,v->id,v->pos[0],v->pos[1],v->length[0],v->length[1],(int)openNodes.size(),(int)openEdges.size());
#endif
                Node *l=nullptr, *r=nullptr;
                if(!openNodes.empty()) {
                    // it points to the first node to the right of v
                    NodeSet::iterator it=openNodes.lower_bound(v);
                    // step left to find the first node to the left of v
                    while(it--!=openNodes.begin()) {
                        if(!xSkipping
                                || dim!=vpsc::HORIZONTAL
                                || overlap(vpsc::HORIZONTAL,*it,v) <= 0
                                || overlap(vpsc::HORIZONTAL,*it,v) <= overlap(vpsc::VERTICAL,*it,v)) {
                            l=*it;
                            break;
                        }
#ifdef STRAIGHTENER_DEBUG
                        printf("l=%d\n",l->id);
#endif
                    }
                    it=openNodes.upper_bound(v);
                    while(it!=openNodes.end()) {
                        if(!xSkipping
                                || dim!=vpsc::HORIZONTAL
                                || overlap(vpsc::HORIZONTAL,v,*it) <= 0
                                || overlap(vpsc::HORIZONTAL,v,*it) <= overlap(vpsc::VERTICAL,v,*it)) {
                            r=*it;
                            break;
                        }
                        it++;
                    }
                }
                vector<Node*> L;
                sortNeighbours(dim,v,l,r,e->pos,openEdges,L,nodes);
#ifdef STRAIGHTENER_DEBUG
                printf("L=[");
                for(unsigned i=0;i<L.size();i++) {
                    printf("%d ",L[i]->id);
                }
                printf("]\n");
#endif
                // for each dummy node w in L:
                //   if w left of v create constraints l<w, w<v    
                //   if w right of v create constraints v<w, w<r
                for(vector<Node*>::iterator i=L.begin();i!=L.end();i++) {
                    Node* w=*i;
                    if(w->dummy) {
                        // node is on an edge
                        Edge *edge=w->edge;
                        if(w->pos[dim]<v->pos[dim]) { // w left of v
                            if(l!=nullptr&&!edge->isEnd(l->id)) {
                                cs.push_back(createConstraint(l,w,dim));
                            }
                            if(!edge->isEnd(v->id)) {
                                cs.push_back(createConstraint(w,v,dim));
                            }
                        } else { // w right of v
                            if(!edge->isEnd(v->id)) {
                                cs.push_back(createConstraint(v,w,dim));
                            }
                            if(r!=nullptr&&!edge->isEnd(r->id)) {
                                cs.push_back(createConstraint(w,r,dim));
                            }
                        }
                    }
                }
                if(e->type==Close) {
                    if(l!=nullptr) cs.push_back(createConstraint(l,v,dim));
                    if(r!=nullptr) cs.push_back(createConstraint(v,r,dim));
                }
            }
            if(e->type==Open) {
                if(v!=nullptr) {
                    openNodes.insert(v);
                } else {
#ifdef STRAIGHTENER_DEBUG
                    printf("EdgeOpen@%f,eid=%d,(u,v)=(%d,%d)\n", e->pos,e->e->id,e->e->startNode,e->e->endNode);
#endif
                    e->e->openInd=openEdges.size();
                    openEdges.push_back(e->e);
                }
            } else {
                // Close
                if(v!=nullptr) {
                    openNodes.erase(v);
                    v->open=false;
                } else {
#ifdef STRAIGHTENER_DEBUG
                    printf("EdgeClose@%f,eid=%d,(u,v)=(%d,%d)\n", e->pos,e->e->id,e->e->startNode,e->e->endNode);
#endif                    
                    unsigned i=e->e->openInd;
                    COLA_ASSERT(openEdges.size()>0);
                    openEdges[i]=openEdges[openEdges.size()-1];
                    openEdges[i]->openInd=i;
                    openEdges.resize(openEdges.size()-1);
                }
            }
            delete e;
        }
    }
    /**
     * set up straightener clusters.
     * For each cola::cluster c:
     *   create a straightener::cluster sc
     *   set each node in c to belong to sc
     *   set scanpos based on avg pos in scan dir
     *   create a chain of dummy nodes for cluster boundary
     */
    void generateClusterBoundaries(
            const vpsc::Dim dim,
            vector<straightener::Node*> & nodes,
            vector<straightener::Edge*> & edges,
            vector<vpsc::Rectangle*> const & rs,
            cola::Cluster const & clusterHierarchy,
            vector<straightener::Cluster*>& sclusters) {
        sclusters.clear();
        for(vector<cola::Cluster*>::const_iterator i
                =clusterHierarchy.clusters.begin();
                i!=clusterHierarchy.clusters.end(); i++) {
            if(cola::ConvexCluster* c=dynamic_cast<cola::ConvexCluster*>(*i)) {
                straightener::Cluster* sc=new straightener::Cluster(c);
                // compute scanpos based on average position in scan direction
                sc->scanpos=0;
                for(set<unsigned>::iterator it= c->nodes.begin();
                        it != c->nodes.end(); ++it) {
                    straightener::Node* u = nodes[*it];
                    sc->scanpos+=u->pos[dim];
                    u->cluster = sc;
                }
                sc->scanpos/=c->nodes.size();
                sclusters.push_back(sc);
                c->computeBoundary(rs);
                // create a chain of dummy nodes for the boundary
                Node* first = new Node(nodes.size(),c->hullX[0],c->hullY[0]);
                nodes.push_back(first);
                Node* u = first;
                unsigned i=1;
                for(;i<c->hullX.size();i++) {
                    Node* v = new Node(nodes.size(),c->hullX[i],c->hullY[i]);
                    nodes.push_back(v);
                    Edge* e = new Edge(edges.size(),u->id,v->id,
                                c->hullX[i-1],c->hullY[i-1],c->hullX[i],c->hullY[i]);
                    edges.push_back(e);
                    sc->boundary.push_back(e);
                    u=v;
                }
                edges.push_back(
                        new Edge(edges.size(),u->id,first->id,
                            c->hullX[i-1],c->hullY[i-1],c->hullX[0],c->hullY[0]));
            }
        }
    }

    void Cluster::updateActualBoundary()
    {
        unsigned n=0;
        for(std::vector<Edge*>::const_iterator e=boundary.begin();
                e!=boundary.end();e++) {
            n+=(*e)->getRoute()->n;
        }
        colaCluster->hullX.resize(n);
        colaCluster->hullY.resize(n);
        unsigned i=0;
        for(std::vector<Edge*>::const_iterator e=boundary.begin();
                e!=boundary.end();e++) {
            Route const * r=(*e)->getRoute();
            for(unsigned j=0;j<r->n;j++) {
                colaCluster->hullX[i]=r->xs[j];
                colaCluster->hullY[i++]=r->ys[j];
            }
        }
    }
    Straightener::Straightener(
            const double strength,
            const vpsc::Dim dim,
            std::vector<vpsc::Rectangle*> const & rs,
            cola::FixedList const & fixed,
            std::vector<Edge*> const & edges, 
            vpsc::Variables const & vs,
            vpsc::Variables & lvs,
            vpsc::Constraints & lcs,
            std::valarray<double> &oldCoords,
            std::valarray<double> &oldG) 
        : strength(strength),
          dim(dim),
          fixed(fixed),
          edges(edges), 
          vs(vs), 
          lvs(lvs) 
    {
        unsigned n=rs.size();
        for (unsigned i=0;i<n;i++) {
            nodes.push_back(new straightener::Node(i,rs[i]));
        }
        vector<cola::SeparationConstraint*> cs;
        straightener::generateConstraints(dim,nodes,edges,cs);
        // after generateConstraints we have new dummy nodes at the end of snodes and
        // constraints in cs
        //   need to create variables for dummy nodes in lvs and constraints in lcs
        N=nodes.size();
        g.resize(N);
        coords.resize(N);
        for(unsigned i=0;i<n;i++) {
            g[i]=oldG[i];
            coords[i]=oldCoords[i];
        }
        for (unsigned i=n;i<N;i++) {
            double desiredPos = nodes[i]->pos[dim];
            lvs.push_back(new vpsc::Variable(i,desiredPos,1));
            g[i]=0;
            coords[i]=desiredPos;
        }
        for (vector<cola::SeparationConstraint*>::iterator i=cs.begin();i!=cs.end();i++) {
            unsigned lv=(*i)->left();
            unsigned rv=(*i)->right();
            double gap=(*i)->gap;
            vpsc::Variable* l = lv<n?vs[lv]:lvs[lv-n];
            vpsc::Variable* r = rv<n?vs[rv]:lvs[rv-n];
            lcs.push_back(new vpsc::Constraint(l,r,gap));
        }
        for(unsigned i=0;i<edges.size();i++) {
            edges[i]->nodePath(nodes,false);
        }
        for_each(cs.begin(),cs.end(),cola::delete_object());
    }
    Straightener::~Straightener() {
        for_each(nodes.begin(),nodes.end(),cola::delete_object());
    }
    void Straightener::computeForces(cola::SparseMap &H) {
        // hessian matrix:
        //   diagonal: sum dy2/l^3
        //   off-diag: -dy2/l^3
        for(unsigned i=0;i<edges.size();i++) {
            //printf("Straightening path:\n");
            //edges[i]->print();
            vector<unsigned>& path=edges[i]->path;
            COLA_ASSERT(path.size()>0);
            for(unsigned j=1;j<path.size();j++) {
                unsigned u=path[j-1], v=path[j];
                double x1=nodes[u]->pos[0], x2=nodes[v]->pos[0],
                       y1=nodes[u]->pos[1], y2=nodes[v]->pos[1];
                double dx=x1-x2, dy=y1-y2;
                double dx2=dx*dx, dy2=dy*dy;
                double l=sqrt(dx2+dy2);
                if(l<0.0000001) continue;
                double f=dim==vpsc::HORIZONTAL?dx:dy;
                f*=strength/l;
                if(!fixed.check(u)) { g[u]+=f; }
                if(!fixed.check(v)) { g[v]-=f; }
                double h=dim==vpsc::HORIZONTAL?dy2:dx2;
                h*=strength/(l*l*l);
                H(u,u)+=h;
                H(v,v)+=h;
                H(u,v)-=h;
                H(v,u)-=h;
            }
        }
    }
    double Straightener::computeStress(std::valarray<double> const &coords) {
        double stress=0;
        for(unsigned i=0;i<edges.size();i++) {
            vector<unsigned>& path=edges[i]->path;
            COLA_ASSERT(path.size()>0);
            for(unsigned j=1;j<path.size();j++) {
                unsigned u=path[j-1], v=path[j];
                double x1,x2,y1,y2;
                if(dim==vpsc::HORIZONTAL) {
                    x1=coords[u];
                    x2=coords[v];
                    y1=nodes[u]->pos[1];
                    y2=nodes[v]->pos[1];
                } else {
                    x1=nodes[u]->pos[0];
                    x2=nodes[v]->pos[0];
                    y1=coords[u];
                    y2=coords[v];
                }
                double dx=x1-x2, dy=y1-y2;
                double dx2=dx*dx, dy2=dy*dy;
                double l=sqrt(dx2+dy2);
                stress+=l;
            }
        }
        return strength*stress;
    }
    double Straightener::computeStress2(std::valarray<double> const &coords)
    {
        COLA_UNUSED(coords);

        double stress=0;
        for(unsigned i=0;i<edges.size();i++) {
            double d = edges[i]->idealLength;
            double weight=1/(d*d);
            //printf("pathLength=%f\n",pathLength(edges[i],nodes));
            double sqrtf=fabs(d-pathLength(edges[i],nodes));
            stress+=weight*sqrtf*sqrtf;
        }
        return strength*stress;
    }
    void Straightener::updateNodePositions() {
        // real nodes
        for (unsigned i=0;i<N;i++) {
            Node *n=nodes[i];
            n->pos[dim]=coords[i];
        }
        // dummy bend nodes
        //printf("got %d dummy nodes\n", (int) lvs.size());
        dummyNodesX.resize(lvs.size());
        dummyNodesY.resize(lvs.size());
        for (unsigned i=0;i<lvs.size();i++) {
            COLA_ASSERT(i+vs.size() < nodes.size());
            Node *n=nodes[i+vs.size()];
            dummyNodesX[i]=n->pos[0];
            dummyNodesY[i]=n->pos[1];
        }
    }
    void Straightener::finalizeRoutes() {
        for(unsigned i=0;i<edges.size();i++) {
            edges[i]->createRouteFromPath(nodes);
            edges[i]->dummyNodes.clear();
            edges[i]->path.clear();
        }
    }
    void setEdgeLengths(double **D, vector<Edge*> & edges) {
        for(unsigned i=0;i<edges.size();i++) {
            Edge* e=edges[i];
            e->idealLength=D[e->startNode][e->endNode];
        } 
    }
    double pathLength(Edge const * e, vector<Node*> const & nodes) {
        double length=0;
        vector<unsigned> const & path=e->path;
        for(unsigned i=1;i<path.size();i++) {
            Node *u=nodes[path[i-1]], *v=nodes[path[i]];
            double dx=u->pos[0]-v->pos[0];
            double dy=u->pos[1]-v->pos[1];
            length+=sqrt(dx*dx+dy*dy);
        }
        return length;
    }
    double computeStressFromRoutes(double strength, vector<Edge*> & edges) {
        double stress=0;
        for(unsigned i=0;i<edges.size();i++) {
            Edge* e=edges[i];
            double d = e->idealLength;
            double weight=1/(d*d);
            double sqrtf=fabs(d-e->getRoute()->routeLength());
            stress+=weight*sqrtf*sqrtf;
        }
        return strength*stress;
    }
}

