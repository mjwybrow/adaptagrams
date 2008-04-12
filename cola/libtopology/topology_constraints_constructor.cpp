/**
 * Everything in here is related to creating a topology_constraints instance.
 * The main complexity here is the definition of structures representing events
 * in a plane scan algorithm for generating topology constraints between
 * rectangles and line segments. 
 * \file topology_constraints_constructor.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 */
#include <libvpsc/constraint.h>
#include <libcola/cola.h>
#include <libcola/straightener.h>
#include "topology_log.h"
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
    virtual string toString()=0;
};
/**
 * There is a NodeEvent for the top and bottom (or left and right sides depending on
 * scan direction) of the rectangle associated with each node
 */
struct NodeEvent : Event {
    Node *node;
    NodeEvent(bool open, double pos, Node *v)
        : Event(open,pos), node(v) {
    }
    ~NodeEvent(){}
    /**
     * topology constraints are generated for the opening and closing edges of each node
     * and every open segment at pos
     */
    void createStraightConstraints(
            const Node* leftNeighbour, const Node* rightNeighbour);
};
/**
 * at node openings the node is placed in the list of #openNodes and a
 * topology constraint is created for one side of the node rectangle and
 * every open segment.
 */
struct NodeOpen : NodeEvent {
    /// position in openNodes
    OpenNodes::iterator openListIndex;
    NodeOpen(Node *node) 
        : NodeEvent(true,node->rect->getMinD(!dim),node) {
    }
    void process() {
        FILE_LOG(logDEBUG) << "NodeOpen::process()";
        pair<OpenNodes::iterator,bool> r =
            openNodes.insert(make_pair(node->rect->getCentreD(dim),this));
        // the following test fails if there is already an entry in
        // openNodes at this position
        //assert(r.second);
        if(!r.second) {
            const Node *n1=node;
            const Node *n2=((r.first)->second)->node;
            printf("scanpos %f, duplicate in open list at position: %f\n",pos,n1->rect->getCentreD(dim));
            printf("  id1=%d, id2=%d\n",n1->id, n2->id);
        }
        assert(r.second);
        openListIndex = r.first;
        OpenNodes::iterator right=openListIndex, left=openListIndex;
        Node *leftNeighbour=NULL, *rightNeighbour=NULL;
        if(left!=openNodes.begin()) {
            leftNeighbour=(--left)->second->node;
        }
        if((++right)!=openNodes.end()) {
            rightNeighbour=right->second->node;
        }
        createStraightConstraints(leftNeighbour,rightNeighbour);
    }
    string toString() {
        stringstream s;
        s<<"NodeOpen@"<<pos;
        return s.str();
    }
};
/**
 * at node closings the node is removed from the list of #openNodes and
 * topology constraints are created for the remaining side of the node
 * rectangle and every open segment.  Also, non-overlap constraints
 * are created between the node and its immediate neighbours in openNodes.
 */
struct NodeClose : NodeEvent {
    /** we store the opening corresponding to this closing so that we can
     * delete it and remove it from the list of OpenNodes.
     */
    NodeOpen* opening;
    vpsc::Constraints& cs;
    NodeClose(Node* node, NodeOpen* o, vpsc::Constraints& cs)
        : NodeEvent(false,node->rect->getMaxD(!dim),node)
        , opening(o)
        , cs(cs) {
        assert(opening->node == node);
    }
    void createNonOverlapConstraint(const Node* left, const Node* right) {
        FILE_LOG(logDEBUG)<<"NodeClose::createNonOverlapConstraint left="<<left<<" right="<<right;
        //double overlap = left->rect->overlapD(!dim,right->rect);
        //if(overlap>1e-5) {
            double g = left->rect->length(dim) + right->rect->length(dim);
            g/=2.0;
            //if(dim==cola::HORIZONTAL) {
                g+=1e-7;
            //}
            //assert(l->getPosition() + g <= r->getPosition());
            cs.push_back(new vpsc::Constraint(left->var, right->var, g));
        //}
    }
    /**
     * remove opening from openNodes, cleanup, and generate
     * TopologyConstraints.
     */
    void process() {
        FILE_LOG(logDEBUG) << "NodeClose::process()";
        OpenNodes::iterator nodePos=opening->openListIndex;
        OpenNodes::iterator right=nodePos, left=nodePos;
        Node *leftNeighbour=NULL, *rightNeighbour=NULL;
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
        createStraightConstraints(leftNeighbour,rightNeighbour);
        delete this;
    }
    string toString() {
        stringstream s;
        s<<"NodeClose@"<<pos;
        return s.str();
    }
};
/**
 * Segment events occur at each end of a segment
 */
struct SegmentEvent : Event {
    Segment *s;
    SegmentEvent(bool open, EdgePoint* v, Segment *s)
        : Event(open,v->pos(!dim)), s(s) {}
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
    string toString() {
        stringstream s;
        s<<"SegmentOpen@"<<pos;
        return s.str();
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
    string toString() {
        stringstream s;
        s<<"SegmentClose@"<<pos;
        return s.str();
    }
};
/** 
 * Create topology constraint from scanpos in every open segment to node.
 * Segments must not be on-top-of rectangles.
 */
void NodeEvent::createStraightConstraints(
        const Node* leftNeighbour, const Node* rightNeighbour) {
    FILE_LOG(logDEBUG)<<"NodeEvent::createStraightConstraints():node->id="<<node->id<<" pos="<<pos;
    const double 
        leftLimit=leftNeighbour?leftNeighbour->rect->getCentreD(dim):-DBL_MAX,
        rightLimit=rightNeighbour?rightNeighbour->rect->getCentreD(dim):DBL_MAX;
    for(OpenSegments::iterator j=openSegments.begin(); j!=openSegments.end();++j) {
        Segment* s=(*j)->s;
        if(s->start->node->id==node->id 
                && s->start->rectIntersect==EdgePoint::CENTRE
        || s->end->node->id==node->id
                && s->end->rectIntersect==EdgePoint::CENTRE) {
            FILE_LOG(logDEBUG1)<<"  Not creating because segment is attached to this node!";
            continue;
        } 
        const double p = s->intersection(pos);
        if(p<leftLimit&&pos>leftNeighbour->rect->getMinD(!dim)&&
                pos<leftNeighbour->rect->getMaxD(!dim)
           ||p>rightLimit&&pos>rightNeighbour->rect->getMinD(!dim)&&
                pos<rightNeighbour->rect->getMaxD(!dim)) { 
            FILE_LOG(logDEBUG1)<<"  Skipping because segment is not visible from this node!";
            continue;
        }
        s->createStraightConstraint(node,pos);
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
            // we don't support zero height/width nodes and events should not
            // have been generated for segments parallel to the scan line.
            // Even so, let opens come before closes when at the same position
            if(!a->open && b->open) return false;
            if(a->open && !b->open) return true;
            // Segment opens at the same position as node opens, segment
            // comes first
            if(a->open && b->open) {
                if(dynamic_cast<SegmentOpen*>(b) 
                   && dynamic_cast<NodeOpen*>(a)) return false;
                if(dynamic_cast<SegmentOpen*>(a) 
                   && dynamic_cast<NodeOpen*>(b)) return true;
            }
            // Segment closes at the same position as node closes, node
            // comes first
            if(!a->open && !b->open) {
                if(dynamic_cast<SegmentClose*>(a) &&
                   dynamic_cast<NodeClose*>(b)) return false;
                if(dynamic_cast<SegmentClose*>(b) &&
                   dynamic_cast<NodeClose*>(a)) return true;
            }
        }
        return false;
    }
};
TriConstraint::TriConstraint(
        const Node *u, 
        const Node *v, 
        const Node *w, 
        double p, double g, bool left)
    : u(u), v(v), w(w), p(p), g(g), leftOf(left) 
{
    assert(assertFeasible());
}

bool Segment::createStraightConstraint(Node* node, double pos) {
    // no straight constraints between a node directly connected by its CENTRE 
    // to this segment.
    assert(!connectedToNode(node));
    const double top = max(end->pos(!dim),start->pos(!dim)), 
                 bottom = min(end->pos(!dim),start->pos(!dim));
    // segments orthogonal to scan direction need no constraints
    FILE_LOG(logDEBUG)<<"Segment::createStraightConstraint, node->id="<<node->id<<", edge->id="<<edge->id<<" pos="<<pos;
    if(top==bottom) {
        FILE_LOG(logDEBUG1)<<"  Not creating because segment is orthogonal to scan direction!";
        return false;
    }
    // segment must overlap in the scan dimension with the potential bend point
    assert(bottom<=pos);
    assert(top>=pos);
    vpsc::Rectangle* r=node->rect;
    FILE_LOG(logDEBUG1)<<"Segment: from "<<start->pos(!dim)<<" to "<<end->pos(!dim);
    FILE_LOG(logDEBUG1)<<"Node: rect "<<*r;
    // determine direction of constraint based on intersection of segment with
    // scan line, i.e. set nodeLeft based on whether the intersection of the
    // potential bend point is to the left or right of the node centre
    double p;
    bool nodeLeft=r->getCentreD(dim) < intersection(pos,p) ;
    // set ri (the vertex of the node rectangle that is to be 
    // kept to the left of the segment
    EdgePoint::RectIntersect ri;
    if(dim==cola::HORIZONTAL) {
        ri=pos < r->getCentreY()
             ? (nodeLeft ? EdgePoint::BR : EdgePoint::BL)
             : (nodeLeft ? EdgePoint::TR : EdgePoint::TL);
    } else {
        ri=pos < r->getCentreX()
             ? (nodeLeft ? EdgePoint::TL : EdgePoint::BL)
             : (nodeLeft ? EdgePoint::TR : EdgePoint::BR);
    }
    if(node->id==start->node->id  && ri==start->rectIntersect) {
        // constraint is redundant because the potential bend point is
        // already a real bend associated with the start EdgePoint of this
        // segment !
        return false;
    }
    if(node->id==end->node->id  && ri==end->rectIntersect) {
        // constraint is redundant - end EdgePoint of this segment!
        return false;
    }
    straightConstraints.push_back(
            new StraightConstraint(this,node,ri,pos,p,nodeLeft));
    return true;
}

/**
 * creates a copy of the StraightConstraint in our own straightConstraints
 * list, but only if this segment is not directly connected to the centre
 * of the StraightConstraint node.
 * @param s the StraightConstraint to be copied across
 */
void Segment::transferStraightConstraint(StraightConstraint* s) {
    if(!connectedToNode(s->node)) {
        createStraightConstraint(s->node,s->pos);
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
        Node* node,
        const EdgePoint::RectIntersect ri,
        const double scanPos,
        const double segmentPos,
        const bool nodeLeft) 
    : segment(s), node(node), ri(ri), pos(scanPos)
{
    FILE_LOG(logDEBUG)<<"StraightConstraint ctor: pos="<<pos<<" edge id="<<s->edge->id<<" node id="<<node->id;
    EdgePoint *u=s->start, *v=s->end;
    
    double g=u->offset()+segmentPos*(v->offset()-u->offset());
    if(nodeLeft) {
        g-=node->rect->length(dim)/2.0;
    } else {
        g+=node->rect->length(dim)/2.0;
    }
    c=new TriConstraint(u->node,v->node,node,segmentPos,g,nodeLeft);
    assertFeasible();
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
    FILE_LOG(logDEBUG)<<"BendConstraint ctor, pos="<<v->pos(!dim);
    assert(v->inSegment!=NULL);
    assert(v->outSegment!=NULL);
    // v must be a bend point around some node
    assert(!v->isEnd());
    assert(v->rectIntersect!=EdgePoint::CENTRE);
    EdgePoint* u=v->inSegment->start, * w=v->outSegment->end;
    // because all of our nodes are boxes we do not expect consecutive
    // segments to change horizontal or vertical direction
    FILE_LOG(logDEBUG1)<<"u: id="<<u->node->id
        <<", ri="<<u->rectIntersect<<", x="<<u->posX()<<", y="<<u->posY();
    FILE_LOG(logDEBUG1)<<"v: id="<<v->node->id
        <<", ri="<<v->rectIntersect<<", x="<<v->posX()<<", y="<<v->posY();
    FILE_LOG(logDEBUG1)<<"w: id="<<w->node->id
        <<", ri="<<w->rectIntersect<<", x="<<w->posX()<<", y="<<w->posY();
    assert(v->assertConvexBend());
    double p;
    /*double i=*/
    v->inSegment->intersection(w->pos(!dim),p);
    double g=u->offset()+p*(v->offset()-u->offset())-w->offset();
    // we could set leftOf=w->pos(dim) < i, but that isn't a good idea
    // when the two segments are already straight!
    // So we choose leftOf based on the rectangle intersect of v
    bool leftOf=false;
    if(dim==cola::HORIZONTAL) {
        if(v->rectIntersect==EdgePoint::TR || v->rectIntersect==EdgePoint::BR) {
            leftOf=true;
        }
    } else {
        if(v->rectIntersect==EdgePoint::TL || v->rectIntersect==EdgePoint::TR) {
            leftOf=true;
        }
    }
    c=new TriConstraint(u->node,v->node,w->node,p,g,leftOf);
    assertFeasible();
}
struct CreateSegmentEvents {
    CreateSegmentEvents(vector<Event*>& events) : events(events) {}
    void operator() (Segment* s) {
        // don't generate events for segments parallel to scan line
        if(s->start->pos(!dim)!=s->end->pos(!dim)) {
            SegmentOpen *open=new SegmentOpen(s);
            SegmentClose *close=new SegmentClose(s,open);
            events.push_back(open);
            events.push_back(close);
        }
    }
    vector<Event*>& events;
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
                assert(u->rect->overlapY(v->rect)<e);
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
    assert(vs.size()==0);
    vs.resize(ns.size());
    transform(ns.begin(),ns.end(),vs.begin(),GetVariable());
}

TopologyConstraints::
TopologyConstraints( 
    const cola::Dim axisDim,
    Nodes& nodes,
    Edges& edges,
    vpsc::Variables& vs,
    vpsc::Constraints& cs
) : n(nodes.size())
  , nodes(nodes)
  , edges(edges) 
  , vs(vs)
  , cs(cs)
{
    FILELog::ReportingLevel() = logERROR;
    //FILELog::ReportingLevel() = logDEBUG1;
    FILE_LOG(logDEBUG)<<"TopologyConstraints::TopologyConstraints():dim="<<axisDim;

    assert(vs.size()>=n);
    assert(noOverlaps());
    assert(assertNoSegmentRectIntersection(nodes,edges));

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
        (*e)->forEach(mem_fun(&EdgePoint::createBendConstraint),
                CreateSegmentEvents(events),true);
    }
    // process events in top to bottom order
    sort(events.begin(),events.end(),CompareEvents());
    for_each(events.begin(),events.end(),mem_fun(&Event::process));
    assert(openSegments.empty());
    assert(openNodes.empty());
    assert(assertFeasible());
    FILE_LOG(logDEBUG)<<"TopologyConstraints::TopologyConstraints()... done.";
}

TopologyConstraints::
~TopologyConstraints() {
    for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
        (*e)->forEach(mem_fun(&EdgePoint::deleteBendConstraint),
                mem_fun(&Segment::deleteStraightConstraints),true);
    }
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
