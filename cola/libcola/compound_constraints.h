#ifndef _COMPOUND_CONSTRAINTS_H
#define _COMPOUND_CONSTRAINTS_H
#include <vector>
#include "sparse_matrix.h"

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
	virtual void generateSeparationConstraints(
            Variables& vars, Constraints& cs,
            cola::SparseMap* &Q) = 0;
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
	void generateSeparationConstraints(
            Variables& vars, 
            Constraints& cs,
            cola::SparseMap* &Q) {
        variable = new vpsc::Variable(vars.size(),position,0.0001);
        vars.push_back(variable);
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
    AlignmentConstraint(double pos) : position(pos), fixed(false), variable(NULL) {}
    void updatePosition() {
        position = variable->position();
    }
    void fixPos(double pos) {
        variable->desiredPosition=pos;
        variable->weight=100000.;
        fixed=true;
    }
    void unfixPos() {
        variable->weight=1.;
        fixed=false;
    }
    // a list of pairs of node indices and their required offsets
    OffsetList offsets;
    // the guide pointer is used by dunnart to keep a ref to it's local representation
    // of the alignment constraint
    void* guide;
    // The position of the alignment line
    double position;
    bool fixed;
	void generateSeparationConstraints(
            Variables& vars, 
            Constraints& cs,
            cola::SparseMap* &Q) {
        variable = new vpsc::Variable(vars.size(),position,0.0001);
        vars.push_back(variable);
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
        : left(l), right(r), al(NULL), ar(NULL), gap(g), equality(equality)  {
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
	void generateSeparationConstraints(
            Variables& vs, 
            Constraints& cs,
            cola::SparseMap* &Q) {
        if(al) {
            left=al->variable->id;
        }
        if(ar) {
            right=ar->variable->id;
        }
        cs.push_back(new vpsc::Constraint(vs[left],vs[right],gap,equality));
    }
};

// A distribution constraint specifies an ordered set of alignment constraints
// and a separation required between them.
// The separation can be variable (but the same between each adjacent pair of
// alignment constraints) or fixed.
class DistributionConstraint : public CompoundConstraint {
public:
    static const double w = -10.;
    DistributionConstraint(bool isVariable = false) 
        : isVariable(isVariable) {}
	void generateSeparationConstraints(
            Variables& vars, 
            Constraints& cs,
            cola::SparseMap* &Q) {
        if(isVariable) {
            if(!Q) {
                Q = new cola::SparseMap(vars.size());
            }
            variable=new vpsc::Variable(vars.size(),sep,0.0001);
            vars.push_back(variable);
            Q->n=vars.size();
        }
        for(vector<std::pair<
                AlignmentConstraint*,AlignmentConstraint*> >::iterator iac
                =acs.begin(); iac!=acs.end();++iac) {
            AlignmentConstraint *c1, *c2;
            c1=iac->first;
            c2=iac->second;
            assert(c1->variable!=NULL);
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
            } else {
                cs.push_back(new vpsc::Constraint(
                        c1->variable,c2->variable,sep,true));
            }
        }
    }
    vector<std::pair<AlignmentConstraint*,AlignmentConstraint*> > acs;
    double sep;
    bool isVariable;
    vpsc::Variable* variable;
};

// creates dummy variables for the edges of the page and constraints
// between all nodes and these dummy vars such that nodes are contained
// between the edges
class PageBoundaryConstraints : public CompoundConstraint {
public:
    PageBoundaryConstraints(double lm, double rm, double w)
        : leftMargin(lm), rightMargin(rm), 
          actualLeftMargin(0), actualRightMargin(0),
          weight(w), vl(NULL), vr(NULL) { }
	void generateSeparationConstraints(
            Variables& vs, 
            Constraints& cs,
            cola::SparseMap* &Q) {
        // create 2 dummy vars, based on the dimension we are in
        vs.push_back(vl=new vpsc::Variable(vs.size(), leftMargin, weight));
        vs.push_back(vr=new vpsc::Variable(vs.size(), rightMargin, weight));
        vl->fixedDesiredPosition=true;
        vr->fixedDesiredPosition=true;

        // for each of the "real" variables, create a constraint that puts that var
        // between our two new dummy vars, depending on the dimension.
        for(OffsetList::iterator o=offsets.begin(); o!=offsets.end(); ++o)  {
            cs.push_back(new vpsc::Constraint(vl, vs[o->first], o->second));
            cs.push_back(new vpsc::Constraint(vs[o->first], vr, o->second));
        }
    }
    void updatePosition() {
        actualLeftMargin = vl->position();
        actualRightMargin = vr->position();
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
    double weight;
    vpsc::Variable *vl, *vr;
};
// defines references to three variables for which the goal function
// will be altered to prefer points u-b-v are in a linear arrangement
// such that b is placed at u+t(v-u).
struct LinearConstraint {
    LinearConstraint(unsigned u, unsigned v, unsigned b, double w, 
                     valarray<double> const & X, valarray<double> const & Y) 
        : u(u),v(v),b(b),w(w)
    {
        // from cosine rule: ub.uv/|uv|=|ub|cos(theta)
        double uvx = X[v] - X[u],
               uvy = Y[v] - Y[u],
               ubx = X[b] - X[u],
               uby = Y[b] - Y[u],
               duv2 = uvx * uvx + uvy * uvy;
        if(duv2 < 0.0001) {
            t=0;
        } else {
            t = (uvx * ubx + uvy * uby)/duv2;
        }
        duu=(1-t)*(1-t);
        duv=t*(1-t);
        dub=t-1;
        dvv=t*t;
        dvb=-t;
        dbb=1;
        //printf("New LC: t=%f\n",t); 
    }
    unsigned u;
    unsigned v;
    unsigned b;
    double w; // weight
    double t;
    // 2nd partial derivatives of the goal function
    //   (X[b] - (1-t) X[u] - t X[v])^2
    double duu;
    double duv;
    double dub;
    double dvv;
    double dvb;
    double dbb;
};
typedef vector<LinearConstraint*> LinearConstraints;
} // namespace cola
#endif // _COMPOUND_CONSTRAINTS_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
