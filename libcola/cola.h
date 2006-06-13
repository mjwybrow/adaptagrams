#ifndef COLA_H
#define COLA_H

#include <utility>
#include <iterator>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "shortest_paths.h"
#include "gradient_projection.h"
#include <generate-constraints.h>


typedef vector<unsigned> Cluster;
typedef vector<Cluster*> Clusters;

namespace cola {
    typedef pair<unsigned, unsigned> Edge;

    struct Route {
        Route(unsigned n) : n(n), xs(new double[n]), ys(new double[n]) {}
        double *xs;
        double *ys;
        unsigned n;
    };
    // defines references to three variables for which the goal function
    // will be altered to prefer points u-b-v are in a linear arrangement
    // such that b is placed at u+t(v-u).
    struct LinearConstraint {
        LinearConstraint(unsigned u, unsigned v, unsigned b, double w, double* X, double* Y) : u(u),v(v),b(b),w(w) {
            double numerator=X[b]-X[u];
            double denominator=X[v]-X[u];
            if(fabs(denominator)<0.001) {
                // if line is close to vertical then use Y coords to compute T
                numerator=Y[b]-Y[u];
                denominator=Y[v]-Y[u];
            }
            if(fabs(denominator)<0.0001) {
                denominator=1;
            }
            t=numerator/denominator;
            duu=(1-t)*(1-t);
            duv=t*(1-t);
            dub=t-1;
            dvv=t*t;
            dvb=-t;
            dbb=1;
             //printf("New LC: t=%f\n",t); 
        }
        unsigned u;
        unsigned v;
        unsigned b;
        double w; // weight
        double t;
        // 2nd partial derivatives of the goal function
        //   (X[b] - (1-t) X[u] - t X[v])^2
        double duu;
        double duv;
        double dub;
        double dvv;
        double dvb;
        double dbb;
    };
    typedef vector<LinearConstraint*> LinearConstraints;
	
	class TestConvergence {
    public:
        double old_stress;
		TestConvergence(const double& tolerance = 0.001, const unsigned maxiterations = 1000)
			: old_stress(DBL_MAX),
              tolerance(tolerance),
              maxiterations(maxiterations),
              iterations(0) { }

		virtual bool operator()(double new_stress, double* X, double* Y) {
            //std::cout<<"iteration="<<iterations<<", new_stress="<<new_stress<<std::endl;
			if (old_stress == DBL_MAX) {
				old_stress = new_stress;
                if(++iterations>=maxiterations) {;
                    return true;
                } else {
          			return false;
                }
			}
            bool converged = 
                fabs(new_stress - old_stress) / (new_stress + 1e-10) < tolerance
                || ++iterations > maxiterations;
            old_stress = new_stress;
			return converged;
		}
  	private:
        double tolerance;
        unsigned maxiterations;
        unsigned iterations;
  	};
    static TestConvergence defaultTest(0.0001,100);
	class ConstrainedMajorizationLayout {
    public:
		ConstrainedMajorizationLayout(
                vector<Rectangle*>& rs,
                vector<Edge>& es,
				double* eweights,
                double maxLength,
				TestConvergence& done=defaultTest)
			: constrainedLayout(false),
              n(rs.size()),
              lapSize(n), lap2(new double*[lapSize]), Q(lap2), Dij(new double*[lapSize]),
              tol(0.0001),
              done(done),
              X(new double[n]),
              Y(new double[n]),
              clusters(NULL),
              layoutXAxis(true),
              layoutYAxis(true),
              linearConstraints(NULL),
              gpX(NULL),
              gpY(NULL)
        {
            assert(rs.size()==n);
            boundingBoxes = new Rectangle*[rs.size()];
            copy(rs.begin(),rs.end(),boundingBoxes);

            double** D=new double*[n];
            for(unsigned i=0;i<n;i++) {
                D[i]=new double[n];
            }
            shortest_paths::johnsons(n,D,es,eweights);
            edge_length = maxLength;
            // Lij_{i!=j}=1/(Dij^2)
            //
            for(unsigned i = 0; i<n; i++) {
                X[i]=rs[i]->getCentreX();
                Y[i]=rs[i]->getCentreY();
                double degree = 0;
                lap2[i]=new double[n];
                Dij[i]=new double[n];
                for(unsigned j=0;j<n;j++) {
                    double w = edge_length * D[i][j];
                    Dij[i][j]=w;
                    if(i==j) continue;
                    degree+=lap2[i][j]=w>1e-30?1.f/(w*w):0;
                }
                lap2[i][i]=-degree;
                delete D[i];
            }
            delete [] D;
        }

        void moveBoundingBoxes() {
            for(unsigned i=0;i<n;i++) {
                boundingBoxes[i]->moveCentreX(X[i]);
                boundingBoxes[i]->moveCentreY(Y[i]);
            }
        }

        void setRoutes(vector<Edge>& edges, vector<Route*>& routes);

        void setupConstraints(
                AlignmentConstraints* acsx, AlignmentConstraints* acsy,
                bool avoidOverlaps, 
                PageBoundaryConstraints* pbcx = NULL,
                PageBoundaryConstraints* pbcy = NULL,
                SimpleConstraints* scx = NULL,
                SimpleConstraints* scy = NULL,
                Clusters* cs = NULL);

        void addLinearConstraints(LinearConstraints* linearConstraints,
               vector<Rectangle*>& rs);

        void setupDummyVars();

        ~ConstrainedMajorizationLayout() {
            if(boundingBoxes) {
                for(unsigned i = 0; i < n; i++) {
                    delete boundingBoxes[i];
                }
                delete [] boundingBoxes;
            }
            if(constrainedLayout) {
                delete gpX;
                delete gpY;
            }
            for(unsigned i=0;i<n;i++) {
                delete [] lap2[i];
                delete [] Dij[i];
            }
            delete [] lap2;
            delete [] Dij;
            delete [] X;
            delete [] Y;
        }
		bool run();
        bool avoidOverlaps;
        bool constrainedLayout;
        void setLayoutAxes(bool xAxis, bool yAxis) {
            layoutXAxis=xAxis;
            layoutYAxis=yAxis;
        }
    private:
        double euclidean_distance(unsigned i, unsigned j) {
            return sqrt(
                (X[i] - X[j]) * (X[i] - X[j]) +
                (Y[i] - Y[j]) * (Y[i] - Y[j]));
        }
        double compute_stress(double **Dij);
        void majlayout(double** Dij,GradientProjection* gp, double* coords);
        unsigned n; // is lapSize + dummyVars
        unsigned lapSize; // lapSize is the number of variables for actual nodes
        double** lap2; // graph laplacian
        double** Q; // quadratic terms matrix used in computations
        double** Dij;
        double tol;
		TestConvergence& done;
        Rectangle** boundingBoxes;
        double *X, *Y;
        Clusters* clusters;
        double edge_length;
        bool layoutXAxis;
        bool layoutYAxis;
        LinearConstraints *linearConstraints;
        GradientProjection *gpX, *gpY;
	};
}
#endif				// COLA_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
