package placement;

import java.util.HashMap;
import java.util.List;
import java.util.Observable;
import java.util.logging.Level;
import java.util.logging.Logger;

public class ActiveSetPlacement extends Observable implements Placement {
	static Logger logger = Logger.getLogger("placement");

	Blocks blocks;

	boolean debugAnimation = false;

	/** list of constraints waiting to be processed (not yet assigned to blocks) */
	Constraints activeConstraints = new Constraints();

	/** canonical list of constraints */
	private Constraints constraints = new Constraints();

	HashMap<String, Variable> vlookup = new HashMap<String, Variable>();

	private DebugFrame debugFrame;

	private long sleepTime = 500;

	public boolean split = true;

	public Constraint addConstraint(String u, String v, double sep) {
		Constraint c = new Constraint(vlookup.get(u), vlookup.get(v), sep);
		constraints.add(c);
		return c;
	}

	/**
	 * Gives a feasible - though not necessarily optimal - solution by
	 * examining blocks in the partial order defined by the directed acyclic
	 * graph of constraints. For each block (when processing left to right) we
	 * maintain the invariant that all constraints to the left of the block
	 * (incoming constraints) are satisfied. This is done by repeatedly merging
	 * blocks into bigger blocks across violated constraints (most violated
	 * first) fixing the position of variables inside blocks relative to one
	 * another so that constraints internal to the block are satisfied.
	 */
	void satisfyConstraints() {
		List<Variable> vs = blocks.totalOrder();
		if (logger.isLoggable(Level.FINE)) {
			logger.fine("block order: " + blocks);
		}
		for (Variable v : vs) {
			System.out.println("Processing "+v.desiredPosition);
			blocks.mergeLeft(v.container, this);
		}
	}

	public ActiveSetPlacement(Variable[] vs) {
		this.vs=vs;
		for (Variable v : vs) {
			v.inConstraints = new Constraints();
			v.outConstraints = new Constraints();
		}
		blocks = new Blocks(vs);
		for (Variable v : vs) {
			vlookup.put(v.name, v);
		}
	}
	Variable[] vs;
	/**
	 * Calculate the optimal solution. After using satisfy() to produce a
	 * feasible solution, solve() examines each block to see if further
	 * refinement is possible by splitting the block. This is done repeatedly
	 * until no further improvement is possible.
	 */
	public double solve() {
		if (debugAnimation)
			debugFrame = new DebugFrame(blocks, constraints);
		// activeConstraints = blocks.getAllConstraints();
		if (logger.isLoggable(Level.FINE)) {
			logger.fine("variables: " + blocks.getAllVariables());
			logger.fine("sorted constraints: " + activeConstraints);
		}
		animate();
		satisfyConstraints();
		System.out.println("Constraints "+constraints);
		assert constraints.violated().isEmpty() : "Violated constraints not resolved";
		if (logger.isLoggable(Level.FINER))
			logger.finer("merged->" + blocks);
		if (logger.isLoggable(Level.FINER))
			logger.finer("Cost:" + blocks.cost());

		while (split) {
			animate();
			Constraint splitConstraint = blocks.splitOnce(this);
			if (splitConstraint == null)
				break;
			animate();
			assert constraints.violated().isEmpty() : "Violated constraints not resolved";
			activeConstraints.add(splitConstraint);
			if (logger.isLoggable(Level.FINER))
				logger.finer("split->" + blocks);
			if (logger.isLoggable(Level.FINER))
				logger.finer("Cost:" + blocks.cost());
		}

		animate();
		assert constraints.violated().isEmpty() : "Violated constraints not resolved";
		if (logger.isLoggable(Level.FINER))
			logger.finer("Final->" + blocks);
		if (logger.isLoggable(Level.FINE))
			logger.fine("Cost:" + blocks.cost());
		return blocks.cost();
	}

	void animate() {
		if (debugAnimation) {
			debugFrame.animate();
			setChanged();
			notifyObservers();
			try {
				Thread.sleep(sleepTime);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}

	public Constraints getConstraints() {
		return constraints;
	}

	public Variables getVariables() {
		return blocks.getAllVariables();
	}
}
