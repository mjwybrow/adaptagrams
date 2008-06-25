#ifndef _GRADIENT_PROJECTION_H
#define _GRADIENT_PROJECTION_H

#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/rectangle.h>
#include <iostream>
#include <cmath>
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

enum SolveWithMosek { Off, Inner, Outer };

class GradientProjection {
public:
	GradientProjection(
		const vpsc::Dim k,
		std::valarray<double> *denseQ,
		const double tol,
		const unsigned max_iterations,
        CompoundConstraints const *ccs,
        UnsatisfiableConstraintInfos *unsatisfiableConstraints,
        NonOverlapConstraints nonOverlapConstraints = None,
        RootCluster* clusterHierarchy = NULL,
		vpsc::Rectangles* rs = NULL,
        const bool scaling = false,
        SolveWithMosek solveWithMosek = Off);
    static void dumpSquareMatrix(std::valarray<double> const &L) {
        unsigned n=static_cast<unsigned>(floor(sqrt(static_cast<double>(L.size()))));
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
        for(vpsc::Constraints::iterator i(gcs.begin()); i!=gcs.end(); i++) {
            delete *i;
        }
        gcs.clear();
        for(unsigned i=0;i<vars.size();i++) {
            delete vars[i];
        }
    }
	unsigned solve(std::valarray<double> const & b, std::valarray<double> & x);
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
	const vpsc::Dim getDimension() const {
        return k;
    }
    void straighten(
		cola::SparseMatrix const * Q, 
		std::vector<SeparationConstraint*> const & ccs,
		std::vector<straightener::Node*> const & snodes);
    std::valarray<double> const & getFullResult() const {
        return result;
    }
private:
    vpsc::IncSolver* setupVPSC();
    double computeCost(std::valarray<double> const &b,
        std::valarray<double> const &x) const;
    double computeSteepestDescentVector(
        std::valarray<double> const &b, std::valarray<double> const &place,
        std::valarray<double> &g) const;
    double computeScaledSteepestDescentVector(
        std::valarray<double> const &b, std::valarray<double> const &place,
        std::valarray<double> &g) const;
    double computeStepSize(
        std::valarray<double> const & g, std::valarray<double> const & d) const;
    bool runSolver(std::valarray<double> & result);
    void destroyVPSC(vpsc::IncSolver *vpsc);
	vpsc::Dim k;
    unsigned numStaticVars; // number of variables that persist
                              // throughout iterations
    const unsigned denseSize; // denseQ has denseSize^2 entries
	std::valarray<double> *denseQ; // dense square graph laplacian matrix
	std::valarray<double> scaledDenseQ; // scaled dense square graph laplacian matrix
    std::vector<vpsc::Rectangle*>* rs;
    CompoundConstraints const *ccs;
    UnsatisfiableConstraintInfos *unsatisfiableConstraints;
    NonOverlapConstraints nonOverlapConstraints;
    Cluster* clusterHierarchy;
    double tolerance;
    unsigned max_iterations;
    cola::SparseMatrix const * sparseQ; // sparse components of goal function
	vpsc::Variables vars; // all variables
                          // computations
    vpsc::Constraints gcs; /* global constraints - persist throughout all
                                iterations */
    vpsc::Constraints lcs; /* local constraints - only for current iteration */
    vpsc::Constraints cs; /* working list of constraints: gcs +lcs */
    std::valarray<double> result;
#ifdef MOSEK_AVAILABLE
    MosekEnv* menv;
#endif
    vpsc::IncSolver* solver;
    SolveWithMosek solveWithMosek;
    const bool scaling;
	std::vector<OrthogonalEdgeConstraint*> orthogonalEdges;
};
} // namespace cola

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
