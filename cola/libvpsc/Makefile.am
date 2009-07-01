#mosek_home=/home/dwyer/devel/mosek/5/tools/platform/linux32x86
INCLUDES = -I$(top_srcdir) #-I$(mosek_home)/h
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
	mosek_quad_solve.h\
	mosek_quad_solve.c\
	linesegment.h

libvpscincludedir = ${includedir}/libvpsc

libvpscinclude_HEADERS = solve_VPSC.h \
	block.h\
	constraint.h\
	exceptions.h\
	rectangle.h\
	variable.h

SUBDIRS = tests
