AM_CPPFLAGS = -I$(top_srcdir)

check_PROGRAMS = rectangleoverlap block satisfy_inc # cycle
satisfy_inc_SOURCES = satisfy_inc.cpp
satisfy_inc_LDADD = $(top_builddir)/libvpsc/libvpsc.la # -L$(mosek_home)/bin -lmosek -lguide -limf -lirc
block_SOURCES = block.cpp
block_LDADD = $(top_builddir)/libvpsc/libvpsc.la
rectangleoverlap_SOURCES = rectangleoverlap.cpp
rectangleoverlap_LDADD = $(top_builddir)/libvpsc/libvpsc.la

#cycle_SOURCES = cycle.cpp
#cycle_LDADD = $(top_builddir)/libvpsc/libvpsc.la

TESTS = $(check_PROGRAMS)

