#INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES = libvpsc.la

libvpsc_la_SOURCES = block.cpp\
	blocks.cpp\
	constraint.cpp\
	rectangle.cpp\
	pairingheap/PairingHeap.cpp\
	remove_rectangle_overlap.cpp\
	solve_VPSC.cpp\
	csolve_VPSC.cpp\
	variable.cpp\
	isnan.h\
	block.h\
	blocks.h\
	constraint.h\
	rectangle.h\
	pairingheap/PairingHeap.h\
	pairingheap/dsexceptions.h\
	remove_rectangle_overlap.h\
	solve_VPSC.h\
	csolve_VPSC.h\
	variable.h

include_HEADERS = solve_VPSC.h \
	variable.h
	constraint.h\
	rectangle.h\
	remove_rectangle_overlap.h\
	csolve_VPSC.h

SUBDIRS = tests
