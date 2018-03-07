/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2007  Michael Wybrow <mjwybrow@users.sourceforge.net>
 * Copyright (C) 2009  Monash University
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

#include "libavoid/libavoid.h"

    

static void connCallback(void *ptr)
{
    Avoid::ConnRef *connRef = (Avoid::ConnRef *) ptr; 

    printf("Connector %u needs rerouting!\n", connRef->id());

    const Avoid::PolyLine& route = connRef->displayRoute();
    printf("New path: ");
    for (size_t i = 0; i < route.ps.size(); ++i) 
    {
        printf("%s(%f, %f)", (i > 0) ? "-" : "", 
                route.ps[i].x, route.ps[i].y);
    }
    printf("\n");
}


int main(void)
{
    Avoid::Router *router = new Avoid::Router(Avoid::PolyLineRouting);
    
    Avoid::Point srcPt(1.2, 0.5);
    Avoid::Point dstPt(1.5, 4);
    Avoid::ConnRef *connRef = new Avoid::ConnRef(router, srcPt, dstPt);
    connRef->setCallback(connCallback, connRef);
    // Force inital callback:
    router->processTransaction();

    printf("\nAdding a shape.\n");
    // Create the ShapeRef:
    Avoid::Polygon shapePoly(3);
    shapePoly.ps[0] = Avoid::Point(1, 1);
    shapePoly.ps[1] = Avoid::Point(2.5, 1.5);
    shapePoly.ps[2] = Avoid::Point(1.5, 2.5);
    Avoid::ShapeRef *shapeRef = new Avoid::ShapeRef(router, shapePoly);
    router->processTransaction();

    printf("\nShifting endpoint.\n");
    Avoid::Point dstPt2(6, 4.5);
    connRef->setDestEndpoint(dstPt2);
    // It's expected you know the connector needs rerouting, so the callback
    // isn't called.  You can force it to be called though, via:
    router->processTransaction();

    printf("\nMoving shape right by 0.5.\n");
    router->moveShape(shapeRef, 0.5, 0);
    router->processTransaction();

    router->outputDiagram("output/example");
    delete router;
    return 0;
}

