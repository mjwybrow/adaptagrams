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

// Regression test to catch a bug where moving detaching a connected 
// endpoint that was scheduled to move would result in a double free.
    

int main(void)
{
    Avoid::Router *router = new Avoid::Router(Avoid::OrthogonalRouting);
    
    // Create the ShapeRef:
    Avoid::Rectangle shapeRect1(Avoid::Point(0, 0), Avoid::Point(10, 10));
    Avoid::ShapeRef *shapeRef1 = new Avoid::ShapeRef(router, shapeRect1);
    const unsigned int CENTRE = 1;
    new Avoid::ShapeConnectionPin(shapeRef1, CENTRE, 
            Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_CENTRE, true, 0.0, Avoid::ConnDirNone);
    
    Avoid::Rectangle shapeRect2(Avoid::Point(0, 0), Avoid::Point(10, 10));
    Avoid::ShapeRef *shapeRef2 = new Avoid::ShapeRef(router, shapeRect1);

    Avoid::ConnEnd dstPt(shapeRef1, CENTRE); 
    Avoid::Point srcPt(1.5, 4);
    Avoid::ConnRef *connRef = new Avoid::ConnRef(router, srcPt, dstPt);
    // Force inital callback:
    router->processTransaction();
    router->outputDiagram("output/connendmove-1");

    Avoid::Point dstPt2(20, 20);
    connRef->setDestEndpoint(dstPt2);
    router->moveShape(shapeRef1, 0.5, 0);

    router->processTransaction();
    router->outputDiagram("output/connendmove-2");

    srcPt.x += 0.5;
    connRef->setSourceEndpoint(srcPt);
    router->moveShape(shapeRef1, 0.5, 0);
    router->moveShape(shapeRef2, 0, 0.5);
    
    srcPt.x += 0.5;
    connRef->setSourceEndpoint(srcPt);
    router->moveShape(shapeRef1, 0.5, 0);
    router->moveShape(shapeRef2, 0, 0.5);
    
    srcPt.x += 0.5;
    connRef->setSourceEndpoint(srcPt);
    router->moveShape(shapeRef1, 0.5, 0);
    router->moveShape(shapeRef2, 0, 0.5);
    
    srcPt.x += 0.5;
    connRef->setSourceEndpoint(srcPt);
    router->moveShape(shapeRef1, 0.5, 0);
    router->moveShape(shapeRef2, 0, 0.5);
    
    router->processTransaction();
    router->outputDiagram("output/connendmove-3");

    delete router;
    return 0;
}

