#include "commondefs.h"

#include <cmath>
#include "cola.h"
#include "shortest_paths.h"
#include "straightener.h"
#include <libvpsc/solve_VPSC.h>
#include <libvpsc/variable.h>
#include <libvpsc/constraint.h>
#include <libvpsc/rectangle.h>
#include <libproject/project.h>
#include <libtopology/topology_graph.h>
#include <libtopology/topology_constraints.h>

namespace cola {
template <class T>
void delete_vector(vector<T*> &v) {
    for_each(v.begin(),v.end(),delete_object());
    v.clear();
}
inline double dotProd(valarray<double> x, valarray<double> y) {
    assert(x.size()==y.size());
    double dp=0;
    for(unsigned i=0;i<x.size();i++) {
        dp+=x[i]*y[i]; 
    }
    return dp;
}
template <typename T>
void dumpSquareMatrix(unsigned n, T** L) {
    printf("Matrix %dX%d\n{",n,n);
    for(unsigned i=0;i<n;i++) {
        printf("{");
        for(unsigned j=0;j<n;j++) {
            std::cout<<L[i][j];
            char c=j==n-1?'}':',';
            printf("%c",c);
        }
        char c=i==n-1?'}':',';
        printf("%c\n",c);
    }
}
ConstrainedFDLayout::ConstrainedFDLayout(
        vector<Rectangle*>& rs,
        vector<Edge> const & es,
        RootCluster* clusterHierarchy,
        double const idealLength,
        std::valarray<double> const * eweights,
        TestConvergence& done,
        PreIteration* preIteration)
    : n(rs.size()),
      X(valarray<double>(n)),
      Y(valarray<double>(n)),
      done(done),
      preIteration(preIteration),
      constrainedX(false), constrainedY(false),
      ccsx(NULL), ccsy(NULL),
      avoidOverlaps(false),
      topologyNodes(NULL),
      topologyRoutes(NULL)
{
    boundingBoxes.resize(n);
    copy(rs.begin(),rs.end(),boundingBoxes.begin());
    done.reset();
    for(unsigned i=0;i<n;i++) {
        vpsc::Rectangle *r=rs[i];
        X[i]=r->getCentreX();
        Y[i]=r->getCentreY();
        //printf("r[%d]=(%f,%f,%f,%f)\n",i,r->getMinX(),r->getMinY(),r->width(),r->height());
    }
    D=new double*[n];
    G=new unsigned*[n];
    for(unsigned i=0;i<n;i++) {
        D[i]=new double[n];
        G[i]=new unsigned[n];
    }
    shortest_paths::johnsons(n,G,es);
    shortest_paths::johnsons(n,D,es,eweights);
    //dumpSquareMatrix<unsigned>(n,G);
    //dumpSquareMatrix<double>(n,D);
    for(unsigned i=0;i<n;i++) {
        for(unsigned j=0;j<n;j++) {
            if(i==j) continue;
            if(!eweights) {
                D[i][j]*=idealLength;
            }
        }
    }
}

void ConstrainedFDLayout::run(const bool xAxis, const bool yAxis) {
    //printf("n==%d\n",n);
    if(n==0) return;
    double stress=computeStress();
    bool firstPass=true;
    do {
        if(xAxis) {
            stress=applyForcesAndConstraints(HORIZONTAL,stress,firstPass);
        }
        if(yAxis) {
            stress=applyForcesAndConstraints(VERTICAL,stress,firstPass);
        }
        firstPass=false;
        move();
    } while(!done(stress,X,Y));
}
/**
 * The following computes an unconstrained solution then uses Projection to
 * make this solution feasible with respect to constraints by moving things as
 * little as possible.  If "meta-constraints" such as avoidOverlaps or edge
 * straightening are required then dummy variables will be generated.
 */
double ConstrainedFDLayout::applyForcesAndConstraints(const Dim dim, const double oldStress,
        const bool firstPass) {
    valarray<double> g(n);
    valarray<double> &coords = (dim==HORIZONTAL)?X:Y;
    SparseMap HMap(n);
    computeForces(dim,HMap,g);
    /*
    printf("g=[");
    for(unsigned i=0;i<n;i++) {
        printf("%f ",g[i]);
    }
    printf("]\n");
    */
    if(topologyRoutes) {
        for(topology::Nodes::iterator i=topologyNodes->begin();
                i!=topologyNodes->end();++i) {
            topology::Node* v=*i;
            v->var = new project::Variable(
                    project::Initial(v->rect->getCentreD(dim)),
                    project::Desired(-1));
            printf("init: id=%d,x=%f,y=%f\n",v->id,v->rect->getCentreD(0),v->rect->getCentreD(1));
        }
        topology::DesiredPositions des;
        if(preIteration) {
            if ((*preIteration)()) {
                for(vector<Lock>::iterator l=preIteration->locks.begin();
                        l!=preIteration->locks.end();l++) {
                    des.push_back(make_pair(l->id,l->pos[dim]));
                    printf("desi: id=%d,x=%f,y=%f\n",l->id,l->pos[0],l->pos[1]);
                }
            }
        }
        vector<project::Constraint*> pcs;
        if(dim==cola::HORIZONTAL) {
                Rectangle::setXBorder(0.1);
        }
        topology::TopologyConstraints t(dim,*topologyNodes,*topologyRoutes,pcs);
        printf(" %d constraint.\n",pcs.size());
        t.steepestDescent(g,HMap,des);
        Rectangle::setXBorder(0);
        for(topology::Nodes::iterator i=topologyNodes->begin();
                i!=topologyNodes->end();++i) {
            topology::Node* v=*i;
            coords[v->id]=v->var->getPosition();
            printf("Setting v[%d]=%f\n",v->id,v->var->getPosition());
            delete v->var;
        }
    } else {
        SparseMatrix H(HMap);
        valarray<double> oldCoords=coords;
        applyDescentVector(g,oldCoords,coords,oldStress,computeStepSize(H,g,g));
        if(!firstPass) {
            valarray<double> d(n);
            d=oldCoords-coords;
            double stepsize=computeStepSize(H,g,d);
            stepsize=max(0.,min(stepsize,1.));
            //printf(" dim=%d beta: ",dim);
            return applyDescentVector(d,oldCoords,coords,oldStress,stepsize);
        }
    }
    return computeStress();
}
double ConstrainedFDLayout::applyDescentVector(
        valarray<double> const &d,
        valarray<double> const &oldCoords,
        valarray<double> &coords,
        const double oldStress,
        double stepsize
        ) const {
    assert(d.size()==oldCoords.size());
    assert(d.size()==coords.size());
    while(fabs(stepsize)>0.00000000001) {
        coords=oldCoords-stepsize*d;
        double stress=computeStress();
        //printf(" applyDV: oldstress=%f, stress=%f, stepsize=%f\n", oldStress,stress,stepsize);
        if(oldStress>=stress) {
            return stress;
        }
        coords=oldCoords;
        stepsize*=0.5;
    }
    return oldStress;
}
void ConstrainedFDLayout::computeForces(
        const Dim dim,
        SparseMap &H,
        valarray<double> &g) {
    if(n==1) return;
    g=0;
    // for each node:
    for(unsigned u=0;u<n;u++) {
        // Stress model
        double Huu=0;
        for(unsigned v=0;v<n;v++) {
            if(u==v) continue;
            // no forces between disconnected parts of the graph
            if(G[u][v]==numeric_limits<unsigned>::max()) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            // we don't want long range attractive forces between 
            // not immediately connected nodes
            if(G[u][v]>1 && l>d) continue;
            // if we compute topology forces then they replace
            // forces between adjacent nodes
            if(topologyRoutes && G[u][v]==1) continue;
            double d2=d*d;
            /* force apart zero distances */
            if (l < 1e-30) {
                l=0.1;
            }
            double dx=dim==HORIZONTAL?rx:ry;
            double dy=dim==HORIZONTAL?ry:rx;
            g[u]+=dx*(l-d)/(d2*l);
            Huu-=H(u,v)=(d*dy*dy/(l*l*l)-1)/d2;
        }
        H(u,u)=Huu;
    }
}
double ConstrainedFDLayout::computeStepSize(
        SparseMatrix const &H, 
        valarray<double> const &g, 
        valarray<double> const &d) const
{
    assert(g.size()==d.size());
    assert(g.size()==H.rowSize());
    // stepsize = g'd / (d' H d)
    double numerator = dotProd(g,d);
    valarray<double> Hd(d.size());
    H.rightMultiply(d,Hd);
    double denominator = dotProd(d,Hd);
    //assert(numerator>=0);
    //assert(denominator>=0);
    if(denominator==0) return 0;
    return numerator/denominator;
}
double ConstrainedFDLayout::computeStress() const {
    double stress=0;
    for(unsigned u=0;u<n-1;u++) {
        for(unsigned v=u+1;v<n;v++) {
            // no forces between disconnected parts of the graph
            if(G[u][v]==numeric_limits<unsigned>::max()) continue;
            // if we compute forces over topological edge paths then they replace
            // forces between adjacent nodes
            if(topologyRoutes && G[u][v]==1) continue;
            double rx=X[u]-X[v], ry=Y[u]-Y[v];
            double l=sqrt(rx*rx+ry*ry);
            double d=D[u][v];
            // we don't want long range attractive forces between 
            // not immediately connected nodes
            if(G[u][v]>1 && l>d) continue;
            double d2=d*d;
            double rl=d-l;
            stress+=rl*rl/d2;
        }
    }
    if(topologyRoutes) {
        cola::sum_over(topologyRoutes->begin(),topologyRoutes->end(),stress,
                topology::ComputeStress());
    }
    return stress;
}
void ConstrainedFDLayout::move() {
        for(unsigned i=0;i<n;i++) {
            boundingBoxes[i]->moveCentre(X[i],Y[i]);
        }
    }

} // namespace cola

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:cindent:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :

