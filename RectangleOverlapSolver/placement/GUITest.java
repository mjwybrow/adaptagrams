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
	private static final RectangleDrawerFrame rdFrame = new RectangleDrawerFrame("Rectangle Drawer");
	private static final int VIEWTIME = 0;

	int iScale = 10;
	static int runID=0;

	RectangleView makeRect(String label, int x, int X, int y, int Y) {
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

	public void off_testCase1() {
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

	public void testRandom() {
		rdFrame.d.generateRandom(5);
		runOverlapRemover();
	}

	private void runOverlapRemover() {
		rdFrame.cleanup(1, 1);
		try {
			Thread.sleep(VIEWTIME);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		try {
			for (RectangleView r1 : rdFrame.d.rectangles) {
				Rectangle2D u = new Rectangle2D.Double(r1.x, r1.y, r1.width,
						r1.height);
				for (RectangleView r2 : rdFrame.d.rectangles) {
					if (r1 != r2) {
						Rectangle2D v = new Rectangle2D.Double(r2.x, r2.y,
								r2.width, r2.height);
						//assertFalse(u.intersects(v));
					}
				}
			}
		} catch (AssertionFailedError e) {
			rdFrame.d.save("AutoTests/after_"+getName()+runID);
			rdFrame.d.undo();
			rdFrame.d.save("AutoTests/before_"+getName()+runID++);
			throw e;
		}
	}
	public static void main(String[] args) {
		TestRunner.run(new RepeatedTest(GUITest.suite(),200));
	}
}
