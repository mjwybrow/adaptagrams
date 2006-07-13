INCLUDES = -I$(top_srcdir)/libvpsc
check_PROGRAMS = rectangleoverlap csolve_VPSC block satisfy_inc
satisfy_inc_SOURCES = satisfy_inc.cpp
satisfy_inc_LDADD = $(top_srcdir)/libvpsc/libvpsc.la
block_SOURCES = block.cpp
block_LDADD = $(top_srcdir)/libvpsc/libvpsc.la
rectangleoverlap_SOURCES = rectangleoverlap.cpp
rectangleoverlap_LDADD = $(top_srcdir)/libvpsc/libvpsc.la
csolve_VPSC_SOURCES = csolve_VPSC.c
csolve_VPSC_LDADD = $(top_srcdir)/libvpsc/libvpsc.la

TESTS = csolve_VPSC $(check_PROGRAMS)

