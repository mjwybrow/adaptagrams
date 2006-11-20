#ifndef _GRADIENT_PROJECTION_H
#define _GRADIENT_PROJECTION_H

#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/rectangle.h>
#include <vector>
#include <iostream>
#include <math.h>
#include <valarray>
#include "sparse_matrix.h"

using std::valarray;
using std::make_pair;
typedef std::vector<vpsc::Constraint*> Constraints;
typedef std::vector<vpsc::Variable*> Variables;
typedef std::vector<std::pair<unsigned,double> > OffsetList;

class SeparationConstraint {
public:
    SeparationConstraint(unsigned l, unsigned r, double g, bool equality = false) 
        : left(l), right(r), gap(g), equality(equality)  {}
    unsigned left;
    unsigned right;
    double gap;
    bool equality;
};
typedef std::vector<SeparationConstraint*> SeparationConstraints;

// An alignment constraint specifies a group of nodes and offsets for those nodes
// such that the nodes must be spaced exactly at those offsets from a vertical or
// horizontal line.
class AlignmentConstraint {
friend class GradientProjection;
public:
    AlignmentConstraint(double pos) : position(pos), variable(NULL) {}
    void updatePosition() {
        position = variable->position();
    }
    // a list of pairs of node indices and their required offsets
    OffsetList offsets;
    // the guide pointer is used by dunnart to keep a ref to it's local representation
    // of the alignment constraint
    void* guide;
    // The position of the alignment line
    double position;
private:
    vpsc::Variable* variable;
};
typedef std::vector<AlignmentConstraint*> AlignmentConstraints;

// A distribution constraint specifies an ordered set of alignment constraints
// and a separation required between them.
// The separation can be variable (but the same between each adjacent pair of
// alignment constraints) or fixed.
struct DistributionConstraint {
    DistributionConstraint(bool isVariable = false) : isVariable(isVariable) {}
    std::vector<std::pair<AlignmentConstraint*,AlignmentConstraint*> > acs;
    double sep;
    bool isVariable;
    vpsc::Variable* variable;
};
typedef std::vector<DistributionConstraint*> DistributionConstraints;

class PageBoundaryConstraints {
public:
    PageBoundaryConstraints(double lm, double rm, double w)
        : leftMargin(lm), rightMargin(rm), weight(w) { }
    void createVarsAndConstraints(Variables &vs, Constraints &cs) {
        vpsc::Variable* vl, * vr;
        // create 2 dummy vars, based on the dimension we are in
        vs.push_back(vl=new vpsc::Variable(vs.size(), leftMargin, weight));
        vs.push_back(vr=new vpsc::Variable(vs.size(), rightMargin, weight));

        // for each of the "real" variables, create a constraint that puts that var
        // between our two new dummy vars, depending on the dimension.
        for(OffsetList::iterator o=offsets.begin(); o!=offsets.end(); ++o)  {
            cs.push_back(new vpsc::Constraint(vl, vs[o->first], o->second));
            cs.push_back(new vpsc::Constraint(vs[o->first], vr, o->second));
        }
    }
    OffsetList offsets;
private:
    double leftMargin;
    double rightMargin;
    double weight;
};

enum Dim { HORIZONTAL, VERTICAL };
/**
 * resolve overlaps:
 *   None: not at all
 *   Horizontal: only horizontally
 *   Both: resolve in either Horizontal or Vertical direction
 *         depending on which leads to less displacement
 */
enum NonOverlapConstraints { None, Horizontal, Both };

class GradientProjection {
public:
	GradientProjection(
        const Dim k,
		valarray<double> const & denseQ,
		valarray<double>& x,
		const double tol,
		const unsigned max_iterations,
        AlignmentConstraints* acs=NULL,
        DistributionConstraints* dcs=NULL,
        NonOverlapConstraints nonOverlapConstraints=None,
        std::vector<vpsc::Rectangle*>* rs = NULL,
        PageBoundaryConstraints *pbc = NULL,
		cola::SparseMap *Q = NULL,
        SeparationConstraints *sc = NULL)
            : k(k), n(x.size()), 
              denseSize(unsigned(floor(sqrt(denseQ.size())))), denseQ(denseQ), 
              place(x), rs(rs),
              nonOverlapConstraints(nonOverlapConstraints),
              tolerance(tol), acs(acs), max_iterations(max_iterations),
              sparseQ(NULL),localSparseMapCreated(false)
    {
        for(unsigned i=0;i<n;i++) {
            vars.push_back(new vpsc::Variable(i,1,1));
        }
        if(acs) {
            for(AlignmentConstraints::iterator iac=acs->begin();
                    iac!=acs->end();++iac) {
                AlignmentConstraint* ac=*iac;
                vpsc::Variable *v=ac->variable=new vpsc::Variable(vars.size(),ac->position,0.0001);
                vars.push_back(v);
                for(OffsetList::iterator o=ac->offsets.begin();
                        o!=ac->offsets.end();
                        o++) {
                    gcs.push_back(new vpsc::Constraint(v,vars[o->first],o->second,true));
                }
            }
        }
        if(dcs) {
            for(DistributionConstraints::iterator idc=dcs->begin();
                    idc!=dcs->end();++idc) {
                double w=-10.;
                DistributionConstraint *dc=*idc;
                if(dc->isVariable) {
                    if(!Q) {
                        Q = new cola::SparseMap(n);
                        localSparseMapCreated=true;
                    }
                    dc->variable=new vpsc::Variable(vars.size(),dc->sep,0.000001);
                    vars.push_back(dc->variable);
                    Q->n=n=vars.size();
                }
                for(std::vector<std::pair<
                        AlignmentConstraint*,AlignmentConstraint*> >::iterator iac
                        =dc->acs.begin(); iac!=dc->acs.end();++iac) {
                    AlignmentConstraint *c1, *c2;
                    c1=iac->first;
                    c2=iac->second;
                    assert(c1->variable!=NULL);
                    if(dc->isVariable) {
                        // set second derivatives of:
                        // (u + g - v)^2 = g^2 + 2gu + u^2 - 2gv - 2uv + v^2
                        (*Q)[make_pair(c1->variable->id,c1->variable->id)]+=w;
                        (*Q)[make_pair(c2->variable->id,c2->variable->id)]+=w;
                        (*Q)[make_pair(dc->variable->id,dc->variable->id)]+=w;
                        (*Q)[make_pair(c1->variable->id,c2->variable->id)]-=w;
                        (*Q)[make_pair(c2->variable->id,c1->variable->id)]-=w;
                        (*Q)[make_pair(c1->variable->id,dc->variable->id)]+=w;
                        (*Q)[make_pair(dc->variable->id,c1->variable->id)]+=w;
                        (*Q)[make_pair(c2->variable->id,dc->variable->id)]-=w;
                        (*Q)[make_pair(dc->variable->id,c2->variable->id)]-=w;
                    } else {
                        gcs.push_back(new vpsc::Constraint(
                                c1->variable,c2->variable,dc->sep,true));
                    }
                }
            }
        }
        if(x.size()<n) {
            // if we added new variables above then we'll have to resize the
            // coords array accordingly
            valarray<double> tmp=x;
            x.resize(n);
            for(unsigned i=0;i<tmp.size();i++) {
                x[i]=tmp[i];
            }
            for(unsigned i=tmp.size();i<n;i++) {
                x[i]=vars[i]->desiredPosition;
            }

        }
        if (pbc)  {          
            pbc->createVarsAndConstraints(vars,gcs);
        }
        if (sc) {
            for(SeparationConstraints::iterator c=sc->begin(); c!=sc->end();++c) {
                gcs.push_back(new vpsc::Constraint(
                        vars[(*c)->left],vars[(*c)->right],
                        (*c)->gap,(*c)->equality));
            }
        }
        if(Q) {
            sparseQ = new cola::SparseMatrix(*Q);
        }
	}
    ~GradientProjection() {
        for(Constraints::iterator i(gcs.begin()); i!=gcs.end(); i++) {
            delete *i;
        }
        gcs.clear();
        for(unsigned i=0;i<vars.size();i++) {
            delete vars[i];
        }
        if(localSparseMapCreated) delete sparseQ;
    }
	unsigned solve(valarray<double> const & b);
    unsigned getSize() { return n; }
private:
    vpsc::IncSolver* setupVPSC();
    double computeSteepestDescentVector(
        valarray<double> const &b, valarray<double> const &place,
        valarray<double> &g);
    double computeScaledSteepestDescentVector(
        valarray<double> const &b, valarray<double> const &place,
        valarray<double> &g);
    double computeStepSize(
        valarray<double> const & g, valarray<double> const & d);
    void destroyVPSC(vpsc::IncSolver *vpsc);
    Dim k;
	unsigned n; // number of actual vars - this may grow in the constructor as we add
                // dummy vars
    const unsigned denseSize; // denseQ has denseSize^2 entries
	valarray<double> const & denseQ; // dense square graph laplacian matrix
    valarray<double> & place;
    std::vector<vpsc::Rectangle*>* rs;
    NonOverlapConstraints nonOverlapConstraints;
    double tolerance;
    AlignmentConstraints* acs;
    unsigned max_iterations;
    cola::SparseMatrix * sparseQ; // sparse components of goal function
	Variables vars; // all variables
                          // computations
    Constraints gcs; /* global constraints - persist throughout all
                                iterations */
    Constraints lcs; /* local constraints - only for current iteration */
    bool localSparseMapCreated;
};

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
