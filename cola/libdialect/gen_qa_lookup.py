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

This Python script generates the file qalookup.cpp.

It is included only so that it can be understood where the lookup table
for quad actions in qalookup.cpp comes from, as well as for any future
development in which it may be desired that the quad actions work
differently.

The lookup table is formatted as a five-dimensional array. If the quadrants are
enumerated as follows:

                            |
                         Q2 | Q3
                        ----+----
                         Q1 | Q0
                            |

then let p, q, r, s be the (reduced) number of nodes in quadrants Q0, Q1, Q2,
Q3, respectively. To say the number is "reduced" means that values >= 2 are
replaced by 2 itself. Thus, the only possible values are 0, 1, and 2.

Furthermore let c be the binary-coded representation of which semiaxes are to
be used:
            1's bit: 1 if EAST is to be used; else 0
            2's bit: 1 if SOUTH is to be used; else 0
            4's bit: 1 if WEST is to be used; else 0
            8's bit: 1 if NORTH is to be used; else 0

Then entry [p][q][r][s][c] in the table is a string naming all the quad actions
that use all and only the semiaxes coded by c, when the reduced number of nodes
in the four quadrants are p, q, r, s respectively.

The quad actions are coded as strings of four code letters, A, B, C, D,
indicating the action for each of the four quadrants, as follows:

        A: "Anticlockwise": Assign the node nearest the anticlockwise
           end of the quadrant to the neighbouring semiaxis. E.g. in
           quadrant Q0 assign the node nearest EAST to that semiaxis.
        B: "Both": Do both the A and C actions.
        C: "Clockwise": Assign the node nearest the clockwise
           end of the quadrant to the neighbouring semiaxis. E.g. in
           quadrant Q0 assign the node nearest SOUTH to that semiaxis.
        D: "Do nothing".

The quad action codes (in general there may be many for a given p, q, r, s, c)
are separated by spaces.

The table is to be accessed using the lookupQuadActions function declared in
the qalookup.h header file.
"""

from collections import defaultdict

class Vect:
    """
    A vector to represent assignments to semiaxes.
    """

    def __init__(self, *args):
        self.comps = args

    def __add__(self,other):
        return Vect(*[a+b for a,b in zip(self.comps,other.comps)])

    def __repr__(self):
        s = ''
        for c in self.comps:
            s += ',%s'%c
        s = '('+s[1:]+')'
        return s

    def invalid(self):
        """
        An assignment to semiaxes is invalid iff any entry is greater than 1.
        """
        for c in self.comps:
            if c > 1:
                return True
        return False

    def height(self):
        return sum(self.comps)

    def binaryUsageCode(self):
        """
        Return an integer from 0 to 15 indicating which semiaxes
        are occupied, encoding semiaxis Sn in the nth binary bit.
        """
        code = 0
        s = 1
        for c in self.comps:
            if c > 0: code += s
            s *= 2
        return code

    def missing(self):
        """
        Return a list of the indices that get a 0, i.e. to which
        no node has been assigned.
        """
        m = []
        for i in range(len(self.comps)):
            if self.comps[i] == 0:
                m.append(i)
        return m

#    |   
#  r | s 
#    |
# ---+---
#    |
#  q | p
#    |

"""
Here we use Vect's to represent how many nodes will be assigned to each semiaxis,
when each of the possible actions is taken in each quadrant.

For example under pH we represent the assignments for the actions D, A, B, C in
the first quadrant.
"""

pH = [
    Vect(0,0,0,0),
    Vect(1,0,0,0),
    Vect(1,1,0,0),
    Vect(0,1,0,0)
]

qH = [
    Vect(0,0,0,0),
    Vect(0,1,0,0),
    Vect(0,1,1,0),
    Vect(0,0,1,0)
]

rH = [
    Vect(0,0,0,0),
    Vect(0,0,1,0),
    Vect(0,0,1,1),
    Vect(0,0,0,1)
]

sH = [
    Vect(0,0,0,0),
    Vect(0,0,0,1),
    Vect(1,0,0,1),
    Vect(1,0,0,0)
]


def enumerate(d):
    """
    d: a reduced distribution

    We write the list of legal quad actions to fill each possible set of semiaxes.
    """
    # Note the possible choices for each quadrant.
    choices = []
    for c in d:
        choices.append([
            [0],
            [0,1,3],
            [0,1,2,3]
        ][c])
    qas = defaultdict(list)
    code_letters = "DABC"
    # Iterate over all possible actions.
    for p in choices[0]:
        h0 = Vect(0,0,0,0) # E, S, W, N
        h1 = h0 + pH[p]
        for q in choices[1]:
            h2 = h1 + qH[q]
            for r in choices[2]:
                h3 = h2 + rH[r]
                for s in choices[3]:
                    hits = h3 + sH[s]
                    # If the hits vector is invalid, we're trying to do something
                    # impossible, so we skip this case.
                    if hits.invalid(): continue
                    # If we didn't make any assignments at all, then there's nothing to report.
                    H = hits.height()
                    if H == 0: continue
                    # Otherwise we'll record this as a valid quad action.
                    assert(1 <= H and H <= 4)
                    B = hits.binaryUsageCode()
                    qa = '%s%s%s%s'% tuple(map(
                        (lambda n : code_letters[n]), [p,q,r,s]
                    ))
                    qas[B].append(qa)
    return ['"' + ' '.join(qas[i]) + '"' for i in range(16)]

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
 * Generated by gen_qa_lookup.py
 *
*/

#include <string>
#include <sstream>
#include <iterator>
#include <vector>

#include "libdialect/quadaction.h"

using std::string;
using std::istringstream;
using std::istream_iterator;
using std::vector;

"""

CPP_FOOTER = """
vector<string> dialect::lookupQuadActions(size_t p, size_t q, size_t r, size_t s, size_t c) {
    string words = quadActionLookup[p][q][r][s][c];
    // Thanks to https://stackoverflow.com/a/237280
    istringstream iss(words);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};
    //
    return tokens;
}

"""

def writeCppArray():
    code = ''
    code += "const string quadActionLookup[3][3][3][3][16] = {\n"
    for p in range(3):
        code += "  {\n"
        for q in range(3):
            code += "    {\n"
            for r in range(3):
                code += "      {\n"
                for s in range(3):
                    dist = (p, q, r, s)
                    e = enumerate(dist)
                    code += "        {" + (', '.join(e)) + "}" + (',\n' if s < 2 else '\n')
                code += "      }" + (',\n' if r < 2 else '\n')
            code += "    }" + (',\n' if q < 2 else '\n')
        code += "  }" + (',\n' if p < 2 else '\n')
    code += "};\n"
    return code

def writeCppModule():
    code = ''
    code += CPP_HEADER
    code += writeCppArray()
    code += CPP_FOOTER
    with open('qalookup.cpp', 'w') as f:
        f.write(code)

if __name__=='__main__':
    writeCppModule()

