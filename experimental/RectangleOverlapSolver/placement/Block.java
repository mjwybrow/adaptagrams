/*
 * Created on 21/02/2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package placement;

import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A block is a grouping of variables placed optimally with respect to a set of
 * active constraints
 * 
 * @author dwyer
 */

public class Block {
	static Logger logger = ActiveSetPlacement.logger;

	static long timeCtr = 0;

	long timeStamp = 0;

	double position = 0;

	double weight = 0;

	double weightedPosition;

	Variables variables = new Variables();

	/**
	 * Priority queues for managing external constraints. Lazily instantiated as
	 * needed by merge operations.
	 */
	MaxPriorityQueue<Constraint> inConstraintsPriorityQueue = null;

	MaxPriorityQueue<Constraint> outConstraintsPriorityQueue = null;

	/**
	 * empty block constructor used for split.
	 */
	Block() {
	}

	/**
	 * Initially each variable is assigned its own block with position set to
	 * the desired position of the variable
	 * 
	 * @param v
	 */
	Block(Variable v) {
		addVariable(v);
		position = v.desiredPosition;
		weightedPosition = v.weight * position;
		v.offset = 0;
	}

	/**
	 * Merges b into this block across c. Can be either a right merge or a left
	 * merge
	 * 
	 * @param b
	 *            block to merge into this
	 * @param c
	 *            constraint being merged
	 * @param distance
	 *            separation required to satisfy c
	 */
	void merge(Block b, Constraint c, double distance) {
		weightedPosition += b.weightedPosition - distance * b.weight;
		weight += b.weight;
		position = weightedPosition / weight;
		c.active = true;
		for (Variable v : b.variables) {
			v.container = this;
			v.offset += distance;
		}
		variables.addAll(b.variables);
	}

	/**
	 * Computes lagrange multipliers across active constraints. Traverses the
	 * active set of constraints in this block with a depth first traversal,
	 * computing the lagrangian multipliers associated with each constraint from
	 * the derivative of the cost function (dfdv) for each variable
	 */
	Constraint findMinLM() {
		Variable f = variables.get(0);
		resetActiveLM(f, null);
		minLM = null;
		compute_dfdv(f, null);
		return minLM;
	}

	private Constraint minLM;

	private boolean canFollowLeft(Constraint c, Variable last) {
		return c.left.container == this && c.active && last != c.left;
	}

	private boolean canFollowRight(Constraint c, Variable last) {
		return c.right.container == this && c.active && last != c.right;
	}

	// resets LMs for all active constraints to 0 by
	// traversing active constraint tree starting from v,
	// not back tracking over u
	private void resetActiveLM(Variable v, Variable u) {
		for (Constraint c : v.outConstraints) {
			if (canFollowRight(c, u)) {
				c.lm = 0;
				resetActiveLM(c.right, v);
			}
		}
		for (Constraint c : v.inConstraints) {
			if (canFollowLeft(c, u)) {
				c.lm = 0;
				resetActiveLM(c.left, v);
			}
		}
	}

	// computes the derivative of v and the lagrange multipliers
	// of v's out constraints (as the recursive sum of those below.
	// Does not backtrack over u.
	// also records the constraint with minimum lagrange multiplier
	// in minLM
	private double compute_dfdv(Variable v, Variable u) {
		// how much v wants to go right (negative if v wants to go left)
		double dfdv = v.weight * (v.getPosition() - v.desiredPosition);
		for (Constraint c : v.outConstraints) {
			if (canFollowRight(c, u)) {
				// if the following call returns a negative then it means the
				// stuff to the right of v wants to go left
				c.lm = compute_dfdv(c.right, v);
				dfdv += c.lm;
				if (minLM == null || c.lm < minLM.lm)
					minLM = c;
			}
		}
		for (Constraint c : v.inConstraints) {
			if (canFollowLeft(c, u)) {
				// if the following call returns a negative then it means the
				// stuff to the left of v wants to go left
				c.lm = -compute_dfdv(c.left, v);
				dfdv -= c.lm;
				if (minLM == null || c.lm < minLM.lm)
					minLM = c;
			}
		}
		return dfdv;
	}

	/**
	 * @return ideal position of block in order to minimise distances of vars to
	 *         their desired positions
	 */
	double desiredPosition() {
		double wp = 0;
		for (Variable v : variables) {
			wp += (v.desiredPosition - v.offset) * v.weight;
		}
		return wp / weight;
	}

	/**
	 * if the block can be split across the constraint and two resultant blocks
	 * moved (without violating the constraint) to better satisfy desired
	 * positions then split will do so.
	 * 
	 * @return list of 2 blocks (result of split) or null if such a split does
	 *         not allow variables to move closer to desired positions
	 */
	void split(Constraint splitConstraint, Block l, Block r) {
		splitConstraint.active = false;
		populateSplitBlock(l, splitConstraint.left, splitConstraint.right);
		populateSplitBlock(r, splitConstraint.right, splitConstraint.left);
		if (logger.isLoggable(Level.FINER))
			logger.finer("Splitting on: " + this + splitConstraint);
	}

	// populates block b by traversing the active constraint tree adding
	// variables as they're visited.
	// Starts from variable v and does not backtrack over variable u.
	private void populateSplitBlock(Block b, Variable v, Variable u) {
		b.addVariable(v);
		for (Constraint c : v.inConstraints) {
			if (canFollowLeft(c, u)) {
				populateSplitBlock(b, c.left, v);
			}
		}
		for (Constraint c : v.outConstraints) {
			if (canFollowRight(c, u)) {
				populateSplitBlock(b, c.right, v);
			}
		}
	}

	private void addVariable(Variable v) {
		variables.add(v);
		weight += v.weight;
		weightedPosition += v.weight * (v.desiredPosition - v.offset);
		position = weightedPosition / weight;
		v.container = this;
	}

	public void setUpInConstraints() {
		inConstraintsPriorityQueue = new MaxPairingHeap<Constraint>();
		for (Variable v : variables) {
			for (Constraint c : v.inConstraints) {
				c.timeStamp = timeCtr;
				if (c.left.container != this) {
					inConstraintsPriorityQueue.add(c);
				}
			}
		}
	}

	public void setUpOutConstraints() {
		outConstraintsPriorityQueue = new MaxPairingHeap<Constraint>();
		for (Variable v : variables) {
			for (Constraint c : v.outConstraints) {
				if (c.right.container != this) {
					outConstraintsPriorityQueue.add(c);
				}
			}
		}
	}

	public String toString() {
		String s = "Block:";
		for (Variable v : variables) {
			s = s + v + " ";
		}
		return s;
	}

	public double cost() {
		double c = 0;
		for (Variable v : variables) {
			double diff = v.getPosition() - v.desiredPosition;
			c += v.weight * diff * diff;
		}
		return c;
	}

	Constraint findMaxInConstraint() {
		logger.fine("Constraint heap: " + inConstraintsPriorityQueue);
		Constraint v = inConstraintsPriorityQueue.findMax();
		ArrayList<Constraint> outOfDate = new ArrayList<Constraint>();
		while (v != null) {
			Block lb = v.left.container;
			Block rb = v.right.container;
			logger.finer("Examining constraint v=" + v + "\n  timestamps: lb="
					+ lb.timeStamp + " rb=" + rb.timeStamp + " v="
					+ v.timeStamp);
			if (lb == rb) {
				// constraint has been merged into the same block
				inConstraintsPriorityQueue.deleteMax();
				v = inConstraintsPriorityQueue.findMax();
				logger.finer("  Skipping internal!");
			} else if (v.timeStamp < lb.timeStamp) {
				// block at other end of constraint has been moved since this
				inConstraintsPriorityQueue.deleteMax();
				outOfDate.add(v);
				logger.finer("  Skipping out of date!");
				v = inConstraintsPriorityQueue.findMax();
			} else {
				break;
			}
		}
		for(Constraint c : outOfDate) {
			c.timeStamp = timeCtr;
			inConstraintsPriorityQueue.add(c);
		}
		v = inConstraintsPriorityQueue.findMax();
		return v;
	}

	Constraint findMaxOutConstraint() {
		Constraint v = outConstraintsPriorityQueue.findMax();
		while (v != null && v.left.container == v.right.container) {
			v = outConstraintsPriorityQueue.deleteMax();
			v = outConstraintsPriorityQueue.findMax();
		}
		return v;
	}

	public void mergeInConstraints(Block b) {
		// We check the top of the heaps to remove possible internal constraints
		findMaxInConstraint();
		b.findMaxInConstraint();
		inConstraintsPriorityQueue.merge(b.inConstraintsPriorityQueue);
		logger.fine("Constraint heaps merged: " + inConstraintsPriorityQueue);
	}

	public void mergeOutConstraints(Block b) {
		// We check the top of the heaps to remove possible internal constraints
		findMaxOutConstraint();
		b.findMaxOutConstraint();
		inConstraintsPriorityQueue.merge(b.inConstraintsPriorityQueue);
		logger.fine("Constraint heaps merged: " + inConstraintsPriorityQueue);
	}

}
