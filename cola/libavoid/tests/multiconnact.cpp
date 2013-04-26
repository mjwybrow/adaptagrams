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
    Avoid::Router *router = new Avoid::Router(Avoid::OrthogonalRouting);
    
    Avoid::Point srcPt(0,400);
    Avoid::Point dstPt(775,400);
    Avoid::ConnRef *connRef = new Avoid::ConnRef(router, srcPt, dstPt);
    connRef->setCallback(connCallback, connRef);

    Avoid::Point srcPt2(775,625);
    Avoid::Point dstPt2(350,475);
    Avoid::ConnRef *connRef2 = new Avoid::ConnRef(router, srcPt2, dstPt2);
    connRef2->setCallback(connCallback, connRef2);
    
    router->processTransaction();

    printf("\nShifting endpoint.\n");
    connRef->setEndpoints(Avoid::Point(0,375), Avoid::Point(775,400));
    connRef2->setEndpoints(Avoid::Point(775,625), Avoid::Point(350,450));
    router->processTransaction();

    printf("\nShifting endpoint.\n");
    connRef->setEndpoints(Avoid::Point(0,400), Avoid::Point(775,400));
    connRef2->setEndpoints(Avoid::Point(775,625), Avoid::Point(350,475));
    router->processTransaction();

    delete router;
    return 0;
}

