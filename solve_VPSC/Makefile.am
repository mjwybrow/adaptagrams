INCLUDES = -I$(top_srcdir)/removeoverlap
check_PROGRAMS = test_rectangleoverlap test_csolve_VPSC
test_rectangleoverlap_SOURCES = test_rectangleoverlap.cpp
test_rectangleoverlap_LDADD = libsolve_VPSC.la
test_csolve_VPSC_SOURCES = test_csolve_VPSC.c
test_csolve_VPSC_LDADD = libsolve_VPSC.la

TESTS = $(check_PROGRAMS)

lib_LTLIBRARIES = libsolve_VPSC.la

libsolve_VPSC_la_SOURCES = removeoverlap/block.cpp\
	removeoverlap/blocks.cpp\
	removeoverlap/constraint.cpp\
	removeoverlap/generate-constraints.cpp\
	removeoverlap/pairingheap/PairingHeap.cpp\
	removeoverlap/remove_rectangle_overlap.cpp\
	removeoverlap/solve_VPSC.cpp\
	removeoverlap/csolve_VPSC.cpp\
	removeoverlap/variable.cpp\
	removeoverlap/isnan.h\
	removeoverlap/block.h\
	removeoverlap/blocks.h\
	removeoverlap/constraint.h\
	removeoverlap/generate-constraints.h\
	removeoverlap/pairingheap/PairingHeap.h\
	removeoverlap/pairingheap/dsexceptions.h\
	removeoverlap/remove_rectangle_overlap.h\
	removeoverlap/solve_VPSC.h\
	removeoverlap/csolve_VPSC.h\
	removeoverlap/variable.h

include_HEADERS = removeoverlap/solve_VPSC.h \
	removeoverlap/variable.h
	removeoverlap/constraint.h\
	removeoverlap/generate-constraints.h\
	removeoverlap/remove_rectangle_overlap.h\
	removeoverlap/csolve_VPSC.h
	

