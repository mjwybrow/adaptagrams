INCLUDES = -I$(top_srcdir)/libcola -I$(top_srcdir)/libvpsc

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
        cycle_detector.cpp

include_HEADERS = cola.h
	conjugate_gradient.h\
	gradient_projection.h\
	shortest_paths.h\
	straightener.h\
        cycle_detector.h

SUBDIRS = tests
