#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include "cola.h"
#include "compound_constraints.h"

using std::vector;

namespace cola {
struct GenerateVariables {
    GenerateVariables(vpsc::Variables& vars) : vars(vars) {}
    void operator() (CompoundConstraint* c) {
        c->generateVariables(vars);
    }
    vpsc::Variables& vars;
};
struct GenerateSeparationConstraints {
    GenerateSeparationConstraints(vpsc::Variables& vars, vpsc::Constraints& cs) : vars(vars), cs(cs) {}
    void operator() (CompoundConstraint* c) {
        c->generateSeparationConstraints(vars,cs);
    }
    vpsc::Variables& vars;
    vpsc::Constraints& cs;
};
void generateVariablesAndConstriants(CompoundConstraints& ccs, vpsc::Variables& vars, vpsc::Constraints& cs) {
	for_each(ccs.begin(),ccs.end(),GenerateVariables(vars));
	for_each(ccs.begin(),ccs.end(),GenerateSeparationConstraints(vars,cs));
}
void BoundaryConstraint::
generateSeparationConstraints( vpsc::Variables& vars, vpsc::Constraints& cs) 
{
    for(OffsetList::iterator o=leftOffsets.begin();
            o!=leftOffsets.end(); o++) {
        cs.push_back(
                new vpsc::Constraint(
                    vars[o->first],variable,o->second));
    }
    for(OffsetList::iterator o=rightOffsets.begin();
            o!=rightOffsets.end(); o++) {
        cs.push_back(
                new vpsc::Constraint(
                    variable,vars[o->first],o->second));
    }
}
void AlignmentConstraint::
generateVariables(vpsc::Variables& vars) {
    variable = new vpsc::Variable(vars.size(),position,0.0001);
    if(isFixed) {
        variable->fixedDesiredPosition=true;
        variable->weight=100000;
    }
    vars.push_back(variable);
}
void AlignmentConstraint::
generateSeparationConstraints(
            vpsc::Variables& vars, 
            vpsc::Constraints& cs) {
    assert(variable!=NULL);
    for(OffsetList::iterator o=offsets.begin();
            o!=offsets.end(); o++) {
        assert(vars.size()>o->first);
        cs.push_back(
                new vpsc::Constraint(
                    variable,vars[o->first],o->second,true));
    }
}
void SeparationConstraint::
generateSeparationConstraints(
            vpsc::Variables& vs, 
            vpsc::Constraints& cs) {
    if(al) {
        left=al->variable->id;
    }
    if(ar) {
        right=ar->variable->id;
    }
    vpscConstraint = new vpsc::Constraint(vs[left],vs[right],gap,equality);
    cs.push_back(vpscConstraint);
}
void SeparationConstraint::
setSeparation(double gap) {
    this->gap = gap;
    if(vpscConstraint!=NULL) {
        vpscConstraint->gap = gap;
    }
}
void OrthogonalEdgeConstraint::
generateSeparationConstraints( vpsc::Variables& vs, vpsc::Constraints& cs) {
    vpscConstraint = new vpsc::Constraint(vs[left],vs[right],0,true);
    cs.push_back(vpscConstraint);
}
void OrthogonalEdgeConstraint::
generateTopologyConstraints(const Dim k, const vpsc::Rectangles& rs, 
        vector<vpsc::Variable*> const & vars, vector<vpsc::Constraint*> & cs) {
    double lBound, rBound, pos;
    if(k==HORIZONTAL) {
        lBound = rs[left]->getCentreY();
        rBound = rs[right]->getCentreY();
        pos = rs[left]->getCentreX();
    } else {
        lBound = rs[left]->getCentreX();
        rBound = rs[right]->getCentreX();
        pos = rs[left]->getCentreY();
    }
    double minBound = std::min(lBound,rBound);
    double maxBound = std::max(lBound,rBound);
    for(unsigned i=0;i<rs.size();i++) {
        if(i==left || i==right) continue;
        vpsc::Rectangle* r=rs[i];
        if(r->allowOverlap()) continue;
        double l, rMin, rMax, rCentre;
        rectBounds(k,r,rMin,rMax,rCentre,l);
        if(rMin >= minBound && rMin <= maxBound || rMax >= minBound && rMax <= maxBound) {
            double g=l/2;
            if(rCentre < pos) {
                cs.push_back(new vpsc::Constraint(vars[i], vars[left], g));
            } else {
                cs.push_back(new vpsc::Constraint(vars[left], vars[i], g));
            }
        }
    }
}
void OrthogonalEdgeConstraint::
rectBounds(const Dim k, vpsc::Rectangle const *r, 
    double & cmin, double & cmax, double & centre, double & l
) const {
    if(k==HORIZONTAL) {
        cmin = r->getMinY();
        cmax = r->getMaxY();
        centre = r->getCentreX();
        l = r->width();
    } else {
        cmin = r->getMinX();
        cmax = r->getMaxX();
        centre = r->getCentreY();
        l = r->height();
    }
}
void MultiSeparationConstraint::
generateSeparationConstraints( vpsc::Variables& vs, vpsc::Constraints& gcs) {
    for(vector<std::pair<
            AlignmentConstraint*,AlignmentConstraint*> >::iterator iac
            =acs.begin(); iac!=acs.end();++iac) {
        AlignmentConstraint *c1, *c2;
        c1=iac->first;
        c2=iac->second;
        vpsc::Constraint* c = new vpsc::Constraint(
            c1->variable,c2->variable,sep,equality);
        gcs.push_back(c);
        cs.push_back(c);
    }
}
void DistributionConstraint::
generateSeparationConstraints( vpsc::Variables& vars, vpsc::Constraints& gcs) {
    cs.clear();
    for(vector<std::pair<
            AlignmentConstraint*,AlignmentConstraint*> >::iterator iac
            =acs.begin(); iac!=acs.end();++iac) {
        AlignmentConstraint *c1, *c2;
        c1=iac->first;
        c2=iac->second;
        assert(c1->variable!=NULL);
        vpsc::Constraint* c=new vpsc::Constraint(
                c1->variable,c2->variable,sep,true);
        gcs.push_back(c);
        cs.push_back(c);
/*
        //The following was an experiment to allow variable distributions 
        //solved by optimisation rather than satisfying constraints
        if(isVariable) {
            // set second derivatives of:
            // (u + g - v)^2 = g^2 + 2gu + u^2 - 2gv - 2uv + v^2
            (*Q)[make_pair(c1->variable->id,c1->variable->id)]+=w;
            (*Q)[make_pair(c2->variable->id,c2->variable->id)]+=w;
            (*Q)[make_pair(variable->id,variable->id)]+=w;
            (*Q)[make_pair(c1->variable->id,c2->variable->id)]-=w;
            (*Q)[make_pair(c2->variable->id,c1->variable->id)]-=w;
            (*Q)[make_pair(c1->variable->id,variable->id)]+=w;
            (*Q)[make_pair(variable->id,c1->variable->id)]+=w;
            (*Q)[make_pair(c2->variable->id,variable->id)]-=w;
            (*Q)[make_pair(variable->id,c2->variable->id)]-=w;
        }
*/
    }
}
void PageBoundaryConstraints::
generateVariables(vpsc::Variables& vars) { 
    // create 2 dummy vars, based on the dimension we are in
    if(leftWeight) {
        vars.push_back(vl=new vpsc::Variable(vars.size(), leftMargin, leftWeight));
        vl->fixedDesiredPosition=true;
    } // end if
    if(rightWeight) {
        vars.push_back(vr=new vpsc::Variable(vars.size(), rightMargin, rightWeight));
        vr->fixedDesiredPosition=true;
    } // end if
}
void PageBoundaryConstraints::
generateSeparationConstraints(
        vpsc::Variables& vs, 
        vpsc::Constraints& cs) {

    // for each of the "real" variables, create a constraint that puts that var
    // between our two new dummy vars, depending on the dimension.
    for(OffsetList::iterator o=offsets.begin(); o!=offsets.end(); ++o)  {
        if(vl)
            cs.push_back(new vpsc::Constraint(vl, vs[o->first], o->second));
        if(vr)
            cs.push_back(new vpsc::Constraint(vs[o->first], vr, o->second));
    }
}
UnsatisfiableConstraintInfo::UnsatisfiableConstraintInfo(const vpsc::Constraint* c) 
: vlid(c->left->id), vrid(c->right->id), gap(c->gap) { }
} // namespace cola
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :
