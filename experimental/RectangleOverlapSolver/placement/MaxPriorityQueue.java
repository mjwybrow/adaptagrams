package placement;

import java.util.ArrayList;

public interface MaxPriorityQueue<T extends Comparable> {
	public void add(T e);
	public T findMax();
	public T deleteMax();
	public void merge(MaxPriorityQueue<T> other);
	public ArrayList<T> getAll();
	public int size();
}
