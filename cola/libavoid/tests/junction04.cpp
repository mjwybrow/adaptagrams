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

// Another testcase to check the functionality for connection pins.


int main(void)
{
    Avoid::Router *router = new Avoid::Router(Avoid::OrthogonalRouting);
    router->setRoutingPenalty((Avoid::PenaltyType)0, 50);
    
    Avoid::Rectangle shapeRect1(Avoid::Point(0, 0), Avoid::Point(30, 20));
    Avoid::ShapeRef *shapeRef1 = new Avoid::ShapeRef(router, shapeRect1);
    
    Avoid::Rectangle shapeRect2(Avoid::Point(70, 7), Avoid::Point(100, 27));
    new Avoid::ShapeRef(router, shapeRect2);
    
    Avoid::Rectangle shapeRect3(Avoid::Point(50, 60), Avoid::Point(80, 155));
    new Avoid::ShapeRef(router, shapeRect3);
    
    Avoid::Rectangle shapeRect4(Avoid::Point(125, 60), Avoid::Point(155, 80));
    new Avoid::ShapeRef(router, shapeRect4);
    
    Avoid::Rectangle shapeRect5(Avoid::Point(15, 150), Avoid::Point(45, 170));
    Avoid::ShapeRef *shapeRef5 = new Avoid::ShapeRef(router, shapeRect5);
    
    Avoid::Rectangle shapeRect6(Avoid::Point(130, 130), Avoid::Point(160, 150));
    Avoid::ShapeRef *shapeRef6 = new Avoid::ShapeRef(router, shapeRect6);
    
    // Add a centre connection pin for the three shapes we'll be using.
    new Avoid::ShapeConnectionPin(shapeRef1, Avoid::CONNECTIONPIN_CENTRE, 
            Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_CENTRE, true, 0.0, Avoid::ConnDirNone);
    new Avoid::ShapeConnectionPin(shapeRef5, Avoid::CONNECTIONPIN_CENTRE, 
            Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_CENTRE, true, 0.0, Avoid::ConnDirNone);
    new Avoid::ShapeConnectionPin(shapeRef6, Avoid::CONNECTIONPIN_CENTRE, 
            Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_CENTRE, true, 0.0, Avoid::ConnDirNone);

    Avoid::ConnEnd srcEnd(shapeRef1, Avoid::CONNECTIONPIN_CENTRE);
    Avoid::ConnEnd dstEnd(shapeRef6, Avoid::CONNECTIONPIN_CENTRE);
    Avoid::ConnRef *conn1= new Avoid::ConnRef(router, srcEnd, dstEnd);
    
    router->processTransaction();
    router->outputDiagram("output/junction04-1");

    // Split the connector on its second segment and add a junction point.
    std::pair<Avoid::JunctionRef *, Avoid::ConnRef *> newObjs = 
            conn1->splitAtSegment(2);

    router->processTransaction();
    router->outputDiagram("output/junction04-2");

    // Create a connector from the centre of shape 5 that connects to 
    // the junction.
    Avoid::ConnEnd srcEnd3(shapeRef5, Avoid::CONNECTIONPIN_CENTRE);
    Avoid::ConnEnd dstEnd3(newObjs.first);
    new Avoid::ConnRef(router, srcEnd3, dstEnd3);

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

