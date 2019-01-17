#!/usr/bin/python
"""
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

This Python script generates the file bendseqlookup.cpp.

It is included so that it can be understood where the lookup table for
minimal-length bend sequences comes from, and for the sake of future
development.

The lookup table gives the bend sequences that can occur on bend-minimal
paths from node A to node Z, leaving A in direction d0 and entering Z in
direction d1.

Note a subtlety here: Whereas the direction in which you depart from node A
is the same as the side of A from which you depart, these two notions (direction
of travel, and side of incidence) are always in opposition with the node Z at
the end of the path.

For example, if you enter node Z while traveling east, you will enter it
from its west side. The direction d1 would be east in this example. That is,
the direction d1 is the direction in which you are taveling, NOT the side
of Z at which you enter.

In order to use the table you must also give the compass direction c from
node Z to node A. Here, c is cardinal if A and Z are aligned, else ordinal.

All bend-minimal sequences were determined and written by hand for the case
in which d1 == EAST, and the results are recorded in the eastFinalLookup
dictionary below.

There, the codes r, g, j, n stand, respectively, for the TLC, TRC, BRC, and
BLC bend shapes. (Lowercase 'r' and 'j' look roughly like the TLC and BRC bends;
in the Korean Hangul alphabet, the letters for the 'g' and 'n' sounds look
like the TRC and BLC bends.)

As for the compass direction c from node Z to node A, we replace this with a
code number p in {0, 1, ... 8}, where these integers represent the positions
in the following grid:
           ___ ___ ___
          | 0 | 1 | 2 |
          `--- --- ---'
          | 3 | 4 | 5 |
          `--- --- ---'
          | 6 | 7 | 8 |
          `--- --- ---'
The idea is that Z sits at position 4, and p will indicate at which position A
sits, where

    p == 0 means A.x < Z.x ^ A.y < Z.y,
    p == 1 means A.x == Z.x ^ A.y < Z.y,
    ...
    p == 8 means A.x > Z.x ^ A.y > Z.y.

HOWEVER: It turns out the the bend sequences for A at position 1 are identical
with those for A at position 2; likewise pos 4 is equivalent to pos 5, and pos
7 is equivalent to pos 8. If you are wondering how node A could be at position
4 (doesn't that mean it lies right on top of node Z?), remember that e.g. in a
figure-8 network, you can have a chain which is not a cycle but which
nevertheless has the same node at each end. I.e. A and Z can be the same node.

Because the 2-5-8 column works the same as the 1-4-7 column in the table, the
eastFinalLookup in fact has no entries for positions 1, 4, 7.  For these we
simply consult 2, 5, and 8, respectively.

In order to utilise the eastFinalLookup in generating the FULL lookup table for
C++, we use the lookupMinimalBendSeqs function. This essentially achieves an
algebraic conjugation, rotating the input values to conform to d1 being EAST, then
un-rotating the results before returning them, to match the case that was actually
requested.

The final C++ table is accessed using

                    minimalBendSeqs.at(c).at(d0).at(d1)

in order to obtain a vector of vectors, giving all possible bend-minimal bend sequences
for the situation in question. You always get at least one sequence, but that sequence
itself may be empty, if in fact the best path has zero bends.
"""

E, S, W, N = 0, 1, 2, 3
r, g, j, n = 0, 3, 5, 2

eastFinalLookup = {
    0: {
        E: [[g,n]],
        S: [[n]],
        W: [[r,n]],
        N: [[r,g,n], [g,r,n]]
    },
    2: {
        E: [[j,g,r,n], [g,j,r,n], [g,j,n,r]],
        S: [[j,r,n], [j,n,r]],
        W: [[r,n]],
        N: [[g,r,n]]
    },
    3: {
        E: [[]],
        S: [[n,j,r]],
        W: [[n,r,g,n], [r,n,j,r]],
        N: [[r,g,n]]
    },
    5: {
        E: [[j,g,r,n], [g,j,n,r]],
        S: [[j,n,r]],
        W: [[n,g,r,n], [r,j,n,r]],
        N: [[g,r,n]]
    },
    6: {
        E: [[j,r]],
        S: [[n,j,r], [j,n,r]],
        W: [[n,r]],
        N: [[r]]
    },
    8: {
        E: [[g,j,n,r], [j,g,n,r], [j,g,r,n]],
        S: [[j,n,r]],
        W: [[n,r]],
        N: [[g,n,r], [g,r,n]]
    }
}

def lookupMinimalBendSeqs(p, d0, d1):
    """
    :param p: The square (0 through 8) in which node A (start of path) sits
    :param d0: Compass direction in which to depart from A
    :param d1: Compass direction in which to enter Z (end of path)

    :return: a list [s0, s1, ..., sk-1] where each si is a list of LinkShapes.
             These are all and only the sequences of bends that can occur on
             a bend-minimal orthogonal route from node A to node Z, with the
             prescribed departure and arrival directions d0 and d1.

             You always get at least one si, but it itself may be empty (meaning
             that the best path has zero bends).
    """
    pMap = {
        E: [0, 2, 2, 3, 5, 5, 6, 8, 8],
        S: [6, 3, 0, 8, 5, 2, 8, 5, 2],
        W: [8, 8, 6, 5, 5, 3, 2, 2, 0],
        N: [2, 5, 8, 2, 5, 8, 0, 3, 6]
    }[d1]
    d0Map = {
        E: [0, 1, 2, 3],
        S: [3, 0, 1, 2],
        W: [2, 3, 0, 1],
        N: [1, 2, 3, 0]
    }[d1]
    bendMap = {
        E: {r:r, g:g, j:j, n:n},
        S: {r:g, g:j, j:n, n:r},
        W: {r:j, g:n, j:r, n:g},
        N: {r:n, g:r, j:g, n:j}
    }[d1]
    return [
        [ bendMap[b] for b in seq ]
        for seq in eastFinalLookup[pMap[p]][d0Map[d0]]
    ]

pDirs = "NW NORTH NE WEST GLORIOSKYZERO! EAST SW SOUTH SE".split()

dirs = "EAST SOUTH WEST NORTH".split()

bends = "TLC H BLC TRC V BRC".split()

def genLookup():
    t = 'const map<CompassDir, map<CardinalDir, map<CardinalDir, vector<vector<LinkShape>>>>> dialect::minimalBendSeqs{\n'
    for p in range(9):
        if p == 4: continue  # We skip this case since there is no compass dir for position 4 in the table!
        t += (' ' * 4) + '{CompassDir::%s, {\n' % pDirs[p]
        for d0 in range(4):
            t += (' ' * 8) + '{CardinalDir::%s, {\n' % dirs[d0]
            for d1 in range(4):
                t += (' ' * 12) + '{CardinalDir::%s, {\n' % dirs[d1]
                seqs = lookupMinimalBendSeqs(p, d0, d1)
                m = len(seqs)
                for i, seq in enumerate(seqs):
                    t += (' ' * 16) + '{'
                    t += ', '.join(['LinkShape::%s' % bends[j] for j in seq])
                    t += '}' + (',' if i < m-1 else '') + '\n'
                t += (' ' * 12) + '}}' + (',' if d1 < 3 else '') + '\n'
            t += (' ' * 8) + '}}' + (',' if d0 < 3 else '') + '\n'
        t += (' ' * 4) + '}}' + (',' if p < 8 else '') + '\n'
    t += '};\n'
    return t

CPP_HEADER = """\
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

/*
 * Generated by gen_bend_seq_lookup.py
 *
*/

#include <map>
#include <vector>

#include "libdialect/ortho.h"
#include "libdialect/chains.h"

using dialect::CompassDir;
using dialect::CardinalDir;
using dialect::LinkShape;
using std::map;
using std::vector;

"""

def test1():
    print genLookup()

def writeCppModule():
    code = ''
    code += CPP_HEADER
    code += genLookup()
    with open('bendseqlookup.cpp', 'w') as f:
        f.write(code)

if __name__=='__main__':
    #test1()
    writeCppModule()
