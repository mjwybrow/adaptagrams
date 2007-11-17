#include <map>
#include <algorithm>
#include "util.h"
#include "project.h"
#include "feasible_projection_algorithm.h"

using namespace std;

namespace project {

typedef vector<Variable*>::iterator Vit;
typedef vector<Constraint*>::iterator Cit;

FeasibleProjection::FeasibleProjection(
        std::vector<Variable*> const &vs, 
        std::vector<Constraint*> const &cs) 
    : vs(vs), cs(cs) 
{
    for(Variables::const_iterator i=vs.begin(); i!=vs.end(); i++) {
        Variable *v=*i;
        avs.push_back(vMap[v]=new algorithm::Variable(*v));
    }
    for(Constraints::const_iterator i=cs.begin(); i!=cs.end(); i++) {
        Constraint *c=*i;
        acs.push_back(cMap[c]=new algorithm::Constraint(vMap[c->l],vMap[c->r],c->g));
    }
    alg=new algorithm::FeasibleProjectionAlgorithm(avs,acs);
}
FeasibleProjection::~FeasibleProjection() {
    for_each(avs.begin(),avs.end(),delete_object());
    for_each(acs.begin(),acs.end(),delete_object());
    delete alg;
}

bool FeasibleProjection::solve() {
    /*
    initBlocks();
    bool optimal;
    do {
        optimal=splitBlocks();
    } while(!optimal);
    */
    return false;
}

} // namespace vpsc
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
