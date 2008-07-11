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

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <boost/regex.hpp>
#include "graphlayouttest.h"

using namespace std;
using namespace cola;

typedef map<unsigned,unsigned> NodeIdMap;
typedef map<string,list<unsigned> > Partitions;

Edge readEdge(ifstream &f) {
    string l;
    int indent=1;
    unsigned source, target;
    boost::regex sourceRE(".*source\\s+(\\d+).*");
    boost::regex targetRE(".*target\\s+(\\d+).*");
    boost::smatch matches;
    while(!getline(f,l).eof()) {
        if(boost::regex_match(l,matches,sourceRE)) {
            std::istringstream i(matches[1]);
            i>>source;
        }
        if(boost::regex_match(l,matches,targetRE)) {
            std::istringstream i(matches[1]);
            i>>target;
        }
        if(l.find("[")!=string::npos) {
            indent++;
        }
        if(l.find("]")!=string::npos) {
            indent--;
            if(indent==0) {
                return make_pair(source,target);
            }
        }
    }
    cerr << "Incomplete edge definition!" << endl;
    return make_pair(0,0);
}

unsigned readNode(ifstream &f, NodeIdMap &nodeIdMap, 
        Partitions &partitions) {
    static unsigned nodeCtr=0;
    int indent=1;
    string l;
    boost::regex idRE("\\s*id\\s+(\\d+).*");
    boost::regex fillRE(".*fill\\s+\"#(.+)\".*");
    boost::smatch matches;
    unsigned id;
    while(!getline(f,l).eof()) {
        if(boost::regex_match(l,matches,idRE)) {
            std::istringstream i(matches[1]);
            i>>id;
            nodeIdMap[id]=nodeCtr++;
        }
        if(boost::regex_match(l,matches,fillRE)) {
            string partition(matches[1]);
            Partitions::iterator i=partitions.find(partition);
            partitions[partition].push_back(nodeIdMap[id]);
        }
        if(l.find("[")!=string::npos) {
            indent++;
        }
        if(l.find("]")!=string::npos) {
            indent--;
            if(indent==0) {
                return nodeCtr;
            }
        }
    }
    cerr << "Incomplete node definition!" << endl;
    return 0;
}

struct PostIteration : TestConvergence {
    PostIteration(vector<vpsc::Rectangle*> &rs, vector<list<unsigned> > &ps, vector<double> &bs, const double d,const unsigned i) : TestConvergence(d,i), rs(rs), ps(ps), bs(bs) {}
    bool operator()(const double new_stress, valarray<double> & X, valarray<double> & Y) {
        static int iterations=0;
        unsigned n=X.size();
        double centreX=0,centreY=0;
        for(unsigned i=0;i<n;i++) {
            centreX+=X[i];
            centreY+=Y[i];
        }
        centreX/=(double)n;
        centreY/=(double)n;
        //printf("centre=(%f,%f)\n",centreX,centreY);
        for(unsigned i=0;i<ps.size();i++) {
            for(list<unsigned>::iterator v=ps[i].begin();v!=ps[i].end();v++) {
                double dx=X[*v]-centreX, dy=Y[*v]-centreY;
                double l=sqrt(dx*dx+dy*dy);
                if(i>0) {
                    double r=bs[i-1]+10;
                    if(l<r) {
                        double dx1=dx*(r/l), dy1=dy*(r/l);
                        X[*v]=centreX+dx1;
                        Y[*v]=centreY+dy1;
                    } 
                }
                if(i<ps.size()-1) {
                    double r=bs[i]-10;
                    if(l>r) {
                        double dx1=dx*(r/l), dy1=dy*(r/l);
                        X[*v]=centreX+dx1;
                        Y[*v]=centreY+dy1;
                    } 
                }
            }
        }
        for(unsigned i=0;i<rs.size();i++) {
            rs[i]->moveCentre(X[i],Y[i]);
        }
        cout << iterations++ << ":stress="<<new_stress<<endl;
        return TestConvergence::operator()(new_stress,X,Y);
    }
    vector<vpsc::Rectangle*> &rs;
    vector<list<unsigned> > &ps;
    vector<double> &bs;
};
int main() {
    //const char *fname="data/test.gml";
    string fname("data/uetzNetworkGSC-all.gml");
    ifstream f(fname.c_str());
    if(!f.is_open()) {
        cout << "Error opening file: " << fname << endl;
        exit(1);
    }
    unsigned V = 0;
    double defaultEdgeLength=40;
    vector<Edge> es;
    CompoundConstraints cx,cy;
    NodeIdMap nodeIdMap; 
    Partitions partitions;
    string l;
    boost::regex nodeRE(".*node\\s+\\[.*");
    boost::regex edgeRE(".*edge\\s+\\[.*");
    while(!getline(f,l).eof()) {
        if(boost::regex_match(l,nodeRE)) {
            V=readNode(f,nodeIdMap,partitions);
        }
        if(boost::regex_match(l,edgeRE)) {
            pair<unsigned,unsigned> e=readEdge(f);
            unsigned start=nodeIdMap[e.first];
            unsigned end=nodeIdMap[e.second];
            //printf("edge (%d,%d)\n",start,end);
            es.push_back(make_pair(start,end));
            //cy.push_back(
                //new SeparationConstraint(start,end,defaultEdgeLength/3));
        }
    }
    assert(V==nodeIdMap.size());
    /*
    unsigned p1=V++;
    unsigned p2=V++;
    // red at the top
    list<unsigned> &p=partitions[string("FF3333")];
    //double cgap=defaultEdgeLength/3;
    double cgap=0;
    for(list<unsigned>::iterator j=p.begin();j!=p.end();j++) {
        cy.push_back(
            new SeparationConstraint(*j,p1,cgap));
    }
    // yellow in the middle
    p=partitions[string("FFFF00")];
    for(list<unsigned>::iterator j=p.begin();j!=p.end();j++) {
        cy.push_back(
            new SeparationConstraint(p1,*j,cgap));
        cy.push_back(
            new SeparationConstraint(*j,p2,cgap));
    }
    // green at the bottom
    p=partitions[string("00FF00")];
    for(list<unsigned>::iterator j=p.begin();j!=p.end();j++) {
        cy.push_back(
            new SeparationConstraint(p2,*j,cgap));
    }
    */
    cout << "V="<<V<<endl;
    double width=1000;
    double height=1000;
    vector<vpsc::Rectangle*> rs;
    cout << "|V|=" << V << endl;
    //srand(time(NULL));
    for(unsigned i=0;i<V;i++) {
        double x=getRand(width), y=getRand(height);
        rs.push_back(new vpsc::Rectangle(x,x+1,y,y+1));
    }
    vector<list<unsigned> > ps;
    ps.push_back(partitions[string("FF3333")]);
    ps.push_back(partitions[string("FFFF00")]);
    ps.push_back(partitions[string("00FF00")]);
    vector<double> bs;
    bs.push_back(80);
    bs.push_back(120);
    PostIteration test(rs,ps,bs,0.0001,200);
    ConstrainedMajorizationLayout alg(rs,es,NULL,defaultEdgeLength,NULL,test);
    //ConstrainedFDLayout alg(rs,es,NULL,defaultEdgeLength,NULL,test);
    //alg.setConstrainedLayout(true);
    //alg.setScaling(true);
    //alg.setXConstraints(&cx);
    //alg.setYConstraints(&cy);
    alg.run();
    /*
    vector<ColourRGBA> colours(V);
    list<unsigned> &p=partitions[string("FF3333")];
    for(list<unsigned>::iterator i=p.begin();i!=p.end();i++) {
        //colours[*i]=ColourRGBA(1.,0,0.3,1.);
        colours[*i]=ColourRGBA(0,0,0,1.);
    }
    p=partitions[string("FFFF00")];
    for(list<unsigned>::iterator i=p.begin();i!=p.end();i++) {
        //colours[*i]=ColourRGBA((unsigned)255,162,99,255);
        colours[*i]=ColourRGBA(0,0,0,1.);
    }
    p=partitions[string("00FF00")];
    for(list<unsigned>::iterator i=p.begin();i!=p.end();i++) {
        //colours[*i]=ColourRGBA((unsigned)24,157,0,255);
        colours[*i]=ColourRGBA(0,0,0,1.);
    }
    */
    OutputFile of(rs,es,NULL,"gml_graph-constrained.svg",false,true);
    //of.colours=&colours;
    of.generate();
    for(unsigned i=0;i<V;i++) {
        delete rs[i];
    }
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
