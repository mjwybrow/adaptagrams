/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2008  Monash University
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

#include <iostream>
#include <vector>
#include <assert.h>
#include <libcola/cola.h>
#include <libcola/cycle_detector.h>
#include "graphlayouttest.h"

using namespace std;
using namespace cola;
using namespace cycle_detector;
using vpsc::Rectangle;

int main()  {
  CycleDetector *cd;
  Edges case_a, case_b, case_c, case_d, case_e, case_f, case_g;

  CyclicEdges *cycles = nullptr;
  vector<Rectangle *> rs;
  unsigned V;

  // create case A
  // case A consists of a basic graph where the start point is a source
  cout << endl << "ENTERING CASE A" << endl;
  V = 5;
  case_a.push_back(Edge(0, 1));
  case_a.push_back(Edge(1, 2));
  case_a.push_back(Edge(2, 3));
  case_a.push_back(Edge(3, 4));
  case_a.push_back(Edge(4, 1));

  // detect the cycles
  cd = new CycleDetector(V, &case_a);
  cycles = cd->detect_cycles();
  
  if (cycles != nullptr)  {
    cout << "cycles->size(): " << cycles->size() << endl; 
    for (unsigned i = 0; i < case_a.size(); i++)  {
      // print out the cycles
      if ((*cycles)[i])  cout << "Cyclic edge found: (" << case_a[i].first << ", " << case_a[i].second << ")" << endl;
    }

    cout << endl;

    // output a picture
    rs.push_back(new Rectangle(10,10+5,10,10+5));
    rs.push_back(new Rectangle(30,30+5,30,30+5));
    rs.push_back(new Rectangle(30,30+5,60,60+5));
    rs.push_back(new Rectangle(65,65+5,60,60+5));
    rs.push_back(new Rectangle(65,65+5,30,30+5));

    assert(rs.size() == V);

    output_svg(rs, case_a, "cycle_detector_case_a.svg", false, true, cycles);
    for (unsigned i = 0; i < rs.size(); i++)  { delete rs[i]; }
    rs.clear();
    delete cycles;
  }
  else  {
    cout << "No cycles found" << endl;
  }

  // create case B
  // case B is the same graph of case A but with more elements
  cout << endl << "ENTERING CASE B" << endl;
  V = 7;
  case_b.push_back(Edge(0, 1));
  case_b.push_back(Edge(1, 2));
  case_b.push_back(Edge(2, 3));
  case_b.push_back(Edge(3, 4));
  case_b.push_back(Edge(4, 1));
  case_b.push_back(Edge(5, 2));
  case_b.push_back(Edge(6, 5));

  // detect the cycles
  cd->mod_graph(V, &case_b);
  cycles = cd->detect_cycles();
  
  if (cycles != nullptr)  {
    cout << "cycles->size(): " << cycles->size() << endl; 
    for (unsigned i = 0; i < case_b.size(); i++)  {
      // print out the cycles
      if ((*cycles)[i])  cout << "Cyclic edge found: (" << case_b[i].first << ", " << case_b[i].second << ")" << endl;
    }

    cout << endl;

    // output a picture
    rs.push_back(new Rectangle(10,10+5,10,10+5));
    rs.push_back(new Rectangle(30,30+5,30,30+5));
    rs.push_back(new Rectangle(30,30+5,60,60+5));
    rs.push_back(new Rectangle(65,65+5,60,60+5));
    rs.push_back(new Rectangle(65,65+5,30,30+5));
    rs.push_back(new Rectangle(30,30+5,90,90+5));
    rs.push_back(new Rectangle(65,65+5,90,90+5));

    assert(rs.size() == V);

    output_svg(rs, case_b, "cycle_detector_case_b.svg", false, true, cycles);
    for (unsigned i = 0; i < rs.size(); i++)  { delete rs[i]; }
    rs.clear();
    delete cycles;
  }

  // create case C
  // case C is a more complicated graph with nested cycles
  cout << endl << "ENTERING CASE C" << endl;
  V = 14;
  case_c.push_back(Edge(0, 1));
  case_c.push_back(Edge(0, 5));
  case_c.push_back(Edge(0, 6));
  case_c.push_back(Edge(2, 0));
  case_c.push_back(Edge(3, 5));
  case_c.push_back(Edge(4, 3));
  case_c.push_back(Edge(5, 4));
  case_c.push_back(Edge(5, 13));
  case_c.push_back(Edge(6, 2));
  case_c.push_back(Edge(6, 9));
  case_c.push_back(Edge(7, 6));
  case_c.push_back(Edge(8, 7));
  case_c.push_back(Edge(9, 10));
  case_c.push_back(Edge(9, 11));
  case_c.push_back(Edge(9, 12));
  case_c.push_back(Edge(10, 6));
  case_c.push_back(Edge(12, 10));
  case_c.push_back(Edge(13, 4));

  // detect the cycles
  //cd = new CycleDetector(V, &case_c);
  cd->mod_graph(V, &case_c);
  cycles = cd->detect_cycles();
  if (cycles != nullptr)  {
    cout << "cycles->size(): " << cycles->size() << endl; 
    for (unsigned i = 0; i < case_c.size(); i++)  {
      // print out the cycles
      if ((*cycles)[i])  cout << "Cyclic edge found: (" << case_c[i].first << ", " << case_c[i].second << ")" << endl;
    }

    cout << endl;

    // output a picture
    rs.push_back(new Rectangle(10,10+5,10,10+5)); // node 0
    rs.push_back(new Rectangle(20,20+5,40,40+5)); // node 1
    rs.push_back(new Rectangle(40,40+5,30,30+5)); // node 2
    rs.push_back(new Rectangle(30,30+5,60,60+5)); // node 3
    rs.push_back(new Rectangle(60,60+5,60,60+5)); // node 4
    rs.push_back(new Rectangle(10,10+5,90,90+5)); // node 5
    rs.push_back(new Rectangle(80,80+5,15,15+5)); // node 6
    rs.push_back(new Rectangle(110,110+5,15,15+5)); // node 7
    rs.push_back(new Rectangle(140,140+5,15,15+5)); // node 8
    rs.push_back(new Rectangle(110,110+5,60,60+5)); // node 9
    rs.push_back(new Rectangle(100,100+5,85,85+5)); // node 10
    rs.push_back(new Rectangle(140,140+5,50,50+5)); // node 11
    rs.push_back(new Rectangle(140,140+5,70,70+5)); // node 12
    rs.push_back(new Rectangle(45,45+5,90,90+5)); // node 13

    assert(rs.size() == V);

    output_svg(rs, case_c, "cycle_detector_case_c.svg", false, true, cycles);
    for(int i = 0; i < V; i++) { delete rs[i]; }
    rs.clear();
    delete cycles;
  }

  // create case D
  // case D consists of the same graph as case A but with a different starting location
  cout << endl << "ENTERING CASE D" << endl;
  V = 5;
  case_d.push_back(Edge(0, 1));
  case_d.push_back(Edge(1, 2));
  case_d.push_back(Edge(2, 3));
  case_d.push_back(Edge(3, 0));
  case_d.push_back(Edge(4, 1));

  // detect the cycles
  cd->mod_graph(V, &case_d);
  cycles = cd->detect_cycles();
  
  if (cycles != nullptr)  {
    cout << "cycles->size(): " << cycles->size() << endl; 
    for (unsigned i = 0; i < case_d.size(); i++)  {
      // print out the cycles
      if ((*cycles)[i])  cout << "Cyclic edge found: (" << case_d[i].first << ", " << case_d[i].second << ")" << endl;
    }

    cout << endl;

    // output a picture
    rs.push_back(new Rectangle(65,65+5,60,60+5));
    rs.push_back(new Rectangle(65,65+5,30,30+5));
    rs.push_back(new Rectangle(30,30+5,30,30+5));
    rs.push_back(new Rectangle(30,30+5,60,60+5));
    rs.push_back(new Rectangle(10,10+5,10,10+5));

    assert(rs.size() == V);

    output_svg(rs, case_d, "cycle_detector_case_d.svg", false, true, cycles);
    for (unsigned i = 0; i < rs.size(); i++)  { delete rs[i]; }
    rs.clear();
    delete cycles;
  }
  else  {
    cout << "No cycles found" << endl;
  }

  // create case E
  // case E is a reordering of case C
  cout << endl << "ENTERING CASE E" << endl;
  V = 14;
  case_e.push_back(Edge(0, 6));
  case_e.push_back(Edge(0, 9));
  case_e.push_back(Edge(1, 4));
  case_e.push_back(Edge(3, 10));
  case_e.push_back(Edge(4, 3));
  case_e.push_back(Edge(5, 0));
  case_e.push_back(Edge(6, 7));
  case_e.push_back(Edge(7, 0));
  case_e.push_back(Edge(7, 2));
  case_e.push_back(Edge(7, 10));
  case_e.push_back(Edge(8, 5));
  case_e.push_back(Edge(9, 12));
  case_e.push_back(Edge(9, 11));
  case_e.push_back(Edge(9, 13));
  case_e.push_back(Edge(10, 1));
  case_e.push_back(Edge(10, 4));
  case_e.push_back(Edge(12, 0));
  case_e.push_back(Edge(11, 12));

  // detect the cycles
  cd->mod_graph(V, &case_e);
  cycles = cd->detect_cycles();
  if (cycles != nullptr)  {
    cout << "cycles->size(): " << cycles->size() << endl; 
    for (unsigned i = 0; i < case_e.size(); i++)  {
      // print out the cycles
      if ((*cycles)[i])  cout << "Cyclic edge found: (" << case_e[i].first << ", " << case_e[i].second << ")" << endl;
    }

    cout << endl;

    // output a picture
    rs.push_back(new Rectangle(80,80+5,15,15+5)); // node 6
    rs.push_back(new Rectangle(45,45+5,90,90+5)); // node 13
    rs.push_back(new Rectangle(20,20+5,40,40+5)); // node 1
    rs.push_back(new Rectangle(30,30+5,60,60+5)); // node 3
    rs.push_back(new Rectangle(60,60+5,60,60+5)); // node 4
    rs.push_back(new Rectangle(110,110+5,15,15+5)); // node 7
    rs.push_back(new Rectangle(40,40+5,30,30+5)); // node 2
    rs.push_back(new Rectangle(10,10+5,10,10+5)); // node 0
    rs.push_back(new Rectangle(140,140+5,15,15+5)); // node 8
    rs.push_back(new Rectangle(110,110+5,60,60+5)); // node 9
    rs.push_back(new Rectangle(10,10+5,90,90+5)); // node 5
    rs.push_back(new Rectangle(140,140+5,70,70+5)); // node 12
    rs.push_back(new Rectangle(100,100+5,85,85+5)); // node 10
    rs.push_back(new Rectangle(140,140+5,50,50+5)); // node 11

    assert(rs.size() == V);

    output_svg(rs, case_e, "cycle_detector_case_e.svg", false, true, cycles);
    for(int i = 0; i < V; i++) { delete rs[i]; }
    rs.clear();
    delete cycles;
  }

  // create case F
  // case F consists of the same graph with sinks
  cout << endl << "ENTERING CASE F" << endl;
  V = 3;
  case_f.push_back(Edge(0, 1));
  case_f.push_back(Edge(0, 2));
  case_f.push_back(Edge(1, 2));

  // detect the cycles
  cd->mod_graph(V, &case_f);
  cycles = cd->detect_cycles();
  
  if (cycles != nullptr)  {
    // output a picture
    rs.push_back(new Rectangle(10,10+5,10,10+5));
    rs.push_back(new Rectangle(40,40+5,30,30+5));
    rs.push_back(new Rectangle(15,15+5,60,60+5));

    assert(rs.size() == V);

    output_svg(rs, case_f, "cycle_detector_case_f.svg", false, true, cycles);
    for (unsigned i = 0; i < rs.size(); i++)  { delete rs[i]; }
    rs.clear();

    cout << "No cycles found" << endl; 
  }

  // This tests the cycle detectors ability to cycle through a chain of cyclic ancestors
  cout << endl << "ENTERING CYCLIC ANCESTOR TEST" << endl;
  Node a(1), b(2), c(3), d(4), e(5), f(6);
  // set up the chain
  f.cyclicAncestor = &e;
  e.cyclicAncestor = &d;
  d.cyclicAncestor = &c;
  c.cyclicAncestor = &b;
  b.cyclicAncestor = &a;
  a.cyclicAncestor = &a;

  Node *ca = cd->get_highest_ca(&f);
  if (ca != nullptr)  { cout << "Highest cyclic ancestor found at vertex(" << ca->id << ")" << endl; }

  // create case G
  // case G hows the ability to find nested cycles and to reassign cyclic ancestors
  cout << endl << "ENTERING CASE G" << endl;
  V = 7;
  case_g.push_back(Edge(0, 1));
  case_g.push_back(Edge(1, 2));
  case_g.push_back(Edge(2, 3));
  case_g.push_back(Edge(3, 4));
  case_g.push_back(Edge(4, 5));
  case_g.push_back(Edge(5, 6));
  case_g.push_back(Edge(6, 5));
  case_g.push_back(Edge(5, 4));
  case_g.push_back(Edge(4, 3));
  case_g.push_back(Edge(3, 2));
  case_g.push_back(Edge(2, 1));
  case_g.push_back(Edge(1, 0));
  case_g.push_back(Edge(0, 6));

  // detect the cycles
  cd->mod_graph(V, &case_g);
  cycles = cd->detect_cycles();
  
  if (cycles != nullptr)  {
    cout << "cycles->size(): " << cycles->size() << endl; 
    for (unsigned i = 0; i < case_g.size(); i++)  {
      // print out the cycles
      if ((*cycles)[i])  cout << "Cyclic edge found: (" << case_g[i].first << ", " << case_g[i].second << ")" << endl;
    }

    cout << endl;

    // output a picture
    cout << "No picture generated" << endl;

    /*rs.push_back(new Rectangle(50,50+5,10,10+5));
    rs.push_back(new Rectangle(50,50+5,30,30+5));
    rs.push_back(new Rectangle(50,50+5,60,60+5));
    rs.push_back(new Rectangle(10,10+5,70,70+5));
    rs.push_back(new Rectangle(50,50+5,100,100+5));
    rs.push_back(new Rectangle(10,10+5,40,40+5));

    assert(rs.size() == V);

    output_svg(rs, case_a, "cycle_detector_case_g.svg", false, true, cycles);
    for (unsigned i = 0; i < rs.size(); i++)  { delete rs[i]; }
    rs.clear();*/
    delete cycles;
  }
  else  {
    cout << "No cycles found" << endl;
  }

  // END TEST
  delete cd;

  return 0;
}
