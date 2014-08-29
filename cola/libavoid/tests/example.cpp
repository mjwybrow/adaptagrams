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
/*
#include "libavoid/libavoid.h"

    

static void connCallback(void *ptr)
{
    Avoid::ConnRef *connRef = (Avoid::ConnRef *) ptr; 

    printf("Connector %u needs rerouting!\n", connRef->id());

    const Avoid::PolyLine& route = connRef->route();
    printf("New path: ");
    for (size_t i = 0; i < route.ps.size(); ++i) 
    {
        printf("%s(%f, %f)", (i > 0) ? "-" : "", 
                route.ps[i].x, route.ps[i].y);
    }
    printf("\n");
}


int main(void)
{
    Avoid::Router *router = new Avoid::Router(Avoid::PolyLineRouting);
    
    Avoid::Point srcPt(1.2, 0.5);
    Avoid::Point dstPt(1.5, 4);
    Avoid::ConnRef *connRef = new Avoid::ConnRef(router, srcPt, dstPt);
    connRef->setCallback(connCallback, connRef);
    // Force inital callback:
    router->processTransaction();

    printf("\nAdding a shape.\n");
    // Create the ShapeRef:
    Avoid::Polygon shapePoly(3);
    shapePoly.ps[0] = Avoid::Point(1, 1);
    shapePoly.ps[1] = Avoid::Point(2.5, 1.5);
    shapePoly.ps[2] = Avoid::Point(1.5, 2.5);
    Avoid::ShapeRef *shapeRef = new Avoid::ShapeRef(router, shapePoly);
    router->processTransaction();

    printf("\nShifting endpoint.\n");
    Avoid::Point dstPt2(6, 4.5);
    connRef->setDestEndpoint(dstPt2);
    // It's expected you know the connector needs rerouting, so the callback
    // isn't called.  You can force it to be called though, via:
    router->processTransaction();

    printf("\nMoving shape right by 0.5.\n");
    router->moveShape(shapeRef, 0.5, 0);
    router->processTransaction();

    router->outputDiagram("output/example");
    delete router;
    return 0;
}
*/

#include "libavoid/libavoid.h"
#include <sstream>
using namespace Avoid;

void test()
{
    Avoid::ConnEnd end1;
    Avoid::ConnEnd end2;
    Avoid::Polygon poly;
    Avoid::Router * router1 = new Avoid::Router(Avoid::OrthogonalRouting);
    router1->setTransactionUse(true);
    router1->setRoutingPenalty(Avoid::segmentPenalty);
    router1->setRoutingPenalty(Avoid::crossingPenalty);
    router1->setRoutingPenalty(Avoid::fixedSharedPathPenalty, 9000);
    router1->setRoutingOption(Avoid::penaliseOrthogonalSharedPathsAtConnEnds, true);
    router1->setRoutingParameter(idealNudgingDistance, 25);
    poly = Avoid::Polygon(4);
    poly.setPoint(0, Avoid::Point(50760, 50691));
    poly.setPoint(1, Avoid::Point(50760, 50775));
    poly.setPoint(2, Avoid::Point(50640, 50775));
    poly.setPoint(3, Avoid::Point(50640, 50691));
    Avoid::ShapeRef * shape386142480 = new Avoid::ShapeRef(router1, poly, 386142480);
    Avoid::ShapeConnectionPin * pin386142480_0 = new Avoid::ShapeConnectionPin(shape386142480, 1,                  0.5,  0.40476190476190477,                    true, 0, 13);
    pin386142480_0->setExclusive(true);
    Avoid::ShapeConnectionPin * pin386142480_1 = new Avoid::ShapeConnectionPin(shape386142480, 2,                  0.5,                    1,                   true, 10, 2);
    pin386142480_1->setExclusive(true);
    poly = Avoid::Polygon(4);
    poly.setPoint(0, Avoid::Point(51185, 50765));
    poly.setPoint(1, Avoid::Point(51185, 50935));
    poly.setPoint(2, Avoid::Point(50975, 50935));
    poly.setPoint(3, Avoid::Point(50975, 50765));
    Avoid::ShapeRef * shape300136482 = new Avoid::ShapeRef(router1, poly, 300136482);
    Avoid::ShapeConnectionPin * pin300136482_0 = new Avoid::ShapeConnectionPin(shape300136482, 1,                    0,                  0.5,                   true, 10, 4);
    pin300136482_0->setExclusive(true);
    Avoid::ShapeConnectionPin * pin300136482_1 = new Avoid::ShapeConnectionPin(shape300136482, 2,  0.95238095238095233,                  0.5,                   true, 10, 8);
    pin300136482_1->setExclusive(true);
    poly = Avoid::Polygon(4);
    poly.setPoint(0, Avoid::Point(50854, 50840));
    poly.setPoint(1, Avoid::Point(50854, 50935));
    poly.setPoint(2, Avoid::Point(50796, 50935));
    poly.setPoint(3, Avoid::Point(50796, 50840));
    Avoid::ShapeRef * shape51003942 = new Avoid::ShapeRef(router1, poly, 51003942);
    Avoid::ShapeConnectionPin * pin51003942_0 = new Avoid::ShapeConnectionPin(shape51003942, 1,                  0.5,  0.10526315789473684,                   true, 10, 1);
    pin51003942_0->setExclusive(true);
    Avoid::JunctionRef * junction502411221 = new Avoid::JunctionRef(router1, Avoid::Point(50825, 50850), 502411221);
    end1 = Avoid::ConnEnd(shape386142480, 2);
    end2 = Avoid::ConnEnd(junction502411221);
    new Avoid::ConnRef(router1, end1, end2);
    end1 = Avoid::ConnEnd(junction502411221);
    end2 = Avoid::ConnEnd(shape300136482, 1);
    new Avoid::ConnRef(router1, end1, end2);
    end1 = Avoid::ConnEnd(shape51003942, 1);
    end2 = Avoid::ConnEnd(junction502411221);
    new Avoid::ConnRef(router1, end1, end2);
    router1->processTransaction();

    //router1->outputDiagram("output/buildOrthogonalChannelInfo1");
    router1->outputDiagramText("output/buildOrthogonalChannelInfo1");
    delete router1;
}

extern "C" int main(void)
{
    test();
    return 0;
}
































