#include <cassert>
#include "treeqp.h"
#include "quadprogpp/QuadProg++.h"
using namespace std;
/**
 * Solve problem using a conventional quadratic programming solver.
 * We use Luca Di Gaspero's GPLed QuadProg++ program 
 * http://www.diegm.uniud.it/digaspero/
 *
 * The problem is converted to the form:
 *
 * \f$\min_x \frac{1}{2} x^T A x + b^T x \f$ 
 * subject to \f$C^T x + c \ge 0 \f$
 *
 * @param ts goal terms
 * @param cs constraints
 * @param result the solution vector
 */
void treeQPSolve(Terms &ts, Constraints &cs, vector<double> &result) {
    const unsigned n=result.size();
    double **A=new double*[n],
           *b=new double[n];
    for(unsigned i=0;i<n;i++) {
        A[i]=new double[n];
        fill(A[i],A[i]+n,0);
    }
    fill(b,b+n,0);

    // requires first variable to be at 0 and forces positive definiteness
    A[0][0]=2.0;

    for(Terms::iterator t=ts.begin(), e=ts.end(); t!=e; ++t) {
        const unsigned p=t->p, c=t->c;
        assert(p<n);
        assert(c<n);
        double at=2.0*t->w;
        A[p][p]+=at;
        A[c][c]+=at;
        A[p][c]-=at;
        A[c][p]-=at;
        double bt=2.0*t->o*t->w;
        b[p]+=bt;
        b[c]+=-bt;
    }
    const unsigned m=cs.size();
    double **C=new double*[n],
           *c=new double[m];
    for(unsigned i=0;i<n;i++) {
        C[i]=new double[m];
        fill(C[i],C[i]+m,0);
    }
    for(unsigned i=0;i<m;i++) {
        const unsigned l=cs[i].l, r=cs[i].r;
        assert(l!=r);
        assert(l<n);
        assert(r<n);
        C[l][i]=-1;
        C[r][i]=1;
        c[i]=-cs[i].g;
    }
    double *x=new double[n];
    solve_quadprog(A, b, n, NULL, NULL, 0, C, c, m, x);
    result.resize(n);
    copy(x,x+n,result.begin());
    for(unsigned i=0;i<n;i++) {
        delete [] A[i];
        delete [] C[i];
    }
	delete [] A;
	delete [] b;
	delete [] C;
	delete [] c;
	delete [] x;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
