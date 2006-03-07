INCLUDES = -I$(top_srcdir)/removeoverlap
check_PROGRAMS = test_rectangleoverlap test_csolve_VPSC test_block test_satisfy_inc
test_satisfy_inc_SOURCES = test_satisfy_inc.cpp
test_satisfy_inc_LDADD = $(top_srcdir)/removeoverlap/libsolve_VPSC.la
test_block_SOURCES = test_block.cpp
test_block_LDADD = $(top_srcdir)/removeoverlap/libsolve_VPSC.la
test_rectangleoverlap_SOURCES = test_rectangleoverlap.cpp
test_rectangleoverlap_LDADD = $(top_srcdir)/removeoverlap/libsolve_VPSC.la
test_csolve_VPSC_SOURCES = test_csolve_VPSC.c
test_csolve_VPSC_LDADD = $(top_srcdir)/removeoverlap/libsolve_VPSC.la

TESTS = $(check_PROGRAMS)

