/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2009  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the 
 * library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * Author(s):   Michael Wybrow <mjwybrow@users.sourceforge.net>
*/

//! @file  libavoid.h
//! @brief Standard libavoid include file which includes all libavoid 
//!        header files.


//! @mainpage 
//!
//! <h2>libavoid Information</h2>
//!
//! libavoid is a cross-platform C++ library providing fast, object-avoiding 
//! orthogonal and polyline connector routing for use in interactive diagram 
//! editors.
//!
//! libavoid is part of the 
//! <a href="http://adaptagrams.sourceforge.net/">Adaptagrams project</a>. 
//! There are no official releases yet, though the code is stable and 
//! available from the Adaptagrams 
//! <a href="http://sourceforge.net/scm/?type=svn&group_id=154331">SVN 
//! repository</a>. 
//!
//! libavoid is written and maintained by Michael Wybrow, a member of the 
//! Adaptive Diagrams and Documents lab at Monash University, Australia.
//!
//! The algorithms used for the connector routing are described in:
//! -  M. Wybrow, K. Marriott, and P.J. Stuckey. Orthogonal connector 
//!    routing.<br />
//!    In Proceedings of 17th International Symposium on Graph Drawing 
//!    (GD '09),<br /> 
//!    To appear 2010. 
//! -  M. Wybrow, K. Marriott, and P.J. Stuckey. Incremental connector 
//!    routing.<br />
//!    In Proceedings of 13th International Symposium on Graph Drawing 
//!    (GD '05),<br />
//!    LNCS 3843, pages 446—457. Springer-Verlag, 2006. 
//!    [<a href="http://dx.doi.org/10.1007/11618058_40">DOI</a>]
//! 
//! libavoid is currently used in the following software:
//! -  Dunnart, a prototype research constraint-based diagram editor, and
//! -  Inkscape, the popular Open Source vector graphics editor.

//! @namespace Avoid
//! @brief The namespace used by code in the libavoid library.

#ifndef AVOID_LIBAVOID_H
#define AVOID_LIBAVOID_H

#include "libavoid/geomtypes.h"
#include "libavoid/connector.h"
#include "libavoid/graph.h"
#include "libavoid/debug.h"
#include "libavoid/timer.h"
#include "libavoid/makepath.h"
#include "libavoid/vertices.h"
#include "libavoid/visibility.h"
#include "libavoid/router.h"

#endif


