#include "libavoid/libavoid.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
/*
 * Test routing between child shapes in a parent shape.
 * All child shapes have two or four shape connection pins(with the same class id per pair) on shape edges for
 * outgoing connections and one shape connection pin in the center for incoming connection.
 * */

using namespace Avoid;

bool compare_float(double x, double y, double epsilon = 0.001f) {
    if(fabs(x - y) < epsilon)
        return true;
    return false;
}

MATCHER_P(IsEqualToRoute, value, "equal routes") {
    std::vector<Point> actualRoute = arg->displayRoute().ps;
    if (actualRoute.size() != value.size()) {
        *result_listener << "Length of routes is not equal"<< actualRoute.size() << value.size();
        return false;
    }

    bool result = true;
    *result_listener << "\n";
    for (int i=0; i < actualRoute.size(); i++) {
        if (!(compare_float(actualRoute.at(i).x, value.at(i).x) &&
                compare_float(actualRoute.at(i).y, value.at(i).y))) {
            *result_listener << "Point " << (i + 1) << ": actual (" << actualRoute.at(i).x << ", " << actualRoute.at(i).y << "), expected (" << value.at(i).x << ", " << value.at(i).y << ")\n";
            result = false;
        }
    }
    return result;
}


class HierarchicalOrthogonalRouter : public ::testing::Test {
protected:
    void SetUp() override {
        router = new Router(OrthogonalRouting);
        router->setRoutingParameter(RoutingParameter::shapeBufferDistance, 4);
        router->setRoutingParameter(RoutingParameter::segmentPenalty, 50);
        router->setRoutingParameter(RoutingParameter::idealNudgingDistance, 4);
        router->setRoutingOption(RoutingOption::nudgeOrthogonalSegmentsConnectedToShapes, true);
        router->setRoutingOption(RoutingOption::nudgeOrthogonalTouchingColinearSegments, true);

        Rectangle parent({ -125.954, -425.8825 }, { 934.85, 966.244 });
        ShapeRef *parentShape = new ShapeRef(router, parent, 1);
    }

    void TearDown() override {
        delete router;
    }

    ShapeRef* addChild(Point topLeft, Point bottomRight, unsigned int shapeId, unsigned int connectionId, unsigned int connectionId2 = 0) {
        Rectangle childRectangle(topLeft, bottomRight);
        ShapeRef *childShape = new ShapeRef(router, childRectangle, shapeId);
        new ShapeConnectionPin(childShape, connectionId, 0, 14, false, 0, ConnDirLeft);
        new ShapeConnectionPin(childShape, connectionId, 200, 14, false, 0, ConnDirRight);
        if (connectionId2 != 0) {
            new ShapeConnectionPin(childShape, connectionId2, 0, 56, false, 0, ConnDirLeft);
            new ShapeConnectionPin(childShape, connectionId2, 200, 56, false, 0, ConnDirRight);
        }
        new ShapeConnectionPin(childShape, CONNECTIONPIN_CENTRE,
                               ATTACH_POS_CENTRE, ATTACH_POS_CENTRE, true, 0.0, ConnDirNone);
        return childShape;
    }

    ConnRef*  connectShapes(ShapeRef *shape1, unsigned int shape1ConnId, ShapeRef *shape2) {
        ConnEnd srcPtEnd(shape1, shape1ConnId);
        ConnEnd dstPtEnd(shape2, CONNECTIONPIN_CENTRE);
        ConnRef *connection = new ConnRef(router, srcPtEnd, dstPtEnd);
        return connection;
    }

    Router *router;
};

namespace Avoid {
    void PrintTo(ConnRef *conn, std::ostream *os) {
        std::vector<Point> route = conn->displayRoute().ps;
        *os << "[";
        for (int i = 0; i < route.size(); i++) {
            *os << "(" << route.at(i).x << ", " << route.at(i).y << ")";
            if (i != route.size() - 1) {
                *os << ", ";
            }
        }
        *os << "]";
    }
}

/* Checks: https://github.com/Aksem/adaptagrams/issues/3 */
TEST_F(HierarchicalOrthogonalRouter, TwoChildrenVertically) {
    ShapeRef *topChildShape = addChild({ 616.26, 565.279 }, { 816.26, 730.279 }, 2, 5);
    ShapeRef *bottomChildShape = addChild({ 616.26, 766.244 }, { 816.26, 931.244 }, 3, 6);

    ConnRef *bottomToTopConn = connectShapes(bottomChildShape, 6, topChildShape);
    ConnRef *topToBottomConn = connectShapes(topChildShape, 5, bottomChildShape);

    router->processTransaction();
    router->outputDiagramSVG(IMAGE_OUTPUT_PATH "output/HierarchicalOrthogonalRouter_TwoChildrenVertically");

    std::vector<Point> expectedBottomToTop = { {616.26, 780.244}, {612.26, 780.244}, {612.26, 647.779}, {716.26, 647.779} };
    EXPECT_THAT(bottomToTopConn, IsEqualToRoute(expectedBottomToTop));
    std::vector<Point> expectedTopToBottom = { {616.26, 579.279}, {608.26, 579.279}, {608.26, 848.744}, {716.26, 848.744} };
    EXPECT_THAT(topToBottomConn, IsEqualToRoute(expectedTopToBottom));
}

TEST_F(HierarchicalOrthogonalRouter, ThreeChildrenVertically) {
    ShapeRef *topChildShape = addChild({ 616.26, 565.279 }, { 816.26, 730.279 }, 2, 5);
    ShapeRef *bottomChildShape = addChild({ 616.26, 766.244 }, { 816.26, 931.244 }, 3, 6);
    ShapeRef *leftChildShape = addChild({145.954, 396.512}, {345.954, 617.512}, 4, 7, 8);

    ConnRef *bottomToTopConn = connectShapes(bottomChildShape, 6, topChildShape);
    ConnRef *topToBottomConn = connectShapes(topChildShape, 5, bottomChildShape);
    ConnRef *leftToTopConn = connectShapes(leftChildShape, 7, topChildShape);
    ConnRef *leftToBottomConn = connectShapes(leftChildShape, 8, bottomChildShape);

    router->processTransaction();
    router->outputDiagramSVG(IMAGE_OUTPUT_PATH "output/HierarchicalOrthogonalRouter_ThreeChildrenVertically");

    std::vector<Point> expectedBottomToTop = { {616.26, 780.244}, {612.26, 780.244}, {612.26, 647.779}, {716.26, 647.779} };
    EXPECT_THAT(bottomToTopConn, IsEqualToRoute(expectedBottomToTop));
    std::vector<Point> expectedTopToBottom = { {616.26, 579.279}, {608.26, 579.279}, {608.26, 848.744}, {716.26, 848.744} };
    EXPECT_THAT(topToBottomConn, IsEqualToRoute(expectedTopToBottom));
    std::vector<Point> expectedLeftToTop = { {345.954, 410.512}, {718.26, 410.512}, {718.26, 647.779} };
    EXPECT_THAT(leftToTopConn, IsEqualToRoute(expectedLeftToTop));
    std::vector<Point> expectedLeftToBottom = { {345.954, 452.512}, {714.26, 452.512}, {714.26, 848.744} };
    EXPECT_THAT(leftToBottomConn, IsEqualToRoute(expectedLeftToBottom));
}
