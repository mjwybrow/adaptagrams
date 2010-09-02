/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2008  Monash University
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


#include "libavoid/shape.h"
#include "libavoid/graph.h"  // For alertConns
#include "libavoid/vertices.h"
#include "libavoid/router.h"
#include "libavoid/connend.h"
#include "libavoid/debug.h"
#include "libavoid/assertions.h"
#include "libavoid/connectionpin.h"


namespace Avoid {


ShapeRef::ShapeRef(Router *router, Polygon& ply, const unsigned int id)
    : Obstacle(router, ply, id)
{
}


ShapeRef::~ShapeRef()
{
}


void ShapeRef::setNewPoly(const Polygon& poly)
{
    COLA_ASSERT(m_first_vert != NULL);
    COLA_ASSERT(m_polygon.size() == poly.size());
    
    VertInf *curr = m_first_vert;
    for (size_t pt_i = 0; pt_i < m_polygon.size(); ++pt_i)
    {
        COLA_ASSERT(curr->visListSize == 0);
        COLA_ASSERT(curr->invisListSize == 0);

        // Reset with the new polygon point.
        curr->Reset(poly.ps[pt_i]);
        curr->pathNext = NULL;
        
        curr = curr->shNext;
    }
    COLA_ASSERT(curr == m_first_vert);
        
    m_polygon = poly;
}


void ShapeRef::moveAttachedConns(const Polygon& newPoly)
{
    // Update positions of attached connector ends.
    for (std::set<ConnEnd *>::iterator curr = m_following_conns.begin();
            curr != m_following_conns.end(); ++curr)
    {
        ConnEnd *connEnd = *curr;
        COLA_ASSERT(connEnd->m_conn_ref != NULL);
        m_router->modifyConnector(connEnd->m_conn_ref, connEnd->type(), 
                *connEnd);
    }
    for (std::set<ShapeConnectionPin *>::iterator curr = 
            m_connection_pins.begin(); curr != m_connection_pins.end(); ++curr)
    {
        ShapeConnectionPin *pin = *curr;
        pin->updatePosition(newPoly);
    }
}


void ShapeRef::transformConnectionPinPositions(
        ShapeTransformationType transform)
{
    for (std::set<ShapeConnectionPin *>::iterator curr = 
            m_connection_pins.begin(); curr != m_connection_pins.end(); ++curr)
    {
        ShapeConnectionPin *pin = *curr;
        double& xPortionOffset = pin->m_x_portion_offset;
        double& yPortionOffset = pin->m_y_portion_offset;
        ConnDirFlags& visDirs =  pin->m_visibility_directions;
        double tmpOffset;

        // Translate to Origin.
        xPortionOffset -= 0.5;
        yPortionOffset -= 0.5;

        // Number of clockwise 90degree rotations;
        unsigned int rotationN = 0;

        switch (transform)
        {
            case TransformationType_CW90:
                rotationN = 3;
                // Y <- inverse X, X <- inverse Y
                tmpOffset = yPortionOffset;
                yPortionOffset = xPortionOffset;
                xPortionOffset = -tmpOffset;
                break;
            case TransformationType_CW180:
                rotationN = 2;
                // Y <- inverse Y, X <- inverse X
                yPortionOffset = -yPortionOffset;
                xPortionOffset = -xPortionOffset;
                break;
            case TransformationType_CW270:
                rotationN = 1;
                // Y <- X, X <- Y
                tmpOffset = yPortionOffset;
                yPortionOffset = -xPortionOffset;
                xPortionOffset = tmpOffset;
                break;
            case TransformationType_FlipX:
                // Y <- Y, X <- inverse X
                xPortionOffset = -xPortionOffset;
                break;
            case TransformationType_FlipY:
                // X <- inverse X, Y <- Y
                yPortionOffset = -yPortionOffset;
                break;
        }
        // Translate back.
        xPortionOffset += 0.5;
        yPortionOffset += 0.5;

        if ( (visDirs & ConnDirAll) && (visDirs != ConnDirAll) )
        {
            // Visibility is set, but not in all directions.
            
            const unsigned int dirU = 0;
            const unsigned int dirR = 1;
            const unsigned int dirD = 2;
            const unsigned int dirL = 3;

            bool visInDir[4] = { false };
            if (visDirs & ConnDirUp)    visInDir[dirU] = true;
            if (visDirs & ConnDirRight) visInDir[dirR] = true;
            if (visDirs & ConnDirDown)  visInDir[dirD] = true;
            if (visDirs & ConnDirLeft)  visInDir[dirL] = true;

            if (transform == TransformationType_FlipY)
            {
                bool tmpBool = visInDir[dirU];
                visInDir[dirU] = visInDir[dirD];
                visInDir[dirD] = tmpBool;
            }
            else if (transform == TransformationType_FlipX)
            {
                bool tmpBool = visInDir[dirL];
                visInDir[dirL] = visInDir[dirR];
                visInDir[dirR] = tmpBool;
            }
            
            visDirs = ConnDirNone;
            if (visInDir[(rotationN + dirU) % 4])  visDirs |= ConnDirUp;
            if (visInDir[(rotationN + dirR) % 4])  visDirs |= ConnDirRight;
            if (visInDir[(rotationN + dirD) % 4])  visDirs |= ConnDirDown;
            if (visInDir[(rotationN + dirL) % 4])  visDirs |= ConnDirLeft;
        }
        pin->updatePositionAndVisibility();
        m_router->modifyConnectionPin(pin);
    }
}


const Polygon& ShapeRef::polygon(void) const
{
    return m_polygon;
}


void ShapeRef::outputCode(FILE *fp) const
{
    fprintf(fp, "    Polygon poly%u(%lu);\n", 
            id(), (unsigned long) polygon().size());
    for (size_t i = 0; i < polygon().size(); ++i)
    {
        fprintf(fp, "    poly%u.ps[%lu] = Point(%g, %g);\n", 
                id(), (unsigned long) i, polygon().at(i).x,
                polygon().at(i).y);
    }
    fprintf(fp, "    ShapeRef *shapeRef%u = new ShapeRef(router, poly%u, "
            "%u);\n", id(), id(), id());
    fprintf(fp, "    router->addShape(shapeRef%u);\n", id());
    for (std::set<ShapeConnectionPin *>::iterator curr = 
            m_connection_pins.begin(); 
            curr != m_connection_pins.end(); ++curr)
    {
        (*curr)->outputCode(fp);
    }
    fprintf(fp, "\n");
}


void ShapeRef::boundingBox(BBox& bbox) const
{
    COLA_ASSERT(!m_polygon.empty());

    bbox.a = bbox.b = m_polygon.ps[0];
    Point& a = bbox.a;
    Point& b = bbox.b;

    for (size_t i = 1; i < m_polygon.size(); ++i)
    {
        const Point& p = m_polygon.ps[i];

        a.x = std::min(p.x, a.x);
        a.y = std::min(p.y, a.y);
        b.x = std::max(p.x, b.x);
        b.y = std::max(p.y, b.y);
    }
}


Point ShapeRef::position(void) const
{
    BBox bb;
    boundingBox(bb);

    Point centre;

    centre.x = bb.a.x + (0.5 * (bb.b.x - bb.a.x));
    centre.y = bb.a.y + (0.5 * (bb.b.y - bb.a.y));

    return centre;
}


}

