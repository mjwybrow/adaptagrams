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
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/


#include <map>
#include <list>

#include "libvpsc/rectangle.h"
#include "libvpsc/assertions.h"
#include "libcola/commondefs.h"
#include "libcola/connected_components.h"

using namespace std;
using namespace vpsc;
namespace cola {
    Component::~Component() {
        /*
        for(unsigned i=0;i<scx.size();i++) {
            delete scx[i];
        }
        for(unsigned i=0;i<scy.size();i++) {
            delete scy[i];
        }
        */
    }
    void Component::moveRectangles(double x, double y) {
        for(unsigned i=0;i<rects.size();i++) {
            rects[i]->moveCentreX(rects[i]->getCentreX()+x);
            rects[i]->moveCentreY(rects[i]->getCentreY()+y);
        }
    }
    Rectangle* Component::getBoundingBox() 
    {
        Rectangle boundingBox;
        for (unsigned i = 0; i < rects.size(); ++i)
        {
            boundingBox = boundingBox.unionWith(*(rects[i]));
        }
        return new Rectangle(boundingBox);
    }

    namespace ccomponents {
        struct Node {
            unsigned id;
            bool visited;
            vector<Node*> neighbours;
            list<Node*>::iterator listPos;
            Rectangle* r;
        };
        // Depth first search traversal of graph to find connected component
        void dfs(Node* v,
                list<Node*>& remaining,
                Component* component,
                map<unsigned,pair<Component*,unsigned> > &cmap) {
            v->visited=true;
            remaining.erase(v->listPos);
            cmap[v->id]=make_pair(component,static_cast<unsigned>(component->node_ids.size()));
            component->node_ids.push_back(v->id);
            component->rects.push_back(v->r);
            for(unsigned i=0;i<v->neighbours.size();i++) {
                Node* u=v->neighbours[i];
                if(!u->visited) {
                    dfs(u,remaining,component,cmap);
                }
            }
        }
    }

    using namespace ccomponents;

    // for a graph of n nodes, return connected components
    void connectedComponents(
            const vector<Rectangle*> &rs,
            const vector<Edge> &es, 
            //const SeparationConstraints &scx,
            //const SeparationConstraints &scy,
            vector<Component*> &components) {
        unsigned n=rs.size();
        vector<Node> vs(n);
        list<Node*> remaining;
        for(unsigned i=0;i<n;i++) {
            vs[i].id=i;
            vs[i].visited=false;
            vs[i].r=rs[i];
            vs[i].listPos = remaining.insert(remaining.end(),&vs[i]);
        }
        vector<Edge>::const_iterator ei;
        for(ei=es.begin();ei!=es.end();ei++) {
            vs[ei->first].neighbours.push_back(&vs[ei->second]);
            vs[ei->second].neighbours.push_back(&vs[ei->first]);
        }
        map<unsigned,pair<Component*,unsigned> > cmap;
        while(!remaining.empty()) {
            Component* component=new Component;
            Node* v=*remaining.begin();
            dfs(v,remaining,component,cmap);
            components.push_back(component);
        }
        for(ei=es.begin();ei!=es.end();ei++) {
            pair<Component*,unsigned> u=cmap[ei->first],
                                      v=cmap[ei->second];
            COLA_ASSERT(u.first==v.first);
            u.first->edges.push_back(make_pair(u.second,v.second));
        }
        /*
        SeparationConstraints::const_iterator ci;
        for(ci=scx.begin();ci!=scx.end();ci++) {
            SeparationConstraint *c=*ci;
            pair<Component*,unsigned> u=cmap[c->left],
                                      v=cmap[c->right];
            COLA_ASSERT(u.first==v.first);
            u.first->scx.push_back(
                    new SeparationConstraint(u.second,v.second,c->gap));
        }
        for(ci=scy.begin();ci!=scy.end();ci++) {
            SeparationConstraint *c=*ci;
            pair<Component*,unsigned> u=cmap[c->left],
                                      v=cmap[c->right];
            COLA_ASSERT(u.first==v.first);
            u.first->scy.push_back(
                    new SeparationConstraint(u.second,v.second,c->gap));
        }
        */
    }
    void separateComponents(const vector<Component*> &components) {
        unsigned n=components.size();
        vector<Rectangle*> bbs(n);
        valarray<double> origX(n);
        valarray<double> origY(n);
        for(unsigned i=0;i<n;i++) {
            bbs[i]=components[i]->getBoundingBox();
            origX[i]=bbs[i]->getCentreX();
            origY[i]=bbs[i]->getCentreY();
        }
        removeoverlaps(bbs);
        for(unsigned i=0;i<n;i++) {
            components[i]->moveRectangles(
                    bbs[i]->getCentreX()-origX[i],
                    bbs[i]->getCentreY()-origY[i]);
            delete bbs[i];
        }
    }
}
