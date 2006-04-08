// Copyright 2004 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef STRESSMAJORIZATION_H
#define STRESSMAJORIZATION_H

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <utility>
#include <iterator>
#include <vector>
#include <boost/limits.hpp>
#include <cmath>

using namespace boost;


// create a typedef for the Graph type
typedef adjacency_list<vecS, vecS, undirectedS, no_property, 
	property<edge_weight_t, double> > Graph;
typedef property_map<Graph, edge_weight_t>::type WeightMap;
typedef graph_traits<Graph>::vertex_descriptor Vertex;

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
typedef iterator_property_map<
		typename Vec::iterator, 
		property_map<Graph, vertex_index_t>::type> Map;
};

void becausewecan();

namespace cola {
	template <typename PositionMap, typename Done >
	struct constrained_majorization_layout_impl {
		constrained_majorization_layout_impl(
				const Graph& g, 
				PositionMap position,
				WeightMap weight,
				Done done) : g(g), position(position), weight(weight), done(done) { }
		bool run() {
		}
		const Graph& g;
		PositionMap position;
		WeightMap weight;
		Done done;
	};
}
template <typename PositionMap, typename Done >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	Done done) {
	becausewecan();	
	cola::constrained_majorization_layout_impl<PositionMap,Done> alg(g,position,weight,done);
	alg.run();
}
template <typename PositionMap >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight) {
	constrained_majorization_layout(g,position,weight,cola::layout_tolerance<double>());
}
#endif				// STRESSMAJORIZATION_H
