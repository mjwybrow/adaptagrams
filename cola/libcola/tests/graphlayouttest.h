#include <libvpsc/rectangle.h>
#include <libcola/cola.h>
#include <libtopology/topology_graph.h>
#include <libcola/output_svg.h>
#include<sstream>

inline double getRand(double range) {
	return range*rand()/RAND_MAX;
}

namespace DFS {
using namespace std;
using namespace cola;
struct Node {
    unsigned id;
	bool visited;
	typedef vector<Node*> PNodes;
	PNodes neighbours;
	void visit(vector<unsigned> &order) {
		visited=true;
        unsigned mid=neighbours.size()/2;
        for(unsigned i=0;i<mid;i++) {
            if(!neighbours[i]->visited) {
                neighbours[i]->visit(order);
            }
        }
        order.push_back(id);
        for(unsigned i=mid;i<neighbours.size();i++) {
            if(!neighbours[i]->visited) {
                neighbours[i]->visit(order);
            }
        }
	}
	void visit_leaves(vector<unsigned> &order) {
        unsigned mid=neighbours.size()/2;
        Node *v;
        for(unsigned i=0;i<mid;i++) {
            v=neighbours[i];
            if(v->neighbours.size()==0) {
                order.push_back(v->id);
            }
        }
        order.push_back(id);
        for(unsigned i=mid;i<neighbours.size();i++) {
            v=neighbours[i];
            if(v->neighbours.size()==0) {
                order.push_back(v->id);
            }
        }
	}
};

struct Graph {
	typedef vector<Edge> Edges;
	typedef vector<Node> Nodes;
	Nodes nodes;
    vector<unsigned> order;
    vector<vector<unsigned> > leaves;
	Graph(unsigned n,vector<Edge> &edges) {
		nodes.resize(n);
        order.resize(0);
        leaves.resize(n);
		for(Edges::iterator i=edges.begin();i!=edges.end();i++) {
			nodes[i->first].neighbours.push_back(&nodes[i->second]);
		}
		for(unsigned i=0;i<nodes.size();i++) {
			nodes[i].id=i;
			nodes[i].visited=false;
		}
		for(Nodes::iterator i=nodes.begin();i!=nodes.end();i++) {
			if(!i->visited) {
				i->visit(order);
			}
		}
		for(unsigned i=0;i<nodes.size();i++) {
            leaves[i].resize(0);
			nodes[i].visit_leaves(leaves[i]);
		}
        assert(order.size()==nodes.size());
	}
};
} // namespace DFS
using namespace cola;
using namespace std;
struct CheckProgress : TestConvergence {
    CheckProgress(const double d,const unsigned i) : TestConvergence(d,i) {}
    bool operator()(const double new_stress, valarray<double> & X, valarray<double> & Y) {
        //cout << "stress="<<new_stress<<" iteration="<<iterations<<endl;
        return TestConvergence::operator()(new_stress,X,Y);
    }
};

enum SolverType { CG, UGP, SGP, IP };
void run_test(
        vector<pair<double,double> > const &startpos,
        vector<Edge> const &es, 
        const double defaultEdgeLength,
        CompoundConstraints &cx,
        CompoundConstraints &cy,
        const SolverType s, 
        const bool constrained,
        const char *fname,
        const char *testdesc) {
    vector<vpsc::Rectangle*> rs;
    unsigned V=startpos.size();
    cout << "|V|=" << V << endl;
    //srand(time(NULL));
    for(unsigned i=0;i<V;i++) {
        double x=startpos[i].first, y=startpos[i].second;
        rs.push_back(new vpsc::Rectangle(x,x+1,y,y+1));
    }
    CheckProgress test(0.001,100);
    clock_t starttime=clock();
    ConstrainedMajorizationLayout alg(rs,es,NULL,defaultEdgeLength,NULL,test);
    switch(s) {
        cout << "Solver: ";
        case SGP:
            cout << "Scaled GP" << endl;
            alg.setConstrainedLayout(true);
            alg.setScaling(true);
            break;
        case UGP:
            cout << "Unscaled GP" << endl;
            alg.setConstrainedLayout(true);
            alg.setScaling(false);
            break;
        case CG:
            cout << "Conjugate Gradient" << endl;
            alg.setConstrainedLayout(false);
            break;
        case IP:
            cout << "Interior Point" << endl;
            alg.setExternalSolver(true);
            alg.setConstrainedLayout(true);
            break;
    }
    if(!constrained) {
        cout << "Unconstrained layout" << endl;
    } else {
        cout << "Constrained layout" << endl;
        printf("|XConstraints|=%u, |YConstraints|=%u\n",(unsigned)cx.size(),(unsigned)cy.size());
        alg.setXConstraints(&cx);
        alg.setYConstraints(&cy);
    }
    alg.run();
    double t=double(clock()-starttime)/double(CLOCKS_PER_SEC);
    ostringstream ss;
    ss << fname << "_" << V << "_" << testdesc << ".svg";

    cout<<ss.str()<<" Time="<<t<<endl;
    OutputFile of(rs,es,NULL,ss.str().c_str(),false,true);
    of.generate();
    for(unsigned i=0;i<V;i++) {
        delete rs[i];
    }
}
void writeFile(const topology::Nodes& vs, const topology::Edges& es, const string& outputFileName) {
    const unsigned n=vs.size();
    vector<cola::Edge> cedges;

    for(unsigned i=0;i<es.size();i++) {
        cedges.push_back(make_pair(1,2));
    }

    vector<straightener::Route*> routes;
    for(topology::Edges::const_iterator e=es.begin();e!=es.end();++e) {
        routes.push_back((*e)->getRoute());
    }

    vector<string> labels(n);
    for(unsigned i=0;i<n;++i) {
        stringstream ss;
        ss << i;
        labels[i]=ss.str();
    }

    vector<vpsc::Rectangle*> rs;
    for(topology::Nodes::const_iterator i=vs.begin();i!=vs.end();++i) {
	    rs.push_back((*i)->rect);
    }
    OutputFile of(rs,cedges,NULL,outputFileName,true,false);
    of.setLabels(labels);
    of.routes=&routes;
    of.generate();

    for_each(routes.begin(),routes.end(),delete_object());
}

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
