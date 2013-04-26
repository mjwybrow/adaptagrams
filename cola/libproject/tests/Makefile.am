AM_CPPFLAGS = -I$(top_srcdir) -Iquadprogpp
LIBPROJECT = $(top_builddir)/libproject/libproject.la
check_PROGRAMS = treeqptest quadprogpptest simple static random
testutil_SOURCES = testutil.cpp quadprogpp/QuadProg++.cc
simple_SOURCES = simple.cpp
simple_LDADD = $(LIBPROJECT)
static_SOURCES = static.cpp $(testutil_SOURCES)
static_LDADD = $(LIBPROJECT)
random_SOURCES = random.cpp $(testutil_SOURCES)
random_LDADD = $(LIBPROJECT)
treeqptest_SOURCES = treeqptest.cpp treeqp.cpp $(testutil_SOURCES)
treeqptest_LDADD = $(LIBPROJECT)
quadprogpptest_SOURCES = quadprogpp/main.cc\
			 quadprogpp/QuadProg++.h\
		     	 quadprogpp/QuadProg++.cc

TESTS = $(check_PROGRAMS)

