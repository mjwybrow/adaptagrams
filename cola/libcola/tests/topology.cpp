#include <vector>
#include <libvpsc/rectangle.h>
#include <libcola/topology_constraints.h>
#include <libcola/cola.h>
#include <libcola/output_svg.h>
#include <libproject/project.h>
using namespace std;
using namespace topology;

Nodes vs;
vector<vpsc::Rectangle*> rs;
EdgePoints ps;
Edges es;

Node* addNode(double x, double y, double w, double h) {
    vpsc::Rectangle* r = new vpsc::Rectangle(x,x+w,y,y+h);
	Node *v = new Node(vs.size(), r, new project::Variable(1,1));
    vs.push_back(v);
    rs.push_back(r);
    return v;
}
void addToPath(Node *v, topology::EdgePoint::RectIntersect i) {
    ps.push_back(new EdgePoint(v,i));
}

void simpleBend() {
    printf("test: simpleBend()\n");
	const unsigned V = 3;
    const char * ls[]={"0","1","2"};
    vector<const char *> labels(ls,ls+V);

    Node *v0 = addNode(571.500000,363.500000,63.000000,43.000000),
         *v1 = addNode(592.148117,469.500000,63.000000,43.000000),
         *v2 = addNode(541.500000,300.500000,63.000000,43.000000);
    addToPath(v1,EdgePoint::CENTRE);
    addToPath(v0,EdgePoint::TR);
    addToPath(v0,EdgePoint::BR);
    addToPath(v2,EdgePoint::CENTRE);

    es.push_back(new topology::Edge(ps));

    vector<straightener::Route*> routes;

    for(vector<topology::Edge*>::iterator i=es.begin();i!=es.end();++i) {
        topology::Edge* e=*i;
        routes.push_back(e->getRoute());
    }

    TopologyConstraints t(cola::HORIZONTAL,vs,es);

    vector<cola::Edge> cedges;
    cedges.push_back(make_pair(1,2));

    OutputFile of(rs,cedges,NULL,"simpleBend.svg",true,false);
    of.setLabels(3,ls);
    of.routes=&routes;
    of.generate();

    for_each(ps.begin(),ps.end(),delete_object());
    for_each(es.begin(),es.end(),delete_object());
    for_each(rs.begin(),rs.end(),delete_object());
    for(Nodes::iterator i=vs.begin();i!=vs.end();i++) {
        delete (*i)->var;
        delete *i;
    }
    for_each(routes.begin(),routes.end(),delete_object());
}

int main() {
    simpleBend();
    return 0;
}
  
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
