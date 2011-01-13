/*
 * Created on 24/02/2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package placement;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.Queue;

/**
 * @author dwyer_2 TODO To change the template for this generated type comment
 *         go to Window - Preferences - Java - Code Style - Code Templates
 */
class Constraints extends ArrayList<Constraint> {
	public Constraints(Constraints inConstraints) {
		super(inConstraints);
	}
	public Constraints() {
		super();
	}

	Constraints getActive(Variable a, Variable b) {
		Constraints active = new Constraints();
		for (Constraint c : this) {
			if (c.left == a && c.right == b || c.left == b && c.right == a) {
				active.add(c);
			}
		}
		return active;
	}

	/**
	 * Currently O(|Constraints|).
	 * @return list of constraints between the same container blocks, at least
	 *         the first of which is violated
	 */
	Constraints removeViolated() {
		Constraints vs = violated();
		Constraint v=null;
		if(!vs.isEmpty()) {
			v=vs.get(vs.size()-1);
		}
		// find all other constraints between same pair of blocks
		vs = new Constraints();
		if (v != null) {
			for (Constraint c : this) {
				if (c.sameContainers(v)) {
					vs.add(c);
				}
			}
			removeAll(vs);
		}
		return vs;
	}

	Constraints getActive(Block a, Block b) {
		Constraints active = new Constraints();
		for (Variable va : a.variables) {
			for (Variable vb : b.variables) {
				active.addAll(getActive(a, b));
			}
		}
		return active;
	}

	Constraint max() {
		double m = 0;
		Constraint mc = null;
		for (Constraint c : this) {
			double d = c.violatedAmount();
			if (d > m) {
				m = d;
				mc = c;
			}
		}
		return mc;
	}

	Constraints violated() {
		Constraints vs = new Constraints();
		for (Constraint c : this) {
			if (c.violatedAmount()>0.00001) {
				vs.add(c);
			}
		}
		Collections.sort(vs);
		return vs;
	}

	/**
	 * @return subset of constraints which are active
	 */
	public Constraints getActive() {
		Constraints active = new Constraints();
		for (Constraint c : this) {
			if (c.active)
				active.add(c);
		}
		return active;
	}	

};