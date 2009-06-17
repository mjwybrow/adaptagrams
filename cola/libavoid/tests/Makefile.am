INCLUDES = -I$(top_srcdir)
check_PROGRAMS = example complex multiconnact inline

example_SOURCES = example.cpp
example_LDADD = $(top_builddir)/libavoid/libavoid.a 

multiconnact_SOURCES = multiconnact.cpp
multiconnact_LDADD = $(top_builddir)/libavoid/libavoid.a 

complex_SOURCES = complex.cpp
complex_LDADD = $(top_builddir)/libavoid/libavoid.a 

inline_SOURCES = inline.cpp
inline_LDADD = $(top_builddir)/libavoid/libavoid.a 

TESTS = $(check_PROGRAMS)

