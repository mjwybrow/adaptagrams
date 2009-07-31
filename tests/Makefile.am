INCLUDES = -I$(top_srcdir)

LDADD = $(top_builddir)/libavoid/libavoid.a

check_PROGRAMS = example complex multiconnact inline infinity latesetup \
	inlineoverlap nudgeintobug activedelete slowrouting

example_SOURCES = example.cpp

multiconnact_SOURCES = multiconnact.cpp

complex_SOURCES = complex.cpp

inline_SOURCES = inline.cpp

infinity_SOURCES = infinity.cpp

latesetup_SOURCES = latesetup.cpp

inlineoverlap_SOURCES = inlineoverlap.cpp

nudgeintobug_SOURCES = nudgeintobug.cpp

activedelete_SOURCES = activedelete.cpp

slowrouting_SOURCES = slowrouting.cpp

TESTS = $(check_PROGRAMS)

