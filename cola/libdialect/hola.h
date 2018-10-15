/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libdialect - A library for computing DiAlEcT layouts:
 *                 D = Decompose/Distribute
 *                 A = Arrange
 *                 E = Expand/Emend
 *                 T = Transform
 *
 * Copyright (C) 2018  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author(s):   Steve Kieffer   <http://skieffer.info>
*/

#ifndef DIALECT_HOLA_H
#define DIALECT_HOLA_H

#include <string>
#include <vector>

#include "libdialect/commontypes.h"
#include "libdialect/opts.h"
#include "libdialect/logging.h"

namespace dialect {

//! @brief  Apply the HOLA layout algorithm to the given Graph.
//!         See Steve Kieffer, Tim Dwyer, Kim Marriott, and Michael Wybrow.
//!         HOLA: Human-like Orthogonal Network Layout.
//!         IEEE Transactions on Visualization and Computer Graphics 22, no. 1 (2016): 349-358.
//! @param[in, out] G  The Graph to be laid out. Node positions are updated in-place. Constraints
//!                    are set in the Graph's SepMatrix.
//! @param[in] opts  Options controlling the layout.
//! @param[out] logger  Optional pointer to a Logger in which to record TGLF for various stages
//!                     of the layout process. Useful for debugging.
void doHOLA(dialect::Graph &G, const dialect::HolaOpts &holaOpts, dialect::Logger *logger = nullptr);

//! @brief  Convenience function to do HOLA layout with default options.
//! @param[in, out] G  The Graph to be laid out. Node positions are updated in-place. Constraints
//!                    are set in the Graph's SepMatrix.
void doHOLA(dialect::Graph &G);


} // namespace dialect

#endif // DIALECT_HOLA_H
