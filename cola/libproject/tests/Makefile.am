#INCLUDES = -I$(top_srcdir)
INCLUDES = -I$(top_srcdir) -Iquadprogpp
LIBPROJECT = $(top_builddir)/libproject/libproject.a
check_PROGRAMS = quadprogpptest simple static random
testutil_SOURCES = testutil.cpp quadprogpp/QuadProg++.cc
simple_SOURCES = simple.cpp
simple_LDADD = $(LIBPROJECT)
static_SOURCES = static.cpp $(testutil_SOURCES)
static_LDADD = $(LIBPROJECT)
random_SOURCES = random.cpp $(testutil_SOURCES)
random_LDADD = $(LIBPROJECT)
quadprogpptest_SOURCES = quadprogpp/main.cc\
			 quadprogpp/QuadProg++.hh\
		     	 quadprogpp/QuadProg++.cc

TESTS = $(check_PROGRAMS)

