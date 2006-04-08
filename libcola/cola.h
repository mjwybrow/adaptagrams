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

using namespace boost;


// create a typedef for the Graph type
typedef adjacency_list<vecS, vecS, undirectedS, no_property, 
	property<edge_weight_t, double> > Graph;
typedef property_map<Graph, edge_weight_t>::type WeightMap;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef property_map<Graph, vertex_index_t>::type IndexMap;
namespace cola {
	// we expect points to have an x and a y member of float type
	template <typename T>
	struct Point {
	    T x;
	    T y;
	};

	template <typename T>
	struct layout_tolerance {
		layout_tolerance(const T& tolerance = T(0.001))
			: tolerance(tolerance), last_energy((std::numeric_limits<T>::max)()),
		last_local_energy((std::numeric_limits<T>::max)()) { }

		bool operator()(T delta_p, const Graph& g) {
			if (last_energy == (std::numeric_limits<T>::max)()) {
				last_energy = delta_p;
          			return false;
			}
			T diff = last_energy - delta_p;
			if (diff < T(0)) diff = -diff;
			bool done = (delta_p == T(0) || diff / last_energy < tolerance);
			last_energy = delta_p;
			return done;
		}
  	private:
    		T tolerance;
    		T last_energy;
    		T last_local_energy;
  	};
}

template <typename Point = cola::Point<double> >
struct Position {
	typedef std::vector<Point> Vec;
	typedef iterator_property_map< typename Vec::iterator, IndexMap> Map;
};

//template <typename Mat, typename PositionMap>
//void majlayout(Mat lap2, PositionMap position);
void majlayout(unsigned n, double** lap2, double** Dij, double** coords);

namespace cola {
	template <typename PositionMap, typename EdgeOrSideLength, typename Done >
	struct constrained_majorization_layout_impl {
		constrained_majorization_layout_impl(
				const Graph& g, 
				PositionMap position,
				WeightMap weight,
				EdgeOrSideLength edge_or_side_length,
				Done done) 
			: g(g), position(position), 
			  weight(weight), 
			  edge_or_side_length(edge_or_side_length), 
			  done(done) {}
		bool run() {
			typedef typename property_traits<WeightMap>::value_type weight_type;
			typename graph_traits<Graph>::vertices_size_type n = num_vertices(g);
			typedef std::vector<weight_type> weight_vec;
			vec_adj_list_vertex_id_map<no_property, unsigned int> index = get(vertex_index,g);
			std::vector<weight_vec> distance(n,weight_vec(n));

			if (!johnson_all_pairs_shortest_paths(g, distance, index, weight, weight_type(0)))
				return false;
			weight_type edge_length = detail::graph::compute_edge_length(g, distance, index,
                                             edge_or_side_length);
			// Lij_{i!=j}=1/(Dij^2)
			//
			double** lap2 = new double*[n];
			double** Dij = new double*[n];
			double** coords = new double*[2];
			coords[0] = new double[n];
			coords[1] = new double[n];
			for(unsigned i = 0; i<n; i++) {
				coords[0][i]=position[i].x;
				coords[1][i]=position[i].y;
			}
			for(unsigned i = 0; i<n; i++) {
				weight_type degree = weight_type(0);
				lap2[i]=new double[n];
				Dij[i]=new double[n];
				for(unsigned j=0;j<n;j++) {
					weight_type w = edge_length * distance[i][j];
					Dij[i][j]=w;
					if(i==j) continue;
					degree+=lap2[i][j]=weight_type(1)/(w*w);
				}
				lap2[i][i]=-degree;
			}
			majlayout(n,lap2, Dij, coords);	
			for(unsigned i = 0; i<n; i++) {
				position[i].x=coords[0][i];
				position[i].y=coords[1][i];
			}
			return false;
		}
		const Graph& g;
		PositionMap position;
		WeightMap weight;
		EdgeOrSideLength edge_or_side_length;
		Done done;
	};
}
template <typename PositionMap, typename T, bool EdgeOrSideLength, typename Done >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	detail::graph::edge_or_side<EdgeOrSideLength, T> edge_or_side_length,
	Done done) {
	cola::constrained_majorization_layout_impl<PositionMap,detail::graph::edge_or_side<EdgeOrSideLength, T>,Done> 
		alg(g,position,weight,edge_or_side_length,done);
	alg.run();
}
template <typename PositionMap, typename T, bool EdgeOrSideLength >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	detail::graph::edge_or_side<EdgeOrSideLength, T> edge_or_side_length) {
	constrained_majorization_layout(g,position,weight,edge_or_side_length,cola::layout_tolerance<double>());
}
#endif				// STRESSMAJORIZATION_H
