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


#include <cassert>
#include <iostream>
#include "libavoid/vertices.h"
#include "libavoid/connector.h"
#include "libavoid/graph.h"
#include "libavoid/static.h"
#include "libavoid/shape.h"
#include "libavoid/visibility.h"
#include "libavoid/debug.h"
#include "libavoid/router.h"

namespace Avoid {

static void computeCompleteVis(Router *router);

static void CreateVisGraph(Router *router, Polygn **obstacles,
        int n_obstacles);
static void DestroyVisGraph(Router *router);

// This should only be used for the static algorithm.
//
// XXX: If to set up the vis graph for incremental it would need 
//      the shapeRef pointers in obs.
//
void CreateVisGraph(Router *router, Polygn **obs, int n_obs)
{
    assert(router->vertices.connsBegin() == NULL);

    for (int poly_i = 0; poly_i < n_obs; poly_i++)
    {
        unsigned int id = obs[poly_i]->id;
        
        new ShapeRef(router, id, *(obs[poly_i]));
    }
    computeCompleteVis(router);
}


static void computeCompleteVis(Router *router)
{
    VertInf *beginVert = router->vertices.shapesBegin();
    VertInf *endVert = router->vertices.end();
    for (VertInf *i = beginVert; i != endVert; i = i->lstNext)
    {
        db_printf("-- CONSIDERING --\n");
        i->id.db_print();

        for (VertInf *j = i->lstPrev ; j != NULL; j = j->lstPrev)
        {
            bool knownNew = true;
            EdgeInf::checkEdgeVisibility(i, j, knownNew);
        }
    }
}


void DestroyVisGraph(Router *router)
{
    ShapeRefList::const_iterator sFinish = router->shapeRefs.end();
    ShapeRefList::const_iterator sCurr;
    
    while ((sCurr = router->shapeRefs.begin()) != sFinish)
    {
        ShapeRef *shape = (*sCurr);

        shape->removeFromGraph();
        delete shape;
    }
    
    ConnRefList::const_iterator cFinish = router->connRefs.end();
    ConnRefList::const_iterator cCurr;
    
    while ((cCurr = router->connRefs.begin())!= cFinish)
    {
        ConnRef *conn = (*cCurr);

        conn->removeFromGraph();
        conn->unInitialise();
    }
    // Clear contains info.
    router->contains.clear();

    assert(router->vertices.connsBegin() == NULL);
}


}

