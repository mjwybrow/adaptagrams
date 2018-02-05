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
#include <cmath>
#include <time.h>
#include <valarray>
#include <fstream>
#include <sstream>

#include <libavoid/libavoid.h>
#include <libavoid/router.h>

#include "graphlayouttest.h"

using namespace std;
using namespace cola;
using namespace vpsc;

/*
// |V|=12, |E|=23
static const unsigned DAGDEPTH = 3;
static const unsigned BRANCHFACTOR = 3;
static const double EXTRAEDGEPROB = 0.5;
*/
/*
// |V|=26, |E|=61
static const unsigned DAGDEPTH = 3;
static const unsigned BRANCHFACTOR = 4;
static const double EXTRAEDGEPROB = 0.1;
*/

/*
// |V|=62, |E|=85
static const unsigned DAGDEPTH = 4;
static const unsigned BRANCHFACTOR = 4;
static const double EXTRAEDGEPROB = 0.01;
*/
/*
// |V|=131, |E|=166
static const unsigned DAGDEPTH = 5;
static const unsigned BRANCHFACTOR = 4;
static const double EXTRAEDGEPROB = 0.005;
*/

static const unsigned DAGDEPTH = 6;
static const unsigned BRANCHFACTOR = 4;
static const double EXTRAEDGEPROB = 0.002;
/*
// |V|=343, |E|=487
seed=1208390913;
static const unsigned DAGDEPTH = 6;
static const unsigned BRANCHFACTOR = 4;
static const double EXTRAEDGEPROB = 0.002;
*/

void makeEdge(unsigned u, unsigned v,
        vector<Edge> &edges, CompoundConstraints &ccs) {
    edges.push_back(make_pair(u,v));
    ccs.push_back(new SeparationConstraint(vpsc::YDIM, u,v,5));
}
vector<Edge> random_dag(unsigned depth, unsigned maxbranch, unsigned &V,
        CompoundConstraints &ccs) {
    printf("DAG depth=%d\nmaxbranch=%d\nextraedgeprob%f\n",depth,maxbranch,EXTRAEDGEPROB);
	vector<Edge> edges;
    unsigned lstart=0, lend=1;
    V=0;
	for(unsigned i=0;i<depth;i++) {
        for(unsigned j=lstart;j<lend;j++) {
            //makeEdge(j,++V,edges,cy);
            //makeEdge(j,++V,edges,cy);
            for(unsigned k=0;k<maxbranch;k++) {
                double r=(double)rand()/(double)RAND_MAX;
                if(r < 0.5) {
                    makeEdge(j,++V,edges,ccs);
                }
            }
        }
        lstart=lend;
        lend=V+1;
    }
    V++;
    /*
    DFS::Graph dfs(V,edges);
    for(unsigned i=1;i<dfs.order.size();i++) {
        cx.push_back(
                new SeparationConstraint(dfs.order[i-1],dfs.order[i],0.5));
    }
    */
    for(unsigned i=0;i<V;++i) {
        for(unsigned j=i+1;j<V;++j) {
            double r=(double)rand()/(double)RAND_MAX;
            if(r < EXTRAEDGEPROB) {
                makeEdge(i,j,edges,ccs);
            }
        }
    }
    /*
    for(unsigned i=0;i<dfs.leaves.size();i++) {
        for(unsigned j=1;j<dfs.leaves[i].size();j++) {
            cx.push_back( new SeparationConstraint(dfs.leaves[i][j-1],dfs.leaves[i][j],10));
        }
    }
    */
	return edges;
}
void removeoverlaps(vpsc::Rectangles &rs, bool bothaxes) {
	double xBorder=0, yBorder=0;
    static const double EXTRA_GAP=1e-5;
	unsigned n=rs.size();
	try {
		// The extra gap avoids numerical imprecision problems
		Rectangle::setXBorder(xBorder+EXTRA_GAP);
		Rectangle::setYBorder(yBorder+EXTRA_GAP);
        vpsc::Variables vs(n);
		unsigned i=0;
		for(Variables::iterator v=vs.begin();v!=vs.end();++v,++i) {
			*v=new Variable(i,0,1);
		}
        vpsc::Constraints cs;
        vpsc::generateXConstraints(rs,vs,cs,bothaxes);
        vpsc::IncSolver vpsc_x(vs,cs);
		vpsc_x.solve();
        vpsc::Rectangles::iterator r=rs.begin();
		for(Variables::iterator v=vs.begin();v!=vs.end();++v,++r) {
			assert((*v)->finalPosition==(*v)->finalPosition);
			(*r)->moveCentreX((*v)->finalPosition);
		}
		assert(r==rs.end());
		for_each(cs.begin(),cs.end(),vpsc::delete_object());
		cs.clear();
        if(bothaxes) {
            // Removing the extra gap here ensures things that were moved to be adjacent to one another above are not considered overlapping
            Rectangle::setXBorder(Rectangle::xBorder-EXTRA_GAP);
            vpsc::generateYConstraints(rs,vs,cs);
            vpsc::IncSolver vpsc_y(vs,cs);
            vpsc_y.solve();
            r=rs.begin();
            for(Variables::iterator v=vs.begin();v!=vs.end();++v,++r) {
                (*r)->moveCentreY((*v)->finalPosition);
            }
            for_each(cs.begin(),cs.end(),vpsc::delete_object());
            cs.clear();
            Rectangle::setYBorder(Rectangle::yBorder-EXTRA_GAP);
            vpsc::generateXConstraints(rs,vs,cs,false);
            vpsc::IncSolver vpsc_x2(vs,cs);
            vpsc_x2.solve();
            r=rs.begin();
            for(Variables::iterator v=vs.begin();v!=vs.end();++v,++r) {
                (*r)->moveCentreX((*v)->finalPosition);
            }
            for_each(cs.begin(),cs.end(),vpsc::delete_object());
        }
		for_each(vs.begin(),vs.end(),vpsc::delete_object());
	} catch (char *str) {
		std::cerr<<str<<std::endl;
		for(vpsc::Rectangles::iterator r=rs.begin();r!=rs.end();++r) {
			std::cerr << **r <<std::endl;
		}
	}
    Rectangle::setXBorder(xBorder);
    Rectangle::setYBorder(yBorder);
}
/*
void writeTextFile(vector<cola::Edge>& edges) {
    ofstream outfile("new.txt",ofstream::binary);
    for(vector<cola::Edge>::iterator e=edges.begin();e!=edges.end();++e) {
        outfile<<"node"<<e->first<<",node"<<e->second<<endl;
    }
    outfile.close();
}
*/
/*
 * Make feasible:
 *   - remove overlaps between rectangular boundaries of nodes/clusters
 *     (respecting structural constraints)
 *   - perform routing (preserve previous topology using rubber banding)
 */
void makeFeasible(vpsc::Rectangles& rs, vector<cola::Edge>& edges,
    std::vector<topology::Edge*>& routes,
    std::vector<topology::Node*>& topologyNodes, double defaultEdgeLength) {
    printf("Removing overlaps...\n");
    removeoverlaps(rs,false);
    printf("done.\n");
    printf("Running libavoid to compute routes...\n");
    clock_t libavoidstarttime=clock();
    // find feasible routes for edges
    Avoid::Router *router = new Avoid::Router(Avoid::PolyLineRouting);
    // Use rotational sweep for point visibility
    router->UseLeesAlgorithm = true;
    // Don't use invisibility graph.
    router->InvisibilityGrph = false;
    double g=0; // make shape that libavoid sees slightly smaller
    for(unsigned i=0;i<rs.size();++i) {
        vpsc::Rectangle* r=rs[i];
        double x=r->getMinX()+g;
        double X=r->getMaxX()-g;
        double y=r->getMinY()+g;
        double Y=r->getMaxY()-g;
        // Create the ShapeRef:
        Avoid::Polygon shapePoly(4);
        // AntiClockwise!
        shapePoly.ps[0] = Avoid::Point(X,y);
        shapePoly.ps[1] = Avoid::Point(X,Y);
        shapePoly.ps[2] = Avoid::Point(x,Y);
        shapePoly.ps[3] = Avoid::Point(x,y);
        //if(i==4||i==13||i==9) {
            //printf("rect[%d]:{%f,%f,%f,%f}\n",i,x,y,X,Y);
        //}
        unsigned int shapeID = i + 1;
        Avoid::ShapeRef *shapeRef = new Avoid::ShapeRef(router, shapePoly,
                shapeID);
        // ShapeRef constructor makes a copy of polygon so we can free it:
        router->addShape(shapeRef);
    }
    for(unsigned i=0;i<edges.size();++i) {
        cola::Edge e=edges[i];
        Avoid::ConnRef *connRef;
        unsigned int connID = i + rs.size() + 1;
        Rectangle* r0=rs[e.first], *r1=rs[e.second];
        Avoid::Point srcPt(r0->getCentreX(),r0->getCentreY());
        Avoid::Point dstPt(r1->getCentreX(),r1->getCentreY());
        connRef = new Avoid::ConnRef(router, srcPt, dstPt, connID);
        router->processTransaction();
        const Avoid::Polygon& route = connRef->route();
        vector<topology::EdgePoint*> eps;
        eps.push_back( new topology::EdgePoint( topologyNodes[e.first],
                    topology::EdgePoint::CENTRE));
        for(size_t j=1;j+1<route.size();j++) {
            const Avoid::Point& p = route.ps[j];
            const unsigned nodeID=p.id-1;
            topology::Node* node=topologyNodes[nodeID];
            topology::EdgePoint::RectIntersect ri;
            switch(p.vn) {
                case 0: ri=topology::EdgePoint::BR;
                        break;
                case 1: ri=topology::EdgePoint::TR;
                        break;
                case 2: ri=topology::EdgePoint::TL;
                        break;
                case 3: ri=topology::EdgePoint::BL;
                        break;
                default: ri=topology::EdgePoint::CENTRE;
            }
            eps.push_back(new topology::EdgePoint(node,ri));
        }
        eps.push_back(new topology::EdgePoint(topologyNodes[e.second],
                    topology::EdgePoint::CENTRE));
        topology::Edge* edgeRoute=new topology::Edge(i,defaultEdgeLength, eps);
        edgeRoute->assertConvexBends();
        routes.push_back(edgeRoute);

    }
    writeFile(topologyNodes,routes,"beautify0.svg");
    assert(topology::assertNoSegmentRectIntersection(topologyNodes,routes));
    double libavoidtime=double(clock()-libavoidstarttime)/double(CLOCKS_PER_SEC);
    cout << "done. Libavoid ran in " << libavoidtime << " seconds" << endl;
    delete router;
}
int main() {
    unsigned V;
    CompoundConstraints ccs;

    int seed = time(nullptr);
    //seed=1207906420;
    //seed=1207920674;
    //seed=1207982613;
    //seed=1207984219;
    seed=1207984299;
    //seed=1207984743;
    //seed=1207985027; // very short edge which seems to cause problems
    //seed=1207986026; // error if we don't check neighbour is actually on scanline when determining visibility when generating straight constraints
    //seed=1207991731;
    //seed=1208303930;
    //seed=1208304508;
    //seed=1208316284;
    //seed=1208319019;
    //seed=1208321702;
    printf("random seed=%d\n",seed);
    srand(seed);
    vector<Edge> es = random_dag(DAGDEPTH,BRANCHFACTOR,V,ccs);
    double defaultEdgeLength=40;

    cout << "V="<<V<<endl;
    cout << "E="<<es.size()<<endl;
    double width=700;
    double height=700;
    vector<pair<double,double> > startpos(V);
    for(unsigned i=0;i<V;i++) {
        double x=getRand(width), y=getRand(height);
        startpos[i]=make_pair(x,y);
    }
	vector<vpsc::Rectangle*> rs;
    vector<topology::Node*> topologyNodes;
    vector<topology::Edge*> routes;
	for(unsigned i=0;i<V;i++) {
		double x=getRand(width), y=getRand(height);
        vpsc::Rectangle* r =new vpsc::Rectangle(x,x+30,y,y+10);
		rs.push_back(r);
        topologyNodes.push_back(new topology::Node(i,r));
	}
	CheckProgress test(0.01,100);
    clock_t unconstrainedstarttime=clock();
    //writeTextFile(es);
    es.clear();
	ConstrainedFDLayout alg2(rs,es,defaultEdgeLength,nullptr,test);
    //alg2.setConstraints(&ccs);

    alg2.makeFeasible(true);
    alg2.run();

    alg2.outputInstanceToSVG();
#if 0
    double totaltime=0;
    double unconstrainedtime=double(clock()-unconstrainedstarttime)/double(CLOCKS_PER_SEC);
    totaltime+=unconstrainedstarttime;
    cout<<"unconstrained layout ran in "<<unconstrainedtime<<" seconds"<<endl;
    clock_t makefeasiblestarttime=clock();
    makeFeasible(rs,es,routes,topologyNodes,defaultEdgeLength);
    double makefeasibletime=double(clock()-makefeasiblestarttime)/double(CLOCKS_PER_SEC);
    totaltime+=makefeasibletime;
    cout<<"makefeasible ran in "<<makefeasibletime<<" seconds"<<endl;
    clock_t beautifystarttime=clock();
    test.reset();
    alg2.setTopology(&topologyNodes, &routes);
    writeFile(topologyNodes,routes,"beautify1.svg");
    std::stringstream dunnartfile;
    dunnartfile << "v" << V << "e" << (int) es.size() << ".svg";
    writeDunnartFile(topologyNodes,es,dunnartfile.str().c_str());

	alg2.run();
    double beautifytime=double(clock()-beautifystarttime)/double(CLOCKS_PER_SEC);
    totaltime+=beautifytime;
    cout<<"beautify ran in "<<beautifytime<<" seconds"<<endl;
    cout<<"TOTAL="<<totaltime<<endl;
    cout <<"---------------------------------------------"<< endl;
    cout <<V<<" & "<<es.size()<<" & "<<unconstrainedtime<<" & "<<makefeasibletime<<" & "<<beautifytime<<" & "<<totaltime<<" \\\\ % Seed: "<< seed <<endl;
    writeFile(topologyNodes,routes,"beautify2.svg");
#endif
	for(unsigned i=0;i<V;i++) {
		delete rs[i];
	}
    return 0;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=99 :
