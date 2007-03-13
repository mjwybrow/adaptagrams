INCLUDES = -I$(top_srcdir) -I/usr/include/cairo -I/usr/include/cairomm-1.0
AM_CXXFLAGS = -Wall

lib_LIBRARIES = libcola.a

libcola_a_SOURCES = cola.h\
	cola.cpp\
	conjugate_gradient.cpp\
	conjugate_gradient.h\
	gradient_projection.cpp\
	gradient_projection.h\
	shortest_paths.cpp\
	shortest_paths.h\
	straightener.h\
	straightener.cpp\
        cycle_detector.h\
        cycle_detector.cpp\
	connected_components.cpp\
	convex_hull.h\
	convex_hull.cpp\
	cluster.cpp\
	compound_constraints.h\
	output_svg.cpp\
	output_svg.h

include_HEADERS = cola.h gradient_projection.h sparse_matrix.h straightener.h 

SUBDIRS = tests
