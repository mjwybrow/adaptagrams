INCLUDES = -I$(top_srcdir)

lib_LIBRARIES = libavoid.a

libavoid_a_SOURCES = connector.cpp\
			geometry.cpp\
			vertices.cpp\
			graph.cpp\
			makepath.cpp\
			router.cpp\
			shape.cpp\
			viscluster.cpp\
			timer.cpp\
			visibility.cpp\
			geomtypes.cpp\
			orthogonal.cpp\
			vpsc.cpp\
			libavoid.h

libavoidincludedir = ${includedir}/libavoid
libavoidinclude_HEADERS = connector.h \
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
			vpsc.h

SUBDIRS = tests

