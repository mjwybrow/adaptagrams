INCLUDES = -I$(top_srcdir)/libcola -I/home/dwyer/devel/solve_VPSC/libvpsc
#check_PROGRAMS = unconstrained constrained containment straightener shortest_paths
check_PROGRAMS = straightener 
shortest_paths_SOURCES = shortest_paths.cpp
shortest_paths_LDADD = $(top_srcdir)/libcola/libcola.a
unconstrained_SOURCES = unconstrained.cpp graphlayouttest.cpp
unconstrained_LDADD = $(top_srcdir)/libcola/libcola.a /home/dwyer/devel/solve_VPSC/libvpsc/libvpsc.la
constrained_SOURCES = constrained.cpp graphlayouttest.cpp
constrained_LDADD = $(top_srcdir)/libcola/libcola.a /home/dwyer/devel/solve_VPSC/libvpsc/libvpsc.la
containment_SOURCES = containment.cpp graphlayouttest.cpp
containment_LDADD = $(top_srcdir)/libcola/libcola.a /home/dwyer/devel/solve_VPSC/libvpsc/libvpsc.la
straightener_SOURCES = straightener.cpp graphlayouttest.cpp
straightener_LDADD = $(top_srcdir)/libcola/libcola.a /home/dwyer/devel/solve_VPSC/libvpsc/libvpsc.la
TESTS = $(check_PROGRAMS)

