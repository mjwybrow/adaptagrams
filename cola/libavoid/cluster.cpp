/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2006  Michael Wybrow <mjwybrow@users.sourceforge.net>
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
*/

#include "libavoid/cluster.h"
#include "libavoid/router.h"
#include "libavoid/polyutil.h"


namespace Avoid {


ClusterRef::ClusterRef(Router *router, unsigned int id, Polygn& ply)
    : _router(router)
    , _id(id)
    , _poly(copyPoly(ply))
    , _active(false)
{
    assert(id > 0);
}


ClusterRef::~ClusterRef()
{
    freePoly(_poly);
}


void ClusterRef::makeActive(void)
{
    assert(!_active);
    
    // Add to connRefs list.
    _pos = _router->clusterRefs.insert(_router->clusterRefs.begin(), this);

    _active = true;
}


void ClusterRef::makeInactive(void)
{
    assert(_active);
    
    // Remove from connRefs list.
    _router->clusterRefs.erase(_pos);

    _active = false;
}
    

void ClusterRef::setNewPoly(Polygn& poly)
{
    freePoly(_poly);
    _poly = copyPoly(poly);
}


unsigned int ClusterRef::id(void)
{
    return _id;
}


Polygn& ClusterRef::poly(void)
{
    return _poly;
}


Router *ClusterRef::router(void)
{
    return _router;
}


}


