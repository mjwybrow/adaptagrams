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
void feach(Container& c, Op op) {
    for_each(c.begin(),c.end(),op);
}


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
    TransformNode( const Rectangles& targets,
            const ResizeMap& resizes)
        : targets(targets), resizes(resizes) {}
    Node* operator() (Node* u) {
        const Rectangle *targetRect = targets[u->id];
        Rectangle *initialRect = new Rectangle(*u->rect);
        ResizeMap::const_iterator ri = resizes.find(u->id);
        double desiredCentre, weight;
        if(ri==resizes.end()) { // no resize
            desiredCentre = targetRect->getCentreD(dim);
            weight = DEFAULT_WEIGHT;
        } else { // resizing!
            double m = initialRect->getMinD(dim);
            initialRect->reset(dim, m, m+DUMMY_RECT_THICKNESS);
            desiredCentre = targetRect->getMinD(dim)+DUMMY_RECT_THICKNESS/2.0;
            weight = FIXED_WEIGHT;
        }
        return new topology::Node(u->id, initialRect,
                new Variable(u->id,desiredCentre,weight));
    }
    const Rectangles& targets;
    const ResizeMap& resizes;
};
/**
 * Functor which, for the specified Resizeinfo, creates a centre and
 * right dummy node and associated variables.
 */
struct CreateCentreRightDummyNodes {
    CreateCentreRightDummyNodes(
            const Rectangles& targets,
            Nodes& nodes)
        : targets(targets), nodes(nodes) {}
    void operator() (pair<const unsigned,ResizeInfo>& p) {
        // dummy nodes will have the same id as the original
        ResizeInfo& ri=p.second;
        const unsigned id=ri.orig->id;
        assert(p.first==id);
        const Rectangle *ro=ri.orig->rect,
                        *target=targets[id];
        static const double w2=DUMMY_RECT_THICKNESS/2.0;
        // Centre
        const double cpos=ro->getCentreD(dim);
        Rectangle *centreRect=new Rectangle(*ro);
        centreRect->reset(dim,cpos-w2,cpos+w2);
        ri.centreNode=new topology::Node(id,centreRect,
                new Variable(nodes.size(),
                    target->getCentreD(dim),FIXED_WEIGHT));
        nodes.push_back(ri.centreNode);
        // Right
        const double rpos=ro->getMaxD(dim);
        Rectangle *rhsRect=new Rectangle(*ro);
        rhsRect->reset(dim,rpos-DUMMY_RECT_THICKNESS,rpos);
        double desiredPos=target->getMaxD(dim)-DUMMY_RECT_THICKNESS/2.0;
        ri.rhsNode=new topology::Node(id,rhsRect,
                new Variable(nodes.size(),desiredPos,FIXED_WEIGHT));
        nodes.push_back(ri.rhsNode);
    }
    const Rectangles& targets;
    Nodes& nodes;
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
    SubstituteNodes(ResizeMap& resizes, 
            const Nodes& tn) 
        : resizes(resizes), tn(tn) {}
    void operator() (Edge* e) {
        e->forEachEdgePoint(*this);
    }
    void operator() (EdgePoint* p) {
        const unsigned id=p->node->id;
        const ResizeMap::const_iterator& ri=resizes.find(id);
        enum Pos {LHS, RHS, CEN} pos;
        if(ri!=resizes.end()) {
            switch(p->rectIntersect) {
                case EdgePoint::TL:
                    pos=dim==cola::HORIZONTAL?LHS:RHS;
                    break;
                case EdgePoint::BL:
                    pos=LHS;
                    break;
                case EdgePoint::TR:
                    pos=RHS;
                    break;
                case EdgePoint::BR:
                    pos=dim==cola::HORIZONTAL?RHS:LHS;
                    break;
                default:
                    pos=CEN;
            }
            switch(pos) {
                case LHS:
                    p->node=tn[id];
                    break;
                case RHS:
                    p->node=ri->second.rhsNode;
                    break;
                default:
                    assert(p->rectIntersect==topology::EdgePoint::CENTRE);
                    p->node=ri->second.centreNode;
            }
        } else {
            p->node=tn[id];
        }
    }
    ResizeMap& resizes;
    const topology::Nodes& tn;
};
struct RevertNodes {
    RevertNodes(Nodes& orig) 
        : orig(orig) {}
    void operator() (Edge* e) {
        e->forEachEdgePoint(*this);
    }
    void operator() (EdgePoint* p) {
        p->node=orig[p->node->id];
    }
    Nodes& orig;
};
struct CopyPositions {
    CopyPositions(const Nodes& tn, const ResizeMap& rm)
        : tn(tn), rm(rm) {}
    void operator() (Node* v) {
        ResizeMap::const_iterator j=rm.find(v->id);
        if(j==rm.end()) {
            v->rect->moveCentreD(dim,tn[v->id]->rect->getCentreD(dim));
        } else {
            const Rectangle *l=tn[v->id]->rect,
                            *r=j->second.rhsNode->rect;
            v->rect->reset(dim,l->getMinD(dim),r->getMaxD(dim));
        }
    }
    const Nodes& tn;
    const ResizeMap& rm;
};
struct DeleteTempNode {
    void operator() (Node* v) {
        delete v->rect;
        delete v;
    }
};
#ifndef NDEBUG
bool approx_equals(double a, double b) {
    return fabs(a-b)<1e-6;
}
static const double DISPLACEMENT_ERROR=1e-3;
bool checkDesired(
        const Nodes& nodes, 
        const Rectangles& targets, 
        const ResizeMap& resizeMap) {
    for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
        const Node* v=*i;
        const unsigned id=v->id;
        const Rectangle* t=targets[id];
        ResizeMap::const_iterator j=resizeMap.find(id);
        if(j==resizeMap.end()) {
            assert(approx_equals(v->var->desiredPosition,
                        t->getCentreD(dim)));
        }
    }
    for(ResizeMap::const_iterator j=resizeMap.begin();j!=resizeMap.end();++j) {
        const unsigned id=j->first;
        const ResizeInfo& ri=j->second;
        assert(ri.orig->id==id);
        const Node *ln=nodes[id], *cn=ri.centreNode, *rn=ri.rhsNode;
        assert(ln->id==id);
        assert(cn->id==id);
        assert(rn->id==id);
        const Rectangle* t=targets[id];
        const double lp=t->getMinD(dim)+DUMMY_RECT_THICKNESS/2.0,
                     cp=t->getCentreD(dim),
                     rp=t->getMaxD(dim)-DUMMY_RECT_THICKNESS/2.0;
        assert(approx_equals(lp,ln->var->desiredPosition));
        assert(approx_equals(cp,cn->var->desiredPosition));
        assert(approx_equals(rp,rn->var->desiredPosition));
    }
    return true;
}
bool checkFinal(
        const Nodes& nodes, 
        const Rectangles& targets, 
        const ResizeMap& resizeMap) {
    static const double DISPLACEMENT_ERROR=1e-3;
    for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
        const Node* v=*i;
        const Rectangle* t=targets[v->id];
        ResizeMap::const_iterator j=resizeMap.find(v->id);
        if(j==resizeMap.end()) {
            assert(fabs(v->rect->getCentreD(dim)-t->getCentreD(dim))<DISPLACEMENT_ERROR);
        } else {
            const Rectangle *l=nodes[v->id]->rect,
                            *c=j->second.centreNode->rect,
                            *r=j->second.rhsNode->rect;
            assert(fabs(l->getMinD(dim)-t->getMinD(dim))<DISPLACEMENT_ERROR);
            assert(fabs(r->getMaxD(dim)-t->getMaxD(dim))<DISPLACEMENT_ERROR);
            assert(fabs(c->getCentreD(dim)-t->getCentreD(dim))
                    <DISPLACEMENT_ERROR);
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
void resizeAxis(const Rectangles& targets,
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

    assert(assertConvexBends(edges));
    assert(assertNoSegmentRectIntersection(nodes,edges));

    transform(nodes.begin(),nodes.end(),tn.begin(),
            TransformNode(targets,resizes));
    feach(resizes, CreateCentreRightDummyNodes(targets,tn));
    assert(tn.size()==nodes.size()+2*resizes.size());

    // update topologyRoutes with references to resized nodes replaced with
    // correct references to lhs/rhs nodes
    feach(edges,SubstituteNodes(resizes,tn));

    assert(assertConvexBends(edges));
    assert(assertNoSegmentRectIntersection(tn,edges));

    // move nodes and reroute
    topology::TopologyConstraints t(dim,tn,edges,cs);
    assert(checkDesired(tn,targets,resizes));
#ifndef NDEBUG
    unsigned loopCtr=0;
#endif
    while(t.solve()) { assert(++loopCtr<1000); }
    //assert(checkFinal(tn,targets,resizes));
    
    // reposition and resize original nodes
    feach(nodes,CopyPositions(tn,resizes));

    // revert topologyRoutes back to original nodes
    feach(edges,RevertNodes(nodes));

    assert(assertConvexBends(edges));
    assert(assertNoSegmentRectIntersection(nodes,edges));

    // clean up
    feach(tn,DeleteTempNode());
    feach(vs,delete_object());
    feach(cs,delete_object());
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
    dim=cola::HORIZONTAL;
    resizeAxis(targets, nodes, edges, resizes);
    dim=cola::VERTICAL;
    resizeAxis(targets, nodes, edges, resizes);
    feach(targets,delete_object());
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
