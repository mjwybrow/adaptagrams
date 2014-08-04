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

#include "libvpsc/assertions.h"

#include "libcola/shapepair.h"

namespace cola {

ShapePair::ShapePair(unsigned ind1, unsigned ind2) 
{
    COLA_ASSERT(ind1 != ind2);
    // Assign the lesser value to m_index1.
    m_index1 = (ind1 < ind2) ? ind1 : ind2;
    // Assign the greater value to m_index2.
    m_index2 = (ind1 > ind2) ? ind1 : ind2;
}

bool ShapePair::operator<(const ShapePair& rhs) const
{
    if (m_index1 != rhs.m_index1)
    {
        return m_index1 < rhs.m_index1;
    }
    return m_index2 < rhs.m_index2;
}

};
