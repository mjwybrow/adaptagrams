AM_CPPFLAGS = -I$(top_srcdir) $(CAIROMM_CFLAGS)
LDADD = $(top_builddir)/libtopology/libtopology.la $(top_builddir)/libcola/libcola.la $(top_builddir)/libvpsc/libvpsc.la $(top_builddir)/libavoid/libavoid.la $(CAIROMM_LIBS)
check_PROGRAMS = simple_bend nooverlap nodedragging beautify orthogonalOpt 
# triangle split kamada nooverlap

simple_bend_SOURCES = simple_bend.cpp 
nooverlap_SOURCES = nooverlap.cpp 
nodedragging_SOURCES = nodedragging.cpp 
#problem_SOURCES = problem.cpp
beautify_SOURCES = beautify.cpp
orthogonalOpt_SOURCES = orthogonalOpt.cpp

#shortest_paths_LDADD = $(top_builddir)/libcola/libcola.la

#triangle_LDADD = $(ldcommon)
#triangle_SOURCES = triangle.cpp 
#split_LDADD = $(ldcommon)
#split_SOURCES = split.cpp 
#kamada_LDADD = $(ldcommon)
#kamada_SOURCES = kamada.cpp 
TESTS = $(check_PROGRAMS)
