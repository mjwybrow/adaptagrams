#ifndef _GRADIENT_PROJECTION_H
#define _GRADIENT_PROJECTION_H

#include <solve_VPSC.h>
#include <variable.h>
#include <constraint.h>
#include <generate-constraints.h>
#include <vector>
#include <iostream>

using namespace std;

struct Offset {
    Offset(unsigned v, double o) : v(v), offset(o) {}
    unsigned v;
    double offset;
};

typedef vector<Offset*> OffsetList;

class AlignmentConstraint {
    friend class GradientProjection;
public:
    AlignmentConstraint(double pos) : position(pos), variable(NULL) {}
    ~AlignmentConstraint() {
        for(OffsetList::iterator i=offsets.begin();i!=offsets.end();i++) {
            delete *i;
        }
    }
    void updatePosition() {
        position = variable->position();
    }
    OffsetList offsets;
    void* guide;
    double position;
private:
    Variable* variable;
};
typedef vector<AlignmentConstraint*> AlignmentConstraints;

typedef vector<pair<unsigned,double> > CList;
class DummyVarPair {
public:
    CList leftof;
    CList rightof;
    Variable* left;
    Variable* right;
    double weight;
    double lap2;
    double b; // linear coefficient in quad form for left (b_right = -b)
    double g; // descent vec for quad form for left (g_right = -g)
    double place_l;
    double place_r;
    double old_place_l;
    double old_place_r;
};
typedef vector<DummyVarPair*> DummyVars;

typedef vector<Constraint*> Constraints;
typedef vector<Variable*> Variables;

enum Dim { HORIZONTAL, VERTICAL };

class GradientProjection {
public:
	GradientProjection(
        const Dim k,
		unsigned n, 
		double** A,
		double* x,
		double tol,
		unsigned max_iterations,
        AlignmentConstraints* acs=NULL,
        bool nonOverlapConstraints=false,
        Rectangle** rs=NULL)
            : k(k), n(n), A(A), place(x), rs(rs),
              nonOverlapConstraints(nonOverlapConstraints),
              tolerance(tol), acs(acs), max_iterations(max_iterations),
              g(new double[n]), d(new double[n]), old_place(new double[n]),
              constrained(false)
    {
        for(unsigned i=0;i<n;i++) {
            vars.push_back(new Variable(i,1,1));
        }
        if(acs) {
            unsigned i=n;
            for(AlignmentConstraints::iterator iac=acs->begin();
                    iac!=acs->end();++iac) {
                AlignmentConstraint* ac=*iac;
                Variable *v=ac->variable=new Variable(i++,ac->position,0.0001);
                vars.push_back(v);
                for(OffsetList::iterator io=ac->offsets.begin();
                        io!=ac->offsets.end();
                        io++) {
                    Offset* o = *io;
                    cerr << " create constraint " << v->id << "=" << o->v << endl;
                    gcs.push_back(new Constraint(v,vars[o->v],o->offset,true));
                }
            }
        }
        if(!gcs.empty() || nonOverlapConstraints) {
            constrained=true;
        }
	}
    ~GradientProjection() {
        delete [] g;
        delete [] d;
        delete [] old_place;
        for(Constraints::iterator i(gcs.begin()); i!=gcs.end(); i++) {
            delete *i;
        }
        gcs.clear();
        for(unsigned i=0;i<n;i++) {
            delete vars[i];
        }
    }
	unsigned solve(double* b);
	unsigned solve_dummy(double* b);
    DummyVars dummy_vars; // special vars that must be considered in Lapl.
private:
    IncVPSC* setupVPSC();
    void destroyVPSC(IncVPSC *vpsc);
    Dim k;
	unsigned n; // number of actual vars
	double** A; // Graph laplacian matrix
    double* place;
	Variables vars; // all variables
                          // computations
    Constraints gcs; /* global constraints - persist throughout all
                                iterations */
    Constraints lcs; /* local constraints - only for current iteration */
    Rectangle** rs;
    bool nonOverlapConstraints;
    double tolerance;
    AlignmentConstraints* acs;
    unsigned max_iterations;
	double* g; /* gradient */
	double* d;
	double* old_place;
    bool constrained;
};

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
