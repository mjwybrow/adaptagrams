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
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
 * Author(s):  Tim Dwyer
*/

#include "libvpsc/assertions.h"
#include "libvpsc/rectangle.h"
#include "libvpsc/constraint.h"
#include "libcola/cola.h"
#include "topology_constraints.h"

namespace topology {
using namespace std;
using namespace vpsc;
using namespace cola;
static const double DW=1e-4;
static const double DW2=DW/2.0;
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
 * to a sliver in the middle of the original rect.
 * In either case the desired position is the centre of the target.
 */
struct TransformNode {
    TransformNode(vpsc::Dim dim, const Rectangles& targets,
            const ResizeMap& resizes, Variables& vs)
        : dim(dim),
          targets(targets),
          resizes(resizes),
          vs(vs)
    {
    }
    Node* operator() (Node* u) {
        const Rectangle *targetRect = targets[u->id];
        Rectangle *rect = new Rectangle(*u->rect);
        ResizeMap::const_iterator ri = resizes.find(u->id);
        Variable* var = vs[u->id];
        var->desiredPosition = targetRect->getCentreD(dim);
        if(ri==resizes.end()) { // no resize
            var->weight = DEFAULT_WEIGHT;
        } else { // resizing!
            const double c=rect->getCentreD(dim);
            rect->reset(dim, c - DW2, c + DW2);
            var->weight = FIXED_WEIGHT;
        }
        return new topology::Node(u->id, rect, var);
    }
    vpsc::Dim dim;
    const Rectangles& targets;
    const ResizeMap& resizes;
    Variables& vs;
};
/**
 * Functor which, for the specified Resizeinfo, creates a centre and
 * right dummy node and associated variables.
 */
struct CreateLeftRightDummyNodes
{
    CreateLeftRightDummyNodes(vpsc::Dim dim, const Rectangles& targets,
            Nodes& nodes, Variables& vs)
        : dim(dim),
          targets(targets),
          nodes(nodes),
          vs(vs)
    {
    }
    void operator() (pair<const unsigned, ResizeInfo>& p) {
        // dummy nodes will have the same id as the original
        ResizeInfo& ri=p.second;
        const unsigned id=ri.orig->id;
        COLA_ASSERT(p.first==id);
        const Rectangle *ro=ri.orig->rect,
                        *targetRect=targets[id];

        // LEFT
        const double l = ro->getMinD(dim);
        Rectangle *lhsRect=new Rectangle(*ro);
        lhsRect->reset(dim, l, l + DW);
        Variable* lv = new Variable(vs.size(),
                targetRect->getMinD(dim) + DW2, FIXED_WEIGHT);
        vs.push_back(lv);
        ri.lhsNode=new topology::Node(id,lhsRect,lv);
        nodes.push_back(ri.lhsNode);
        // Right
        const double r = ro->getMaxD(dim);
        Rectangle *rhsRect=new Rectangle(*ro);
        rhsRect->reset(dim, r - DW, r);
        Variable* rv = new Variable(vs.size(), 
                targetRect->getMaxD(dim) - DW2, FIXED_WEIGHT);
        vs.push_back(rv);
        ri.rhsNode=new topology::Node(id, rhsRect, rv);
        nodes.push_back(ri.rhsNode);
    }
    vpsc::Dim dim;
    const Rectangles& targets;
    Nodes& nodes;
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
struct SubstituteNodes
{
    SubstituteNodes(vpsc::Dim dim, ResizeMap& resizes, const Nodes& tn)
        : dim(dim),
          resizes(resizes),
          tn(tn)
    {
    }
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
                    pos=dim==vpsc::HORIZONTAL?LHS:RHS;
                    break;
                case EdgePoint::BL:
                    pos=LHS;
                    break;
                case EdgePoint::TR:
                    pos=RHS;
                    break;
                case EdgePoint::BR:
                    pos=dim==vpsc::HORIZONTAL?RHS:LHS;
                    break;
                default:
                    pos=CEN;
            }
            switch(pos) {
                case LHS:
                    p->node=ri->second.lhsNode;
                    break;
                case RHS:
                    p->node=ri->second.rhsNode;
                    break;
                default:
                    COLA_ASSERT(p->rectIntersect==topology::EdgePoint::CENTRE);
                    p->node=tn[id];
            }
        } else {
            p->node=tn[id];
        }
    }
    vpsc::Dim dim;
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
    CopyPositions(vpsc::Dim dim, const Nodes& tn, const ResizeMap& rm)
        : dim(dim),
          tn(tn),
          rm(rm)
    {
    }
    void operator() (Node* v) {
        ResizeMap::const_iterator j=rm.find(v->id);
        if(j==rm.end()) {
            v->rect->moveCentreD(dim,tn[v->id]->rect->getCentreD(dim));
        } else {
            const Rectangle *l=j->second.lhsNode->rect,
                            *r=j->second.rhsNode->rect;
            v->rect->reset(dim,l->getMinD(dim),r->getMaxD(dim));
        }
    }
    vpsc::Dim dim;
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
static bool checkDesired(vpsc::Dim dim, const Nodes& nodes,
        const Rectangles& targets, const ResizeMap& resizeMap)
{
    for (Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
        const Node* v=*i;
        const unsigned id=v->id;
        const Rectangle* t=targets[id];
        ResizeMap::const_iterator j=resizeMap.find(id);
        if(j==resizeMap.end()) {
            COLA_ASSERT(approx_equals(v->var->desiredPosition,
                        t->getCentreD(dim)));
        }
    }
    for(ResizeMap::const_iterator j=resizeMap.begin();j!=resizeMap.end();++j) {
        const unsigned id=j->first;
        const ResizeInfo& ri=j->second;
        COLA_ASSERT(ri.orig->id==id);
        const Node *ln=ri.lhsNode, *cn=nodes[id], *rn=ri.rhsNode;
        COLA_ASSERT(ln->id==id);
        COLA_ASSERT(cn->id==id);
        COLA_ASSERT(rn->id==id);
        const Rectangle* t=targets[id];
        const double lp = t->getMinD(dim) + DW2,
                     cp = t->getCentreD(dim),
                     rp = t->getMaxD(dim) - DW2;
        COLA_ASSERT(approx_equals(lp,ln->var->desiredPosition));
        COLA_ASSERT(approx_equals(cp,cn->var->desiredPosition));
        COLA_ASSERT(approx_equals(rp,rn->var->desiredPosition));
    }
    return true;
}
#if 0
static bool checkFinal(vpsc::Dim dim, const Nodes& nodes,
        const Rectangles& targets, const ResizeMap& resizeMap)
{
    static const double DISPLACEMENT_ERROR=1e-3;
    for(Nodes::const_iterator i=nodes.begin();i!=nodes.end();++i) {
        const Node* v=*i;
        const Rectangle* t=targets[v->id];
        ResizeMap::const_iterator j=resizeMap.find(v->id);
        if(j==resizeMap.end()) {
            COLA_ASSERT(fabs(v->rect->getCentreD(dim)-t->getCentreD(dim))
                    <DISPLACEMENT_ERROR);
        } else {
            const Rectangle *l=j->second.lhsNode->rect,
                            *c=nodes[v->id]->rect,
                            *r=j->second.rhsNode->rect;
            COLA_ASSERT(fabs(l->getMinD(dim)-t->getMinD(dim))<DISPLACEMENT_ERROR);
            COLA_ASSERT(fabs(r->getMaxD(dim)-t->getMaxD(dim))<DISPLACEMENT_ERROR);
            COLA_ASSERT(fabs(c->getCentreD(dim)-t->getCentreD(dim))
                    <DISPLACEMENT_ERROR);
        }
    }
    return true;
}
#endif
#endif
/**
 * Applies the required resizes to nodes in the specified axis, rerouting edges
 * around the resized nodes.
 * @param dim axis
 * @param targets the target rectangles (in both axes)
 * @param nodes to be moved and/or resized
 * @param edges to be rerouted around nodes
 * @param resizes ResizeInfo for specific nodes
 * @param vs canonical list of variables passed into solver.  Note that
 * the first nodes.size() variables are used for each corresponding node.  
 * Note also that new variables for the dummy nodes will be appended to this
 * list and will need to be cleaned up later.
 * @param cs canonical list of constraints over variables.  Note that new
 * non-overlap constraints may be appended to the end of this list.
 */
static void resizeAxis(vpsc::Dim dim, const Rectangles& targets,
        Nodes& nodes, Edges& edges,  RootCluster *clusters, ResizeMap& resizes,
        Variables& vs, Constraints& cs)
{
    COLA_ASSERT(vs.size()>=nodes.size());

    //  - create copy tn of topologyNodes with resize rects replaced with
    //    three nodes: one for the lhs of rect, one for centre and one for rhs.
    //    lhs node goes at position of replaced node, the others are appended
    //    to end of tn.
    //  - set desired positions of each lhs node to be the left side
    //    of resized rect and symmetric for rhs node, centre node's desired
    //    pos it at the centre
    Nodes tn(nodes.size());

    COLA_ASSERT(assertConvexBends(edges));
    COLA_ASSERT(assertNoSegmentRectIntersection(nodes,edges));

    transform(nodes.begin(),nodes.end(),tn.begin(),
            TransformNode(dim, targets,resizes,vs));
    feach(resizes, CreateLeftRightDummyNodes(dim,targets,tn,vs));
    COLA_ASSERT(tn.size()==nodes.size()+2*resizes.size());
    COLA_ASSERT(vs.size()>=tn.size());

    // update topologyRoutes with references to resized nodes replaced with
    // correct references to lhs/rhs nodes
    feach(edges,SubstituteNodes(dim,resizes,tn));

    COLA_ASSERT(assertConvexBends(edges));
    COLA_ASSERT(assertNoSegmentRectIntersection(tn,edges));

    // move nodes and reroute
    topology::TopologyConstraints t(dim, tn, edges, clusters, vs, cs);
    COLA_ASSERT(checkDesired(dim,tn,targets,resizes));
#ifndef NDEBUG
    unsigned loopCtr=0;
#endif
    while(t.solve()) { COLA_ASSERT(++loopCtr<1000); }
    //COLA_ASSERT(checkFinal(tn,targets,resizes));
    
    // reposition and resize original nodes
    feach(nodes,CopyPositions(dim,tn,resizes));

    // revert topologyRoutes back to original nodes
    feach(edges,RevertNodes(nodes));

    COLA_ASSERT(assertConvexBends(edges));
    COLA_ASSERT(assertNoSegmentRectIntersection(nodes,edges));

    // clean up
    feach(tn,DeleteTempNode());
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
            COLA_ASSERT(target->width() > 3.0*DW);
            COLA_ASSERT(target->height() > 3.0*DW);
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
 * @param xvs horizontal vars list
 * @param xcs horizontal constraints
 * @param yvs vertical vars
 * @param ycs vertical constraints
 */
void applyResizes(Nodes& nodes, Edges& edges, RootCluster *clusters, 
        ResizeMap& resizes, Variables& xvs, Constraints& xcs, 
        Variables& yvs, Constraints& ycs) {
    // targets will hold an overlap free placement of the resized rectangles
    Rectangles targets(nodes.size());
    // rectangles that are resized should be fixed when finding overlap free placement
    // the following is populated with their ids
    set<unsigned> fixed;
    transform(nodes.begin(),nodes.end(),targets.begin(),CreateTargetRect(resizes,fixed));
    removeoverlaps(targets,fixed);
    resizeAxis(vpsc::XDIM, targets, nodes, edges, clusters, resizes, xvs, xcs);
    resizeAxis(vpsc::YDIM, targets, nodes, edges, clusters, resizes, yvs, ycs);
    feach(targets,delete_object());
}
} // namespace topology
