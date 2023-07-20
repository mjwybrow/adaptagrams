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

#ifndef DIALECT_IO_H
#define DIALECT_IO_H

#include <istream>
#include <string>
#include <memory>
#include <iostream>
#include <cstdio>

namespace dialect {

class Graph;

/*
 * Support for TGLF: Trivial Graph Layout Format.
 *
 * TGLF looks like this:
 *
 * // This line is a comment.
 * i0 cx0 cy0 w0 h0
 * i1 cx1 cy1 w1 h1
 * ...
 * in xn yn wn hn
 * #
 * i0 i1
 * i2 i3 a  b  c  d
 * i4 i5 a' b'
 * i6 i7
 * ...
 * #
 * j0 j1 T  D  R  G
 * j2 j3 T' D' R' G'
 * ...
 *
 * and is of the form:
 *
 * NODES
 * #
 * LINKS
 * #
 * SEPCOS
 *
 * where
 *
 * NODES defines one node per line, giving an ID as an unsigned int, followed by
 * four floats: the x and y coordinates of the centre of the node, and its width
 * and height.
 *
 * LINKS defines one link (or "edge" or "connector") per line. A line begins
 * with two node IDs, as defined in NODES. This is followed by zero or more pairs
 * of floats giving the (x, y)-coordinates of route points. The centres of the
 * source and target nodes are implicit route points, and should not be listed.
 *
 * SEPCOS defines one separation constraint per line. As with the LINKS,
 * a line begins with two node IDs. This is followed by a *gap type*,
 * a *direction*, a *relation*, and a *gap* G.
 * 
 *   gap type: a character among: B, C
 *      B: "boundary gap". The gap is between the node boundaries.
 *      C: "centre gap". The gap is between the node centres.
 * 
 *   direction: a character among: N, S, E, W, U, D, L, R, X, Y
 *      N, S, E, W: north, south, east, west. A separation, plus an alignment.
 *      U, D, L, R: up, down, left, right. Just a separation.
 *      X, Y: synonyms for R and D, respectively
 * 
 *   relation: one of the strings '>=' or '=='
 *      >=: the constraint sets a minimum gap
 *      ==: the constraint sets an exact gap
 * 
 *   gap: a float value, specifying the desired gap
 *
 * The NODES section must be nonempty, but either or both of the LINKS
 * and SEPCOS sections may be empty or omitted.
 *
*/

//! @brief  Build a Graph object from TGLF.
//!
//! @param[in]  in  An istream containing TGLF.
//!
//! @return  A Graph built on the given TGLF.
std::shared_ptr<Graph> buildGraphFromTglf(std::istream &in);

//! @brief  Build a Graph object from TGLF.
//!
//! @param[in]  in  A string containing TGLF.
//!
//! @return  A Graph built on the given TGLF.
std::shared_ptr<Graph> buildGraphFromTglf(std::string &s);

//! @brief  Build a Graph object from a file containing TGLF.
//!
//! @param[in]  filepath  Full filesystem path to a file containing TGLF.
//!
//! @return  A Graph built on the given TGLF.
std::shared_ptr<Graph> buildGraphFromTglfFile(const std::string &filepath);

//! @brief  Write a string to a file.
//!
//! @param[in]  s  the string to be written
//! @param[in]  filepath  full filesystem path to the file to be written
void writeStringToFile(const std::string &s, const std::string &filepath);

} // namespace dialect

#endif // DIALECT_IO_H
