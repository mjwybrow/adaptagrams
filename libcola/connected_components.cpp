#include "cola.h"
using namespace std;

namespace cola {
    namespace ccomponents {
        struct Node {
            unsigned id;
            bool visited;
            vector<Node*> neighbours;
        };
        // Depth first search traversal of graph to find connected component
        void dfs(Node* v,
                set<Node*>& remaining,
                vector<unsigned>* component) {
            v->visited=true;
            remaining.erase(v);
            component->push_back(v->id);
            for(unsigned i=0;i<v->neighbours.size();i++) {
                Node* u=v->neighbours[i];
                if(!u->visited) {
                    dfs(u,remaining,component);
                }
            }
        }
    }

    using namespace ccomponents;

    // for a graph of n nodes, return connected components
    void connectedComponents(const unsigned n, 
            vector<Edge> &es, vector<vector<unsigned>*> &components) {
        vector<Node> vs(n);
        set<Node*> remaining;
        for(unsigned i=0;i<n;i++) {
            vs[i].id=i;
            vs[i].visited=false;
            remaining.insert(&vs[i]);
        }
        for(vector<Edge>::iterator e=es.begin();e!=es.end();e++) {
            vs[e->first].neighbours.push_back(&vs[e->second]);
        }
        while(!remaining.empty()) {
            vector<unsigned>* component=new vector<unsigned>;
            Node* v=*remaining.begin();
            dfs(v,remaining,component);
            components.push_back(component);
        }
    }
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
