INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES = libavoid.la


libavoid_la_SOURCES = connector.cpp \
			connend.cpp \
			geometry.cpp \
			vertices.cpp \
			graph.cpp \
			makepath.cpp \
			router.cpp \
			shape.cpp \
			viscluster.cpp \
			timer.cpp \
			visibility.cpp \
			geomtypes.cpp \
			orthogonal.cpp \
			vpsc.cpp \
			connectionpin.cpp \
			junction.cpp \
			libavoid.h

libavoidincludedir = ${includedir}/libavoid
libavoidinclude_HEADERS = assertions.h \
			connector.h \
			connend.h \
			geomtypes.h \
			makepath.h \
			router.h \
			timer.h \
			debug.h \
			graph.h \
			shape.h \
			vertices.h \
			geometry.h \
			libavoid.h \
			visibility.h \
			viscluster.h \
			orthogonal.h \
			connectionpin.h \
			junction.h \
			vpsc.h

SUBDIRS = . tests

