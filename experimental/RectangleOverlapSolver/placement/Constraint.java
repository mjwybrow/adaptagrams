/*
 * Created on 21/02/2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package placement;

import java.awt.Color;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.Queue;

/**
 * @author dwyer_2 TODO To change the template for this generated type comment
 *         go to Window - Preferences - Java - Code Style - Code Templates
 */
public class Constraint implements Comparable<Constraint> {
	long timeStamp=0;
	Variable left;

	Variable right;

	double separation;

	boolean active = false;

	public Color colour = Color.WHITE;

	public double lm;

	Constraint(Variable left, Variable right, double separation) {
		this.left = left;
		this.right = right;
		this.separation = separation;
		left.addOutConstraint(this);
		right.addInConstraint(this);
	}

	boolean isViolated() {
		if (violatedAmount() > 0.00001) {
			return true;
		}
		return false;
	}

	double violatedAmount() {
		return -slack();
	}

	double slack() {
		return right.getPosition() - (left.getPosition() + separation);
	}

	public String toString() {
		DecimalFormat nf = new DecimalFormat("#.##");
		return left + "+" + nf.format(separation) + "<=" + right + "(" + nf.format(slack())
				+ ")";
	}

	/**
	 * @param v
	 * @return neighbour of v in this constraint
	 */
	public Variable neighbour(Variable v) {
		assert v == left || v == right;
		if (v == left)
			return right;
		return left;
	}

	/**
	 * @param c
	 *            another constraint
	 * @return true if c is between the same blocks as this
	 */
	public boolean sameContainers(Constraint c) {
		if (c.left.container == left.container
				&& c.right.container == right.container
				|| c.left.container == right.container
				&& c.right.container == left.container) {
			return true;
		}
		return false;
	}

	/**
	 * remove all traces of c
	 */
	public void delete() {
		left.outConstraints.remove(this);
		right.inConstraints.remove(this);
	}

	public boolean isTight() {
		if (Math.abs(violatedAmount()) < 0.0001) {
			return true;
		}
		return false;
	}

	/**
	 * If the constraint of the argument is more violated than this then it is
	 * considered greater (and we return 1). NOTE: constraints within the same
	 * block are treated as having infinite violated amount. This is a hack so
	 * that the pairing heaps used to manage in and out constraints are not
	 * messed up when they contain internal constraints (which do not move
	 * relative to the block)
	 * 
	 * @param c
	 *            Constraint to compare against
	 * @return 1 if c more violated than this, 0 if equal, -1 if less than
	 */
	public int compareTo(Constraint c) {
		double va = violatedAmount();
		double cva = c.violatedAmount();
		if (left.container == right.container) {
			va = Double.MAX_VALUE;
		}
		if (c.left.container == c.right.container) {
			cva = Double.MAX_VALUE;
		}
		double d = cva - va;
		if (d < 0) {
			return 1;
		} else if (d > 0) {
			return -1;
		} else {
			return c.toString().compareTo(toString());
		}
	}
}
