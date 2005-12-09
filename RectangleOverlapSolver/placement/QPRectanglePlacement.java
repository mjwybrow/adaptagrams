package placement;

import java.awt.Color;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Observable;
import java.util.Observer;
import java.util.logging.Logger;

public class QPRectanglePlacement extends Observable implements
		RectanglePlacement, Observer {
	static Logger logger = Logger.getLogger(QPRectanglePlacement.class
			.getName());

	enum Algorithm {
		ACTIVESET, CACTIVESET, MOSEK
	}

	private Algorithm algorithm = Algorithm.ACTIVESET;

	private boolean animate;

	private Hashtable<Rectangle2D, Color> rectangleColourMap;

	private boolean splitRefinement;

	public ConstraintGenerator constraintGenerator;

	/**
	 * @param completeConstraints
	 * @param animate
	 */
	public QPRectanglePlacement(boolean split, boolean completeConstraints,
			boolean orthogonalOrdering, Algorithm algorithm, double xgap,
			double ygap, boolean animate) {
		this.splitRefinement = split;
		this.constraintGenerator = new ConstraintGenerator(completeConstraints,
				true, orthogonalOrdering);
		this.algorithm = algorithm;
		this.animate = animate;
		XChunk.g = xgap;
		YChunk.g = ygap;
	}

	void placeXNative(ArrayList<PRect> rectangles) {
		SolveVPSC svpsc = new SolveVPSC(rectangles);
		double weights[] = new double[rectangles.size()];
		for (int i = 0; i < weights.length; i++) {
			weights[i] = 1.0;
		}
		svpsc.generateXConstraints(weights);
		try {
			svpsc.solve();
			Variables vs = svpsc.getVariables();
			for (int i = 0; i < vs.size(); i++) {
				Rectangle2D rect = rectangles.get(i).r;
				double min = vs.get(i).getPosition();
				if (rect instanceof java.awt.Rectangle) {
					// because awt Rectangles have int coords!
					min = Math.ceil(min);
				}
				rect.setRect(min, rect.getMinY(), rect.getWidth(), rect
						.getHeight());
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	void placeYNative(ArrayList<PRect> rectangles) {
		SolveVPSC svpsc = new SolveVPSC(rectangles);
		double weights[] = new double[rectangles.size()];
		for (int i = 0; i < weights.length; i++) {
			weights[i] = 1.0;
		}
		svpsc.generateYConstraints(weights);
		try {
			svpsc.solve();
			Variables vs = svpsc.getVariables();
			for (int i = 0; i < vs.size(); i++) {
				Rectangle2D rect = rectangles.get(i).r;
				double min = vs.get(i).getPosition();
				if (rect instanceof java.awt.Rectangle) {
					// because awt Rectangles have int coords!
					min = Math.ceil(min);
				}
				rect.setRect(rect.getMinX(), min, rect.getWidth(), rect
						.getHeight());
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	void placeX(ArrayList<PRect> rectangles) {

		double cost;
		long t1 = System.currentTimeMillis();
		XChunk[] xs = new XChunk[rectangles.size()];
		logger.fine("*****************Placing X");
		for (int i = 0; i < rectangles.size(); i++) {
			PRect r = rectangles.get(i);
			xs[i] = new XChunk(r.r, new YChunk(r.r));
			xs[i].id = r.label;
		}
		constraintGenerator.initVarsAndConstraints(xs, false);
		setChanged();
		notifyObservers();
		long t2 = System.currentTimeMillis();
		System.out.println("Gen hor. cs: time=" + (t2 - t1));
		cost = placement();
		long t3 = System.currentTimeMillis();
		System.out.println("Place hor.: cost=" + cost + " time=" + (t3 - t2));
	}

	public void place(ArrayList<PRect> rectangles) {
		long t1 = System.currentTimeMillis();
		if (algorithm == Algorithm.CACTIVESET) {
			placeNative(rectangles);
		} else {
			XChunk.g += 0.01;
			placeX(rectangles);
			XChunk.g -= 0.01;
			placeY(rectangles);
		}
		long t2 = System.currentTimeMillis();
		System.out.println("Total time=" + (t2 - t1));
	}

	void placeNative(ArrayList<PRect> rectangles) {
		long t1 = System.currentTimeMillis();
		// placeXNative(rectangles);
		// placeYNative(rectangles);
		SolveVPSC s = new SolveVPSC(rectangles);
		s.removeOverlaps();
		long t2 = System.currentTimeMillis();
		System.out.println("Total time=" + (t2 - t1));
	}

	void placeY(ArrayList<PRect> rectangles) {
		long t1 = System.currentTimeMillis();
		YChunk[] ys = new YChunk[rectangles.size()];
		logger.fine("*****************Placing Y");
		for (int i = 0; i < rectangles.size(); i++) {
			PRect r = rectangles.get(i);
			ys[i] = new YChunk(r.r, new XChunk(r.r));
			ys[i].id = r.label;
		}
		constraintGenerator.initVarsAndConstraints(ys, true);
		setChanged();
		notifyObservers();
		long t2 = System.currentTimeMillis();
		System.out.println("Gen ver. cs: time=" + (t2 - t1));

		double cost = placement();
		long t3 = System.currentTimeMillis();
		System.out.println("Place ver.: cost=" + cost + " time=" + (t3 - t1));
	}

	double placement() {
		double cost = 0;
		Chunk[] chunks = constraintGenerator.getChunks();
		Variable[] vs = new Variable[chunks.length];
		for (int i = 0; i < chunks.length; i++)
			vs[i] = chunks[i].v;
		Placement p = null;
		switch (algorithm) {
		case MOSEK:
			System.out.println("Running mosek with:\n   |V|=" + vs.length);
			p = new MosekPlacement(vs);
			break;
		case CACTIVESET:
			System.out.println("Running native AS with:\n   |V|=" + vs.length);
			p = new SolveVPSC(vs);
			((SolveVPSC) p).split = splitRefinement;
			break;
		default:
			System.out.println("Running AS with:\n   |V|=" + vs.length);
			p = new ActiveSetPlacement(vs);
			((ActiveSetPlacement) p).split = splitRefinement;
			((ActiveSetPlacement) p).debugAnimation = animate;
			((ActiveSetPlacement) p).addObserver(this);
		}
		for (Constraint c : constraintGenerator.getConstraints()) {
			p.addConstraint(c.left.name, c.right.name, c.separation);
		}
		System.out.println("   |C|=" + p.getConstraints().size());
		try {
			cost = p.solve();
			/*
			 * if (splitRefinement) { p = new MosekPlacement(p.getVariables(),
			 * p.getConstraints()); double mcost = p.solve();
			 * System.out.println("cost=" + cost + ", mcost=" + mcost); assert
			 * (2 * Math.abs(cost - mcost) / (1 + mcost + cost) < 0.001) :
			 * "Solver did not find optimal solution!"; }
			 */

		} catch (Exception e) {
			e.printStackTrace();
		}
		for (Chunk c : chunks) {
			c.setCentre(c.v.getPosition());
		}
		return cost;
	}

	public void addObserver(Observer o) {
		super.addObserver(o);
	}

	public void update(Observable arg0, Object arg1) {
		for (Chunk c : constraintGenerator.getChunks()) {
			c.setMin(c.v.getPosition());
			if (c.v.colour != null)
				rectangleColourMap.put(c.rect, c.v.colour);
		}
		setChanged();
		notifyObservers();
	}

}
