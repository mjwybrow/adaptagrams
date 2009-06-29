#include "libavoid/libavoid.h"
using namespace Avoid;
int main(void) {
    Router *router = new Router(OrthogonalRouting);
    router->segmt_penalty = 50;
    router->shared_path_penalty = (2.1 * router->segmt_penalty);
    router->setOrthogonalNudgeDistance(25);
    Rectangle rect335855988(Point(51140, 50190), Point(51510, 50885));
    ShapeRef *shapeRef335855988 = new ShapeRef(router, rect335855988, 335855988);
    router->addShape(shapeRef335855988);
    Rectangle rect548374163(Point(51915, 50415), Point(52285, 50885));
    ShapeRef *shapeRef548374163 = new ShapeRef(router, rect548374163, 548374163);
    router->addShape(shapeRef548374163);
    Rectangle rect942733064(Point(51771, 50790), Point(51829, 50885));
    ShapeRef *shapeRef942733064 = new ShapeRef(router, rect942733064, 942733064);
    router->addShape(shapeRef942733064);
    ConnEnd srcPt100850179(Point(51925, 50800), 4);
    ConnEnd dstPt100850179(Point(51500, 50650), 8);
    ConnEnd srcPt240732432(Point(51925, 50650), 4);
    ConnEnd dstPt240732432(Point(51800, 50800), 1);
    new ConnRef(router, srcPt240732432, dstPt240732432, 240732432);
    new ConnRef(router, srcPt100850179, dstPt100850179, 100850179);
    router->processTransaction();
    router->outputInstanceToSVG();
    delete router;
    return 0;
};
