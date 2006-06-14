/*
** vim: set cindent 
** vim: ts=4 sw=4 et tw=0 wm=0
*/
#ifndef STRAIGHTENER_H
#define STRAIGHTENER_H
#include <set>
#include <generate-constraints.h>
#include "gradient_projection.h"
#include <cola.h>
namespace straightener {
    class Node;
    struct Edge {
        unsigned id;
        unsigned openInd; // position in openEdges
        unsigned startNode, endNode;
        cola::Route* route;
        double xmin, xmax, ymin, ymax;
        vector<unsigned> dummyNodes;
        vector<unsigned> path;
        Edge(unsigned id, unsigned start, unsigned end, unsigned pts, Route* route)
        : id(id), startNode(start), endNode(end), route(route),
          xmin(DBL_MAX), xmax(-DBL_MAX), ymin(DBL_MAX), ymax(-DBL_MAX) 
        {
            for(unsigned i=0;i<route->n;i++) {
                xmin=min(xmin,route->xs[i]);
                xmax=max(xmax,route->ys[i]);
                ymin=min(ymin,route->xs[i]);
                ymax=max(ymax,route->ys[i]);
            }	
        }
        ~Edge() {
            delete [] xroute;
            delete [] yroute;
        }
        bool isEnd(unsigned n) {
            if(startNode==n||endNode==n) return true;
            return false;
        }
        void nodePath(vector<Node*>& nodes);
        void createRouteFromPath(double* X, double* Y) {
            delete route;
            route=new Route(path.size());
            for(unsigned i=0;i<path.size();i++) {
                route->xs[i]=X[path[i]];
                route->ys[i]=Y[path[i]];
            }
        }
        void xpos(double y, vector<double>& xs) {
            // search line segments for intersection points with y pos
            for(unsigned i=1;i<route->n;i++) {
                double ax=route->xs[i-1], bx=route->xs[i], ay=route->ys[i-1], by=route->ys[i];
                double r=(y-ay)/(by-ay);
                // as long as y is between ay and by then r>0
                if(r>0&&r<=1) {
                    xs.push_back(ax+(bx-ax)*r);
                }
            }
        }
        void ypos(double x, vector<double>& ys) {
            // search line segments for intersection points with x pos
            for(unsigned i=1;i<route->n;i++) {
                double ax=route->xs[i-1], bx=route->xs[i], ay=route->ys[i-1], by=route->ys[i];
                double r=(x-ax)/(bx-ax);
                // as long as y is between ax and bx then r>0
                if(r>0&&r<=1) {
                    ys.push_back(ay+(by-ay)*r);
                }
            }
        }
    };
    class Node {
    public:
        unsigned id;
        double x,y;
        double scanpos;
        double width, height;
        double xmin, xmax, ymin, ymax;
        Edge *edge;
        bool dummy;
        double weight;
        bool open;
        Node(unsigned id, Rectangle* r) :
            id(id),x(r->getCentreX()),y(r->getCentreY()), width(r->width()), height(r->height()),
            xmin(x-width/2),xmax(x+width/2),
            ymin(y-height/2),ymax(y+height/2),
            edge(NULL),dummy(false),weight(-0.1),open(false) { }
    private:
        friend void sortNeighbours(Node* v, Node* l, Node* r, 
            double conjpos, vector<Edge*>& openEdges, 
            vector<Node*>& L,vector<Node*>& nodes, bool horizontal);
        Node(unsigned id, double x, double y, Edge* e) : 
            id(id),x(x),y(y), width(4), height(width),
            xmin(x-width/2),xmax(x+width/2),
            ymin(y-height/2),ymax(y+height/2),
            edge(e),dummy(true),weight(-0.1)  {
                e->dummyNodes.push_back(id);
            }
    };
    struct CmpNodePos {
        bool operator() (const Node* u, const Node* v) const {
            if (u->scanpos < v->scanpos) {
                return true;
            }
            if (v->scanpos < u->scanpos) {
                return false;
            }
            return u < v;
        }
    };
    typedef std::set<Node*,CmpNodePos> NodeSet;
    void generateConstraints(vector<Node*>& nodes, vector<Edge*>& edges,vector<SimpleConstraint*>& cs, bool horizontal);
    void nodePath(Edge& e,vector<Node*>& nodes, vector<unsigned>& path);
}

#endif
