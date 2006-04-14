INCLUDES = -I$(top_srcdir)/libcola -I$(top_srcdir)/libvpsc
check_PROGRAMS = graphlayout
graphlayout_SOURCES = graphlayout.cpp
graphlayout_LDADD = $(top_srcdir)/libcola/libcola.a\
	$(top_srcdir)/libvpsc/libvpsc.la

TESTS = $(check_PROGRAMS)

