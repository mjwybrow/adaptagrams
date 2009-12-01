/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
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

#ifndef _COMPOUND_CONSTRAINTS_H
#define _COMPOUND_CONSTRAINTS_H
#include <vector>
#include <list>
#include <utility>
#include "sparse_matrix.h"

namespace vpsc {
    class Constraint;
    class Variable;
    typedef std::vector<vpsc::Constraint*> Constraints;
    typedef std::vector<vpsc::Variable*> Variables;
}
namespace cola {


// A component of a CompoundConstraint.
class SubConstraint
{
    public:
        SubConstraint(vpsc::Dim dim, vpsc::Constraint constraint, 
                double cost = 0)
            : dim(dim),
              constraint(constraint),
              cost(cost)
        {
        }
        bool operator<(const SubConstraint& rhs) const
        {
            return cost < rhs.cost;
        }

        vpsc::Dim dim;
        vpsc::Constraint constraint;
        double cost;
};


// A list of alternative SubConstraints. 
typedef std::list<SubConstraint> SubConstraintAlternatives;


static const unsigned int DEFAULT_CONSTRAINT_PRIORITY = 300;

class SubConstraintInfo 
{
    public:
        SubConstraintInfo(unsigned ind) :
            varIndex(ind),
            satisfied(false)
        {
        }
        unsigned varIndex;
        bool satisfied;
};

typedef std::vector<SubConstraintInfo *> SubConstraintInfoList;


/** 
 * A compound constraint is a conceptual, diagramming application oriented
 * type of constraint, which can be translated into a set of simple
 * separation constraints, possibly extra dummy variables, and possibly
 * even some extra terms for the quadratic objective function used
 * in the gradient projection solver.
 */
class CompoundConstraint {
public:
    CompoundConstraint(vpsc::Dim primaryDim, 
            unsigned int priority = DEFAULT_CONSTRAINT_PRIORITY);
    /**
     * generate any additional variables required by this compound constraint
     * and add them to vars.  These variables should be cleaned up by
     * the caller after the vpsc problem is solved.
     * The variables' ideal position and weight should be set and they
     * should be added to the end of vars.
     * @param vars the list of variables for the overall problem instance
     * to which any variables generated should be appended.
     */
    virtual void generateVariables(const vpsc::Dim dim, 
            vpsc::Variables& vars) = 0;
    /**
     * create the separation constraints that will effect this
     * CompoundConstraint.
     */
	virtual void generateSeparationConstraints(const vpsc::Dim dim, 
            vpsc::Variables& var, vpsc::Constraints& cs) = 0;
    /**
     * after the vpsc instance is solved the following should be called
     * to send position information back to the interface.
     */
    virtual void updatePosition(const vpsc::Dim dim) {};
    virtual ~CompoundConstraint();
    vpsc::Dim dimension(void) const;
    unsigned int priority(void) const;
    
    virtual void markAllSubConstraintsAsInactive(void);
    virtual bool subConstraintsRemaining(void) const;
    virtual void markCurrSubConstraintAsActive(const bool satisfiable);
    virtual SubConstraintAlternatives getCurrSubConstraintAlternatives(
            vpsc::Variables vs[]) = 0;
    std::list<unsigned> subConstraintObjIndexes(void) const;

protected:
    void assertValidVariableIndex(const vpsc::Variables& vars, 
            const unsigned index);

    // The dimension that this compound constraint operates.
    vpsc::Dim _primaryDim;
    // The alternate dimension.
    vpsc::Dim _secondaryDim;
    // The priority used to assign order for solving constraints.
    unsigned int _priority;
    
    // Info about the sub constraints within this compound constraint.
    SubConstraintInfoList _subConstraintInfo;
    // The index of the current subConstraint being made feasible.
    size_t _currSubConstraintIndex;
};
typedef std::vector<CompoundConstraint*> CompoundConstraints;


/**
 * Generate all the variables and constraints for a collection of 
 * CompoundConstraints.
 */
void generateVariablesAndConstraints(CompoundConstraints& ccs, 
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& cs);


/**
 * Generate just all the variables for a collection of CompoundConstraints.
 */
void generateVariables(CompoundConstraints& ccs, const vpsc::Dim dim, 
        vpsc::Variables& vars);


/**
 * A boundary constraint gives a bounding line (position stored in the variable)
 * and a set of nodes required to be to the left of that boundary and
 * nodes required to be to the right of the line.  leftOffsets and
 * rightOffsets store minimum separations required b/n each of these nodes and
 * the line.
 */
class BoundaryConstraint : public CompoundConstraint 
{
    public:
        BoundaryConstraint(const vpsc::Dim);
        void addShape(const unsigned int index, const double offset);

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);

        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& cs);
        void updatePosition(const vpsc::Dim dim);
        
        double position;
        vpsc::Variable* variable;
};


/*
 * An alignment constraint specifies a group of nodes and offsets for those
 * nodes such that the nodes must be spaced exactly at those offsets from a
 * vertical or horizontal line.
 */
class AlignmentConstraint : public CompoundConstraint 
{
    public:
        AlignmentConstraint(const vpsc::Dim dim, double position = 0.0);
        void addShape(const unsigned int index, const double offset);

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);

        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& cs);
        void updatePosition(const vpsc::Dim dim);
        void fixPos(double pos);
        void unfixPos(void);
        double position(void) const;
        bool isFixed(void) const;
        
        /** the guide pointer is used by dunnart to keep a ref to it's local
         * representation of the alignment constraint
         */
        void* guide;
        vpsc::Variable* variable;
    private:
        // The position of the alignment line
        double _position;
        bool _isFixed;
};


// A simple horizontal or vertical spacing constraint between 2 nodes
// or alignment constraints
class SeparationConstraint : public CompoundConstraint 
{
    public:
        SeparationConstraint(const vpsc::Dim dim, unsigned l, unsigned r, 
                double g, bool equality = false);
        SeparationConstraint(const vpsc::Dim dim, AlignmentConstraint *l, 
                AlignmentConstraint *r, double g, bool equality = false);

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vs, vpsc::Constraints& cs);
        void setSeparation(double gap);
        unsigned left(void) const;
        unsigned right(void) const;

        double gap;
        bool equality;
        vpsc::Constraint *vpscConstraint;
};


// Orthogonal edges must have their end points aligned horizontally or vertically
class OrthogonalEdgeConstraint : public CompoundConstraint 
{
    public:
        OrthogonalEdgeConstraint(const vpsc::Dim dim, unsigned l, unsigned r);
        
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vs, vpsc::Constraints& cs);
        void generateTopologyConstraints(const vpsc::Dim k, 
                std::vector<vpsc::Rectangle*> const& rs, 
                std::vector<vpsc::Variable*> const& vars, 
                std::vector<vpsc::Constraint*>& cs);

        unsigned left;
        unsigned right;
        vpsc::Constraint* vpscConstraint;
    private:
        void rectBounds(const vpsc::Dim k, vpsc::Rectangle const *r, 
                double& cmin, double& cmax, double& centre, double& l) const;
};


// A set of horizontal or vertical spacing constraints between adjacent pairs
// of alignment constraints
class MultiSeparationConstraint : public CompoundConstraint 
{
    public:
        MultiSeparationConstraint(const vpsc::Dim dim, double minSep = 0, 
                bool equality = false);
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void addAlignmentPair(AlignmentConstraint *ac1, 
                AlignmentConstraint *ac2);

        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vs, vpsc::Constraints& gcs);
        void setSeparation(double sep);

        vpsc::Constraints cs;
        void *indicator;
        double sep;
        bool equality;
};


// A distribution constraint specifies an ordered set of alignment constraints
// and a separation required between them.
// The separation can be variable (but the same between each adjacent pair of
// alignment constraints) or fixed.
class DistributionConstraint : public CompoundConstraint {
    public:
        DistributionConstraint(const vpsc::Dim dim);
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void addAlignmentPair(AlignmentConstraint *ac1, 
                AlignmentConstraint *ac2);

        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs);
        void setSeparation(double sep);

        vpsc::Constraints cs;
        void *indicator;
        double sep;
};


// creates dummy variables for the edges of the page and constraints
// between all nodes and these dummy vars such that nodes are contained
// between the edges
class PageBoundaryConstraints : public CompoundConstraint {
    public:
        PageBoundaryConstraints(double lBoundary, double rBoundary, 
                double bBoundary, double tBoundary, double w = 100.0);
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs);
        void updatePosition(const vpsc::Dim dim);
        double getActualLeftMargin(const vpsc::Dim dim);
        double getActualRightMargin(const vpsc::Dim dim);
        void addShape(unsigned id, double halfW, double halfH);

    private:
        double leftMargin[2];
        double rightMargin[2];
        double actualLeftMargin[2];
        double actualRightMargin[2];
        double leftWeight[2];    
        double rightWeight[2];  
        vpsc::Variable *vl[2], *vr[2];
};


class ShapeOffsets;
class ShapePairInfo;

// Non-overlap constraints prevent a set of given shapes from overlapping.
class NonOverlapConstraints : public CompoundConstraint {
    public:
        NonOverlapConstraints();
        void addShape(unsigned id, double halfW, double halfH);
        void computeAndSortOverlap(vpsc::Variables vs[]);
        void markCurrSubConstraintAsActive(const bool satisfiable);
        void markAllSubConstraintsAsInactive(void);
        bool subConstraintsRemaining(void) const;
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs);

    private:
        std::list<ShapePairInfo> pairInfoList;
        std::vector<ShapeOffsets> shapeOffsets;
};


/**
 * Info about constraints that could not be satisfied in gradient projection
 * process
 */
class UnsatisfiableConstraintInfo {
public:
    UnsatisfiableConstraintInfo(const vpsc::Constraint* c);
    unsigned vlid, vrid;
    double gap;
};
typedef std::vector<UnsatisfiableConstraintInfo*> UnsatisfiableConstraintInfos;
} // namespace cola
#endif // _COMPOUND_CONSTRAINTS_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
