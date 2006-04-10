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

}

template <typename Point = cola::Point<double> >
struct Position {
	typedef std::vector<Point> Vec;
	typedef iterator_property_map< typename Vec::iterator, IndexMap> Map;
};

#include "conjugate_gradient.h"
namespace cola {
	
	template <typename T>
	struct layout_tolerance {
		layout_tolerance(const T& tolerance = T(0.001), const unsigned maxiterations = 100)
			: tolerance(tolerance),
              maxiterations(maxiterations),
              old_stress((std::numeric_limits<T>::max)()),
              iterations(0) { }

		bool operator()(T new_stress, const Graph& g) {
            std::cout<<"iteration="<<iterations<<", new_stress="<<new_stress<<std::endl;
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
    		T old_stress;
            unsigned iterations;
  	};
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
			  edge_or_side_length(edge_or_side_length), done(done) {}

        static inline double euclidean_distance(double **coords, int i, int j)
        {
            double sum = 
                (coords[0][i] - coords[0][j]) * (coords[0][i] - coords[0][j]) +
                (coords[1][i] - coords[1][j]) * (coords[1][i] - coords[1][j]);
            return sqrt(sum);
        }

        static inline double compute_stress(unsigned n, double **coords, double **Dij)
        {
            /* compute the overall stress */
            double sum = 0, d, diff;
            for (unsigned i = 1; i < n; i++) {
                for (unsigned j = 0; j < i; j++) {
                    d = Dij[i][j];
                    diff = d - euclidean_distance(coords,i,j);
                    sum += diff*diff / (d*d);
                }
            }
            return sum;
        }

        void majlayout(unsigned n, double** lap2, double** Dij, double** coords){
            bool Verbose = true;
            double b[n];
            double L_ij,dist_ij,degree,conj_tol=0.0001;
            while(!done(compute_stress(n, coords, Dij),g)) {
                /* Axis-by-axis optimization: */
                for (unsigned k = 0; k < 2; k++) {
                    /* compute the vector b */
                    /* multiply on-the-fly with distance-based laplacian */
                    /* (for saving storage we don't construct this Laplacian explicitly) */
                    for (unsigned i = 0; i < n; i++) {
                        degree = 0;
                        b[i] = 0;
                        for (unsigned j = 0; j < n; j++) {
                            if (j == i) continue;
                            dist_ij = euclidean_distance(coords, i, j);
                            if (dist_ij > 1e-30) {	/* skip zero distances */
                                /* calculate L_ij := w_{ij}*d_{ij}/dist_{ij} */
                                L_ij = -1.0 / (dist_ij * Dij[i][j]);
                                degree -= L_ij;
                                b[i] += L_ij * coords[k][j];
                            }
                        }
                        b[i] += degree * coords[k][i];
                    }
                    conjugate_gradient(lap2, coords[k], b, n, conj_tol, n, true);
                }
            }
            if (Verbose) fprintf(stderr, "\nfinal e = %f\n", compute_stress(n, coords, Dij));

        }	  
		bool run() {
			double** coords;
            typename graph_traits<Graph>::vertices_size_type n = num_vertices(g);
			coords = new double*[2];
			coords[0] = new double[n];
			coords[1] = new double[n];
			return run(n,coords);
		}
		bool run(unsigned n, double** coords) {
			typedef typename property_traits<WeightMap>::value_type weight_type;
			vec_adj_list_vertex_id_map<no_property, unsigned int> index = get(vertex_index,g);
			typedef std::vector<weight_type> weight_vec;
			std::vector<weight_vec> distance(n,weight_vec(n));

			if (!johnson_all_pairs_shortest_paths(g, distance, index, weight, weight_type(0)))
				return false;
			weight_type edge_length = detail::graph::compute_edge_length(g, distance, index,
                                             edge_or_side_length);
			// Lij_{i!=j}=1/(Dij^2)
			//
			double** lap2 = new double*[n];
			double** Dij = new double*[n];
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
                delete [] lap2[i];
                delete [] Dij[i];
            }
            delete [] lap2;
            delete [] Dij;
            delete [] coords[0];
            delete [] coords[1];
			return true;
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
	return alg.run();
}
template <typename PositionMap, typename T, bool EdgeOrSideLength, typename Done >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	detail::graph::edge_or_side<EdgeOrSideLength, T> edge_or_side_length,
	Done done,
    double** coords) {
	cola::constrained_majorization_layout_impl<PositionMap,detail::graph::edge_or_side<EdgeOrSideLength, T>,Done> 
		alg(g,position,weight,edge_or_side_length,done);
	return alg.run(num_vertices(g),coords);
}
template <typename PositionMap, typename T, bool EdgeOrSideLength >
bool constrained_majorization_layout(
	const Graph& g,
	PositionMap position,
	WeightMap weight,
	detail::graph::edge_or_side<EdgeOrSideLength, T> edge_or_side_length) {
	return constrained_majorization_layout(g,position,weight,edge_or_side_length,cola::layout_tolerance<double>());
}
#endif				// STRESSMAJORIZATION_H
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
