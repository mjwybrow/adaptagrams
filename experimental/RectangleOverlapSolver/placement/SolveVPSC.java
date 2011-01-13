package placement;

import java.util.ArrayList;
import java.util.Hashtable;

public class SolveVPSC implements Placement {
	public native double solve(String vName[], double vWeight[],
			double vDesPos[], int cLeft[], int cRight[], double cGap[],
			double vResult[], int mode);
	public native int generateXConstraints(
			double rectMinX[],double rectMaxX[],
			double rectMinY[],double rectMaxY[],
			double weights[]);
	public native int generateYConstraints(
			double rectMinX[],double rectMaxX[],
			double rectMinY[],double rectMaxY[],
			double weights[]);
	public native void getConstraints(int cLeft[], int cRight[], double cGap[]);
	public native void removeOverlaps(
			double rectMinX[],double rectMaxX[],
			double rectMinY[],double rectMaxY[]);

	static {
		System.loadLibrary("placement_SolveVPSC");
	}

	Hashtable<Variable, Integer> vMap = new Hashtable<Variable, Integer>();

	Variables vs;

	Constraints cs;
	ArrayList<RectangleView> rectangles;
	Hashtable<String, Integer> varLookup = new Hashtable<String, Integer>();

	int varCounter = 0;
	
	boolean split = false;

	/* (non-Javadoc)
	 * @see placement.Pl#addConstraint(java.lang.String, java.lang.String, double)
	 */
	public Constraint addConstraint(String u, String v, double sep) {
		Variable v1 = vs.get(varLookup.get(u));
		Variable v2 = vs.get(varLookup.get(v));
		Constraint c = new Constraint(v1, v2, sep);
		cs.add(c);
		return c;
	}

	public SolveVPSC(ArrayList<RectangleView> rectangles) {
		this.rectangles=rectangles;
	}
	public SolveVPSC(Variable[] vs) {
		new Blocks(vs);
		this.vs = new Variables();
		cs = new Constraints();
		int varCounter=0;
		for (Variable v : vs) {
			this.vs.add(v);
			vMap.put(v, varCounter);
			varLookup.put(v.name, varCounter++);
		}
	}
	public SolveVPSC(Variables vars, Constraints constraints) {
		this.vs = vars;
		this.cs = constraints;
		int varCounter = 0;
		for (Variable v : vs) {
			vMap.put(v, varCounter++);
		}
	}

	public double solve() throws Exception {
		String[] vName = new String[vs.size()];
		double[] vWeight = new double[vs.size()];
		double[] vDesPos = new double[vs.size()];
		double[] vResult = new double[vs.size()];
		int i = 0;
		for (Variable v : vs) {
			vMap.put(v, i);
			vName[i] = v.name;
			vWeight[i] = v.weight;
			vDesPos[i++] = v.desiredPosition;
		}
		int[] cLeft = new int[cs.size()];
		int[] cRight = new int[cs.size()];
		double[] cGap = new double[cs.size()];
		i = 0;
		for (Constraint c : cs) {
			cLeft[i] = vMap.get(c.left);
			cRight[i] = vMap.get(c.right);
			cGap[i++] = c.separation;
		}
		double cost = solve(vName, vWeight, vDesPos, cLeft, cRight,
				cGap, vResult, split?1:0);
		for (i = 0; i < vs.size(); i++) {
			Variable v = vs.get(i);/*
			System.out.println("var=" + v.name + " v1=" + v.getPosition()
					+ " v2=" + vResult[i]);*/
			v.offset = vResult[i];
			if(v.container!=null) v.container.position = 0;
		}
		return cost;
	}
	
	public void generateXConstraints(double weights[]) {
		int n= rectangles.size();
		double[] rectMinX = new double[n], rectMaxX = new double[n], rectMinY = new double[n], rectMaxY = new double[n];
		vs=new Variables();
		for(int i=0;i<n;i++) {
			rectMinX[i]=rectangles.get(i).x;
			rectMaxX[i]=rectangles.get(i).getMaxX();
			rectMinY[i]=rectangles.get(i).y;
			rectMaxY[i]=rectangles.get(i).getMaxY();
			vs.add(new Variable("r"+i,rectMinX[i],1));
		}
		int m = generateXConstraints(
				rectMinX, rectMaxX,
				 rectMinY, rectMaxY,
				 weights);
		int[] cLeft=new int[m], cRight=new int[m];
		double[] cGap=new double[m];
		getConstraints(cLeft, cRight, cGap);
		cs = new Constraints();
		for(int i=0;i<m;i++) {
			cs.add(new Constraint(vs.get(cLeft[i]),vs.get(cRight[i]),cGap[i]));
		}
	}
	public void generateYConstraints(double weights[]) {
		int n= rectangles.size();
		double[] rectMinX = new double[n], rectMaxX = new double[n], rectMinY = new double[n], rectMaxY = new double[n];
		vs=new Variables();
		for(int i=0;i<n;i++) {
			rectMinX[i]=rectangles.get(i).x;
			rectMaxX[i]=rectangles.get(i).getMaxX();
			rectMinY[i]=rectangles.get(i).y;
			rectMaxY[i]=rectangles.get(i).getMaxY();
			vs.add(new Variable("r"+i,rectMinY[i],1));
		}
		int m = generateYConstraints(
				rectMinX, rectMaxX,
				 rectMinY, rectMaxY,
				 weights);
		int[] cLeft=new int[m], cRight=new int[m];
		double[] cGap=new double[m];
		getConstraints(cLeft, cRight, cGap);
		cs = new Constraints();
		for(int i=0;i<m;i++) {
			cs.add(new Constraint(vs.get(cLeft[i]),vs.get(cRight[i]),cGap[i]));
		}
	}

	public Constraints getConstraints() {
		return cs;
	}

	public Variables getVariables() {
		return vs;
	}
	public void removeOverlaps() {
		int n= rectangles.size();
		double[] rectMinX = new double[n], rectMaxX = new double[n], rectMinY = new double[n], rectMaxY = new double[n];
		for(int i=0;i<n;i++) {
			rectMinX[i]=rectangles.get(i).x;
			rectMaxX[i]=rectangles.get(i).getMaxX();
			rectMinY[i]=rectangles.get(i).y;
			rectMaxY[i]=rectangles.get(i).getMaxY();
		}
		removeOverlaps(rectMinX,rectMaxX,rectMinY,rectMaxY);
		for(int i=0;i<n;i++) {
			RectangleView r=rectangles.get(i);
			r.moveTo(rectMinX[i],rectMinY[i]);
		}
	}

}
