/**
 * Everything in here is related to creating a topology_constraints instance.
 * The main complexity here is the definition of structures representing events
 * in a plane scan algorithm for generating topology constraints between
 * rectangles and line segments. 
 * \file topology_constraints_constructor.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 */
#include <libproject/project.h>
#include <libcola/cola.h>
#include <libcola/straightener.h>
#include "log.h"
#include "topology_graph.h"
#include "topology_constraints.h"
using namespace std;
using vpsc::Rectangle;
namespace topology {
cola::Dim dim;
struct SegmentOpen;
struct NodeOpen;
typedef list<SegmentOpen*> OpenSegments;
/**
 * open segments are scanned on node openings and closings to create
 * topology constraints between the node and each open segment
 */
OpenSegments openSegments; 
typedef map<double,NodeOpen*> OpenNodes;
/** 
 * open nodes are stored in a map keyed on position along scan line.
 * We use this to find neighbouring rectangles at a NodeClose event
 * so that we can generate non-overlap constraints between the closing
 * node and its immediate neighbours.
 * Note that this assumes no overlaps between rectangles.
 */
OpenNodes openNodes;

/**
 * The scan algorithm works by processing events in the order they are encountered
 * by the scan line.
 */
struct Event {
    /// an event is either an opening or closing of some object
    bool open;
    /// the position of the scan line at which the event is triggered
    double pos;
    Event(bool open, double pos) : open(open), pos(pos) {}
    virtual ~Event() {};
    /**
     * process is called for each event in pos order as part of the scan
     * algorithm to generate topology constraints.
     */
    virtual void process()=0;
};
/**
 * There is a NodeEvent for the top and bottom (or left and right sides depending on
 * scan direction) of the rectangle associated with each node
 */
struct NodeEvent : Event {
    Node const *node;
    NodeEvent(bool open, double pos, Node const *v)
        : Event(open,pos), node(v) {
    }
    ~NodeEvent(){}
    /**
     * topology constraints are generated for the opening and closing edges of each node
     * and every open segment at pos
     */
    void createTopologyConstraint();
};
/**
 * at node openings the node is placed in the list of #openNodes and a
 * topology constraint is created for one side of the node rectangle and
 * every open segment.
 */
struct NodeOpen : NodeEvent {
    /// position in openNodes
    OpenNodes::iterator openListIndex;
    NodeOpen(Node const *node) 
        : NodeEvent(true,node->rect->getMinD(!dim),node) {
    }
    void process() {
        pair<OpenNodes::iterator,bool> r =
            openNodes.insert(make_pair(node->rect->getCentreD(dim),this));
        // the following test fails if there is already an entry in
        // openNodes at this position
        assert(r.second);
        openListIndex = r.first;
        createTopologyConstraint();
    }
};
/**
 * at node closings the node is removed from the list of #openNodes and
 * topology constraints are created for the remaining side of the node
 * rectangle and every open segment.  Also, non-overlap constraints
 * are created between the node and its immediate neighbours in openNodes.
 */
struct NodeClose : NodeEvent {
    /** we store the opening corresponding to this closing so that we can delete it and
     *  remove it from the list of OpenNodes.
     */
    NodeOpen* opening;
    project::Constraints& cs;
    NodeClose(Node* node, NodeOpen* o, project::Constraints& cs)
        : NodeEvent(false,node->rect->getMaxD(!dim),node)
        , opening(o)
        , cs(cs) {
        assert(opening->node == node);
    }
    void createNonOverlapConstraint(const Node* left, const Node* right) {
        FILE_LOG(logDEBUG)<<"NodeClose::createNonOverlapConstraint left="<<left<<" right="<<right;
        double overlap = left->rect->overlapD(!dim,right->rect);
        if(overlap>1e-5) {
            double g = left->rect->length(dim) + right->rect->length(dim);
            g/=2.0;
            cs.push_back(new project::Constraint(left->var, right->var, g));
        }
    }
    /**
     * remove opening from openNodes, cleanup, and generate
     * TopologyConstraints.
     */
    void process() {
        OpenNodes::iterator nodePos=opening->openListIndex;
        OpenNodes::iterator right=nodePos, left=nodePos;
        if(left--!=openNodes.begin()) {
            createNonOverlapConstraint(left->second->node,node);
        }
        if((++right)!=openNodes.end()) {
            createNonOverlapConstraint(node,right->second->node);
        }
        openNodes.erase(nodePos);
        delete opening;
        // create topology constraint from scanpos in every open edge to node
        createTopologyConstraint();
        delete this;
    }
};
/**
 * Segment events occur at each end of a segment
 */
struct SegmentEvent : Event {
    Segment *s;
    SegmentEvent(bool open, EdgePoint* v, Segment *s)
        : Event(open,v->pos[!dim]), s(s) {}
};
/**
 * at a segment open we add the segment to the list of open segments
 */
struct SegmentOpen : SegmentEvent {
    /// position in openSegments
    OpenSegments::iterator openListIndex;
    SegmentOpen(Segment *s) 
        : SegmentEvent(true,s->getMin(),s) {}
    /// add to list of open segments
    void process() {
        openListIndex=openSegments.insert(openSegments.end(),this);
    }
};
/**
 * at a segment closing we remove the segment from the list of openings and cleanup
 */
struct SegmentClose : SegmentEvent {
    /// opening corresponding to this closing
    SegmentOpen* opening;
    SegmentClose(Segment *s, SegmentOpen* so)
        : SegmentEvent(false,s->getMax(),s), opening(so) 
    {
        assert(opening->s==s);
    }
    void process() {
        OpenSegments::iterator i=openSegments.erase(opening->openListIndex);
        delete opening;
        delete this;
    }
};
/** 
 * Create topology constraint from scanpos in every open segment to node.
 * Segments must not be on-top-of rectangles.
 */
void NodeEvent::createTopologyConstraint() {
    FILE_LOG(logDEBUG)<<"NodeEvent::createTopologyConstraint():node@"<<node<<" pos="<<pos;
    for(OpenSegments::iterator j=openSegments.begin(); j!=openSegments.end();++j) {
        Segment* s=(*j)->s;
        // skip if edge is attached to this node
        if(s->edge->firstSegment->start->node==node 
                || s->edge->lastSegment->end->node==node) {
            continue;
        }
        // segments orthogonal to scan direction need no constraints
        if(s->start->pos[!dim]-s->end->pos[!dim]==0) continue;

        s->straightConstraints.push_back(new StraightConstraint(
                    s,node,pos));
    }
}
/**
 * create a constraint between a segment and a node that is
 * activated when the segment needs to be bent (divided into
 * two new segments
 * @param s the segment
 * @param node the node
 * @param pos the position in !dim (i.e. position of scan line) at
 * which to create the constraint
 */
StraightConstraint::StraightConstraint(
        Segment* s, 
        const Node* node,
        const double pos) 
    : segment(s), node(node), pos(pos)
{
    FILE_LOG(logDEBUG)<<"StraightConstraint ctor: pos="<<pos;
    EdgePoint *u=s->start, *v=s->end;
    // segments orthogonal to scan direction need no constraints
    assert(v->pos[!dim]-u->pos[!dim]!=0);

    vpsc::Rectangle* r=node->rect;
    FILE_LOG(logDEBUG1)<<"Segment: from "<<u->pos[!dim]<<" to "<<v->pos[!dim];
    FILE_LOG(logDEBUG1)<<"Node: rect "<<*r;
    // segment must overlap in the scan dimension with the rectangle
    assert(min(v->pos[!dim],u->pos[!dim])<=pos);
    assert(max(v->pos[!dim],u->pos[!dim])>=pos);
    assert(r->getMinD(!dim)<=pos);
    assert(r->getMaxD(!dim)>=pos);
    // determine direction of constraint based on intersection
    // of segment with scan line
    bool passLeft=false;
    double p;
    if(s->intersection(pos,p) < node->rect->getCentreD(dim)) {
        passLeft=true;
    } 
    ri=pos < node->rect->getCentreD(!dim)
         ? passLeft ? EdgePoint::BL : EdgePoint::BR
         : passLeft ? EdgePoint::TL : EdgePoint::TR;
    double g=u->offset()+p*(v->offset()-u->offset());
    if(passLeft) {
        g+=r->length(dim)/2.0;
    } else {
        g-=r->length(dim)/2.0;
    }
    c=new TriConstraint(
                u->node->var,v->node->var,node->var,p,g,passLeft);
}
/**
 * create a constraint between the two segments joined by this
 * EdgePoint such that the constraint is activated when the segments
 * are aligned.
 * @param bendPoint the articulation point
 */
BendConstraint::
BendConstraint(EdgePoint* v) 
    : bendPoint(v) 
{
    FILE_LOG(logDEBUG)<<"BendConstraint ctor, pos="<<v->pos[!dim];
    EdgePoint* u=v->inSegment->start, * w=v->outSegment->end;
    // because all of our nodes are boxes we do not expect consecutive
    // segments to change horizontal or vertical direction
    FILE_LOG(logDEBUG1)<<"u="<<u->pos[!dim]<<" v="<<v->pos[!dim]<<" w="<<w->pos[!dim];
    FLUSH_LOG(logDEBUG1);
    assert(u->pos[!dim]<v->pos[!dim]&&v->pos[!dim]<=w->pos[!dim]
        || u->pos[!dim]>v->pos[!dim]&&v->pos[!dim]>=w->pos[!dim]);
    double p;
    double i=v->inSegment->intersection(w->pos[!dim],p);
    double g=u->offset()+p*(v->offset()-u->offset())-w->offset();
    bool leftOf = w->pos[dim] < i;
    c = new TriConstraint(
        u->node->var,v->node->var,w->node->var,p,g,leftOf);
}
struct CompareEvents {
    bool operator() (Event *const &a, Event *const &b) const {
        if(a->pos < b->pos) {
            return true;
        } else if(a->pos==b->pos) {
            // note: Segments orthogonal to scan direction (i.e.
            // OpenPos=ClosePos) can still have node events between the
            // segment open and close... need to handle elsewhere 
            // Let closes come before opens when at the same position
            if(!a->open && b->open) return true;
            if(a->open && !b->open) return false;
            // Segment opens at the same position as node opens, node
            // comes first
            if(a->open && b->open) {
                if(dynamic_cast<SegmentOpen*>(b) 
                   && dynamic_cast<NodeOpen*>(a)) return true;
                if(dynamic_cast<SegmentOpen*>(a) 
                   && dynamic_cast<NodeOpen*>(b)) return false;
            }
            // Segment closes at the same position as node closes, segment
            // comes first
            if(!a->open && !b->open) {
                if(dynamic_cast<SegmentClose*>(a) &&
                   dynamic_cast<NodeClose*>(b)) return true;
                if(dynamic_cast<SegmentClose*>(b) &&
                   dynamic_cast<NodeClose*>(a)) return false;
            }
        }
        return false;
    }
};
struct createBendConstraints {
    void operator() (EdgePoint* p) {
        Segment* in = p->inSegment, * out = p->outSegment;
        // don't generate events for segments parallel to scan line
        // or points at the end of an edge
        if(in!=NULL && out!=NULL
           && in->start->pos[!dim]!=in->end->pos[!dim]
           && out->start->pos[!dim]!=out->end->pos[!dim] ) {
            // if it's a valid bend point then create a TopologyConstraint
            // that becomes active when the bend straightens
            p->bendConstraint = new BendConstraint(p);
        }
    }
};
struct createSegmentEvents {
    createSegmentEvents(vector<Event*>& events) : events(events) {}
    void operator() (Segment* s) {
        // don't generate events for segments parallel to scan line
        if(s->start->pos[!dim]!=s->end->pos[!dim]) {
            SegmentOpen *open=new SegmentOpen(s);
            SegmentClose *close=new SegmentClose(s,open);
            events.push_back(open);
            events.push_back(close);
        }
    }
    vector<Event*>& events;
};

TopologyConstraints::
TopologyConstraints( 
    const cola::Dim axisDim,
    Nodes& nodes,
    Edges& edges,
    project::Constraints & cs
) : n(nodes.size())
  , nodes(nodes)
  , edges(edges) 
  , cs(cs)
{
    FILELog::ReportingLevel() = logDEBUG1;

    dim = axisDim;

    vector<Event*> events;

    // scan vertically to create horizontal topology constraints
    // place Segment opening/closing and Rectangle opening/closing into event queue
    for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
        Node* v=*i;
        NodeOpen *open=new NodeOpen(v);
        NodeClose *close=new NodeClose(v,open,cs);
        events.push_back(open);
        events.push_back(close);
    }
    for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
        (*e)->forEach(createBendConstraints(),createSegmentEvents(events));
    }
    // process events in top to bottom order
    std::sort(events.begin(),events.end(),CompareEvents());
    for_each(events.begin(),events.end(),mem_fun(&Event::process));
    assert(openSegments.empty());
    assert(openNodes.empty());
}

TopologyConstraints::
~TopologyConstraints() {
    for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
        (*e)->forEach(mem_fun(&EdgePoint::deleteBendConstraint),
                mem_fun(&Segment::deleteStraightConstraints));
    }
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
