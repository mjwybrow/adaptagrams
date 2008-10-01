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

/**
 * \file testutil.cpp
 * utilities for testing libproject
 */
#include <cassert>
#include <cmath>
#include <algorithm>
#include <libproject/project.h>
#include <libproject/util.h>
#include "testutil.h"
#include "quadprogpp/QuadProg++.h"

using namespace project;
using namespace std;

/**
 * float comparisons (i.e. \f$a=b\f$) in our tests are actually \f$|a-b|<epsilon\f$.
 */
const double testEpsilon = 1e-10;

/**
 * Solve a project problem using a conventional quadratic programming solver.
 * We use Luca Di Gaspero's GPLed QuadProg++ program 
 * http://www.diegm.uniud.it/digaspero/
 *
 * The problem is converted to the form:
 *
 * \f$\min_x \frac{1}{2} x^T G x + g^T x \f$ 
 * subject to \f$C^T x + c \ge 0 \f$
 *
 * @param vs variables
 * @param cs constraints
 * @param result the solution vector
 */
void qps(Variables &vs, Constraints &cs, vector<double> &result) {
    const unsigned n=vs.size();
    double **G, *g;
	MatrixStorage _G(G,n,n);
	VectorStorage<double> _g(g,n);
    VMap vmap;
    for(unsigned i=0;i<n;i++) {
        Variable *v=vs[i];
        vmap[v]=i;
        g[i]=-2.0*v->getWeight()*v->getDesiredPosition();
        for(unsigned j=0;j<n;j++) {
            G[i][j]=
                i==j ? (2.0*v->getWeight()) : 0;
        }
    }
    const unsigned m=cs.size();
    double **C, *c;
	MatrixStorage _C(C,n,m);
	VectorStorage<double> _c(c,m);
    for(unsigned i=0;i<n;i++) {
        fill(C[i],C[i]+m,0);
    }
    for(unsigned i=0;i<m;i++) {
        const unsigned l=vmap[cs[i]->l],
                       r=vmap[cs[i]->r];
        C[l][i]=-1;
        C[r][i]=1;
        c[i]=-cs[i]->g;
    }
    double *x;
	VectorStorage<double> _x(x,n);
    solve_quadprog(G, g, n, NULL, NULL, 0, C, c, m, x);
    result.resize(n);
    copy(x,x+n,result.begin());
}

/**
 * Checks that each constraint is satisfied.
 * @return false if unsatisfied constraint found
 */
bool feasible(Constraints &cs) {
    for(Constraints::iterator i=cs.begin();i!=cs.end();++i) {
        Constraint* c=*i;
        double slack = c->r->getPosition() - c->l->getPosition() - c->g;
        if (slack < -testEpsilon) {
            printf("Unsatisfiable constraint: slack = %f\n",slack);    
            return false;
        }
    }
    return true;
}
/**
 * Print out the problem instance (as code that can easily be pasted into
 * a test case)
 */
void printProblem(Variables &vs, 
        vector<double> &XI, 
        Constraints &cs, 
        VMap &vmap) 
{
    printf("problem instance:\n");
    printf("//------ snip -----------------\n");
    printf("const char* testname(Variables& vs, Constraints &cs) {\n");
    vector<double> x;
    qps(vs,cs,x);
    for(unsigned i=0;i<vs.size();i++) {
        char *match="==";
        if(!approxEquals(vs[i]->getPosition(),x[i])) {
            match="!=";
        }
        printf("vs.push_back(new Variable(%f,%f,%f));//soln=%f%srslt=%f\n",
                XI[i],vs[i]->getDesiredPosition(),vs[i]->getWeight(),x[i],match,vs[i]->getPosition());
    }
    for(Constraints::iterator i=cs.begin();i!=cs.end();++i) {
        Constraint* c=*i;
        unsigned l=vmap[c->l], r=vmap[c->r];
        printf("cs.push_back(new Constraint(vs[%d],vs[%d],%f));\n", l,r,c->g);
    }
    printf("return __ASSERT_FUNCTION;\n}\n");
    printf("//------ end snip ------------\n");
}
/**
 * get random number in the range [0,max]
 */
double getRand(const double max) {
    return (double)max*rand()/(RAND_MAX);
}
/**
 * Check approximate equality of two doubles.
 * float comparisons (i.e. \f$a=b\$f) in our tests are actually \$f|a-b|<epsilon\$f.
 * @return true if the above condition is met
 */
bool approxEquals(double a, double b) {
    return fabs(a-b)<testEpsilon;
}
/**
 * Attempt to use libproject to solve a given projection problem instance.
 * @param t pointer to a function that will populate a list of variables and a list
 * of constraints.
 */
void test(void (*t)(string&, Variables&, Constraints&), bool silentPass) {
    Variables vs;
    Constraints cs;
    string tn;

    // call given function to generate the problem instance
    t(tn,vs,cs);
    
    // store initial positions
    vector<double> XI(vs.size());
    VMap vmap;
    for(unsigned i=0;i<vs.size();i++) {
        XI[i]=vs[i]->getPosition();
        vmap[vs[i]]=i;
    }

    // assert initial solution is feasible:
    assert(feasible(cs));

    // run standard qpsolver
    vector<double> qpresult;
    qps(vs,cs,qpresult);

    try {
        // run our solver
        solve(vs,cs);

        // assert final solution is feasible:
        if(!feasible(cs)) {
            throw "infeasible solution!";
        }
        // assert solution matches solution from standard QP solver
        for(unsigned i=0;i<vs.size();i++) {
            if(!approxEquals(vs[i]->getPosition(),qpresult[i])) {
                throw "incorrect solution!";
            }
        }
        if(!silentPass) {
            printf("PASS: %s\n",tn.c_str());
        }
    } catch(CriticalFailure &f) {
        f.print();
        printProblem(vs,XI,cs,vmap);
        fprintf(stderr,"FAIL: %s\n",tn.c_str());
        exit(1);
    }
    for_each(vs.begin(),vs.end(),delete_object());
    for_each(cs.begin(),cs.end(),delete_object());
}

/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
