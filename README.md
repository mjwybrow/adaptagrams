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
 *  [libavoid][libavoid] 
    	      - a cross-platform C++ library providing fast, polyline and
    		orthogonal object-avoiding connector routing for use in 
		interactive diagram editors.  

These libraries are collectively known as cola (for Constraint Layout). The
newest version of the C++ source code for cola can be found in the 
Adaptagrams github repository:

    https://github.com/mjwybrow/adaptagrams/

All code in the Adaptagrams repository is released as open source under the
terms of the LGPL 2.1 or later, see the LICENSE file. Software using the 
above libraries include: Inkscape, Graphviz and Dunnart.

The Adaptagrams code is maintained by [Michael Wybrow][mw].

The algorithms were developed by [Kim Marriott][km]'s Adaptive Diagrams Research Group at [Monash University][monash] in Melbourne, Australia.  The Adaptagrams libraries were originally written by [Tim Dwyer][td] and [Michael Wybrow][mw].


Building
--------

The library code is all contained in the `cola` directory of the repository.

We use GNU `automake` to build.  We've tried to make the contents of the
repository as platform agnostic as possible, so you'll need to call `aclocal`,
`autoconf`, and `automake` before `configure`.
 
A somewhat transient dependency (depending on whether `libcola/output_svg.cpp` 
is included in the build) is [Cairo](http://cairographics.org/).  It's only
used by a few examples so you can easily build the core libraries without it.

Run `./autogen.sh` to compile from scratch.


[td]: http://www.csse.monash.edu.au/~tdwyer/
[km]: http://www.csse.monash.edu.au/~kmarriott/
[mw]: http://www.csse.monash.edu.au/~mwybrow/
[monash]: http://wwww.csse.monash.edu.au/
[libavoid]: http://adaptagrams.sourceforge.net/libavoid/
