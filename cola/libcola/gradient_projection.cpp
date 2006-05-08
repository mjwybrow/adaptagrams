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
    if(!dummy_vars.empty()) {
        solve_dummy(b);
    }

	bool converged=false;

    IncVPSC* vpsc=NULL;

    if(constrained) {
    // find initial placement with all constraints satisfied
        vpsc = setupVPSC();
	    for (i=0;i<n;i++) {
		    vars[i]->desiredPosition=place[i];
	    }
        vpsc->satisfy();
        for (i=0;i<n;i++) {
            place[i]=vars[i]->position();
        }
    }
    cout << "X=";
    for (i=0;i<n;i++) {
        cout <<" "<<place[i];
    }
    cout << endl;
	
	for (counter=0; counter<max_iterations&&!converged; counter++) {
		converged=true;		
		// find steepest descent direction
        //  g = 2 ( b - Ax )
    cout << "g=";
		for (i=0; i<n; i++) {
			old_place[i]=place[i];
			g[i] = 2*b[i];
			for (j=0; j<n; j++) {
				g[i] -= 2*A[i][j]*place[j];
			}
        cout <<" "<<g[i];
		}		
    cout << endl;
        // compute step size: alpha = ( g' g ) / ( 2 g' A g )
		double numerator = 0, denominator = 0, r;
		for (i=0; i<n; i++) {
			numerator += g[i]*g[i];
			r=0;
			for (j=0; j<n; j++) {
				r += 2*A[i][j]*g[j];
			}
			denominator -= r*g[i];
		}
		double alpha = numerator/denominator;
        if(fabs(numerator)+fabs(denominator) < tolerance) {
            alpha=1;
        }
        cout << "numerator=" << numerator << " denominator=" << denominator << " alpha=" << alpha << endl;

        // move to new unconstrained position
		for (i=0; i<n; i++) {
			place[i]-=alpha*g[i];
		}
        if(constrained) {
            //project to constraint boundary
            for (i=0;i<n;i++) {
                vars[i]->desiredPosition=place[i];
            }
            vpsc->satisfy();
            for (i=0;i<n;i++) {
                place[i]=vars[i]->position();
            }
        }
        // compute d, the vector from last pnt to projection pnt
		for (i=0; i<n; i++) {
			d[i]=place[i]-old_place[i];
		}	
		// now compute beta, optimal step size from last pnt to projection pnt
        //   beta = ( g' d ) / ( 2 d' A d )
		numerator = 0, denominator = 0;
		for (i=0; i<n; i++) {
			numerator += g[i]*d[i];
			r=0;
			for (j=0; j<n; j++) {
				r += 2*A[i][j]*d[j];
			}
			denominator += r*d[i];
		}
		double beta = numerator/denominator;

		double test=0;
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
    if(constrained) destroyVPSC(vpsc);
	return counter;
}
/*
 * Use gradient-projection to solve an instance of
 * the Variable Placement with Separation Constraints problem.
 * Uses messy sparse matrix techniques to handle pairs of dummy
 * vars.
 */
unsigned GradientProjection::solve_dummy (double * b) {
	unsigned i,j,counter;
	if(max_iterations==0) return 0;

    unsigned n_d = dummy_vars.size();
    cout << "In solve_DUMMY_vars, n_d=" << n_d << endl;

	bool converged=false;

    IncVPSC* vpsc=NULL;

    vpsc = setupVPSC();
    for (i=0;i<n;i++) {
        vars[i]->desiredPosition=place[i];
    }
    try {
        vpsc->satisfy();
    } catch (char const *str) {
		std::cerr<<str<<std::endl;
		for(int i=0;i<n;i++) {
			std::cerr << *rs[i]<<std::endl;
		}
	}

    for (i=0;i<n;i++) {
        place[i]=vars[i]->position();
    }
    for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
        DummyVarPair* p = *it;
        p->place_l=p->left->position();
        p->place_r=p->right->position();
    }
	
	for (counter=0; counter<max_iterations&&!converged; counter++) {
		converged=true;		
		// find steepest descent direction
        //  g = 2 ( b - Ax )
		for (i=0; i<n; i++) {
			old_place[i]=place[i];
			g[i] = b[i];
			for (j=0; j<n; j++) {
				g[i] -= A[i][j]*place[j];
			}
            g[i] *= 2.0;
		}		
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            DummyVarPair* p = *it;
            p->old_place_l=p->place_l;
            p->old_place_r=p->place_r;
            p->g = 2.0 * ( p->b + p->lap2 * ( p->place_l - p->place_r ) );
        }
        // compute step size: alpha = ( g' g ) / ( 2 g' A g )
        //   g terms for dummy vars cancel out so don't consider
		double numerator = 0, denominator = 0, r;
		for (i=0; i<n; i++) {
			numerator += g[i]*g[i];
			r=0;
			for (j=0; j<n; j++) {
				r += A[i][j]*g[j];
			}
			denominator -= 2.0 * r*g[i];
		}
		double alpha = numerator/denominator;

        // move to new unconstrained position
		for (i=0; i<n; i++) {
			place[i]-=alpha*g[i];
            vars[i]->desiredPosition=place[i];
		}
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            DummyVarPair* p = *it;
            p->place_l -= alpha*p->g;
            p->place_r += alpha*p->g;
            p->left->desiredPosition=p->place_l;
            p->right->desiredPosition=p->place_r;
        }

        //project to constraint boundary
        try {
            vpsc->satisfy();
        } catch (char const *str) {
            std::cerr<<str<<std::endl;
            for(int i=0;i<n;i++) {
                std::cerr << *rs[i]<<std::endl;
            }
        }
        for (i=0;i<n;i++) {
            place[i]=vars[i]->position();
        }
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            DummyVarPair* p = *it;
            p->place_l=p->left->position();
            p->place_r=p->right->position();
        }
        // compute d, the vector from last pnt to projection pnt
		for (i=0; i<n; i++) {
			d[i]=place[i]-old_place[i];
		}	
		// now compute beta, optimal step size from last pnt to projection pnt
        //   beta = ( g' d ) / ( 2 d' A d )
		numerator = 0, denominator = 0;
		for (i=0; i<n; i++) {
			numerator += g[i] * d[i];
			r=0;
			for (j=0; j<n; j++) {
				r += A[i][j] * d[j];
			}
			denominator += 2.0 * r * d[i];
		}
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            DummyVarPair* p = *it;
            double pdl = p->place_l-p->old_place_l;
            double pdr = p->place_r-p->old_place_r;
            numerator += p->g * ( pdl - pdr );
            r = 2 * p->lap2 * ( pdr - pdl );
            denominator += r*pdl - r * pdr;
        }
		double beta = numerator/denominator;

        // beta > 1.0 takes us back outside the feasible region
        // beta < 0 clearly not useful and may happen due to numerical imp.
        if(beta>0&&beta<1.0) {
            for (i=0; i<n; i++) {
                place[i]=old_place[i]+beta*d[i];
            }
            for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
                DummyVarPair* p = *it;
                p->left->desiredPosition += beta*(p->left->position() - p->left->desiredPosition);
                p->right->desiredPosition += beta*(p->right->position() - p->right->desiredPosition);
            }
        }
		double test=0;
		for (i=0; i<n; i++) {
			test += fabs(place[i]-old_place[i]);
		}
        for (DummyVars::iterator it=dummy_vars.begin();it!=dummy_vars.end();++it){
            DummyVarPair* p = *it;
            test += fabs(p->place_l-p->old_place_l) 
                + fabs(p->place_r-p->old_place_r);
        }
		if(test>tolerance) {
			converged=false;
		}
	}
    destroyVPSC(vpsc);
	return counter;
}
// Setup an instance of the Variable Placement with Separation Constraints
// for one iteration.
// Generate transient local constraints --- such as non-overlap constraints 
// --- that are only relevant to one iteration, and merge these with the
// global constraint list (including alignment constraints,
// dir-edge constraints, containment constraints, etc).
IncVPSC* GradientProjection::setupVPSC() {
    Constraint **cs;
    //assert(lcs.size()==0);
    
    for(DummyVars::iterator dit=dummy_vars.begin();
            dit!=dummy_vars.end(); ++dit) {
        DummyVarPair* p = *dit;
        p->left = new Variable(vars.size(),p->place_l,1/*weight*/);
        vars.push_back(p->left);
        p->right = new Variable(vars.size(),p->place_r,1/*weight*/);
        vars.push_back(p->right);
        for(CList::iterator cit=p->leftof.begin();
                cit!=p->leftof.end(); ++cit) {
            Variable* v = vars[(*cit).first];
            lcs.push_back(new Constraint(p->left,v,(*cit).second)); 
        }
        for(CList::iterator cit=p->rightof.begin();
                cit!=p->rightof.end(); ++cit) {
            Variable* v = vars[(*cit).first];
            lcs.push_back(new Constraint(v,p->right,(*cit).second)); 
        }
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
    for(Constraints::iterator ci = lcs.begin();ci!=lcs.end();++ci) {
        cs[m++] = *ci;
    }
    for(Constraints::iterator ci = gcs.begin();ci!=gcs.end();++ci) {
        cs[m++] = *ci;
    }
    return new IncVPSC(vars.size(),vs,m,cs);
}
void GradientProjection::destroyVPSC(IncVPSC *vpsc) {
    if(acs) {
        for(AlignmentConstraints::iterator ac=acs->begin(); ac!=acs->end();++ac) {
            (*ac)->updatePosition();
        }
    }
    Constraint** cs = vpsc->getConstraints();
    Variable** vs = vpsc->getVariables();
    delete vpsc;
    delete [] cs;
    delete [] vs;
    for(Constraints::iterator i=lcs.begin();i!=lcs.end();i++) {
            delete *i;
    }
    lcs.clear();
    cout << " Vars count = " << vars.size() << " Dummy vars cnt=" << dummy_vars.size() << endl;
    vars.resize(vars.size()-dummy_vars.size()*2);
    for(DummyVars::iterator i=dummy_vars.begin();i!=dummy_vars.end();++i) {
        DummyVarPair* p = *i;
        delete p->left;
        delete p->right;
    }
}

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
