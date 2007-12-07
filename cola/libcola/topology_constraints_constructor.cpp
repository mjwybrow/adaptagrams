/**
 * Everything in here is related to creating a topology_constraints instance.
 * The main complexity here is the definition of structures representing events
 * in a plane scan algorithm for generating topology constraints between
 * rectangles and line segments. 
 * \file topology_constraints_constructor.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 */
#include "topology_constraints.h"
#include "cola.h"
#include "straightener.h"
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
     * We use this to find neighbouring rectangles so that we can generate
     * non-overlap constraints between them
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
        /**
         * print is useful in debugging
         */
        virtual void print()=0;
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
     * at node openings the node is placed in the list of #openNodes and a topology constraint
     * is created for one side of the node rectangle and every open segment.
     */
    struct NodeOpen : NodeEvent {
        /// position in openNodes
        OpenNodes::iterator openListIndex;
        NodeOpen(Node const *node) 
            : NodeEvent(true,node->rect->getMinD(!dim),node) {
        }
        void process() {
            print();
            pair<OpenNodes::iterator,bool> r =
                openNodes.insert(make_pair(node->rect->getCentreD(dim),this));
            // the following test fails if there is already an entry in
            // openNodes at this position
            assert(r.second);
            openListIndex = r.first;
            createTopologyConstraint();
        }
        void print() {
            printf("NodeOpen: pos=%f\n",pos);
        }
    };
    /**
     * at node closings the node is removed from the list of #openNodes and topology constraints
     * are created for the remaining side of the node rectangle and every open segment.
     */
    struct NodeClose : NodeEvent {
        /** we store the opening corresponding to this closing so that we can delete it and
         *  remove it from the list of OpenNodes.
         */
        NodeOpen* opening;
        NodeClose(Node* node, NodeOpen* o) :
            NodeEvent(false,node->rect->getMaxD(!dim),node), opening(o) {
            assert(opening->node == node);
        }
        /**
         * remove opening from openNodes, cleanup, and generate TopologyConstraints.
         */
        void process() {
            print();
            openNodes.erase(opening->openListIndex);
            delete opening;
            // create topology constraint from scanpos in every open edge to node
            createTopologyConstraint();
            delete this;
        }
        void print() {
            printf("NodeClose: pos=%f\n",pos);
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
            print();
            openListIndex=openSegments.insert(openSegments.end(),this);
        }
        void print() {
            printf("SegmentOpen: pos=%f\n",pos);
            //s->print();
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
            print();
            OpenSegments::iterator i=openSegments.erase(opening->openListIndex);
            delete opening;
            delete this;
        }
        void print() {
            printf("SegmentClose: pos=%f\n",pos);
        }
    };
    /** 
     * Create topology constraint from scanpos in every open segment to node.
     * Segments must not be on-top-of rectangles.
     */
    void NodeEvent::createTopologyConstraint() {
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
        EdgePoint *u=s->start, *v=s->end;
        // segments orthogonal to scan direction need no constraints
        assert(v->pos[!dim]-u->pos[!dim]!=0);

        vpsc::Rectangle* r=node->rect;
        // segment must overlap in the scan dimension with the rectangle
        assert(!(
                    u->pos[!dim]>=r->getMaxD(!dim)
                  &&v->pos[!dim]>=r->getMaxD(!dim)
                ));
        assert(!(
                    u->pos[!dim]<=r->getMinD(!dim)
                  &&v->pos[!dim]<=r->getMinD(!dim)
                ));
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
    BendConstraint::BendConstraint(EdgePoint* v) 
        : bendPoint(v) {
            printf("creating BendConstraint\n");
        // we only consider articulation points (not first or last points
        // in edge)
        assert(!v->isReal());
        EdgePoint* u=v->inSegment->start, * w=v->outSegment->end;
        // because all of our nodes are boxes we do not expect consecutive
        // segments to change horizontal or vertical direction
        assert(u->pos[!dim]<v->pos[!dim]&&v->pos[!dim]<=w->pos[!dim]
            || u->pos[!dim]>v->pos[!dim]&&v->pos[!dim]>=w->pos[!dim]);
        double p;
        double i=v->inSegment->intersection(w->pos[!dim],p);
        double g=w->offset()+u->offset()+p*(v->offset()-u->offset());
        bool leftOf = w->pos[dim] < i;
        c = new TriConstraint(
            u->node->var,v->node->var,w->node->var,p,g,leftOf);
    }
    struct CompareEvents {
        bool operator() (Event *const &a, Event *const &b) const {
            if(a->pos < b->pos) {
                return true;
            } else if(a->pos==b->pos) {
#ifndef NDEBUG
                SegmentEvent* sa = dynamic_cast<SegmentEvent*>(a);
                SegmentEvent* sb = dynamic_cast<SegmentEvent*>(b);
                if(sa&&sb) {
                    // events should not have been generated for 
                    // segments parallel to scan line
                    assert(sa->s!=sb->s);
                }
#endif
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
    struct constructEventsAndBendConstraints {
        constructEventsAndBendConstraints(vector<Event*>& events)
            : events(events) {}
        void operator() (Segment* s) {
            // don't generate events for segments parallel to scan line
            if(s->start->pos[!dim]!=s->end->pos[!dim]) {
                // if it's a bend point then create a TopologyConstraint
                // that becomes active when the bend straightens
                if(!s->end->isReal()) {
                    s->end->bendConstraint = new BendConstraint(s->end);
                }
                SegmentOpen *open=new SegmentOpen(s);
                SegmentClose *close=new SegmentClose(s,open);
                events.push_back(open);
                events.push_back(close);
            }
        }
        vector<Event*>& events;
    };
    TopologyConstraints::TopologyConstraints( const cola::Dim axisDim,
            Nodes const & nodes,
            Edges const & edges) : edges(edges) {

        dim = axisDim;
        printf("Generating topology constraints! (dim=%d)\n",dim);

        vector<Event*> events;
        unsigned ctr=0;
        // scan vertically to create horizontal topology constraints
        // place Segment opening/closing and Rectangle opening/closing into event queue
        for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
            Node* v=*i;
            vpsc::Rectangle* r=v->rect;
            printf("v%d={%f,%f,%f,%f}\n",ctr++,r->getMinX(),r->getMinY(),r->width(),r->height());
            NodeOpen *open=new NodeOpen(v);
            NodeClose *close=new NodeClose(v,open);
            events.push_back(open);
            events.push_back(close);
        }
        for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
            (*e)->forEachSegment(
                    constructEventsAndBendConstraints(events));
        }
        // process events in top to bottom order
        std::sort(events.begin(),events.end(),CompareEvents());
        for_each(events.begin(),events.end(),mem_fun(&Event::process));
        assert(openSegments.empty());
        assert(openNodes.empty());
    }
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
