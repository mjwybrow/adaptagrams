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
#include <exception>
#include "sparse_matrix.h"

namespace vpsc {
    class Constraint;
    class Variable;
    typedef std::vector<vpsc::Constraint*> Constraints;
    typedef std::vector<vpsc::Variable*> Variables;
}
namespace cola {

typedef std::vector<std::pair<unsigned,double> > OffsetList;


/** 
 * A compound constraint is a conceptual, diagramming application oriented
 * type of constraint, which can be translated into a set of simple
 * separation constraints, possibly extra dummy variables, and possibly
 * even some extra terms for the quadratic objective function used
 * in the gradient projection solver.
 */
class CompoundConstraint {
public:
    /**
     * generate any additional variables required by this compound constraint
     * and add them to vars.  These variables should be cleaned up by
     * the caller after the vpsc problem is solved.
     * The variables' ideal position and weight should be set and they
     * should be added to the end of vars.
     * @param vars the list of variables for the overall problem instance
     * to which any variables generated should be appended.
     */
    virtual void generateVariables(vpsc::Variables& vars) = 0;
    /**
     * create the separation constraints that will effect this
     * CompoundConstraint.
     */
	virtual void generateSeparationConstraints(
            vpsc::Variables& vars, vpsc::Constraints& cs) = 0;
    /**
     * after the vpsc instance is solved the following should be called
     * to send position information back to the interface.
     */
    virtual void updatePosition() {};
    virtual ~CompoundConstraint() {}
    
protected:
    void assertValidVariableIndex(const vpsc::Variables& vars, 
            const unsigned index);
};
typedef std::vector<CompoundConstraint*> CompoundConstraints;

class InvalidVariableIndexException: public std::exception
{
    public:
    InvalidVariableIndexException(CompoundConstraint *c, unsigned i) 
        : constraint(c),
          index(i)
    { }
    virtual const char* what() const throw()
    {
        return "Invalid variable index";
    }
    CompoundConstraint *constraint;
    unsigned index;
};


/**
 * generate all the variables and constraints for a collection of CompoundConstraint
 */
void generateVariablesAndConstraints(CompoundConstraints& ccs, vpsc::Variables& vars, vpsc::Constraints& cs);
/**
 * A boundary constraint gives a bounding line (position stored in the variable)
 * and a set of nodes required to be to the left of that boundary and
 * nodes required to be to the right of the line.  leftOffsets and
 * rightOffsets store minimum separations required b/n each of these nodes and
 * the line.
 */
class BoundaryConstraint : public CompoundConstraint {
public:
    BoundaryConstraint(double pos) : position(pos), variable(NULL) {}
    void updatePosition() {
        position = variable->finalPosition;
    }
    /**
     * just one variable is generated, associated with the position of the
     * boundary.
     */
    void generateVariables(vpsc::Variables& vars) {
        variable = new vpsc::Variable(vars.size(),position,0.0001);
        vars.push_back(variable);
    }
	void generateSeparationConstraints( vpsc::Variables& vars, vpsc::Constraints& cs);
    double position;
    OffsetList leftOffsets, rightOffsets;
    vpsc::Variable* variable;
};

/*
 * An alignment constraint specifies a group of nodes and offsets for those
 * nodes such that the nodes must be spaced exactly at those offsets from a
 * vertical or horizontal line.
 */
class AlignmentConstraint : public CompoundConstraint {
public:
    AlignmentConstraint(double pos) 
        : position(pos), 
          isFixed(false),
          variable(NULL) {}
    void updatePosition() {
        position = variable->finalPosition;
    }
    void fixPos(double pos) {
        position=pos;
        isFixed=true;
    }
    void unfixPos() {
        isFixed=false;
    }
    //! a list of pairs of node indices and their required offsets
    OffsetList offsets;
    /** the guide pointer is used by dunnart to keep a ref to it's local
     * representation of the alignment constraint
     */
    void* guide;
    // The position of the alignment line
    double position;
    bool isFixed;
    void generateVariables(vpsc::Variables& vars);
	void generateSeparationConstraints( vpsc::Variables& vars, vpsc::Constraints& cs);
    vpsc::Variable* variable;
};

// A simple horizontal or vertical spacing constraint between 2 nodes
// or alignment constraints
class SeparationConstraint : public CompoundConstraint {
public:
    SeparationConstraint(unsigned l, unsigned r, double g, bool equality = false) 
        : left(l), right(r), al(NULL), ar(NULL), gap(g), equality(equality),
          vpscConstraint(NULL)  {
    }
    SeparationConstraint(AlignmentConstraint *l, AlignmentConstraint *r, 
            double g, bool equality = false) 
        : left(0), right(0), al(l), ar(r), gap(g), equality(equality)  {
    }
    unsigned left;
    unsigned right;
    AlignmentConstraint *al;
    AlignmentConstraint *ar;
    double gap;
    bool equality;
    void generateVariables(vpsc::Variables& vars) { }
	void generateSeparationConstraints( vpsc::Variables& vs, vpsc::Constraints& cs);
    void setSeparation(double gap);
    vpsc::Constraint* vpscConstraint;
};
// Orthogonal edges must have their end points aligned horizontally or vertically
class OrthogonalEdgeConstraint : public CompoundConstraint {
public:
    OrthogonalEdgeConstraint(unsigned l, unsigned r)
        : left(l), right(r), 
          vpscConstraint(NULL)  {
    }
    unsigned left;
    unsigned right;
    void generateVariables(vpsc::Variables& vars) { }
	void generateSeparationConstraints( vpsc::Variables& vs, vpsc::Constraints& cs);
    void generateTopologyConstraints(const vpsc::Dim k, std::vector<vpsc::Rectangle*> const & rs, 
            std::vector<vpsc::Variable*> const & vars, std::vector<vpsc::Constraint*> & cs);
    vpsc::Constraint* vpscConstraint;
private:
	void rectBounds(const vpsc::Dim k, vpsc::Rectangle const *r, 
            double & cmin, double & cmax, double & centre, double & l) const;
};

// A set of horizontal or vertical spacing constraints between adjacent pairs
// of alignment constraints
class MultiSeparationConstraint : public CompoundConstraint {
public:
    MultiSeparationConstraint(double minSep=0, bool equality=false)
        : sep(minSep), equality(equality)  {
    }
    void generateVariables(vpsc::Variables& vars) { }
	void generateSeparationConstraints( vpsc::Variables& vs, vpsc::Constraints& gcs);
    void setSeparation(double sep) { this->sep = sep; }
    vpsc::Constraints cs;
    std::vector<std::pair<AlignmentConstraint*,AlignmentConstraint*> > acs;
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
    DistributionConstraint() {}
    void generateVariables(vpsc::Variables& vars) { }
	void generateSeparationConstraints(vpsc::Variables& vars, vpsc::Constraints& gcs);
    void setSeparation(double sep) {
        this->sep = sep;
    }
    vpsc::Constraints cs;
    std::vector<std::pair<AlignmentConstraint*,AlignmentConstraint*> > acs;
    void *indicator;
    double sep;
};

// creates dummy variables for the edges of the page and constraints
// between all nodes and these dummy vars such that nodes are contained
// between the edges
class PageBoundaryConstraints : public CompoundConstraint {
public:
    PageBoundaryConstraints(double lm, double rm, double w)
        : leftMargin(lm), rightMargin(rm), 
          actualLeftMargin(0), actualRightMargin(0),
          leftWeight(w), rightWeight(w), vl(NULL), vr(NULL) { }
    PageBoundaryConstraints(double lm, double rm, double lw, double rw)
        : leftMargin(lm), rightMargin(rm), 
          actualLeftMargin(0), actualRightMargin(0),
          leftWeight(lw), rightWeight(rw), vl(NULL), vr(NULL) { }
    void generateVariables(vpsc::Variables& vars);
	void generateSeparationConstraints(vpsc::Variables& vars, vpsc::Constraints& gcs);
    void updatePosition() {
        if(vl) actualLeftMargin = vl->finalPosition;
        if(vr) actualRightMargin = vr->finalPosition;
        //printf("updatePosition(): actualLeftMargin=%f, actualRightMargin=%f\n",actualLeftMargin,actualRightMargin);
    }
    double getActualLeftMargin() {
        return actualLeftMargin;
    }
    double getActualRightMargin() {
        return actualRightMargin;
    }
    OffsetList offsets;
private:
    double leftMargin;
    double rightMargin;
    double actualLeftMargin;
    double actualRightMargin;
    double leftWeight;    
    double rightWeight;  
    vpsc::Variable *vl, *vr;
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
