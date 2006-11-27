#INCLUDES = -I$(top_srcdir)

lib_LIBRARIES = libvpsc.a

libvpsc_a_SOURCES = block.cpp\
	blocks.cpp\
	constraint.cpp\
	rectangle.cpp\
	pairingheap/PairingHeap.cpp\
	remove_rectangle_overlap.cpp\
	solve_VPSC.cpp\
	csolve_VPSC.cpp\
	variable.cpp\
	cbuffer.cpp\
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
	variable.h\
	cbuffer.h

libvpscincludedir = ${includedir}/libvpsc

libvpscinclude_HEADERS = solve_VPSC.h \
	block.h\
	constraint.h\
	csolve_VPSC.h\
	rectangle.h\
	remove_rectangle_overlap.h\
	variable.h

SUBDIRS = tests
