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

using Avoid::ConnDirUp;
using Avoid::ConnDirDown;
using Avoid::ConnDirLeft;
using Avoid::ConnDirRight;

int main(void)
{
    Avoid::Router *router = new Avoid::Router(Avoid::OrthogonalRouting);
    
    // Create the ShapeRef:
    Avoid::Rectangle shapePoly(Avoid::Point(0, 0), Avoid::Point(100, 100));
    new Avoid::ShapeRef(router, shapePoly);

    Avoid::ConnEnd srcPt(Avoid::Point(200, 200));

    Avoid::ConnEnd dstPt(Avoid::Point(95, 25), 
            ConnDirUp | ConnDirRight | ConnDirDown);
    new Avoid::ConnRef(router, srcPt, dstPt);
    
    dstPt = Avoid::ConnEnd(Avoid::Point(95, 75), 
            ConnDirUp | ConnDirRight | ConnDirDown);
    new Avoid::ConnRef(router, srcPt, dstPt);
    
    // Up goes towards negative coordinates.
    dstPt = Avoid::ConnEnd(Avoid::Point(25, 5), 
            ConnDirLeft | ConnDirRight | ConnDirUp);
    new Avoid::ConnRef(router, srcPt, dstPt);
    
    dstPt = Avoid::ConnEnd(Avoid::Point(75, 5), 
            ConnDirLeft | ConnDirRight | ConnDirUp);
    new Avoid::ConnRef(router, srcPt, dstPt);

    dstPt = Avoid::ConnEnd(Avoid::Point(25, 95), 
            ConnDirLeft | ConnDirRight | ConnDirDown);
    new Avoid::ConnRef(router, srcPt, dstPt);
    
    dstPt = Avoid::ConnEnd(Avoid::Point(75, 95), 
            ConnDirLeft | ConnDirRight | ConnDirDown);
    new Avoid::ConnRef(router, srcPt, dstPt);
    
    router->processTransaction();
    router->outputDiagram("output/inline");

    delete router;
    return 0;
}

