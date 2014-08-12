EXTRA_DIST=libvpsc.pc.in
lib_LTLIBRARIES = libvpsc.la
libvpsc_la_CPPFLAGS = -I$(top_srcdir) -I$(includedir)/libvpsc -fPIC
libvpsc_la_LDFLAGS = -no-undefined

#DEFS=-DLIBVPSC_LOGGING


libvpsc_la_SOURCES = block.cpp\
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
	pairingheap.h\
	solve_VPSC.h\
	variable.h\
	cbuffer.h\
	linesegment.h\
	assertions.h

libvpscincludedir = $(includedir)/libvpsc

libvpscinclude_HEADERS = solve_VPSC.h \
	block.h\
	constraint.h\
	exceptions.h\
	rectangle.h\
	variable.h \
	assertions.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libvpsc.pc

SUBDIRS = . tests

