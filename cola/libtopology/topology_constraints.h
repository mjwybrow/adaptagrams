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
#include "topology_graph.h"
#include <vector>
#include <set>
#include <list>
namespace cola {
    class SparseMap;
}
namespace project {
    class Desired;
    class Variable;
}
/**
 * namespace for classes uses in generating and solving forces and constraints associated with
 * topology preserving layout.
 */
namespace topology {
    extern cola::Dim dim;
    using std::valarray;
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
        /// Variables are directly entered into the libproject solver
        project::Variable *u, *v, *w;
        /// p is the parameter for the constraint line, g is the offset constant
        double p, g;
        /** 
         * determines direction of inequality, w to the left of uv or to the
         * right
         */
        bool leftOf;
        TriConstraint(
                project::Variable *u, 
                project::Variable *v, 
                project::Variable *w, 
                double p, double g, bool left)
            : u(u), v(v), w(w), p(p), g(g), leftOf(left) { }
        /** 
         * @return the maximum move we can make along the line from initial to
         * desired positions without violating this constraint
         */
        double maxSafeAlpha() const;
        /**
         * amount of slack at current positions of variables
         */
        double slack() const;
        /**
         * amount of slack at desired positions of variables
         */
        double slackAtDesired() const;
    private:
        double slack(const double, const double, const double) const;
    };
    class TopologyConstraint {
    public:
        TriConstraint* c;
        /**
         * depending on the type of constraint (i.e. whether it is a constraint
         * between a segment and a node or between two segments) we either
         * split the segment (creating a new bend EdgePoint) or merge 
         * the segment with its neighbour (removing an EdgePoint).
         */
        virtual void satisfy() = 0;
        /// for log messages
        virtual std::string toString() = 0;
        virtual ~TopologyConstraint() {
            delete c;
        }
    protected:
        TopologyConstraint() : c(NULL) { }
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
        BendConstraint(EdgePoint* bendPoint);
        void satisfy();
        std::string toString();
    };
    /**
     * A constraint between a Node and a Segment that is activated when
     * the Node wants to move through the Segment to create a bend point
     */
    class StraightConstraint : public TopologyConstraint {
    public:
        Segment* segment;
        const Node* node;
        const double pos;
        EdgePoint::RectIntersect ri;
        /** 
         * create a constraint between a segment and one corner of a node such
         * that the constraint is activated when the segment needs to be bent
         * (divided into two new segments)
         * @param s the segment
         * @param node the node
         * @param pos the position of the scan line along which the constraint
         * lies
         */
        StraightConstraint(Segment* s, const Node* node, double pos);
        void satisfy();
        std::string toString();
    };
    /**
     * desired positions which should override those computed by applying forces
     * are passed in for a set of nodes.  The first entry is the Node->id, the
     * second is the desired position.
     */
    typedef std::pair<unsigned,project::Desired> DesiredPosition;
    typedef std::vector<DesiredPosition> DesiredPositions;
    /**
     * Define a topology over a diagram by generating a set of
     * TopologyConstraint
     */
    class TopologyConstraints {
    public:
        const size_t n;
        TopologyConstraints(
            const cola::Dim dim, 
            Nodes &vs,
            Edges &es,
            project::Constraints& cs);
        ~TopologyConstraints();
        void violated(std::vector<TopologyConstraint*> & ts) const;
        void constraints(std::vector<TopologyConstraint*> & ts) const;
        TopologyConstraint* mostViolated() const;
        void computeForces(valarray<double>& g, cola::SparseMap& h);
        double computeStress() const;
        void steepestDescent(valarray<double>& g, 
                cola::SparseMap& h);
        void steepestDescent(valarray<double>& g, 
                cola::SparseMap& h, 
                const DesiredPositions& d);
        double reachedDesired(const DesiredPositions& d);
        void printInstance() const;
    private:
        Nodes& nodes;
        Edges& edges;
        project::Constraints& cs;
    };
} // namespace topology
#endif // TOPOLOGY_CONSTRAINTS_H
// vim: cindent ts=4 sw=4 et tw=0 wm=0
