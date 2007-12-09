/**
 * \file topology_constraints.h
 * \author Tim Dwyer
 * \date 2007
 *
 * Classes used in generating and managing topology constraints, i.e.
 * constraints of the form (e.g.) \f$w_x + \frac{1}{2}\mathrm{width}(w) \le u_x
 * + (v_x - u_x) \frac {(w_y-u_y)}{(v_y-u_y)}\f$ where (u,v) is an edge segment
 * and w is a node constrained to lie to the left of that segment.  Right-,
 * above- and below-of constraints are similarly defined.
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
            : u(u), v(v), w(w), p(p), g(g), leftOf(left) {
                printf("TopologyConstraint created!\n");
        }
        /** 
         * @return the maximum move we can make along the line from initial to
         * desired positions without violating this constraint
         */
        double maxSafeAlpha() const;
        double slack () const;
        void print() const;
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
        virtual ~TopologyConstraint() {
            delete c;
        }
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
    };
    /**
     * Define a topology over a diagram by generating a set of
     * TopologyConstraint
     */
    class TopologyConstraints {
    public:
        const Edges& edges;
        TopologyConstraints(
            const cola::Dim dim, 
            project::Constraints& cs,
            const Nodes &vs,
            const Edges &es);
        ~TopologyConstraints() { }
        void violated(std::vector<TopologyConstraint*> & ts) const;
        void constraints(std::vector<TopologyConstraint*> & ts) const;
        TopologyConstraint* mostViolated() const;
        void computeForces(cola::SparseMap &H, valarray<double> &g);
        double computeStress() const;
        void finalizeRoutes();
    private:
        double len(const EdgePoint* u, const EdgePoint* v, 
                double& dx, double& dy,
                double& dx2, double& dy2);
        double gRule1(const EdgePoint* a, const EdgePoint* b);
        double gRule2(const EdgePoint* a, const EdgePoint* b, const EdgePoint* c);
        double hRuleD1(const EdgePoint* u, const EdgePoint* v, const double sqrtf);
        double hRuleD2(const EdgePoint* u, const EdgePoint* v, const EdgePoint* w, 
                const double sqrtf);
        double hRule2(const EdgePoint* u, const EdgePoint* v, const EdgePoint* w,
                const double sqrtf);
        double hRule3(const EdgePoint* u, const EdgePoint* v, const EdgePoint* w,
                const double sqrtf);
        double hRule4(const EdgePoint* a, const EdgePoint* b,
                const EdgePoint* c, const EdgePoint* d);
        double hRule56(const EdgePoint* u, const EdgePoint* v, 
                const EdgePoint* a, const EdgePoint* b, const EdgePoint* c);
        double hRule7(const EdgePoint* a, const EdgePoint* b, 
                const EdgePoint* c, const EdgePoint* d, const double sqrtf);
        double hRule8(const EdgePoint* u, const EdgePoint* v, const EdgePoint* w,
                const EdgePoint* a, const EdgePoint* b, const EdgePoint* c);
    };
} // namespace topology
#endif // TOPOLOGY_CONSTRAINTS_H
// vim: cindent ts=4 sw=4 et tw=0 wm=0
