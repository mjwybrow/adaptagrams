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
using namespace Avoid;

// Another testcase to check the functionality for connection pins.


int main(void)
{
    Router *router = new Router(OrthogonalRouting);
    router->setRoutingPenalty((PenaltyType)0, 50);
    
    Rectangle shapeRect1(Point(0, 0), Point(30, 20));
    ShapeRef *shapeRef1 = new ShapeRef(router, shapeRect1);
    
    Rectangle shapeRect2(Point(70, 7), Point(100, 27));
    new ShapeRef(router, shapeRect2);
    
    Rectangle shapeRect3(Point(50, 60), Point(80, 155));
    new ShapeRef(router, shapeRect3);
    
    Rectangle shapeRect4(Point(125, 60), Point(155, 80));
    new ShapeRef(router, shapeRect4);
    
    Rectangle shapeRect5(Point(15, 150), Point(45, 170));
    ShapeRef *shapeRef5 = new ShapeRef(router, shapeRect5);
    
    Rectangle shapeRect6(Point(130, 130), Point(160, 150));
    ShapeRef *shapeRef6 = new ShapeRef(router, shapeRect6);
    
    // Add a centre connection pin for the three shapes we'll be using.
    new ShapeConnectionPin(shapeRef1, CONNECTIONPIN_CENTRE, 
            ATTACH_POS_CENTRE, ATTACH_POS_CENTRE, true, 0.0, ConnDirNone);
    new ShapeConnectionPin(shapeRef5, CONNECTIONPIN_CENTRE, 
            ATTACH_POS_CENTRE, ATTACH_POS_CENTRE, true, 0.0, ConnDirNone);
    new ShapeConnectionPin(shapeRef6, CONNECTIONPIN_CENTRE, 
            ATTACH_POS_CENTRE, ATTACH_POS_CENTRE, true, 0.0, ConnDirNone);

    ConnEnd srcEnd(shapeRef1, CONNECTIONPIN_CENTRE);
    ConnEnd dstEnd(shapeRef6, CONNECTIONPIN_CENTRE);
    ConnRef *conn1= new ConnRef(router, srcEnd, dstEnd);
    
    router->processTransaction();
    router->outputDiagram("output/junction04-1");

    // Split the connector on its second segment and add a junction point.
    std::pair<JunctionRef *, ConnRef *> newObjs = 
            conn1->splitAtSegment(2);

    router->processTransaction();
    router->outputDiagram("output/junction04-2");

    // Create a connector from the centre of shape 5 that connects to 
    // the junction.
    ConnEnd srcEnd3(shapeRef5, CONNECTIONPIN_CENTRE);
    ConnEnd dstEnd3(newObjs.first);
    new ConnRef(router, srcEnd3, dstEnd3);

    router->processTransaction();
    router->outputDiagram("output/junction04-3");

    // Delete one half of the original connector, up to the junction.
    router->deleteConnector(conn1);
    conn1 = nullptr;

    router->processTransaction();
    router->outputDiagram("output/junction04-4");
    
    // The junction just has two connector now, so merge these into one.
    newObjs.first->removeJunctionAndMergeConnectors();
    router->processTransaction();
    router->outputDiagram("output/junction04-5");
    
    router->processTransaction();

    delete router;
    return 0;
}

