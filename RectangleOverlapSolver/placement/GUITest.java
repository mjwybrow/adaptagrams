package placement;

import java.awt.geom.Rectangle2D;

import junit.extensions.RepeatedTest;
import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;
import junit.textui.TestRunner;

public class GUITest extends TestCase {
	{
		rdFrame.init();
	}

	private static final RectangleDrawerFrame rdFrame = new RectangleDrawerFrame(
			"Rectangle Drawer");

	private static final int VIEWTIME = 100000;

	int iScale = 3;

	static int runID = 0;

	RectangleView makeRect(String label, int x, int X, int y, int Y) {
		x = x * iScale;
		y = y * iScale;
		X = X * iScale;
		Y = Y * iScale;
		return new RectangleView(label, x, y, X - x, Y - y);
	}
	RectangleView makeRect(String label, double x, double X, double y, double Y) {
		x = x * iScale;
		y = y * iScale;
		X = X * iScale;
		Y = Y * iScale;
		return new RectangleView(label, x, y, X - x, Y - y);
	}

	protected void setUp() {
	}

	public static Test suite() {
		return new TestSuite(GUITest.class);
	}

	public void testCase1() {
		int rs[][] = new int[][] { { 0, 50, 0, 30 }, { 10, 20, 10, 29 },
				{ 30, 70, 39, 70 }, { 0, 90, 40, 50 }, { 30, 70, 1, 29 } };

		iScale = 3;
		int ctr = 0;
		for (int r[] : rs) {
			rdFrame.d.rectangles.add(makeRect("r" + ctr++, r[0], r[1], r[2],
					r[3]));
		}
		runOverlapRemover();
	}

	public void testCase2() {
		int rs[][] = new int[][] { { 7, 22, 39, 54 }, { 7, 33, 0, 59 },
				{ 3, 26, 16, 56 }, { 7, 17, 18, 20 }, { 1, 59, 11, 26 },
				{ 19, 20, 13, 49 }, { 1, 10, 0, 4 }, { 47, 52, 1, 3 } };
		int ctr = 0;
		iScale = 10;
		for (int r[] : rs) {
			rdFrame.d.rectangles.add(makeRect("r" + ctr++, r[0], r[1], r[2],
					r[3]));
		}
		//rdFrame.algorithm = RectangleDrawerFrame.Algorithm.NATIVE_ACTIVESET;
		rdFrame.split = true;
		runOverlapRemover();
	}

	public void testCase3() {
		int rs[][] = new int[][] { { 8, 32, 29, 36 }, { 19, 24, 2, 27 },
				{ 4, 5, 27, 55 }, { 6, 7, 13, 26 }, { 3, 39, 46, 62 },
				{ 6, 23, 2, 19 }, { 18, 39, 5, 23 }, { 35, 63, 42, 78 },
				{ 16, 18, 14, 72 }, { 12, 32, 10, 58 } };
		int ctr = 0;
		iScale = 10;
		for (int r[] : rs) {
			rdFrame.d.rectangles.add(makeRect("r" + ctr++, r[0], r[1], r[2],
					r[3]));
		}
		//rdFrame.algorithm = RectangleDrawerFrame.Algorithm.NATIVE_ACTIVESET;
		runOverlapRemover();
	}

	public void testCase4() {
		double rs[][] = new double[][] {
				{ 315.755, 355.288, 353.595, 449.627 },
				{ 395.048, 395.635, 253.943, 362.228 },
				{ 254.439, 393.289, 278.708, 286.346 },
				{ 209.852, 370.831, 326.496, 507.255 },
				{ 271.947, 415.74, 362.228, 450.318 },
				{ 293.408, 405.197, 220.61, 244.119 },
				{ 276.482, 386.472, 286.346, 435.767 },
				{ 268.211, 436.23, 192.807, 220.61 },
				{ 378.008, 502.118, 358.437, 475.587 },
				{ 340.68, 472.597, 249.492, 335.448 } };
		int ctr = 0;
		iScale = 1;
		for (double r[] : rs) {
			rdFrame.d.rectangles.add(makeRect("r" + ctr++, r[0], r[1], r[2],
					r[3]));
		}
		//rdFrame.algorithm = RectangleDrawerFrame.Algorithm.NATIVE_ACTIVESET;
		rdFrame.algorithm = RectangleDrawerFrame.Algorithm.ACTIVESET;
		runOverlapRemover();
	}
	public void testCase5() {
		double rs[][] = new double[][] {
	 {7, 22, 39, 54},
	 {7, 33, 0, 59},
	 {3, 26, 16, 56},
	 {7, 17, 18, 20},
	 {1, 59, 11, 26},
	 {19, 20, 13, 49},
	 {1, 10, 0, 4},
	 {47, 52, 1, 3}};
		int ctr = 0;
		iScale = 1;
		for (double r[] : rs) {
			rdFrame.d.rectangles.add(makeRect("r" + ctr++, r[0], r[1], r[2],
					r[3]));
		}
		//rdFrame.algorithm = RectangleDrawerFrame.Algorithm.NATIVE_ACTIVESET;
		runOverlapRemover();
	}
	public void testRandom() {
		rdFrame.d.generateRandom(5);
		runOverlapRemover();
	}

	private void runOverlapRemover() {
		rdFrame.cleanup(0, 0);
		try {
			Thread.sleep(VIEWTIME);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		try {
			for (RectangleView r1 : rdFrame.d.rectangles) {
				Rectangle2D u = new Rectangle2D.Double(r1.x, r1.y, r1.width-1,
						r1.height-1);
				for (RectangleView r2 : rdFrame.d.rectangles) {
					if (r1 != r2) {
						Rectangle2D v = new Rectangle2D.Double(r2.x, r2.y,
								r2.width-1, r2.height-1);
						assertFalse(r1.label+" intersects "+r2.label,u.intersects(v));
					}
				}
			}
		} catch (AssertionFailedError e) {
			rdFrame.d.save("after_" + getName() + runID);
			rdFrame.d.undo();
			rdFrame.d.save("before_" + getName() + runID++);
			throw e;
		}
		rdFrame.d.clear();
	}

	public static void main(String[] args) {
		TestRunner.run(new RepeatedTest(GUITest.suite(), 200));
	}
}
