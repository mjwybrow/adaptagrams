/*
 * Regression test for ShapeRef::transformConnectionPinPositions() on a non-square shape.
 * Produces output diagrams and validates pin positions/directions after each transform.
 */

#include <cmath>
#include <iostream>
#include "libavoid/libavoid.h"
#include "libavoid/connectionpin.h"

static bool approx(double a, double b)
{
	return std::fabs(a - b) < 1e-6;
}

static bool samePoint(const Avoid::Point& a, const Avoid::Point& b)
{
	return approx(a.x, b.x) && approx(a.y, b.y);
}

static Avoid::Point transformPoint(const Avoid::Point& p, const Avoid::Point& c, Avoid::ShapeTransformationType t)
{
	const double x = p.x - c.x;
	const double y = p.y - c.y;

	switch (t) {
		case Avoid::TransformationType_CW90:
			return Avoid::Point(c.x - y, c.y + x);
		case Avoid::TransformationType_CW180:
			return Avoid::Point(c.x - x, c.y - y);
		case Avoid::TransformationType_CW270:
			return Avoid::Point(c.x + y, c.y - x);
		case Avoid::TransformationType_FlipX:
			return Avoid::Point(c.x - x, c.y + y);
		case Avoid::TransformationType_FlipY:
			return Avoid::Point(c.x + x, c.y - y);
	}
	return p;
}

static Avoid::ConnDirFlags transformDirs(Avoid::ConnDirFlags dirs, Avoid::ShapeTransformationType t)
{
	switch (t) {
		case Avoid::TransformationType_CW90:
			if (dirs == Avoid::ConnDirLeft) return Avoid::ConnDirUp;
			if (dirs == Avoid::ConnDirUp) return Avoid::ConnDirRight;
			if (dirs == Avoid::ConnDirRight) return Avoid::ConnDirDown;
			if (dirs == Avoid::ConnDirDown) return Avoid::ConnDirLeft;
			return dirs;

		case Avoid::TransformationType_CW180:
			if (dirs == Avoid::ConnDirLeft) return Avoid::ConnDirRight;
			if (dirs == Avoid::ConnDirRight) return Avoid::ConnDirLeft;
			if (dirs == Avoid::ConnDirUp) return Avoid::ConnDirDown;
			if (dirs == Avoid::ConnDirDown) return Avoid::ConnDirUp;
			return dirs;

		case Avoid::TransformationType_CW270:
			if (dirs == Avoid::ConnDirLeft) return Avoid::ConnDirDown;
			if (dirs == Avoid::ConnDirDown) return Avoid::ConnDirRight;
			if (dirs == Avoid::ConnDirRight) return Avoid::ConnDirUp;
			if (dirs == Avoid::ConnDirUp) return Avoid::ConnDirLeft;
			return dirs;

		case Avoid::TransformationType_FlipX:
			if (dirs == Avoid::ConnDirLeft) return Avoid::ConnDirRight;
			if (dirs == Avoid::ConnDirRight) return Avoid::ConnDirLeft;
			return dirs;

		case Avoid::TransformationType_FlipY:
			if (dirs == Avoid::ConnDirUp) return Avoid::ConnDirDown;
			if (dirs == Avoid::ConnDirDown) return Avoid::ConnDirUp;
			return dirs;
	}
	return dirs;
}

struct ExpectedPin {
	Avoid::Point pos;
	Avoid::ConnDirFlags dirs;
};

static bool checkPin(
	const char* stepName,
	const char* pinName,
	Avoid::ShapeConnectionPin* pin,
	const Avoid::Polygon& poly,
	const ExpectedPin& expected)
{
	bool ok = true;
	const Avoid::Point actualPos = pin->position(poly);
	const Avoid::ConnDirFlags actualDirs = pin->directions();

	if (!samePoint(actualPos, expected.pos)) {
		std::cerr
			<< stepName << " " << pinName << " position mismatch: "
			<< "got (" << actualPos.x << ", " << actualPos.y << "), "
			<< "expected (" << expected.pos.x << ", " << expected.pos.y << ")\n";
		ok = false;
	}

	if (actualDirs != expected.dirs) {
		std::cerr
			<< stepName << " " << pinName << " direction mismatch: "
			<< "got " << actualDirs << ", expected " << expected.dirs << "\n";
		ok = false;
	}

	return ok;
}

static bool validateStep(
	const char* stepName,
	Avoid::ShapeRef* shapeRef,
	Avoid::ShapeConnectionPin* pin1,
	Avoid::ShapeConnectionPin* pin2,
	const ExpectedPin& expected1,
	const ExpectedPin& expected2)
{
	const Avoid::Polygon& poly = shapeRef->polygon();
	bool ok = true;
	ok &= checkPin(stepName, "pin1", pin1, poly, expected1);
	ok &= checkPin(stepName, "pin2", pin2, poly, expected2);
	return ok;
}

static void applyExpectedTransform(
	ExpectedPin& pin1,
	ExpectedPin& pin2,
	const Avoid::Point& center,
	Avoid::ShapeTransformationType t)
{
	pin1.pos = transformPoint(pin1.pos, center, t);
	pin1.dirs = transformDirs(pin1.dirs, t);

	pin2.pos = transformPoint(pin2.pos, center, t);
	pin2.dirs = transformDirs(pin2.dirs, t);
}

int main(void)
{
	int failures = 0;

	Avoid::Router* router = new Avoid::Router(Avoid::OrthogonalRouting);
	router->setRoutingPenalty(Avoid::segmentPenalty, 50);

	Avoid::Rectangle shapeRect1(Avoid::Point(0, 0), Avoid::Point(10, 10));
	Avoid::ShapeRef* shapeRef1 = new Avoid::ShapeRef(router, shapeRect1);

	Avoid::Rectangle shapeRect2(Avoid::Point(0, 90), Avoid::Point(20, 100));
	Avoid::ShapeRef* shapeRef2 = new Avoid::ShapeRef(router, shapeRect2);

	// Non-square target shape.
	Avoid::Rectangle shapeRect3(Avoid::Point(50, 40), Avoid::Point(110, 80));
	Avoid::ShapeRef* shapeRef3 = new Avoid::ShapeRef(router, shapeRect3);

	const unsigned int PIN1 = 1;
	const unsigned int PIN2 = 2;

	// Two pins on the target shape, on opposite sides, using absolute coordinates.
	Avoid::ShapeConnectionPin* pin1 = new Avoid::ShapeConnectionPin(
		shapeRef3, PIN1, Avoid::ATTACH_POS_MIN_OFFSET, 10.0, false, 0.0, Avoid::ConnDirLeft);

	Avoid::ShapeConnectionPin* pin2 = new Avoid::ShapeConnectionPin(
		shapeRef3, PIN2, Avoid::ATTACH_POS_MAX_OFFSET, 30.0, false, 0.0, Avoid::ConnDirRight);

	new Avoid::ShapeConnectionPin(
		shapeRef1, Avoid::CONNECTIONPIN_CENTRE,
		Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_CENTRE,
		true, 0.0, Avoid::ConnDirNone);

	new Avoid::ShapeConnectionPin(
		shapeRef2, Avoid::CONNECTIONPIN_CENTRE,
		Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_CENTRE,
		true, 0.0, Avoid::ConnDirNone);

	Avoid::ConnEnd srcEnd(shapeRef2, Avoid::CONNECTIONPIN_CENTRE);
	Avoid::ConnEnd dstEnd(shapeRef3, PIN2);
	new Avoid::ConnRef(router, srcEnd, dstEnd);

	srcEnd = Avoid::ConnEnd(shapeRef1, Avoid::CONNECTIONPIN_CENTRE);
	dstEnd = Avoid::ConnEnd(shapeRef3, PIN1);
	new Avoid::ConnRef(router, srcEnd, dstEnd);

	const Avoid::Point center(80.0, 60.0);

	ExpectedPin exp1{Avoid::Point(50.0, 50.0), Avoid::ConnDirLeft};
	ExpectedPin exp2{Avoid::Point(110.0, 70.0), Avoid::ConnDirRight};

	router->processTransaction();
	router->outputDiagram("output/connectionpinRegression-1");
	if (!validateStep("initial", shapeRef3, pin1, pin2, exp1, exp2)) {
		++failures;
	}

	auto transformShape = [&](Avoid::ShapeTransformationType t) {
		const Avoid::Polygon& poly = shapeRef3->polygon();
		Avoid::Polygon result((int)poly.size());
		for(size_t i = 0; i < poly.size(); ++i) {
			result.setPoint(i, transformPoint(poly.at(i), center, t));
		}
		router->moveShape(shapeRef3, result);
	};

	auto doStep = [&](const char* fileName, const char* stepName, Avoid::ShapeTransformationType t) {
		shapeRef3->transformConnectionPinPositions(t);
		transformShape(t);
		router->processTransaction();
		router->outputDiagram(fileName);

		applyExpectedTransform(exp1, exp2, center, t);
		if (!validateStep(stepName, shapeRef3, pin1, pin2, exp1, exp2)) {
			++failures;
		}
	};

	doStep("output/connectionpinRegression-2", "cw90",  Avoid::TransformationType_CW90);
	doStep("output/connectionpinRegression-3", "cw180", Avoid::TransformationType_CW180);
	doStep("output/connectionpinRegression-4", "cw270", Avoid::TransformationType_CW270);
	doStep("output/connectionpinRegression-5", "flipX", Avoid::TransformationType_FlipX);
	doStep("output/connectionpinRegression-6", "flipY", Avoid::TransformationType_FlipY);

	delete router;
	return failures == 0 ? 0 : 1;
}
