EXTRA_DIST=libdialect.pc.in
lib_LTLIBRARIES = libdialect.la
libdialect_la_CPPFLAGS = -I$(top_srcdir) -I$(includedir)/libdialect -fPIC
libdialect_la_LDFLAGS = -no-undefined

# Depends on libavoid, libcola, and libvpsc
libdialect_la_LIBADD = $(top_builddir)/libavoid/libavoid.la $(top_builddir)/libcola/libcola.la $(top_builddir)/libvpsc/libvpsc.la

libdialect_la_SOURCES = \
    aca.cpp \
    aca.h \
    bendseqlookup.cpp \
    chains.cpp \
    chains.h \
    commontypes.h \
    constraints.cpp \
    constraints.h \
    edges.cpp \
    expansion.cpp \
    expansion.h \
    faces.cpp \
    faces.h \
    graphs.cpp \
    graphs.h \
    hola.cpp \
    hola.h \
    io.cpp \
    io.h \
    logging.cpp \
    logging.h \
    nearalign.cpp \
    nearalign.h \
    nexes.cpp \
    nodeconfig.cpp \
    nodeconfig.h \
    nodes.cpp \
    opts.h \
    ortho.cpp \
    ortho.h \
    peeling.cpp \
    peeling.h \
    planarise.cpp \
    planarise.h \
    qalookup.cpp \
    quadaction.cpp \
    quadaction.h \
    routing.cpp \
    routing.h \
    sides.cpp \
    treeplacement.cpp \
    treeplacement.h \
    trees.cpp \
    trees.h \
    util.h

libdialectincludedir = $(includedir)/libdialect

libdialectinclude_HEADERS = \
    aca.h \
    chains.h \
    commontypes.h \
    constraints.h \
    expansion.h \
    faces.h \
    graphs.h \
    hola.h \
    io.h \
    logging.h \
    nearalign.h \
    nodeconfig.h \
    opts.h \
    ortho.h \
    peeling.h \
    planarise.h \
    quadaction.h \
    routing.h \
    treeplacement.h \
    trees.h \
    util.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdialect.pc

SUBDIRS = . tests
