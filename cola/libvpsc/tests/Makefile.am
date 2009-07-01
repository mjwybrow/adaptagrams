INCLUDES = -I$(top_srcdir)
check_PROGRAMS = rectangleoverlap block satisfy_inc # cycle
satisfy_inc_SOURCES = satisfy_inc.cpp
satisfy_inc_LDADD = $(top_builddir)/libvpsc/libvpsc.a # -L$(mosek_home)/bin -lmosek -lguide -limf -lirc
block_SOURCES = block.cpp
block_LDADD = $(top_builddir)/libvpsc/libvpsc.a
rectangleoverlap_SOURCES = rectangleoverlap.cpp
rectangleoverlap_LDADD = $(top_builddir)/libvpsc/libvpsc.a

#cycle_SOURCES = cycle.cpp
#cycle_LDADD = $(top_builddir)/libvpsc/libvpsc.a

TESTS = $(check_PROGRAMS)

