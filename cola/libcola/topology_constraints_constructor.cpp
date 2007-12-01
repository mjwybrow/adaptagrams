#include "topology_constraints.h"
#include "cola.h"
#include "straightener.h"
using namespace std;
using vpsc::Rectangle;
namespace topology {
    cola::Dim dim;
    /** The following structures represent events in a plane scan algorithm
     * for generating topology constraints between rectangles and line segments. 
     *
     *                Event        
     *               /     \               
     *      NodeEvent       SegmentEvent    
     *     /      \            /        \     
     * NodeOpen NodeClose SegmentOpen SegmentClose
     */
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
        virtual void process()=0;
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
        void createTopologyConstraint();
    };
    /**
     * at node openings the node is placed in the list of #openNodes and a topology constraint
     * is created for one side of the node rectangle and every open segment.
     */
    struct NodeOpen : NodeEvent {
        NodeOpen(Node const *node) 
            : NodeEvent(true,node->rect->getMinD(!dim),node) {
        }
        void process() {
            print();
            openNodes[node->rect->getCentreD(dim)]=this;
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
        NodeOpen* opening;
        NodeClose(Node* node) :
            NodeEvent(false,node->rect->getMinD(!dim),node) {}
        void process() {
            print();
            openNodes.erase(node->rect->getCentreD(dim));
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
        OpenSegments::iterator openListIndex;
        SegmentOpen(Segment *s) 
            : SegmentEvent(true,s->getMin(),s) {}
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
     * at a segment closing we remove the segment from the list of openings
     */
    struct SegmentClose : SegmentEvent {
        SegmentOpen* opening;
        SegmentClose(Segment *s, SegmentOpen* so)
            : SegmentEvent(false,s->getMax(),s), opening(so) {}
        void process() {
            print();
            openSegments.erase(opening->openListIndex);
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
        printf("---------createTopologyConstraint()------------------------\n");
        for(OpenSegments::iterator j=openSegments.begin(); j!=openSegments.end();j++) {
            Segment* s=(*j)->s;
            // skip if edge is attached to this node
            if(s->edge->start()->node==node || s->edge->end()->node==node) {
                continue;
            }
            EdgePoint *u=s->start, *v=s->end;
            // segments orthogonal to scan direction need no constraints
            if(v->pos[!dim]-u->pos[!dim]==0) continue;

            vpsc::Rectangle* r=node->rect;
            // segment must overlap in the scan dimension with the rectangle
            assert(!(
                        u->pos[!dim]>r->getMaxD(!dim)
                      &&v->pos[!dim]>r->getMaxD(!dim)
                    ));
            assert(!(
                        u->pos[!dim]<r->getMinD(!dim)
                      &&v->pos[!dim]<r->getMinD(!dim)
                    ));
            // determine direction of constraint based on intersection of segment with scan line
            bool passLeft=false;
            double p;
            if(s->intersection(pos,p) < node->rect->getCentreD(dim)) {
                passLeft=true;
            } 
#ifdef STRAIGHTENER_DEBUG
            if(dim==cola::HORIZONTAL) {
                s->edge->sEdge->debugLines.push_back(
                        straightener::DebugLine( dummyNode->pos[0], pos, intersection, pos, 1));
            } else {
                s->edge->sEdge->debugLines.push_back(
                        straightener::DebugLine( pos, dummyNode->pos[1], pos, intersection, 1));
            }
#endif
            double g=u->offset()+p*(v->offset()-u->offset())+r->length(dim)/2.0;
            s->topologyConstraints.push_back(new TopologyConstraint(
                        u->node->var,v->node->var,node->var,p,g,passLeft));
        }
        printf("---------createTopologyConstraint done!------------------\n");
    }
    struct CompareEvents {
        bool operator() (Event *const &a, Event *const &b) const {
            if(a->pos < b->pos) {
                return true;
            } else if(a->pos==b->pos) {
                // All opens should come before closes when at the same position
                if(a->open && !b->open) return true;
                if(!a->open && b->open) return false;
                // Segment opens at the same position as node opens, segment comes first
                if(a->open && b->open) {
                    if(dynamic_cast<SegmentOpen*>(a) && dynamic_cast<NodeOpen*>(b)) return true;
                    if(dynamic_cast<SegmentOpen*>(b) && dynamic_cast<NodeOpen*>(a)) return false;
                }
                // Segment closes at the same position as node closes, node comes first
                if(!a->open && !b->open) {
                    if(dynamic_cast<SegmentOpen*>(a) && dynamic_cast<NodeOpen*>(b)) return false;
                    if(dynamic_cast<SegmentOpen*>(b) && dynamic_cast<NodeOpen*>(a)) return true;
                }
                // note: Segments orthogonal to scan direction (i.e. OpenPos=ClosePos) can still
                // have node events between the segment open and close... need to handle elsewhere 
            }
            return false;
        }
    };
    TopologyConstraints::TopologyConstraints( const cola::Dim axisDim,
            Nodes const & nodes,
            Edges const & edges) : edges(edges) {

        dim = axisDim;
        printf("Generating topology constraints! (dim=%d)\n",dim);

        vector<Event*> events;
        // scan vertically to create horizontal topology constraints
        // place Segment opening/closing and Rectangle opening/closing into event queue
        for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
            Node* v=*i;
            NodeOpen *open=new NodeOpen(v);
            NodeClose *close=new NodeClose(v);
            events.push_back(open);
            events.push_back(close);
        }
        for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
            for(Segments::iterator si=(*e)->segments.begin();
                    si!=(*e)->segments.end();si++) {
                Segment* s=*si;
                SegmentOpen *open=new SegmentOpen(s);
                SegmentClose *close=new SegmentClose(s,open);
                events.push_back(open);
                events.push_back(close);
            }
        }
        //   sort events top to bottom
        std::sort(events.begin(),events.end(),CompareEvents());
        for(vector<Event*>::iterator i=events.begin();i!=events.end();i++) {
            Event* e=*i;
            e->process();
        }
        assert(openSegments.empty());
        assert(openNodes.empty());
    }
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
