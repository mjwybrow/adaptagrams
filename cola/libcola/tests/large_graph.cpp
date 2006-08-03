#include<iostream>
#include<iomanip>
#include<fstream>
#include<vector>
#include "graphlayouttest.h"

using namespace std;
using namespace cola;

struct CheckProgress : TestConvergence {
	CheckProgress(double d,unsigned i) : TestConvergence(d,i) {}
	bool operator()(double new_stress, double* X, double* Y) {
		cout << "stress="<<new_stress<<endl;
		return TestConvergence::operator()(new_stress,X,Y);
	}
};
int main() {
	ifstream f("data/1138_bus.txt");
	string startlabel, endlabel;
	unsigned V = 0;
	double defaultEdgeLength=40;
	vector<Edge> es;
	SimpleConstraints scy;
	while(!getline(f,startlabel,' ').eof()) {
		getline(f,endlabel);
		unsigned start = atoi(startlabel.c_str()),
			 end = atoi(endlabel.c_str());
		es.push_back(make_pair(start,end));
		scy.push_back(
			new SimpleConstraint(end,start,defaultEdgeLength/3));
		V=max(V,max(start,end));
	}
	V++;
	cout << "V="<<V<<endl;
	double width=1000;
	double height=1000;
	vector<vpsc::Rectangle*> rs;
	//srand(time(NULL));
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
		rs.push_back(new vpsc::Rectangle(x,x+1,y,y+1));
	}
	CheckProgress test(0.001,100);
	ConstrainedMajorizationLayout alg(rs,es,defaultEdgeLength,NULL,test);
	alg.setupConstraints(NULL,NULL,false,NULL,NULL,NULL,&scy);
	alg.run();
	output_svg(rs,es,"large_graph.svg");
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
}
