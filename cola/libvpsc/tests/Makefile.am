INCLUDES = -I$(top_srcdir)
check_PROGRAMS = rectangleoverlap block satisfy_inc #csolve_VPSC cycle
satisfy_inc_SOURCES = satisfy_inc.cpp
satisfy_inc_LDADD = $(top_builddir)/libvpsc/libvpsc.a # -L$(mosek_home)/bin -lmosek -lguide -limf -lirc
block_SOURCES = block.cpp
block_LDADD = $(top_builddir)/libvpsc/libvpsc.a
rectangleoverlap_SOURCES = rectangleoverlap.cpp
rectangleoverlap_LDADD = $(top_builddir)/libvpsc/libvpsc.a
# this example requires libvpsc to be compiled as a dynamic library
# (which we're not currently doing!)
#csolve_VPSC_SOURCES = csolve_VPSC.c
#csolve_VPSC_LDADD = $(top_builddir)/libvpsc/libvpsc.a

#cycle_SOURCES = cycle.cpp
#cycle_LDADD = $(top_builddir)/libvpsc/libvpsc.a

TESTS = $(check_PROGRAMS)

