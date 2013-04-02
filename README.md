Adaptagrams
===========

Adaptagrams is a library of tools and reusable code for adaptive diagramming
applications, for example: drawing tools, automated document and diagram
layout, smart presentation software, graph drawing, chart layout, etc.

Currently, the Adaptagrams repository includes:

 *  libvpsc  
              - a solver for the Variable Placement with Separation 
		Constraints problem. This is a quadratic programming 
		problem in which the squared differences between a 
		placement vector and some ideal placement are minimised 
		subject to a set of separation constraints. This is very 
		useful in a number of layout problems.
 *  libcola  
    	      - a library for constraint graph layout.  Specifically, 
    		force-directed layout using the stress-majorization 
		method subject to separation constraints. Applications 
		include layout with non-overlapping nodes and clusters, 
		directed graph layout and layout preserving the crossing 
		properties of a given starting layout.  
	      - libcola depends on libvpsc.
 *  [libavoid][libavoid]  
    	      - a cross-platform C++ library providing fast, polyline and
    		orthogonal object-avoiding connector routing for use in 
		interactive diagram editors.  
 *  libtopology  
              - a library containing extensions to libcola to support 
		topology preserving constraint-based layout.  
	      - libtopology depends on libavoid, libcola and libvpsc.

These libraries are collectively known as cola (for Constraint Layout). The
newest version of the C++ source code for cola can be found in the 
Adaptagrams github repository:

 *  [https://github.com/mjwybrow/adaptagrams/][repo]

The Adaptagrams code is maintained by [Michael Wybrow][mw].

All code in the Adaptagrams repository is released as open source under the
terms of the LGPL 2.1 or later, see the LICENSE file. 

Software using one or more of the Adaptagrams libraries include: 

 *  [Dunnart][dunnart], constraint-based diagram editor,
 *  [Inkscape][inkscape], the popular open source vector graphics editor,
 *  [Graphviz][graphviz], open source graph visualisation software,
 *  [Arcadia][arcadia], a visualisation tool for metabolic pathways,
 *  [Gaphas][gaphor], an open source Python-based diagramming widget for GTK+, and
 *  [BRL-CAD][brlcad], a powerful cross-platform open source solid modeling system that includes interactive geometry editing, high-performance ray-tracing for rendering and geometric analysis, image and signal-processing tools, a system performance analysis benchmark suite, libraries for robust geometric representation, with more than 20 years of active development.

The algorithms were developed by [Kim Marriott][km]'s Adaptive Diagrams Research Group at [Monash University][monash] in Melbourne, Australia.  The Adaptagrams libraries were originally written by [Tim Dwyer][td] and [Michael Wybrow][mw].


Building
--------

The library code is all contained in the `cola` directory of the repository.

We use GNU `automake` to build.  We've tried to make the contents of the
repository as platform agnostic as possible, so you'll need to call `aclocal`,
`autoconf`, and `automake` before `configure`.
 
A somewhat transient dependency (depending on whether `libcola/output_svg.cpp` 
is included in the build) is [Cairo][cairo].  It's only used by a few examples 
so you can easily build the core libraries without it.

Run `./autogen.sh` to compile from scratch.


[td]: http://www.csse.monash.edu.au/~tdwyer/
[km]: http://www.csse.monash.edu.au/~kmarriott/
[mw]: http://www.csse.monash.edu.au/~mwybrow/
[monash]: http://wwww.csse.monash.edu.au/
[libavoid]: http://adaptagrams.sourceforge.net/libavoid/
[dunnart]: http://www.dunnart.org/
[inkscape]: http://www.inkscape.org/
[graphviz]: http://www.graphviz.org/
[arcadia]: http://arcadiapathways.sourceforge.net/
[gaphor]: http://gaphor.sourceforge.net/
[cairo]: http://cairographics.org/
[repo]: https://github.com/mjwybrow/adaptagrams/
[brlcad]: http://brlcad.org/

