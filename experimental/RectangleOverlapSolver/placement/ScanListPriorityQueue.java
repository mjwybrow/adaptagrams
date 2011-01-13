package placement;

import java.util.ArrayList;
import java.util.Collections;

public class ScanListPriorityQueue<T extends Comparable> implements
		MaxPriorityQueue<T> {
	ArrayList<T> q = new ArrayList<T>();

	MaxPairingHeap<T> ph = new MaxPairingHeap<T>();

	public void add(T e) {
		q.add(e);
		ph.add(e);
	}

	public T findMax() {
		T mc = null;
		if (q.size() > 0) {
			Collections.sort((ArrayList<Constraint>) q);
			mc = q.get(q.size() - 1);
			if (ph.findMax() != mc) {
				Constraint c = (Constraint) ph.findMax();
				if (c.left.container != c.right.container) {
					System.out.println("Vrap");
				}
			}
			// assert (ph.findMax() == mc);
		}
		return mc;
	}

	public T deleteMax() {
		T m = findMax();
		q.remove(q.size() - 1);
		ph.deleteMax();
		return m;
	}

	public void merge(MaxPriorityQueue<T> other) {
		q.addAll(((ScanListPriorityQueue) other).q);
		ph.merge(((ScanListPriorityQueue) other).ph);
		assert (q.size() == ph.size());
	}

	public String toString() {
		String s = "";
		for (T e : q) {
			s = s + e.toString() + ";";
		}
		return s;
	}

	public ArrayList<T> getAll() {
		return q;
	}

	public int size() {
		return q.size();
	}
}