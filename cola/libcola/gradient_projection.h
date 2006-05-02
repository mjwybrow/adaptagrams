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

typedef vector<Constraint*> Constraints;
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
            : k(k), n(n), n_dummy(0), A(A), place(x), 
              vs(NULL), gcs(NULL), rs(rs),
              nonOverlapConstraints(nonOverlapConstraints),
              tolerance(tol), acs(acs), max_iterations(max_iterations),
              g(new double[n]), d(new double[n]), old_place(new double[n])
    {
        vector<Variable*> vars;
        for(unsigned i=0;i<n;i++) {
            vars.push_back(new Variable(i,1,1));
        }
        if(acs) {
            gcs=new Constraints;
            n_dummy+=acs->size();
            cerr << "  " << n_dummy << " alignment constraints" << endl;
            unsigned i=n;
            for(AlignmentConstraints::iterator iac=acs->begin();
                    iac!=acs->end();iac++,i++) {
                AlignmentConstraint* ac=*iac;
                Variable *v=ac->variable=new Variable(i,ac->position,0.0001);
                vars.push_back(v);
                for(OffsetList::iterator io=ac->offsets.begin();
                        io!=ac->offsets.end();
                        io++) {
                    Offset* o = *io;
                    cerr << " create constraint " << v->id << "=" << o->v << endl;
                    gcs->push_back(new Constraint(v,vars[o->v],o->offset,true));
                }
            }
        }
        vs = new Variable*[vars.size()];
        for(unsigned i=0;i<vars.size();i++) {
            vs[i]=vars[i];
        }
	}
    ~GradientProjection() {
        delete [] g;
        delete [] d;
        delete [] old_place;
        if(gcs) {
            for(Constraints::iterator i(gcs->begin()); i!=gcs->end(); i++) {
                delete *i;
            }
            delete gcs;
        }
        for(unsigned i=0;i<n;i++) {
            delete vs[i];
        }
        delete [] vs;
    }
	unsigned solve(double* b);
private:
    pair<IncVPSC*, Constraint**> setupVPSC();
    void destroyVPSC(IncVPSC *vpsc, Constraint **lcs);
    Dim k;
	unsigned n; // number of actual vars
    unsigned n_dummy; // number of dummy vars
	double** A; // Graph laplacian matrix
    double* place;
	Variable** vs;
    Constraints* gcs; /* global constraints - persist throughout all
                                iterations */
    Rectangle** rs;
    bool nonOverlapConstraints;
    double tolerance;
    AlignmentConstraints* acs;
    unsigned max_iterations;
	double* g; /* gradient */
	double* d;
	double* old_place;
};

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
