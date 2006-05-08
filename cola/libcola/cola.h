#ifndef STRESSMAJORIZATION_H
#define STRESSMAJORIZATION_H

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <utility>
#include <iterator>
#include <vector>
#include <boost/limits.hpp>
#include <cmath>
#include <iostream>
//#include "conjugate_gradient.h"
#include "gradient_projection.h"
#include <generate-constraints.h>

using namespace boost;


// create a typedef for the Graph type
typedef adjacency_list<vecS, vecS, undirectedS, no_property, 
	property<edge_weight_t, double> > Graph;
typedef property_map<Graph, edge_weight_t>::type WeightMap;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef property_map<Graph, vertex_index_t>::type IndexMap;

typedef vector<unsigned> Cluster;
typedef vector<Cluster*> Clusters;

namespace cola {
	// we expect points to have an x and a y member of float type
	template <typename T>
	struct Point {
	    T x;
	    T y;
	};
}

template <typename Point = cola::Point<double> >
struct Position {
	typedef std::vector<Point> Vec;
	typedef iterator_property_map< typename Vec::iterator, IndexMap> Map;
};

namespace cola {
	
	template <typename T>
	class layout_tolerance {
    public:
        T old_stress;
		layout_tolerance(const T& tolerance = T(0.001), const unsigned maxiterations = 100)
			: old_stress((std::numeric_limits<T>::max)()),
              tolerance(tolerance),
              maxiterations(maxiterations),
              iterations(0) { }

		bool operator()(T new_stress, const Graph& g, double* X, double* Y) {
            //std::cout<<"iteration="<<iterations<<", new_stress="<<new_stress<<std::endl;
			if (old_stress == (std::numeric_limits<T>::max)()) {
				old_stress = new_stress;
          			return false;
			}
            bool converged = 
                fabs(new_stress - old_stress) / (new_stress + 1e-10) < tolerance
                || ++iterations >= maxiterations;
            old_stress = new_stress;
			return converged;
		}
  	private:
        T tolerance;
        unsigned maxiterations;
        unsigned iterations;
  	};
	template <typename PositionMap, typename EdgeOrSideLength, typename Done >
	class constrained_majorization_layout_impl {
    public:
		constrained_majorization_layout_impl(
				const Graph& g, 
				PositionMap position,
				WeightMap weight,
				EdgeOrSideLength edge_or_side_length,
				Done done) 
			: constrainedLayout(false),
              g(g), n(num_vertices(g)), position(position), 
              lap2(new double*[n]),
              tol(0.0001),
			  weight(weight), 
			  edge_or_side_length(edge_or_side_length), 
              done(done),
              boundingBoxes(NULL),
              X(new double[n]),
              Y(new double[n]),
              clusters(NULL) { }

        void moveBoundingBoxes() {
            for(unsigned i=0;i<n;i++) {
                boundingBoxes[i]->moveCentreX(X[i]);
                boundingBoxes[i]->moveCentreY(Y[i]);
            }
        }

        void setupConstraints(
                AlignmentConstraints* acsx, AlignmentConstraints* acsy,
                bool avoidOverlaps, PositionMap* dim = NULL,
                Clusters* cs = NULL);

        void setupDummyVars(GradientProjection* gp[2], double* coords[2]);

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
    private:
        double euclidean_distance(unsigned i, unsigned j) {
            return sqrt(
                (X[i] - X[j]) * (X[i] - X[j]) +
                (Y[i] - Y[j]) * (Y[i] - Y[j]));
        }
        double compute_stress(double **Dij);
        void majlayout(double** Dij);
        GradientProjection *gpX, *gpY;
		const Graph& g;
        unsigned n;
		PositionMap position;
        double** lap2; 
        double tol;
		WeightMap weight;
		EdgeOrSideLength edge_or_side_length;
		Done done;
        Rectangle** boundingBoxes;
        double *X, *Y;
        Clusters* clusters;
        double edge_length;
	};
}
// the following overloaded functions provide wrappers to setup for the most common
// usages of the constrained_majorization_layout_impl

// this is probably the simplest type of layout:
//   args are:
//   g - the graph definition
//   position map - for starting positions and resultant layout
//   weight map - set to all 1 if you've got nothing better
//   edge_length(value) or side_length(value) 
//     - where value is either the ideal edge length or an upper bound on the total width/height of the graph.
template <typename PositionMap, typename T, bool EdgeOrSideLength >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	detail::graph::edge_or_side<EdgeOrSideLength, T> edge_or_side_length) {
	return constrained_majorization_layout(g,position,weight,edge_or_side_length,cola::layout_tolerance<double>());
}

// additional args over the above:
//   done - is a functor which will be called every iteration to check convergence (and whatever else you like)
//     either duplicate the functionality of cola::layout_tolerance or call layout_tolerance from your functor
template <typename PositionMap, typename T, bool EdgeOrSideLength, typename Done >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	detail::graph::edge_or_side<EdgeOrSideLength, T> edge_or_side_length,
	Done done) {
	cola::constrained_majorization_layout_impl<PositionMap,detail::graph::edge_or_side<EdgeOrSideLength, T>,Done> 
		alg(g,position,weight,edge_or_side_length,done);
	return alg.run();
}
// additional args over the above:
//   acsx - horizontal alignment constraints
//   acsy - vertical alignment constraints
template <typename PositionMap, typename T, bool EdgeOrSideLength >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	detail::graph::edge_or_side<EdgeOrSideLength, T> edge_or_side_length,
    AlignmentConstraints* acsx,
    AlignmentConstraints* acsy) {
	return constrained_majorization_layout(
            g, position, weight, edge_or_side_length,
            cola::layout_tolerance<double>(),
            false, (PositionMap*)NULL,
            acsx, acsy, (Clusters*)NULL);
}
// additional args over the above:
//   clusters - vector of vectors of nodes grouped into clusters
template <typename PositionMap, typename T, bool EdgeOrSideLength >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	detail::graph::edge_or_side<EdgeOrSideLength, T> edge_or_side_length,
    Clusters* cs) {
	return constrained_majorization_layout(
            g, position, weight, edge_or_side_length,
            cola::layout_tolerance<double>(),
            false, (PositionMap*)NULL,
            (AlignmentConstraints*)NULL,(AlignmentConstraints*)NULL,cs);
}
// additional args over the above:
//   avoidOverlaps - if true, generate constraints to prevent rectangular node boundaries from overlapping 
//   dim - dimensions of node boundaries to be used in non-overlap constraints
template <typename PositionMap, typename T, bool EdgeOrSideLength, typename Done >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	detail::graph::edge_or_side<EdgeOrSideLength, T> edge_or_side_length,
	Done done,
    bool avoidOverlaps,
    PositionMap* dim,
    AlignmentConstraints* acsx,
    AlignmentConstraints* acsy,
    Clusters* cs) {
	cola::constrained_majorization_layout_impl<PositionMap,detail::graph::edge_or_side<EdgeOrSideLength, T>,Done> 
		alg(g,position,weight,edge_or_side_length,done);
    alg.setupConstraints(acsx,acsy,avoidOverlaps,dim,cs);
	return alg.run();
}
#include "cola.cpp"
#endif				// STRESSMAJORIZATION_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
