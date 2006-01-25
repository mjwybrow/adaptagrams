/*
 * Created on 6/03/2005
 *
 */
package placement;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Random;

import junit.framework.TestCase;

/**
 * @author dwyer_2
 */
public class PlacementTest extends TestCase {
	HashMap<String, Variable> vlookup = new HashMap<String, Variable>();

	Variable vs[];

	/*
	 * public String toString() { String s = getClass().getName() + "-Positions: ";
	 * for (Variable v : vs) { s = s + v + "=" + v.getPosition() + " "; } s = s +
	 * "\n" + blocks; return s; }
	 */

	/**
	 * Constructor for PlacementTest.
	 * 
	 * @param arg0
	 */
	public PlacementTest(String arg0) {
		super(arg0);
	}

	private void run(Placement p, double expected) throws Exception {
		double acost = p.solve();
		System.out.println("COST=" + acost);
		// assertTrue(acceptableCost(acost, expected));
		// p = new MosekPlacement(p.getVariables(), p.getConstraints());
		// assertTrue(acceptableCost(p.solve(), acost));
		// p = new SolveVPSC(p.getVariables(), p.getConstraints());
		assertTrue(acceptableCost(p.solve(), acost));
	}

	boolean acceptableCost(double cost, double expected) {
		if (Math.abs(cost - expected) < 0.001) {
			return true;
		}
		return false;
	}

	public void testPairingHeap() throws Exception {
		{
			long startTime = System.currentTimeMillis();
			Random r = new Random();
			MaxPairingHeap<Double> h1 = new MaxPairingHeap<Double>();
			MaxPairingHeap<Double> h2 = new MaxPairingHeap<Double>();
			for (int i = 0; i < 10000; i++) {
				h1.add(r.nextDouble());
				h2.add(r.nextDouble());
			}
			long time = System.currentTimeMillis() - startTime;
			System.out.println("Inserts completed in " + time + "ms.");
			h1.merge(h2);
			double last = Double.MAX_VALUE;
			for (int i = 0; i < 20000; i++) {
				double d = h1.deleteMax();
				assertTrue(d < last);
				last = d;
			}
			time = System.currentTimeMillis() - startTime;
			System.out.println("1st Pairing heap test completed in " + time
					+ "ms.");
		}
		{
			MinPairingHeap<Integer> h = new MinPairingHeap<Integer>();
			int numItems = 1000;
			int i = 37;
			int j;

			for (i = 37; i != 0; i = (i + 37) % numItems) {
				h.insert(i);
			}
			for (i = 1; i < numItems; i++) {
				int v = h.deleteMin();
				assertTrue(v == i);
			}

			PNode<Integer>[] p = new PNode[numItems];
			for (i = 0, j = numItems / 2; i < numItems; i++, j = (j + 71)
					% numItems)
				p[j] = h.insert(j + numItems);
			for (i = 0, j = numItems / 2; i < numItems; i++, j = (j + 53)
					% numItems)
				h.decreaseKey(p[j], p[j].element + numItems);
			i = numItems - 1;
			while (!h.isEmpty()) {
				int v = h.deleteMin();
				assertTrue(v == ++i);
			}
		}
		{
			MaxPairingHeap<Integer> h = new MaxPairingHeap<Integer>();
			int numItems = 1000;
			int i = 37;
			int j;

			for (i = 37; i != 0; i = (i + 37) % numItems) {
				h.add(i);
			}
			for (i = numItems - 1; i > 0; i--) {
				int v = h.deleteMax();
				assertTrue(v == i);
			}

			PNode<Integer>[] p = new PNode[numItems];
			for (i = 0, j = numItems / 2; i < numItems; i++, j = (j + 71)
					% numItems) {
				p[j] = new PNode<Integer>(j + numItems);
				h.add(p[j]);
			}
			for (i = 0, j = numItems / 2; i < numItems; i++, j = (j + 53)
					% numItems)
				h.increaseKey(p[j], p[j].element + numItems);
			i = 3 * numItems;
			while (!h.isEmpty()) {
				int v = h.deleteMax();
				if (v != --i) {
					assertTrue(v == i);
				}
			}
		}
	}

	class HeapNode implements Comparable<HeapNode> {
		int key;

		boolean dirty = false;

		HeapNode(int k) {
			key = k;
		}
		public String toString() {
			return ""+key;
		}
		public int compareTo(HeapNode o) {
			// Dirty entries have to bubble up and be dealt with
			if(dirty&&!o.dirty) {
				return 1;
			} else if (!dirty&&o.dirty) {
				return -1;
			} else
			if (key > o.key) {
				return 1;
			} else if (key < o.key) {
				return -1;
			}
			return 0;
		}
	}

	public void testPairingHeap2() throws Exception {
		int n = 50, d = 0, od = 0, nd=0;
		Random r = new Random(System.currentTimeMillis());
		HeapNode[] ns = new HeapNode[n];
		try {
			for (int repeat = 100000; repeat > 0; repeat--) {
				MaxPairingHeap<HeapNode> h = new MaxPairingHeap<HeapNode>();
				for (int i = 0; i < n; i++) {
					ns[i] = new HeapNode(r.nextInt(50));
					h.add(ns[i]);
				}
				for (int j = 0; j < 5; j++) {
					d = r.nextInt(n);
					od = ns[d].key;
					int sub=r.nextInt(50);
					ns[d].key -= sub;
					nd = ns[d].key;
					ns[d].dirty = true;
				}
				int last = Integer.MAX_VALUE;
				while (!h.isEmpty()) {
					HeapNode i = h.findMax();
					ArrayList<HeapNode> outOfDate = new ArrayList<HeapNode>();
					while (i!=null&&i.dirty) {
						h.deleteMax();
						i.dirty = false;
						outOfDate.add(i);
						i = h.findMax();
					}
					for (HeapNode v : outOfDate) {
						h.add(v);
					}
					i = h.findMax();
					assertTrue(i.key <= last);
					last = i.key;
					h.deleteMax();
				}
			}
		} catch (junit.framework.AssertionFailedError e) {
			ns[d].key=od;
			System.err.print("int[] orig = {" + ns[0].key);
			for (int i = 1; i < n; i++) {
				System.err.print("," + ns[i].key);
			}
			System.err.println("};");
			System.err.println("int d=" + d + ", nd=" + nd+";");
			throw e;
		}
	}

	public void testPairingHeap3() throws Exception {
		MaxPairingHeap<HeapNode> h = new MaxPairingHeap<HeapNode>();
		int[] orig = {19,8,2,24,12,7,19,37,32,48};
		int d=9, nd=42;
		int n = orig.length;
		HeapNode[] ns = new HeapNode[n];
		for (int i = 0; i < n; i++) {
			ns[i] = new HeapNode(orig[i]);
			h.add(ns[i]);
		}
		System.out.println("Original heap: "+h);
		ns[d].key = nd;
		ns[d].dirty = true;
		int last = 100;
		while (!h.isEmpty()) {
			System.out.println(""+h);
			HeapNode i = h.findMax();
			ArrayList<HeapNode> outOfDate = new ArrayList<HeapNode>();
			while (i!=null&&i.dirty) {
				System.out.println("Skipping: "+i.key);
				h.deleteMax();
				i.dirty = false;
				outOfDate.add(i);
				i = h.findMax();
			}
			for (HeapNode v : outOfDate) {
				h.add(v);
			}
			i = h.findMax();
			System.out.println(i.key+"<="+last);
			assertTrue(i.key <= last);
			last = i.key;
			h.deleteMax();
		}
	}

	public void testPlacement1() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x1", 1, 3), new Variable("x2", 1, 2),
				new Variable("x3", 1, 3), new Variable("x4", 3, 2),
				new Variable("x5", 3, 1) });
		p.addConstraint("x1", "x2", 1);
		p.addConstraint("x1", "x3", 1);
		p.addConstraint("x2", "x4", 1);
		p.addConstraint("x3", "x4", 1);
		p.addConstraint("x4", "x5", 1);
		run(p, 2.541);
	}

	public void testPlacement1a() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x3", 1, 3), new Variable("x1", 1, 3),
				new Variable("x4", 3, 2), new Variable("x5", 3, 1),
				new Variable("x2", 1, 2) });
		p.addConstraint("x1", "x2", 1);
		p.addConstraint("x1", "x3", 1);
		p.addConstraint("x2", "x4", 1);
		p.addConstraint("x3", "x4", 1);
		p.addConstraint("x4", "x5", 1);
		run(p, 2.541);
	}

	public void testPlacement2() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x1", 0, 1), new Variable("x2", 1, 1),
				new Variable("x3", 4, 1), new Variable("x4", 4, 1),
				new Variable("x5", 3, 1), new Variable("x6", 6, 1),
				new Variable("x7", 5, 1), new Variable("x8", 0, 1),
				new Variable("x9", 7, 1) });
		p.addConstraint("x1", "x2", 1);
		p.addConstraint("x1", "x3", 1);
		p.addConstraint("x1", "x4", 1);
		p.addConstraint("x2", "x5", 3);
		p.addConstraint("x3", "x5", 1);
		p.addConstraint("x3", "x6", 1);
		p.addConstraint("x4", "x5", 1);
		p.addConstraint("x4", "x8", 1);
		p.addConstraint("x5", "x7", 1);
		p.addConstraint("x6", "x9", 4);
		p.addConstraint("x7", "x9", 2);
		p.addConstraint("x8", "x9", 3);
		run(p, 19.0);
	}

	public void testPlacement3() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x1", 0, 1), new Variable("x2", 0, 1),
				new Variable("x3", 0, 1), new Variable("x4", 0, 1),
				new Variable("x5", 8, 1), new Variable("x6", 8, 1),
				new Variable("x7", 8, 1), new Variable("x8", 8, 1),
				new Variable("x9", 4, 20) });
		p.addConstraint("x1", "x2", 1);
		p.addConstraint("x2", "x3", 1);
		p.addConstraint("x3", "x4", 1);
		p.addConstraint("x5", "x6", 1);
		p.addConstraint("x6", "x7", 1);
		p.addConstraint("x7", "x8", 1);
		p.addConstraint("x9", "x3", 1);
		run(p, 61.0);
	}

	public void testPlacement4() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x1", 1, 3), new Variable("x4", 3, 2),
				new Variable("x5", 3, 1), new Variable("x2", 1, 2),
				new Variable("x3", 1, 2) });
		p.addConstraint("x1", "x2", 1);
		p.addConstraint("x1", "x3", 1);
		p.addConstraint("x2", "x4", 1);
		p.addConstraint("x3", "x4", 1);
		p.addConstraint("x4", "x5", 1);
		run(p, 2.3809524);
	}

	public void testPlacement5() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x1", 1, 1), new Variable("x2", 2, 1),
				new Variable("x3", 2, 1), new Variable("x4", 3, 1),
				new Variable("x5", 5, 1), new Variable("x6", 6, 1),
				new Variable("x7", 7, 1) });
		p.addConstraint("x1", "x2", 2);
		p.addConstraint("x1", "x3", 2);
		p.addConstraint("x4", "x5", 2);
		p.addConstraint("x2", "x6", 2);
		p.addConstraint("x5", "x7", 2);
		p.addConstraint("x3", "x4", 1);
		p.addConstraint("x6", "x7", 2);
		run(p, 1.3);
	}

	public void testPlacement6() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x1", 0, 1), new Variable("x2", 2, 1),
				new Variable("x3", 1, 8), new Variable("x4", 3, 1) });
		p.addConstraint("x1", "x2", 3);
		p.addConstraint("x1", "x3", 3);
		p.addConstraint("x2", "x4", 1.5f);
		run(p, 3.6805556);
	}

	public void testPlacement7() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x1", 1, 7), new Variable("x2", -1, 2),
				new Variable("x3", 1, 2), new Variable("x4", -1, 2),
				new Variable("x5", 1, 2), new Variable("x6", -1, 2),
				new Variable("x7", 1, 2), new Variable("x8", -1, 2),
				new Variable("x9", 1, 2), new Variable("x10", -1, 7) });
		p.addConstraint("x1", "x2", 0);
		p.addConstraint("x1", "x3", 0);
		p.addConstraint("x1", "x4", 0);
		p.addConstraint("x1", "x5", 0);
		p.addConstraint("x1", "x6", 0);
		p.addConstraint("x1", "x7", 0);
		p.addConstraint("x1", "x8", 0);
		p.addConstraint("x1", "x9", 0);
		p.addConstraint("x2", "x10", 0);
		p.addConstraint("x3", "x10", 0);
		p.addConstraint("x4", "x10", 0);
		p.addConstraint("x5", "x10", 0);
		p.addConstraint("x6", "x10", 0);
		p.addConstraint("x7", "x10", 0);
		p.addConstraint("x8", "x10", 0);
		p.addConstraint("x9", "x10", 0);
		run(p, 29.866667);
	}

	public void testPlacement8() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x1", 0, 2), new Variable("x2", 0, 1),
				new Variable("x3", 3, 2), new Variable("x4", 2, 5),
				new Variable("x5", 5, 5), new Variable("x6", 4, 2),
				new Variable("x7", 7, 1), new Variable("x8", 7, 2),
				new Variable("x9", 1, 4) });
		p.addConstraint("x1", "x2", 1);
		p.addConstraint("x2", "x3", 1);
		p.addConstraint("x3", "x4", 1);
		p.addConstraint("x4", "x5", 1);
		p.addConstraint("x5", "x6", 1);
		p.addConstraint("x6", "x7", 1);
		p.addConstraint("x7", "x8", 1);
		p.addConstraint("x1", "x9", 1);
		p.addConstraint("x9", "x8", 6);
		run(p, 12.799999);
	}

	public void testPlacement9() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x0", 0, 1), new Variable("x1", 1, 1),
				new Variable("x2", 2, 1), new Variable("x3", 3, 1),
				new Variable("x4", 4, 1) });
		p.addConstraint("x0", "x4", 9);
		p.addConstraint("x0", "x2", 9);
		p.addConstraint("x2", "x4", 3);
		p.addConstraint("x0", "x1", 3);
		p.addConstraint("x1", "x2", 3);
		p.addConstraint("x2", "x3", 3);
		p.addConstraint("x3", "x4", 3);
		run(p, 666.46497);
	}

	public void testPlacement10() throws Exception {
		Placement p = new ActiveSetPlacement(new Variable[] {
				new Variable("x0", 0, 9), new Variable("x2", 0.2f, 3),
				new Variable("x4", 0.4f, 3) });
		p.addConstraint("x0", "x4", 9);
		p.addConstraint("x0", "x2", 9);
		p.addConstraint("x2", "x4", 3);
		run(p, 386.304);
	}
}
