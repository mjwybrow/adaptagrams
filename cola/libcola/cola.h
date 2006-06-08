#ifndef COLA_H
#define COLA_H

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <utility>
#include <iterator>
#include <vector>
#include <algorithm>
#include <boost/limits.hpp>
#include <cmath>
#include <iostream>
//#include "conjugate_gradient.h"
#include "gradient_projection.h"
#include <generate-constraints.h>

using namespace boost;

namespace cola {
struct route {
	double *xs;
	double *ys;
	unsigned n;
};
}
// create a typedef for the Graph type
typedef adjacency_list<vecS, vecS, undirectedS, no_property, 
	property<edge_weight_t, double,
      property<edge_index_t, cola::route* > > > Graph;
typedef property_map<Graph, edge_weight_t>::type WeightMap;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef property_map<Graph, vertex_index_t>::type IndexMap;
typedef property_map<Graph, edge_index_t>::type EdgeRouteMap;

typedef vector<unsigned> Cluster;
typedef vector<Cluster*> Clusters;

namespace cola {
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
	
	template <typename T>
	class layout_tolerance {
    public:
        T old_stress;
		layout_tolerance(const T& tolerance = T(0.001), const unsigned maxiterations = 1000)
			: old_stress((std::numeric_limits<T>::max)()),
              tolerance(tolerance),
              maxiterations(maxiterations),
              iterations(0) { }

		bool operator()(T new_stress, const Graph& g, double* X, double* Y) {
            //std::cout<<"iteration="<<iterations<<", new_stress="<<new_stress<<std::endl;
			if (old_stress == (std::numeric_limits<T>::max)()) {
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
        T tolerance;
        unsigned maxiterations;
        unsigned iterations;
  	};
	template <typename EdgeOrSideLength, typename Done >
	class constrained_majorization_layout_impl {
    public:
		constrained_majorization_layout_impl(
				const Graph& g, 
                vector<Rectangle*>& rs,
				WeightMap& weight,
				EdgeOrSideLength edge_or_side_length,
				Done done)
			: constrainedLayout(false),
              g(g), n(num_vertices(g)),
              lapSize(n), lap2(new double*[lapSize]), Q(lap2), Dij(new double*[lapSize]),
              tol(0.0001),
			  weight(weight), 
			  edge_or_side_length(edge_or_side_length), 
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
            typedef typename property_traits<WeightMap>::value_type weight_type;
            vec_adj_list_vertex_id_map<no_property, unsigned int> index = get(vertex_index,g);
            typedef std::vector<weight_type> weight_vec;
            std::vector<weight_vec> distance(n,weight_vec(n));
            assert(rs.size()==n);
            boundingBoxes = new Rectangle*[rs.size()];
            copy(rs.begin(),rs.end(),boundingBoxes);

            if (!johnson_all_pairs_shortest_paths(g, distance, index, weight, weight_type(0))) {
                cerr << "All pairs shortest paths computation returned false!" << endl;
                return;
            }
            edge_length = detail::graph::compute_edge_length(g, distance, index,
                                             edge_or_side_length);
            // Lij_{i!=j}=1/(Dij^2)
            //
            for(unsigned i = 0; i<n; i++) {
                X[i]=rs[i]->getCentreX();
                Y[i]=rs[i]->getCentreY();
                weight_type degree = weight_type(0);
                lap2[i]=new double[n];
                Dij[i]=new double[n];
                for(unsigned j=0;j<n;j++) {
                    weight_type w = edge_length * distance[i][j];
                    Dij[i][j]=w;
                    if(i==j) continue;
                    degree+=lap2[i][j]=w>1e-30?weight_type(1)/(w*w):0;
                }
                lap2[i][i]=-degree;
            }
        }
		~constrained_majorization_layout_impl() {
            for(unsigned i=0;i<n;i++) {
                delete [] lap2[i];
                delete [] Dij[i];
            }
            delete [] lap2;
            delete [] Dij;
            delete [] boundingBoxes;
            if(gpX!=NULL) delete gpX;
            if(gpY!=NULL) delete gpY;
            delete [] X;
            delete [] Y;
        }

        void moveBoundingBoxes() {
            for(unsigned i=0;i<n;i++) {
                boundingBoxes[i]->moveCentreX(X[i]);
                boundingBoxes[i]->moveCentreY(Y[i]);
            }
        }

        void setupConstraints(
                AlignmentConstraints* acsx, AlignmentConstraints* acsy,
                bool avoidOverlaps, 
                PageBoundaryConstraints* pbcx = NULL,
                PageBoundaryConstraints* pbcy = NULL,
                SimpleConstraints* scx = NULL,
                SimpleConstraints* scy = NULL,
                Clusters* cs = NULL);

        void addLinearConstraints(LinearConstraints* linearConstraints, vector<Rectangle*>& dim = NULL);

        void setupDummyVars();

        ~constrained_majorization_layout_impl() {
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
            delete [] lap2;
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
		const Graph& g;
        unsigned n; // is lapSize + dummyVars
        unsigned lapSize; // lapSize is the number of variables for actual nodes
        double** lap2; // graph laplacian
        double** Q; // quadratic terms matrix used in computations
        double** Dij;
        double tol;
		WeightMap weight;
		EdgeOrSideLength edge_or_side_length;
		Done done;
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
#include "cola.cpp"
#endif				// COLA_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
