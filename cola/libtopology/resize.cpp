#include <tr1/functional>
#include <libvpsc/rectangle.h>
#include <libvpsc/constraint.h>
#include <libcola/cola.h>
#include "topology_constraints.h"

namespace topology {
using namespace std;
using namespace vpsc;
static const double DUMMY_RECT_THICKNESS=1e-4;
static const double DEFAULT_WEIGHT = 1.0;
static const double FIXED_WEIGHT = 10000;

template <class Container, class Op>
void mapf(Container& c, Op op) {
    for_each(c.begin(),c.end(),op);
}

typedef map<Node*, Node*> RevertMap;

/**
 * Functor that creates a copy of the specified Node and creates a
 * new Rectangle and a Variable associated with the node.
 * If the node appears in the resizes lookup the rectangle is reduced
 * to a sliver of the left hand side (or bottom depending on dim) of the
 * original rect and the desired position of the variable is the left
 * side of the target rect.
 * If not in the resizes list, the rectangle is simply a copy of
 * the initial rectangle and the desired position is the centre of the
 * target.
 */
struct TransformNode {
    TransformNode(const cola::Dim dim, const Rectangles& targets,
            const ResizeMap& resizes, RevertMap& revertMap, Variables& vs)
        : dim(dim), targets(targets), resizes(resizes), 
          revertMap(revertMap), vs(vs) {}
    Node* operator() (Node* u) {
        Rectangle *initialRect = new Rectangle(*u->rect),
                  *targetRect = targets[u->id];
        ResizeMap::const_iterator ri=resizes.find(u->id);
        double desiredCentre, weight;
        if(ri==resizes.end()) { // no resize
            desiredCentre=targetRect->getCentreD(dim);
            weight=DEFAULT_WEIGHT;
        } else { // resizing!
            double m=initialRect->getMinD(dim);
            initialRect->reset(dim,m,m+DUMMY_RECT_THICKNESS);
            desiredCentre=targetRect->getMinD(dim)+DUMMY_RECT_THICKNESS/2.0;
            weight=FIXED_WEIGHT;
        }
        vs.push_back(new Variable(u->id,desiredCentre,weight));
        Node* v = new topology::Node(u->id,initialRect);
        revertMap[v] = u;
        return v;
    }
    const cola::Dim dim;
    const Rectangles& targets;
    const ResizeMap& resizes;
    RevertMap& revertMap;
    Variables& vs;
};
/**
 * Functor which, for the specified Resizeinfo, creates a centre and
 * right dummy node and associated variables.
 */
struct CreateCentreRightDummyNodes {
    CreateCentreRightDummyNodes(const cola::Dim dim, Nodes& nodes, 
            RevertMap& revertMap, Variables& vs)
        : dim(dim), nodes(nodes), revertMap(revertMap), vs(vs) {}
    void operator() (pair<const unsigned,ResizeInfo>& p) {
        ResizeInfo& ri=p.second;
        assert(p.first==ri.orig->id);
        const Rectangle* ro=ri.orig->rect;
        const double M=ro->getMaxD(dim),
                     c=ro->getCentreD(dim),
                     w2=DUMMY_RECT_THICKNESS/2.0;
        Rectangle *centreRect=new Rectangle(*ro), *rhsRect=new Rectangle(*ro);
        centreRect->reset(dim,c-w2,c+w2);
        rhsRect->reset(dim,M-DUMMY_RECT_THICKNESS,M);
        unsigned j=nodes.size();
        ri.centreNode=new topology::Node(j,centreRect);
        revertMap[ri.centreNode]=ri.orig;
        nodes.push_back(ri.centreNode);
        vs.push_back(new Variable(j,ri.targetRect->getCentreD(dim),FIXED_WEIGHT));
        j++;
        ri.rhsNode=new topology::Node(j,rhsRect);
        revertMap[ri.rhsNode]=ri.orig;
        nodes.push_back(ri.rhsNode);
        double desiredPos=ri.targetRect->getMaxD(dim)-DUMMY_RECT_THICKNESS/2.0;
        vs.push_back(new Variable(j,desiredPos,FIXED_WEIGHT));
    }
    const cola::Dim dim;
    Nodes& nodes;
    RevertMap& revertMap;
    Variables& vs;
};
/**
 * Functor to reassign EdgePoint Node pointers.  
 * If the Node associated with an EdgePoint is resizing, the EdgePoint Node
 * pointer is reassigned to either the lhs, rhs or centre dummy Node.
 * Otherwise (and also actually for the lhs node which is already set
 * in the new Node list), the Node pointer is simply updated to the equivalent Node
 * in the new Node list.
 */
struct SubstituteNodes {
    SubstituteNodes(cola::Dim dim, ResizeMap& resizes, 
            const Nodes& nodes) 
        : resizes(resizes), nodes(nodes) {}
    void operator() (Edge* e) {
        e->forEachEdgePoint(*this);
    }
    void operator() (EdgePoint* p) {
        const unsigned id=p->node->id;
        const ResizeMap::const_iterator& ri=resizes.find(id);
        enum Pos {LHS, RHS, CEN} pos;
        if(ri!=resizes.end()) {
            switch(p->rectIntersect) {
                case topology::EdgePoint::TL:
                    pos=dim==cola::HORIZONTAL?LHS:RHS;
                    break;
                case topology::EdgePoint::BL:
                    pos=LHS;
                    break;
                case topology::EdgePoint::TR:
                    pos=RHS;
                    break;
                case topology::EdgePoint::BR:
                    pos=dim==cola::HORIZONTAL?RHS:LHS;
                    break;
                default:
                    pos=CEN;
            }
            switch(pos) {
                case LHS:
                    p->node=nodes[id];
                    break;
                case RHS:
                    p->node=ri->second.rhsNode;
                    break;
                default:
                    assert(p->rectIntersect==topology::EdgePoint::CENTRE);
                    p->node=ri->second.centreNode;
            }
        } else {
            p->node=nodes[id];
        }
    }
    ResizeMap& resizes;
    const topology::Nodes& nodes;
};
struct RevertNodes {
    RevertNodes(cola::Dim dim, RevertMap& revertMap) 
        : revertMap(revertMap) {}
    void operator() (Edge* e) {
        e->forEachEdgePoint(*this);
    }
    void operator() (EdgePoint* p) {
        p->node=revertMap[p->node];
    }
    RevertMap& revertMap;
};
struct CopyPositions {
    CopyPositions(const cola::Dim dim, Nodes::const_iterator i)
        : dim(dim), i(i) {}
    void operator() (Node* v) {
        assert(i.base()!=NULL); // fails if target container is bigger than source
        const Node* u=*i++;
        v->rect->moveCentreD(dim,u->rect->getCentreD(dim));
    }
    const cola::Dim dim;
    Nodes::const_iterator i;
};
struct ResizeRects {
    ResizeRects(const cola::Dim dim)
        : dim(dim) {}
    void operator() (pair<const unsigned,ResizeInfo>& p) {
        ResizeInfo& ri=p.second; 
        assert(p.first==ri.orig->id);
        ri.orig->rect->reset(dim,ri.targetRect->getMinD(dim),ri.targetRect->getMaxD(dim));
    }
    const cola::Dim dim;
};
struct DeleteTempNode {
    void operator() (Node* v) {
        delete v->rect;
        delete v;
    }
};
#ifndef NDEBUG
bool checkTargets(
        const cola::Dim dim,
        const Nodes& nodes, 
        const Rectangles& targets, 
        const ResizeMap& resizeMap,
        const RevertMap& revertMap) {
    for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
        const Node* u=*i;
        const Rectangle* t=targets[u->id];
        ResizeMap::const_iterator j=resizeMap.find(u->id);
        if(j!=resizeMap.end()) {
            const Rectangle *l=u->rect,
                            *c=j->second.centreNode->rect,
                            *r=j->second.rhsNode->rect;
            assert(fabs(l->getMinD(dim)-t->getMinD(dim))<1e-5);
            assert(fabs(r->getMaxD(dim)-t->getMaxD(dim))<1e-5);
            assert(fabs(c->getCentreD(dim)-t->getCentreD(dim))<1e-5);
        }
    }
    return true;
}
#endif
/**
 * Applies the required resizes to nodes in the specified axis, rerouting edges
 * around the resized nodes.
 * @param dim axis
 * @param targets the target rectangles (in both axes)
 * @param nodes to be moved and/or resized
 * @param edges to be rerouted around nodes
 * @param resizes ResizeInfo for specific nodes
 */
void resizeAxis(const cola::Dim dim, const Rectangles& targets,
        Nodes& nodes, Edges& edges, ResizeMap& resizes) {
    //  - create copy tn of topologyNodes with resize rects replaced with
    //    three nodes: one for the lhs of rect, one for centre and one for rhs.
    //    lhs node goes at position of replaced node, the others are appended
    //    to end of tn.
    //  - set desired positions of each lhs node to be the left side
    //    of resized rect and symmetric for rhs node, centre node's desired
    //    pos it at the centre
    Nodes tn(nodes.size());
    Variables vs;
    Constraints cs;
    RevertMap revertMap;
    transform(nodes.begin(),nodes.end(),tn.begin(),
            TransformNode(dim,targets,resizes,revertMap,vs));
    mapf(resizes, CreateCentreRightDummyNodes(dim,tn,revertMap,vs));
    assert(tn.size()==nodes.size()+2*resizes.size());
    assert(vs.size()==tn.size());
    assert(revertMap.size()==tn.size());

    // update topologyRoutes with references to resized nodes replaced with
    // correct references to lhs/rhs nodes
    mapf(edges,SubstituteNodes(dim,resizes,tn));

    // move nodes and reroute
    topology::TopologyConstraints t(dim,tn,edges,vs,cs);
    while(t.solve()) {}
    assert(checkTargets(dim,tn,targets,resizes,revertMap));
    
    // reposition and resize original nodes
    mapf(nodes,CopyPositions(dim,tn.begin()));
    mapf(resizes,ResizeRects(dim));

    // revert topologyRoutes back to original nodes
    mapf(edges,RevertNodes(dim,revertMap));

    // clean up
    mapf(tn,DeleteTempNode());
    mapf(vs,delete_object());
    mapf(cs,delete_object());
}
/**
 * Functor that for a given node, if that node is in the resizes lookup,
 * will create a copy of the target rectangle and adds the node id to
 * the fixed list.
 * Otherwise, it simply creates a copy of the node's existing rectangle.
 */
struct CreateTargetRect {
    CreateTargetRect(ResizeMap& resizes, set<unsigned>& fixed) : resizes(resizes), fixed(fixed) {}
    Rectangle* operator() (Node* v) {
        ResizeMap::iterator r=resizes.find(v->id);
        Rectangle* target;
        if(r==resizes.end()) {
            target=new Rectangle(*v->rect);
        } else {
            fixed.insert(v->id); // resized rectangles are required to stay
                                 // where they have been placed
            target=new Rectangle(*r->second.targetRect);
            assert(target->width() > 3.0*DUMMY_RECT_THICKNESS);
            assert(target->height() > 3.0*DUMMY_RECT_THICKNESS);
        }
        return target;
    }
    ResizeMap& resizes;
    set<unsigned>& fixed;
};
/**
 * Find an overlap free arrangement of resized rectangles and reroute
 * edges around them without changing topology.
 * @param nodes topology node definitions to be resized
 * @param edges edges to be reroutes
 * @param resizes list of resizes to be applied to nodes
 */
void applyResizes(Nodes& nodes, Edges& edges, ResizeMap& resizes) {
    // targets will hold an overlap free placement of the resized rectangles
    Rectangles targets(nodes.size());
    // rectangles that are resized should be fixed when finding overlap free placement
    // the following is populated with their ids
    set<unsigned> fixed;
    transform(nodes.begin(),nodes.end(),targets.begin(),CreateTargetRect(resizes,fixed));
    removeoverlaps(targets,fixed);
    resizeAxis(cola::HORIZONTAL, targets, nodes, edges, resizes);
    resizeAxis(cola::VERTICAL, targets, nodes, edges, resizes);
    mapf(targets,delete_object());
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
