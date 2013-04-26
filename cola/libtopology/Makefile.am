
lib_LTLIBRARIES = libtopology.la
libtopology_la_CPPFLAGS = -I$(top_srcdir) -I{includedir}/libtopology -fPIC

libtopology_la_SOURCES = topology_graph.cpp\
			topology_constraints.cpp\
			compute_forces.cpp\
			topology_constraints_constructor.cpp\
			resize.cpp \
			cola_topology_addon.cpp \
                        orthogonal_topology.cpp \
			topology_constraints.h \
			topology_graph.h \
			cola_topology_addon.h \
			orthogonal_topology.h \
			topology_log.h \
			util.h


libtopologyincludedir = ${includedir}/libtopology
libtopologyinclude_HEADERS = topology_constraints.h \
			topology_graph.h \
			cola_topology_addon.h \
			orthogonal_topology.h \
			topology_log.h \
			util.h


SUBDIRS = . tests
