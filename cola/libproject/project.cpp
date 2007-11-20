#include <map>
#include <algorithm>
#include "util.h"
#include "project.h"
#include "feasible_projection_algorithm.h"

using namespace std;

namespace project {

typedef map<Variable const*,algorithm::Variable*> VMap;
typedef map<Constraint const*,algorithm::Constraint*> CMap;

struct CreateAlgVar {
    CreateAlgVar(VMap &m) : m(m) {}
    algorithm::Variable* operator()(Variable const *v) {
        return m[v]=new algorithm::Variable(*v);
    }
    VMap &m;
};
struct CreateAlgCons {
    CreateAlgCons(VMap &vm, CMap &cm) : vm(vm), cm(cm) {}
    algorithm::Constraint* operator()(Constraint const *c) {
        return cm[c]=new algorithm::Constraint(vm[c->l],vm[c->r],c->g);
    }
    VMap &vm;
    CMap &cm;
};
FeasibleProjection::FeasibleProjection(
        std::vector<Variable*> const &vs, 
        std::vector<Constraint*> const &cs) 
    : vs(vs), cs(cs), avs(vs.size()), acs(cs.size())
{
    transform(vs.begin(),vs.end(),avs.begin(),CreateAlgVar(vMap));
    transform(cs.begin(),cs.end(),acs.begin(),CreateAlgCons(vMap,cMap));
    alg=new algorithm::FeasibleProjectionAlgorithm(avs,acs);
}
FeasibleProjection::~FeasibleProjection() {
    for_each(avs.begin(),avs.end(),delete_object());
    for_each(acs.begin(),acs.end(),delete_object());
    delete alg;
}

struct CopyResult {
    CopyResult(VMap &m) : m(m) {}
    void operator()(Variable *v) {
        v->x = m[v]->x;
    }
    VMap &m;
};
bool FeasibleProjection::solve() {
    bool result = alg->project();
    //for_each(vs.begin(),vs.end(),CopyResult(vMap));
    Variables::const_iterator i=vs.begin();
    algorithm::Variables::iterator j=avs.begin();
    for(;i!=vs.end();i++,j++) {
        (*i)->x = (*j)->x;
    }
    return result;
}

} // namespace vpsc
/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
