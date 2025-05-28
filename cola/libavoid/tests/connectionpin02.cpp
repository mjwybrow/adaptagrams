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
    router->setRoutingPenalty((PenaltyType)0, 50);
    
    // Create the ShapeRef:
    Rectangle shapeRect1(Point(0, 0), Point(10, 10));
    ShapeRef *shapeRef1 = new ShapeRef(router, shapeRect1);
    
    Rectangle shapeRect2(Point(0, 90), Point(20, 100));
    ShapeRef *shapeRef2 = new ShapeRef(router, shapeRect2);

    Rectangle shapeRect3(Point(50, 40), Point(90, 80));
    ShapeRef *shapeRef3 = new ShapeRef(router, shapeRect3);

    // Create pins two input pins on the left side of shape 3.
    const unsigned int INPUT = 1;
    new ShapeConnectionPin(shapeRef3, INPUT, 0.25, 
            0.4, true, 0, ConnDirLeft);
    new ShapeConnectionPin(shapeRef3, INPUT, 0.75, 
            0.6, true, 0, ConnDirLeft);
    
    // And centre pins for two other shapes.
    new ShapeConnectionPin(shapeRef1, CONNECTIONPIN_CENTRE, 
            ATTACH_POS_CENTRE, ATTACH_POS_CENTRE, true, 0.0, ConnDirNone);
    new ShapeConnectionPin(shapeRef2, CONNECTIONPIN_CENTRE, 
            ATTACH_POS_CENTRE, ATTACH_POS_CENTRE, true, 0.0, ConnDirNone);

    // Create connectors from each shape to an input pin on shape 3.
    ConnEnd srcEnd(shapeRef2, CONNECTIONPIN_CENTRE);
    ConnEnd dstEnd(shapeRef3, INPUT);
    new ConnRef(router, srcEnd, dstEnd);
    
    srcEnd = ConnEnd(shapeRef1, CONNECTIONPIN_CENTRE);
    dstEnd = ConnEnd(shapeRef3, INPUT);
    new ConnRef(router, srcEnd, dstEnd);

    router->processTransaction();
    router->outputDiagram("output/connectionpin02-1");

    router->moveShape(shapeRef1, 0, 90);
    router->moveShape(shapeRef2, 0, -90);

    router->processTransaction();
    router->outputDiagram("output/connectionpin02-2");
    
    router->moveShape(shapeRef3, 0, -60);

    router->processTransaction();
    router->outputDiagram("output/connectionpin02-3");
    
    shapeRef3->transformConnectionPinPositions(TransformationType_CW90);
    /*
    delete pin1;
    delete pin2;
    pin1 = new ShapeConnectionPin(shapeRef3, INPUT, 
            ATTACH_POS_RIGHT, 0.25, 5, ConnDirRight);
    pin2 = new ShapeConnectionPin(shapeRef3, INPUT,
            ATTACH_POS_RIGHT, 0.75, 5, ConnDirRight);
    */
    
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-4");
    
    shapeRef3->transformConnectionPinPositions(TransformationType_CW180);
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-5");

    shapeRef3->transformConnectionPinPositions(TransformationType_CW270);
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-6");

    shapeRef3->transformConnectionPinPositions(TransformationType_FlipX);
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-7");

    shapeRef3->transformConnectionPinPositions(TransformationType_FlipY);
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-8");

    delete router;
    return 0;
}

