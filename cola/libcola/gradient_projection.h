#ifndef _GRADIENT_PROJECTION_H
#define _GRADIENT_PROJECTION_H

#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/rectangle.h>
#include <iostream>
#include <math.h>
#include <valarray>
#include "commondefs.h"
#include "compound_constraints.h"
#include "cluster.h"
#include "sparse_matrix.h"
#ifdef MOSEK_AVAILABLE
#include <libvpsc/mosek_quad_solve.h>
#endif

namespace straightener {
    class Node;
}
namespace cola {
using std::valarray;

enum SolveWithMosek { Off, Inner, Outer };

class GradientProjection {
public:
	GradientProjection(
        const Dim k,
		valarray<double> *denseQ,
		const double tol,
		const unsigned max_iterations,
        CompoundConstraints const *ccs,
        UnsatisfiableConstraintInfos *unsatisfiableConstraints,
        NonOverlapConstraints nonOverlapConstraints = None,
        RootCluster* clusterHierarchy = NULL,
        std::vector<vpsc::Rectangle*>* rs = NULL,
        const bool scaling = false,
        SolveWithMosek solveWithMosek = Off) 
            : k(k), 
              denseSize(unsigned(floor(sqrt(denseQ->size())))),
              denseQ(denseQ), 
              rs(rs),
              ccs(ccs),
              unsatisfiableConstraints(unsatisfiableConstraints),
              nonOverlapConstraints(nonOverlapConstraints),
              clusterHierarchy(clusterHierarchy),
              tolerance(tol), 
              max_iterations(max_iterations),
              sparseQ(NULL),
              solveWithMosek(solveWithMosek),
              scaling(scaling)
    {
        printf("GP Instance: scaling=%d, mosek=%d\n",scaling,solveWithMosek);
        for(unsigned i=0;i<denseSize;i++) {
            vars.push_back(new vpsc::Variable(i,1,1));
        }
        if(scaling) {
            scaledDenseQ.resize(denseSize*denseSize);
            for(unsigned i=0;i<denseSize;i++) {
                double d=(*denseQ)[i*denseSize+i];
                if(d!=0) {
                    vars[i]->scale=1./sqrt(fabs((*denseQ)[i*denseSize+i]));
                } else {
                    vars[i]->scale=1;
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
                    c!=ccs->end();c++) {
                (*c)->generateVariables(vars);
            }
            for(CompoundConstraints::const_iterator c=ccs->begin();
                    c!=ccs->end();c++) {
                (*c)->generateSeparationConstraints(vars,gcs);
            }
        }
        if(clusterHierarchy) {
            clusterHierarchy->createVars(k,*rs,vars);
        }
        numStaticVars=vars.size();
        //solver=setupVPSC();
	}
    static void dumpSquareMatrix(valarray<double> const &L) {
        unsigned n=(unsigned)floor(sqrt(L.size()));
        printf("Matrix %dX%d\n{",n,n);
        for(unsigned i=0;i<n;i++) {
            printf("{");
            for(unsigned j=0;j<n;j++) {
                char c=j==n-1?'}':',';
                printf("%f%c",1. * L[i*n+j],c);
            }
            char c=i==n-1?'}':',';
            printf("%c\n",c);
        }
    }

    unsigned getNumStaticVars() const {
        return numStaticVars;
    }
    ~GradientProjection() {
        //destroyVPSC(solver);
        for(Constraints::iterator i(gcs.begin()); i!=gcs.end(); i++) {
            delete *i;
        }
        gcs.clear();
        for(unsigned i=0;i<vars.size();i++) {
            delete vars[i];
        }
    }
	unsigned solve(valarray<double> const & b, valarray<double> & x);
    void unfixPos(unsigned i) {
        if(vars[i]->fixedDesiredPosition) {
            vars[i]->weight=1;
            vars[i]->fixedDesiredPosition=false;
        }
    }
    void fixPos(const unsigned i,const double pos) {
        vars[i]->weight=100000.;
        vars[i]->desiredPosition=pos;
        vars[i]->fixedDesiredPosition=true;
    }
    const Dim getDimension() const {
        return k;
    }
    void straighten(
		cola::SparseMatrix const * Q, 
        vector<SeparationConstraint*> const & ccs,
        vector<straightener::Node*> const & snodes);
    valarray<double> const & getFullResult() const {
        return result;
    }
private:
    vpsc::IncSolver* setupVPSC();
    double computeCost(valarray<double> const &b,
        valarray<double> const &x) const;
    double computeSteepestDescentVector(
        valarray<double> const &b, valarray<double> const &place,
        valarray<double> &g) const;
    double computeScaledSteepestDescentVector(
        valarray<double> const &b, valarray<double> const &place,
        valarray<double> &g) const;
    double computeStepSize(
        valarray<double> const & g, valarray<double> const & d) const;
    bool runSolver(valarray<double> & result);
    void destroyVPSC(vpsc::IncSolver *vpsc);
    Dim k;
    unsigned numStaticVars; // number of variables that persist
                              // throughout iterations
    const unsigned denseSize; // denseQ has denseSize^2 entries
	valarray<double> *denseQ; // dense square graph laplacian matrix
	valarray<double> scaledDenseQ; // scaled dense square graph laplacian matrix
    std::vector<vpsc::Rectangle*>* rs;
    CompoundConstraints const *ccs;
    UnsatisfiableConstraintInfos *unsatisfiableConstraints;
    NonOverlapConstraints nonOverlapConstraints;
    Cluster* clusterHierarchy;
    double tolerance;
    unsigned max_iterations;
    cola::SparseMatrix const * sparseQ; // sparse components of goal function
	Variables vars; // all variables
                          // computations
    Constraints gcs; /* global constraints - persist throughout all
                                iterations */
    Constraints lcs; /* local constraints - only for current iteration */
    Constraints cs; /* working list of constraints: gcs +lcs */
    valarray<double> result;
#ifdef MOSEK_AVAILABLE
    MosekEnv* menv;
#endif
    vpsc::IncSolver* solver;
    SolveWithMosek solveWithMosek;
    const bool scaling;
};
} // namespace cola

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
