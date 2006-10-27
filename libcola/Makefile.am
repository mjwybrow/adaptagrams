INCLUDES = -I$(top_srcdir)
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
	convex_hull.cpp

include_HEADERS = cola.h gradient_projection.h sparse_matrix.h straightener.h

SUBDIRS = tests
