AM_CPPFLAGS = -I$(top_srcdir) $(CAIROMM_CFLAGS)

LDADD = \
  $(top_builddir)/libvpsc/libvpsc.la \
  $(top_builddir)/libtopology/libtopology.la \
  $(top_builddir)/libcola/libcola.la \
  $(top_builddir)/libavoid/libavoid.la \
  $(CAIROMM_LIBS)

check_PROGRAMS = random_graph page_bounds constrained unsatisfiable invalid makefeasible rectclustershapecontainment FixedRelativeConstraint01 StillOverlap01 StillOverlap02 shortest_paths rectangularClusters01 overlappingClusters01 overlappingClusters02 overlappingClusters04 initialOverlap
#check_PROGRAMS = unconstrained constrained containment shortest_paths connected_components large_graph convex_hull scale_free trees random_graph large_graph topology boundary planar #resize
#check_PROGRAMS = topology boundary planar resize resizealignment

# problem_SOURCES = problem.cpp

initialOverlap_SOURCES = initialOverlap.cpp

overlappingClusters01_SOURCES = overlappingClusters01.cpp
overlappingClusters02_SOURCES = overlappingClusters02.cpp
overlappingClusters04_SOURCES = overlappingClusters04.cpp

rectangularClusters01_SOURCES = rectangularClusters01.cpp

StillOverlap01_SOURCES = StillOverlap01.cpp 
StillOverlap02_SOURCES = StillOverlap02.cpp 

FixedRelativeConstraint01_SOURCES = FixedRelativeConstraint01.cpp 

rectclustershapecontainment_SOURCES = rectclustershapecontainment.cpp 

random_graph_SOURCES = random_graph.cpp 

page_bounds_SOURCES = page_bounds.cpp 

constrained_SOURCES = constrained.cpp

unsatisfiable_SOURCES = unsatisfiable.cpp 

invalid_SOURCES = invalid.cpp 

makefeasible_LDADD = $(LDADD) $(top_srcdir)/libavoid/libavoid.la
makefeasible_SOURCES = makefeasible02.cpp 

shortest_paths_SOURCES = shortest_paths.cpp
#unconstrained_SOURCES = unconstrained.cpp 
#containment_SOURCES = containment.cpp 
#topology_SOURCES = topology.cpp 
#boundary_SOURCES = boundary.cpp 
#resize_SOURCES = resize.cpp 
#resizealignment_SOURCES = resizealignment.cpp 
#planar_SOURCES = planar.cpp 
#connected_components_SOURCES = connected_components.cpp
# test Requires boost
#sparse_matrix_SOURCES = sparse_matrix.cpp
#convex_hull_SOURCES = convex_hull.cpp
#convex_hull_LDADD = $(LDADD) -lcairo -lcairomm-1.0
#aarontest_SOURCES = aarontest.cpp
#aarontest_LDADD = $(top_srcdir)/libcola/libcola.la $(top_builddir)/libvpsc/libvpsc.la -lcairo -lcairomm-1.0
#qo1_SOURCES = qo1.c
#qo1_LDADD = -L$(mosek_home)/bin -lmosek -lguide -limf
#small_graph_SOURCES = small_graph.cpp 
#large_graph_SOURCES = large_graph.cpp 
#gml_graph_LDADD = $(common_LDADD) /usr/lib/libboost_regex.so
#gml_graph_SOURCES = gml_graph.cpp 
#scale_free_SOURCES = scale_free.cpp 
#trees_SOURCES = trees.cpp 

TESTS = $(check_PROGRAMS)
