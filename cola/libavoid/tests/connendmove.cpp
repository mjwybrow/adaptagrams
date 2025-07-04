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
using namespace Avoid;
// Regression test to catch a bug where moving detaching a connected 
// endpoint that was scheduled to move would result in a double free.
    

int main(void)
{
    Router *router = new Router(OrthogonalRouting);
    
    // Create the ShapeRef:
    Rectangle shapeRect1(Point(0, 0), Point(10, 10));
    ShapeRef *shapeRef1 = new ShapeRef(router, shapeRect1);
    const unsigned int CENTRE = 1;
    new ShapeConnectionPin(shapeRef1, CENTRE, 
            ATTACH_POS_CENTRE, ATTACH_POS_CENTRE, true, 0.0, ConnDirNone);
    
    Rectangle shapeRect2(Point(0, 0), Point(10, 10));
    ShapeRef *shapeRef2 = new ShapeRef(router, shapeRect1);

    ConnEnd dstPt(shapeRef1, CENTRE); 
    Point srcPt(1.5, 4);
    ConnRef *connRef = new ConnRef(router, srcPt, dstPt);
    // Force inital callback:
    router->processTransaction();
    router->outputDiagram("output/connendmove-1");

    Point dstPt2(20, 20);
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

