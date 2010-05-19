/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libtopology - Classes used in generating and managing topology constraints.
 *
 * Copyright (C) 2007-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
*/

/**
 * Method body and support routines for topology::TopologyConstraints::computeForces.
 *
 * \file compute_forces.cpp
 * \author Tim Dwyer
 * \date Dec 2007
 */
#include <valarray>

#include "libvpsc/assertions.h"
#include "libcola/sparse_matrix.h"
#include "topology_constraints.h"
#include "topology_log.h"
using std::valarray;

namespace topology {
/**
 * @param u an EdgePoint
 * @param v another EdgePoint
 * @param dx x distance between u and v
 * @param dy y distance between u and v
 * @param dx2 squared x distance between u and v
 * @param dy2 squared y distance between u and v
 * @return euclidean distance between a pair of EdgePoint (u,v)
 */
static double len(const EdgePoint* u, const EdgePoint* v,
        double& dx, double& dy,
        double& dx2, double& dy2) {
    dx=u->posX()-v->posX();
    dy=u->posY()-v->posY();
    dx2=dx*dx;
    dy2=dy*dy;
    return sqrt(dx2+dy2);
}
static double gRule1(vpsc::Dim dim, const EdgePoint* a, const EdgePoint* b) {
    double dxab, dyab, dxab2, dyab2;
    double lab=dim==vpsc::HORIZONTAL?
        len(a,b,dxab,dyab,dxab2,dyab2):
        len(a,b,dyab,dxab,dyab2,dxab2);
    COLA_ASSERT(lab!=0);
    return dxab/lab;
}
static double gRule2(vpsc::Dim dim, const EdgePoint* a, const EdgePoint* b,
        const EdgePoint* c)
{
    double dxab, dyab, dxab2, dyab2;
    double lab=dim==vpsc::HORIZONTAL?
        len(a,b,dxab,dyab,dxab2,dyab2):
        len(a,b,dyab,dxab,dyab2,dxab2);
    COLA_ASSERT(lab!=0);
    double dxbc, dybc, dxbc2, dybc2;
    double lbc=dim==vpsc::HORIZONTAL?
        len(b,c,dxbc,dybc,dxbc2,dybc2):
        len(b,c,dybc,dxbc,dybc2,dxbc2);
    COLA_ASSERT(lbc!=0);
    return dxab/lab - dxbc/lbc;
}
static double hRuleD1(vpsc::Dim dim, const EdgePoint* u, const EdgePoint* v,
          const double dl)
{
    double dx, dy, dx2, dy2;
    double l=dim==vpsc::HORIZONTAL?
        len(u,v,dx,dy,dx2,dy2):
        len(u,v,dy,dx,dy2,dx2);
    COLA_ASSERT(l!=0);
    return dl*(dx2/(l*l*l) - 1/l) + dx2/(l*l);
}
static double hRuleD2(vpsc::Dim dim, const EdgePoint* u, const EdgePoint* v,
          const EdgePoint* w, const double dl)
{
    double dxuv, dyuv, dxuv2, dyuv2;
    double luv=dim==vpsc::HORIZONTAL?
        len(u,v,dxuv,dyuv,dxuv2,dyuv2):
        len(u,v,dyuv,dxuv,dyuv2,dxuv2);
    COLA_ASSERT(luv!=0);
    double dxvw, dyvw, dxvw2, dyvw2;
    double lvw=dim==vpsc::HORIZONTAL?
        len(v,w,dxvw,dyvw,dxvw2,dyvw2):
        len(v,w,dyvw,dxvw,dyvw2,dxvw2);
    COLA_ASSERT(lvw!=0);
    double p1=dl*(dxuv2/(luv*luv*luv) - 1/luv + dxvw2/(lvw*lvw*lvw) - 1/lvw);
    double p2=(dxuv/luv - dxvw/lvw);
    return p1+p2*p2;
}
static double hRule2(vpsc::Dim dim, const EdgePoint* u, const EdgePoint* v,
         const EdgePoint* w, const double dl)
{
    double dxuv, dyuv, dxuv2, dyuv2;
    double luv=dim==vpsc::HORIZONTAL?
        len(u,v,dxuv,dyuv,dxuv2,dyuv2):
        len(u,v,dyuv,dxuv,dyuv2,dxuv2);
    COLA_ASSERT(luv!=0);
    double dxvw, dyvw, dxvw2, dyvw2;
    double lvw=dim==vpsc::HORIZONTAL?
        len(v,w,dxvw,dyvw,dxvw2,dyvw2):
        len(v,w,dyvw,dxvw,dyvw2,dxvw2);
    COLA_ASSERT(lvw!=0);
    return -dl*dxuv2/(luv*luv*luv)
        +dl/luv
        -dxuv2/(luv*luv)
        +dxuv*dxvw/(luv*lvw);
}
static double hRule3(vpsc::Dim dim, const EdgePoint* u, const EdgePoint* v,
        const EdgePoint* w, const double dl)
{
    double dxuv, dyuv, dxuv2, dyuv2;
    double luv=dim==vpsc::HORIZONTAL?
        len(u,v,dxuv,dyuv,dxuv2,dyuv2):
        len(u,v,dyuv,dxuv,dyuv2,dxuv2);
    COLA_ASSERT(luv!=0);
    double dxvw, dyvw, dxvw2, dyvw2;
    double lvw=dim==vpsc::HORIZONTAL?
        len(v,w,dxvw,dyvw,dxvw2,dyvw2):
        len(v,w,dyvw,dxvw,dyvw2,dxvw2);
    COLA_ASSERT(lvw!=0);
    return -dl*dxvw2/(lvw*lvw*lvw)
        -dxvw2/(lvw*lvw)
        +dl/lvw
        +dxuv*dxvw/(luv*lvw);
}
static double hRule4(vpsc::Dim dim, const EdgePoint* a, const EdgePoint* b,
        const EdgePoint* c, const EdgePoint* d)
{
    double dxab, dyab, dxab2, dyab2;
    double lab=dim==vpsc::HORIZONTAL?
        len(a,b,dxab,dyab,dxab2,dyab2):
        len(a,b,dyab,dxab,dyab2,dxab2);
    COLA_ASSERT(lab!=0);
    double dxcd, dycd, dxcd2, dycd2;
    double lcd=dim==vpsc::HORIZONTAL?
        len(c,d,dxcd,dycd,dxcd2,dycd2):
        len(c,d,dycd,dxcd,dycd2,dxcd2);
    COLA_ASSERT(lcd!=0);
    return -dxab*dxcd/(lab*lcd);
}
static double hRule56(vpsc::Dim dim, const EdgePoint* u, const EdgePoint* v,
        const EdgePoint* a, const EdgePoint* b, const EdgePoint* c)
{
    double dxuv, dyuv, dxuv2, dyuv2;
    double luv=dim==vpsc::HORIZONTAL?
        len(u,v,dxuv,dyuv,dxuv2,dyuv2):
        len(u,v,dyuv,dxuv,dyuv2,dxuv2);
    COLA_ASSERT(luv!=0);
    double dxab, dyab, dxab2, dyab2;
    double lab=dim==vpsc::HORIZONTAL?
        len(a,b,dxab,dyab,dxab2,dyab2):
        len(a,b,dyab,dxab,dyab2,dxab2);
    COLA_ASSERT(lab!=0);
    double dxbc, dybc, dxbc2, dybc2;
    double lbc=dim==vpsc::HORIZONTAL?
        len(b,c,dxbc,dybc,dxbc2,dybc2):
        len(b,c,dybc,dxbc,dybc2,dxbc2);
    COLA_ASSERT(lbc!=0);
    return dxuv/luv * ( dxbc/lbc - dxab/lab );
}
static double hRule7(vpsc::Dim dim, const EdgePoint* a, const EdgePoint* b,
        const EdgePoint* c, const EdgePoint* d, const double dl)
{
    double dxab, dyab, dxab2, dyab2;
    double lab=dim==vpsc::HORIZONTAL?
        len(a,b,dxab,dyab,dxab2,dyab2):
        len(a,b,dyab,dxab,dyab2,dxab2);
    COLA_ASSERT(lab!=0);
    double dxbc, dybc, dxbc2, dybc2;
    double lbc=dim==vpsc::HORIZONTAL?
        len(b,c,dxbc,dybc,dxbc2,dybc2):
        len(b,c,dybc,dxbc,dybc2,dxbc2);
    COLA_ASSERT(lbc!=0);
    double dxcd, dycd, dxcd2, dycd2;
    double lcd=dim==vpsc::HORIZONTAL?
        len(c,d,dxcd,dycd,dxcd2,dycd2):
        len(c,d,dycd,dxcd,dycd2,dxcd2);
    COLA_ASSERT(lcd!=0);
    return dl*(1/lbc - dxbc2/(lbc*lbc*lbc))
        +(dxab/lab - dxbc/lbc)*(dxbc/lbc - dxcd/lcd);
}
static double hRule8(vpsc::Dim dim, const EdgePoint* u, const EdgePoint* v,
        const EdgePoint* w, const EdgePoint* a, const EdgePoint* b,
        const EdgePoint* c)
{
    double dxuv, dyuv, dxuv2, dyuv2;
    double luv=dim==vpsc::HORIZONTAL?
        len(u,v,dxuv,dyuv,dxuv2,dyuv2):
        len(u,v,dyuv,dxuv,dyuv2,dxuv2);
    COLA_ASSERT(luv!=0);
    double dxvw, dyvw, dxvw2, dyvw2;
    double lvw=dim==vpsc::HORIZONTAL?
        len(v,w,dxvw,dyvw,dxvw2,dyvw2):
        len(v,w,dyvw,dxvw,dyvw2,dxvw2);
    COLA_ASSERT(lvw!=0);
    double dxab, dyab, dxab2, dyab2;
    double lab=dim==vpsc::HORIZONTAL?
        len(a,b,dxab,dyab,dxab2,dyab2):
        len(a,b,dyab,dxab,dyab2,dxab2);
    COLA_ASSERT(lab!=0);
    double dxbc, dybc, dxbc2, dybc2;
    double lbc=dim==vpsc::HORIZONTAL?
        len(b,c,dxbc,dybc,dxbc2,dybc2):
        len(b,c,dybc,dxbc,dybc2,dxbc2);
    COLA_ASSERT(lbc!=0);
    return (dxuv/luv - dxvw/lvw) * (dxab/lab - dxbc/lbc);
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
void TopologyConstraints::computeForces(valarray<double>& gradient,
        cola::SparseMap& hessian)
{
    FILE_LOG(logDEBUG1) << "TopologyConstraints::computeForces";
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
        FILE_LOG(logDEBUG2) << "  path: n="<<n;
        COLA_ASSERT(n>=2);
        double d=e->idealLength;

        double weight=2.0/(d*d);
        double dl=d-e->pathLength();

        if(dl>=0) continue;

        // first and last entries
        // gradient
        u=path[0]; v=path[1];
        COLA_ASSERT(v->inSegment->length()>0);
        double h=weight*hRuleD1(dim,u,v,dl);
        H(u,u)+=h;
        double g1=weight*dl*gRule1(dim,u,v);
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
        g[v]+=weight*dl*gRule1(dim,u,v);
        H(v,v)+=weight*hRuleD1(dim,u,v,dl);
        // remaining diagonal entries
        for(unsigned j=1;j<n-1;j++) {
            u=path[j-1], v=path[j], w=path[j+1];
            COLA_ASSERT(v->inSegment->length()>0);
            COLA_ASSERT(w->inSegment->length()>0);
            H(v,v)+=weight*hRuleD2(dim,u,v,w,dl);
            g[v]+=weight*dl*gRule2(dim,u,v,w);
        }

        // off diagonal entries
        // hRule 2
        u=path[0], v=path[1], w=path[2];
        h=weight*hRule2(dim,u,v,w,dl);
        H(u,v)+=h;
        H(v,u)+=h;
        // hRule 3
        u=path[n-3], v=path[n-2], w=path[n-1];
        h=weight*hRule3(dim,u,v,w,dl);
        H(v,w)+=h;
        H(w,v)+=h;
        // hRule 4
        u=path[0], v=path[n-1];
        h=weight*hRule4(dim,u,path[1],path[n-2],v);
        H(u,v)+=h;
        H(v,u)+=h;
        if(n==3) continue;
        for(unsigned j=2;j<n-1;j++) {
            // hRule 5
            u=path[0],v=path[j];
            h=weight*hRule56(dim,u,path[1],path[j-1],v,path[j+1]);
            H(u,v)+=h;
            H(v,u)+=h;
            // hRule 6
            u=path[n-1], v=path[n-1-j];
            h=weight*hRule56(dim,u,path[n-2],path[n-1-j-1],v,path[n-1-j+1]);
            H(u,v)+=h;
            H(v,u)+=h;
            // hRule 7
            u=path[j-1], v=path[j];
            h=weight*hRule7(dim,path[j-2],u,v,path[j+1],dl);
            H(u,v)+=h;
            H(v,u)+=h;
        } 
        for(unsigned j=1;j<n-3;j++) {
            for(unsigned k=j+2;k<n-1;k++) {
                u=path[j]; v=path[k];
                h=weight*hRule8(dim,path[j-1],u,path[j+1],path[k-1],v,path[k+1]);
                H(u,v)+=h;
                H(v,u)+=h;
            }
        }
    }
    for(unsigned i=0;i<gradient.size();i++) {
        COLA_ASSERT(gradient[i]==gradient[i]);
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
} // namespace topology
