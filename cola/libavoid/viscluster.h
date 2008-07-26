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
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 * 
*/

#ifndef AVOID_CLUSTER_H
#define AVOID_CLUSTER_H

#include <list>

#include "libavoid/geometry.h"


namespace Avoid {

class Router;
class ClusterRef;
typedef std::list<ClusterRef *> ClusterRefList;


class ClusterRef
{
    public:
        ClusterRef(Router *router, unsigned int id, Polygn& poly);
        ~ClusterRef();
        void setNewPoly(Polygn& poly);
        unsigned int id(void);
        ReferencingPolygn& poly(void);
        Router *router(void);
        void makeActive(void);
        void makeInactive(void);

    private:
        Router *_router;
        unsigned int _id;
        ReferencingPolygn _poly;
        bool _active;
        ClusterRefList::iterator _pos;
};


}


#endif


