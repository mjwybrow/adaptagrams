INCLUDES = -I$(top_srcdir)/libcola -I/home/dwyer/devel/solve_VPSC/libvpsc
#check_PROGRAMS = unconstrained constrained containment straightener
check_PROGRAMS = straightener
unconstrained_SOURCES = unconstrained.cpp graphlayouttest.cpp
unconstrained_LDADD = $(top_srcdir)/libcola/libcola.a /home/dwyer/devel/solve_VPSC/libvpsc/libvpsc.la
constrained_SOURCES = constrained.cpp graphlayouttest.cpp
constrained_LDADD = $(top_srcdir)/libcola/libcola.a /home/dwyer/devel/solve_VPSC/libvpsc/libvpsc.la
containment_SOURCES = containment.cpp graphlayouttest.cpp
containment_LDADD = $(top_srcdir)/libcola/libcola.a /home/dwyer/devel/solve_VPSC/libvpsc/libvpsc.la
straightener_SOURCES = straightener.cpp graphlayouttest.cpp
straightener_LDADD = $(top_srcdir)/libcola/libcola.a /home/dwyer/devel/solve_VPSC/libvpsc/libvpsc.la
TESTS = $(check_PROGRAMS)

