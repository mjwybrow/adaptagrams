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

using namespace Avoid;

// A testcase to check the functionality for connection pins.


int main(void)
{
    Router *router = new Router(OrthogonalRouting);
    
    // Create the ShapeRef:
    Rectangle shapeRect1(Point(0, 0), Point(10, 10));
    ShapeRef *shapeRef1 = new ShapeRef(router, shapeRect1);

    
    Rectangle shapeRect2(Point(40, 20), Point(50, 30));
    ShapeRef *shapeRef2 = new ShapeRef(router, shapeRect2);

    const unsigned int CENTRE = 1;
    new ShapeConnectionPin(shapeRef1, CENTRE, ATTACH_POS_CENTRE, 
            ATTACH_POS_CENTRE, true, 0.0, ConnDirNone);
    new ShapeConnectionPin(shapeRef2, CENTRE, ATTACH_POS_CENTRE, 
            ATTACH_POS_CENTRE, true, 0.0, ConnDirNone);

    ConnEnd dstEnd(shapeRef1, CENTRE);
    ConnEnd srcEnd(shapeRef2, CENTRE);
    new ConnRef(router, srcEnd, dstEnd);
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

