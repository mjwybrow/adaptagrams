#ifndef TOPOLOGY_CONSTRAINTS_H
#define TOPOLOGY_CONSTRAINTS_H
#include <vector>
#include <set>
#include <list>
#include "commondefs.h"
namespace vpsc {
    class Rectangle;
    class Variable;
    class Constraint;
}
namespace straightener {
    class Edge;
}
namespace cola {
    class SparseMap;
    typedef std::vector<vpsc::Variable*> Variables;
    typedef std::vector<vpsc::Constraint*> Constraints;
}
namespace topology {
    extern cola::Dim dim;
    using std::valarray;
    class Segment;
    class TopologyConstraint;
    class Edge;
    class DummyNode {
    public:
        const unsigned id;
        double pos[2];
        vpsc::Rectangle* r;
        Segment* inSegment;
        Segment* outSegment;
        vpsc::Variable* var;
        std::set<vpsc::Constraint*> cs;
        DummyNode(const unsigned id, double x, double y, vpsc::Rectangle* r=NULL) 
                : id(id), r(r), inSegment(NULL), outSegment(NULL), var(NULL) {
            pos[0]=x; pos[1]=y;
        }
    };
    class Segment {
    public:
        Edge* edge;
        std::list<Segment*>::iterator edgePos;
        DummyNode* start;
        DummyNode* end;
        std::vector<TopologyConstraint*> topologyConstraints;
        Segment(Edge* edge, DummyNode* start, DummyNode* end) 
            : edge(edge), start(start), end(end) {
            if(start->r==NULL) {
                start->outSegment = this;
            }
            if(end->r==NULL) {
                end->inSegment = this;
            }
        }
        double getMin() {
            return std::min(start->pos[!dim], end->pos[!dim]);
        }
        double getMax() {
            return std::max(start->pos[!dim], end->pos[!dim]);
        }
        bool intersects(double pos) {
            //printf("Testing intersection: pos=%f, segmin=%f, segmax=%f\n",pos,getMin(),getMax());
            if(pos>=getMin() && pos<=getMax()) {
                return true;
            }
            return false;
        }
        void print() {
            double x1=start->pos[0],y1=start->pos[1],x2=end->pos[0],y2=end->pos[1];
            double dx=x2-x1, dy=y2-y1;
            double length=sqrt(dx*dx+dy*dy);
            printf("  segment((%d,%d):((%f,%f),(%f,%f)) l=%f\n",start->id,end->id,start->pos[0],start->pos[1], end->pos[0],end->pos[1],length);
        }
    };
    static double PI=2*acos(0.);
    class Edge {
    public:
        straightener::Edge* sEdge;
        std::list<Segment*> segments;
        Edge(straightener::Edge * e) :sEdge(e) { }
        ~Edge() {
            for_each(segments.begin(),segments.end(),delete_object());
        }
        void getPath(std::vector<unsigned> & path) const;
        void splitSegment(Segment* o, Segment* n1, Segment* n2) {
            n1->edgePos=segments.insert(o->edgePos, n1);
            n2->edgePos=segments.insert(o->edgePos, n2);
            segments.erase(o->edgePos);
        }
        void mergeSegment(Segment* o1, Segment* o2, Segment* n) {
            n->edgePos=segments.insert(o1->edgePos,n);
            segments.erase(o1->edgePos);
            segments.erase(o2->edgePos);
        }
        void print() {
            DummyNode *u=NULL, *v=segments.front()->start;
            double minTheta=PI;
            for(std::list<Segment*>::const_iterator i=segments.begin();i!=segments.end();i++) {
                DummyNode* w=(*i)->end;
                if(u) {
                    double ax=v->pos[0]-u->pos[0],
                           ay=v->pos[1]-u->pos[1],
                           bx=w->pos[0]-v->pos[0],
                           by=w->pos[1]-v->pos[1];
                    double ab=ax*bx+ay*by;
                    double la=sqrt(ax*ax+ay*ay);
                    double lb=sqrt(bx*bx+by*by);
                    double theta=PI-acos(ab/(la*lb));
                    //printf("    angle=%f,a=(%f,%f),b=(%f,%f)\n",theta,ax,ay,bx,by);
                    if(theta<minTheta) minTheta = theta;
                }
                //(*i)->print();
                u=v;
                v=w;
            }
            //assert(minTheta>PI/2.);
        }
    };
    class TopologyConstraint {
        // a constraint between an edge segment uv and some shape w with which may potentially
        // intersect after movement along position pos in axis dim.
        // For example if dim==HORIZONTAL and w is initially to the right of uv the topology
        // constraint is:
        //      w_x > u_x + (v_x - u_x)|w_y-u_y|/|v_y-u_y| + g
        // where g is usually width(w)/2
    public:
        Segment* segment; // uv
        DummyNode* node;  // w
        double pos;
        double g;
        TopologyConstraint(Segment* segment, DummyNode* node, const double y, const double g,
                const bool segmentLeft);
        bool violated() {
            if(slack()<0) {
                return true;
            }
            return false;
        }
        void satisfy(DummyNode* z, valarray<double> & coords, std::set<vpsc::Constraint*> & lcs); 
        double slack() const {
            double d=idealPoint();
            if(segmentLeft) {
                return node->pos[dim] - g - d;
            } else {
                return d - node->pos[dim] - g;
            }
        }
        void print() const {
            char dir=segmentLeft?'<':'>';
            char sign=segmentLeft?'+':'-';
            DummyNode *a=segment->start, *b=segment->end;
            printf("  TC: segment(%d@(%f,%f),%d@(%f,%f)):%f %c %f %c node(%d@(%f,%f)): slack=%f\n", a->id,a->pos[0],a->pos[1], b->id,b->pos[0],b->pos[1], idealPoint(),sign,g,dir, node->id,node->pos[0],node->pos[1], slack());
        }
        bool segmentLeft;
    private:
        double idealPoint() const;
        double idealPoint(
                DummyNode const * u, DummyNode const * v,  const double pos) const;
    };
    class TopologyConstraints {
    public:
        std::vector<Edge*> edges;
        std::vector<DummyNode*> dummyNodes;
        unsigned N() { return dummyNodes.size(); }
        valarray<double> g;
        valarray<double> coords;
        cola::FixedList const & fixed;
        TopologyConstraints(
            const cola::Dim dim, 
            std::vector<vpsc::Rectangle*> const & rs,
            cola::FixedList const & fixed,
            std::vector<straightener::Edge*> const & edges,
            cola::Variables const & vs,
            cola::Variables & lvs,
            std::set<vpsc::Constraint*> *lcs,
            valarray<double> & oldCoords,
            valarray<double> & oldG);
        ~TopologyConstraints() {
            for_each(edges.begin(),edges.end(),delete_object());
            for_each(dummyNodes.begin(),dummyNodes.end(),delete_object());
        }
        DummyNode* addDummyNode(cola::Variables & lvs);
        void violated(std::vector<TopologyConstraint*> & ts) const;
        TopologyConstraint* mostViolated() const;
        void computeForces(cola::SparseMap &H);
        double computeStress() const;
        void updateNodePositionsFromVars();
        void coordsToNodePositions();
        void finalizeRoutes();
        void verify(std::vector<vpsc::Rectangle*> &rs);
        void getInactiveDummyNodes(std::vector<DummyNode*> &inactive) const;
        void tightenSegments(std::set<vpsc::Constraint*> &lcs);
    private:
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
} // namespace topology
#endif // TOPOLOGY_CONSTRAINTS_H
// vim: cindent ts=4 sw=4 et tw=0 wm=0
