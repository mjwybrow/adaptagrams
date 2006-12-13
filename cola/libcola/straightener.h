#ifndef STRAIGHTENER_H
#define STRAIGHTENER_H
#include <set>
#include <libvpsc/rectangle.h>
#include <valarray>
#include "gradient_projection.h"
namespace cola { class Cluster; }
namespace straightener {
    using std::valarray;
    struct Route {
        Route(unsigned n) : n(n), xs(new double[n]), ys(new double[n]) {}
        ~Route() {
            delete [] xs;
            delete [] ys;
        }
        void boundingBox(double &xmin,double &ymin,double &xmax,double &ymax) {
            using std::min;
            using std::max;

            xmin=ymin=DBL_MAX;
            xmax=ymax=-DBL_MAX;
            for(unsigned i=0;i<n;i++) {
                xmin=min(xmin,xs[i]);
                xmax=max(xmax,xs[i]);
                ymin=min(ymin,ys[i]);
                ymax=max(ymax,ys[i]);
            } 
        }
        unsigned n;
        double *xs;
        double *ys;
    };
    class Node;
    struct Edge {
        unsigned id;
        unsigned openInd; // position in openEdges
        unsigned startNode, endNode;
        Route* route;
        double xmin, xmax, ymin, ymax;
        std::vector<unsigned> dummyNodes;
        std::vector<unsigned> path;
        std::vector<unsigned> activePath;
        // Edge with a non-trivial route
        Edge(unsigned id, unsigned start, unsigned end, Route* route)
        : id(id), startNode(start), endNode(end), route(route)
        {
            route->boundingBox(xmin,ymin,xmax,ymax);
        }
        // Edge with a trivial route
        Edge(unsigned id, unsigned start, unsigned end,
                double x1, double y1, double x2, double y2) 
        : id(id), startNode(start), endNode(end) {
            route = new Route(2);
            route->xs[0]=x1; route->ys[0]=y1;
            route->xs[1]=x2; route->ys[1]=y2;
            route->boundingBox(xmin,ymin,xmax,ymax);
        }
        ~Edge() {
            delete route;
        }
        void setRoute(Route* r) {
            delete route;
            route=r;
            route->boundingBox(xmin,ymin,xmax,ymax);
        }
        bool isEnd(unsigned n) {
            if(startNode==n||endNode==n) return true;
            return false;
        }
        void nodePath(std::vector<Node*>& nodes);
        void createRouteFromPath(valarray<double> const & X, valarray<double> const & Y) {
            Route* r=new Route(path.size());
            for(unsigned i=0;i<path.size();i++) {
                r->xs[i]=X[path[i]];
                r->ys[i]=Y[path[i]];
            }
            setRoute(r);
        }
        void xpos(double y, std::vector<double>& xs) {
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
        void ypos(double x, std::vector<double>& ys) {
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
    class Cluster {
    public:
        double scanpos;
        std::vector<Edge*> boundary;
        void getActualBoundary(valarray<double> &X, valarray<double> &Y) const
        {
            unsigned n=0;
            for(std::vector<Edge*>::const_iterator e=boundary.begin();
                    e!=boundary.end();e++) {
                n+=(*e)->route->n;
            }
            X.resize(n);
            Y.resize(n);
            unsigned i=0;
            for(std::vector<Edge*>::const_iterator e=boundary.begin();
                    e!=boundary.end();e++) {
                Route* r=(*e)->route;
                for(unsigned j=0;j<r->n;j++) {
                    X[i]=r->xs[j];
                    Y[i++]=r->ys[j];
                }
            }
        }
    };
    class Node {
    public:
        unsigned id;
        Cluster* cluster;
        // Nodes may optionally belong to a cluster.  
        // Neg cluster_id means no cluster - i.e. top-level membership.
        double x,y;
        double scanpos;
        double width, height;
        double xmin, xmax, ymin, ymax;
        Edge* edge;
        bool dummy; // nodes on edge paths (but not ends) are dummy
        bool scan; // triggers scan events
        bool active; // node is active if it is not dummy or is dummy and involved in
                     // a violated constraint
        bool open; // a node is opened (if scan is true) when the scanline first reaches
                   // its boundary and closed when the scanline leaves it.
        Node(const unsigned id, vpsc::Rectangle const * r) :
            id(id),cluster(NULL),
            x(r->getCentreX()),y(r->getCentreY()), width(r->width()), height(r->height()),
            xmin(x-width/2),xmax(x+width/2),
            ymin(y-height/2),ymax(y+height/2),
            edge(NULL),dummy(false),scan(true),active(true),open(false) { }
        Node(const unsigned id, const double x, const double y) :
            id(id),cluster(NULL),
            x(x),y(y),width(4),height(width),
            xmin(x-width/2),xmax(x+width/2),
            ymin(y-height/2),ymax(y+height/2),
            edge(NULL),dummy(false),scan(false),active(true),open(false) {};

    private:
        friend void sortNeighbours(const cola::Dim dim, Node * v, Node * l, Node * r, 
            const double conjpos, std::vector<Edge*> const & openEdges, 
            std::vector<Node *>& L, std::vector<Node *>& nodes);
        Node(const unsigned id, const double x, const double y, Edge* e) : 
            id(id),cluster(NULL),
            x(x),y(y), width(4), height(width),
            xmin(x-width/2),xmax(x+width/2),
            ymin(y-height/2),ymax(y+height/2),
            edge(e),dummy(true),scan(false),active(false)  {
                e->dummyNodes.push_back(id);
            }
    };
    struct CmpNodePos {
        bool operator() (const Node* u, const Node* v) const {
            double upos = u->scanpos;
            double vpos = v->scanpos;
            bool tiebreaker = u < v;
            if (u->cluster != v->cluster) {
                if(u->cluster!=NULL) {
                    upos = u->cluster->scanpos;
                }
                if(v->cluster!=NULL) {
                    vpos = v->cluster->scanpos;
                }
                tiebreaker = u->cluster < v->cluster;
            }
            if (upos < vpos) {
                return true;
            }
            if (vpos < upos) {
                return false;
            }
            return tiebreaker;
        }
    };
    typedef std::set<Node*,CmpNodePos> NodeSet;
    void generateConstraints(
            const cola::Dim dim, 
            std::vector<Node*> & nodes, 
            std::vector<Edge*> & edges, 
            std::vector<cola::SeparationConstraint*>& cs);
    void nodePath(Edge& e, std::vector<Node*>& nodes, std::vector<unsigned>& path);
    void generateClusterBoundaries(
		    const cola::Dim dim,
		    std::vector<straightener::Node*> & nodes,
            std::vector<straightener::Edge*> & edges,
            std::vector<vpsc::Rectangle*> const & rs,
		    std::vector<cola::Cluster*> const & clusters,
		    std::vector<straightener::Cluster*>& sclusters);
}

#endif
// vim: cindent ts=4 sw=4 et tw=0 wm=0
