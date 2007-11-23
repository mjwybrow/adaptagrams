#mosek_home=/home/dwyer/devel/mosek/5/tools/platform/linux32x86
#INCLUDES = -I$(top_srcdir) -I$(mosek_home)/h
lib_LIBRARIES = libproject.a

libproject_a_SOURCES = feasible_projection_algorithm.cpp \
		       util.cpp

libprojectincludedir = ${includedir}/libproject

libprojectinclude_HEADERS = project.h \
	feasible_projection_algorithm.h \
	util.h

SUBDIRS = tests
