package placement;

import java.io.File;
import java.util.Calendar;
import java.util.Random;

public class SimpleTest {

	/**
	 * @param args
	 */
	public static void main(String[] args) {  
		Calendar c=Calendar.getInstance();
		String dir = "Test"+String.format("_%1$tF_%1$tH-%1$tM-%1$tS", c);
		if(!(new File(dir)).mkdirs()) throw new Error("Couldn't make directory!");
		RectangleDrawerPanel d = new RectangleDrawerPanel();
		Random rand = new Random();
		for (int i = 0; i < 100000; i++) {
			d.generateRandom(10,rand,200,200);
			try {
				QPRectanglePlacement r = new QPRectanglePlacement(false, false,
						false, QPRectanglePlacement.Algorithm.ACTIVESET, 0, 0,
						false);
				r.place(d.rectangles);
			} catch (AssertionError e) {
				d.save(dir+"/t"+i);
				e.printStackTrace();
			}
			d.clear();
		}
	}
}
