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
#include <assert.h>
#include <solve_VPSC.h>
#include <variable.h>
#include <constraint.h>
#include "gradient_projection.h"
#include <iostream>
#include "boost/tuple/tuple.hpp"

using namespace std;
//#define CONMAJ_LOGGING 1

/*
 * Use gradient-projection to solve an instance of
 * the Variable Placement with Separation Constraints problem.
 */
unsigned GradientProjection::solve (double * b) {
	unsigned i,j,counter;
	if(max_iterations==0) return 0;

	bool converged=false;
    bool constrained = gcs || nonOverlapConstraints;

    IncVPSC* vpsc=NULL;
    Constraint** lcs=NULL;

    if(constrained) {
        boost::tie(vpsc,lcs) = setupVPSC();
	    for (i=0;i<n;i++) {
		    vs[i]->desiredPosition=place[i];
	    }
        vpsc->satisfy();
        for (i=0;i<n;i++) {
            place[i]=vs[i]->position();
        }
    }
	
	for (counter=0; counter<max_iterations&&!converged; counter++) {
		converged=true;		
		// find steepest descent direction
		for (i=0; i<n; i++) {
			old_place[i]=place[i];
			g[i] = 2*b[i];
			for (j=0; j<n; j++) {
				g[i] -= 2*A[i][j]*place[j];
			}
		}		
		float numerator = 0, denominator = 0, r;
		for (i=0; i<n; i++) {
			numerator += g[i]*g[i];
			r=0;
			for (j=0; j<n; j++) {
				r += 2*A[i][j]*g[j];
			}
			denominator -= r*g[i];
		}
		float alpha = numerator/denominator;
		for (i=0; i<n; i++) {
			place[i]-=alpha*g[i];
		}
        if(constrained) {
            //project to constraint boundary
            for (i=0;i<n;i++) {
                vs[i]->desiredPosition=place[i];
            }
            vpsc->satisfy();
            for (i=0;i<n;i++) {
                place[i]=vs[i]->position();
            }
        }
		// set place to the intersection of old_place-g and boundary and compute d, the vector from intersection pnt to projection pnt
		for (i=0; i<n; i++) {
			d[i]=place[i]-old_place[i];
		}	
		// now compute beta
		numerator = 0, denominator = 0;
		for (i=0; i<n; i++) {
			numerator += g[i]*d[i];
			r=0;
			for (j=0; j<n; j++) {
				r += 2*A[i][j]*d[j];
			}
			denominator += r*d[i];
		}
		float beta = numerator/denominator;

		float test=0;
		for (i=0; i<n; i++) {
			// beta > 1.0 takes us back outside the feasible region
			// beta < 0 clearly not useful and may happen due to numerical imp.
			if(beta>0&&beta<1.0) {
				place[i]=old_place[i]+beta*d[i];
			}
			test+= fabs(place[i]-old_place[i]);
		}
		if(test>tolerance) {
			converged=false;
		}
	}
    if(constrained) destroyVPSC(vpsc,lcs);
	return counter;
}
// Setup an instance of the Variable Placement with Separation Constraints
// for one iteration.
// Generate transient local constraints --- such as non-overlap constraints 
// --- that are only relevant to one iteration, and merge these with the
// global constraint list (including alignment constraints,
// dir-edge constraints, containment constraints, etc).
std::pair<IncVPSC*, Constraint**> GradientProjection::setupVPSC() {
    Constraint **cs, **lcs=NULL;
    unsigned m=0;
    if(nonOverlapConstraints) {
        if(k==HORIZONTAL) {
            Rectangle::setXBorder(0.0001);
            m=generateXConstraints(n,rs,vs,lcs,true); 
            Rectangle::setXBorder(0);
        } else {
            m=generateYConstraints(n,rs,vs,lcs); 
        }
    }
    unsigned gm = 0;
    if(gcs) gm=gcs->size();
    cs = new Constraint*[m + gm];
    for(unsigned i = 0 ; i<m; i++) {
        cs[i] = lcs[i];
    }
    if(gcs) for(Constraints::iterator ci = gcs->begin();ci!=gcs->end();++ci) {
        cs[m++] = *ci;
    }
    return std::make_pair(new IncVPSC(n+n_dummy,vs,m,cs), lcs);
}
void GradientProjection::destroyVPSC(IncVPSC *vpsc, Constraint **lcs) {
    if(acs) {
        for(AlignmentConstraints::iterator ac=acs->begin(); ac!=acs->end();++ac) {
            (*ac)->updatePosition();
        }
    }
    Constraint** cs = vpsc->getConstraints();
    delete vpsc;
    delete [] cs;
    if(lcs) {
        unsigned m = sizeof(lcs)/sizeof(Constraint*);
        for(unsigned i=0;i<m;i++) {
            //delete lcs[i];
        }
        delete [] lcs;
    }
}

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
