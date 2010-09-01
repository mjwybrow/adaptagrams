/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2005-2008  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file LICENSE; if not, 
 * write to the Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307  USA
 *
*/

#include <cassert>
#include <iostream>

#include "libvpsc/variable.h"
#include "libvpsc/constraint.h"
#include "libvpsc/blocks.h"
#include "libvpsc/block.h"
using namespace std;
using namespace vpsc;


void test1() {
    Blocks *blocks = new Blocks(Variables());
    cout << "Block test 1..." << endl;
    Variable *a1=new Variable(1,0,1);
    Variable *a2=new Variable(2,0,1);
    Constraint *c=new Constraint(a1,a2,1);
    a1->out.push_back(c);
    a2->in.push_back(c);
    Block *b1=new Block(blocks, a1);
    Block *b2=new Block(blocks, a2);
    b1->merge(b2,c);
    cout << "Block: " << *b1 << endl;
    a1->desiredPosition = -1;
    a2->desiredPosition = 2;
    Constraint *m = b1->findMinLMBetween(a1,a2);
    cout << "Min lm constraint: " << *m << endl;
    assert(c==m);
    cout << "  lm=" << c->lm << endl;
    cout << "Block test 1... Success!" << endl;
}

/*
 * Constraint tree:
 *    \_/
 *    / \
 */
void test2() {
    Blocks *blocks = new Blocks(Variables());
    cout << "Block test 2..." << endl;
    Variable *a[]={
        new Variable(0,0,1),
        new Variable(1,0,1),
        new Variable(2,1,1),
        new Variable(3,2,1),
        new Variable(4,3,1),
        new Variable(5,3,1)};
    Constraint *c[]={
        new Constraint(a[0],a[2],2),
        new Constraint(a[1],a[2],2),
        new Constraint(a[2],a[3],2),
        new Constraint(a[3],a[4],2),
        new Constraint(a[3],a[5],2)};
    for(int i=0;i<6;i++) {
        new Block(blocks,a[i]);
    }
    for(int i=0;i<5;i++) {
        c[i]->left->out.push_back(c[i]);
        c[i]->right->in.push_back(c[i]);
    }
    for(int i=0;i<5;i++) {
        Block *l=c[i]->left->block, *r=c[i]->right->block;
        l->merge(r,c[i]);
    }
    Block *b=a[0]->block;
    cout << "Block: " << *b << endl;
    for(int i=0;i<6;i++) {
            a[i]->desiredPosition = i!=4?-2:5;
    }
    cout << "calc min lm:" << endl;
    Constraint *m = b->findMinLMBetween(a[0],a[4]);
    cout << "Min lm constraint: " << *m << endl;
    assert(m==c[3]);
    cout << "Block test 2... Success!" << endl;
}
int main() {
    test1();
    test2();
    return 0;
}
