package placement;

import java.util.Hashtable;

import mosek.Env;
import mosek.Exit;
import mosek.Task;

class msgclass extends mosek.Stream {
	public msgclass() {
		super();
	}

	public void print(String msg) {
		System.out.print(msg);
	}
}

public class MosekPlacement implements Placement {
	Hashtable<String, Integer> varLookup = new Hashtable<String, Integer>();

	int varCounter = 0;

	Variables vars = new Variables();

	Constraints constraints = new Constraints();

	/* (non-Javadoc)
	 * @see placement.Pl#addConstraint(java.lang.String, java.lang.String, double)
	 */
	public Constraint addConstraint(String u, String v, double sep) {
		Variable v1 = vars.get(varLookup.get(u));
		Variable v2 = vars.get(varLookup.get(v));
		Constraint c = new Constraint(v1, v2, sep);
		constraints.add(c);
		return c;
	}

	public MosekPlacement(Variable[] vs) {
		new Blocks(vs);
		for (Variable v : vs) {
			vars.add(v);
			varLookup.put(v.name, varCounter++);
		}
	}

	public MosekPlacement(Variables vs, Constraints cs) {
		for (Variable v : vs) {
			vars.add(v);
			varLookup.put(v.name, varCounter++);
		}
		constraints = cs;
	}
	/* (non-Javadoc)
	 * @see placement.Pl#solve()
	 */
	public double solve() throws Exception {
		try {
			Env env = new Env();
			env.init();
			int n = vars.size(), m = constraints.size();
			Task task = new Task(env, m, n);
			//task.connectStream(new msgclass(), mosek.Env.streamkeye.log);
			task.resizetask(m, n, 0, 2 * m, n);
			task.append(0, n);
			task.append(1, m);
			int[] qsubi = new int[n], qsubj = new int[n], bkx = new int[n];
			double[] qval = new double[n];
			for (int i = 0; i < n; i++) {
				qsubi[i] = i;
				qsubj[i] = i;
				qval[i] = 2.0*vars.get(i).weight;
				task.putcj(i, -qval[i] * vars.get(i).desiredPosition);
				task.putbound(0, i, Env.boundkeye.ra, -Env.Val.infinity,
						Env.Val.infinity);
			}
			for (int i = 0; i < m; i++) {
				Constraint con = constraints.get(i);
				int l = varLookup.get(con.left.name);
				int r = varLookup.get(con.right.name);
				task.putavec(1, i, 2, new int[] { l, r },
						new double[] { -1, 1 });
				task.putbound(1, i, Env.boundkeye.lo, con.separation,
						Env.Val.infinity);
			}
			task.putqobj(n, qsubi, qsubj, qval);
			task.optimize();
			double[] xx = new double[n];
			task.getsolutionslice(mosek.Env.soltpekeye.sol_itr,
					mosek.Env.solitmkeye.xx, 0, n, xx);

			System.out.println("Primal solution");
			for (int j = 0; j < n; ++j) {
				Variable v = vars.get(j);
				v.offset = xx[j];
				v.container.position = 0;
				//System.out.println("x" + (j+1) + ": " + xx[j]);
			}

			task.cleanup();
			env.cleanup();
		} catch (mosek.ArrayLengthException e)
		/* Catch both Error and Warning */
		{
			System.out.println(e.toString());
			throw e;
		} catch (mosek.Exception e) {
			System.out.println(e.msg);
			throw e;
		}
		return getCost();
	}
	/* (non-Javadoc)
	 * @see placement.Pl#getCost()
	 */
	public double getCost() {
		double cost = 0;
		for (Variable v : vars) {
			double diff = v.getPosition() - v.desiredPosition;
			cost += v.weight * diff * diff;
		}
		return cost;
	}

	public Constraints getConstraints() {
		return constraints;
	}

	public Variables getVariables() {
		return vars;
	}
}
