srcdir = .
VPATH = .

cola_includedir = $(srcdir)/libcola
cola_libdir = $(srcdir)/libcola
vpsc_libdir = libvpsc/.libs


all: draw-bn

CPP_FLAGS=`pkg-config --cflags pangoft2 gtkmm-2.4 glib-2.0 libxml-2.0 pangocairo` -g
CPP_LN_FLAGS=`pkg-config --cflags --libs pangoft2 gtkmm-2.4 glib-2.0 libxml-2.0 pangocairo` -g 

EXTRA_CPPFLAGS:=$(shell pkg-config --cflags gtkmm-2.4) \
		$(shell pkg-config --cflags cairo) 

EXTRA_LDFLAGS:=$(shell pkg-config --libs cairomm-1.0 gtkmm-2.4 libxml-2.0) -L$(cola_libdir) -L$(vpsc_libdir) -L. -lcola libvpsc/libvpsc.a


CXX=g++
CXXFLAGS=-g -Wall -Wextra -Wno-unused-parameter -Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch
CPPFLAGS=-I$(srcdir) -I$(lib2geomdir) -I$(cola_includedir) `pkg-config --cflags cairomm-1.0 pangoft2 gtkmm-2.4 glib-2.0 libxml-2.0 pangocairo`


TARGETOBJS=draw-bn.o
BADTARGETOBJS=

LIBOBJS=

EXTRAOBJS=graphlayouttest.o $(TARGETOBJS)

TARGETS=$(TARGETOBJS:.o=)

ALLOBJS=$(LIBOBJS) $(EXTRAOBJS)

# Dependency magic.
%o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $*.cpp -o $*.o
	@$(CXX) -MM $(CPPFLAGS) $*.cpp > $*.d
	@cp -f $*.d $*.d.tmp 
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# Grab dependencies where they exist.
-include $(ALLOBJS:.o=.d)

$(EXTRAOBJS): CPPFLAGS:=$(EXTRA_CPPFLAGS) $(CPPFLAGS)


all: $(TARGETS)

draw-bn: draw-bn.o graphlayouttest.o

libscroll.a: $(LIBOBJS)
	ar cr libscroll.a $(LIBOBJS)
	ranlib libscroll.a


$(TARGETS):

$(TARGETS): %: %.o

# Targets;

$(TARGETS):
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(EXTRA_LDFLAGS)

.PHONY: all clean clobber mkdirs
clean:
	rm -f $(ALLOBJS) *~

clobber: clean
	rm -f $(TARGETS)

mkdirs:
	[ -d non-uniform ] || mkdir non-uniform
	[ -e texts ] || ( [ -e $(srcdir)/texts ] && ln -s $(srcdir)/texts texts )
	[ -e pages ] || ( [ -e $(srcdir)/pages ] && ln -s $(srcdir)/pages pages )


bb: bb-constraint.cc
	g++ -g -o bb bb-constraint.cc -laffa
