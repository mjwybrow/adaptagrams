INCLUDES = -I$(top_srcdir)/libcola -I../../solve_VPSC/libvpsc 
#check_PROGRAMS = unconstrained constrained containment straightener shortest_paths
check_PROGRAMS = unconstrained
shortest_paths_SOURCES = shortest_paths.cpp
shortest_paths_LDADD = $(top_srcdir)/libcola/libcola.a
unconstrained_SOURCES = unconstrained.cpp graphlayouttest.cpp
unconstrained_LDADD = $(top_srcdir)/libcola/libcola.a ../../solve_VPSC/libvpsc/libvpsc.la
constrained_SOURCES = constrained.cpp graphlayouttest.cpp
constrained_LDADD = $(top_srcdir)/libcola/libcola.a ../../solve_VPSC/libvpsc/libvpsc.la
containment_SOURCES = containment.cpp graphlayouttest.cpp
containment_LDADD = $(top_srcdir)/libcola/libcola.a ../../solve_VPSC/libvpsc/libvpsc.la
straightener_SOURCES = straightener.cpp graphlayouttest.cpp
straightener_LDADD = $(top_srcdir)/libcola/libcola.a ../../solve_VPSC/libvpsc/libvpsc.la
#test_cg_LDADD = -lgsl -lblas -lstdc++ $(top_srcdir)/libcola/libcola.a
TESTS = $(check_PROGRAMS)

