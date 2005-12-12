package placement;

import java.util.Random;

import placement.QPRectanglePlacement.Algorithm;

public class SimpleTest {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		RectangleDrawerPanel d = new RectangleDrawerPanel();
		Random rand = new Random();
		for (int i = 0; i < 200; i++) {
			d.generateRandom(10,rand,200,200);
			try {
				QPRectanglePlacement r = new QPRectanglePlacement(false, false,
						false, QPRectanglePlacement.Algorithm.ACTIVESET, 0, 0,
						false);
				r.place(d.rectangles);
			} catch (AssertionError e) {
				d.save("AutoTests/after"+i);
				d.undo();
				d.save("AutoTests/before"+i);
				e.printStackTrace();
			}
		}
	}
}
