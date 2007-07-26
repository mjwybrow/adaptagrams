/**
 * \brief Functions to automatically generate constraints for the
 * rectangular node overlap removal problem.
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */

#include "commondefs.h"

#include <set>
#include <list>
#include <cassert>
#include <iostream>
#include <cmath>
#include "cola.h"
#include "compound_constraints.h"
#include "straightener.h"

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
        if(fabs(tx-ty)<0.001 && tx>0 && tx<=1) {
            return true;
        }
        return false;
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
                double px=nodes[*j]->x;
                double py=nodes[*j]->y;
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
        assert(ds.empty());
    }
    void Edge::createRouteFromPath(std::vector<Node *> const & nodes) {
        Route* r=new Route(path.size());
#ifdef STRAIGHTENER_DEBUG
        printf("Route:");
#endif
        for(unsigned i=0;i<path.size();i++) {
            r->xs[i]=nodes[path[i]]->x;
            r->ys[i]=nodes[path[i]]->y;
#ifdef STRAIGHTENER_DEBUG
            printf("(%f,%f)",r->xs[i],r->ys[i]);
#endif
        }
#ifdef STRAIGHTENER_DEBUG
        printf("\n");
#endif
        setRoute(r);
    }

    typedef enum {Open, Close} EventType;
    struct Event {
        EventType type;
        Node *v;
        Edge *e;
        double pos;
        Event(EventType t, Node *v, double p) : type(t),v(v),e(NULL),pos(p) {};
        Event(EventType t, Edge *e, double p) : type(t),v(NULL),e(e),pos(p) {};
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
     * new dummy nodes for each edge intersected).
     * The new dummy nodes are also added to the end of the canonical
     * node list: nodes.
     */
    void sortNeighbours(const cola::Dim dim, Node * v, Node * l, Node * r, 
            const double conjpos, vector<Edge*> const & openEdges, 
            vector<Node *>& L,vector<Node *>& nodes) {
        double minpos=-DBL_MAX, maxpos=DBL_MAX;
        if(l!=NULL) {
            L.push_back(l);
            minpos=l->scanpos;
        }
        typedef pair<double,Edge*> PosEdgePair;
        set<PosEdgePair> sortedEdges;
        for(unsigned i=0;i<openEdges.size();i++) {
            Edge *e=openEdges[i];
            vector<double> bs;
            if(dim==cola::HORIZONTAL) {
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
            //if(l!=NULL&&(e->startNode==l->id||e->endNode==l->id)) continue;
            //cerr << "edge("<<e->startNode<<","<<e->endNode<<",pts="<<e->pts<<")"<<endl;
            // here, we probably want to search for existing dummy 
            // nodes associated with the same edge within some
            // range of (pos,conjpos), rather than creating new ones.
            // Would require some sort of quad-tree structure
            Node* d=dim==cola::HORIZONTAL?
                new Node(nodes.size(),pos,conjpos,e):
                new Node(nodes.size(),conjpos,pos,e);
            L.push_back(d);
            nodes.push_back(d);
        }
        L.push_back(v);

        if(r!=NULL) {
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
            //if(r!=NULL&&(e->startNode==r->id||e->endNode==r->id)) continue;
            //cerr << "edge("<<e->startNode<<","<<e->endNode<<",pts="<<e->pts<<")"<<endl;
            Node* d=dim==cola::HORIZONTAL?
                new Node(nodes.size(),pos,conjpos,e):
                new Node(nodes.size(),conjpos,pos,e);
            L.push_back(d);
            nodes.push_back(d);
        }
        if(r!=NULL) {
            L.push_back(r);
        }
    }
    static double xOverlap(Node const *u, Node const *v) {
		if (u->x <= v->x && v->xmin < u->xmax)
			return u->xmax - v->xmin;
		if (v->x <= u->x && u->xmin < v->xmax)
			return v->xmax - u->xmin;
		return 0;
	}
	static double yOverlap(Node const *u, Node const *v) {
		if (u->y <= v->y && v->ymin < u->ymax)
			return u->ymax - v->ymin;
		if (v->y <= u->y && u->ymin < v->ymax)
			return v->ymax - u->ymin;
		return 0;
	}

    static cola::SeparationConstraint* createConstraint(
            Node* u, Node* v, cola::Dim dim) {
        double g=dim==cola::HORIZONTAL?(u->width+v->width):(u->height+v->height);
        g/=2;
        double sep=dim==cola::HORIZONTAL?(v->x-u->x):(v->y-u->y);
        if(sep < g) {
            u->active = true;
            v->active = true;
        }
        //cerr << "Constraint: "<< u->id << "+"<<g<<"<="<<v->id<<endl;
        return new cola::SeparationConstraint(u->id,v->id,g);
    }

    template <typename T>
    Event* createEvent(
            const cola::Dim dim, 
            const EventType type,
            T *v,
            double border) {
        double pos = dim==cola::HORIZONTAL ? (
            type==Open ? v->ymin-border
                       : v->ymax+border
            ) : (
            type==Open ? v->xmin-border
                       : v->xmax+border
            );
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
            const cola::Dim dim, 
            vector<Node*> & nodes, 
            vector<Edge*> const & edges, 
            vector<cola::SeparationConstraint*>& cs,
            bool xSkipping = true) {
        vector<Event*> events;
        double nodeFudge=-2, edgeFudge=0;
#ifdef STRAIGHTENER_DEBUG
        cout << dim==cola::HORIZONTAL
            ?"scanning top to bottom..."
            :"scanning left to right..."
            << endl;
#endif
        for(unsigned i=0;i<nodes.size();i++) {
            Node *v=nodes[i];
            if(v->scan) {
                v->scanpos=dim==cola::HORIZONTAL?v->x:v->y;
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
            if(v!=NULL) {
                v->open = true;
#ifdef STRAIGHTENER_DEBUG
                printf("NEvent@%f,nid=%d,(%f,%f),w=%f,h=%f,openn=%d,opene=%d\n",e->pos,v->id,v->x,v->y,v->width,v->height,(int)openNodes.size(),(int)openEdges.size());
#endif
                Node *l=NULL, *r=NULL;
                if(!openNodes.empty()) {
                    // it points to the first node to the right of v
                    NodeSet::iterator it=openNodes.lower_bound(v);
                    // step left to find the first node to the left of v
                    while(it--!=openNodes.begin()) {
                        if(!xSkipping
                                || dim!=cola::HORIZONTAL
                                || xOverlap(*it,v) <= 0
                                || xOverlap(*it,v) <= yOverlap(*it,v)) {
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
                                || dim!=cola::HORIZONTAL
                                || xOverlap(v,*it) <= 0
                                || xOverlap(v,*it) <= yOverlap(v,*it)) {
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
                
                // Case A: create constraints between adjacent edges skipping edges joined
                // to l,v or r.
                Node* lastNode=NULL;
                for(vector<Node*>::iterator i=L.begin();i!=L.end();i++) {
                    if((*i)->dummy) {
                        // node is on an edge
                        Edge *edge=(*i)->edge;
                        if(!edge->isEnd(v->id)
                                &&(l!=NULL&&!edge->isEnd(l->id)||l==NULL)
                                &&(r!=NULL&&!edge->isEnd(r->id)||r==NULL)) {
                            if(lastNode!=NULL) {
#ifdef STRAIGHTENER_DEBUG
                                printf("  Rule A: Constraint: v%d +g <= v%d\n",lastNode->id,(*i)->id);
                                edge->debugLines.push_back(DebugLine(
                                            lastNode->x,lastNode->y,
                                            (*i)->x,(*i)->y,
                                            0
                                            ));
#endif
                                cs.push_back(createConstraint(lastNode,*i,dim));
                            }
                            lastNode=*i;
                        }
                    } else {
                        // is an actual node
                        lastNode=NULL;
                    }
                }
                // Case B: create constraints for all the edges connected to the right of
                // their own end, also in the scan line
                vector<Node*> skipList;
                lastNode=NULL;
                for(vector<Node*>::iterator i=L.begin();i!=L.end();i++) {
                    if((*i)->dummy) {
                        // node is on an edge
                        if(lastNode!=NULL) {
                            if((*i)->edge->isEnd(lastNode->id)) {
                                skipList.push_back(*i);
                            } else {
                                for(vector<Node*>::iterator j=skipList.begin();
                                        j!=skipList.end();j++) {
#ifdef STRAIGHTENER_DEBUG
                                    printf("  Rule B: Constraint: v%d +g <= v%d\n",(*j)->id,(*i)->id);
                                    (*i)->edge->debugLines.push_back(DebugLine(
                                                (*j)->x,(*j)->y,
                                                (*i)->x,(*i)->y,
                                                1
                                                ));
#endif
                                    cs.push_back(createConstraint(*j,*i,dim));
                                }
                                skipList.clear();
                            }
                        }
                    } else {
                        // is an actual node
                        skipList.clear();
                        skipList.push_back(*i);
                        lastNode=*i;
                    }
                }
                skipList.clear();
                // Case C: reverse of B
                lastNode=NULL;
                for(vector<Node*>::reverse_iterator i=L.rbegin();i!=L.rend();i++) {
                    if((*i)->dummy) {
                        // node is on an edge
                        if(lastNode!=NULL) {
                            if((*i)->edge->isEnd(lastNode->id)) {
                                skipList.push_back(*i);
                            } else {
                                for(vector<Node*>::iterator j=skipList.begin();
                                        j!=skipList.end();j++) {
#ifdef STRAIGHTENER_DEBUG
                                    printf("  Rule C: Constraint: v%d +g <= v%d\n",(*i)->id,(*j)->id);
                                    (*i)->edge->debugLines.push_back(DebugLine(
                                                (*i)->x,(*i)->y,
                                                (*j)->x,(*j)->y,
                                                2
                                                ));
#endif
                                    cs.push_back(createConstraint(*i,*j,dim));
                                }
                                skipList.clear();
                            }
                        }
                    } else {
                        // is an actual node
                        skipList.clear();
                        skipList.push_back(*i);
                        lastNode=*i;
                    }
                }
                if(e->type==Close) {
                    if(l!=NULL) cs.push_back(createConstraint(l,v,dim));
                    if(r!=NULL) cs.push_back(createConstraint(v,r,dim));
                }
            }
            if(e->type==Open) {
                if(v!=NULL) {
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
                if(v!=NULL) {
                    openNodes.erase(v);
                    v->open=false;
                } else {
#ifdef STRAIGHTENER_DEBUG
                    printf("EdgeClose@%f,eid=%d,(u,v)=(%d,%d)\n", e->pos,e->e->id,e->e->startNode,e->e->endNode);
#endif                    
                    unsigned i=e->e->openInd;
                    assert(openEdges.size()>0);
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
		    const cola::Dim dim,
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
                for(unsigned i=0;i<c->nodes.size();i++) {
                    straightener::Node* u = nodes[c->nodes[i]];
                    sc->scanpos+=dim==cola::HORIZONTAL?u->x:u->y;
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
            n+=(*e)->route->n;
        }
        colaCluster->hullX.resize(n);
        colaCluster->hullY.resize(n);
        unsigned i=0;
        for(std::vector<Edge*>::const_iterator e=boundary.begin();
                e!=boundary.end();e++) {
            Route* r=(*e)->route;
            for(unsigned j=0;j<r->n;j++) {
                colaCluster->hullX[i]=r->xs[j];
                colaCluster->hullY[i++]=r->ys[j];
            }
        }
    }
    Straightener::Straightener(
            const cola::Dim dim,
            std::vector<vpsc::Rectangle*> const & rs,
            std::vector<Edge*> const & edges, 
            cola::Variables const & vs,
            cola::Variables & lvs,
            cola::Constraints & lcs) 
        : dim(dim),
          edges(edges), 
          vs(vs), 
          lvs(lvs) 
    {
        for (unsigned i=0;i<rs.size();i++) {
            nodes.push_back(new straightener::Node(i,rs[i]));
        }
        vector<cola::SeparationConstraint*> cs;
        straightener::generateConstraints(dim,nodes,edges,cs);
        // after generateConstraints we have new dummy nodes at the end of snodes and
        // constraints in cs
        //   need to create variables for dummy nodes in lvs and constraints in lcs
        for (unsigned i=rs.size();i<nodes.size();i++) {
            double desiredPos = (dim==cola::HORIZONTAL)?nodes[i]->x:nodes[i]->y;
            lvs.push_back(new vpsc::Variable(i,desiredPos,1));
        }
        for (vector<cola::SeparationConstraint*>::iterator i=cs.begin();i!=cs.end();i++) {
            unsigned lv=(*i)->left;
            unsigned rv=(*i)->right;
            double g=(*i)->gap;
            vpsc::Variable* l = lv<rs.size()?vs[lv]:lvs[lv-rs.size()];
            vpsc::Variable* r = rv<rs.size()?vs[rv]:lvs[rv-rs.size()];
            lcs.push_back(new vpsc::Constraint(l,r,g));
        }
        for(unsigned i=0;i<edges.size();i++) {
            edges[i]->nodePath(nodes,false);
        }
    }
    Straightener::~Straightener() {
        for_each(nodes.begin(),nodes.end(),delete_object());
    }
    void Straightener::applyForces() {
        for(unsigned i=0;i<edges.size();i++) {
            //printf("Straightening path:\n");
            vector<unsigned>& path=edges[i]->path;
            for(unsigned j=1;j<path.size();j++) {
                unsigned u=path[j-1], v=path[j];
                double x1=nodes[u]->x, x2=nodes[v]->x,
                       y1=nodes[u]->y, y2=nodes[v]->y;
                double dx=x1-x2, dy=y1-y2;
                double dx2=dx*dx, dy2=dy*dy;
                double l=sqrt(dx2+dy2);
                double f=dim==cola::HORIZONTAL?dx:dy;
                if(l>0.0001) {
                    f/=l;
                } else {
                    continue;
                }
                vpsc::Variable *var1=u<vs.size()?vs[u]:lvs[u-vs.size()];
                vpsc::Variable *var2=v<vs.size()?vs[v]:lvs[v-vs.size()];
                var1->desiredPosition-=f;
                var2->desiredPosition+=f;
                //printf("  (%d,%d)=((%f,%f),(%f,%f)):f=%f\n",u,v,x1,y1,x2,y2,f);
            }
        }
    }
    double Straightener::computeStress() {
        double stress=0;
        for(unsigned i=0;i<edges.size();i++) {
            Route* r=edges[i]->route;
            for(unsigned j=1;j<r->n;j++) {
                double x1=r->xs[j-1], x2=r->xs[j],
                       y1=r->ys[j-1], y2=r->ys[j];
                double dx=x1-x2, dy=y1-y2;
                double dx2=dx*dx, dy2=dy*dy;
                double l=sqrt(dx2+dy2);
                stress+=l;
            }
        }
        return stress;
    }
    void Straightener::updateNodePositions() {
        // real nodes
        for (unsigned i=0;i<vs.size();i++) {
            double pos = vs[i]->finalPosition;
            Node *n=nodes[i];
            if(dim==cola::HORIZONTAL) {
                n->x=pos;
            } else {
                n->y=pos;
            }
        }
        // dummy bend nodes
        //printf("got %d dummy nodes\n", (int) lvs.size());
        dummyNodesX.resize(lvs.size());
        dummyNodesY.resize(lvs.size());
        for (unsigned i=0;i<lvs.size();i++) {
            assert(i+vs.size() < nodes.size());
            double pos = lvs[i]->finalPosition;
            Node *n=nodes[i+vs.size()];
            if(dim==cola::HORIZONTAL) {
                n->x=pos;
                dummyNodesX[i]=pos;
                dummyNodesY[i]=n->y;
            } else {
                n->y=pos;
                dummyNodesX[i]=n->x;
                dummyNodesY[i]=pos;
            }
        }
        for(unsigned i=0;i<edges.size();i++) {
            edges[i]->createRouteFromPath(nodes);
            edges[i]->dummyNodes.clear();
            edges[i]->path.clear();
        }
    }
}

// vim: cindent ts=4 sw=4 et tw=0 wm=0
