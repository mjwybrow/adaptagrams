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

	private long sleepTime = 0;

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
		logger.fine("block order: ");
		for (Variable v : vs) {
			logger.fine("  "+v);
		}
		for (Variable v : vs) {
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
	protected double refine() {
		while (split) {
			animate();
			Constraint splitConstraint = blocks.splitOnce(this);
			if (splitConstraint == null)
				break;
			animate();
			assert constraints.violated().isEmpty() : "Violated constraints not resolved";
			activeConstraints.add(splitConstraint);
			logger.finer("split->" + blocks);
			logger.finer("Cost:" + blocks.cost());
		}

		animate();
		assert constraints.violated().isEmpty() : "Violated constraints not resolved";
		logger.finer("Final->" + blocks);
		logger.fine("Cost:" + blocks.cost());
		return blocks.cost();		
	}
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
		logger.fine("variables: " + blocks.getAllVariables());
		logger.fine("constraints: " + constraints);
		
		animate();
		satisfyConstraints();
		if(!constraints.violated().isEmpty()) {
			logger.severe("Violated constraints not resolved: "+constraints.violated());
		}
		assert constraints.violated().isEmpty() : "Violated constraints not resolved";
		logger.finer("merged->" + blocks);
		logger.finer("Cost:" + blocks.cost());
		return refine();
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
