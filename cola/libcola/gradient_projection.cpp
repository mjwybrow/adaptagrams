/**********************************************************
 *
 * Solve a quadratic function f(X) = X' A X + b X
 * subject to a set of separation constraints cs
 *
 * Tim Dwyer, 2006
 **********************************************************/

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <float.h>
#include <cassert>
#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include "gradient_projection.h"
#include <iostream>

using namespace std;
using namespace vpsc;
//#define CONMAJ_LOGGING 1

static void dumpVPSCException(char const *str, IncSolver* solver) {
    cerr<<str<<endl;
    unsigned m;
    Constraint** cs = solver->getConstraints(m);
    for(unsigned i=0;i<m;i++) {
        cerr << *cs[i] << endl;
    }
}

static inline double dotProd(valarray<double> const & a, valarray<double> const & b) {
    double p = 0;
    for (unsigned i=0; i<a.size(); i++) {
        p += a[i]*b[i];
    }
    return p;
}

double GradientProjection::computeSteepestDescentVector(
        valarray<double> const &b,
        valarray<double> const &place,
        valarray<double> &g) {
    // find steepest descent direction
    //  g = 2 ( b - A x )
    //    where: A = denseQ + sparseQ
    //  g = 2 ( b - denseQ x) - 2 sparseQ x
    //
    //  except the 2s don't matter because we compute 
    //  the optimal stepsize anyway
    g = b;
    for (unsigned i=0; i<n; i++) {
        if(i<denseSize) { for (unsigned j=0; j<denseSize; j++) {
            g[i] -= denseQ[i*denseSize+j]*place[j];
        } }
    }
    // sparse part:
    if(sparseQ) {
        valarray<double> r(n);
        sparseQ->rightMultiply(place,r);
        g-=r;
    }
    return computeStepSize(g,g);
}
// compute optimal step size along descent vector d relative to
// a gradient related vector g (which is actually -gradient/2 as
// computed above)
//   stepsize = ( g' d ) / ( 2 d' A d )
double GradientProjection::computeStepSize(
        valarray<double> const & g, valarray<double> const & d) {
    valarray<double> Ad;
    if(sparseQ) {
        Ad.resize(n);
        sparseQ->rightMultiply(d,Ad);
    }
    double const numerator = dotProd(g, d);
    double denominator = 0;
    for (unsigned i=0; i<n; i++) {
        double r = sparseQ ? Ad[i] : 0;
        if(i<denseSize) { for (unsigned j=0; j<denseSize; j++) {
            r += denseQ[i*denseSize+j] * d[j];
        } }
        denominator += r * d[i];
    }
    return numerator/(2.*denominator);
}
double GradientProjection::computeScaledSteepestDescentVector(
        valarray<double> const &b,
        valarray<double> const &place,
        valarray<double> &d) {
    valarray<double> g(n); /* gradient */
    // find steepest descent direction
    //  g = D - g
    //    where: D = inverse of the diagonal elements of the hessian
    g = b;
    for (unsigned i=0; i<n; i++) {
        if(i<denseSize) { 
            for (unsigned j=0; j<denseSize; j++) {
                g[i] -= denseQ[i*denseSize+j]*place[j];
                d[i] = -g[i]/denseQ[i*denseSize+i];
            }
        }
    }
    // sparse part:
    if(sparseQ) {
        valarray<double> r(n);
        sparseQ->rightMultiply(place,r);
        g-=r;
    }
    for (unsigned i=0; i<n; i++) {
    }
    return -computeStepSize(g,d);
}

/*
 * Use gradient-projection to solve an instance of
 * the Variable Placement with Separation Constraints problem.
 * Uses sparse matrix techniques to handle pairs of dummy
 * vars.
 */
unsigned GradientProjection::solve(valarray<double> const &b) {
	unsigned i,counter;
	if(max_iterations==0) return 0;

	bool converged=false;

    IncSolver* solver=NULL;

    solver = setupVPSC();
    //cerr << "in gradient projection: n=" << n << endl;
    for (i=0;i<n;i++) {
        assert(!isnan(place[i]));
        assert(!isinf(place[i]));
        vars[i]->desiredPosition=place[i];
    }
    try {
        solver->satisfy();
    } catch (char const *str) {
        dumpVPSCException(str,solver);
	}

    for (i=0;i<n;i++) {
        place[i]=vars[i]->position();
    }
    	
	valarray<double> g(n); /* gradient */
	valarray<double> old_place(n); /* stored positions */
    valarray<double> d(n); /* actual descent vector */
	for (counter=0; counter<max_iterations&&!converged; counter++) {
		converged=true;		
        old_place=place;
        double alpha=computeSteepestDescentVector(b,place,g);

        // move to new unconstrained position
		for (i=0; i<n; i++) {
			place[i]+=alpha*g[i];
            assert(!isnan(place[i]));
            assert(!isinf(place[i]));
            vars[i]->desiredPosition=place[i];
		}

        //project to constraint boundary
        bool constrainedOptimum = false;
        try {
            constrainedOptimum=
                solver->satisfy();
        } catch (char const *str) {
            dumpVPSCException(str,solver);
        }
        for (i=0;i<n;i++) {
            place[i]=vars[i]->position();
        }
        if(constrainedOptimum) {
            /* The following step limits the step-size in the feasible
             * direction
             */
            d = place - old_place;
            const double beta = computeStepSize(g, d);
            // beta > 1.0 takes us back outside the feasible region
            // beta < 0 clearly not useful and may happen due to numerical imp.
            if(beta>0&&beta<0.99999) {
                printf("beta=%f\n",beta);
                for (i=0; i<n; i++) {
                    place[i]=old_place[i]+beta*d[i];
                }
            }
        }
		double distanceMoved=0;
		for (i=0; i<n; i++) {
			distanceMoved += fabs(place[i]-old_place[i]);
		}
		if(distanceMoved>tolerance) {
			converged=false;
		}
	}
    //destroyVPSC(solver);
	return counter;
}
// Setup an instance of the Variable Placement with Separation Constraints
// for one iteration.
// Generate transient local constraints --- such as non-overlap constraints 
// --- that are only relevant to one iteration, and merge these with the
// global constraint list (including alignment constraints,
// dir-edge constraints, containment constraints, etc).
IncSolver* GradientProjection::setupVPSC() {
    Constraint **cs;
    //assert(lcs.size()==0);
    
    for(DummyVars::iterator dit=dummy_vars.begin();
            dit!=dummy_vars.end(); ++dit) {
        (*dit)->setupVPSC(vars,lcs);
    }
    Variable** vs = new Variable*[vars.size()];
    for(unsigned i=0;i<vars.size();i++) {
        vs[i]=vars[i];
    }
    if(nonOverlapConstraints) {
        Constraint** tmp_cs=NULL;
        unsigned m=0;
        if(k==HORIZONTAL) {
            Rectangle::setXBorder(0.0001);
            m=generateXConstraints(n,rs,vs,tmp_cs,true); 
            Rectangle::setXBorder(0);
        } else {
            m=generateYConstraints(n,rs,vs,tmp_cs); 
        }
        for(unsigned i=0;i<m;i++) {
            lcs.push_back(tmp_cs[i]);
        }
    }
    cs = new Constraint*[lcs.size() + gcs.size()];
    unsigned m = 0 ;
    for(vector<Constraint*>::iterator ci = lcs.begin();ci!=lcs.end();++ci) {
        cs[m++] = *ci;
    }
    for(vector<Constraint*>::iterator ci = gcs.begin();ci!=gcs.end();++ci) {
        cs[m++] = *ci;
    }
    return new IncSolver(vars.size(),vs,m,cs);
}
void GradientProjection::clearDummyVars() {
    for(DummyVars::iterator i=dummy_vars.begin();i!=dummy_vars.end();++i) {
        delete *i;
    }
    dummy_vars.clear();
}
void GradientProjection::destroyVPSC(IncSolver *vpsc) {
    if(acs) {
        for(AlignmentConstraints::iterator ac=acs->begin(); ac!=acs->end();++ac) {
            (*ac)->updatePosition();
        }
    }
    unsigned m,n;
    Constraint** cs = vpsc->getConstraints(m);
    const Variable* const* vs = vpsc->getVariables(n);
    delete vpsc;
    delete [] cs;
    delete [] vs;
    for(vector<Constraint*>::iterator i=lcs.begin();i!=lcs.end();i++) {
            delete *i;
    }
    lcs.clear();
    //cout << " Vars count = " << vars.size() << " Dummy vars cnt=" << dummy_vars.size() << endl;
    vars.resize(vars.size()-dummy_vars.size()*2);
    for(DummyVars::iterator i=dummy_vars.begin();i!=dummy_vars.end();++i) {
        DummyVarPair* p = *i;
        delete p->left;
        delete p->right;
    }
}

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
