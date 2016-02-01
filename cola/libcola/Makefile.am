EXTRA_DIST=libcola.pc.in

lib_LTLIBRARIES = libcola.la
libcola_la_CPPFLAGS = -I$(top_srcdir) $(CAIROMM_CFLAGS) -I$(includedir)/libcola -fPIC

# Depends on libvpsc
libcola_la_LIBADD = $(top_builddir)/libvpsc/libvpsc.la $(CAIROMM_LIBS)

libcola_la_SOURCES = cola.h\
	cola.cpp\
	colafd.cpp\
	conjugate_gradient.cpp\
	conjugate_gradient.h\
	exceptions.h\
	gradient_projection.cpp\
	gradient_projection.h\
	shortest_paths.h\
	straightener.h\
	straightener.cpp\
	connected_components.cpp\
	convex_hull.h\
	convex_hull.cpp\
	cluster.cpp\
	compound_constraints.h\
	compound_constraints.cpp\
	pseudorandom.h \
	pseudorandom.cpp \
	output_svg.cpp\
	output_svg.h \
	unused.h \
	cc_clustercontainmentconstraints.cpp \
	cc_clustercontainmentconstraints.h \
	cc_nonoverlapconstraints.cpp \
	cc_nonoverlapconstraints.h \
	box.cpp \
	box.h \
	shapepair.cpp \
	shapepainr.h

libcolaincludedir = $(includedir)/libcola
libcolainclude_HEADERS = cola.h\
	cluster.h\
	commondefs.h\
	compound_constraints.h\
	pseudorandom.h \
	exceptions.h\
	gradient_projection.h\
	sparse_matrix.h\
	straightener.h \
	output_svg.h \
	unused.h \
	cc_clustercontainmentconstraints.h \
	cc_nonoverlapconstraints.h \
	box.h \
	shapepair.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libcola.pc

SUBDIRS = . tests
