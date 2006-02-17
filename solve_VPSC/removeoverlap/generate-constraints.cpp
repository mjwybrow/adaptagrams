/**
 * \brief Remove overlaps function
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include <set>
#include <cassert>
#include "generate-constraints.h"
#include "constraint.h"

#include "isnan.h" /* Include last */

using std::set;
using std::vector;

std::ostream& operator <<(std::ostream &os, const Rectangle &r) {
	os << "{"<<r.minX<<","<<r.maxX<<","<<r.minY<<","<<r.maxY<<"},";
	return os;
}
Rectangle::Rectangle(double x, double X, double y, double Y) 
: minX(x),maxX(X),minY(y),maxY(Y) {
		assert(x<=X);
		assert(y<=Y);
}

struct Node;
struct CmpNodePos { bool operator()(const Node* u, const Node* v) const; };

typedef set<Node*,CmpNodePos> NodeSet;

struct Node {
	Variable *v;
	Rectangle *r;
	double pos;
	Node *firstAbove, *firstBelow;
	NodeSet *leftNeighbours, *rightNeighbours;
	Node(Variable *v, Rectangle *r, double p) : v(v),r(r),pos(p) {
		firstAbove=firstBelow=NULL;
		leftNeighbours=rightNeighbours=NULL;
		assert(r->width()<1e40);
	}
	~Node() {
		delete leftNeighbours;
		delete rightNeighbours;
	}
	void addLeftNeighbour(Node *u) {
		leftNeighbours->insert(u);
	}
	void addRightNeighbour(Node *u) {
		rightNeighbours->insert(u);
	}
	void setNeighbours(NodeSet *left, NodeSet *right) {
		leftNeighbours=left;
		rightNeighbours=right;
		for(NodeSet::iterator i=left->begin();i!=left->end();i++) {
			Node *v=*(i);
			v->addRightNeighbour(this);
		}
		for(NodeSet::iterator i=right->begin();i!=right->end();i++) {
			Node *v=*(i);
			v->addLeftNeighbour(this);
		}
	}
};
bool CmpNodePos::operator() (const Node* u, const Node* v) const {
	if (u->pos < v->pos) {
		return true;
	}
	if (v->pos < u->pos) {
		return false;
	}
	if (isNaN(u->pos) != isNaN(v->pos)) {
		return isNaN(u->pos);
	}
	return u < v;

	/* I don't know how important it is to handle NaN correctly
	 * (e.g. we probably handle it badly in other code anyway, and
	 * in any case the best we can hope for is to reduce the
	 * badness of other nodes).
	 *
	 * Nevertheless, we try to do the right thing here and in
	 * event comparison.  The issue is that (on platforms with
	 * ieee floating point comparison) NaN compares neither less
	 * than nor greater than any other number, yet sort wants a
	 * well-defined ordering.  In particular, we want to ensure
	 * transitivity of equivalence, which normally wouldn't be
	 * guaranteed if the "middle" item in the transitivity
	 * involves a NaN.  (NaN is neither less than nor greater than
	 * other numbers, so tends to be considered as equal to all
	 * other numbers: even unequal numbers.)
	 */
}

NodeSet* getLeftNeighbours(NodeSet &scanline,Node *v) {
	NodeSet *leftv = new NodeSet;
	NodeSet::iterator i=scanline.find(v);
	while(i--!=scanline.begin()) {
		Node *u=*(i);
		if(u->r->overlapX(v->r)<=0) {
			leftv->insert(u);
			return leftv;
		}
		if(u->r->overlapX(v->r)<=u->r->overlapY(v->r)) {
			leftv->insert(u);
		}
	}
	return leftv;
}
NodeSet* getRightNeighbours(NodeSet &scanline,Node *v) {
	NodeSet *rightv = new NodeSet;
	NodeSet::iterator i=scanline.find(v);
	for(i++;i!=scanline.end(); i++) {
		Node *u=*(i);
		if(u->r->overlapX(v->r)<=0) {
			rightv->insert(u);
			return rightv;
		}
		if(u->r->overlapX(v->r)<=u->r->overlapY(v->r)) {
			rightv->insert(u);
		}
	}
	return rightv;
}

typedef enum {Open, Close} EventType;
struct Event {
	EventType type;
	Node *v;
	double pos;
	Event(EventType t, Node *v, double p) : type(t),v(v),pos(p) {};
};
Event **events;
int compare_events(const void *a, const void *b) {
	Event *ea=*(Event**)a;
	Event *eb=*(Event**)b;
	if(ea->v->r==eb->v->r) {
		// when comparing opening and closing from the same rect
		// open must come first
		if(ea->type==Open) return -1;
		return 1;
	} else if(ea->pos > eb->pos) {
		return 1;
	} else if(ea->pos < eb->pos) {
		return -1;
	} else if(isNaN(ea->pos) != isNaN(ea->pos)) {
		/* See comment in CmpNodePos. */
		return ( isNaN(ea->pos)
			 ? -1
			 : 1 );
	}
	return 0;
}

/**
 * Prepares variables and constraints in order to apply VPSC horizontally.
 * useNeighbourLists determines whether or not a heuristic is used to deciding whether to resolve
 * all overlap in the x pass, or leave some overlaps for the y pass.
 */
int generateXConstraints(Rectangle *rs[], double weights[], const int n, Variable **&vars, Constraint **&cs, bool useNeighbourLists) {
	events=new Event*[2*n];
	int i,m,ctr=0;
	vector<Constraint*> constraints;
	vars=new Variable*[n];
	for(i=0;i<n;i++) {
		vars[i]=new Variable(i,rs[i]->getCentreX(),weights[i]);
		Node *v = new Node(vars[i],rs[i],rs[i]->getCentreX());
		events[ctr++]=new Event(Open,v,rs[i]->getMinY());
		events[ctr++]=new Event(Close,v,rs[i]->getMaxY());
	}
	qsort((Event*)events, (size_t)2*n, sizeof(Event*), compare_events );

	NodeSet scanline;
	for(i=0;i<2*n;i++) {
		Event *e=events[i];
		Node *v=e->v;
		if(e->type==Open) {
			scanline.insert(v);
			if(useNeighbourLists) {
				v->setNeighbours(
					getLeftNeighbours(scanline,v),
					getRightNeighbours(scanline,v)
				);
			} else {
				NodeSet::iterator it=scanline.find(v);
				if(it--!=scanline.begin()) {
					Node *u=*it;
					v->firstAbove=u;
					u->firstBelow=v;
				}
				it=scanline.find(v);
				if(++it!=scanline.end()) {
					Node *u=*it;
					v->firstBelow=u;
					u->firstAbove=v;
				}
			}
		} else {
			// Close event
			int r;
			if(useNeighbourLists) {
				for(NodeSet::iterator i=v->leftNeighbours->begin();
					i!=v->leftNeighbours->end();i++
				) {
					Node *u=*i;
					double sep = (v->r->width()+u->r->width())/2.0;
					constraints.push_back(new Constraint(u->v,v->v,sep));
					r=u->rightNeighbours->erase(v);
				}
				
				for(NodeSet::iterator i=v->rightNeighbours->begin();
					i!=v->rightNeighbours->end();i++
				) {
					Node *u=*i;
					double sep = (v->r->width()+u->r->width())/2.0;
					constraints.push_back(new Constraint(v->v,u->v,sep));
					r=u->leftNeighbours->erase(v);
				}
			} else {
				Node *l=v->firstAbove, *r=v->firstBelow;
				if(l!=NULL) {
					double sep = (v->r->width()+l->r->width())/2.0;
					constraints.push_back(new Constraint(l->v,v->v,sep));
					l->firstBelow=v->firstBelow;
				}
				if(r!=NULL) {
					double sep = (v->r->width()+r->r->width())/2.0;
					constraints.push_back(new Constraint(v->v,r->v,sep));
					r->firstAbove=v->firstAbove;
				}
			}
			r=scanline.erase(v);
			delete v;
		}
		delete e;
	}
	delete [] events;
	cs=new Constraint*[m=constraints.size()];
	for(i=0;i<m;i++) cs[i]=constraints[i];
	return m;
}

/**
 * Prepares variables and constraints in order to apply VPSC vertically to remove ALL overlap.
 */
int generateYConstraints(Rectangle *rs[], double weights[], const int n, Variable **&vars, Constraint **&cs) {
	events=new Event*[2*n];
	int ctr=0,i,m;
	vector<Constraint*> constraints;
	vars=new Variable*[n];
	for(i=0;i<n;i++) {
		vars[i]=new Variable(i,rs[i]->getCentreY(),weights[i]);
		Node *v = new Node(vars[i],rs[i],rs[i]->getCentreY());
		events[ctr++]=new Event(Open,v,rs[i]->getMinX());
		events[ctr++]=new Event(Close,v,rs[i]->getMaxX());
	}
	qsort((Event*)events, (size_t)2*n, sizeof(Event*), compare_events );
	NodeSet scanline;
	for(i=0;i<2*n;i++) {
		Event *e=events[i];
		Node *v=e->v;
		if(e->type==Open) {
			scanline.insert(v);
			NodeSet::iterator i=scanline.find(v);
			if(i--!=scanline.begin()) {
				Node *u=*i;
				v->firstAbove=u;
				u->firstBelow=v;
			}
			i=scanline.find(v);
			if(++i!=scanline.end())	 {
				Node *u=*i;
				v->firstBelow=u;
				u->firstAbove=v;
			}
		} else {
			// Close event
			Node *l=v->firstAbove, *r=v->firstBelow;
			if(l!=NULL) {
				double sep = (v->r->height()+l->r->height())/2.0;
				constraints.push_back(new Constraint(l->v,v->v,sep));
				l->firstBelow=v->firstBelow;
			}
			if(r!=NULL) {
				double sep = (v->r->height()+r->r->height())/2.0;
				constraints.push_back(new Constraint(v->v,r->v,sep));
				r->firstAbove=v->firstAbove;
			}
			scanline.erase(v);
			delete v;
		}
		delete e;
	}
	delete [] events;
	cs=new Constraint*[m=constraints.size()];
	for(i=0;i<m;i++) cs[i]=constraints[i];
	return m;
}
