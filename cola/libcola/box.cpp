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

#include "libcola/box.h"

namespace cola {

Box::Box(double xMin, double xMax, double yMin, double yMax)
{
    m_min[vpsc::XDIM] = nonNegative(xMin);
    m_max[vpsc::XDIM] = nonNegative(xMax);
    m_min[vpsc::YDIM] = nonNegative(yMin);
    m_max[vpsc::YDIM] = nonNegative(yMax);
}

Box::Box(double all)
{
    double value = nonNegative(all);
    m_min[vpsc::XDIM] = value;
    m_max[vpsc::XDIM] = value;
    m_min[vpsc::YDIM] = value;
    m_max[vpsc::YDIM] = value;
}

Box::Box()
{
    m_min[vpsc::XDIM] = 0;
    m_max[vpsc::XDIM] = 0;
    m_min[vpsc::YDIM] = 0;
    m_max[vpsc::YDIM] = 0;
}

Box::~Box()
{
}

bool Box::empty(void) const
{
    // values will be nonnegative so can sum to check if empty.
    int total = m_min[vpsc::XDIM] + m_max[vpsc::XDIM] + 
            m_min[vpsc::YDIM] + m_max[vpsc::YDIM];
    return (total == 0);
}

void Box::outputCode(FILE *fp) const
{
    if ((m_min[vpsc::XDIM] == m_max[vpsc::XDIM]) &&
        (m_min[vpsc::XDIM] == m_min[vpsc::YDIM]) &&
        (m_min[vpsc::XDIM] == m_max[vpsc::YDIM]))
    {
        fprintf(fp, "Box(%g)", m_min[vpsc::XDIM]);
    }
    else
    {
        fprintf(fp, "Box(%g, %g, %g, %g)", m_min[vpsc::XDIM],
                m_max[vpsc::XDIM], m_min[vpsc::YDIM], m_max[vpsc::YDIM]);
    }
}

vpsc::Rectangle Box::rectangleByApplyingBox(
        const vpsc::Rectangle rectangle) const
{
    if (!rectangle.isValid())
    {
        return rectangle;
    }

    return vpsc::Rectangle(
            rectangle.getMinX() - m_min[vpsc::XDIM], 
            rectangle.getMaxX() + m_max[vpsc::XDIM], 
            rectangle.getMinY() - m_min[vpsc::YDIM], 
            rectangle.getMaxY() + m_max[vpsc::YDIM]);
}

double Box::nonNegative(double value)
{
    return (value >= 0) ? value: 0;
}

double Box::min(size_t dim) const
{
    return (dim < 2) ? m_min[dim] : 0;
}

double Box::max(size_t dim) const
{
    return (dim < 2) ? m_max[dim] : 0;
}


}

