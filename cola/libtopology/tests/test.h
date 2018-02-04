/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libtopology - Classes used in generating and managing topology constraints.
 *
 * Copyright (C) 2007-2008  Monash University
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

#include "libtopology/topology_graph.h"
using namespace topology;

template <typename T>
T getRand(T range) {
    double d=static_cast<double>(rand())/static_cast<double>(RAND_MAX);
    T r=static_cast<T>(static_cast<double>(range)*d);
	return r;
}

void generateRandomRects(unsigned n, vpsc::Rectangles &rs) {
	rs.resize(n);
	const int rect_size = 50;
	const int fld_size = static_cast<int>(
            sqrt(static_cast<double>(rect_size * n) / 2.0));
	int coords[4];
	for (unsigned i = 0; i < n; ++i) {
		for (unsigned d = 0; d < 2; ++d) {
			//unsigned const end = 1 + (rand() % (fld_size - 1));
			//unsigned const start = rand() % end;
			int const start = getRand(fld_size);
			int const end = start + getRand(rect_size) + 1;
			coords[2 * d] = start;
			coords[2 * d + 1] = end;
		} 
		rs[i]=new vpsc::Rectangle(coords[0],coords[1],coords[2],coords[3]);
	}
}

topology::Node* addNode(Nodes& vs, double x, double y, double w, double h) {
    vpsc::Rectangle* r = new vpsc::Rectangle(x,x+w,y,y+h);
    topology::Node *v = new topology::Node(vs.size(), r, new vpsc::Variable(vs.size()));
    vs.push_back(v);
    return v;
}
struct delete_node {
	void operator() (topology::Node* v) {
        delete v->rect;
        delete v;
    }
};
void addToPath(EdgePoints& ps, topology::Node *v, topology::EdgePoint::RectIntersect i) {
    ps.push_back(new EdgePoint(v,i));
}

void noRectOverlaps(const vpsc::Rectangles& rs) {
	for(vpsc::Rectangles::const_iterator i=rs.begin();i!=rs.end();++i) {
		for(vpsc::Rectangles::const_iterator j=i+1;j!=rs.end();++j) {
            vpsc::Rectangle *u=*i, *v=*j;
            double xoverlap=u->overlapX(v);
            double yoverlap=u->overlapY(v);
			if(xoverlap>0 && yoverlap>0) {
                printf("Overlap=%f,%f\n",xoverlap,yoverlap);
                throw "OVERLAP FOUND!";
			}
		}
	}
}
void writeFile(const topology::Nodes& vs, const Edges& es, const string& outputFileName) {
    const unsigned n=vs.size();
    vector<cola::Edge> cedges;

    for(unsigned i=0;i<es.size();i++) {
        cedges.push_back(make_pair(1,2));
    }

    vector<straightener::Route*> routes;
    for(vector<Edge*>::const_iterator e=es.begin();e!=es.end();++e) {
        routes.push_back((*e)->getRoute());
    }

    vector<string> labels(n);
    for(unsigned i=0;i<n;++i) {
        stringstream ss;
        ss << i;
        labels[i]=ss.str();
    }

    vector<vpsc::Rectangle*> rs;
    for(Nodes::const_iterator i=vs.begin();i!=vs.end();++i) {
	    rs.push_back((*i)->rect);
    }
    OutputFile of(rs,cedges,nullptr,outputFileName.c_str(),true,false);
    of.setLabels(labels);
    of.routes=&routes;
    of.generate();

    for_each(routes.begin(),routes.end(),delete_object());
}
void check(const TopologyConstraints& t, valarray<double>& g, cola::SparseMap& h,
        double* eg, double* eh) {
    for(unsigned i=0;i<t.n;++i) {
        //printf("g[%d]=%f,eg[%d]=%f\n",i,g[i],i,expectedG[i]);
        assert(fabs(g[i]-eg[i])<1e-4);
        for(unsigned j=0;j<t.n;++j) {
            //printf("h[%d,%d]=%f,eh[%d,%d]=%f\n",i,j,h(i,j),i,j,expectedH[i*n+j]);
            //printf("%f,",h(i,j));
            assert(fabs(h(i,j)-eh[i*t.n+j])<1e-4);
        }
        printf("\n");
    }
}
void setVariableDesiredPositions(vpsc::Variables& vs, const cola::DesiredPositionsInDim& des, valarray<double>& coords) {
    unsigned n=coords.size();
    assert(vs.size()>=n);
    for(unsigned i=0;i<n;++i) {
        vpsc::Variable* v=vs[i];
        v->desiredPosition = coords[i];
        v->weight=1;
    }
    for(cola::DesiredPositionsInDim::const_iterator d=des.begin();
            d!=des.end();++d) {
        assert(d->first<vs.size());
        vpsc::Variable* v=vs[d->first];
        v->desiredPosition = d->second;
        v->weight=10000;
    }
}
inline double dotProd(valarray<double> x, valarray<double> y) {
    assert(x.size()==y.size());
    double dp=0;
    for(unsigned i=0;i<x.size();i++) {
        dp+=x[i]*y[i]; 
    }
    return dp;
}
double computeStepSize(
        cola::SparseMatrix const &H, 
        valarray<double> const &g, 
        valarray<double> const &d)
{
    assert(g.size()==d.size());
    assert(g.size()==H.rowSize());
    // stepsize = g'd / (d' H d)
    double numerator = dotProd(g,d);
    valarray<double> Hd(d.size());
    H.rightMultiply(d,Hd);
    double denominator = dotProd(d,Hd);
    //assert(numerator>=0);
    //assert(denominator>=0);
    if(denominator==0) return 0;
    return numerator/denominator;
}
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:textwidth=80 :
