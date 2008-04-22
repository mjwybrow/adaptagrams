# We use gnu automake to build.  I've tried to make the contents of the
# repository as platform agnostic as possible, so you'll need to call
# aclocal, autoconf, and automake before configure.
# A somewhat transient dependency (depending on whether I include
# libcola/output_svg.cpp in the build) is Cairo (cairographics.org).
# It's only used by a few examples so you can easily build the core libraries
# without it.
# The following sequence builds from scratch:
aclocal $ACLOCAL_FLAGS
#libtoolize
autoconf
automake -a --add-missing
./configure
make
# The following builds examples in libvpsc/tests and libcola/tests.
# Please look at these before sending me questions.
make check

# Tim Dwyer - 16/03/2007 (www.csse.monash.edu.au/~tdwyer)
