/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libproject - Solves an instance of the Variable Placement with Separation
 *              Constraints problem, that is a projection onto separation 
 *              constraints, whilst always maintaining feasibility.
 *
 * Copyright (C) 2005-2008  Monash University
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
void treeQPSolve(Terms &ts, EqualityConstraints &ecs, InequalityConstraints &ics, vector<double> &result) {
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
    // equality constraints:
    const unsigned me=ecs.size();
    double **CE=new double*[n],
           *ce=new double[me];
    for(unsigned i=0;i<n;i++) {
        CE[i]=new double[me];
        fill(CE[i],CE[i]+me,0);
    }
    for(unsigned i=0;i<me;i++) {
        EqualityConstraint& ec=ecs[i];
        const unsigned l=ec.l, r=ec.r;
        const double g=ec.g;
        assert(l!=r);
        assert(l<n);
        assert(r<n);
        if(ec.threeVars) {
            const unsigned p=ec.p;
            assert(p<n);
            CE[p][i]=0.5; 
            CE[l][i]=-0.5; 
            CE[r][i]=-0.5; 
            ce[i]=-g/2.0;
        } else {
            CE[l][i]=-1;
            CE[r][i]=1;
            ce[i]=-g;
        }
    }

    // inequality constraints:
    const unsigned mi=ics.size();
    double **CI=new double*[n],
           *ci=new double[mi];
    for(unsigned i=0;i<n;i++) {
        CI[i]=new double[mi];
        fill(CI[i],CI[i]+mi,0);
    }
    for(unsigned i=0;i<mi;i++) {
        const unsigned l=ics[i].l, r=ics[i].r;
        assert(l!=r);
        assert(l<n);
        assert(r<n);
        CI[l][i]=-1;
        CI[r][i]=1;
        ci[i]=-ics[i].g;
    }

    double *x=new double[n];
    solve_quadprog(A, b, n, CE, ce, me, CI, ci, mi, x);
    result.resize(n);
    copy(x,x+n,result.begin());
    for(unsigned i=0;i<n;i++) {
        delete [] A[i];
        delete [] CE[i];
        delete [] CI[i];
    }
	delete [] A;
	delete [] b;
	delete [] CE;
	delete [] CI;
	delete [] ce;
	delete [] ci;
	delete [] x;
}
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
