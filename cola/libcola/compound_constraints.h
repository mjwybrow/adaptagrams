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
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

#ifndef _COMPOUND_CONSTRAINTS_H
#define _COMPOUND_CONSTRAINTS_H

#include <vector>
#include <list>
#include <set>
#include <utility>

#include "libvpsc/rectangle.h"
#include "libvpsc/constraint.h"
#include "libcola/sparse_matrix.h"
#include "libcola/unused.h"

namespace vpsc {
    class Constraint;
    class Variable;

// Avoid SWIG redefinition warnings.
#ifndef SWIG
    typedef std::vector<vpsc::Constraint *> Constraints;
    typedef std::vector<vpsc::Variable *> Variables;
#endif
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

/**
 * @brief  Holds a mapping between two sets of Variable indices.
 *
 * This can be used to rewrite the Rectangles to which a set of 
 * CompoundConstraints apply to.  This is useful when creating another
 * instance of the problem, but using the same CompoundConstraints list.
 * You should not usually need to use this yourself.  It is utilised by 
 * addons such as topology::AvoidTopologyAddon.
 *
 * If a mapping for a particular value is not set, it is considered to be
 * equal on both sides of the mapping.
 */
class VariableIDMap
{
    public:
        VariableIDMap();
        ~VariableIDMap();

        /**
         * @brief Adds a mapping between a pair of variables.
         *
         * @param[in] from  The variable index to map from.
         * @param[in] to    The variable index to map to.
         * @return          True if the mapping was successfully added.
         */
        bool addMappingForVariable(const unsigned from, const unsigned to);
        unsigned mappingForVariable(const unsigned var,
                bool forward = true) const;
        void clear(void);
        void printCreationCode(FILE *fp) const;

    private:
        std::list<std::pair<unsigned, unsigned> > m_mapping;
};

class SubConstraintInfo 
{
    public:
        SubConstraintInfo(unsigned ind) :
            varIndex(ind),
            satisfied(false)
        {
        }
        virtual ~SubConstraintInfo()
        {
        }
        virtual void updateVarIDsWithMapping(const VariableIDMap& idMap,
                bool forward);

        unsigned varIndex;
        bool satisfied;
};

typedef std::vector<SubConstraintInfo *> SubConstraintInfoList;


/** 
 * @brief An abstract base class for all high-level compound constraints.
 *
 * A compound constraint is a conceptual, diagramming application oriented
 * type of constraint, which can be translated into a set of simple
 * separation constraints, possibly extra dummy variables, and perhaps
 * even some extra terms for the quadratic objective function used
 * in the gradient projection solver.
 */
class CompoundConstraint {
public:
    CompoundConstraint(vpsc::Dim primaryDim, 
            unsigned int priority = DEFAULT_CONSTRAINT_PRIORITY);
    /**
     * @brief Implemented by the compound constraint to generate any 
     *        additional required variables in the given dimension.
     *
     * Generate any additional variables required by this compound constraint
     * when operating in the given dimension dim and add them to vars.  These
     * variables should be cleaned up by the caller after the VPSC problem is
     * solved.
     *
     * The variables' ideal position and weight should be set by the compound 
     * constraint and they should be added to the end of vars.
     *
     * @param[in]     dim   The current active dimension.
     * @param[in,out] vars  The list of variables for the overall problem 
     *                      instance to which any variables generated should 
     *                      be appended.
     */
    virtual void generateVariables(const vpsc::Dim dim, 
            vpsc::Variables& vars) = 0;
    /**
     * @brief Implemented by the compound constraint to generate the low-level
     *        separation constraints in the given dimension.
     *
     * These constraints will be added to the list of constraints cs.
     *
     * @param[in]     dim   The current active dimension.
     * @param[in]     vars  The list of variables for the overall problem 
     *                      instance.
     * @param[in,out] cs    The list of constraints to which the generated 
     *                      constraints will be added.
     * @param[in]     bbs   The list of bounding boxes for all rectangles 
     *                      in the current problem.  It has the same order as
     *                      vars.
     */
    virtual void generateSeparationConstraints(const vpsc::Dim dim, 
            vpsc::Variables& var, vpsc::Constraints& cs,
            vpsc::Rectangles& bbs) = 0;
    /**
     * @brief Implemented by the compound constraint to send position
     *        information back to the interface.
     *
     * This will be called for each compound constraint once the VPSC 
     * instance is solved to allow them to pass information such as 
     * variable values back to the graphical user interface.
     *
     * @param[in]     dim   The current active dimension.
     */
    virtual void updatePosition(const vpsc::Dim dim)
    {
        COLA_UNUSED(dim);
    }

    /**
     *  @brief Returns a textual description of the compound constraint.
     *
     *  @return     A string describing the compound constraint.
     */
    virtual std::string toString(void) const = 0;

// To prevent C++ objects from being destroyed in garbage collected languages
// when the libraries are called from SWIG, we hide the declarations of the
// destructors and prevent generation of default destructors.
#ifndef SWIG
    virtual ~CompoundConstraint();
#endif

    vpsc::Dim dimension(void) const;
    unsigned int priority(void) const;
    virtual void updateVarIDsWithMapping(const VariableIDMap& idMap,
            bool forward = true);
    virtual void updateShapeOffsetsForDifferentCentres(
                const std::vector<double>& offsets, bool forward = true)
    {
        COLA_UNUSED(offsets);
        COLA_UNUSED(forward);
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
//! @brief A vector of pointers to CompoundConstraint objects.
typedef std::vector<CompoundConstraint *> CompoundConstraints;


/**
 * @brief Generate all the variables and constraints for a collection of 
 *        CompoundConstraints.
 */
void generateVariablesAndConstraints(CompoundConstraints& ccs, 
        const vpsc::Dim dim, vpsc::Variables& vars, vpsc::Constraints& cs,
        vpsc::Rectangles& bbs);


/**
 * @brief Generate just all the variables for a collection of 
 *        CompoundConstraints.
 */
void generateVariables(CompoundConstraints& ccs, const vpsc::Dim dim, 
        vpsc::Variables& vars);


/**
 * @brief A boundary constraint specifies a bounding line that a set of nodes 
 *        must be either to the left or right of.
 *
 * A boundary constraint gives a bounding line in a particular dimension (with
 * position stored in a variable) and a set of nodes required to be to the left
 * of that line and nodes required to be to the right of the line.  Separations
 * are determined by offsets passed to addShape().
 */
class BoundaryConstraint : public CompoundConstraint 
{
    public:
        /**
         * @brief Constructs a new BoundaryConstraint in the specified 
         *        dimension.
         *
         * @param[in] dim The dimension the constraints will operate in.
         */
        BoundaryConstraint(const vpsc::Dim dim);
        /**
         * @brief Mark a node as being part of this boundary constraint.
         *
         * @param[in] index   The index of the node in the Rectangles vector.
         * @param[in] offset  The minimum amount to separate the node from the
         *                    boundary line.  Negative if left-of, positive if 
         *                    right-of.  Will usually be equal to half a node's
         *                    size plus a buffer amount.
         */
        void addShape(const unsigned int index, const double offset);

        /**
         *  @brief Returns a textual description of the compound constraint.
         *
         *  @return     A string describing the compound constraint.
         */
        std::string toString(void) const;

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& cs,
                vpsc::Rectangles& bbs);
        void updatePosition(const vpsc::Dim dim);
        void printCreationCode(FILE *fp) const;
        
        //! Holds the position of the boundary line, once layout is complete.
        double position;

        vpsc::Variable* variable;
};


/**
 * @brief An alignment constraint specifies a alignment line that a set of
 *        nodes must be constrained to by an exact amount.
 *
 * This is represented as a variable representing the position of a vertical 
 * or horizontal and a then group of nodes and offsets for those nodes such 
 * that the nodes must be spaced exactly at those offsets from the alignment
 * position.
 *
 * Optionally, the alignment may be given a suggested position and/or marked 
 * as "fixed".  When fixed, the position variable will be given a higher 
 * weight to attempt to keep it at that position.
 */
class AlignmentConstraint : public CompoundConstraint 
{
    public:
        /**
         * @brief Constructs a new AlignmentConstraint in the specified 
         *        dimension.
         *
         * @param[in] dim The dimension the constraints will operate in.
         */
        AlignmentConstraint(const vpsc::Dim dim, double position = 0.0);
        /**
         * @brief Mark a node as being part of this alignment constraint.
         *
         * @param[in] index   The index of the node in the Rectangles vector.
         * @param[in] offset  The exact amount to separate the node from the
         *                    alignment line.  Negative if left-of, positive if 
         *                    right-of.  Will usually be equal to half a node's
         *                    size if aligning to the side of a node, or zero
         *                    if aligning with the centre of a node.
         */
        void addShape(const unsigned int index, const double offset);
        /**
         * @brief Mark the alignment as ideally having a fixed position.
         *
         * This causes the position variable for the alignment to be given
         * the ideal position pos and a higher weight.
         *
         * @param[in] pos  The ideal position value for the alignment.
         */
        void fixPos(double pos);
        /**
         * @brief Mark the alignment as not having a fixed position.
         *
         * This is the default.
         */
        void unfixPos(void);
        /**
         * @brief Indicates if the alignment position is marked as fixed.
         *
         * @returns True if the alignment position is marked as fixed, or 
         *          false otherwise.
         */
        bool isFixed(void) const;

        /**
         *  @brief Returns a textual description of the compound constraint.
         *
         *  @return     A string describing the compound constraint.
         */
        std::string toString(void) const;

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& cs,
                vpsc::Rectangles& bbs);
        void updatePosition(const vpsc::Dim dim);
        double position(void) const;
        void printCreationCode(FILE *fp) const;
        void updateShapeOffsetsForDifferentCentres(
                const std::vector<double>& offsets, bool forward = true);
        
        /// Generic pointer to an indicator object being used to represent 
        /// this compound constraint in the GUI.
        void *indicator;
        
        vpsc::Variable* variable;
    private:
        // The position of the alignment line
        double _position;
        bool _isFixed;
};


/**
 * @brief A separation constraint specifies a simple horizontal or vertical 
 *        spacing constraint between 2 nodes or alignment constraints.
 *
 * The non-equality constraint is  lPos + g <= rPos
 * and the equality constraint is  lPos + g  = rPos
 *
 * @note With an equality, you can effectively reverse the ordering of the 
 *       two variables by making the gap a negative number.  This is not so
 *       for the non-equality case, there you need to keep the same gap value
 *       but reverse the order of the variables passed to the constructor.
 */
class SeparationConstraint : public CompoundConstraint 
{
    public:
        /**
         * @brief Constructs a new SeparationConstraint between two nodes in
         *        the specified dimension.
         *
         * The constraint will keep the centre of the left node to the left of
         * the right node by exactly or more than the specified gap.
         *
         * @param[in] dim      The dimension the constraint will operate in.
         * @param[in] l        The index of the left node.
         * @param[in] r        The index of the right node.
         * @param[in] g        The minimum or exact distance to separate the
         *                     two nodes.
         * @param[in] equality Whether or not the constraint is an exact 
         *                     distance.
         */
        SeparationConstraint(const vpsc::Dim dim, unsigned l, unsigned r, 
                double g, bool equality = false);
        /**
         * @brief Constructs a new SeparationConstraint between two alignment 
         *        constraints in the specified dimension.
         *
         * The constraint will keep the centre of the left alignment line to 
         * the left of the right alignment line  by exactly or more than the 
         * specified gap.
         *
         * @param[in] dim      The dimension the constraint will operate in.
         * @param[in] l        A pointer to the left AlignmentConstraint.
         * @param[in] r        A pointer to the right AlignmentConstraint.
         * @param[in] g        The minimum or exact distance to separate the
         *                     two alignment constraints.
         * @param[in] equality Whether or not the constraint is an exact 
         *                     distance.
         */
        SeparationConstraint(const vpsc::Dim dim, AlignmentConstraint *l, 
                AlignmentConstraint *r, double g, bool equality = false);

        /**
         *  @brief Returns a textual description of the compound constraint.
         *
         *  @return     A string describing the compound constraint.
         */
        std::string toString(void) const;

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vs, vpsc::Constraints& cs,
                vpsc::Rectangles& bbs);
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
                vpsc::Rectangles& bbs);
        void generateTopologyConstraints(const vpsc::Dim k, 
                vpsc::Rectangles const& rs, 
                std::vector<vpsc::Variable*> const& vars, 
                std::vector<vpsc::Constraint*>& cs);
        void printCreationCode(FILE *fp) const;
        std::string toString(void) const;

        unsigned left;
        unsigned right;
        vpsc::Constraint* vpscConstraint;
    private:
        void rectBounds(const vpsc::Dim k, vpsc::Rectangle const *r, 
                double& cmin, double& cmax, double& centre, double& l) const;
};


/**
 * @brief A multi-separation constraint Specifies a set of horizontal or 
 *        vertical equal spacing constraints between pairs of alignment 
 *        constraints.
 *
 * This is a way of arranging a group of alignment lines to be equally 
 * distributed, or given a uniform minimum spacing.
 */
class MultiSeparationConstraint : public CompoundConstraint 
{
    public:
        /**
         * @brief Constructs a new empty MultiSeparationConstraint with a 
         *        minimum or exact spacing.
         *
         * @param[in] dim      The dimension the constraints will operate in.
         * @param[in] minSep   The minimum or exact distance to separate the
         *                     alignment constraints.
         * @param[in] equality Whether or not the constraints will be an exact 
         *                     distance.
         */
        MultiSeparationConstraint(const vpsc::Dim dim, double minSep = 0, 
                bool equality = false);
        /**
         * @brief Mark a pair of alignment constraints as being part of this 
         *        multi separation constraint.
         *
         * You will often specify spacing beteen a set of alignments (e.g., 
         * {1, 2, 3, 4}) by calling this method with each neighbouring pair 
         * (e.g., {(1, 2), (2, 3), (3, 4)}), but you can also specify 
         * non-neighbouring alignment constraints, if you wish them to have 
         * equal exact or minimum separation.
         *
         * @param[in] ac1  A pointer to the left AlignmentConstraint object
         *                 of the pair.
         * @param[in] ac2  A pointer to the right AlignmentConstraint object
         *                 of the pair.
         */
        void addAlignmentPair(AlignmentConstraint *ac1, 
                AlignmentConstraint *ac2);
        /**
         * @brief Alter the minimum or exact spacing between each pair of
         *        alignment constraints.
         *
         * @param[in] sep  The minimum or exact distance to separate the
         *                 alignment constraints.
         */
        void setSeparation(double sep);

        /**
         *  @brief Returns a textual description of the compound constraint.
         *
         *  @return     A string describing the compound constraint.
         */
        std::string toString(void) const;

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vs, vpsc::Constraints& gcs,
                vpsc::Rectangles& bbs);
        void printCreationCode(FILE *fp) const;

        vpsc::Constraints cs;
        
        /// Generic pointer to an indicator object being used to represent 
        /// this compound constraint in the GUI.
        void *indicator;
        
        double sep;
        bool equality;
};


/**
 * @brief A distribution constraint specifies an ordered set of alignment 
 *        constraints and a fixed separation required between them.
 *
 * This compound constraint it used to keep a set of alignment constraints
 * equally distributed.
 *
 * If no separation distance is set, then it is detemined from the distance 
 * between the two outer alignments, divided by the number of alignments - 1.
 */
class DistributionConstraint : public CompoundConstraint {
    public:
        /**
         * @brief Constructs a new empty DistributionConstraint with a 
         *        minimum or exact spacing.
         *
         * @param[in] dim      The dimension the constraints will operate in.
         */
        DistributionConstraint(const vpsc::Dim dim);
        /**
         * @brief Mark a pair of alignment constraints as being part of this 
         *        distribution constraint.
         *
         * You should specify spacing beteen a set of alignments (e.g., 
         * {1, 2, 3, 4}) by calling this method with each neighbouring pair 
         * (e.g., {(1, 2), (2, 3), (3, 4)}).
         *
         * @param[in] ac1  A pointer to the left AlignmentConstraint object 
         *                 of the pair.
         * @param[in] ac2  A pointer to the right AlignmentConstraint object
         *                 of the pair.
         */
        void addAlignmentPair(AlignmentConstraint *ac1, 
                AlignmentConstraint *ac2);
        /**
         * @brief Alter the exact spacing between each pair of alignment 
         *        constraints.
         *
         * @param[in] sep  The exact distance to separate the alignment
         *                 constraints.
         */
        void setSeparation(double sep);

        /**
         *  @brief Returns a textual description of the compound constraint.
         *
         *  @return     A string describing the compound constraint.
         */
        std::string toString(void) const;

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs,
                vpsc::Rectangles& bbs);
        void printCreationCode(FILE *fp) const;

        vpsc::Constraints cs;
        
        /// Generic pointer to an indicator object being used to represent 
        /// this compound constraint in the GUI.
        void *indicator;
        
        double sep;
};

/**
 * @brief A fixed-relative constraint specifies that a group of nodes are
 *        constrained to be fixed in position relative to each other.  
 *
 * These nodes are fixed relative to each other in both the x- and y-dimensions
 * but still free to move as a group.
 *
 * Optionally, this compound constraint can be marked as desiring a fixed
 * position.  If this is specified, the group of nodes will attempt to stay
 * close to its current position.
 */
class FixedRelativeConstraint : public CompoundConstraint {
    public:
        /**
         * @brief Constructs a new FixedRelativeConstraint between a set of 
         *        nodes, optionally with a fixed position.
         *
         * @param[in] rs        The list of bounding boxes for the rectangles
         *                      for all nodes in the current problem.
         * @param[in] shapeIds  A vector of indices into the rc vector for the 
         *                      nodes that will be fixed relative to each other.
         * @param[in] fixedPosition  Whether of not the nodes should ideally be
         *                           fixed to the current position.  The default
         *                           is not fixed.
         */
        FixedRelativeConstraint(const vpsc::Rectangles& rs,
                std::vector<unsigned> shapeIds, 
                const bool fixedPosition = false);

        /**
         *  @brief Returns a textual description of the compound constraint.
         *
         *  @return     A string describing the compound constraint.
         */
        std::string toString(void) const;

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs,
                vpsc::Rectangles& bbs);
        void printCreationCode(FILE *fp) const;
        void updateVarIDsWithMapping(const VariableIDMap& idMap,
            bool forward = true);

    private:
        bool m_fixed_position;
        std::vector<unsigned> m_shape_vars;
};


/**
 * @brief A page boundary contraint specifies constraints that attempt to keep
 *        the given nodes within a defined rectangular region.
 *
 * This compound constraint creates dummy variables for each of the four edges 
 * of the page and constraints between all nodes and these dummy vars such 
 * that nodes are contained between the edges.  The variables for the page 
 * edges have a high weight but will "balloon out" if other constraints force
 * nodes to stick out past the ideal edge positions.
 */
class PageBoundaryConstraints : public CompoundConstraint {
    public:
        /**
         * @brief  Constructs a new PageBoundaryConstraints object with given
         *         page boundary positions and weight.
         *
         * @param[in] xLow    The position of the left edge of the page.
         * @param[in] xHigh   The position of the right edge of the page.
         * @param[in] yLow    The position of the bottom edge of the page.
         * @param[in] yHigh   The position of the top edge of the page.
         * @param[in] weight  The weight to give the positions variables
         *                    for the page edges.  The default is 100.0.
         */
        PageBoundaryConstraints(double xLow, double xHigh, 
                double yLow, double yHigh, double weight = 100.0);
        /**
         * @brief Mark a node as being contained within this page boundary.
         *
         * @param[in] index   The index of the node in the Rectangles vector.
         * @param[in] halfW   Half of the width of the node.  Needed because
         *                    node position variables represent their centre.
         * @param[in] halfH   Half of the height of the node.  
         */
        void addShape(unsigned index, double halfW, double halfH);

        /**
         *  @brief Returns a textual description of the compound constraint.
         *
         *  @return     A string describing the compound constraint.
         */
        std::string toString(void) const;

        SubConstraintAlternatives getCurrSubConstraintAlternatives(
                vpsc::Variables vs[]);
        void generateVariables(const vpsc::Dim dim, vpsc::Variables& vars);
        void generateSeparationConstraints(const vpsc::Dim dim, 
                vpsc::Variables& vars, vpsc::Constraints& gcs,
                vpsc::Rectangles& bbs);
        void updatePosition(const vpsc::Dim dim);
        double getActualLeftMargin(const vpsc::Dim dim);
        double getActualRightMargin(const vpsc::Dim dim);
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
 * @brief Info about constraints that could not be satisfied in gradient 
 *        projection process.
 */
class UnsatisfiableConstraintInfo {
    public:
        UnsatisfiableConstraintInfo(const vpsc::Constraint* c);
        
        //! The index of the left variable.
        unsigned leftVarIndex;
        //! The index of the right variable.
        unsigned rightVarIndex;
        //! The separation.
        double separation;
        //! Whether the separation is an exact distance or not.
        bool equality;
        //! The index of the CompoundConstraint that created this.
        cola::CompoundConstraint *cc;

        std::string toString(void) const
        {
            std::stringstream stream;
            stream << "Unsatisfiable constraint: var(" << leftVarIndex << ") ";
            if (separation < 0)
            {
                stream << "- " << -separation;
            }
            else
            {
                stream << "+ " << separation;
            }
            stream << " " << ((equality) ? "== " : "<= ");
            stream << "var(" << rightVarIndex << ")";
            if (cc)
            {
                stream << "\n   From " << cc->toString();
            }

            return stream.str();
        }
};
//! @brief A vector of pointers to UnsatisfiableConstraintInfo objects.
typedef std::vector<UnsatisfiableConstraintInfo *> UnsatisfiableConstraintInfos;

} // namespace cola
#endif // _COMPOUND_CONSTRAINTS_H
