#include "topology_constraints.h"
#include "cola.h"
#include "straightener.h"
using namespace std;
using vpsc::Rectangle;
namespace topology {
    double pathLength(Edge const * e,vector<DummyNode*> const &dummyNodes) {
        double length=0;
        vector<unsigned> path;
        e->getPath(path);
        for(unsigned i=1;i<path.size();i++) {
            DummyNode *u=dummyNodes[path[i-1]], *v=dummyNodes[path[i]];
            double dx=u->pos[0]-v->pos[0];
            double dy=u->pos[1]-v->pos[1];
            length+=sqrt(dx*dx+dy*dy);
        }
        return length;
    }
    void TopologyConstraints::coordsToNodePositions() {
        for (unsigned i=0;i<dummyNodes.size();i++) {
            dummyNodes[i]->pos[dim]=coords[i];
        }
    }
    void TopologyConstraints::updateNodePositionsFromVars() {
        for (unsigned i=0;i<dummyNodes.size();i++) {
            DummyNode *n=dummyNodes[i];
            n->pos[dim]=n->var->finalPosition;
            coords[i]=n->var->finalPosition;
        }
    }
    double TopologyConstraint::idealPoint(
            DummyNode const * u, DummyNode const * v,  const double pos) const {
        assert(v->pos[!dim] - u->pos[!dim] != 0);
        return u->pos[dim] + (v->pos[dim] - u->pos[dim]) * fabs((pos-u->pos[!dim])
                /(v->pos[!dim]-u->pos[!dim]));
    }
    double TopologyConstraint::idealPoint() const {
        assert(segment->intersects(pos));
        return idealPoint(segment->start,segment->end,pos);
    }
    TopologyConstraint::TopologyConstraint(Segment* segment, DummyNode* node, 
            const double pos, const double g, const bool segmentLeft) 
               : segment(segment), node(node), pos(pos), g(g), segmentLeft(segmentLeft) {
        print();
    }
    DummyNode* TopologyConstraints::addDummyNode(cola::Variables & lvs) {
        DummyNode* z = new DummyNode(N(),1,1,false);
        z->var = new vpsc::Variable(z->id,1,1);
        dummyNodes.push_back(z);
        lvs.push_back(z->var);
        valarray<double> oldCoords=coords;
        coords.resize(N());
        for(unsigned i=0;i<N()-1;i++) {
            coords[i]=oldCoords[i];
        }
        coords[N()-1]=1;
        return z;
    }
    /*
     * satisfies a violated topology constraint by splitting the associated
     * edge segment into two new segments connected by the new dummy node z
     * whose ideal position is set to midway between the ends.
     */
    void TopologyConstraint::satisfy(DummyNode* z, valarray<double> & coords,
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
    void TopologyConstraints::violated(vector<TopologyConstraint*> & ts) const {
        for(vector<Edge*>::const_iterator i=edges.begin();i!=edges.end();i++) {
            list<Segment*> segments=(*i)->segments;
            for(list<Segment*>::const_iterator j=segments.begin();j!=segments.end();j++) {
                vector<TopologyConstraint*> sts=(*j)->topologyConstraints;
                for(vector<TopologyConstraint*>::const_iterator k=sts.begin();k!=sts.end();k++) {
                    TopologyConstraint* t=*k;
                    if(t->violated()) {
                        ts.push_back(t);
                    }
                }
            }
        }
    }
    /**
     * check the constraints associated with each dummy node
     * If no active constraints are found then add the node to inactive list
     */
    void TopologyConstraints::
    getInactiveDummyNodes(std::vector<DummyNode*> &inactive) const {
        for(vector<DummyNode*>::const_iterator i=dummyNodes.begin(); 
                i!=dummyNodes.end(); i++) {
            DummyNode* v = *i;
            if(v->r!=NULL || v->inSegment == NULL || v->outSegment == NULL) {
                // either deleted already or
                // associated with real shape, i.e. not just a bend point
                continue;
            }
            for(set<vpsc::Constraint*>::iterator j=v->cs.begin();j!=v->cs.end(); j++) {
                vpsc::Constraint* c=*j;
                if(c->active) {
                    goto active_found;
                }
            }
            inactive.push_back(v);
active_found:;
        }
    }
    /*
     * merge segments to either side of dummy nodes with no or inactive constraints
     */
    void TopologyConstraints::
    tightenSegments(set<vpsc::Constraint*> &lcs) {
        // dummy vars with inactive (or no) separation constraints
        vector<topology::DummyNode*> inactive;
        getInactiveDummyNodes(inactive);
        // for each v in inactive: 
        //   replace segments (s1,v),(v,s2) 
        //     with new segment s in s1->edge->segments
        //   remove associated separation constraint c over v,w from lcs
        //     and create new topology constraint over w,s1->start,s2->end 
        //     propagate all topology constraints in s1 and s2 up to s
        //   (note: we do not delete variables but such inactive variables 
        //     will be unconstrained and hence not affect the VPSC result)
        for(vector<DummyNode*>::iterator i=inactive.begin(); 
                i!=inactive.end(); i++) {
            DummyNode* v = *i;
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
                DummyNode* w;
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
            typedef map<DummyNode*,bool> CheckSides;
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
    TopologyConstraint* TopologyConstraints::mostViolated() const {
        vector<TopologyConstraint*> ts;
        violated(ts);
        double minSlack = -1e-4;
        TopologyConstraint* v = NULL;
        for(unsigned i=0;i<ts.size();i++) {
            double slack = ts[i]->slack();
            if(slack<minSlack) {
                v = ts[i];
                minSlack = slack;
            }
        }
        return v;
    }
    double TopologyConstraints::len(const unsigned u, const unsigned v, 
            double& dx, double& dy,
            double& dx2, double& dy2) {
        dx=dummyNodes[u]->pos[0]-dummyNodes[v]->pos[0];
        dy=dummyNodes[u]->pos[1]-dummyNodes[v]->pos[1];
        dx2=dx*dx;
        dy2=dy*dy;
        return sqrt(dx2+dy2);
    }
    double TopologyConstraints::gRule1(const unsigned a, const unsigned b) {
        double dxab, dyab, dxab2, dyab2;
        double lab=dim==cola::HORIZONTAL?
            len(a,b,dxab,dyab,dxab2,dyab2):
            len(a,b,dyab,dxab,dyab2,dxab2);
        return dxab/lab;
    }
    double TopologyConstraints::gRule2(const unsigned a, const unsigned b, const unsigned c) {
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
    double TopologyConstraints::hRuleD1(const unsigned u, const unsigned v, const double dl) {
        double dx, dy, dx2, dy2;
        double l=dim==cola::HORIZONTAL?
            len(u,v,dx,dy,dx2,dy2):
            len(u,v,dy,dx,dy2,dx2);
        return dl*(dx2/(l*l*l) - 1/l) + dx2/(l*l);
    }
    double TopologyConstraints::hRuleD2(const unsigned u, const unsigned v, const unsigned w, 
            const double dl) {
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
    double TopologyConstraints::hRule2(const unsigned u, const unsigned v, const unsigned w, 
            const double dl) {
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
    double TopologyConstraints::hRule3(const unsigned u, const unsigned v, const unsigned w, 
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
    double TopologyConstraints::hRule4(const unsigned a, const unsigned b, 
            const unsigned c, const unsigned d) {
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
    double TopologyConstraints::hRule56(const unsigned u, const unsigned v, 
            const unsigned a, const unsigned b, const unsigned c) {
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
    double TopologyConstraints::hRule7(const unsigned a, const unsigned b, 
            const unsigned c, const unsigned d, const double dl) {
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
    double TopologyConstraints::hRule8(const unsigned u, const unsigned v, const unsigned w,
            const unsigned a, const unsigned b, const unsigned c) {
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
    void Edge::getPath(vector<unsigned> & path) const {
        path.push_back(segments.front()->start->id);
        for(list<Segment*>::const_iterator i=segments.begin();i!=segments.end();i++) {
            path.push_back((*i)->end->id);
        }
    }
    double TopologyConstraints::computeStress() const {
        double stress=0;
        for(unsigned i=0;i<edges.size();i++) {
            double d = edges[i]->sEdge->idealLength;
            double weight=1/(d*d);
            //printf("pathLength=%f\n",pathLength(edges[i],nodes));
            double sqrtf=fabs(d-pathLength(edges[i],dummyNodes));
            stress+=weight*sqrtf*sqrtf;
        }
        return stress;
    }
    void TopologyConstraints::computeForces(cola::SparseMap &H) {
        // hessian matrix:
        unsigned u,v,w;
        for(unsigned i=0;i<edges.size();i++) {
            //printf("Straightening path:\n");
            //edges[i]->print();
            Edge* e=edges[i];
            vector<unsigned> path;
            e->getPath(path);
            unsigned n=path.size();
            assert(n>=2);
            double d=e->sEdge->idealLength;
            printf("idealLength=%f\n",d);
            double weight=1/(d*d);
            double dl=d-pathLength(e,dummyNodes);

            // first and last entries
            // gradient
            u=path[0], v=path[1];
            double h=weight*hRuleD1(u,v,dl);
            if(!fixed.check(u)) H(u,u)+=h;
            double g1=weight*dl*gRule1(u,v);
            if(!fixed.check(u)) { g[u]-=g1; }
            if(n==2||dl>0) {
                // rule 1
                if(!fixed.check(v)) {
                    H(v,v)+=h;
                    g[v]+=g1; 
                }
                if(!fixed.check(u)&&!fixed.check(v)) {
                    H(u,v)-=h;
                    H(v,u)-=h;
                }
                continue;
            }
            u=path[n-2]; v=path[n-1];
            if(!fixed.check(v)) {
                g[v]+=weight*dl*gRule1(u,v);
            }
            if(!fixed.check(v)) H(v,v)+=weight*hRuleD1(u,v,dl);
            // remaining diagonal entries
            for(unsigned j=1;j<n-1;j++) {
                u=path[j-1], v=path[j], w=path[j+1];
                if(!fixed.check(v)) H(v,v)+=weight*hRuleD2(u,v,w,dl);
                if(!fixed.check(v)) { 
                    g[v]+=weight*dl*gRule2(u,v,w);
                }
            }

            // off diagonal entries
            // hRule 2
            u=path[0], v=path[1], w=path[2];
            h=weight*hRule2(u,v,w,dl);
            if(!fixed.check(u)) H(u,v)+=h;
            if(!fixed.check(v)) H(v,u)+=h;
            // hRule 3
            u=path[n-3], v=path[n-2], w=path[n-1];
            h=weight*hRule3(u,v,w,dl);
            if(!fixed.check(v)) H(v,w)+=h;
            if(!fixed.check(w)) H(w,v)+=h;
            // hRule 4
            u=path[0], v=path[n-1];
            h=weight*hRule4(u,path[1],path[n-2],v);
            if(!fixed.check(u)) H(u,v)+=h;
            if(!fixed.check(v)) H(v,u)+=h;
            if(n==3) continue;
            for(unsigned j=2;j<n-1;j++) {
                // hRule 5
                u=path[0],v=path[j];
                h=weight*hRule56(u,path[1],path[j-1],v,path[j+1]);
                if(!fixed.check(u)) H(u,v)+=h;
                if(!fixed.check(v)) H(v,u)+=h;
                // hRule 6
                u=path[n-1], v=path[n-1-j];
                h=weight*hRule56(u,path[n-2],path[n-1-j-1],v,path[n-1-j+1]);
                if(!fixed.check(u)) H(u,v)+=h;
                if(!fixed.check(v)) H(v,u)+=h;
                // hRule 7
                u=path[j-1], v=path[j];
                h=weight*hRule7(path[j-2],u,v,path[j+1],dl);
                if(!fixed.check(u)) H(u,v)+=h;
                if(!fixed.check(v)) H(v,u)+=h;
            } 
            for(unsigned j=1;j<n-3;j++) {
                for(unsigned k=j+2;k<n-1;k++) {
                    u=path[j]; v=path[k];
                    h=weight*hRule8(path[j-1],u,path[j+1],path[k-1],v,path[k+1]);
                    if(!fixed.check(u)) H(u,v)+=h;
                    if(!fixed.check(v)) H(v,u)+=h;
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
    void TopologyConstraints::verify(vector<vpsc::Rectangle*> &rs) {
        for(unsigned e=0;e<edges.size();e++) {
            Edge* edge=edges[e];
            list<Segment*> &segments=edge->segments;
            for(list<Segment*>::const_iterator s=segments.begin();
                    s!=segments.end();s++) {
                DummyNode *u=(*s)->start, *v=(*s)->end;
                for(unsigned r=0;r<rs.size();r++) {
                    if(edge->sEdge->startNode==r) continue;
                    if(edge->sEdge->endNode==r) continue;

                    assert(!rs[r]->overlaps(
                            u->pos[0],u->pos[1],v->pos[0],v->pos[1]));
                }
            }
        }
    }

    void TopologyConstraints::finalizeRoutes() {
        //printf("Routes:\n");
        for(unsigned e=0;e<edges.size();e++) {
            edges[e]->print();
            list<Segment*> &segments=edges[e]->segments;
            straightener::Route* r=new straightener::Route(segments.size()+1);
            DummyNode* u=segments.front()->start;
            unsigned rcnt=0;
            r->xs[rcnt]=u->pos[0];
            r->ys[rcnt++]=u->pos[1];
            for(list<Segment*>::const_iterator s=segments.begin();
                    s!=segments.end();s++) {
                u=(*s)->end;
                assert(rcnt<=segments.size());
                r->xs[rcnt]=u->pos[0];
                r->ys[rcnt++]=u->pos[1];
            }
            r->n=rcnt;
            edges[e]->sEdge->setRoute(r);
        }
    }
}
// vim: cindent ts=4 sw=4 et tw=0 wm=0
