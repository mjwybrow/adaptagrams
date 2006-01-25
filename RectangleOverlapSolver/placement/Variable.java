/*
 * Created on 21/02/2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package placement;

import java.awt.Color;
import java.text.DecimalFormat;
import java.util.HashMap;

/**
 * @author dwyer_2
 * 
 * TODO To change the template for this generated type comment go to Window -
 * Preferences - Java - Code Style - Code Templates
 */
public class Variable {
	String name;
	
	double desiredPosition;

	double weight;

	double constraintDepth;

	double offset;

	Block container;

	Constraints inConstraints = new Constraints();

	Constraints outConstraints = new Constraints();

	Variable(String name, double desiredPosition, double weight) {
		this.desiredPosition = desiredPosition;
		this.weight = weight;
		this.name = name;
	}

	HashMap<Class, Object> data = new HashMap<Class, Object>();

	double getPosition() {
		if (container != null) {
			return container.position + offset;
		} else
			return offset;
	}

	public String toString() {
		DecimalFormat nf = new DecimalFormat("#.##");
		return "(" + name + "=" + nf.format(getPosition()) + ")";
	}

	public void addInConstraint(Constraint c) {
		inConstraints.add(c);
	}

	public void addOutConstraint(Constraint c) {
		outConstraints.add(c);
		// if(container!=null) container.addOutConstraint(c);
	}

	public Constraints getInConstraints() {
		return inConstraints;
	}

	public Constraints getOutConstraints() {
		return outConstraints;
	}

	boolean visited = false;

	public Color colour;

	public int inConstraintCounter;
}
