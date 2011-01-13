package placement;

/**
 * Implements a pairing heap. Supports a increaseKey operation. Note that all
 * "matching" is based on the compareTo method.
 * 
 * Based on implementation by Mark Allen Weiss from the book "Data Stuctures & Algorithm Analysis in Java" - Prentice Hall 1999
 */

public class MinPairingHeap<T extends Comparable> {

	/**
	 * Construct the pairing heap.
	 */
	public MinPairingHeap() {
		root = null;
	}

	/**
	 * Insert into the priority queue, and return a PNode that can be used by
	 * decreaseKey. Duplicates are allowed.
	 * 
	 * @param x
	 *            the item to insert.
	 * @return the PNode containing the newly inserted item.
	 */
	public PNode<T> insert(T x) {
		PNode<T> newPNode = new PNode<T>(x);

		if (root == null)
			root = newPNode;
		else
			root = compareAndLink(root, newPNode);
		return newPNode;
	}

	/**
	 * Find the smallest item in the priority queue.
	 * 
	 * @return the smallest item, or null if empty.
	 */
	public T findMin() {
		if (isEmpty())
			return null;
		return root.element;
	}

	/**
	 * Remove the smallest item from the priority queue.
	 * 
	 * @return the smallest item, or null if empty.
	 */
	public T deleteMin() {
		if (isEmpty())
			return null;

		T x = findMin();
		if (root.leftChild == null)
			root = null;
		else
			root = combineSiblings(root.leftChild);

		return x;
	}

	/**
	 * Change the value of the item stored in the pairing heap. Does nothing if
	 * newVal is larger than the currently stored value.
	 * 
	 * @param p
	 *            any PNode returned by addItem.
	 * @param newVal
	 *            the new value, which must be smaller than the currently stored
	 *            value.
	 */
	public void decreaseKey(PNode<T> p, T newVal) {
		if (p.element.compareTo(newVal) < 0)
			return; // newVal cannot be bigger
		p.element = newVal;
		if (p != root) {
			if (p.nextSibling != null)
				p.nextSibling.prev = p.prev;
			if (p.prev.leftChild == p)
				p.prev.leftChild = p.nextSibling;
			else
				p.prev.nextSibling = p.nextSibling;

			p.nextSibling = null;
			root = compareAndLink(root, p);
		}
	}

	/**
	 * Test if the priority queue is logically empty.
	 * 
	 * @return true if empty, false otherwise.
	 */
	public boolean isEmpty() {
		return root == null;
	}

	/**
	 * Make the priority queue logically empty.
	 */
	public void makeEmpty() {
		root = null;
	}

	private PNode<T> root;

	/**
	 * Internal method that is the basic operation to maintain order. Links
	 * first and second together to satisfy heap order.
	 * 
	 * @param first
	 *            root of tree 1, which may not be null. first.nextSibling MUST
	 *            be null on entry.
	 * @param second
	 *            root of tree 2, which may be null.
	 * @return result of the tree merge.
	 */
	private PNode compareAndLink(PNode<T> first, PNode<T> second) {
		if (second == null)
			return first;

		if (second.element.compareTo(first.element) < 0) {
			// Attach first as leftmost child of second
			second.prev = first.prev;
			first.prev = second;
			first.nextSibling = second.leftChild;
			if (first.nextSibling != null)
				first.nextSibling.prev = first;
			second.leftChild = first;
			return second;
		} else {
			// Attach second as leftmost child of first
			second.prev = first;
			first.nextSibling = second.nextSibling;
			if (first.nextSibling != null)
				first.nextSibling.prev = first;
			second.nextSibling = first.leftChild;
			if (second.nextSibling != null)
				second.nextSibling.prev = second;
			first.leftChild = second;
			return first;
		}
	}

	private PNode<T>[] doubleIfFull(PNode<T>[] array, int index) {
		if (index == array.length) {
			PNode<T>[] oldArray = array;

			array = new PNode[index * 2];
			for (int i = 0; i < index; i++)
				array[i] = oldArray[i];
		}
		return array;
	}

	/**
	 * Internal method that implements two-pass merging.
	 * 
	 * @param firstSibling
	 *            the root of the conglomerate; assumed not null.
	 */
	private PNode<T> combineSiblings(PNode<T> firstSibling) {
		PNode<T>[] treeArray = new PNode[5];
		if (firstSibling.nextSibling == null)
			return firstSibling;

		// Store the subtrees in an array
		int numSiblings = 0;
		for (; firstSibling != null; numSiblings++) {
			treeArray = doubleIfFull(treeArray, numSiblings);
			treeArray[numSiblings] = firstSibling;
			firstSibling.prev.nextSibling = null; // break links
			firstSibling = firstSibling.nextSibling;
		}
		treeArray = doubleIfFull(treeArray, numSiblings);
		treeArray[numSiblings] = null;

		// Combine subtrees two at a time, going left to right
		int i = 0;
		for (; i + 1 < numSiblings; i += 2)
			treeArray[i] = compareAndLink(treeArray[i], treeArray[i + 1]);

		int j = i - 2;

		// j has the result of last compareAndLink.
		// If an odd number of trees, get the last one.
		if (j == numSiblings - 3)
			treeArray[j] = compareAndLink(treeArray[j], treeArray[j + 2]);

		// Now go right to left, merging last tree with
		// next to last. The result becomes the new last.
		for (; j >= 2; j -= 2)
			treeArray[j - 2] = compareAndLink(treeArray[j - 2], treeArray[j]);

		return treeArray[0];
	}
	public static void main(String args[]) {
		MinPairingHeap<Integer> h = new MinPairingHeap();
		int numItems = 100;
		int i = 37;
		int j;
		boolean pass=true;

		System.out.println("Checking Pairing Heap...");
		for (i = 37; i != 0; i = (i + 37) % numItems) {
			h.insert(i);
		}
		for (i = 1; i < numItems; i++) {
			int v = h.deleteMin();
			if (v != i) {
				pass=false;
				System.out.println("Failed insert/deleteMin test! Found: " +v+", expected: " + i);
			}
		}

		PNode<Integer>[] p = new PNode[numItems];
		for (i = 0, j = numItems / 2; i < numItems; i++, j = (j + 71)
				% numItems)
			p[j] = h.insert(j + numItems);
		for (i = 0, j = numItems / 2; i < numItems; i++, j = (j + 53)
				% numItems)
			h.decreaseKey(p[j], p[j].element + numItems);
		i = numItems-1;
		while (!h.isEmpty()) {
			int v=h.deleteMin();
			if (v != ++i) {
				pass=false;
				System.out.println("Failed increaseKey/deleteMax test! Found: " +v+", expected: " + i);
			}
		}
		System.out.println("Check completed: "+(pass?"Pass":"Fail"));
	}
}
