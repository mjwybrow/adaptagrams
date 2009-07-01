INCLUDES = -I$(top_srcdir) 
lib_LIBRARIES = libvpsc.a
#DEFS=-DLIBVPSC_LOGGING

libvpsc_a_SOURCES = block.cpp\
	blocks.cpp\
	constraint.cpp\
	rectangle.cpp\
	solve_VPSC.cpp\
	variable.cpp\
	cbuffer.cpp\
	isnan.h\
	block.h\
	blocks.h\
	constraint.h\
	rectangle.h\
	pairingheap/PairingHeap.h\
	solve_VPSC.h\
	variable.h\
	cbuffer.h\
	linesegment.h

libvpscincludedir = ${includedir}/libvpsc

libvpscinclude_HEADERS = solve_VPSC.h \
	block.h\
	constraint.h\
	exceptions.h\
	rectangle.h\
	variable.h

SUBDIRS = tests
