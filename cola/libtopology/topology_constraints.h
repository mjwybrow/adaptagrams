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
*/

/**
 * Classes used in generating and managing topology constraints, i.e.
 * constraints of the form (e.g.) \f$w_x + \frac{1}{2}\mathrm{width}(w) \le u_x
 * + (v_x - u_x) \frac {(w_y-u_y)}{(v_y-u_y)}\f$ where (u,v) is an edge segment
 * and w is a node constrained to lie to the left of that segment.  Right-,
 * above- and below-of constraints are similarly defined.
 *
 * \file topology_constraints.h
 * \author Tim Dwyer
 * \date 2007
 */
#ifndef TOPOLOGY_CONSTRAINTS_H
#define TOPOLOGY_CONSTRAINTS_H
#include <libcola/commondefs.h>
#include "libtopology/topology_graph.h"
#include <valarray>
#include <vector>
#include <map>
#include <list>
namespace vpsc {
    class Constraint;
    class Variable;
    typedef std::vector<Constraint*> Constraints;
    typedef std::vector<Variable*> Variables;
}
namespace cola {
    struct SparseMap;
    class RootCluster;
}
/**
 * namespace for classes uses in generating and solving forces and constraints associated with
 * topology preserving layout.
 */
namespace topology {
    class StraightConstraint;
    class Edge;
    /**
     * A constraint between three variables \f$u,v,w\f$ where \f$w\f$ is
     * required to be to one side of the line between \f$u,v\f$.  That is, e.g.
     * if we require \f$w\f$ to be some minimum distance \f$g\f$ to the left of
     * the parameterised distance \f$p\f$ along the line \f$(u,v)\f$ we have:
     * \f[ w + g\le u + p*(v - u) \f] Right-of constraints are similar.
     */
    class TriConstraint {
    public:
        /// A TriConstraint is associated with the positions of 3 nodes
        const Node *u, *v, *w;
        /// p is the parameter for the constraint line, g is the offset constant
        double p, g;
        /** 
         * determines direction of inequality, w to the left of uv or to the
         * right
         */
        bool leftOf;
        vpsc::Dim scanDim;
        TriConstraint(vpsc::Dim dim, const Node *u,  const Node *v, const Node *w,
                double p, double g, bool left);
        /** 
         * @return the maximum move we can make along the line from initial to
         * desired positions without violating this constraint
         */
        double maxSafeAlpha() const;
        /**
         * amount of slack at current positions of variables
         */
        double slackAtInitial() const;
        /**
         * amount of slack at desired positions of variables
         */
        double slackAtFinal() const;
        /**
         * checks initial positions
         */
        bool assertFeasible() const;
    private:
        double slack(const double, const double, const double) const;
    };
    class TopologyConstraint {
    public:
        TriConstraint* c;
        vpsc::Dim scanDim;
        /**
         * depending on the type of constraint (i.e. whether it is a constraint
         * between a segment and a node or between two segments) we either
         * split the segment (creating a new bend EdgePoint) or merge 
         * the segment with its neighbour (removing an EdgePoint).
         */
        virtual void satisfy() = 0;
        /// for log messages
        virtual std::string toString() const = 0;
        virtual unsigned getEdgeID() const = 0;
        virtual ~TopologyConstraint() {
            delete c;
        }
        /** 
         * checks the underlying TriConstraint to ensure that it is feasible
         * at the initial positions of its constituent variables.  Note that
         * these initial positions must be up-to-date before hand.
         */
        bool assertFeasible() const;
    protected:
        TopologyConstraint(vpsc::Dim dim) : c(NULL), scanDim(dim) { }
    };
    /**
     * A constraint around a bend point that becomes active when the bend
     * goes straight
     */
    class BendConstraint : public TopologyConstraint {
    public:
        EdgePoint* bendPoint;
        /**
         * create a constraint between the two segments joined by this
         * EdgePoint such that the constraint is activated when the segments
         * are aligned.
         * @param bendPoint the articulation point
         */
        BendConstraint(EdgePoint* bendPoint, vpsc::Dim dim);
        void satisfy();
        std::string toString() const;
        unsigned getEdgeID() const;
    };
    /**
     * A constraint between a Node and a Segment that is activated when
     * the Node wants to move through the Segment to create a bend point
     */
    class StraightConstraint : public TopologyConstraint {
    public:
        Segment* segment;
        Node* node;
        EdgePoint::RectIntersect ri;
        const double pos;
        /** 
         * create a constraint between a segment and one corner of a node such
         * that the constraint is activated when the segment needs to be bent
         * (divided into two new segments)
         * @param s the segment
         * @param node the node
         * @param ri the vertex of the node causing this constraint
         * @param scanPos the position of the scan line along which the
         * constraint lies
         * @param segmentPos the ratio (s->start,scanPos)/(s->start,s->end)
         */
        StraightConstraint(Segment* s, vpsc::Dim dim,
                Node* node, const EdgePoint::RectIntersect ri,
                const double scanPos, const double segmentPos,
                const bool nodeLeft);
        void satisfy();
        std::string toString() const;
        unsigned getEdgeID() const {
            return segment->edge->id;
        }
    };
    /**
     * desired positions which should override those computed by applying forces
     * are passed in for a set of nodes.  The first entry is the Node->id, the
     * second is the desired position.
     */
    typedef std::pair<unsigned,double> DesiredPosition;
    typedef std::vector<DesiredPosition> DesiredPositions;
    /**
     * Define a topology over a diagram by generating a set of
     * TopologyConstraint
     */
    class TopologyConstraints {
    public:
        const size_t n;
        /**
         * @param dim HORIZONTAL or VERTICAL
         * @param nodes topology nodes
         * @param edges topology edges
         * @param vs canonical list of variables to feed into constraint
         * solver, this must include all the variables
         * associated with nodes (ie. vs.size()>=nodes.size()) though they
         * do not have to appear in the same order.
         * @param cs constraints on variables, list will be appended with
         * automatically generated non-overlap constraints
         */
        TopologyConstraints(
            const vpsc::Dim dim, 
            Nodes& nodes,
            Edges& edges,
            cola::RootCluster* clusterHierarchy,
            vpsc::Variables& vs,
            vpsc::Constraints& cs);
        ~TopologyConstraints();
        bool solve();
        void constraints(std::vector<TopologyConstraint*> & ts) const;
        void computeForces(std::valarray<double>& g, cola::SparseMap& h);
        bool assertFeasible() const;
        void printInstance(std::valarray<double>& g) const;
        bool noOverlaps() const;
    private:
        Nodes& nodes;
        Edges& edges;
        cola::RootCluster* clusters;
        vpsc::Variables& vs;
        vpsc::Constraints& cs;
        vpsc::Dim dim;
    };
    /**
     * The following just copies variables in ns into vs.  May be useful
     * in calling the TopologyConstraints constructor if you're not interested
     * in constructing your own list of variables in advance.
     * @param ns source list of nodes
     * @param vs target list (we expect this to be 0 and resize to ns.size())
     */
    void getVariables(Nodes& ns, vpsc::Variables& vs);
    /**
     * Details new dimensions for a given rectangle.
     */
    struct ResizeInfo {
        const Node* orig;
        const vpsc::Rectangle* targetRect;
        ResizeInfo(Node* v, const vpsc::Rectangle* target)
            : orig(v), 
              targetRect(target),
              lhsNode(NULL),
              rhsNode(NULL) { };
        Node *lhsNode, *rhsNode;
    };
    typedef std::map<unsigned, ResizeInfo> ResizeMap;
    void applyResizes(Nodes& nodes, Edges& edges, 
            cola::RootCluster *clusters, ResizeMap& resizes,
            vpsc::Variables& xvs, vpsc::Constraints& xcs, 
            vpsc::Variables& yvs, vpsc::Constraints& ycs);
    /**
     * compute the p-stress over a set of edge paths:
     * \f[
     *   \sigma = \sum_{e \in E} \left( d_e - \sum_{s \in S(e)} |s| \right)^2
     * \f]
     */
    double computeStress(const Edges& es);
} // namespace topology
#endif // TOPOLOGY_CONSTRAINTS_H
