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
typedef std::vector<vpsc::Constraint*> Constraints;
typedef std::vector<vpsc::Variable*> Variables;
typedef std::vector<std::pair<unsigned,double> > OffsetList;

class SimpleConstraint {
public:
    SimpleConstraint(unsigned l, unsigned r, double g, bool equality = false) 
        : left(l), right(r), gap(g), equality(equality)  {}
    unsigned left;
    unsigned right;
    double gap;
    bool equality;
};
typedef std::vector<SimpleConstraint*> SimpleConstraints;
class AlignmentConstraint {
friend class GradientProjection;
public:
    AlignmentConstraint(double pos) : position(pos), variable(NULL) {}
    void updatePosition() {
        position = variable->position();
    }
    OffsetList offsets;
    void* guide;
    double position;
private:
    vpsc::Variable* variable;
};
typedef std::vector<AlignmentConstraint*> AlignmentConstraints;
struct DistributionConstraint {
    std::vector<std::pair<AlignmentConstraint*,AlignmentConstraint*> > acs;
    double sep;
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
        vpsc::Rectangle** rs=NULL,
        PageBoundaryConstraints *pbc = NULL,
		cola::SparseMatrix const * sparseQ = NULL,
        SimpleConstraints *sc = NULL)
            : k(k), n(x.size()), 
              denseSize(unsigned(floor(sqrt(denseQ.size())))), denseQ(denseQ), 
              place(x), rs(rs),
              nonOverlapConstraints(nonOverlapConstraints),
              tolerance(tol), acs(acs), max_iterations(max_iterations),
              sparseQ(sparseQ),
              constrained(false)
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
                DistributionConstraint *dc=*idc;
                for(std::vector<std::pair<
                        AlignmentConstraint*,AlignmentConstraint*> >::iterator iac
                        =dc->acs.begin(); iac!=dc->acs.end();++iac) {
                    AlignmentConstraint *c1, *c2;
                    c1=iac->first;
                    c2=iac->second;
                    assert(c1->variable!=NULL);
                    gcs.push_back(new vpsc::Constraint(
                                c1->variable,c2->variable,dc->sep,true));
                }
            }
        }
        if (pbc)  {          
            pbc->createVarsAndConstraints(vars,gcs);
        }
        if (sc) {
            for(SimpleConstraints::iterator c=sc->begin(); c!=sc->end();++c) {
                gcs.push_back(new vpsc::Constraint(
                        vars[(*c)->left],vars[(*c)->right],
                        (*c)->gap,(*c)->equality));
            }
        }
        if(!gcs.empty() || nonOverlapConstraints!=None) {
            constrained=true;
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
    }
	unsigned solve(valarray<double> const & b);
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
	const unsigned n; // number of actual vars
    const unsigned denseSize; // denseQ has denseSize^2 entries
	valarray<double> const & denseQ; // dense square graph laplacian matrix
    valarray<double> & place;
    vpsc::Rectangle** rs;
    NonOverlapConstraints nonOverlapConstraints;
    double tolerance;
    AlignmentConstraints* acs;
    unsigned max_iterations;
    cola::SparseMatrix const * sparseQ; // sparse components of goal function
	Variables vars; // all variables
                          // computations
    Constraints gcs; /* global constraints - persist throughout all
                                iterations */
    Constraints lcs; /* local constraints - only for current iteration */
    bool constrained;
};

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
