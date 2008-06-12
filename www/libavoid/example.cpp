/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2007  Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
*/

#include "libavoid.h"

    

static void connCallback(void *ptr)
{
    Avoid::ConnRef *connRef = (Avoid::ConnRef *) ptr; 

    printf("Connector %u needs rerouting!\n", connRef->id());

    if (connRef->needsReroute())
    {
        connRef->generatePath();
        Avoid::PolyLine& route = connRef->route();
        printf("New path: ");
        for (int i = 0; i < route.pn; ++i) 
        {
            printf("%s(%f, %f)", (i > 0) ? "-" : "", 
                    route.ps[i].x, route.ps[i].y);
        }
        printf("\n");
    }
}


int main(void)
{
    Avoid::Router *router = new Avoid::Router();
    
    // Create the ShapeRef:
    Avoid::Polygn shapePoly = Avoid::newPoly(3);
    shapePoly.ps[0] = Avoid::Point(1, 1);
    shapePoly.ps[1] = Avoid::Point(2.5, 1.5);
    shapePoly.ps[2] = Avoid::Point(1.5, 2.5);
    unsigned int shapeID = 1;
    Avoid::ShapeRef *shapeRef = new Avoid::ShapeRef(router, shapeID, shapePoly);
    // ShapeRef constructor makes a copy of polygon so we can free it:
    Avoid::freePoly(shapePoly);

    unsigned int connID = 2;
    Avoid::Point srcPt(1.2, 0.5);
    Avoid::Point dstPt(1.5, 4);
    Avoid::ConnRef *connRef = new Avoid::ConnRef(router, connID, srcPt, dstPt);
    connRef->updateEndPoint(Avoid::VertID::src, srcPt);
    connRef->updateEndPoint(Avoid::VertID::tar, dstPt);
    connRef->setCallback(connCallback, connRef);

    // Force inital callback:
    connRef->handleInvalid();

    printf("\nAdding a shape.\n");
    router->addShape(shapeRef);

    printf("\nShifting endpoint.\n");
    Avoid::Point dstPt2(6, 4.5);
    connRef->updateEndPoint(Avoid::VertID::tar, dstPt2);
    // It's expected you know the connector needs rerouting, so the callback
    // isn't called.  You can force it to be called though, via:
    connRef->handleInvalid();

    printf("\nMoving shape right by 0.5.\n");
    Avoid::Polygn newPoly = Avoid::newPoly(3);
    newPoly.ps[0] = Avoid::Point(1.5, 1);
    newPoly.ps[1] = Avoid::Point(3, 1.5);
    newPoly.ps[2] = Avoid::Point(2, 2.5);
    router->moveShape(shapeRef, &newPoly);
    Avoid::freePoly(newPoly);
    router->processMoves();

    return 0;
}

