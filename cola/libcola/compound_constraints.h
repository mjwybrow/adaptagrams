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
#include <set>
#include <utility>

#include "libcola/sparse_matrix.h"


namespace vpsc {
    class Constraint;
    class Variable;
    typedef std::vector<vpsc::Constraint*> Constraints;
    typedef std::vector<vpsc::Variable*> Variables;
}
namespace cola {

class Cluster;
class RootCluster;


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

static const double freeWeight = 0.0001;

static const unsigned int DEFAULT_CONSTRAINT_PRIORITY = 30000;
static const unsigned int PRIORITY_NONOVERLAP = 
        DEFAULT_CONSTRAINT_PRIORITY - 2000;


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


class VariableIDMap
{
    public:
        VariableIDMap();
        ~VariableIDMap();

        bool addMappingForVariable(const unsigned from, const unsigned to);
        unsigned getMappingForVariable(const unsigned var) const;
        unsigned getReverseMappingForVariable(const unsigned var) const;
        void clear(void);
        void printCreationCode(FILE *fp) const;

    private:
        std::list<std::pair<unsigned, unsigned> > m_mapping;
};

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
     * Generate any additional variables required by this compound constraint
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
     * Create the separation constraints that will effect this
     * CompoundConstraint.
     */
    virtual void generateSeparationConstraints(const vpsc::Dim dim, 
            vpsc::Variables& var, vpsc::Constraints& cs,
            std::vector<vpsc::Rectangle*>& bbs) = 0;
    /**
     * After the vpsc instance is solved the following should be called
     * to send position information back to the interface.
     */
    virtual void updatePosition(const vpsc::Dim dim)
    {
        // Suppress unused parameter warning.
        (void) dim;
    }
    virtual ~CompoundConstraint();
    vpsc::Dim dimension(void) const;
    unsigned int priority(void) const;
    virtual void updateVarIDsWithMapping(const VariableIDMap& idMap,
            bool forward = true)
    {
    }

    // The following methods are only needed for initially solving feasibility
    // of the constraints, and do not need to be implemented for most compound
    // constraints.
    virtual void markAllSubConstraintsAsInactive(void);
    virtual bool subConstraintsRemaining(void) const;
    virtual void markCurrSubConstraintAsActive(const bool satisfiable);
    virtual SubConstraintAlternatives getCurrSubConstraintAlternatives(
            vpsc::Variables vs[]) = 0;
    std::list<unsigned> subConstraintObjIndexes(void) const;
    virtual void printCreationCode(FILE *fp) const;
    bool shouldCombineSubConstraints(void) const;

protected:
    void assertValidVariableIndex(const vpsc::Variables& vars, 
            const unsigned index);

    // The dimension that this compound constraint operates.
    vpsc::Dim _primaryDim;
    // The alternate dimension.
    vpsc::Dim _secondaryDim;
    // The priority used to assign order for solving constraints.
    unsigned int _priority;
    // Describes whether to process sub constraints individually, or all
    // at once, during the makeFeasible opteration.
    bool _combineSubConstraints;
    
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
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& cs,
        vpsc::Rectangles& bbs);


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
                vpsc::Variables& vars, vpsc::Constraints& cs,
                std::vector<vpsc::Rectangle*>& bbs);
        void updatePosition(const vpsc::Dim dim);
        void printCreationCode(FILE *fp) const;
        
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
                vpsc::Variables& vars, vpsc::Constraints& cs,
                std::vector<vpsc::Rectangle*>& bbs);
        void updatePosition(const vpsc::Dim dim);
        void fixPos(double pos);
        void unfixPos(void);
        double position(void) const;
        bool isFixed(void) const;
        void printCreationCode(FILE *fp) const;
        void updateVarIDsWithMapping(const VariableIDMap& idMap, 
                bool forward = true);
        
        /** the indicator pointer is used by dunnart to keep a ref to it's 
         * local representation of the alignment constraint
         */
        void *indicator;
        vpsc::Variable* variable;
    private:
        // The position of the alignment line
        double _position;
        bool _isFixed;
};


// A simple horizontal or vertical spacing constraint between 2 nodes
// or alignment constraints.
//
// The non-equality constraint is  lPos + g <= rPos
// and the equality constraint is  lPos + g  = rPos
//
// Note: With an equality, you can effectively reverse the ordering of the 
//       two variables by making the gap a negative number.   This is not so
//       for the non-equality case, there you need to keep the same gap value
//       but reverse the order of the variables passed to the constructor.
//
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
                vpsc::Variables& vs, vpsc::Constraints& cs,
                std::vector<vpsc::Rectangle*>& bbs);
        void setSeparation(double gap);
        unsigned left(void) const;
        unsigned right(void) const;
        void printCreationCode(FILE *fp) const;

        double gap;
        bool equality;
        vpsc::Constraint *vpscConstraint;
};


// XXX: This is experimental
//
// Orthogonal edges must have their end points aligned horizontally or 
// vertically
class OrthogonalEdgeConstraint : public CompoundConstraint 
{
    public:
        OrthogonalEdgeConstraint(const vpsc::Dim dim, unsigned l, unsigned r);
        
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vs, vpsc::Constraints& cs,
                std::vector<vpsc::Rectangle*>& bbs);
        void generateTopologyConstraints(const vpsc::Dim k, 
                std::vector<vpsc::Rectangle*> const& rs, 
                std::vector<vpsc::Variable*> const& vars, 
                std::vector<vpsc::Constraint*>& cs);
        void printCreationCode(FILE *fp) const;

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
                vpsc::Variables& vs, vpsc::Constraints& gcs,
                std::vector<vpsc::Rectangle*>& bbs);
        void setSeparation(double sep);
        void printCreationCode(FILE *fp) const;

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
                vpsc::Variables& vars, vpsc::Constraints& gcs,
                std::vector<vpsc::Rectangle*>& bbs);
        void setSeparation(double sep);
        void printCreationCode(FILE *fp) const;

        vpsc::Constraints cs;
        void *indicator;
        double sep;
};

// XXX: This is experimental
//
// A Fixed-relative constraint specifies that a group of nodes are constrained
// to be fixed in position relative to each other.  A weight can be given to 
// try to keep the whole group at the desired position.
class FixedRelativeConstraint : public CompoundConstraint {
    public:
        FixedRelativeConstraint(const vpsc::Rectangles& rs,
                std::set<unsigned> shapeIds, const bool fixedPosition = false);
        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);

        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs,
                std::vector<vpsc::Rectangle*>& bbs);
        void printCreationCode(FILE *fp) const;

        bool m_fixed_position;
        std::set<unsigned> m_shape_vars;
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
                vpsc::Variables& vars, vpsc::Constraints& gcs,
                std::vector<vpsc::Rectangle*>& bbs);
        void updatePosition(const vpsc::Dim dim);
        double getActualLeftMargin(const vpsc::Dim dim);
        double getActualRightMargin(const vpsc::Dim dim);
        void addShape(unsigned id, double halfW, double halfH);
        void printCreationCode(FILE *fp) const;

    private:
        double leftMargin[2];
        double rightMargin[2];
        double actualLeftMargin[2];
        double actualRightMargin[2];
        double leftWeight[2];    
        double rightWeight[2];  
        vpsc::Variable *vl[2], *vr[2];
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
