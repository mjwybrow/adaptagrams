package placement;

import java.util.ArrayList;
import java.util.Observable;
import java.util.Observer;
import java.util.logging.Logger;

public class QPRectanglePlacement extends Observable implements
		RectanglePlacement, Observer {
	private static final double EXTRA_GAP = 0.0001;

	static Logger logger = Logger.getLogger(QPRectanglePlacement.class
			.getName());

	enum Algorithm {
		ACTIVESET, CACTIVESET, MOSEK
	}

	private Algorithm algorithm = Algorithm.ACTIVESET;

	private boolean animate;

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

	void placeX(ArrayList<RectangleView> rectangles) {

		double cost;
		long t1 = System.currentTimeMillis();
		XChunk[] xs = new XChunk[rectangles.size()];
		logger.fine("*****************Placing X");
		for (int i = 0; i < rectangles.size(); i++) {
			RectangleView r = rectangles.get(i);
			r.oldX = r.x;
			xs[i] = new XChunk(r, new YChunk(r));
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

	void replaceX(ArrayList<RectangleView> rectangles) {

		double cost;
		long t1 = System.currentTimeMillis();
		XChunk[] xs = new XChunk[rectangles.size()];
		logger.fine("*****************Placing X");
		for (int i = 0; i < rectangles.size(); i++) {
			RectangleView r = rectangles.get(i);
			r.moveTo(r.oldX, r.y);
			xs[i] = new XChunk(r, new YChunk(r));
			xs[i].id = r.label;
		}
		constraintGenerator.initVarsAndConstraints(xs, true);
		setChanged();
		notifyObservers();
		long t2 = System.currentTimeMillis();
		System.out.println("Gen hor. cs: time=" + (t2 - t1));
		cost = placement();
		long t3 = System.currentTimeMillis();
		System.out.println("Place hor.: cost=" + cost + " time=" + (t3 - t2));
	}
	public void place(ArrayList<RectangleView> rectangles) {
		long t1 = System.currentTimeMillis();
		if (algorithm == Algorithm.CACTIVESET) {
			placeNative(rectangles);
		} else {
			XChunk.g += EXTRA_GAP;
			YChunk.g += EXTRA_GAP;
			DebugPanel.direction=DebugPanel.Direction.Horizontal;
			placeX(rectangles);
			XChunk.g -= EXTRA_GAP;
			DebugPanel.direction=DebugPanel.Direction.Vertical;
			placeY(rectangles);
			DebugPanel.direction=DebugPanel.Direction.Horizontal;
			YChunk.g -= EXTRA_GAP;
			replaceX(rectangles);
		}
		long t2 = System.currentTimeMillis();
		//System.out.println("Total time=" + (t2 - t1));
	}

	void placeNative(ArrayList<RectangleView> rectangles) {
		// placeXNative(rectangles);
		// placeYNative(rectangles);
		SolveVPSC s = new SolveVPSC(rectangles);
		s.removeOverlaps();
	}

	void placeY(ArrayList<RectangleView> rectangles) {
		long t1 = System.currentTimeMillis();
		YChunk[] ys = new YChunk[rectangles.size()];
		logger.fine("*****************Placing Y");
		for (int i = 0; i < rectangles.size(); i++) {
			RectangleView r = rectangles.get(i);
			ys[i] = new YChunk(r, new XChunk(r));
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
			c.setCentre(c.v.getPosition());
		}
		setChanged();
		notifyObservers();
	}

}
