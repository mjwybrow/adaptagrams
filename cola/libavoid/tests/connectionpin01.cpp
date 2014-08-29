/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2010  Monash University
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
#include "libavoid/connectionpin.h"

// A testcase to check the functionality for connection pins.


int main(void)
{
    Avoid::Router *router = new Avoid::Router(Avoid::OrthogonalRouting);
    
    // Create the ShapeRef:
    Avoid::Rectangle shapeRect1(Avoid::Point(0, 0), Avoid::Point(10, 10));
    Avoid::ShapeRef *shapeRef1 = new Avoid::ShapeRef(router, shapeRect1);

    
    Avoid::Rectangle shapeRect2(Avoid::Point(40, 20), Avoid::Point(50, 30));
    Avoid::ShapeRef *shapeRef2 = new Avoid::ShapeRef(router, shapeRect2);

    const unsigned int CENTRE = 1;
    new Avoid::ShapeConnectionPin(shapeRef1, CENTRE, Avoid::ATTACH_POS_CENTRE, 
            Avoid::ATTACH_POS_CENTRE, true, 0.0, Avoid::ConnDirNone);
    new Avoid::ShapeConnectionPin(shapeRef2, CENTRE, Avoid::ATTACH_POS_CENTRE, 
            Avoid::ATTACH_POS_CENTRE, true, 0.0, Avoid::ConnDirNone);

    Avoid::ConnEnd dstEnd(shapeRef1, CENTRE);
    Avoid::ConnEnd srcEnd(shapeRef2, CENTRE);
    new Avoid::ConnRef(router, srcEnd, dstEnd);
    // Force inital callback:
    router->processTransaction();
    router->outputDiagram("output/connectionpin01-1");

    router->moveShape(shapeRef2, 5, 0);

    router->processTransaction();
    router->outputDiagram("output/connectionpin01-2");
    
    router->moveShape(shapeRef1, 0, -10);

    router->processTransaction();
    router->outputDiagram("output/connectionpin01-3");

    delete router;
    return 0;
}

