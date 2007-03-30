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

namespace straightener {
    class Node;
}
namespace cola {
using std::valarray;

class GradientProjection {
public:
	GradientProjection(
        const Dim k,
		valarray<double> const & denseQ,
		const double tol,
		const unsigned max_iterations,
        CompoundConstraints const * ccs,
        NonOverlapConstraints nonOverlapConstraints = None,
        RootCluster* clusterHierarchy = NULL,
        std::vector<vpsc::Rectangle*>* rs = NULL) 
            : k(k), 
              denseSize(unsigned(floor(sqrt(denseQ.size())))),
              denseQ(denseQ), 
              rs(rs),
              ccs(ccs),
              nonOverlapConstraints(nonOverlapConstraints),
              clusterHierarchy(clusterHierarchy),
              tolerance(tol), 
              max_iterations(max_iterations),
              sparseQ(NULL)
    {
        for(unsigned i=0;i<denseSize;i++) {
            vars.push_back(new vpsc::Variable(i,1,1));
        }
        scaledDenseQ.resize(denseSize*denseSize);
        for(unsigned i=0;i<denseSize;i++) {
            double s=1./sqrt(fabs(denseQ[i*denseSize+i]));
            for(unsigned j=0;j<denseSize;j++) {
                scaledDenseQ[i*denseSize+j]=denseQ[i*denseSize+j]*s
                    *1./sqrt(fabs(denseQ[j*denseSize+j]));
            }
        }

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
	}
    unsigned getNumStaticVars() const {
        return numStaticVars;
    }
    ~GradientProjection() {
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
    void destroyVPSC(vpsc::IncSolver *vpsc);
    Dim k;
    unsigned numStaticVars; // number of variables that persist
                              // throughout iterations
    const unsigned denseSize; // denseQ has denseSize^2 entries
	valarray<double> const & denseQ; // dense square graph laplacian matrix
	valarray<double> scaledDenseQ; // scaled dense square graph laplacian matrix
    std::vector<vpsc::Rectangle*>* rs;
    CompoundConstraints const * ccs;
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
    valarray<double> result;
};
} // namespace cola

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
