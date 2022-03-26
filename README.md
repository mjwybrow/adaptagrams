Adaptagrams
===========

Adaptagrams is a library of tools and reusable code for adaptive diagramming
applications, for example: drawing tools, automated document and diagram
layout, smart presentation software, graph drawing, chart layout, etc.

Currently, the [Adaptagrams repository][repo] includes five cross-platform C++ 
libraries:

 *  [libvpsc][libvpsc]  
              - a solver for the Variable Placement with Separation 
		Constraints problem. This is a quadratic programming 
		problem in which the squared differences between a 
		placement vector and some ideal placement are minimised 
		subject to a set of separation constraints. This is very 
		useful in a number of layout problems.
 *  [libcola][libcola]  
    	      - a library for constraint graph layout.  Specifically, 
    		force-directed layout using the stress-majorization 
		method subject to separation constraints. Applications 
		include layout with non-overlapping nodes and clusters, 
		directed graph layout and layout preserving the crossing 
		properties of a given starting layout.  
	      - libcola depends on libvpsc.
 *  [libavoid][libavoid]  
    	      - a library providing high-quality object-avoiding polyline 
		and orthogonal connector routing for use in interactive 
		diagram editors. 
 *  [libtopology][libtopology]  
              - a library containing extensions to libcola to support 
		topology preserving constraint-based layout.  
	      - libtopology depends on libavoid, libcola and libvpsc.
 *  [libdialect][libdialect]  
              - a library for computing human-like orthogonal network
	        (DiAlEcT) layouts via the following steps: 
		D = Decompose/Distribute; A = Arrange; 
                E = Expand/Emend; and T = Transform.  
	      - libdialect depends on libavoid, libcola and libvpsc.

These libraries are collectively known as cola (for Constraint Layout). The
newest version of the C++ source code for cola can be found in the 
Adaptagrams GitHub repository maintained by [Michael Wybrow][mw]:

 *  [https://github.com/mjwybrow/adaptagrams/][repo]

The algorithms were developed by members of the [Immersive Analytics Lab][ialab] 
at [Monash University][monash] in Melbourne, Australia.  The Adaptagrams libraries 
were written by [Tim Dwyer][td], [Michael Wybrow][mw] and [Steve Kieffer][sk].

All code in the Adaptagrams repository is released as open source software
under the terms of the LGPL 2.1 or later, see the LICENSE file. 

We also dual-license the Adaptagrams libraries and for a fee we can provide
them under a less-restrictive commercial license as well as extend them to fit
your needs (contact us).  For this reason, if you contribute code to the
project and would like it to appear in the main Adaptagrams repository, we
require that you assign the copyright on your changes to Monash University with
the following statement: "I hereby assign copyright in this code to Monash
University, to be licensed under the same terms as the rest of the code."
 
Software using one or more of the Adaptagrams libraries include: 

 *  [Dunnart][dunnart], constraint-based diagram editor,
 *  [Inkscape][inkscape], the popular open source vector graphics editor,
 *  [Graphviz][graphviz], open source graph visualisation software,
 *  [Arcadia][arcadia], a visualisation tool for metabolic pathways,
 *  [Gaphas][gaphor], an open source Python-based diagramming widget for GTK+, and
 *  [BRL-CAD][brlcad], a powerful cross-platform open source solid modeling system that includes interactive geometry editing, high-performance ray-tracing for rendering and geometric analysis, image and signal-processing tools, a system performance analysis benchmark suite, libraries for robust geometric representation, with more than 20 years of active development.

Building
--------

The library code is all contained in the `cola` directory of the repository.

The only dependency is [Cairo][cairo] if debugging SVG output is to be included in several example test cases. The libraries themselves have no dependencies.

Both `cmake` and GNU `automake` can be used to build. We've tried to make the contents of the
repository as platform-agnostic as possible.

### CMake

Use common `cmake`, `make` and `make install`. Default options have values to build library as in release mode. Tests, debug logs and more can be enabled with apropriate options, see `CMakeLists.txt` of the library you want to build.

### GNU automake

You'll need to call `aclocal`,
`autoconf`, and `automake` before `configure`.

Run `./autogen.sh` to compile from scratch.

Use from other languages
------------------------

### WebAssembly

There are Javascript/WebAssembly bindings of libavoid library available [here][libavoid-js]. 

### SWIG (Python & Java)

Bindings for use of the Adaptagrams libraries can be generated using [SWIG][swig]. The repository contains a SWIG interface file `cola/adaptagrams.i`. We have successfully tested and used Adaptagrams from Java and Python in this way.

**Current state:** untested and can be buggy([one of issues][swigissue]).

### Cola in the browser


[cola.js][webcola] (a.k.a. WebCola) is a JavaScript based rewrite of libcola which works well with [D3.js][d3]

[d3]: https://d3js.org/
[webcola]: https://ialab.it.monash.edu/webcola/
[swig]: http://www.swig.org/
[td]: https://users.monash.edu/~tdwyer/
[mw]: https://users.monash.edu/~mwybrow/
[sk]: https://skieffer.info/
[ialab]: http://ialab.it.monash.edu/
[monash]: http://wwww.csse.monash.edu.au/
[libvpsc]: http://www.adaptagrams.org/documentation/libvpsc.html
[libavoid]: http://www.adaptagrams.org/documentation/libavoid.html
[libcola]: http://www.adaptagrams.org/documentation/libcola.html
[libtopology]: http://www.adaptagrams.org/documentation/libtopology.html
[libdialect]: http://www.adaptagrams.org/documentation/libdialect.html
[dunnart]: http://users.monash.edu/~mwybrow/dunnart/
[inkscape]: http://www.inkscape.org/
[graphviz]: http://www.graphviz.org/
[arcadia]: http://arcadiapathways.sourceforge.net/
[gaphor]: http://gaphor.sourceforge.net/
[cairo]: http://cairographics.org/
[repo]: https://github.com/mjwybrow/adaptagrams/
[brlcad]: http://brlcad.org/
[libavoid-js]: https://github.com/Aksem/libavoid-js
[swigissue]: https://github.com/mjwybrow/adaptagrams/issues/50
