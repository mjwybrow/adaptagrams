package placement;

import java.awt.geom.Rectangle2D;
import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.ObjectOutput;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.io.Writer;
import java.util.ArrayList;
import java.util.IdentityHashMap;
import java.util.Random;
import java.util.Vector;

class Result {
	int size;

	double k;

	Vector<Cost> costs = new Vector<Cost>();

	void writeToFile() {
		File f = new File("PerformanceResults.log");
		Writer output = null;
		try {
			// use buffering
			output = new BufferedWriter(new FileWriter(f, true));
			output.write(toString() + "\n");
			output.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public String toString() {
		String s = size + "," + k;
		for(Cost c:costs) {
			s+=","+c;
		}
		return s;
	}

	public void writeHeader() {
		File f = new File("PerformanceResults.log");
		Writer output = null;
		try {
			// use buffering
			output = new BufferedWriter(new FileWriter(f, true));
			output.write("size,k");
			for(Cost c:costs) {
				output.write(","+c.hdr());
			}
			output.write("\n");
			output.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}

class Cost {
	String name;
	long time;
	double xDisplacement;
	double yDisplacement;
	public double displacement;

	public String hdr() {
		return name+" time,"+name+" X,"+name+" Y,"+name+" tot";
	}
	public String toString() {
		String s = time + "," + xDisplacement+ "," + yDisplacement+ "," + displacement;
		return s;
	}
}

public class PerformanceTest {

	PerformanceTest() {
		ArrayList<Result> results = new ArrayList<Result>();
		boolean first=true;
		for (int size = 500; size <= 500; size += 10) {
			for (int i = 0; i < 10; i++) { // run 10 times at each size
				Result result = new Result();
				result.size = size;
				ArrayList<Rectangle2D> rs = generateRandom(result, 100.0, 100.0,
						Math.sqrt(size) / 5);
				QPRectanglePlacement p = new QPRectanglePlacement(true, false,
						false, QPRectanglePlacement.Algorithm.CACTIVESET, 0, 0,
						false);
				result.costs.add(run(p, rs,"SOL-neighbourlists"));
				p.constraintGenerator.neighbourLists=false;
				result.costs.add(run(p, rs,"SOL"));
				/*
				result.costs.add(run(
						new QPRectanglePlacement(false, false, false,
								QPRectanglePlacement.Algorithm.CACTIVESET, 0,
								0, false), rs,"SAT"));
				result.costs.add(run(new QPRectanglePlacement(true, false,
						false, QPRectanglePlacement.Algorithm.CACTIVESET, 0, 0,
						false), rs,"SOL"));
				result.costs.add(run(new QPRectanglePlacement(false, false,
						false, QPRectanglePlacement.Algorithm.MOSEK, 0, 0,
						false), rs,"QP"));
				result.costs.add(run(
						new QPRectanglePlacement(false, false, true,
								QPRectanglePlacement.Algorithm.CACTIVESET, 0,
								0, false), rs,"SAT_OO"));
				result.costs.add(run(new QPRectanglePlacement(true, false,
						true, QPRectanglePlacement.Algorithm.CACTIVESET, 0, 0,
						false), rs,"SOL_OO"));
				result.costs.add(run(
						new QPRectanglePlacement(false, false, true,
								QPRectanglePlacement.Algorithm.MOSEK, 0, 0,
								false), rs,"QP_OO"));
				result.costs.add(run(new FSA(0, 0), rs,"FSA"));
				*/
				results.add(result);
				if(first) {
					result.writeHeader();
					first=false;
				}
				result.writeToFile();
			}
		}
		for (Result r : results) {
			System.out.println(r);
		}

	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		PerformanceTest p = new PerformanceTest();
	}

	private ArrayList<Rectangle2D> copyRects(ArrayList<Rectangle2D> orig) {
		ArrayList<Rectangle2D> rs = new ArrayList<Rectangle2D>();
		for (Rectangle2D r : orig) {
			Rectangle2D r2 = new Rectangle2D.Double();
			r2.setRect(r);
			rs.add(r2);
		}
		return rs;
	}
	private Cost run(RectanglePlacement p, ArrayList<Rectangle2D> orig, String alg) {
		System.gc();
		Cost c = new Cost();
		ArrayList<Rectangle2D> rs = copyRects(orig);
		long t0 = System.currentTimeMillis();
		// TODO p.place(rects);
		c.time = System.currentTimeMillis() - t0;
		c.xDisplacement = 0;
		c.yDisplacement = 0;
		c.name = alg;
		double dx, dy;
		for (int i=0;i<rs.size();i++) {
			Rectangle2D r=rs.get(i);
			Rectangle2D o=orig.get(i);
			//assert(r.getWidth()==o.getWidth());
			//assert(r.getHeight()==o.getHeight());
			//assert(r.getMinX()==rMap.get(r).getMinX());
			//assert(r.getMinY()==rMap.get(r).getMinY());
			c.xDisplacement += dx=Math.abs(r.getMinX() - o.getMinX());
			c.yDisplacement += dy=Math.abs(r.getMinY() - o.getMinY());
			c.displacement += Math.sqrt(dx * dx + dy * dy);
		}
		return c;
	}
	@Override
	protected void finalize() throws Throwable {
		// TODO Auto-generated method stub
		super.finalize();
	}

	private Cost run(RectanglePlacement p, ArrayList<Rectangle2D> orig, ArrayList<Rectangle2D> xplaced, String alg) {
		System.gc();
		Cost c = new Cost();
		ArrayList<Rectangle2D> rs = copyRects(orig);
		ArrayList<Rectangle2D> xrs = copyRects(xplaced);
		long t0 = System.currentTimeMillis();
		// TODO
		//((QPRectanglePlacement)p).placeX(rs,null,null);
		//((QPRectanglePlacement)p).placeY(xrs,null,null);
		c.time = System.currentTimeMillis() - t0;
		c.xDisplacement = 0;
		c.yDisplacement = 0;
		c.name = alg;
		for (int i=0;i<xrs.size();i++) {
			Rectangle2D r=rs.get(i);
			Rectangle2D xr=xrs.get(i);
			Rectangle2D o=orig.get(i);
			assert(r.getWidth()==o.getWidth());
			assert(r.getHeight()==o.getHeight());
			//assert(r.getMinX()==rMap.get(r).getMinX());
			//assert(r.getMinY()==rMap.get(r).getMinY());
			c.xDisplacement += Math.abs(r.getMinX() - o.getMinX());
			c.yDisplacement += Math.abs(xr.getMinY() - o.getMinY());
			//c.displacement += Math.sqrt(dx * dx + dy * dy);
		}
		return c;
	}
	static ArrayList<Rectangle2D> generateRandom(Result result, double w,
			double h, double rSize) {
		Random rand = new Random();
		ArrayList<Rectangle2D> rectangles = new ArrayList<Rectangle2D>();
		for (int i = 0; i < result.size; i++) {
			Rectangle2D r = new Rectangle2D.Double(w+ w * rand.nextDouble(), h+h
					* rand.nextDouble(), (w / rSize) * rand.nextDouble(),
					(h / rSize) * rand.nextDouble());
			rectangles.add(r);
		}
		int overlapCount = 0;
		for (int i = 0; i < rectangles.size(); i++) {
			Rectangle2D u = rectangles.get(i);
			for (int j = i + 1; j < rectangles.size(); j++) {
				Rectangle2D v = rectangles.get(j);
				if (u.intersects(v))
					overlapCount++;
			}
		}
		double k = (double) overlapCount / (double) result.size;
		System.out.println("Random graph has k=" + k);
		result.k = k;
		ObjectOutput output = null;
		File file = new File("randomrects.blocks");
		ArrayList<Rectangle2D> rs = new ArrayList<Rectangle2D>();
		rs.add(new Rectangle2D.Double(1,2,3,4));

		return rectangles;
	}

}
