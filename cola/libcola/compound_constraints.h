#ifndef _COMPOUND_CONSTRAINTS_H
#define _COMPOUND_CONSTRAINTS_H
#include <vector>
#include "sparse_matrix.h"

namespace vpsc {
    class Constraint;
    class Variable;
}
namespace cola {
using std::make_pair;
using std::vector;
	
typedef vector<vpsc::Constraint*> Constraints;
typedef vector<vpsc::Variable*> Variables;
typedef vector<std::pair<unsigned,double> > OffsetList;

// A compound constraint is a conceptual, diagramming application oriented
// type of constraint, which can be translated into a set of simple
// separation constraints, possibly extra dummy variables, and possibly
// even some extra terms for the quadratic objective function used
// in the gradient projection solver.
class CompoundConstraint {
public:
    virtual void generateVariables(Variables& vars) = 0;
	virtual void generateSeparationConstraints(
            Variables& vars, Constraints& cs) = 0;
    virtual void updatePosition() {};
    virtual ~CompoundConstraint() {}
};
typedef vector<CompoundConstraint*> CompoundConstraints;
// A boundary constraint gives a bounding line (position stored in the variable)
// and a set of nodes required to be to the left of that boundary and
// nodes required to be to the right of the line.  leftOffsets and
// rightOffsets store minimum separations required b/n each of these nodes and
// the line.
class BoundaryConstraint : public CompoundConstraint {
public:
    BoundaryConstraint(double pos) : position(pos), variable(NULL) {}
    void updatePosition() {
        position = variable->position();
    }
    void generateVariables(Variables& vars) {
        variable = new vpsc::Variable(vars.size(),position,0.0001);
        vars.push_back(variable);
    }
	void generateSeparationConstraints(
            Variables& vars, 
            Constraints& cs) {
        for(OffsetList::iterator o=leftOffsets.begin();
                o!=leftOffsets.end(); o++) {
            cs.push_back(
                    new vpsc::Constraint(
                        vars[o->first],variable,o->second));
        }
        for(OffsetList::iterator o=rightOffsets.begin();
                o!=rightOffsets.end(); o++) {
            cs.push_back(
                    new vpsc::Constraint(
                        variable,vars[o->first],o->second));
        }
    }
    double position;
    OffsetList leftOffsets, rightOffsets;
    vpsc::Variable* variable;
};

// An alignment constraint specifies a group of nodes and offsets for those nodes
// such that the nodes must be spaced exactly at those offsets from a vertical or
// horizontal line.
class AlignmentConstraint : public CompoundConstraint {
public:
    AlignmentConstraint(double pos) 
        : position(pos), 
          fixed(false),
          variable(new vpsc::Variable(0,position,0.0001)
        ) {}
    void updatePosition() {
        position = variable->position();
    }
    void fixPos(double pos) {
        variable->desiredPosition=pos;
        variable->weight=100000.;
        variable->fixedDesiredPosition=true;
        fixed=true;
    }
    void unfixPos() {
        variable->weight=1.;
        fixed=false;
        variable->fixedDesiredPosition=false;
    }
    // a list of pairs of node indices and their required offsets
    OffsetList offsets;
    // the guide pointer is used by dunnart to keep a ref to it's local representation
    // of the alignment constraint
    void* guide;
    // The position of the alignment line
    double position;
    bool fixed;
    void generateVariables(Variables& vars) {
        variable->id=vars.size();
        if(fixed) {
            variable->fixedDesiredPosition=true;
            variable->weight=100000;
        }
        vars.push_back(variable);
    }
	void generateSeparationConstraints(
            Variables& vars, 
            Constraints& cs) {
        for(OffsetList::iterator o=offsets.begin();
                o!=offsets.end(); o++) {
            cs.push_back(
                    new vpsc::Constraint(
                        variable,vars[o->first],o->second,true));
        }
    }
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
    void generateVariables(Variables& vars) { }
	void generateSeparationConstraints(
            Variables& vs, 
            Constraints& cs) {
        if(al) {
            left=al->variable->id;
        }
        if(ar) {
            right=ar->variable->id;
        }
        vpscConstraint = new vpsc::Constraint(vs[left],vs[right],gap,equality);
        cs.push_back(vpscConstraint);
    }
    void setSeparation(double gap) {
        this->gap = gap;
        if(vpscConstraint!=NULL) {
            vpscConstraint->gap = gap;
        }
    }
    vpsc::Constraint* vpscConstraint;
};

// A set of horizontal or vertical spacing constraints between adjacent pairs
// of alignment constraints
class MultiSeparationConstraint : public CompoundConstraint {
public:
    MultiSeparationConstraint(double minSep=0, bool equality=false)
        : sep(minSep), equality(equality)  {
    }
    void generateVariables(Variables& vars) { }
	void generateSeparationConstraints(
            Variables& vs, 
            Constraints& gcs) {
        for(vector<std::pair<
                AlignmentConstraint*,AlignmentConstraint*> >::iterator iac
                =acs.begin(); iac!=acs.end();++iac) {
            AlignmentConstraint *c1, *c2;
            c1=iac->first;
            c2=iac->second;
            vpsc::Constraint* c = new vpsc::Constraint(
                c1->variable,c2->variable,sep,equality);
            gcs.push_back(c);
            cs.push_back(c);
        }
    }
    void setSeparation(double sep) {
        this->sep = sep;
        for(unsigned i=0;i<cs.size();i++) {
            cs[i]->gap = sep;
        }
    }
    Constraints cs;
    vector<std::pair<AlignmentConstraint*,AlignmentConstraint*> > acs;
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
    void generateVariables(Variables& vars) { }
	void generateSeparationConstraints(
            Variables& vars, 
            Constraints& gcs) {
        cs.clear();
        for(vector<std::pair<
                AlignmentConstraint*,AlignmentConstraint*> >::iterator iac
                =acs.begin(); iac!=acs.end();++iac) {
            AlignmentConstraint *c1, *c2;
            c1=iac->first;
            c2=iac->second;
            assert(c1->variable!=NULL);
            vpsc::Constraint* c=new vpsc::Constraint(
                    c1->variable,c2->variable,sep,true);
            gcs.push_back(c);
            cs.push_back(c);
/*
            //The following was an experiment to allow variable distributions 
            //solved by optimisation rather than satisfying constraints
            if(isVariable) {
                // set second derivatives of:
                // (u + g - v)^2 = g^2 + 2gu + u^2 - 2gv - 2uv + v^2
                (*Q)[make_pair(c1->variable->id,c1->variable->id)]+=w;
                (*Q)[make_pair(c2->variable->id,c2->variable->id)]+=w;
                (*Q)[make_pair(variable->id,variable->id)]+=w;
                (*Q)[make_pair(c1->variable->id,c2->variable->id)]-=w;
                (*Q)[make_pair(c2->variable->id,c1->variable->id)]-=w;
                (*Q)[make_pair(c1->variable->id,variable->id)]+=w;
                (*Q)[make_pair(variable->id,c1->variable->id)]+=w;
                (*Q)[make_pair(c2->variable->id,variable->id)]-=w;
                (*Q)[make_pair(variable->id,c2->variable->id)]-=w;
            }
*/
        }
    }
    void setSeparation(double sep) {
        this->sep = sep;
        for(unsigned i=0;i<cs.size();i++) {
            cs[i]->gap = sep;
        }
    }
    Constraints cs;
    vector<std::pair<AlignmentConstraint*,AlignmentConstraint*> > acs;
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
    PageBoundaryConstraints(double lm, double rm, double lw, double rw) // magmy20070509: New constructor with separate edge weights
        : leftMargin(lm), rightMargin(rm), 
          actualLeftMargin(0), actualRightMargin(0),
          leftWeight(lw), rightWeight(rw), vl(NULL), vr(NULL) { }
    void generateVariables(Variables& vars) { 
        // create 2 dummy vars, based on the dimension we are in
        if(leftWeight) {
            vars.push_back(vl=new vpsc::Variable(vars.size(), leftMargin, leftWeight));
            vl->fixedDesiredPosition=true;
        } // end if
        if(rightWeight) {
            vars.push_back(vr=new vpsc::Variable(vars.size(), rightMargin, rightWeight));
            vr->fixedDesiredPosition=true;
        } // end if
    }
	void generateSeparationConstraints(
            Variables& vs, 
            Constraints& cs) {

        // for each of the "real" variables, create a constraint that puts that var
        // between our two new dummy vars, depending on the dimension.
        for(OffsetList::iterator o=offsets.begin(); o!=offsets.end(); ++o)  {
            if(vl)
                cs.push_back(new vpsc::Constraint(vl, vs[o->first], o->second));
            if(vr)
                cs.push_back(new vpsc::Constraint(vs[o->first], vr, o->second));
        }
    }
    void updatePosition() {
        if(vl) actualLeftMargin = vl->position();
        if(vr) actualRightMargin = vr->position();
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
    double leftWeight;        // magmy20070508: Split weight member into separate weights for left and right
    double rightWeight;       // - " -
    vpsc::Variable *vl, *vr;
};
} // namespace cola
#endif // _COMPOUND_CONSTRAINTS_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
