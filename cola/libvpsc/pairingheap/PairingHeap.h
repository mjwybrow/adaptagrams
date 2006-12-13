/**
 * \brief Pairing heap datastructure implementation
 *
 * Based on example code in "Data structures and Algorithm Analysis in C++"
 * by Mark Allen Weiss, used and released under the LGPL by permission
 * of the author.
 *
 * No promises about correctness.  Use at your own risk!
 *
 * Authors:
 *   Mark Allen Weiss
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
#ifndef PAIRING_HEAP_H_
#define PAIRING_HEAP_H_
#include <stdlib.h>
#include <fstream>
#include <cassert>
#include <vector>
#include <list>
class Underflow { };

// Pairing heap class
//
// CONSTRUCTION: with no parameters
//
// ******************PUBLIC OPERATIONS*********************
// PairNode & insert( x ) --> Insert x
// deleteMin( minItem )   --> Remove (and optionally return) smallest item
// T findMin( )  --> Return smallest item
// bool isEmpty( )        --> Return true if empty; else false
// bool isFull( )         --> Return true if empty; else false
// void makeEmpty( )      --> Remove all items
// void decreaseKey( PairNode p, newVal )
//                        --> Decrease value in node p
// ******************ERRORS********************************
// Throws Underflow as warranted


// Node and forward declaration because g++ does
// not understand nested classes.
template <class T> 
class PairingHeap;

template <class T>
std::ostream& operator<< (std::ostream &os,const PairingHeap<T> &b);

template <class T>
class PairNode
{
	friend std::ostream& operator<< <T>(std::ostream &os,const PairingHeap<T> &b);
	T   element;
	PairNode    *leftChild;
	PairNode    *nextSibling;
	PairNode    *prev;

	PairNode( const T & theElement ) :
	       	element( theElement ),
		leftChild(NULL), nextSibling(NULL), prev(NULL)
       	{ }
	friend class PairingHeap<T>;
};

template <class T>
class Comparator
{
public:
	virtual bool isLessThan(T const &lhs, T const &rhs) const = 0;
};

template <class T>
class PairingHeap
{
	friend std::ostream& operator<< <T>(std::ostream &os,const PairingHeap<T> &b);
public:
	PairingHeap( bool (*lessThan)(T const &lhs, T const &rhs) );
	PairingHeap( const PairingHeap & rhs );
	~PairingHeap( );

	bool isEmpty( ) const;
	bool isFull( ) const;
	unsigned size() const;

	PairNode<T> *insert( const T & x );
	const T & findMin( ) const;
	void deleteMin( );
	const T extractMin( ) {
		T v = findMin();
		deleteMin();
		return v;
	}
	void makeEmpty( );
	void decreaseKey( PairNode<T> *p, const T & newVal );
	void merge( PairingHeap<T> *rhs )
	{	
		unsigned rhsSize;
		PairNode<T> *broot=rhs->removeRootForMerge(rhsSize);
		if (root == NULL) {
			root = broot;
		} else {
			compareAndLink(root, broot);
		}
		counter+=rhsSize;
	}

	const PairingHeap & operator=( const PairingHeap & rhs );
protected:
	// Destructively gets the root for merging into another heap.
	PairNode<T> * removeRootForMerge(unsigned& size) {
		PairNode<T> *r=root;
		root=NULL;
		size=counter;
		counter=0;
		return r;
	}
	bool (*lessThan)(T const &lhs, T const &rhs);
private:
	PairNode<T> *root;
	unsigned counter;
	void reclaimMemory( PairNode<T> *t ) const;
	void compareAndLink( PairNode<T> * & first, PairNode<T> *second ) const;
	PairNode<T> * combineSiblings( PairNode<T> *firstSibling ) const;
	PairNode<T> * clone( PairNode<T> * t ) const;
};

using namespace std;   // fixme: Remove this, especially since this is a #include file.

/**
* Construct the pairing heap.
*/
template <class T>
PairingHeap<T>::PairingHeap( bool (*lessThan)(T const &lhs, T const &rhs) )
{
	root = NULL;
	counter=0;
	this->lessThan=lessThan;
}


/**
* Copy constructor
*/
template <class T>
PairingHeap<T>::PairingHeap( const PairingHeap<T> & rhs )
{
	root = NULL;
	counter=rhs->size();
	*this = rhs;
}

/**
* Destroy the leftist heap.
*/
template <class T>
PairingHeap<T>::~PairingHeap( )
{
	makeEmpty( );
}

/**
* Insert item x into the priority queue, maintaining heap order.
* Return a pointer to the node containing the new item.
*/
template <class T>
PairNode<T> *
PairingHeap<T>::insert( const T & x )
{
	PairNode<T> *newNode = new PairNode<T>( x );

	if( root == NULL )
		root = newNode;
	else
		compareAndLink( root, newNode );
	counter++;
	return newNode;
}

template <class T>
unsigned PairingHeap<T>::size() const {
	return counter;
}

/**
* Find the smallest item in the priority queue.
* Return the smallest item, or throw Underflow if empty.
*/
template <class T>
const T & PairingHeap<T>::findMin( ) const
{
	if( isEmpty( ) )
		throw Underflow( );
	return root->element;
}
/**
 * Remove the smallest item from the priority queue.
 * Throws Underflow if empty.
 */
template <class T>
void PairingHeap<T>::deleteMin( )
{
    if( isEmpty( ) )
        throw Underflow( );

    PairNode<T> *oldRoot = root;

    if( root->leftChild == NULL )
        root = NULL;
    else
        root = combineSiblings( root->leftChild );
    assert(counter);
    counter--;
    delete oldRoot;
}

/**
* Test if the priority queue is logically empty.
* Returns true if empty, false otherwise.
*/
template <class T>
bool PairingHeap<T>::isEmpty( ) const
{
	return root == NULL;
}

/**
* Test if the priority queue is logically full.
* Returns false in this implementation.
*/
template <class T>
bool PairingHeap<T>::isFull( ) const
{
	return false;
}

/**
* Make the priority queue logically empty.
*/
template <class T>
void PairingHeap<T>::makeEmpty( )
{
	reclaimMemory( root );
	root = NULL;
	counter = 0;
}

/**
* Deep copy.
*/
template <class T>
const PairingHeap<T> &
PairingHeap<T>::operator=( const PairingHeap<T> & rhs )
{
	if( this != &rhs )
	{
		makeEmpty( );
		root = clone( rhs.root );
		counter = rhs.size();
		lessThan = rhs.lessThan;
	}

	return *this;
}

/**
* Internal method to make the tree empty.
* WARNING: This is prone to running out of stack space.
*/
template <class T>
void PairingHeap<T>::reclaimMemory( PairNode<T> * t ) const
{
	if( t != NULL )
	{
		reclaimMemory( t->leftChild );
		reclaimMemory( t->nextSibling );
		delete t;
	}
}

/**
* Change the value of the item stored in the pairing heap.
* Does nothing if newVal is larger than currently stored value.
* p points to a node returned by insert.
* newVal is the new value, which must be smaller
*    than the currently stored value.
*/
template <class T>
void PairingHeap<T>::decreaseKey( PairNode<T> *p,
				  const T & newVal )
{
	if( lessThan(p->element,newVal) )
		return;    // newVal cannot be bigger
	p->element = newVal;
	if( p != root )
	{
		if( p->nextSibling != NULL )
			p->nextSibling->prev = p->prev;
		if( p->prev->leftChild == p )
			p->prev->leftChild = p->nextSibling;
		else
			p->prev->nextSibling = p->nextSibling;

		p->nextSibling = NULL;
		compareAndLink( root, p );
	}
}

/**
* Internal method that is the basic operation to maintain order.
* Links first and second together to satisfy heap order.
* first is root of tree 1, which may not be NULL.
*    first->nextSibling MUST be NULL on entry.
* second is root of tree 2, which may be NULL.
* first becomes the result of the tree merge.
*/
template <class T>
void PairingHeap<T>::
compareAndLink( PairNode<T> * & first,
		PairNode<T> *second ) const
{
	if( second == NULL )
		return;
	if( lessThan(second->element,first->element) )
	{
		// Attach first as leftmost child of second
		second->prev = first->prev;
		first->prev = second;
		first->nextSibling = second->leftChild;
		if( first->nextSibling != NULL )
			first->nextSibling->prev = first;
		second->leftChild = first;
		first = second;
	}
	else
	{
		// Attach second as leftmost child of first
		second->prev = first;
		first->nextSibling = second->nextSibling;
		if( first->nextSibling != NULL )
			first->nextSibling->prev = first;
		second->nextSibling = first->leftChild;
		if( second->nextSibling != NULL )
			second->nextSibling->prev = second;
		first->leftChild = second;
	}
}

/**
* Internal method that implements two-pass merging.
* firstSibling the root of the conglomerate;
*     assumed not NULL.
*/
template <class T>
PairNode<T> *
PairingHeap<T>::combineSiblings( PairNode<T> *firstSibling ) const
{
	if( firstSibling->nextSibling == NULL )
		return firstSibling;

	// Allocate the array
	static vector<PairNode<T> *> treeArray( 5 );

	// Store the subtrees in an array
	int numSiblings = 0;
	for( ; firstSibling != NULL; numSiblings++ )
	{
		if( numSiblings == (int)treeArray.size( ) )
			treeArray.resize( numSiblings * 2 );
		treeArray[ numSiblings ] = firstSibling;
		firstSibling->prev->nextSibling = NULL;  // break links
		firstSibling = firstSibling->nextSibling;
	}
	if( numSiblings == (int)treeArray.size( ) )
		treeArray.resize( numSiblings + 1 );
	treeArray[ numSiblings ] = NULL;

	// Combine subtrees two at a time, going left to right
	int i = 0;
	for( ; i + 1 < numSiblings; i += 2 )
		compareAndLink( treeArray[ i ], treeArray[ i + 1 ] );

	int j = i - 2;

	// j has the result of last compareAndLink.
	// If an odd number of trees, get the last one.
	if( j == numSiblings - 3 )
		compareAndLink( treeArray[ j ], treeArray[ j + 2 ] );

	// Now go right to left, merging last tree with
	// next to last. The result becomes the new last.
	for( ; j >= 2; j -= 2 )
		compareAndLink( treeArray[ j - 2 ], treeArray[ j ] );
	return treeArray[ 0 ];
}

/**
* Internal method to clone subtree.
* WARNING: This is prone to running out of stack space.
*/
template <class T>
PairNode<T> *
PairingHeap<T>::clone( PairNode<T> * t ) const
{
	if( t == NULL ) 
		return NULL;
	else
	{
		PairNode<T> *p = new PairNode<T>( t->element );
		if( ( p->leftChild = clone( t->leftChild ) ) != NULL )
			p->leftChild->prev = p;
		if( ( p->nextSibling = clone( t->nextSibling ) ) != NULL )
			p->nextSibling->prev = p;
		return p;
	}
}

template <class T>
ostream& operator <<(ostream &os, const PairingHeap<T> &b)
{
	os<<"Heap:";
	if (b.root != NULL) {
		PairNode<T> *r = b.root;
		list<PairNode<T>*> q;
		q.push_back(r);
		while (!q.empty()) {
			r = q.front();
			q.pop_front();
			if (r->leftChild != NULL) {
				os << *r->element << ">";
				PairNode<T> *c = r->leftChild;
				while (c != NULL) {
					q.push_back(c);
					os << "," << *c->element;
					c = c->nextSibling;
				}
				os << "|";
			}
		}
	}
	return os;
}

#endif /* !PAIRING_HEAP_H_ */
