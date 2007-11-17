mosek_home=/home/dwyer/devel/mosek/5/tools/platform/linux32x86
#INCLUDES = -I$(top_srcdir) -I/usr/include/cairo -I/usr/include/cairomm-1.0
INCLUDES = -I$(top_srcdir) -I/usr/include/cairo -I/usr/include/cairomm-1.0 -I$(mosek_home)/h
#check_PROGRAMS = rectangleoverlap block satisfy_inc #csolve_VPSC #cycle
check_PROGRAMS = satisfy_inc
satisfy_inc_SOURCES = satisfy_inc.cpp
satisfy_inc_LDADD = $(top_builddir)/libvpsc/libvpsc.a -L$(mosek_home)/bin -lmosek -lguide -limf -lirc
block_SOURCES = block.cpp
block_LDADD = $(top_builddir)/libvpsc/libvpsc.a
rectangleoverlap_SOURCES = rectangleoverlap.cpp
rectangleoverlap_LDADD = $(top_builddir)/libvpsc/libvpsc.a
csolve_VPSC_SOURCES = csolve_VPSC.c
csolve_VPSC_LDADD = $(top_builddir)/libvpsc/libvpsc.a
cycle_SOURCES = cycle.cpp
cycle_LDADD = $(top_builddir)/libvpsc/libvpsc.a

TESTS = $(check_PROGRAMS)

