/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libtopology - Classes used in generating and managing topology constraints.
 *
 * Copyright (C) 2007-2010  Monash University
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
 *             Michael Wybrow
*/

/*
 * Everything in here is related to creating a topology_constraints instance.
 * The main complexity here is the definition of structures representing events
 * in a plane scan algorithm for generating topology constraints between
 * rectangles and line segments. 
 * \file topology_constraints_constructor.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 */
#include "libvpsc/assertions.h"
#include "libvpsc/constraint.h"
#include "libcola/cola.h"
#include "libcola/straightener.h"
#include "libcola/cluster.h"

#include "libtopology/topology_log.h"
#include "libtopology/topology_graph.h"
#include "libtopology/topology_constraints.h"
using namespace std;
using vpsc::Rectangle;
namespace topology {
struct SegmentOpen;
struct NodeOpen;
typedef list<SegmentOpen*> OpenSegments;
typedef map<double,NodeOpen*> OpenNodes;

/*
 * The scan algorithm works by processing events in the order they 
 * are encountered by the scan line.
 */
struct Event {
    // an event is either an opening or closing of some object
    bool open;
    // the position of the scan line at which the event is triggered
    double pos;
    vpsc::Dim scanDim;
    Event(bool open, double pos) : open(open), pos(pos), scanDim(vpsc::UNSET) {}
    virtual ~Event() {};
    /*
     * process is called for each event in pos order as part of the scan
     * algorithm to generate topology constraints.
     */
    virtual void process(OpenNodes& openNodes, OpenSegments& openSegments) = 0;
    virtual string toString() = 0;
};
/*
 * There is a NodeEvent for the top and bottom (or left and right sides depending on
 * scan direction) of the rectangle associated with each node
 */
struct NodeEvent : Event {
    Node *node;
    NodeEvent(bool open, double pos, Node *v)
        : Event(open,pos), node(v)
    {
    }
    ~NodeEvent(){}
    /*
     * Topology constraints are generated for the opening and closing 
     * edges of each node and every open segment at pos.
     */
    void createStraightConstraints(OpenSegments& openSegments,
            const Node *leftNeighbour, const Node *rightNeighbour);
};
/*
 * at node openings the node is placed in the list of #openNodes and a
 * topology constraint is created for one side of the node rectangle and
 * every open segment.
 */
struct NodeOpen : NodeEvent {
    /// position in openNodes
    OpenNodes::iterator openListIndex;
    NodeOpen(vpsc::Dim dim, Node *node)
        : NodeEvent(true,node->rect->getMinD(vpsc::conjugate(dim)),node)
    {
        scanDim = dim;
    }
    void process(OpenNodes& openNodes, OpenSegments& openSegments)
    {
        FILE_LOG(logDEBUG) << "NodeOpen::process()";
        pair<OpenNodes::iterator,bool> r =
            openNodes.insert(make_pair(node->rect->getCentreD(scanDim),this));
        // the following test fails if there is already an entry in
        // openNodes at this position
        //COLA_ASSERT(r.second);
        if(!r.second) {
            const Node *n1=node;
            const Node *n2=((r.first)->second)->node;
            printf("scanpos %f, duplicate in open list at position: %f\n",pos,
                   n1->rect->getCentreD(scanDim));
            printf("  id1=%d, id2=%d\n",n1->id, n2->id);
        }
        COLA_ASSERT(r.second);
        openListIndex = r.first;
        OpenNodes::iterator right=openListIndex, left=openListIndex;
        Node *leftNeighbour=nullptr, *rightNeighbour=nullptr;
        if(left!=openNodes.begin()) {
            leftNeighbour=(--left)->second->node;
        }
        if((++right)!=openNodes.end()) {
            rightNeighbour=right->second->node;
        }
        createStraightConstraints(openSegments, leftNeighbour,rightNeighbour);
    }
    string toString() {
        stringstream s;
        s<<"NodeOpen@"<<pos;
        return s.str();
    }
};
/*
 * at node closings the node is removed from the list of #openNodes and
 * topology constraints are created for the remaining side of the node
 * rectangle and every open segment.  Also, non-overlap constraints
 * are created between the node and its immediate neighbours in openNodes.
 */
struct NodeClose : NodeEvent {
    /* we store the opening corresponding to this closing so that we can
     * delete it and remove it from the list of OpenNodes.
     */
    NodeOpen* opening;
    vpsc::Constraints& cs;
    NodeClose(vpsc::Dim dim, Node* node, NodeOpen* o, vpsc::Constraints& cs)
        : NodeEvent(false,node->rect->getMaxD(vpsc::conjugate(dim)),node)
        , opening(o)
        , cs(cs)
    {
        COLA_ASSERT(opening->node == node);
        scanDim = dim;
    }
    void createNonOverlapConstraint(const Node* left, const Node* right)
    {
        FILE_LOG(logDEBUG)<<"NodeClose::createNonOverlapConstraint left="<<left<<" right="<<right;
        //double overlap = left->rect->overlapD(vpsc::conjugate(scanDim),right->rect);
        //if(overlap>1e-5) {
            double g = left->rect->length(scanDim) + right->rect->length(scanDim);
            g/=2.0;
            //if(scanDim==vpsc::HORIZONTAL) {
                g+=1e-7;
            //}
            //COLA_ASSERT(l->getPosition() + g <= r->getPosition());
            cs.push_back(new vpsc::Constraint(left->var, right->var, g));
        //}
    }
    /*
     * remove opening from openNodes, cleanup, and generate
     * TopologyConstraints.
     */
    void process(OpenNodes& openNodes, OpenSegments& openSegments)
    {
        FILE_LOG(logDEBUG) << "NodeClose::process()";
        OpenNodes::iterator nodePos=opening->openListIndex;
        OpenNodes::iterator right=nodePos, left=nodePos;
        Node *leftNeighbour=nullptr, *rightNeighbour=nullptr;
        if(left!=openNodes.begin()) {
            leftNeighbour=(--left)->second->node;
            createNonOverlapConstraint(leftNeighbour,node);
        }
        if((++right)!=openNodes.end()) {
            rightNeighbour=right->second->node;
            createNonOverlapConstraint(node,rightNeighbour);
        }
        openNodes.erase(nodePos);
        delete opening;
        // create StraightConstraint from scanpos in every open edge 
        // visible before left and right from node
        createStraightConstraints(openSegments, leftNeighbour,rightNeighbour);
        delete this;
    }
    string toString() {
        stringstream s;
        s<<"NodeClose@"<<pos;
        return s.str();
    }
};
/*
 * Segment events occur at each end of a segment
 */
struct SegmentEvent : Event {
    Segment *s;
    SegmentEvent(vpsc::Dim dim, bool open, EdgePoint* v, Segment *s)
        : Event(open,v->pos(vpsc::conjugate(dim))), s(s) {}
};
/*
 * at a segment open we add the segment to the list of open segments
 */
struct SegmentOpen : SegmentEvent {
    /// position in openSegments
    OpenSegments::iterator openListIndex;
    SegmentOpen(vpsc::Dim dim, Segment *s)
        : SegmentEvent(dim, true,s->getMin(dim),s)
    {
        scanDim = dim;
    }
    /// add to list of open segments
    void process(OpenNodes& openNodes, OpenSegments& openSegments)
    {
        COLA_UNUSED(openNodes);

        openListIndex=openSegments.insert(openSegments.end(),this);
    }
    string toString() {
        stringstream s;
        s<<"SegmentOpen@"<<pos;
        return s.str();
    }
};
/*
 * at a segment closing we remove the segment from the list of openings and cleanup
 */
struct SegmentClose : SegmentEvent {
    /// opening corresponding to this closing
    SegmentOpen* opening;
    SegmentClose(vpsc::Dim dim, Segment *s, SegmentOpen* so)
        : SegmentEvent(dim, false,s->getMax(dim),s), opening(so)
    {
        COLA_ASSERT(opening->s==s);
        scanDim = dim;
    }
    void process(OpenNodes& openNodes, OpenSegments& openSegments)
    {
        COLA_UNUSED(openNodes);

        openSegments.erase(opening->openListIndex);
        delete opening;
        delete this;
    }
    string toString() {
        stringstream s;
        s<<"SegmentClose@"<<pos;
        return s.str();
    }
};
/* 
 * Create topology constraint from scanpos in every open segment to node.
 * Segments must not be on-top-of rectangles.
 */
void NodeEvent::createStraightConstraints(OpenSegments& openSegments,
        const Node* leftNeighbour, const Node* rightNeighbour) {
    FILE_LOG(logDEBUG)<<"NodeEvent::createStraightConstraints():node->id="<<node->id<<" pos="<<pos;
    const double 
        leftLimit=leftNeighbour?leftNeighbour->rect->getCentreD(scanDim):-DBL_MAX,
        rightLimit=rightNeighbour?rightNeighbour->rect->getCentreD(scanDim):DBL_MAX;
    for(OpenSegments::iterator j=openSegments.begin(); j!=openSegments.end();++j) {
        Segment* s=(*j)->s;
        if ( (s->start->node->id==node->id 
                && s->start->rectIntersect==EdgePoint::CENTRE)
          || (s->end->node->id==node->id
                && s->end->rectIntersect==EdgePoint::CENTRE) )
        {
            FILE_LOG(logDEBUG1)<<
                    "  Not creating because segment is attached to this node!";
            continue;
        } 
        const double p = s->forwardIntersection(scanDim, pos);
        if ( (p<leftLimit&&pos>leftNeighbour->rect->getMinD(vpsc::conjugate(scanDim))&&
                pos<leftNeighbour->rect->getMaxD(vpsc::conjugate(scanDim)))
          || (p>rightLimit&&pos>rightNeighbour->rect->getMinD(vpsc::conjugate(scanDim))&&
                pos<rightNeighbour->rect->getMaxD(vpsc::conjugate(scanDim))) )
        { 
            FILE_LOG(logDEBUG1)<<
                    "  Skipping because segment is not visible from this node!";
            continue;
        }
        s->createStraightConstraint(scanDim, node,pos);
    }
}
struct CompareEvents {
    bool operator() (Event *const &a, Event *const &b) const {
        if(a==b) {
            // Irreflexivity
            return false;
        }
        if(a->pos < b->pos) {
            return true;
        } else if(a->pos==b->pos) {
            NodeOpen *aNO=dynamic_cast<NodeOpen*>(a),
                     *bNO=dynamic_cast<NodeOpen*>(b);
            NodeClose *aNC=dynamic_cast<NodeClose*>(a),
                      *bNC=dynamic_cast<NodeClose*>(b);
            SegmentOpen *aSO=dynamic_cast<SegmentOpen*>(a),
                     *bSO=dynamic_cast<SegmentOpen*>(b);
            SegmentClose *aSC=dynamic_cast<SegmentClose*>(a),
                      *bSC=dynamic_cast<SegmentClose*>(b);

            // segment opens before closes so that we handle
            // segments parallel to the scan line properly
            if(aSO&&bSC) return true;
            if(aSC&&bSO) return false;
            // Segment closes at same pos as Node opens, Segment first
            if(aSC&&bNO) return true;
            if(bSC&&aNO) return false;
            // Segment opens at the same position as Node closes, Node first
            if(aSO&&bNC) return false;
            if(bSO&&aNC) return true;
            // Segment opens at the same position as node opens, segment
            // comes first
            if(aSO&&bNO) return true;
            if(bSO&&aNO) return false;
            // Segment closes at the same position as node closes, node
            // comes first
            if(aSC&&bNC) return false;
            if(bSC&&aNC) return true;

            // close nodes before we open new ones so we don't generate
            // unnecessary non-overlap constraints
            if(aNO&&bNC) {
                COLA_ASSERT(aNO->node!=bNC->node); // no zero height nodes thanks!
                return false;
            }
            if(aNC&&bNO) {
                COLA_ASSERT(aNC->node!=bNO->node); 
                return true;
            }
        }
        // Transitivity of equivalence
        return false;
    }
};
TriConstraint::TriConstraint(vpsc::Dim dim, const Node *u, const Node *v,
        const Node *w, double p, double g, bool left)
    : u(u), v(v), w(w), p(p), g(g), leftOf(left), scanDim(dim)
{
    COLA_ASSERT(assertFeasible());
}

bool Segment::createStraightConstraint(vpsc::Dim scanDim, Node* node, double pos)
{
    COLA_ASSERT((scanDim==vpsc::XDIM)||(scanDim==vpsc::YDIM));
    // no straight constraints between a node directly connected by its CENTRE 
    // to this segment.
    COLA_ASSERT(!connectedToNode(node));
    const double top = max(end->pos(vpsc::conjugate(scanDim)), start->pos(vpsc::conjugate(scanDim))),
                 bottom = min(end->pos(vpsc::conjugate(scanDim)),start->pos(vpsc::conjugate(scanDim)));
    // segments orthogonal to scan direction need no StraightConstraints
    FILE_LOG(logDEBUG)<<"Segment::createStraightConstraint, node->id="<<node->id<<", edge->id="<<edge->id<<" pos="<<pos;
    if(top==bottom) {
        FILE_LOG(logDEBUG1)<<"  Not creating because segment is orthogonal to scan direction!";
        return false;
    }
    // segment must overlap in the scan dimension with the potential bend point
    //COLA_ASSERT(bottom<=pos);
    //COLA_ASSERT(top>=pos);
    vpsc::Rectangle* r=node->rect;
    FILE_LOG(logDEBUG1)<<"Segment: from {"<<start->pos(scanDim)<<","<<start->pos(vpsc::conjugate(scanDim))<<"},{"<<end->pos(scanDim)<<","<<end->pos(vpsc::conjugate(scanDim))<<"}";
    FILE_LOG(logDEBUG1)<<"Node: rect "<<*r;
    // determine direction of constraint based on intersection of segment with
    // scan line, i.e. set nodeLeft based on whether the intersection of the
    // potential bend point is to the left or right of the node centre
    double p;
    bool nodeLeft=r->getCentreD(scanDim) < forwardIntersection(scanDim, pos,p) ;
    // set ri (the vertex of the node rectangle that is to be 
    // kept to the left of the segment
    EdgePoint::RectIntersect ri;
    if(scanDim==vpsc::HORIZONTAL) {
        ri=pos < r->getCentreY()
             ? (nodeLeft ? EdgePoint::BR : EdgePoint::BL)
             : (nodeLeft ? EdgePoint::TR : EdgePoint::TL);
    } else {
        ri=pos < r->getCentreX()
             ? (nodeLeft ? EdgePoint::TL : EdgePoint::BL)
             : (nodeLeft ? EdgePoint::TR : EdgePoint::BR);
    }
    if(node->id==start->node->id  && ri==start->rectIntersect) {
        FILE_LOG(logDEBUG1)<<"Not creating StraightConstraint because bend point is already a real bend associated with the start EdgePoint of this segment!";
        return false;
    }
    if(node->id==end->node->id  && ri==end->rectIntersect) {
        FILE_LOG(logDEBUG1)<<"Not creating StraightConstraint because bend point is already a real bend associated with the end EdgePoint of this segment!";
        return false;
    }
    straightConstraints.push_back(
            new StraightConstraint(this, scanDim, node, ri, pos, p, nodeLeft));
    return true;
}

/*
 * creates a copy of the StraightConstraint in our own straightConstraints
 * list, but only if this segment is not directly connected to the centre
 * of the StraightConstraint node.
 * @param s the StraightConstraint to be copied across
 */
void Segment::transferStraightConstraint(StraightConstraint* s) {
    if(!connectedToNode(s->node)) {
        createStraightConstraint(s->scanDim, s->node,s->pos);
    }
}
/*
 * create a constraint between a segment and a node that is
 * activated when the segment needs to be bent (divided into
 * two new segments
 * @param s the segment
 * @param node the node
 * @param pos the position in !dim (i.e. position of scan line) at
 * which to create the constraint
 */
StraightConstraint::StraightConstraint(Segment* s, vpsc::Dim dim,
        Node* node,
        const EdgePoint::RectIntersect ri,
        const double scanPos,
        const double segmentPos,
        const bool nodeLeft) 
    : TopologyConstraint(dim), segment(s), node(node), ri(ri), pos(scanPos)
{
    FILE_LOG(logDEBUG)<<"StraightConstraint ctor: pos="<<pos<<" edge id="<<s->edge->id<<" node id="<<node->id;
    EdgePoint *u=s->start, *v=s->end;
    FILE_LOG(logDEBUG1)<<"s->start: id="<<u->node->id
        <<", ri="<<u->rectIntersect<<", x="<<u->posX()<<", y="<<u->posY();
    FILE_LOG(logDEBUG1)<<"node:     id="<<node->id
        <<", ri="<<ri<<", scanpos="<<scanPos;
    FILE_LOG(logDEBUG1)<<"s->end:   id="<<v->node->id
        <<", ri="<<v->rectIntersect<<", x="<<v->posX()<<", y="<<v->posY();
    
    double g=u->offset(scanDim)+segmentPos*(v->offset(scanDim)-u->offset(scanDim));
    if(nodeLeft) {
        g-=node->rect->length(scanDim)/2.0;
    } else {
        g+=node->rect->length(scanDim)/2.0;
    }
    c=new TriConstraint(scanDim, u->node,v->node,node,segmentPos,g,nodeLeft);
    assertFeasible();
}
/*
 * create a constraint between the two segments joined by this
 * EdgePoint such that the constraint is activated when the segments
 * are aligned.
 * @param bendPoint the articulation point
 */
BendConstraint::BendConstraint(EdgePoint* v, vpsc::Dim dim)
    : TopologyConstraint(dim),
      bendPoint(v)
{
    FILE_LOG(logDEBUG)<<"BendConstraint ctor, pos="<<v->pos(vpsc::conjugate(dim));
    COLA_ASSERT(v->inSegment!=nullptr);
    COLA_ASSERT(v->outSegment!=nullptr);
    // v must be a bend point around some node
    COLA_ASSERT(!v->isEnd());
    COLA_ASSERT(v->rectIntersect!=EdgePoint::CENTRE);
    EdgePoint *u=v->inSegment->start, *w=v->outSegment->end;
    COLA_ASSERT(v->assertConvexBend());
    bool leftOf=false;
    if(dim==vpsc::HORIZONTAL) {
        if(v->rectIntersect==EdgePoint::TR || v->rectIntersect==EdgePoint::BR) {
            leftOf=true;
        }
    } else {
        if(v->rectIntersect==EdgePoint::TL || v->rectIntersect==EdgePoint::TR) {
            leftOf=true;
        }
    }
    FILE_LOG(logDEBUG1)<<"u: id="<<u->node->id
        <<", ri="<<u->rectIntersect<<", x="<<u->posX()<<", y="<<u->posY();
    FILE_LOG(logDEBUG1)<<"v: id="<<v->node->id
        <<", ri="<<v->rectIntersect<<", x="<<v->posX()<<", y="<<v->posY();
    FILE_LOG(logDEBUG1)<<"w: id="<<w->node->id
        <<", ri="<<w->rectIntersect<<", x="<<w->posX()<<", y="<<w->posY();
    // bend constraint will be more accurate if the reference segment is the
    // one most orthogonal to scan line.
    double p;
    if(v->inSegment->length(vpsc::conjugate(dim))>v->outSegment->length(vpsc::conjugate(dim))) {
        v->inSegment->forwardIntersection(scanDim, w->pos(vpsc::conjugate(dim)),p);
        double g=u->offset(scanDim)+p*(v->offset(scanDim)-u->offset(scanDim))-w->offset(scanDim);
        c=new TriConstraint(scanDim, u->node,v->node,w->node,p,g,leftOf);
    } else {
        v->outSegment->reverseIntersection(scanDim, u->pos(vpsc::conjugate(dim)),p);
        double g=w->offset(scanDim)+p*(v->offset(scanDim)-w->offset(scanDim))-u->offset(scanDim);
        c=new TriConstraint(scanDim, w->node,v->node,u->node,p,g,leftOf);
        FILE_LOG(logDEBUG1)<<"  Reverse bend constraint!";
    }
    assertFeasible();
}
struct CreateBendConstraints
{
    CreateBendConstraints(vpsc::Dim dim)
        : scanDim(dim)
    { }
    void operator() (EdgePoint *ep)
    {
        ep->createBendConstraint(scanDim);
    }
    vpsc::Dim scanDim;
};
struct CreateSegmentEvents
{
    CreateSegmentEvents(vector<Event*>& events, vpsc::Dim dim)
        : events(events),
          scanDim(dim)
    { }
    void operator() (Segment* s)
    {
        // don't generate events for segments parallel to scan line
        if (s->start->pos(vpsc::conjugate(scanDim)) !=
                s->end->pos(vpsc::conjugate(scanDim)))
        {
            SegmentOpen *open=new SegmentOpen(scanDim, s);
            SegmentClose *close=new SegmentClose(scanDim, s, open);
            events.push_back(open);
            events.push_back(close);
        }
    }
    vector<Event*>& events;
    vpsc::Dim scanDim;
};

bool TopologyConstraints::noOverlaps() const {
    const double e=1e-7;
    for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
        const Node* u=*i;
        for(Nodes::const_iterator j=nodes.begin();j!=nodes.end();++j) {
            const Node* v=*j;
            if(u==v) continue;
            /*
            cout<<"checking overlap ru="<<*u->rect<<" rv="<<*v->rect<<endl;
            cout<<"   overlapX="<<u->rect->overlapX(v->rect)<<endl;
            cout<<"   overlapY="<<u->rect->overlapY(v->rect)<<endl;
            */
            if(u->rect->overlapX(v->rect)>e) {
                COLA_ASSERT(u->rect->overlapY(v->rect)<e);
            }
        }
    }
    return true;
}

struct GetVariable {
    vpsc::Variable* operator() (Node* n) {
        return n->var;
    }
};
void getVariables(Nodes& ns, vpsc::Variables& vs) {
    COLA_ASSERT(vs.size()==0);
    vs.resize(ns.size());
    transform(ns.begin(),ns.end(),vs.begin(),GetVariable());
}
inline bool validTurn(EdgePoint* u, EdgePoint* v, EdgePoint* w) {
    double cpuvw = crossProduct(u->posX(),u->posY(),v->posX(),v->posY(),
            w->posX(),w->posY());
    if(cpuvw==0) { // colinear: can safely remove v
        return true;
    }
    // r is the shape that v turns around
    vpsc::Rectangle* r=v->node->rect;
    double rx = r->getCentreX(), ry = r->getCentreY();
    double cpuvr = crossProduct(u->posX(),u->posY(),v->posX(),v->posY(),rx,ry);
    double cpvwr = crossProduct(v->posX(),v->posY(),w->posX(),w->posY(),rx,ry);
    if(cpuvw*cpuvr>0 && cpuvw*cpvwr>0) {
        return true;
    }
    return false;
}
struct PruneDegenerate {
    PruneDegenerate(vpsc::Dim dim, list<EdgePoint*>& pruneList)
        : pruneList(pruneList),
          scanDim(dim)
    {
    }
    void operator() (EdgePoint* p) {
        if(p->inSegment && p->outSegment) {
            EdgePoint *o=p->inSegment->start, *q=p->outSegment->end;
            double inSegLen = p->inSegment->length(), 
                   outSegLen = p->outSegment->length();
            if(inSegLen>0 && outSegLen>0
                    && o->pos(vpsc::conjugate(scanDim))==p->pos(vpsc::conjugate(scanDim))
                    && p->pos(vpsc::conjugate(scanDim))==q->pos(vpsc::conjugate(scanDim))) {
                FILE_LOG(logDEBUG)<<"EdgePoint collinear in scan dimension!";
                FILE_LOG(logDEBUG)<<"  need to prune";
                pruneList.push_back(p);
            } 
            if(inSegLen==0 && o->inSegment
                    && !validTurn(o->inSegment->start,p,q)) {
                COLA_ASSERT(validTurn(o->inSegment->start,o,q));
                FILE_LOG(logDEBUG)<<"Pruning node after 0 length segment!";
                pruneList.push_back(p);
            } else if(outSegLen==0 && q->outSegment
                    && !validTurn(o,p,q->outSegment->end)) {
                COLA_ASSERT(validTurn(o,q,q->outSegment->end));
                pruneList.push_back(p);
            }
        }
    }
    list<EdgePoint*>& pruneList;
    vpsc::Dim scanDim;
};

static void recCreateTopologyClusterNodes(cola::Cluster *cluster, 
        const vpsc::Dim dim, Nodes& clusterNodes)
{
    if (cluster == nullptr)
    {
        return;
    }

    for (cola::Clusters::iterator i = cluster->clusters.begin(); 
            i != cluster->clusters.end(); ++i)
    {
        recCreateTopologyClusterNodes(*i, dim, clusterNodes);
    }
    
    // ConvexClusters are handled as connectors by the topology layout, but
    // for RectangularClusters we make each side look like a separate node
    // of zero width/height in one dimenstion -- effectively a shape 
    // representing the border on each side of the cluster.
    cola::RectangularCluster *rectCluster = 
            dynamic_cast<cola::RectangularCluster *> (cluster);
    if (rectCluster)
    {
        clusterNodes.push_back(new Node(rectCluster->clusterVarId, 
                rectCluster->getMinEdgeRect(dim),
                (dim == 0) ? rectCluster->vXMin : rectCluster->vYMin));
        clusterNodes.push_back(new Node(rectCluster->clusterVarId + 1, 
                rectCluster->getMaxEdgeRect(dim),
                (dim == 0) ? rectCluster->vXMax : rectCluster->vYMax));
    }
}

TopologyConstraints::TopologyConstraints(const vpsc::Dim axisDim, Nodes& nodes,
        Edges& edges, cola::RootCluster* clusterHierarchy, vpsc::Variables& vs,
        vpsc::Constraints& cs)
    : n(nodes.size()),
      nodes(nodes),
      edges(edges),
      clusters(clusterHierarchy),
      vs(vs),
      cs(cs),
      dim(axisDim)
{
    /*
     * open segments are scanned on node openings and closings to create
     * topology constraints between the node and each open segment
     */
    OpenSegments openSegments;
    /*
     * open nodes are stored in a map keyed on position along scan line.
     * We use this to find neighbouring rectangles at a NodeClose event
     * so that we can generate non-overlap constraints between the closing
     * node and its immediate neighbours.
     * Note that this assumes no overlaps between rectangles.
     */
    OpenNodes openNodes;

    FILELog::ReportingLevel() = logERROR;
    //FILELog::ReportingLevel() = logDEBUG1;
    FILE_LOG(logDEBUG)<<"TopologyConstraints::TopologyConstraints():dim="<<axisDim;
    COLA_ASSERT(vs.size()>=n);
    COLA_ASSERT(noOverlaps());
    COLA_ASSERT(assertNoSegmentRectIntersection(nodes,edges));

    vector<Event*> events;
    
    // We handle rectangular cluster non-overlap by creating two fake shapes 
    // for each cluster with zero width for the left and right side cluster 
    // edges.
    Nodes clusterNodes;
    recCreateTopologyClusterNodes(clusterHierarchy, dim, clusterNodes);
    
    // allNodes is a set of nodes representing topologyNodes and the clusters.
    Nodes allNodes = nodes;
    allNodes.insert(allNodes.end(), clusterNodes.begin(), clusterNodes.end());

    // Scan vertically to create horizontal topology constraints.
    // Place Segment opening/closing and Rectangle opening/closing into 
    // the event queue
    for (Nodes::const_iterator i = allNodes.begin(), e = allNodes.end();
            i != e; ++i)
    {
        Node* v=*i;
        NodeOpen *open=new NodeOpen(dim, v);
        NodeClose *close=new NodeClose(dim, v,open,cs);
        events.push_back(open);
        events.push_back(close);
    }
    list<EdgePoint*> pruneList;
    for(Edges::const_iterator i=edges.begin(),e=edges.end();i!=e;++i) {
        (*i)->forEachEdgePoint(PruneDegenerate(dim,pruneList),true);
    }
    for(list<EdgePoint*>::iterator i=pruneList.begin(), e=pruneList.end();
            i!=e; ++i) {
        (*i)->prune(dim);
    }
    COLA_ASSERT(assertNoZeroLengthEdgeSegments(edges));
    for(Edges::const_iterator i=edges.begin(),e=edges.end();i!=e;++i) {
        (*i)->forEach(CreateBendConstraints(dim),
                CreateSegmentEvents(events, dim),true);
    }
    // process events in top to bottom order
    sort(events.begin(),events.end(),CompareEvents());
    for (vector<Event *>::iterator curr = events.begin();
            curr != events.end(); ++curr)
    {
        (*curr)->process(openNodes, openSegments);
    }
    COLA_ASSERT(openSegments.empty());
    COLA_ASSERT(openNodes.empty());
    COLA_ASSERT(assertFeasible());
    FILE_LOG(logDEBUG)<<"TopologyConstraints::TopologyConstraints()... done.";
    // Delete the temporary clusterNodes.
    for_each(clusterNodes.begin(), clusterNodes.end(), delete_object());
}

TopologyConstraints::
~TopologyConstraints() {
    for(Edges::const_iterator i=edges.begin(),e=edges.end();i!=e;++i) {
        (*i)->forEach(mem_fn(&EdgePoint::deleteBendConstraint),
                mem_fn(&Segment::deleteStraightConstraints),true);
    }
}
} // namespace topology
