#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    {
    Router *router = new Router(OrthogonalRouting);
    Rectangle rect548374163(Point(51900, 50400), Point(52300, 50900));
    ShapeRef *shapeRef548374163 = new ShapeRef(router, rect548374163, 548374163);
    router->addShape(shapeRef548374163);
    router->processTransaction();
    router->removeShape(shapeRef548374163);
    router->processTransaction();
    delete router;
    }
    {
    Router *router = new Router(OrthogonalRouting);
    ConnEnd srcPt29344262(Point(51150, 50500), 4);
    ConnEnd dstPt29344262(Point(51925, 50000), 4);
    ConnRef *conn = new ConnRef(router, srcPt29344262, dstPt29344262, 29344262);
    router->processTransaction();
    conn->setSourceEndpoint(Point(5, 5));
    delete conn;
    router->processTransaction();
    delete router;
    }
    return 0;
};

