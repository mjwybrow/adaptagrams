package placement;

public class PNode<S extends Comparable> {
	/**
	 * Construct the PNode.
	 * 
	 * @param theElement
	 *            the value stored in the PNode.
	 */
	PNode(S theElement) {
		element = theElement;
		leftChild = null;
		nextSibling = null;
		prev = null;
	}

	// Friendly data; accessible by other package routines
	S element;

	PNode<S> leftChild;

	PNode<S> nextSibling;

	PNode<S> prev;
}