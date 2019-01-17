/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2014  Monash University
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
 * Author(s):  Michael Wybrow
 *
*/

#ifndef COLA_SHAPEPAIR_H
#define COLA_SHAPEPAIR_H

namespace cola {

// A pair of indexes.  
// Specified unordered but stored ordered so it can be compared and 
// stored in a set.
//
class ShapePair
{
    public:
        ShapePair(unsigned ind1, unsigned ind2);
        bool operator<(const ShapePair& rhs) const;
        unsigned short index1(void) const {return m_index1;}
        unsigned short index2(void) const {return m_index2;}

    private:
        unsigned short m_index1;
        unsigned short m_index2;
};


};

#endif

