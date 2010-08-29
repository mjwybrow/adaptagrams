INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES = libavoid.la


libavoid_la_SOURCES = connectionpin.cpp \
			connector.cpp \
			connend.cpp \
			geometry.cpp \
			geomtypes.cpp \
			graph.cpp \
			junction.cpp \
			makepath.cpp \
			obstacle.cpp \
			orthogonal.cpp \
			router.cpp \
			shape.cpp \
			timer.cpp \
			vertices.cpp \
			viscluster.cpp \
			visibility.cpp \
			vpsc.cpp \
			libavoid.h

libavoidincludedir = ${includedir}/libavoid
libavoidinclude_HEADERS = assertions.h \
			connector.h \
			connectionpin.h \
			connend.h \
			debug.h \
			geometry.h \
			geomtypes.h \
			graph.h \
			junction.h \
			libavoid.h \
			makepath.h \
			obstacle.h \
			orthogonal.h \
			router.h \
			shape.h \
			timer.h \
			vertices.h \
			viscluster.h \
			visibility.h \
			vpsc.h

SUBDIRS = . tests

