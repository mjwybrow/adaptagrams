INCLUDES = -I$(top_srcdir)

LDADD = $(top_builddir)/libavoid/libavoid.a

check_PROGRAMS = example complex multiconnact inline infinity latesetup \
	inlineoverlap01 inlineoverlap02 inlineoverlap03 nudgeintobug \
	activedelete slowrouting

example_SOURCES = example.cpp

multiconnact_SOURCES = multiconnact.cpp

complex_SOURCES = complex.cpp

inline_SOURCES = inline.cpp

infinity_SOURCES = infinity.cpp

latesetup_SOURCES = latesetup.cpp

inlineoverlap01_SOURCES = inlineoverlap01.cpp
inlineoverlap02_SOURCES = inlineoverlap02.cpp
inlineoverlap03_SOURCES = inlineoverlap03.cpp

nudgeintobug_SOURCES = nudgeintobug.cpp

activedelete_SOURCES = activedelete.cpp

slowrouting_SOURCES = slowrouting.cpp

TESTS = $(check_PROGRAMS)

