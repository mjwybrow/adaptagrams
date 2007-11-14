#include "topology_constraints.h"
#include "cola.h"
#include "straightener.h"
using namespace std;
using vpsc::Rectangle;
namespace topology {
    cola::Dim dim;
    /*
     * Generally the scan produces separation constraints for segment
     * opens/closes and topology constraints for node opens/closes.  However,
     * when a segment that overlaps a node rectangle is found, extra processing
     * must be performed and topology constraints generated from segment
     * openings and separation constraints from node opens.
     */
    void processIntersectingNodeEdge() {
    }
    /*
     *                           Event        
     *                          /     \               
     *            RectangleEvent       SegmentEvent    
     *              /      \             /        \     
     * RectangleOpen RectangleClose SegmentOpen SegmentClose
     */
    struct SegmentOpen;
    struct RectangleOpen;
    vector<SegmentOpen*> openSegments; 
    // open rectangles are stored in map keyed on position parallel to scan line.
    // Note that this assumes no overlaps between rectangles.
    typedef map<double,RectangleOpen*> OpenRectangleSet;
    OpenRectangleSet openRectangles;
    set<vpsc::Constraint*> *separationConstraints;
    struct Event {
        bool open;
        double pos;
        DummyNode* dummyNode;
        Event(bool open, double pos, DummyNode* v) : open(open), pos(pos), dummyNode(v) {}
        virtual ~Event() {};
        virtual void process()=0;
        virtual void print()=0;
    };
    struct RectangleEvent : Event {
        Rectangle const *r;
        RectangleEvent(bool open, double pos, DummyNode* v, Rectangle const *r)
            : Event(open,pos,v), r(r) {
            //printf("RectangleEvent v->id=%d, pos=%f\n",v->id,pos);
        }
        ~RectangleEvent(){}
        void createTopologyConstraint();
    };
    struct RectangleOpen : RectangleEvent {
        RectangleOpen(double pos, DummyNode* v, Rectangle const *r) 
            : RectangleEvent(true,pos,v,r) {
            //printf("RectangleOpen v->id=%d, pos=%f\n",v->id,pos);
        }
        void process() {
            // Rectangle opening
            print();
            openRectangles[dummyNode->pos[dim]]=this;
            createTopologyConstraint();
        }
        void print() {
            printf("RectangleOpen: pos=%f\n",pos);
        }
    };
    struct RectangleClose : RectangleEvent {
        RectangleClose(double pos, DummyNode* v, Rectangle const *r) :
            RectangleEvent(false,pos,v,r) {}
        void process() {
            print();
            openRectangles.erase(dummyNode->pos[dim]);
            // create topology constraint from scanpos in every open edge to node
            createTopologyConstraint();
            delete this;
        }
        void print() {
            printf("RectangleClose: pos=%f\n",pos);
        }
    };
    struct SegmentEvent : Event {
        Segment *s;
        SegmentEvent(bool open, double pos, DummyNode* v, Segment *s)
            : Event(open,pos,v), s(s) {}
        void createSeparationConstraint() {
            unsigned startPointID=s->edge->sEdge->startNode;
            unsigned endPointID=s->edge->sEdge->endNode;
            // dummy node is one of the ends of the segment
            //   if dummyNode is the real end of the edge then no need for a constraint
            //   (should be handled by internode nonoverlap constraints)
            if(dummyNode->id == startPointID || dummyNode->id == endPointID) {
                return;
            }
            // if segment is parallel to scan line then ignore
            if(s->start->pos[!dim]-s->end->pos[!dim]==0) {
                return;
            }
            printf("------------createSeparationConstraint:---------------------\n");
            printf("  at scanpos=%f, for v=(%d) on seg(%d,%d)\n",pos,dummyNode->id,s->start->id,s->end->id);

            // create separation constraint from dummyNode to neighbours in openRectangle list
            double vpos = dummyNode->pos[dim];
            RectangleOpen* ns[2];
            ns[0]=ns[1]=NULL;
            OpenRectangleSet::iterator it=openRectangles.lower_bound(vpos);
            // it is the first node to the right of v
            if(it!=openRectangles.end()) {
                ns[1]=it->second;
                // step left to find the first node to the left of v
                if(it!=openRectangles.begin()) {
                    it--;
                    ns[0]=it->second;
                }
            }
            if(ns[1]!=NULL && ns[0]!=NULL) {
                assert(ns[1]!=ns[0]);
            }
            for(unsigned i=0;i<2;i++) {
                RectangleOpen* o=ns[i];
                if(o==NULL || o->dummyNode->id==startPointID || o->dummyNode->id==endPointID) {
                    continue;
                }
                printf("  neighbourRect[%d]=%d\n",i,o->dummyNode->id);
                double g = o->r->length(dim)/2.0+0.1, 
                       rpos = o->dummyNode->pos[dim], 
                       vpos = dummyNode->pos[dim];
#ifdef STRAIGHTENER_DEBUG
                s->edge->sEdge->debugLines.push_back(
                        straightener::DebugLine( dummyNode->pos[0], dummyNode->pos[1], 
                            o->dummyNode->pos[0], o->dummyNode->pos[1], 2));
#endif
                vpsc::Constraint* c;
                if(rpos < vpos) {
                    c = new vpsc::Constraint(o->dummyNode->var,dummyNode->var,g);
                    printf("new SC: v[%d]+%f<=v[%d]\n",o->dummyNode->id,g,dummyNode->id);
                } else {
                    c = new vpsc::Constraint(dummyNode->var,o->dummyNode->var,g);
                    printf("new SC: v[%d]+%f<=v[%d]\n",dummyNode->id,g,o->dummyNode->id);
                }
                separationConstraints->insert(c);
                o->dummyNode->cs.insert(c);
                dummyNode->cs.insert(c);
            }
            printf("-----done createSeparationConstraint.---------------------\n");
        }
    };
    struct SegmentOpen : SegmentEvent {
        unsigned openListIndex;
        SegmentOpen(double pos, DummyNode* v, Segment *s) : SegmentEvent(true,pos,v,s) {}
        void process() {
            print();
            openListIndex=openSegments.size();
            openSegments.push_back(this);
            // if dummy node create separation constraint 
            // to every open node
            createSeparationConstraint();
        }
        void print() {
            printf("SegmentOpen: pos=%f\n",pos);
            //s->print();
        }
    };
    struct SegmentClose : SegmentEvent {
        SegmentOpen* opening;
        SegmentClose(double pos, DummyNode* v, Segment *s, SegmentOpen* so)
            : SegmentEvent(false,pos,v,s), opening(so) {}
        void removeFromOpenList(vector<SegmentOpen*> &openList) {
            assert(openList.size()>0);
            unsigned m=opening->openListIndex, n=openList.size()-1;
            //printf(" exchanging m=%d, n=%d\n",m,n);
            swap(openList[m],openList[n]);
            openList[m]->openListIndex=m;
            delete openList[n];
            openList.resize(n);
        }
        void process() {
            // Segment Closing
            print();
            removeFromOpenList(openSegments);
            // if dummy node create separation constraint 
            // to every open node
            createSeparationConstraint();
            delete this;
        }
        void print() {
            printf("SegmentClose: pos=%f\n",pos);
        }
    };
    void RectangleEvent::createTopologyConstraint() {
        printf("---------createTopologyConstraint()------------------------\n");
        // create topology constraint from scanpos in every open segment to node
        for(vector<SegmentOpen*>::const_iterator j=openSegments.begin();
                j!=openSegments.end();j++) {
            Segment* s=(*j)->s;
            if(s->edge->sEdge->startNode==dummyNode->id
                    || s->edge->sEdge->endNode==dummyNode->id) {
                // skip if edge is attached to this rect
                continue;
            }
            DummyNode *u=s->start, *v=s->end;
            // segments orthogonal to scan direction need no constraints
            if(v->pos[!dim]-u->pos[!dim]==0) continue;
            if(u->pos[!dim]>=r->getMaxD(!dim)&&v->pos[!dim]>=r->getMaxD(!dim)) {
                // segment is below/right of rect
                continue;
            }
            if(u->pos[!dim]<=r->getMinD(!dim)&&v->pos[!dim]<=r->getMinD(!dim)) {
                // segment is above/left of rect
                continue;
            }
            // compute intersection with scan line
            double denom=v->pos[!dim]-u->pos[!dim];
            assert(denom!=0);
            double intersection=u->pos[dim]+
                (v->pos[dim]-u->pos[dim])*
                fabs((pos-u->pos[!dim])/denom);
            bool passLeft=false;
            if(intersection < dummyNode->pos[dim]) {
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
            double g=r->length(dim)/2.0+0.1;
            s->topologyConstraints.push_back(new TopologyConstraint(s,dummyNode,pos,g,passLeft));
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
                    if(dynamic_cast<SegmentOpen*>(a) && dynamic_cast<RectangleOpen*>(b)) return true;
                    if(dynamic_cast<SegmentOpen*>(b) && dynamic_cast<RectangleOpen*>(a)) return false;
                }
                // Segment closes at the same position as node closes, node comes first
                if(!a->open && !b->open) {
                    if(dynamic_cast<SegmentOpen*>(a) && dynamic_cast<RectangleOpen*>(b)) return false;
                    if(dynamic_cast<SegmentOpen*>(b) && dynamic_cast<RectangleOpen*>(a)) return true;
                }
                // note: Segments orthogonal to scan direction (i.e. OpenPos=ClosePos) can still
                // have node events between the segment open and close... need to handle elsewhere 
            }
            return false;
        }
    };
    TopologyConstraints::TopologyConstraints( const cola::Dim axisDim,
            vector<Rectangle*> const & rs,
            cola::FixedList const & fixed,
            vector<straightener::Edge*> const & sEdges,
            cola::Variables const & vs,
            cola::Variables & lvs,
            set<vpsc::Constraint*> *lcs,
            valarray<double> & oldCoords,
            valarray<double> & oldG)
     : fixed(fixed) {
        //  We must generate separation constraints for bend points that become
        //  active after a movement lazily:  
        //    - generate dummy nodes for each of the bend points in the input routing
        //      (also dummy nodes are generated actual shapes)
        //    - then for each edge segment (u,v) which overlaps with a node w in the axis 
        //      of concern (assume x), we generate a topology constraint:
        //        w_x > u_x + (v_x - u_x)|w_y-u_y|/|v_y-u_y| + g
        //      to keep w to the right of uv, or a symmetric constraint to keep w
        //      to the left of uv (where g is half the width of w)
        //    - separation constraints are generated for active bend points
        // process edge routes creating segments

        separationConstraints=lcs;

        dim = axisDim;
        printf("Generating topology constraints! (dim=%d)\n",dim);

        unsigned n=rs.size();
        for(unsigned i=0;i<n;i++) {
            Rectangle* r=rs[i];
            dummyNodes.push_back(new DummyNode(i,r->getCentreX(), r->getCentreY(),r));
            printf("addRect(rs,%f,%f,%f,%f);\n",r->getMinX(),r->getMinY(),r->width(),r->height());
        }
        
        for(vector<straightener::Edge*>::const_iterator e=sEdges.begin();e!=sEdges.end();e++) {
            Edge *edge=new Edge(*e);
            edges.push_back(edge);
            edge->sEdge->rerouteAround(rs);
            DummyNode *start=dummyNodes[edge->sEdge->startNode], *end=NULL;
            straightener::Route const *r=edge->sEdge->getRoute();
            for(unsigned i=1;i<r->n;i++) {
                if(i==r->n-1) {
                    end=dummyNodes[edge->sEdge->endNode];
                } else {
                    end=new DummyNode(dummyNodes.size(),r->xs[i],r->ys[i]);
                    dummyNodes.push_back(end);
                }
                Segment *segment=new Segment(edge,start,end);
                segment->edgePos=edge->segments.insert(edge->segments.end(),segment);
                start = end;
            }
        }

        g.resize(N());
        coords.resize(N());
        for(unsigned i=0;i<n;i++) {
            g[i]=oldG[i];
            coords[i]=oldCoords[i];
            dummyNodes[i]->var = vs[i];
        }
        for (unsigned i=n;i<N();i++) {
            // the desired position is set after we compute the forces
            vpsc::Variable* v=new vpsc::Variable(i,1,1);
            lvs.push_back(v);
            g[i]=0;
            coords[i]=dummyNodes[i]->pos[dim];
            dummyNodes[i]->var = v;
        }

        //printf("dummyNodes.size=%d\n",N());

        vector<Event*> events;
        // scan vertically to create horizontal topology constraints
        // place Segment opening/closing and Rectangle opening/closing into event queue
        for(unsigned i=0;i<n;i++) {
            Rectangle* r=rs[i];
            double openPos=r->getMinD(!dim);
            double closePos=r->getMaxD(!dim);
            printf("Rectangle: dim=%d, openPos=%f, closePos=%f\n",dim,openPos,closePos);
            RectangleOpen *open=new RectangleOpen(openPos,dummyNodes[i],r);
            RectangleClose *close=new RectangleClose(closePos,dummyNodes[i],r);
            events.push_back(open);
            events.push_back(close);
        }
        for(unsigned i=0;i<edges.size();i++) {
            Edge* e=edges[i];
            for(list<Segment*>::iterator si=e->segments.begin();
                    si!=e->segments.end();si++) {
                Segment* s=*si;
                DummyNode *openNode, *closeNode;
                if(s->start->pos[!dim]==s->getMin()) {
                    openNode=s->start; closeNode=s->end;
                } else {
                    openNode=s->end; closeNode=s->start;
                }
                SegmentOpen *open=new SegmentOpen(s->getMin(),openNode,s);
                Event *close=new SegmentClose(s->getMax(),closeNode,s,open);
                events.push_back(open);
                events.push_back(close);
            }
        }
        //   sort events top to bottom
        std::sort(events.begin(),events.end(),CompareEvents());
        /*
        for(vector<Event*>::iterator i=events.begin();i!=events.end();i++) {
            Event* e=*i;
            e->print();
        }
        */
        for(vector<Event*>::iterator i=events.begin();i!=events.end();i++) {
            Event* e=*i;
            e->process();
        }
        assert(openSegments.empty());
        assert(openRectangles.empty());
    }
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
