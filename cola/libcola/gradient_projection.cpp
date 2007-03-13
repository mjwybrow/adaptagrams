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
#include "straightener.h"
#include <iostream>

using namespace std;
using namespace vpsc;
namespace cola {

const double DistributionConstraint::w;

static inline double dotProd(valarray<double> const & a, valarray<double> const & b) {
    double p = 0;
    for (unsigned i=0; i<a.size(); i++) {
        p += a[i]*b[i];
    }
    return p;
}
double GradientProjection::computeCost(
        valarray<double> const &b,
        valarray<double> const &x) const {
    // computes cost = 2 b x - x A x
    double cost = 2. * dotProd(b,x);
    valarray<double> Ax(x.size());
    for (unsigned i=0; i<denseSize; i++) {
        Ax[i] = 0;
        for (unsigned j=0; j<denseSize; j++) {
            Ax[i] += denseQ[i*denseSize+j]*x[j];
        }
    }
    if(sparseQ) {
        valarray<double> r(x.size());
        sparseQ->rightMultiply(x,r);
        Ax+=r;
    }
    return cost - dotProd(x,Ax);
}

double GradientProjection::computeSteepestDescentVector(
        valarray<double> const &b,
        valarray<double> const &x,
        valarray<double> &g) const {
    // find steepest descent direction
    //  g = 2 ( b - A x )
    //    where: A = denseQ + sparseQ
    //  g = 2 ( b - denseQ x) - 2 sparseQ x
    //
    //  except the 2s don't matter because we compute 
    //  the optimal stepsize anyway
    assert(x.size()==b.size() && b.size()==g.size());
    g = b;
    for (unsigned i=0; i<denseSize; i++) {
        for (unsigned j=0; j<denseSize; j++) {
            g[i] -= denseQ[i*denseSize+j]*x[j];
        }
    }
    // sparse part:
    if(sparseQ) {
        valarray<double> r(x.size());
        sparseQ->rightMultiply(x,r);
        g-=r;
    }
    return computeStepSize(g,g);
}
// compute optimal step size along descent vector d relative to
// a gradient related vector g 
//    stepsize = ( g' d ) / ( d' A d )
double GradientProjection::computeStepSize(
        valarray<double> const & g, valarray<double> const & d) const {
    assert(g.size()==d.size());
    valarray<double> Ad;
    if(sparseQ) {
        Ad.resize(g.size());
        sparseQ->rightMultiply(d,Ad);
    }
    double const numerator = dotProd(g, d);
    double denominator = 0;
    for (unsigned i=0; i<g.size(); i++) {
        double r = sparseQ ? Ad[i] : 0;
        if(i<denseSize) { for (unsigned j=0; j<denseSize; j++) {
            r += denseQ[i*denseSize+j] * d[j];
        } }
        denominator += r * d[i];
    }
    if(denominator==0) {
        return 0;
    }
    return numerator/(2.*denominator);
}
double GradientProjection::computeScaledSteepestDescentVector(
        valarray<double> const &b,
        valarray<double> const &x,
        valarray<double> &d) const {
    unsigned n=x.size();
    valarray<double> g(n); /* gradient */
    // find steepest descent direction
    //  g = D - g
    //    where: D = inverse of the diagonal elements of the hessian
    g = b;
    for (unsigned i=0; i<n; i++) {
        if(i<denseSize) { 
            for (unsigned j=0; j<denseSize; j++) {
                g[i] -= denseQ[i*denseSize+j]*x[j];
                d[i] = -g[i]/denseQ[i*denseSize+i];
            }
        }
    }
    // sparse part:
    if(sparseQ) {
        valarray<double> r(n);
        sparseQ->rightMultiply(x,r);
        g-=r;
    }
    for (unsigned i=0; i<n; i++) {
    }
    return -0.5*computeStepSize(g,d);
}


/*
 * Use gradient-projection to solve an instance of
 * the Variable Placement with Separation Constraints problem.
 */
unsigned GradientProjection::solve(
        valarray<double> const &linearCoefficients, 
        valarray<double> &x) {
    assert(linearCoefficients.size()==x.size());
    assert(x.size()==denseSize);
    assert(numStaticVars>=denseSize);
    assert(sparseQ==NULL || sparseQ!=NULL && vars.size()==sparseQ->rowSize());
	if(max_iterations==0) return 0;

	bool converged=false;

    IncSolver* solver=NULL;

    solver = setupVPSC();
    // load desired positions into vars, note that we keep desired positions 
    // already calculated for dummy vars
    for (unsigned i=0;i<x.size();i++) {
        assert(!isnan(x[i]));
        assert(!isinf(x[i]));
        if(!vars[i]->fixedDesiredPosition) vars[i]->desiredPosition=x[i];
    }
    solver->satisfy();
    // it may be that we have to consider dummy vars, which the caller didn't know
    // about.  Thus vars.size() may not equal x.size()
    unsigned n = vars.size();
    valarray<double> b(n);
    result.resize(n);
    for (unsigned i=0;i<n;i++) {
        b[i]=i<linearCoefficients.size()?linearCoefficients[i]:0;
        result[i]=vars[i]->position();
    }
    	
	valarray<double> g(n); /* gradient */
	valarray<double> previous(n); /* stored positions */
    valarray<double> d(n); /* actual descent vector */

    double previousCost = DBL_MAX;
    unsigned counter=0;
	for (; counter<max_iterations&&!converged; counter++) {
        previous=result;
        double alpha=computeSteepestDescentVector(b,result,g);

        // move to new unconstrained position
		for (unsigned i=0; i<n; i++) {
            // dividing by variable weight is a cheap trick to make these
            // weights mean something in terms of the descent vector
			result[i]+=alpha*g[i]/vars[i]->weight;
            assert(!isnan(result[i]));
            assert(!isinf(result[i]));
            if(!vars[i]->fixedDesiredPosition) vars[i]->desiredPosition=result[i];
		}

        //project to constraint boundary
        bool constrainedOptimum = false;
        constrainedOptimum=
            solver->satisfy();
        for (unsigned i=0;i<n;i++) {
            result[i]=vars[i]->position();
        }
        constrainedOptimum=false;
        if(constrainedOptimum) {
            /* The following step limits the step-size in the feasible
             * direction
             */
            d = result - previous;
            const double beta = 0.5*computeStepSize(g, d);
            // beta > 1.0 takes us back outside the feasible region
            // beta < 0 clearly not useful and may happen due to numerical imp.
            if(beta>0&&beta<0.99999) {
		    //printf("beta=%f\n",beta);
                for (unsigned i=0; i<n; i++) {
                    result[i]=previous[i]+beta*d[i];
                }
            }
        }
        if(counter%2) {
            double cost = computeCost(b,result);
            //printf("gp[%d] cost=%f\n",counter,cost);
            assert(previousCost>cost);
            if(fabs(previousCost - cost) < tolerance) {
                converged = true;
            }
            previousCost = cost;
        }
	}
    for(unsigned i=0;i<x.size();i++) {
        x[i]=result[i];
    }
    destroyVPSC(solver);
	return counter;
}
// Setup an instance of the Variable Placement with Separation Constraints
// for one iteration.
// Generate transient local constraints --- such as non-overlap constraints 
// --- that are only relevant to one iteration, and merge these with the
// global constraint list (including alignment constraints,
// dir-edge constraints, containment constraints, etc).
IncSolver* GradientProjection::setupVPSC() {
    Variable** vs = new Variable*[vars.size()];
    for(unsigned i=0;i<vars.size();i++) {
        vs[i]=vars[i];
    }
    if(nonOverlapConstraints!=None) {
        if(clusterHierarchy) {
            printf("Setup cluster constraints, dim=%d--------------\n",k);
            clusterHierarchy->
                generateNonOverlapConstraints(k,nonOverlapConstraints,*rs,vars,lcs);
        } else {
            Constraint** tmp_cs=NULL;
            unsigned m=0;
            vector<Rectangle*> lrs;
            for(vector<Rectangle*>::const_iterator i=rs->begin();
                    i!=rs->end();i++) {
                Rectangle* r=*i;
                if(!r->allowOverlap()) {
                    lrs.push_back(r);
                }
            }
            if(k==HORIZONTAL) {
                Rectangle::setXBorder(0.0001);
                // use rs->size() rather than n because some of the variables may
                // be dummy vars with no corresponding rectangle
                m=generateXConstraints(lrs.size(),lrs,vs,tmp_cs,nonOverlapConstraints==Both?true:false); 
                Rectangle::setXBorder(0);
            } else {
                m=generateYConstraints(lrs.size(),lrs,vs,tmp_cs); 
            }
            for(unsigned i=0;i<m;i++) {
                lcs.push_back(tmp_cs[i]);
            }
        }
    }
    Constraint **cs = new Constraint*[lcs.size() + gcs.size()];
    unsigned m = 0 ;
    for(vector<Constraint*>::iterator ci = lcs.begin();ci!=lcs.end();++ci) {
        cs[m++] = *ci;
    }
    for(vector<Constraint*>::iterator ci = gcs.begin();ci!=gcs.end();++ci) {
        cs[m++] = *ci;
    }
    return new IncSolver(vars.size(),vs,m,cs);
}
void GradientProjection::destroyVPSC(IncSolver *vpsc) {
    if(ccs) {
        for(CompoundConstraints::const_iterator c=ccs->begin(); c!=ccs->end();++c) {
            (*c)->updatePosition();
        }
    }
    if(clusterHierarchy) {
        clusterHierarchy->computeBoundary(*rs);
    }
    unsigned m,n;
    if(sparseQ) {
        for(unsigned i=numStaticVars;i<vars.size();i++) {
            delete vars[i];
        }
        vars.resize(numStaticVars);
        sparseQ=NULL;
    }
    for(vector<Constraint*>::iterator i=lcs.begin();i!=lcs.end();i++) {
        delete *i;
    }
    lcs.clear();
    Constraint** cs = vpsc->getConstraints(m);
    const Variable* const* vs = vpsc->getVariables(n);
    delete vpsc;
    delete [] cs;
    delete [] vs;
}
void GradientProjection::straighten(
    cola::SparseMatrix const * Q, 
    vector<SeparationConstraint*> const & cs,
    vector<straightener::Node*> const & snodes) 
{
    assert(Q->rowSize()==snodes.size());
    assert(vars.size()==numStaticVars);
    sparseQ = Q;
    for(unsigned i=numStaticVars;i<snodes.size();i++) {
        Variable* v=new vpsc::Variable(i,k==HORIZONTAL?snodes[i]->x:snodes[i]->y,1);
        assert(k==HORIZONTAL&&v->desiredPosition==snodes[i]->x
                || k==VERTICAL&&v->desiredPosition==snodes[i]->y);
        vars.push_back(v);
    }
    assert(lcs.size()==0);
    for(vector<SeparationConstraint*>::const_iterator i=cs.begin();i!=cs.end();i++) {
        (*i)->generateSeparationConstraints(vars,lcs); 
    }
}
} // namespace cola

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
