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

#include <set>
#include <list>
#include <cassert>
#include "straightener.h"
#include <iostream>
#include <cmath>
#include "cola.h"

using std::list;
using std::make_pair;
using std::pair;
using std::set;
using std::vector;

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
     * activePath list is also set up with a subset of nodes from path, each of
     * which is active (a start/end node or involved in a violated constraint).
     */
    void Edge::nodePath(vector<Node*>& nodes) {
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
                if(nodes[j->second]->active) {
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

    typedef enum {Open, Close} EventType;
    struct Event {
        EventType type;
        Node *v;
        Edge *e;
        double pos;
        Event(EventType t, Node *v, double p) : type(t),v(v),e(NULL),pos(p) {};
        Event(EventType t, Edge *e, double p) : type(t),v(NULL),e(e),pos(p) {};
    };
    Event **events;
    int compare_events(const void *a, const void *b) {
        Event *ea=*(Event**)a;
        Event *eb=*(Event**)b;
        if(ea->v!=NULL&&ea->v==eb->v||ea->e!=NULL&&ea->e==eb->e) {
            // when comparing opening and closing from object
            // open must come first
            if(ea->type==Open) return -1;
            return 1;
        } else if(ea->pos > eb->pos) {
            return 1;
        } else if(ea->pos < eb->pos) {
            return -1;
        }	
        return 0;
    }

    /**
     * Search along scan line at conjpos for open edges to the left of v
     * as far as l, and to the right of v as far as r.
     * The result is a list of nodes L (including l,v,r and a bunch of
     * new dummy nodes for each edge intersected).
     * The new dummy nodes are also added to the end of the canonical
     * node list: nodes.
     */
    void sortNeighbours(const Dim dim, Node * v, Node * l, Node * r, 
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
            if(dim==HORIZONTAL) {
                e->xpos(conjpos,bs);
            } else {
                e->ypos(conjpos,bs);
            }
            //cerr << "edge(intersections="<<bs.size()<<":("<<e->startNode<<","<<e->endNode<<"))"<<endl;
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
            Node* d=dim==HORIZONTAL?
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
            Node* d=dim==HORIZONTAL?
                new Node(nodes.size(),pos,conjpos,e):
                new Node(nodes.size(),conjpos,pos,e);
            L.push_back(d);
            nodes.push_back(d);
        }
        if(r!=NULL) {
            L.push_back(r);
        }
    }
    static SimpleConstraint* createConstraint(Node* u, Node* v, Dim dim) {
        double g=dim==HORIZONTAL?(u->width+v->width):(u->height+v->height);
        g/=2;
        double sep=dim==HORIZONTAL?(v->x-u->x):(v->y-u->y);
        if(sep < g) {
            u->active = true;
            v->active = true;
        }
        //cerr << "Constraint: "<< u->id << "+"<<g<<"<="<<v->id<<endl;
        return new SimpleConstraint(u->id,v->id,g);
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
            const Dim dim, 
            vector<Node*> & nodes, 
            vector<Edge*> & edges, 
            vector<SimpleConstraint*>& cs) {
        unsigned nevents=2*nodes.size()+2*edges.size();
        events=new Event*[nevents];
        unsigned ctr=0;
        double nodeFudge=0.01, edgeFudge=-1;
        if(dim==HORIZONTAL) {
            //cout << "Scanning top to bottom..." << endl;
            for(unsigned i=0;i<nodes.size();i++) {
                Node *v=nodes[i];
                if(v->scan) {
                    v->scanpos=v->x;
                    events[ctr++]=new Event(Open,v,v->ymin+nodeFudge);
                    events[ctr++]=new Event(Close,v,v->ymax-nodeFudge);
                } else {
                    nevents -= 2;
                }
            }
            for(unsigned i=0;i<edges.size();i++) {
                Edge *e=edges[i];
                events[ctr++]=new Event(Open,e,e->ymin+edgeFudge);
                events[ctr++]=new Event(Close,e,e->ymax-edgeFudge);
            }
        } else {
            //cout << "Scanning left to right..." << endl;
            for(unsigned i=0;i<nodes.size();i++) {
                Node *v=nodes[i];
                if(v->scan) {
                    v->scanpos=v->y;
                    events[ctr++]=new Event(Open,v,v->xmin+nodeFudge);
                    events[ctr++]=new Event(Close,v,v->xmax-nodeFudge);
                } else {
                    nevents -= 2;
                }
            }
            for(unsigned i=0;i<edges.size();i++) {
                Edge *e=edges[i];
                events[ctr++]=new Event(Open,e,e->xmin+edgeFudge);
                events[ctr++]=new Event(Close,e,e->xmax-edgeFudge);
            }
        }
        qsort((Event*)events, (size_t)nevents, sizeof(Event*), compare_events );

        NodeSet openNodes;
        vector<Edge*> openEdges;
        for(unsigned i=0;i<nevents;i++) {
            Event *e=events[i];
            Node *v=e->v;
            if(v!=NULL) {
                v->open = true;
                //printf("NEvent@%f,nid=%d,(%f,%f),w=%f,h=%f,openn=%d,opene=%d\n",e->pos,v->id,v->x,v->y,v->width,v->height,(int)openNodes.size(),(int)openEdges.size());
                Node *l=NULL, *r=NULL;
                if(!openNodes.empty()) {
                    // it points to the first node to the right of v
                    NodeSet::iterator it=openNodes.lower_bound(v);
                    // step left to find the first node to the left of v
                    if(it--!=openNodes.begin()) {
                        l=*it;
                        //printf("l=%d\n",l->id);
                    }
                    it=openNodes.upper_bound(v);
                    if(it!=openNodes.end()) {
                        r=*it;
                    }
                }
                vector<Node*> L;
                sortNeighbours(dim,v,l,r,e->pos,openEdges,L,nodes);
                /*printf("L=[");
                for(unsigned i=0;i<L.size();i++) {
                    printf("%d ",L[i]->id);
                }
                printf("]\n");
                */
                
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
                                //printf("  Rule A: Constraint: v%d +g <= v%d\n",lastNode->id,(*i)->id);
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
                                    //printf("  Rule B: Constraint: v%d +g <= v%d\n",(*j)->id,(*i)->id);
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
                                    //printf("  Rule C: Constraint: v%d +g <= v%d\n",(*i)->id,(*j)->id);
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
                    //printf("EdgeOpen@%f,eid=%d,(u,v)=(%d,%d)\n", e->pos,e->e->id,e->e->startNode,e->e->endNode);
                    e->e->openInd=openEdges.size();
                    openEdges.push_back(e->e);
                }
            } else {
                // Close
                if(v!=NULL) {
                    openNodes.erase(v);
                    v->open=false;
                } else {
                    //printf("EdgeClose@%f,eid=%d,(u,v)=(%d,%d)\n", e->pos,e->e->id,e->e->startNode,e->e->endNode);
                    unsigned i=e->e->openInd;
                    openEdges[i]=openEdges[openEdges.size()-1];
                    openEdges[i]->openInd=i;
                    openEdges.resize(openEdges.size()-1);
                }
            }
            delete e;
        }
        delete [] events;
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
		    const Dim dim,
		    vector<straightener::Node*> & nodes,
            vector<straightener::Edge*> & edges,
            vector<vpsc::Rectangle*> const & rs,
		    cola::Clusters const & clusters,
		    vector<straightener::Cluster*>& sclusters) {
        sclusters.clear();
        for(cola::Clusters::const_iterator c=clusters.begin();
                c!=clusters.end(); c++) {
            straightener::Cluster* sc=new straightener::Cluster;
            // compute scanpos based on average position in scan direction
            sc->scanpos=0;
            for(unsigned i=0;i<(*c)->nodes.size();i++) {
                straightener::Node* u = nodes[(*c)->nodes[i]];
                sc->scanpos+=dim==HORIZONTAL?u->x:u->y;
                u->cluster = sc;
            }
            sc->scanpos/=(*c)->nodes.size();
            sclusters.push_back(sc);
            (*c)->computeBoundary(rs,sc->hullX,sc->hullY);
            // create a chain of dummy nodes for the boundary
            Node* first = new Node(nodes.size(),sc->hullX[0],sc->hullY[0]);
            nodes.push_back(first);
            Node* u = first;
            unsigned i=1;
            for(;i<sc->hullX.size();i++) {
                Node* v = new Node(nodes.size(),sc->hullX[i],sc->hullY[i]);
                nodes.push_back(v);
                Edge* e = new Edge(edges.size(),u->id,v->id,
                            sc->hullX[i-1],sc->hullY[i-1],sc->hullX[i],sc->hullY[i]);
                edges.push_back(e);
                sc->boundary.push_back(e);
                u=v;
            }
            edges.push_back(
                    new Edge(edges.size(),u->id,first->id,
                        sc->hullX[i-1],sc->hullY[i-1],sc->hullX[0],sc->hullY[0]));
        }
    }
}

// vim: cindent ts=4 sw=4 et tw=0 wm=0
