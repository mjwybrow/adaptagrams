#mosek_home=/home/dwyer/devel/mosek/5/tools/platform/linux32x86
#INCLUDES = -I$(top_srcdir) -I$(mosek_home)/h

INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES = libproject.la

libproject_la_SOURCES = project.cpp \
			util.h \
			util.cpp

libprojectincludedir = ${includedir}/libproject

libprojectinclude_HEADERS = project.h \
	util.h

SUBDIRS = . tests
