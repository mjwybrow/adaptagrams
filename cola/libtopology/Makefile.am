INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES = libtopology.la

libtopology_la_SOURCES = topology_graph.cpp\
			topology_constraints.cpp\
			compute_forces.cpp\
			topology_graph.h\
			topology_constraints.h\
			topology_constraints_constructor.cpp\
			resize.cpp \
			cola_topology_addon.cpp \
			orthogonal_topology.cpp


libtopologyincludedir = ${includedir}/libtopology
libtopologyinclude_HEADERS = topology_constraints.h \
			topology_graph.h \
			cola_topology_addon.h \
			orthogonal_topology.h


SUBDIRS = . tests
