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

namespace cola {
using std::valarray;

class GradientProjection {
public:
	GradientProjection(
        const Dim k,
		valarray<double> const & denseQ,
		valarray<double>& x,
		const double tol,
		const unsigned max_iterations,
        CompoundConstraints const * ccs,
        NonOverlapConstraints nonOverlapConstraints = None,
        RootCluster* clusterHierarchy = NULL,
        std::vector<vpsc::Rectangle*>* rs = NULL,
		cola::SparseMap *Q = NULL)
            : k(k), 
              n(x.size()), 
              denseSize(unsigned(floor(sqrt(denseQ.size())))),
              denseQ(denseQ), 
              place(x),
              rs(rs),
              ccs(ccs),
              nonOverlapConstraints(nonOverlapConstraints),
              clusterHierarchy(clusterHierarchy),
              tolerance(tol), 
              max_iterations(max_iterations),
              sparseQ(NULL),
              localSparseMapCreated(false)
    {
        for(unsigned i=0;i<n;i++) {
            vars.push_back(new vpsc::Variable(i,1,1));
        }
        if(ccs) {
            cola::SparseMap* oldQ = Q;
            for(CompoundConstraints::const_iterator c=ccs->begin();
                    c!=ccs->end();c++) {
                (*c)->generateSeparationConstraints(vars,gcs,Q);
                if(Q!=oldQ) localSparseMapCreated = true;
            }
        }
        if(clusterHierarchy) {
            clusterHierarchy->createVars(k,*rs,vars);
        }
        n = vars.size();
        if(x.size()<n) {
            // if we added new variables above then we'll have to resize the
            // coords array accordingly
            valarray<double> tmp=x;
            x.resize(n);
            for(unsigned i=0;i<tmp.size();i++) {
                x[i]=tmp[i];
            }
            for(unsigned i=tmp.size();i<n;i++) {
                x[i]=vars[i]->desiredPosition;
            }
        }
        if(Q) {
            sparseQ = new cola::SparseMatrix(*Q);
            //sparseQ->print();
        }
	}
    ~GradientProjection() {
        for(Constraints::iterator i(gcs.begin()); i!=gcs.end(); i++) {
            delete *i;
        }
        gcs.clear();
        for(unsigned i=0;i<vars.size();i++) {
            delete vars[i];
        }
        if(localSparseMapCreated) delete sparseQ;
    }
	unsigned solve(valarray<double> const & b);
    unsigned getSize() { return n; }
    void unfixPos(unsigned i) {
        if(vars[i]->fixedDesiredPosition) {
            vars[i]->weight=1;
            vars[i]->fixedDesiredPosition=false;
        }
    }
    void fixPos(unsigned i,double pos) {
        vars[i]->weight=100000.;
        vars[i]->desiredPosition=pos;
        vars[i]->fixedDesiredPosition=true;
        place[i]=pos;
    }
    Dim getDimension() {
        return k;
    }
private:
    vpsc::IncSolver* setupVPSC();
    double computeSteepestDescentVector(
        valarray<double> const &b, valarray<double> const &place,
        valarray<double> &g);
    double computeScaledSteepestDescentVector(
        valarray<double> const &b, valarray<double> const &place,
        valarray<double> &g);
    double computeStepSize(
        valarray<double> const & g, valarray<double> const & d);
    void destroyVPSC(vpsc::IncSolver *vpsc);
    Dim k;
	unsigned n; // number of actual vars - this may grow in the constructor as we add
                // dummy vars
    const unsigned denseSize; // denseQ has denseSize^2 entries
	valarray<double> const & denseQ; // dense square graph laplacian matrix
    valarray<double> & place;
    std::vector<vpsc::Rectangle*>* rs;
    CompoundConstraints const * ccs;
    NonOverlapConstraints nonOverlapConstraints;
    Cluster* clusterHierarchy;
    double tolerance;
    unsigned max_iterations;
    cola::SparseMatrix * sparseQ; // sparse components of goal function
	Variables vars; // all variables
                          // computations
    Constraints gcs; /* global constraints - persist throughout all
                                iterations */
    Constraints lcs; /* local constraints - only for current iteration */
    bool localSparseMapCreated;
};
} // namespace cola

#endif /* _GRADIENT_PROJECTION_H */

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
