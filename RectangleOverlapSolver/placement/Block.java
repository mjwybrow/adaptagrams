/*
 * Created on 21/02/2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package placement;

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

	static long timeStampCtr = 0;

	long timeStamp = 0;

	double position = 0;

	double weight = 0;

	double weightedPosition;

	Constraints activeConstraints = new Constraints();

	Variables variables = new Variables();

	/**
	 * Priority queues for managing external constraints. Lazily instantiated as
	 * needed by merge operations.
	 */
	MaxPriorityQueue<Constraint> inConstraintsPriorityQueue = null;

	MaxPriorityQueue<Constraint> outConstraintsPriorityQueue = null;

	private boolean incomingChanged=false;

	private boolean merged=false;

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

	void merge(Block b, Constraint c, double distance) {
		weightedPosition += b.weightedPosition - distance * b.weight;
		weight += b.weight;
		position = weightedPosition / weight;
		activeConstraints.addAll(b.activeConstraints);
		activeConstraints.add(c);
		c.active = true;
		for (Variable v : b.variables) {
			v.container = this;
			v.offset += distance;
		}
		inConstraintsPriorityQueue.merge(b.inConstraintsPriorityQueue);
		if (outConstraintsPriorityQueue == null)
			setUpOutConstraints();
		if (b.outConstraintsPriorityQueue == null)
			b.setUpOutConstraints();
		outConstraintsPriorityQueue.merge(b.outConstraintsPriorityQueue);
		variables.addAll(b.variables);
		merged=true;
	}

	/**
	 * Computes lagrange multipliers across active constraints. Traverses the
	 * active set of constraints in this block with a depth first traversal,
	 * computing the lagrangian multipliers associated with each constraint from
	 * the derivative of the cost function (dfdv) for each variable
	 */
	void computeLagrangeMultipliers() {
		for (Constraint c : activeConstraints) {
			c.lagrangeMultiplier = 0;
		}
		compute_dfdv(variables.get(0), null);
	}

	private double compute_dfdv(Variable v, Variable u) {
		// how much v wants to go right (negative if v wants to go left)
		double dfdv = v.weight * (v.getPosition() - v.desiredPosition);
		for (Constraint c : v.outConstraints) {
			if (c.right.container == this && c.active && u != c.right) {
				// if the following call returns a negative then it means the
				// stuff to the right of v wants to go left
				c.lagrangeMultiplier = compute_dfdv(c.right, v);
				dfdv += c.lagrangeMultiplier;
			}
		}
		for (Constraint c : v.inConstraints) {
			if (c.left.container == this && c.active && u != c.left) {
				// if the following call returns a negative then it means the
				// stuff to the left of v wants to go left
				c.lagrangeMultiplier = -compute_dfdv(c.left, v);
				dfdv -= c.lagrangeMultiplier;
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

	void populateSplitBlock(Block b, Variable v, Variable u) {
		b.addVariable(v);
		for (Constraint c : v.inConstraints) {
			if (c.left.container == this && c.active && u != c.left) {
				b.activeConstraints.add(c);
				populateSplitBlock(b, c.left, v);
			}
		}
		for (Constraint c : v.outConstraints) {
			if (c.right.container == this && c.active && u != c.right) {
				b.activeConstraints.add(c);
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
		Constraint v = inConstraintsPriorityQueue.findMax();
		while (v != null) {
			Block lb = v.left.container;
			Block rb = v.right.container;
			if (lb == rb) {
				// constraint has been merged into the same block
				inConstraintsPriorityQueue.deleteMax();
				v = inConstraintsPriorityQueue.findMax();
			} else if (lb.timeStamp > rb.timeStamp && v.timeStamp < lb.timeStamp) {
				// block at other end of constraint has been moved since this
				inConstraintsPriorityQueue.deleteMax();
				v.timeStamp = ++Block.timeStampCtr;
				inConstraintsPriorityQueue.add(v);
				v = inConstraintsPriorityQueue.findMax();
				incomingChanged=true;
			} else {
				break;
			}
		}
		return v;
	}	Constraint findMaxOutConstraint() {
		Constraint v = outConstraintsPriorityQueue.findMax();
		while (v != null && v.left.container == v.right.container) {
			v = outConstraintsPriorityQueue.deleteMax();
			v = outConstraintsPriorityQueue.findMax();
		}
		return v;
	}

	public void updateTimeStamp() {
		//if(merged||incomingChanged) {
			timeStamp = ++timeStampCtr;
		//}
		incomingChanged=false;
		merged=false;
	}
}
