/*
 * Created on 28/02/2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package placement;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

/**
 * @author dwyer
 */

public class Variables extends ArrayList<Variable> {
	Variables() {
		super();
	}

	Variables(Variable[] vs) {
		for (Variable v : vs) {
			add(v);
		}
	}

	Variables getSources() {
		Variables sources = new Variables();
		for (Variable v : this) {
			if (v.getInConstraints().size() == 0) {
				v.visited = true;
				sources.add(v);
			}
		}
		return sources;
	}

	double getMaxPos() {
		double max = 0;
		for (Variable v : this) {
			if (v.getPosition() > max)
				max = v.getPosition();
		}
		return max;
	}

	double getMinPos() {
		double min = Double.MAX_VALUE;
		for (Variable v : this) {
			if (v.getPosition() < min)
				min = v.getPosition();
		}
		return min;
	}

	boolean contains(String label) {
		for (Variable v : this) {
			if (v.name.equals(label)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Sorts the variables in this list from lowest constraintDepth to highest.
	 */
	void sortOnConstraintDepth() {
		Collections.sort(this, new Comparator<Variable>() {
			public int compare(Variable v1, Variable v2) {
				if (v1.constraintDepth > v2.constraintDepth)
					return 1;
				if (v1.constraintDepth < v2.constraintDepth)
					return -1;
				return 0;
			}

		});
	}

	public Variable max() {
		Variable max = get(0);
		double m = max.getPosition();
		for (int i = 1;i < size();i++) {
			Variable v = get(i);
			double p = v.getPosition();
			if (p > m) {
				max = v;
				m = p;
			}
		}
		return max;
	}

	public Variable min() {
		Variable min = get(0);
		double m = min.getPosition();
		for (int i = 1;i < size();i++) {
			Variable v = get(i);
			double p = v.getPosition();
			if (p < m) {
				min = v;
				m = p;
			}
		}
		return min;
	}
}
