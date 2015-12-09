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
    router->setRoutingPenalty((Avoid::PenaltyType)0, 50);
    
    // Create the ShapeRef:
    Avoid::Rectangle shapeRect1(Avoid::Point(0, 0), Avoid::Point(10, 10));
    Avoid::ShapeRef *shapeRef1 = new Avoid::ShapeRef(router, shapeRect1);
    
    Avoid::Rectangle shapeRect2(Avoid::Point(0, 90), Avoid::Point(20, 100));
    Avoid::ShapeRef *shapeRef2 = new Avoid::ShapeRef(router, shapeRect2);

    Avoid::Rectangle shapeRect3(Avoid::Point(50, 40), Avoid::Point(90, 80));
    Avoid::ShapeRef *shapeRef3 = new Avoid::ShapeRef(router, shapeRect3);

    // Create pins two input pins on the left side of shape 3.
    const unsigned int INPUT = 1;
    new Avoid::ShapeConnectionPin(shapeRef3, INPUT, 0.25, 
            0.4, true, 0, Avoid::ConnDirLeft);
    new Avoid::ShapeConnectionPin(shapeRef3, INPUT, 0.75, 
            0.6, true, 0, Avoid::ConnDirLeft);
    
    // And centre pins for two other shapes.
    new Avoid::ShapeConnectionPin(shapeRef1, Avoid::CONNECTIONPIN_CENTRE, 
            Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_CENTRE, true, 0.0, Avoid::ConnDirNone);
    new Avoid::ShapeConnectionPin(shapeRef2, Avoid::CONNECTIONPIN_CENTRE, 
            Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_CENTRE, true, 0.0, Avoid::ConnDirNone);

    // Create connectors from each shape to an input pin on shape 3.
    Avoid::ConnEnd srcEnd(shapeRef2, Avoid::CONNECTIONPIN_CENTRE);
    Avoid::ConnEnd dstEnd(shapeRef3, INPUT);
    new Avoid::ConnRef(router, srcEnd, dstEnd);
    
    srcEnd = Avoid::ConnEnd(shapeRef1, Avoid::CONNECTIONPIN_CENTRE);
    dstEnd = Avoid::ConnEnd(shapeRef3, INPUT);
    new Avoid::ConnRef(router, srcEnd, dstEnd);

    router->processTransaction();
    router->outputDiagram("output/connectionpin02-1");

    router->moveShape(shapeRef1, 0, 90);
    router->moveShape(shapeRef2, 0, -90);

    router->processTransaction();
    router->outputDiagram("output/connectionpin02-2");
    
    router->moveShape(shapeRef3, 0, -60);

    router->processTransaction();
    router->outputDiagram("output/connectionpin02-3");
    
    shapeRef3->transformConnectionPinPositions(Avoid::TransformationType_CW90);
    /*
    delete pin1;
    delete pin2;
    pin1 = new Avoid::ShapeConnectionPin(shapeRef3, INPUT, 
            Avoid::ATTACH_POS_RIGHT, 0.25, 5, Avoid::ConnDirRight);
    pin2 = new Avoid::ShapeConnectionPin(shapeRef3, INPUT,
            Avoid::ATTACH_POS_RIGHT, 0.75, 5, Avoid::ConnDirRight);
    */
    
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-4");
    
    shapeRef3->transformConnectionPinPositions(Avoid::TransformationType_CW180);
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-5");

    shapeRef3->transformConnectionPinPositions(Avoid::TransformationType_CW270);
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-6");

    shapeRef3->transformConnectionPinPositions(Avoid::TransformationType_FlipX);
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-7");

    shapeRef3->transformConnectionPinPositions(Avoid::TransformationType_FlipY);
    router->processTransaction();
    router->outputDiagram("output/connectionpin02-8");

    delete router;
    return 0;
}

