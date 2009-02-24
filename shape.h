/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2004-2007  Michael Wybrow <mjwybrow@users.sourceforge.net>
 * Copyright (C) 2008  Monash University
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


#ifndef AVOID_SHAPE_H
#define AVOID_SHAPE_H

#include "libavoid/geometry.h"
#include <list>


namespace Avoid {

class VertInf;
class Router;
class ShapeRef;
typedef std::list<ShapeRef *> ShapeRefList;


class ShapeRef
{
    public:
        ShapeRef(Router *router, unsigned int id, Polygon& poly);
        ~ShapeRef();
        void setNewPoly(Polygon& poly);
        VertInf *firstVert(void);
        VertInf *lastVert(void);
        unsigned int id(void);
        const Polygon& poly(void);
        Router *router(void);
        void boundingBox(BBox& bbox);

        void makeActive(void);
        void makeInactive(void);

        void removeFromGraph(void);
        void markForMove(void);
        void clearMoveMark(void);

        VertInf *getPointVertex(const Point& point);

    private:
        Router *_router;
        unsigned int _id;
        Polygon _poly;
        bool _active;
        bool _inMoveList;
        ShapeRefList::iterator _pos;
        VertInf *_firstVert;
        VertInf *_lastVert;
};


}


#endif


