/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

/**********************************************************
 *
 * Solve a quadratic function f(X) = X' A X + b X
 * subject to a set of separation constraints cs
 *
 **********************************************************/

#include <iostream>
#include <cmath>
#include <ctime>

#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/assertions.h>

#include "libcola/cluster.h"
#include "libcola/gradient_projection.h"
#include "libcola/straightener.h"
#include "libcola/commondefs.h"
//#define CHECK_CONVERGENCE_BY_COST 1


using namespace std;
using namespace vpsc;
namespace cola {
GradientProjection::GradientProjection(
    const Dim k,
    std::valarray<double> *denseQ,
    const double tol,
    const unsigned max_iterations,
    CompoundConstraints const *ccs,
    UnsatisfiableConstraintInfos *unsatisfiableConstraints,
    NonOverlapConstraintsMode nonOverlapConstraints,
    RootCluster* clusterHierarchy,
    vpsc::Rectangles* rs,
    const bool scaling,
    SolveWithMosek solveWithMosek) 
        : k(k), 
          denseSize(static_cast<unsigned>((floor(sqrt(static_cast<double>(denseQ->size())))))),
          denseQ(denseQ), 
          rs(rs),
          ccs(ccs),
          unsatisfiableConstraints(unsatisfiableConstraints),
          nonOverlapConstraints(nonOverlapConstraints),
          clusterHierarchy(clusterHierarchy),
          tolerance(tol), 
          max_iterations(max_iterations),
          sparseQ(nullptr),
          solveWithMosek(solveWithMosek),
          scaling(scaling)
{
    //printf("GP Instance: scaling=%d, mosek=%d\n",scaling,solveWithMosek);
    for(unsigned i=0;i<denseSize;i++) {
        vars.push_back(new vpsc::Variable(i,1,1));
    }
    if(scaling) {
        scaledDenseQ.resize(denseSize*denseSize);
        for(unsigned i=0;i<denseSize;i++) {
            vars[i]->scale=1./sqrt(fabs((*denseQ)[i*denseSize+i]));
            // XXX: Scale can sometimes be set to infinity here when 
            //      there are nodes not connected to any other node.
            //      Thus we just set the scale for such a variable to 1.
            if (!std::isfinite(vars[i]->scale))
            {
                vars[i]->scale = 1;
            }
        }
        // the following computes S'QS for Q=denseQ
        // and S is diagonal matrix of scale factors
        for(unsigned i=0;i<denseSize;i++) {
            for(unsigned j=0;j<denseSize;j++) {
                scaledDenseQ[i*denseSize+j]=(*denseQ)[i*denseSize+j]*vars[i]->scale
                    *vars[j]->scale;
            }
        }
        this->denseQ = &scaledDenseQ;
    }
    //dumpSquareMatrix(*this->denseQ);
    //dumpSquareMatrix(scaledDenseQ);

    if(ccs) {
        for(CompoundConstraints::const_iterator c=ccs->begin();
                c!=ccs->end();++c) {
            (*c)->generateVariables(k, vars);
            OrthogonalEdgeConstraint* e=dynamic_cast<OrthogonalEdgeConstraint*>(*c);
            if(e) {
                orthogonalEdges.push_back(e);
            }
        }
        for(CompoundConstraints::const_iterator c=ccs->begin();
                c!=ccs->end();++c) {
            (*c)->generateSeparationConstraints(k, vars, gcs, *rs);
        }
    }
    /*
    if(clusterHierarchy) {
        clusterHierarchy->createVars(k,*rs,vars);
    }
    */
    numStaticVars=vars.size();
    //solver=setupVPSC();
}
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
            Ax[i] += (*denseQ)[i*denseSize+j]*x[j];
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
    COLA_ASSERT(x.size()==b.size() && b.size()==g.size());
    g = b;
    for (unsigned i=0; i<denseSize; i++) {
        for (unsigned j=0; j<denseSize; j++) {
            g[i] -= (*denseQ)[i*denseSize+j]*x[j];
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
    COLA_ASSERT(g.size()==d.size());
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
            r += (*denseQ)[i*denseSize+j] * d[j];
        } }
        denominator += r * d[i];
    }
    if(denominator==0) {
        return 0;
    }
    return numerator/(2.*denominator);
}

bool GradientProjection::runSolver(valarray<double> & result) {
    bool activeConstraints = false;
    switch(solveWithMosek) {
        case Off:
            activeConstraints = solver->satisfy();
            for (unsigned i=0;i<vars.size();i++) {
                result[i]=vars[i]->finalPosition;
            }
            break;
        case Inner:
#ifdef MOSEK_AVAILABLE
            float *ba=new float[vars.size()];
            float *coords=new float[vars.size()];
            for(unsigned i=0;i<vars.size();i++) {
                ba[i]=vars[i]->desiredPosition;
            }
            mosek_quad_solve_sep(menv,ba,coords);
            for(unsigned i=0;i<vars.size();i++) {
                //printf("x[%d]=%f\n",i,coords[i]);
                result[i]=coords[i];
            }
            delete [] ba;
            delete [] coords;
            break;
#endif
        default:
            break;
    }
    return activeConstraints;
}

/*
 * Use gradient-projection to solve an instance of
 * the Variable Placement with Separation Constraints problem.
 */
unsigned GradientProjection::solve(
        valarray<double> const &linearCoefficients, 
        valarray<double> &x) {
    COLA_ASSERT(linearCoefficients.size()==x.size());
    COLA_ASSERT(x.size()==denseSize);
    COLA_ASSERT(numStaticVars>=denseSize);
    COLA_ASSERT(sparseQ==nullptr || 
                (sparseQ!=nullptr && (vars.size()==sparseQ->rowSize())) );

    if(max_iterations==0) return 0;

    bool converged=false;

    solver = setupVPSC();
#ifdef MOSEK_AVAILABLE
    if(solveWithMosek==Outer) {
        float* ba=new float[vars.size()];
        float* xa=new float[vars.size()];
        for(unsigned i=0;i<vars.size();i++) {
            ba[i]=-linearCoefficients[i];
        }
        mosek_quad_solve_sep(menv,ba,xa);
        for(unsigned i=0;i<vars.size();i++) {
            //printf("mosek result x[%d]=%f\n",i,xa[i]);
            x[i]=xa[i];
        }
        delete [] ba;
        delete [] xa;
        return 1;
    }
#endif
    // it may be that we have to consider dummy vars, which the caller didn't know
    // about.  Thus vars.size() may not equal x.size()
    unsigned n = vars.size();
    valarray<double> b(n);
    result.resize(n);
    
    // load desired positions into vars, note that we keep desired positions 
    // already calculated for dummy vars
    for (unsigned i=0;i<x.size();i++) {
        COLA_ASSERT(!std::isnan(x[i]));
        COLA_ASSERT(std::isfinite(x[i]));
        b[i]=i<linearCoefficients.size()?linearCoefficients[i]:0;
        result[i]=x[i];
        if(scaling) {
            b[i]*=vars[i]->scale;
            result[i]/=vars[i]->scale;
        } 
        if(!vars[i]->fixedDesiredPosition) vars[i]->desiredPosition=result[i];
    }
    runSolver(result);
        
    valarray<double> g(n); /* gradient */
    valarray<double> previous(n); /* stored positions */
    valarray<double> d(n); /* actual descent vector */

#ifdef CHECK_CONVERGENCE_BY_COST
    double previousCost = DBL_MAX;
#endif
    unsigned counter=0;
    double stepSize;
    for (; counter<max_iterations&&!converged; counter++) {
        previous=result;
        stepSize=0;
        double alpha=computeSteepestDescentVector(b,result,g);

        //printf("Iteration[%d]\n",counter);
        // move to new unconstrained position
        for (unsigned i=0; i<n; i++) {
            // dividing by variable weight is a cheap trick to make these
            // weights mean something in terms of the descent vector
            double step=alpha*g[i]/vars[i]->weight;
            result[i]+=step;
            //printf("   after unconstrained step: x[%d]=%f\n",i,result[i]);
            stepSize+=step*step;
            COLA_ASSERT(!std::isnan(result[i]));
            COLA_ASSERT(std::isfinite(result[i]));
            if(!vars[i]->fixedDesiredPosition) vars[i]->desiredPosition=result[i];
        }

        //project to constraint boundary
        bool constrainedOptimum = false;
        constrainedOptimum=runSolver(result);
        stepSize=0;
        for (unsigned i=0;i<n;i++) {
            double step = previous[i]-result[i];
            stepSize+=step*step;
        }
        //constrainedOptimum=false;
        // beta seems, more often than not, to be >1!
        if(constrainedOptimum) {
            // The following step limits the step-size in the feasible
            // direction
            d = result - previous;
            const double beta = 0.5*computeStepSize(g, d);
            // beta > 1.0 takes us back outside the feasible region
            // beta < 0 clearly not useful and may happen due to numerical imp.
            //printf("beta=%f\n",beta);
            if(beta>0&&beta<0.99999) {
                stepSize=0;
                for (unsigned i=0; i<n; i++) {
                    double step=beta*d[i];
                    result[i]=previous[i]+step;
                    stepSize+=step*step;
                }
            }
        }
#ifdef CHECK_CONVERGENCE_BY_COST
        /* This would be the slow way to detect convergence */
        //if(counter%2) {
            double cost = computeCost(b,result);
            printf("     gp[%d] %.15f %.15f\n",counter,previousCost,cost);
            //COLA_ASSERT(previousCost>cost);
            if(fabs(previousCost - cost) < tolerance) {
                converged = true;
            }
            previousCost = cost;
        //}
#else
        if(stepSize<tolerance) converged = true; 
#endif
    }
    //printf("GP[%d] converged after %d iterations.\n",k,counter);
    for(unsigned i=0;i<x.size();i++) {
        x[i]=result[i];
        if(scaling) {
            x[i]*=vars[i]->scale;
        }
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
    if(nonOverlapConstraints!=None) {
        if(clusterHierarchy) {
            //printf("Setup up cluster constraints, dim=%d--------------\n",k);
            //clusterHierarchy->generateNonOverlapConstraints(k,nonOverlapConstraints,*rs,vars,lcs);
        } else {
            for(vector<OrthogonalEdgeConstraint*>::iterator i=orthogonalEdges.begin();i!=orthogonalEdges.end();i++) {
                OrthogonalEdgeConstraint* e=*i;
                e->generateTopologyConstraints(k,*rs,vars,lcs);
            }
            if(k==HORIZONTAL) {
                // Make rectangles a little bit wider when processing horizontally so that any overlap
                // resolved horizontally is strictly non-overlapping when processing vertically
                Rectangle::setXBorder(0.0001);
                // use rs->size() rather than n because some of the variables may
                // be dummy vars with no corresponding rectangle
                generateXConstraints(*rs,vars,lcs,nonOverlapConstraints==Both?true:false); 
                Rectangle::setXBorder(0);
            } else {
                generateYConstraints(*rs,vars,lcs); 
            }
        }
    }
    cs=gcs;
    cs.insert(cs.end(),lcs.begin(),lcs.end());
    switch(solveWithMosek) {
        case Off:
            break;
#ifdef MOSEK_AVAILABLE
        case Inner:
            menv = mosek_init_sep_ls(vars.size(),cs);
            break;
        case Outer:
            unsigned n = vars.size();
            float* lap = new float[n*(n+1)/2];
            unsigned k=0;
            for(unsigned i=0;i<n;i++) {
                for(unsigned j=i;j<n;j++) {
                    lap[k]=(*denseQ)[i*n+j];
                    k++;
                }
            }
            menv = mosek_init_sep(lap,n,cs,1);
            delete [] lap;
            break;
#endif
        default:
            break;
    }
    return new IncSolver(vars,cs);
}
void GradientProjection::destroyVPSC(IncSolver *vpsc) {
    if(ccs) {
        for(CompoundConstraints::const_iterator c=ccs->begin(); 
                c!=ccs->end();++c) {
            (*c)->updatePosition(vpsc::XDIM);
            (*c)->updatePosition(vpsc::YDIM);
        }
    }
    if(unsatisfiableConstraints) {
        unsatisfiableConstraints->clear();
        for(Constraints::iterator i=cs.begin();i!=cs.end();i++) {
            Constraint* c=*i;
            if(c->unsatisfiable) {
                UnsatisfiableConstraintInfo *ci = new UnsatisfiableConstraintInfo(c);
                unsatisfiableConstraints->push_back(ci);
            }
        }
    }
    if(clusterHierarchy) {
        clusterHierarchy->computeBoundary(*rs);
    }
    if(sparseQ) {
        for(unsigned i=numStaticVars;i<vars.size();i++) {
            delete vars[i];
        }
        vars.resize(numStaticVars);
        sparseQ=nullptr;
    }
    for(vector<Constraint*>::iterator i=lcs.begin();i!=lcs.end();i++) {
        delete *i;
    }
    lcs.clear();
    delete vpsc;
#ifdef MOSEK_AVAILABLE
    if(solveWithMosek!=Off) mosek_delete(menv);
#endif
}
void GradientProjection::straighten(
    cola::SparseMatrix const * Q, 
    vector<SeparationConstraint*> const & cs,
    vector<straightener::Node*> const & snodes) 
{
    COLA_ASSERT(Q->rowSize()==snodes.size());
    COLA_ASSERT(vars.size()==numStaticVars);
    sparseQ = Q;
    for(unsigned i=numStaticVars;i<snodes.size();i++) {
        Variable* v=new vpsc::Variable(i,snodes[i]->pos[k],1);
        COLA_ASSERT(v->desiredPosition==snodes[i]->pos[k]);
        vars.push_back(v);
    }
    COLA_ASSERT(lcs.size()==0);
    for(vector<SeparationConstraint*>::const_iterator i=cs.begin();i!=cs.end();i++) {
        (*i)->generateSeparationConstraints(k, vars, lcs, *rs); 
    }
}
} // namespace cola
