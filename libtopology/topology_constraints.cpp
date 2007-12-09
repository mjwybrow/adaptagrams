#include <libproject/project.h>
#include <libcola/cola.h>
#include <libcola/straightener.h>
#include "topology_graph.h"
#include "topology_constraints.h"
using namespace std;
namespace topology {
    /** 
     * @return the maximum move we can make along the line from initial to
     * desired positions without violating this constraint
     */
    double TriConstraint::maxSafeAlpha() const {
        double u1=u->relativeInitialPos();
        double u2=u->relativeDesiredPos();
        double v1=v->relativeInitialPos();
        double v2=v->relativeDesiredPos();
        double w1=w->relativeInitialPos();
        double w2=w->relativeDesiredPos();
        double numerator=w1 - g - u1 + p*(u1-v1);
        // There are a number of situations where the following can
        // be 0!
        double denominator=u2-u1 + p*(u1-u2 + v2-v1) + w1-w2;
        //assert(denominator!=0);
        if(denominator==0) {
            return 1;
        }
        return numerator/denominator;
    }
    double TriConstraint::slack () const {
        double rhs = u->x+p*(v->x-u->x)+g;
        double lhs = w->x;
        return leftOf ? rhs - lhs : lhs - rhs;
    }
    void TriConstraint::print() const {
        printf("TopologyConstraint@%p\n",this);
        printf("  u=%f\n  v=%f\n  w=%f\n  p=%f\n  g=%f\n  left=%d\n",
                u->x,v->x,w->x,p,g,leftOf);
    }
    struct transferStraightConstraint {
        transferStraightConstraint(Segment* target)
            : target(target) {}
        void operator() (StraightConstraint* s) {
            target->straightConstraints.push_back(
                    new StraightConstraint(target,s->node,s->pos));
        }
        Segment* target;
    };
    /**
     * The bend has become straight, remove bend
     */
    void BendConstraint::satisfy() {
        Segment* s1 = bendPoint->inSegment,
               * s2 = bendPoint->outSegment;
        Edge* e = s1->edge;
        EdgePoint* start = s1->start,
                 * end = s2->end;
        Segment* s = new Segment(e,start,end);
        if(e->firstSegment==s1) {
            e->firstSegment=s;
        }
        if(e->lastSegment==s2) {
            e->lastSegment=s;
        }
        // transfer each StraightConstraint from s1 and s2 to newSegment.
        transferStraightConstraint transfer(s);
        for_each(s1->straightConstraints.begin(),
                 s1->straightConstraints.end(),
                 transfer);
        for_each(s2->straightConstraints.begin(),
                 s2->straightConstraints.end(),
                 transfer);
        // update each BendConstraint involving bendPoint
        if(!start->isReal()) {
            delete start->bendConstraint;
            start->bendConstraint = new BendConstraint(start);
        }
        if(!end->isReal()) {
            delete end->bendConstraint;
            end->bendConstraint = new BendConstraint(end);
        }
        // create a new StraightConstraint to replace the BendConstraint
        s->straightConstraints.push_back(new StraightConstraint(
                    s, bendPoint->node, bendPoint->pos[!dim]));
                 
        e->nSegments--;
        delete bendPoint;
        delete s1;
        delete s2;
    }
    struct transferStraightConstraintChoose {
        transferStraightConstraintChoose(Segment* target1, Segment* target2,
                double pos, StraightConstraint* ignore)
            : target1(target1), target2(target2)
            , pos(pos), ignore(ignore) {}
        void operator() (StraightConstraint* c) {
            if(c!=ignore) {
                if(target1->start->pos[!dim] > target2->end->pos[!dim]
                   && pos > target1->end->pos[!dim]) {
                    target1->straightConstraints.push_back(
                            new StraightConstraint(target1,c->node,c->pos));
                } else {
                    target2->straightConstraints.push_back(
                            new StraightConstraint(target2,c->node,c->pos));
                }
            }
        }
        Segment* target1, * target2;
        double pos;
        StraightConstraint* ignore;
    };
    /**
     * Segment needs to bend
     */
    void StraightConstraint::satisfy() {
        Edge* e = segment->edge;
        EdgePoint* start = segment->start,
                 * end = segment->end,
                 * bend = new EdgePoint(node,ri);
        Segment* s1 = new Segment(e,start,bend);
        Segment* s2 = new Segment(e,bend,end);
        if(e->firstSegment==segment) {
            e->firstSegment=s1;
        }
        if(e->lastSegment==segment) {
            e->lastSegment=s2;
        }
        // transfer other StraightConstraint constraints 
        // from s to s1 or s2 depending on which side of p they are on.
        transferStraightConstraintChoose transfer(s1,s2,pos,this);
        for_each(segment->straightConstraints.begin(),
                 segment->straightConstraints.end(),
                 transfer);
        // BendConstraint constraints associated with segment->end and 
        // segment->start need to be updated
        if(!start->isReal()) {
            delete start->bendConstraint;
            start->bendConstraint = new BendConstraint(start);
        }
        if(!end->isReal()) {
            delete end->bendConstraint;
            end->bendConstraint = new BendConstraint(end);
        }
                 
        e->nSegments++;
        delete segment;
    }
    /**
     * satisfies a violated topology constraint by splitting the associated
     * edge segment into two new segments connected by the new dummy node z
     * whose ideal position is set to midway between the ends.
     */
        /*
    void TopologyConstraint::satisfy(EdgePoint* z, valarray<double> & coords,
            set<vpsc::Constraint*> & lcs) {
        //printf("  TopologyConstraint::satisfy: \n");
        if(dim==cola::HORIZONTAL) {
            coords[z->id]=z->pos[0]=node->pos[0]+(segmentLeft?-g:g);
            z->pos[1]=pos;
        } else {
            z->pos[0]=pos;
            coords[z->id]=z->pos[1]=node->pos[1]+(segmentLeft?-g:g);
        }
        z->var->desiredPosition=idealPoint();
        //printf("   splitting segment: ");
        segment->print();
        Edge* e=segment->edge;
        Segment* s1 = new Segment(e,segment->start,z);
        Segment* s2 = new Segment(e,z,segment->end);
        e->splitSegment(segment,s1,s2);
        //printf("    new segments:\n");
        //printf("      "); s1->print();
        //printf("      "); s2->print();
        double s1Span = s1->start->pos[!dim] - s1->end->pos[!dim];
        double s2Span = s2->start->pos[!dim] - s2->end->pos[!dim];
        //    for each t in segment->topologyConstraints\this:
        //      t->segment = s1 or s2 based on t->pos
        for(vector<TopologyConstraint*>::iterator i=segment->topologyConstraints.begin();
                i!=segment->topologyConstraints.end(); i++) {
            TopologyConstraint* t=*i;
            if(t==this) continue;
            if(s1->intersects(t->pos) && s1Span!=0) {
                s1->topologyConstraints.push_back(t);
                t->segment=s1;
                //printf("Propagating to segment 1:\n");
                t->print();
            } else if(s2->intersects(t->pos) && s2Span!=0) {
                assert(s2->intersects(t->pos));
                s2->topologyConstraints.push_back(t);
                t->segment=s2;
                //printf("Propagating to segment 2:\n");
                t->print();
            }
        }
        vpsc::Constraint* c=segmentLeft
            ? new vpsc::Constraint(z->var,node->var,g)
            : new vpsc::Constraint(node->var,z->var,g);
        lcs.insert(c);
        z->cs.insert(c);
        node->cs.insert(c);
        //printf("    creating separation constraint: v[%d]+%f<=v[%d]:\n",c->left->id,g,c->right->id);
        //cout << *c << endl;
        delete segment;
    }
        */
    /*
     * merge segments to either side of dummy nodes with no or inactive constraints
    void TopologyConstraints::
    tightenSegments(set<vpsc::Constraint*> &lcs) {
        // dummy vars with inactive (or no) separation constraints
        vector<topology::EdgePoint*> inactive;
        getInactiveEdgePoints(inactive);
        // for each v in inactive: 
        //   replace segments (s1,v),(v,s2) 
        //     with new segment s in s1->edge->segments
        //   remove associated separation constraint c over v,w from lcs
        //     and create new topology constraint over w,s1->start,s2->end 
        //     propagate all topology constraints in s1 and s2 up to s
        //   (note: we do not delete variables but such inactive variables 
        //     will be unconstrained and hence not affect the VPSC result)
        for(vector<EdgePoint*>::iterator i=inactive.begin(); 
                i!=inactive.end(); i++) {
            EdgePoint* v = *i;
            // v is doomed!
            assert(v->r==NULL);
            Edge* e = v->inSegment->edge;
            Segment *s1=v->inSegment, *s2=v->outSegment;
            assert(s1 != NULL && s2 != NULL);
            double p1=s1->start->pos[!dim], 
                   p2=v->pos[!dim], 
                   p3=s2->end->pos[!dim];
            if(!(p2>=min(p1,p3) && p2<=max(p1,p3))) {
                // merging away this dummy node would decrease the span of
                // the edge in the scan axis... and hence potentially leave
                // topology constraints dangling or conflicting!
                continue;
            }
            Segment* s = new Segment(e,s1->start,s2->end);
            e->mergeSegment(s1,s2,s);
            printf("--------------------------------------------------------------\n");
            printf("mergeSegments: (%d,%d),(%d,%d)=>(%d,%d)\n",
                    s1->start->id,s1->end->id,s2->start->id,s2->end->id,
                    s->start->id,s->end->id);
            printf("s1: "); s1->print();
            printf("s2: "); s2->print();
            printf("s: "); s->print();
            vector<TopologyConstraint*> 
                &t1=s1->topologyConstraints, 
                &t2=s2->topologyConstraints, 
                &t=s->topologyConstraints;
            for(unsigned j=0;j<t1.size()+t2.size();j++) {
                TopologyConstraint* c;
                if(j<t1.size()) {
                    c=t1[j];
                } else {
                    c=t2[j-t1.size()];
                }
                if(s->intersects(c->pos)) {
                    c->segment=s;
                    c->print();
                    t.push_back(c);
                }
            }
            for(set<vpsc::Constraint*>::iterator j=v->cs.begin();j!=v->cs.end(); j++) {
                vpsc::Constraint* c=*j;
                printf("deleting separation constraint: v[%d]+%f<=v[%d]:\n",c->left->id,c->gap,c->right->id);
                lcs.erase(c);
                EdgePoint* w;
                bool segmentLeft=true;
                if(v->var == c->left) {
                    w = dummyNodes[c->right->id];
                } else {
                    assert(v->var == c->right);
                    segmentLeft=false;
                    w = dummyNodes[c->left->id];
                }
                // w is a real shape, not a dummy node on an edge
                assert(w->inSegment==NULL && w->outSegment==NULL);
                assert(w->r!=NULL);
                printf("  deleted dummy node scan pos = %f, s(%f,%f)\n",v->pos[!dim],s->start->pos[!dim],s->end->pos[!dim]);
                assert(s->intersects(v->pos[!dim]));
                t.push_back(
                    new TopologyConstraint(s,w,v->pos[!dim],c->gap,segmentLeft));
                w->cs.erase(c);
                delete c;
            }
            v->cs.clear();
            v->inSegment=v->outSegment=NULL;
            typedef map<EdgePoint*,bool> CheckSides;
            CheckSides checkSides;
            for(unsigned j=0;j<t.size();j++) {
                TopologyConstraint* c=t[j];
                CheckSides::iterator it=checkSides.find(c->node);
                if(it == checkSides.end()) {
                    checkSides[c->node]=c->segmentLeft;
                } else {
                    assert(it->second==c->segmentLeft);
                }
            }
            delete s1;
            delete s2;
        }
        printf("-------merge done---------------------------------------------\n");
    }
     */
    double TopologyConstraints::len(const EdgePoint* u, const EdgePoint* v, 
            double& dx, double& dy,
            double& dx2, double& dy2) {
        dx=u->pos[0]-v->pos[0];
        dy=u->pos[1]-v->pos[1];
        dx2=dx*dx;
        dy2=dy*dy;
        return sqrt(dx2+dy2);
    }
    double TopologyConstraints::gRule1(const EdgePoint* a, const EdgePoint* b) {
        double dxab, dyab, dxab2, dyab2;
        double lab=dim==cola::HORIZONTAL?
            len(a,b,dxab,dyab,dxab2,dyab2):
            len(a,b,dyab,dxab,dyab2,dxab2);
        return dxab/lab;
    }
    double TopologyConstraints::gRule2(
            const EdgePoint* a, const EdgePoint* b, const EdgePoint* c) {
        double dxab, dyab, dxab2, dyab2;
        double lab=dim==cola::HORIZONTAL?
            len(a,b,dxab,dyab,dxab2,dyab2):
            len(a,b,dyab,dxab,dyab2,dxab2);
        double dxbc, dybc, dxbc2, dybc2;
        double lbc=dim==cola::HORIZONTAL?
            len(b,c,dxbc,dybc,dxbc2,dybc2):
            len(b,c,dybc,dxbc,dybc2,dxbc2);
        return dxab/lab - dxbc/lbc;
    }
    double TopologyConstraints::hRuleD1(const EdgePoint* u, const EdgePoint* v, const double dl) {
        double dx, dy, dx2, dy2;
        double l=dim==cola::HORIZONTAL?
            len(u,v,dx,dy,dx2,dy2):
            len(u,v,dy,dx,dy2,dx2);
        return dl*(dx2/(l*l*l) - 1/l) + dx2/(l*l);
    }
    double TopologyConstraints::hRuleD2(
            const EdgePoint* u, const EdgePoint* v, const EdgePoint* w, const double dl) {
        double dxuv, dyuv, dxuv2, dyuv2;
        double luv=dim==cola::HORIZONTAL?
            len(u,v,dxuv,dyuv,dxuv2,dyuv2):
            len(u,v,dyuv,dxuv,dyuv2,dxuv2);
        double dxvw, dyvw, dxvw2, dyvw2;
        double lvw=dim==cola::HORIZONTAL?
            len(v,w,dxvw,dyvw,dxvw2,dyvw2):
            len(v,w,dyvw,dxvw,dyvw2,dxvw2);
        double p1=dl*(dxuv2/(luv*luv*luv) - 1/luv + dxvw2/(lvw*lvw*lvw) - 1/lvw);
        double p2=(dxuv/luv - dxvw/lvw);
        return p1+p2*p2;
    }
    double TopologyConstraints::hRule2(
            const EdgePoint* u, const EdgePoint* v, const EdgePoint* w, const double dl) {
        double dxuv, dyuv, dxuv2, dyuv2;
        double luv=dim==cola::HORIZONTAL?
            len(u,v,dxuv,dyuv,dxuv2,dyuv2):
            len(u,v,dyuv,dxuv,dyuv2,dxuv2);
        double dxvw, dyvw, dxvw2, dyvw2;
        double lvw=dim==cola::HORIZONTAL?
            len(v,w,dxvw,dyvw,dxvw2,dyvw2):
            len(v,w,dyvw,dxvw,dyvw2,dxvw2);
        return -dl*dxuv2/(luv*luv*luv)
            +dl/luv
            -dxuv2/(luv*luv)
            +dxuv*dxvw/(luv*lvw);
    }
    double TopologyConstraints::hRule3(
            const EdgePoint* u, const EdgePoint* v, const EdgePoint* w, 
            const double dl) {
        double dxuv, dyuv, dxuv2, dyuv2;
        double luv=dim==cola::HORIZONTAL?
            len(u,v,dxuv,dyuv,dxuv2,dyuv2):
            len(u,v,dyuv,dxuv,dyuv2,dxuv2);
        double dxvw, dyvw, dxvw2, dyvw2;
        double lvw=dim==cola::HORIZONTAL?
            len(v,w,dxvw,dyvw,dxvw2,dyvw2):
            len(v,w,dyvw,dxvw,dyvw2,dxvw2);
        return -dl*dxvw2/(lvw*lvw*lvw)
            -dxvw2/(lvw*lvw)
            +dl/lvw
            +dxuv*dxvw/(luv*lvw);
    }
    double TopologyConstraints::hRule4(const EdgePoint* a, const EdgePoint* b, 
            const EdgePoint* c, const EdgePoint* d) {
        double dxab, dyab, dxab2, dyab2;
        double lab=dim==cola::HORIZONTAL?
            len(a,b,dxab,dyab,dxab2,dyab2):
            len(a,b,dyab,dxab,dyab2,dxab2);
        double dxcd, dycd, dxcd2, dycd2;
        double lcd=dim==cola::HORIZONTAL?
            len(c,d,dxcd,dycd,dxcd2,dycd2):
            len(c,d,dycd,dxcd,dycd2,dxcd2);
        return -dxab*dxcd/(lab*lcd);
    }
    double TopologyConstraints::hRule56(const EdgePoint* u, const EdgePoint* v, 
            const EdgePoint* a, const EdgePoint* b, const EdgePoint* c) {
        double dxuv, dyuv, dxuv2, dyuv2;
        double luv=dim==cola::HORIZONTAL?
            len(u,v,dxuv,dyuv,dxuv2,dyuv2):
            len(u,v,dyuv,dxuv,dyuv2,dxuv2);
        double dxab, dyab, dxab2, dyab2;
        double lab=dim==cola::HORIZONTAL?
            len(a,b,dxab,dyab,dxab2,dyab2):
            len(a,b,dyab,dxab,dyab2,dxab2);
        double dxbc, dybc, dxbc2, dybc2;
        double lbc=dim==cola::HORIZONTAL?
            len(b,c,dxbc,dybc,dxbc2,dybc2):
            len(b,c,dybc,dxbc,dybc2,dxbc2);
        return dxuv/luv * ( dxbc/lbc - dxab/lab );
    }
    double TopologyConstraints::hRule7(const EdgePoint* a, const EdgePoint* b, 
            const EdgePoint* c, const EdgePoint* d, const double dl) {
        double dxab, dyab, dxab2, dyab2;
        double lab=dim==cola::HORIZONTAL?
            len(a,b,dxab,dyab,dxab2,dyab2):
            len(a,b,dyab,dxab,dyab2,dxab2);
        double dxbc, dybc, dxbc2, dybc2;
        double lbc=dim==cola::HORIZONTAL?
            len(b,c,dxbc,dybc,dxbc2,dybc2):
            len(b,c,dybc,dxbc,dybc2,dxbc2);
        double dxcd, dycd, dxcd2, dycd2;
        double lcd=dim==cola::HORIZONTAL?
            len(c,d,dxcd,dycd,dxcd2,dycd2):
            len(c,d,dycd,dxcd,dycd2,dxcd2);
        return dl*(1/lbc - dxbc2/(lbc*lbc*lbc))
            +(dxab/lab - dxbc/lbc)*(dxbc/lbc - dxcd/lcd);
    }
    double TopologyConstraints::hRule8(const EdgePoint* u, const EdgePoint* v, const EdgePoint* w,
            const EdgePoint* a, const EdgePoint* b, const EdgePoint* c) {
        double dxuv, dyuv, dxuv2, dyuv2;
        double luv=dim==cola::HORIZONTAL?
            len(u,v,dxuv,dyuv,dxuv2,dyuv2):
            len(u,v,dyuv,dxuv,dyuv2,dxuv2);
        double dxvw, dyvw, dxvw2, dyvw2;
        double lvw=dim==cola::HORIZONTAL?
            len(v,w,dxvw,dyvw,dxvw2,dyvw2):
            len(v,w,dyvw,dxvw,dyvw2,dxvw2);
        double dxab, dyab, dxab2, dyab2;
        double lab=dim==cola::HORIZONTAL?
            len(a,b,dxab,dyab,dxab2,dyab2):
            len(a,b,dyab,dxab,dyab2,dxab2);
        double dxbc, dybc, dxbc2, dybc2;
        double lbc=dim==cola::HORIZONTAL?
            len(b,c,dxbc,dybc,dxbc2,dybc2):
            len(b,c,dybc,dxbc,dybc2,dxbc2);
        return (dxuv/luv - dxvw/lvw) * (dxab/lab - dxbc/lbc);
    }
    /**
     * compute the stress:
     * \f[
     *   \sigma = \sum_{e \in E} \left( d_e - \sum_{s \in S(e)} |s| \right)^2
     * \f]
     */
    double TopologyConstraints::computeStress() const {
        double stress=0;
        for(Edges::const_iterator i=edges.begin();i!=edges.end();++i) {
            Edge* e=*i;
            double d = e->idealLength;
            double weight=1.0/(d*d);
            //printf("pathLength=%f\n",e->pathLength());
            double sqrtf=fabs(d-e->pathLength());
            stress+=weight*sqrtf*sqrtf;
        }
        return stress;
    }
    /**
     * a wrapper for a SparseMap so that we can index it by two EdgePoint
     */
    struct SparseMapMap {
        cola::SparseMap &H;
        SparseMapMap(cola::SparseMap &H) : H(H) {}
        double& operator()(const EdgePoint *i, const EdgePoint* j) {
            return H(i->node->id,j->node->id);
        }
    };
    /**
     * a wrapper for a valarray so that we can index it by an EdgePoint
     */
    template <typename T>
    struct ArrayMap {
        valarray<T>& a;
        ArrayMap(valarray<T>& a) : a(a) {}
        double& operator[](const EdgePoint *i) {
            return a[i->node->id];
        }
    };
    /**
     * Compute the forces associated with each EdgePoint (bend or end point) along each each
     * on the nodes/rectangles in the graph.
     */
    void TopologyConstraints::computeForces(cola::SparseMap &hessian, valarray<double> &gradient) {
        SparseMapMap H(hessian);
        ArrayMap<double> g(gradient);
        const EdgePoint *u,*v,*w;
        for(Edges::const_iterator i=edges.begin();i!=edges.end();i++) {
            //printf("Straightening path:\n");
            //edges[i]->print();
            Edge* e=*i;
            ConstEdgePoints path;
            e->getPath(path);
            unsigned n=path.size();
            assert(n>=2);
            double d=e->idealLength;

            double weight=2.0/(d*d);
            double dl=d-e->pathLength();

            // first and last entries
            // gradient
            u=path[0]; v=path[1];
            double h=weight*hRuleD1(u,v,dl);
            H(u,u)+=h;
            double g1=weight*dl*gRule1(u,v);
            g[u]-=g1;
            if(n==2||dl>0) {
                // rule 1
                H(v,v)+=h;
                g[v]+=g1; 
                H(u,v)-=h;
                H(v,u)-=h;
                continue;
            }
            u=path[n-2]; v=path[n-1];
            g[v]+=weight*dl*gRule1(u,v);
            H(v,v)+=weight*hRuleD1(u,v,dl);
            // remaining diagonal entries
            for(unsigned j=1;j<n-1;j++) {
                u=path[j-1], v=path[j], w=path[j+1];
                H(v,v)+=weight*hRuleD2(u,v,w,dl);
                g[v]+=weight*dl*gRule2(u,v,w);
            }

            // off diagonal entries
            // hRule 2
            u=path[0], v=path[1], w=path[2];
            h=weight*hRule2(u,v,w,dl);
            H(u,v)+=h;
            H(v,u)+=h;
            // hRule 3
            u=path[n-3], v=path[n-2], w=path[n-1];
            h=weight*hRule3(u,v,w,dl);
            H(v,w)+=h;
            H(w,v)+=h;
            // hRule 4
            u=path[0], v=path[n-1];
            h=weight*hRule4(u,path[1],path[n-2],v);
            H(u,v)+=h;
            H(v,u)+=h;
            if(n==3) continue;
            for(unsigned j=2;j<n-1;j++) {
                // hRule 5
                u=path[0],v=path[j];
                h=weight*hRule56(u,path[1],path[j-1],v,path[j+1]);
                H(u,v)+=h;
                H(v,u)+=h;
                // hRule 6
                u=path[n-1], v=path[n-1-j];
                h=weight*hRule56(u,path[n-2],path[n-1-j-1],v,path[n-1-j+1]);
                H(u,v)+=h;
                H(v,u)+=h;
                // hRule 7
                u=path[j-1], v=path[j];
                h=weight*hRule7(path[j-2],u,v,path[j+1],dl);
                H(u,v)+=h;
                H(v,u)+=h;
            } 
            for(unsigned j=1;j<n-3;j++) {
                for(unsigned k=j+2;k<n-1;k++) {
                    u=path[j]; v=path[k];
                    h=weight*hRule8(path[j-1],u,path[j+1],path[k-1],v,path[k+1]);
                    H(u,v)+=h;
                    H(v,u)+=h;
                }
            }
        }
        /*
        for(unsigned i=0;i<edges.size();i++) {
            Edge* e=edges[i];
            vector<unsigned>& path=e->path;
            unsigned n=path.size();
            printf("d=%f;\n",e->idealLength);
            printf("X={");
            for(unsigned j=0;j<n;j++) {
                printf("%f",nodes[path[j]]->x);
                if(j<n-1) {
                    printf(",");
                }
            }
            printf("};\n");
            printf("Y={");
            for(unsigned j=0;j<n;j++) {
                printf("%f",nodes[path[j]]->y);
                if(j<n-1) {
                    printf(",");
                }
            }
            printf("};\n");
            printf("H=\n");
            for(unsigned j=0;j<n;j++) {
                for(unsigned k=0;k<n;k++) {
                    unsigned u=path[j], v=path[k];
                    printf("%f ",H(u,v));
                }
                printf("\n");
            }
            printf("g=");
            for(unsigned j=0;j<n;j++) {
                printf("%f ",g[path[j]]);
            }
            printf("\n");
        }
        */
    }

    struct buildRoute {
        buildRoute(straightener::Route* r, unsigned& n) : r(r), n(n) {}
        void operator() (const Segment* s) {
            EdgePoint* u=s->end;
            r->xs[n]=u->pos[0];
            r->ys[n++]=u->pos[1];
        }
        straightener::Route* r;
        unsigned& n;
    };
    void TopologyConstraints::finalizeRoutes() {
        //printf("Routes:\n");
        for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
            //edges[e]->print();
            Segment* s=(*e)->firstSegment;
            straightener::Route* r=new straightener::Route((*e)->nSegments+1);
            EdgePoint* u=s->start;
            unsigned n=0;
            r->xs[n]=u->pos[0];
            r->ys[n++]=u->pos[1];
            (*e)->forEachSegmentConst(buildRoute(r,n));
            r->n=n;
            //edges[e]->sEdge->setRoute(r);
        }
    }
struct getTopologyConstraints {
    getTopologyConstraints(vector<TopologyConstraint*>& ts) : ts(ts) {}
    void operator() (const Segment* s) {
        for(vector<StraightConstraint*>::const_iterator 
            t=s->straightConstraints.begin();
            t!=s->straightConstraints.end();++t) {
            ts.push_back(*t);
        }
        if(!s->end->isReal()) {
            ts.push_back(s->end->bendConstraint);
        }
    }
    vector<TopologyConstraint*>& ts;
};
void TopologyConstraints::
constraints(std::vector<TopologyConstraint*> & ts) const {
    for(Edges::const_iterator e=edges.begin();e!=edges.end();++e) {
        (*e)->forEachSegmentConst(getTopologyConstraints(ts));
    }
}
} // namespace topology
// vim: cindent ts=4 sw=4 et tw=0 wm=0
