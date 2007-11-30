#include <vector>
#include <libvpsc/rectangle.h>
#include <libcola/topology_constraints.h>
#include <libcola/cola.h>
#include <libcola/output_svg.h>
#include <libproject/project.h>
using namespace std;
using namespace topology;

Nodes vs;
EdgePoints ps;
Edges es;

Node* addNode(double x, double y, double w, double h) {
	Node *v = new Node(vs.size(),new project::Variable(1,1), new vpsc::Rectangle(x,x+w,y,y+h));
    vs.push_back(v);
    return v;
}
void addToPath(Node *v, topology::DummyNode::RectIntersect i) {
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
    addToPath(v1,DummyNode::CENTRE);
    addToPath(v0,DummyNode::TR);
    addToPath(v0,DummyNode::BR);
    addToPath(v2,DummyNode::CENTRE);

    es.push_back(new topology::Edge(vs));

    vector<straightener::Route*> routes;

    for(vector<topology::Edge*>::iterator i=tes.begin();i!=tes.end();++i) {
        topology::Edge* e=*i;
        routes.push_back(e->getRoute());
    }

    TopologyConstraints t(cola::HORIZONTAL,vs,tes);

    OutputFile of(rs,es,NULL,"simpleBend.svg",true,false);
    of.setLabels(3,ls);
    of.routes=&routes;
    of.generate();

    for_each(ps.begin(),ps.end(),delete_object());
    for_each(tes.begin(),tes.end(),delete_object());
    for(Nodes::iterator i=vs.begin();i!=vs.end();i++) {
        delete (*i)->rect;
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
