#!/bin/sh -xe
#
# We use gnu automake to build.  I've tried to make the contents of the
# repository as platform agnostic as possible, so you'll need to call
# autoreconf before configure.

# Make m4 directory, otherwise autoreconf fails when it doesn't exist.
mkdir -p m4

autoreconf --install --verbose

# The following sequence gives the manual steps if autoreconf isn't available.
#
#aclocal $ACLOCAL_FLAGS
#glibtoolize
#autoheader
#autoconf
#automake -a --add-missing


# Configure.
./configure
# Compile with CXXFLAGS="-std=c++11" when using g++ 14 or newer
# ./configure CXXFLAGS="-std=c++11"

# Instead, use this line if building for SWIG Java or SWIG Python 3:
# ./configure CPPFLAGS="-DUSE_ASSERT_EXCEPTIONS"
# Compile with CXXFLAGS="-std=c++11" when using g++ 14 or newer
# ./configure CXXFLAGS="-std=c++11" CPPFLAGS="-DUSE_ASSERT_EXCEPTIONS"

# Instead, use this line if building for SWIG Python:
# (this line doesn't seem to work anymore, it's causing problems in particular with
# libdialect and "-O3 -DNDEBUG")
# ./configure CXXFLAGS="-O3 -DNDEBUG -arch x86_64 -arch i386" LDFLAGS="-arch x86_64 -arch i386"

# Instead, use this line for development and for debugging the tests:
#./configure --disable-shared CXXFLAGS="-g -O0"

make

# The following builds examples and tests for each of the libraries.
# Please look at these before sending me questions.
make check

