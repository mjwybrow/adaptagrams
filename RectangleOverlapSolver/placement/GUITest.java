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

	private static final int VIEWTIME = 0;

	double iScale = 3;

	static int runID = 0;

	RectangleView makeRect(String label, int x, int X, int y, int Y) {
		double dx = x * iScale, dy = y * iScale, dX = X * iScale, dY = Y
				* iScale;
		return new RectangleView(label, dx, dy, dX - dx, dY - dy);
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
		runOverlapRemover();
	}

	public void testCase5() {
		double rs[][] = new double[][] { { 7, 22, 39, 54 }, { 7, 33, 0, 59 },
				{ 3, 26, 16, 56 }, { 7, 17, 18, 20 }, { 1, 59, 11, 26 },
				{ 19, 20, 13, 49 }, { 1, 10, 0, 4 }, { 47, 52, 1, 3 } };
		int ctr = 0;
		iScale = 1;
		for (double r[] : rs) {
			rdFrame.d.rectangles.add(makeRect("r" + ctr++, r[0], r[1], r[2],
					r[3]));
		}
		runOverlapRemover();
	}

	public void testCase6() {
		double rs[][] = new double[][] { { 40, 69, 63, 69 }, { 1, 5, 27, 64 },
				{ 34, 66, 20, 22 }, { 1, 24, 10, 25 }, { 1, 19, 9, 61 },
				{ 0, 56, 8, 70 }, { 33, 35, 13, 28 }, { 11, 31, 33, 35 },
				{ 12, 22, 3, 23 } };
		int ctr = 0;
		iScale = 5;
		for (double r[] : rs) {
			rdFrame.d.rectangles.add(makeRect("r" + ctr++, r[0], r[1], r[2],
					r[3]));
		}
		runOverlapRemover();
	}

	public void testCase7() {
		double rs[][] = new double[][] {
				{ 341.594, 388.459, 373.491, 518.168 },
				{ 271.214, 324.782, 311.332, 409.166 },
				{ 293.848, 475.064, 305.194, 391.162 },
				{ 255.317, 447.738, 342.671, 489.923 },
				{ 228.375, 261.057, 206.422, 327.794 },
				{ 383.552, 462.834, 363.132, 412.843 },
				{ 288.859, 481.054, 351.895, 497.728 },
				{ 201.307, 368.511, 387.02, 394.95 },
				{ 257.961, 259.673, 386.503, 518.403 },
				{ 200.178, 275.606, 364.968, 466.787 } };
		int ctr = 0;
		iScale = 5;
		for (double r[] : rs) {
			rdFrame.d.rectangles.add(makeRect("r" + ctr++, r[0], r[1], r[2],
					r[3]));
		}
		runOverlapRemover();
	}

	public void testCase8() {
		double rs[][] = new double[][] {
				{ 300.763, 399.668, -48.994, 9.37972 },
				{ 292.944, 472.907, 586.945, 771.251 },
				{ 213.576, 316.07, 390.138, 539.323 },
				{ 397.777, 557.537, 269.46, 407.065 },
				{ 211.573, 355.103, 329.964, 390.137 },
				{ 356.053, 439.284, 483.416, 657.366 },
				{ 380.165, 561.491, -117.362, -68.3409 },
				{ 331.094, 410.119, 399.923, 491.138 },
				{ 239.304, 434.553, 9.37982, 63.0763 },
				{ 234.779, 356.548, 379.936, 454.313 },
				{ 321.638, 450.396, 771.251, 932.621 },
				{ 200.203, 337.728, 164.883, 280.221 },
				{ 218.8, 269.785, 96.999, 122.443 },
				{ 361.936, 459.41, 329.803, 434.498 },
				{ 337.221, 376.991, 140.796, 281.642 },
				{ 310.252, 439.023, 371.812, 402.413 },
				{ 254.891, 394.928, 881.542, 1019.28 },
				{ 235.764, 357.198, 192.217, 362.08 },
				{ 220.461, 221.906, 245.827, 349.642 },
				{ 230.853, 391.069, -123.824, -117.363 },
				{ 386.097, 435.297, 309.264, 504.577 },
				{ 264.3, 424.637, 591.648, 654.27 },
				{ 215.501, 307.432, 539.323, 586.254 },
				{ 395.533, 531.528, -81.995, -4.39529 },
				{ 358.484, 479.044, 189.017, 329.803 },
				{ 244.895, 425.449, -68.3408, -48.9941 },
				{ 333.52, 435.568, 180.205, 324.778 },
				{ 386.33, 473.66, 299.321, 363.526 },
				{ 209.677, 231.608, 362.947, 442.021 },
				{ 378.9, 568.762, 544.595, 674.377 },
				{ 329.325, 408.885, 163.78, 188.981 },
				{ 203.549, 254.138, 369.055, 565.234 },
				{ 363.038, 552.984, 138.776, 269.46 },
				{ 321.494, 398.329, 63.0764, 163.78 },
				{ 238.098, 434.199, 654.27, 780.879 },
				{ 220.68, 413.679, 487.039, 628.287 },
				{ 331.987, 377.805, 115.361, 129.283 },
				{ 366.68, 514.56, 115.543, 189.017 },
				{ 372.306, 409.058, 324.778, 354.862 },
				{ 324.464, 410.476, 178.834, 315.801 },
				{ 334.489, 418.235, 315.801, 371.812 },
				{ 267.432, 329.374, 243.969, 439.479 },
				{ 278.759, 291.041, 211.517, 284.993 },
				{ 340.121, 366.46, 654.27, 780.914 },
				{ 317.692, 449.673, 354.862, 487.971 },
				{ 390.997, 453.852, 674.377, 720.033 },
				{ 355.829, 549.874, -4.39519, 138.776 },
				{ 241.73, 346.358, 362.08, 561.245 },
				{ 292.723, 379.273, 242.404, 281.642 },
				{ 309.36, 318.712, 125.738, 139.197 },
				{ 230.827, 302.345, 586.254, 731.225 },
				{ 247.768, 369.572, 454.313, 487.039 },
				{ 279.643, 297.273, 262.057, 446.171 },
				{ 324.852, 427.59, 487.972, 620.33 },
				{ 397.528, 530.053, 61.6616, 115.542 },
				{ 301.807, 354.872, 271.221, 313.688 },
				{ 362.555, 511.138, 363.526, 483.416 },
				{ 230.933, 243.138, 345.959, 412.732 },
				{ 213.862, 379.945, 329.011, 371.812 },
				{ 238.36, 397.895, 139.197, 271.22 },
				{ 354.865, 362.518, 395.361, 592.164 },
				{ 208.111, 305.932, 209.846, 362.947 },
				{ 359.758, 547.687, 674.377, 864.503 },
				{ 333.893, 381.46, 654.27, 803.239 },
				{ 374.708, 427.011, 132.488, 180.205 },
				{ 355.037, 386.194, 356.862, 551.483 },
				{ 255.138, 343.834, 313.688, 379.936 },
				{ 342.985, 518.7, 720.033, 864.211 },
				{ 230.522, 296.752, 124.143, 125.738 },
				{ 354.737, 468.323, 189.017, 299.321 },
				{ 338.078, 442.819, 487.972, 586.945 },
				{ 319.65, 428.994, 45.1203, 180.205 },
				{ 316.785, 345.795, 205.242, 331.095 },
				{ 376.016, 568.244, 407.065, 544.595 },
				{ 303.117, 314.749, 106.308, 124.142 },
				{ 323.809, 346.726, 372.922, 445.778 },
				{ 319.902, 478.33, 434.498, 632.13 },
				{ 310.011, 383.567, 432.665, 521.371 },
				{ 399.723, 540.021, -159.31, -81.9951 },
				{ 312.928, 462.85, -68.3408, 45.1202 },
				{ 234.251, 259.691, 330.621, 434.599 },
				{ 241.899, 376.556, 628.287, 790.087 },
				{ 290.924, 406.743, 432.665, 591.648 },
				{ 318.801, 399.261, 188.981, 281.642 },
				{ 388.388, 430.194, 208.614, 379.542 },
				{ 257.764, 289.913, 280.221, 284.993 },
				{ 372.082, 390.973, 374.781, 427.304 },
				{ 209.496, 215.993, 346.893, 451.783 },
				{ 381.404, 401.776, 305.724, 427.215 },
				{ 286.623, 295.377, 326.399, 460.316 },
				{ 249.832, 319.766, 390.138, 405.821 },
				{ 336.459, 351.215, 780.879, 866.931 },
				{ 229.438, 337.452, 122.443, 284.993 },
				{ 277.601, 375.128, -74.4538, -68.3409 },
				{ 281.175, 417.039, 129.283, 242.404 },
				{ 366.766, 494.05, 306.843, 407.065 },
				{ 222.354, 413.583, 780.879, 881.542 },
				{ 212.266, 336.013, 284.993, 329.964 },
				{ 272.728, 432.243, 281.642, 315.801 },
				{ 200.539, 390.202, 402.413, 432.665 } };
		int ctr = 0;
		iScale = 1;
		for (double r[] : rs) {
			rdFrame.d.rectangles.add(makeRect("r" + ctr++, r[0], r[1], r[2],
					r[3]));
		}

		runOverlapRemover();
	}

	public void testRandom() {
		rdFrame.d.generateRandom(5);
		runOverlapRemover();
	}

	private void runOverlapRemover() {
		//rdFrame.algorithm = RectangleDrawerFrame.Algorithm.NATIVE_ACTIVESET;
		rdFrame.split=false;
		try {
			rdFrame.cleanup(0, 0);
			Thread.sleep(VIEWTIME);
			for (RectangleView r1 : rdFrame.d.rectangles) {
				Rectangle2D u = new Rectangle2D.Double(r1.x, r1.y,
						r1.width - 0.001, r1.height - 0.001);
				for (RectangleView r2 : rdFrame.d.rectangles) {
					if (r1 != r2) {
						Rectangle2D v = new Rectangle2D.Double(r2.x, r2.y,
								r2.width - 0.001, r2.height - 0.001);
						assertFalse(r1.label + " intersects " + r2.label, u
								.intersects(v));
					}
				}
			}
		} catch (AssertionFailedError e) {
			rdFrame.d.save("after_" + getName() + runID);
			rdFrame.d.undo();
			rdFrame.d.save("before_" + getName() + runID++);
			throw e;
		} catch (AssertionError e) {
			rdFrame.d.save("after_" + getName() + runID);
			rdFrame.d.undo();
			rdFrame.d.save("before_" + getName() + runID++);
			throw e;
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		rdFrame.d.clear();
	}

	public static void main(String[] args) {
		TestRunner.run(new RepeatedTest(GUITest.suite(), 200));
	}
}
