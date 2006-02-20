INCLUDES = -I$(top_srcdir)/removeoverlap
check_PROGRAMS = test_rectangleoverlap test_csolve_VPSC
test_rectangleoverlap_SOURCES = test_rectangleoverlap.cpp
test_rectangleoverlap_LDADD = $(top_srcdir)/removeoverlap/libsolve_VPSC.la
test_csolve_VPSC_SOURCES = test_csolve_VPSC.c
test_csolve_VPSC_LDADD = $(top_srcdir)/removeoverlap/libsolve_VPSC.la

TESTS = $(check_PROGRAMS)

