#ifndef _GRADIENT_PROJECTION_H
#define _GRADIENT_PROJECTION_H

#include <solve_VPSC.h>
#include <variable.h>
#include <constraint.h>
#include <generate-constraints.h>

class GradientProjection {
public:
	GradientProjection(
		unsigned n, 
		double** A,
		double* x,
		Variable** vs,
        unsigned m,
	    Constraint** cs,
		double tol,
		unsigned max_iterations) 
            : n(n), A(A), place(x), 
              vs(vs), m(m), cs(cs),
              tolerance(tol), max_iterations(max_iterations),
              g(new double[n]), d(new double[n]), old_place(new double[n]),
              vpsc(new IncVPSC(n,vs,m,cs))
             { }
	GradientProjection(
		unsigned n, 
		double** A,
		double* x,
		double tol,
		unsigned max_iterations) 
            : n(n), A(A), place(x), 
              vs(new Variable*[n]), m(0), cs(NULL),
              tolerance(tol), max_iterations(max_iterations),
              g(new double[n]), d(new double[n]), old_place(new double[n])
    {
        for(unsigned i=0;i<n;i++) {
            vs[i]=new Variable(i,1,1);
        }
        vpsc=new IncVPSC(n,vs,m,cs);
	}
    ~GradientProjection() {
        delete [] g;
        delete [] d;
        delete [] old_place;
        delete vpsc;
        if(cs) {
            for(unsigned i=0;i<m;i++) {
                delete cs[i];
            }
            delete [] cs;
        }
        for(unsigned i=0;i<n;i++) {
            delete vs[i];
        }
        delete [] vs;
    }
    void generateNonOverlapConstraints(unsigned k, Rectangle** rs);
	unsigned solve(double* b);
private:
	unsigned n;   // number of actual vars
	double** A; // Graph laplacian matrix
    double* place;
	Variable** vs;
	unsigned m; /* total number of constraints for next iteration */
	Constraint** cs;
    double tolerance;
    unsigned max_iterations;
	double* g; /* gradient */
	double* d;
	double* old_place;
    IncVPSC* vpsc;
};

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
