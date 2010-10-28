INCLUDES = -I$(top_srcdir) $(CAIROMM_CFLAGS)
common_LDADD = $(top_builddir)/libcola/libcola.la $(top_builddir)/libvpsc/libvpsc.la $(top_builddir)/libtopology/libtopology.la $(CAIROMM_LIBS)
check_PROGRAMS = random_graph nodedragging page_bounds constrained beautify unsatisfiable invalid makefeasible rectclustershapecontainment FixedRelativeConstraint01
#check_PROGRAMS = unconstrained constrained containment shortest_paths connected_components large_graph convex_hull scale_free trees random_graph large_graph nodedragging topology boundary planar beautify #resize
#check_PROGRAMS = beautify nodedragging topology boundary planar beautify resize resizealignment

FixedRelativeConstraint01_LDADD = $(common_LDADD)
FixedRelativeConstraint01_SOURCES = FixedRelativeConstraint01.cpp 

rectclustershapecontainment_LDADD = $(common_LDADD)
rectclustershapecontainment_SOURCES = rectclustershapecontainment.cpp 

random_graph_LDADD = $(common_LDADD)
random_graph_SOURCES = random_graph.cpp 

nodedragging_LDADD = $(common_LDADD)
nodedragging_SOURCES = nodedragging.cpp 

page_bounds_LDADD = $(common_LDADD)
page_bounds_SOURCES = page_bounds.cpp 

constrained_LDADD = $(common_LDADD)
constrained_SOURCES = constrained.cpp

unsatisfiable_LDADD = $(common_LDADD)
unsatisfiable_SOURCES = unsatisfiable.cpp 

invalid_LDADD = $(common_LDADD)
invalid_SOURCES = invalid.cpp 

makefeasible_LDADD = $(common_LDADD) $(top_srcdir)/libavoid/libavoid.la
makefeasible_SOURCES = makefeasible02.cpp 

beautify_LDADD = $(common_LDADD) $(top_srcdir)/libavoid/libavoid.la
beautify_SOURCES = beautify.cpp 

#shortest_paths_LDADD = $(top_builddir)/libcola/libcola.la
#shortest_paths_SOURCES = shortest_paths.cpp
#unconstrained_LDADD = $(common_LDADD)
#unconstrained_SOURCES = unconstrained.cpp 
#containment_LDADD = $(common_LDADD)
#containment_SOURCES = containment.cpp 
#topology_LDADD = $(common_LDADD)
#topology_SOURCES = topology.cpp 
#boundary_LDADD = $(common_LDADD)
#boundary_SOURCES = boundary.cpp 
#resize_LDADD = $(common_LDADD)
#resize_SOURCES = resize.cpp 
#resizealignment_LDADD = $(common_LDADD)
#resizealignment_SOURCES = resizealignment.cpp 
#planar_LDADD = $(common_LDADD)
#planar_SOURCES = planar.cpp 
#connected_components_LDADD = $(common_LDADD)
#connected_components_SOURCES = connected_components.cpp
# test Requires boost
#sparse_matrix_LDADD = $(top_srcdir)/libcola/libcola.la $(top_srcdir)/libvpsc/libvpsc.la
#sparse_matrix_SOURCES = sparse_matrix.cpp
#convex_hull_SOURCES = convex_hull.cpp
#convex_hull_LDADD = $(common_LDADD) -lcairo -lcairomm-1.0
#aarontest_SOURCES = aarontest.cpp
#aarontest_LDADD = $(top_srcdir)/libcola/libcola.la $(top_builddir)/libvpsc/libvpsc.la -lcairo -lcairomm-1.0
#qo1_SOURCES = qo1.c
#qo1_LDADD = -L$(mosek_home)/bin -lmosek -lguide -limf
#small_graph_LDADD = $(top_srcdir)/libcola/libcola.la $(top_srcdir)/libvpsc/libvpsc.la
#small_graph_SOURCES = small_graph.cpp 
#large_graph_LDADD = $(common_LDADD)
#large_graph_SOURCES = large_graph.cpp 
#gml_graph_LDADD = $(common_LDADD) /usr/lib/libboost_regex.so
#gml_graph_SOURCES = gml_graph.cpp 
#scale_free_LDADD = $(common_LDADD)
#scale_free_SOURCES = scale_free.cpp 
#trees_LDADD = $(common_LDADD)
#trees_SOURCES = trees.cpp 

TESTS = $(check_PROGRAMS)
