package placement;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.PriorityQueue;

public class SlowPriorityQueue<T extends Comparable> implements MaxPriorityQueue<T> {
	PriorityQueue<T> q = new PriorityQueue<T>(10, new Comparator<T>() {

		public int compare(T a, T b) {
			return -a.compareTo(b);
		}
		
	});
	
	public void add(T e) {
		q.add(e);
	}

	public T findMax() {
		return q.peek();
	}

	public T deleteMax() {
		return q.poll();
	}

	public void merge(MaxPriorityQueue<T> other) {
		q.addAll(((SlowPriorityQueue<T>)other).q);
	}
	
	public String toString() {
		String s = "";
		for(T e:q) {
			s=s+e.toString()+";";
		}
		return s;
	}
	
	public ArrayList<T> getAll() {
		ArrayList<T> l = new ArrayList<T>();
		for(T e:q) {
			l.add(e);
		}
		return l;
	}

	public int size() {
		return q.size();
	}
}
