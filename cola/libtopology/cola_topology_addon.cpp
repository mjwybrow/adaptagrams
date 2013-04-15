/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libtopology - Classes used in generating and managing topology constraints.
 *
 * Copyright (C) 2012  Monash University
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
 * Author(s):  Michael Wybrow
*/

#include <utility>

#include "libvpsc/rectangle.h"
#include "libvpsc/constraint.h"
#include "libcola/compound_constraints.h"
#include "libtopology/topology_constraints.h"
#include "libtopology/cola_topology_addon.h"

// Needs to come last since it will include windows.h on WIN32 and
// may mess up C++ std library include on GCC 4.4
#include "libcola/cola_log.h"

namespace topology {

ColaTopologyAddon::ColaTopologyAddon()
    : cola::TopologyAddonInterface()
{
}

ColaTopologyAddon::ColaTopologyAddon(
        std::vector<topology::Node*>& tnodes, 
        std::vector<topology::Edge*>& routes)
    : cola::TopologyAddonInterface(),
      topologyNodes(tnodes),
      topologyRoutes(routes)
{
}

cola::TopologyAddonInterface *ColaTopologyAddon::clone(void) const
{
    return new ColaTopologyAddon(*this);
}

void ColaTopologyAddon::handleResizes(const cola::Resizes& resizeList,
        unsigned n, std::valarray<double>& X, std::valarray<double>& Y, 
        cola::CompoundConstraints& ccs, vpsc::Rectangles& boundingBoxes,
        cola::RootCluster* clusterHierarchy)
{
    FILE_LOG(cola::logDEBUG) << "ColaTopologyAddon::handleResizes()...";
    if(topologyNodes.empty()) {
        COLA_ASSERT(topologyRoutes.empty());
        return;
    }
    // all shapes to be resized are wrapped in a ResizeInfo and
    // placed in a lookup table, resizes, indexed by id
    ResizeMap resizes;
    for(cola::Resizes::const_iterator r=resizeList.begin();r!=resizeList.end();++r) {
        topology::ResizeInfo ri(topologyNodes[r->getID()],r->getTarget());
        resizes.insert(std::make_pair(r->getID(),ri));
    }
    vpsc::Variables xvs, yvs;
    vpsc::Constraints xcs, ycs;
    cola::setupVarsAndConstraints(n, ccs, vpsc::HORIZONTAL, boundingBoxes,
            clusterHierarchy, xvs, xcs, X);
    cola::setupVarsAndConstraints(n, ccs, vpsc::VERTICAL, boundingBoxes,
            clusterHierarchy, yvs, ycs, Y);
    topology::applyResizes(topologyNodes, topologyRoutes, clusterHierarchy,
            resizes, xvs, xcs, yvs, ycs);
    for_each(xvs.begin(), xvs.end(), delete_object());
    for_each(yvs.begin(), yvs.end(), delete_object());
    for_each(xcs.begin(), xcs.end(), delete_object());
    for_each(ycs.begin(), ycs.end(), delete_object());
    FILE_LOG(cola::logDEBUG) << "ColaTopologyAddon::handleResizes()... done.";
}

void ColaTopologyAddon::computePathLengths(unsigned short** G)
{
    // we don't need to compute attractive forces between nodes connected
    // by an edge if there is a topologyRoute between them (since the
    // p-stress force will be used instead)
    if (!topologyRoutes.empty()) {
        for(std::vector<topology::Edge*>::iterator i=topologyRoutes.begin();
                i!=topologyRoutes.end();++i) {
            topology::Edge* e=*i;
            if(!e->cycle()) {
                unsigned u=e->firstSegment->start->node->id,
                         v=e->lastSegment->end->node->id;
                G[u][v]=G[v][u]=2;
            }
        }
    }
}

double ColaTopologyAddon::computeStress(void) const
{
    double stress = 0;
    if (!topologyRoutes.empty())
    {
        stress = topology::computeStress(topologyRoutes);
        FILE_LOG(cola::logDEBUG2)<<"s(topology)="<<stress;
    }
    return stress;
}

bool ColaTopologyAddon::useTopologySolver(void) const
{
    return (!topologyNodes.empty() && !topologyRoutes.empty());
}

void ColaTopologyAddon::makeFeasible(bool generateNonOverlapConstraints, 
        vpsc::Rectangles& boundingBoxes, cola::RootCluster* clusterHierarchy)
{
    if (generateNonOverlapConstraints)
    {
        // Set up topologyNodes:
        unsigned nodesTotal = boundingBoxes.size();
        topologyNodes = topology::Nodes(nodesTotal);
        for (unsigned id = 0; id < nodesTotal; ++id)
        {
            topologyNodes[id] = new topology::Node(id, boundingBoxes[id]);
        }
    }
  
    if (clusterHierarchy)
    {
        // create cluster boundaries
        unsigned clusterCount=0;
        for (std::vector<cola::Cluster*>::iterator i = 
                clusterHierarchy->clusters.begin();
                i != clusterHierarchy->clusters.end(); ++i, ++clusterCount)
        {
            (*i)->computeBoundary(boundingBoxes);
            cola::ConvexCluster* c=dynamic_cast<cola::ConvexCluster*>(*i);
            if(c!=NULL) {
                double idealCircumference=2.0*sqrt(M_PI*c->area(boundingBoxes));
                std::vector<topology::EdgePoint*> eps;
                for(unsigned j=0;j<c->hullRIDs.size();++j) {
                    const unsigned id = c->nodes[c->hullRIDs[j]];
                    const unsigned char corner = c->hullCorners[j];
                    COLA_ASSERT(id < topologyNodes.size());
                    //cout << "addToPath(vs[" << id << "],";
                    topology::Node *node= topologyNodes[id];
                    topology::EdgePoint::RectIntersect ri;
                    switch(corner) {
                        case 0: 
                            ri=topology::EdgePoint::BR; 
                            //cout << "EdgePoint::BR);" << endl;
                            break;
                        case 1: 
                            ri=topology::EdgePoint::TR; 
                            //cout << "EdgePoint::TR);" << endl;
                            break;
                        case 2: 
                            ri=topology::EdgePoint::TL;
                            //cout << "EdgePoint::TL);" << endl;
                            break;
                        default:
                            COLA_ASSERT(corner==3);
                            ri=topology::EdgePoint::BL; 
                            //cout << "EdgePoint::BL);" << endl;
                            break;
                    }
                    eps.push_back(new topology::EdgePoint(node,ri));
                }
                eps.push_back(eps[0]);
                //cout << "addToPath(vs[" << eps[0]->node->id << "],(EdgePoint::RectIntersect)"<<eps[0]->rectIntersect<<");" << endl;
                topology::Edge* e = new topology::Edge(clusterCount,idealCircumference, eps);
                topologyRoutes.push_back(e);
            }
        }
    }
}

void ColaTopologyAddon::moveTo(const vpsc::Dim dim, 
        vpsc::Variables& vs, vpsc::Constraints& cs,
        std::valarray<double> &coords, cola::RootCluster* clusterHierarchy)
{
    topology::setNodeVariables(topologyNodes,vs);
    topology::TopologyConstraints t(dim, topologyNodes, topologyRoutes,
            clusterHierarchy, vs, cs);
    bool interrupted;
    int loopBreaker=100;
    do {
        interrupted=t.solve();
        loopBreaker--;
    } while(interrupted&&loopBreaker>0);
    for(topology::Nodes::iterator i=topologyNodes.begin();
            i!=topologyNodes.end();++i) {
        topology::Node* v=*i;
        coords[v->id]=v->rect->getCentreD(dim);
    }
}

double ColaTopologyAddon::applyForcesAndConstraints(
        cola::ConstrainedFDLayout *layout, const vpsc::Dim dim,
        std::valarray<double>& g, vpsc::Variables& vs,
        vpsc::Constraints& cs, std::valarray<double> &coords, 
        cola::DesiredPositionsInDim& des, double oldStress)
{
    FILE_LOG(cola::logDEBUG1) << "applying topology preserving layout...";
    vpsc::Rectangle::setXBorder(0);
    vpsc::Rectangle::setYBorder(0);
    if(dim==vpsc::HORIZONTAL) {
        vpsc::Rectangle::setXBorder(0);
    }
    topology::setNodeVariables(topologyNodes,vs);
    topology::TopologyConstraints t(dim, topologyNodes, topologyRoutes,
            layout->clusterHierarchy, vs, cs);
    bool interrupted;
    int loopBreaker=100;
    cola::SparseMap HMap(layout->n);
    layout->computeForces(dim,HMap,g);
    std::valarray<double> oldCoords=coords;
    t.computeForces(g,HMap);
    cola::SparseMatrix H(HMap);
    layout->applyDescentVector(g,oldCoords,coords,oldStress,
            layout->computeStepSize(H,g,g));
    cola::setVariableDesiredPositions(vs,cs,des,coords);
    do {
        interrupted=t.solve();
        unsigned vptr=0;
        for(topology::Nodes::iterator i=topologyNodes.begin();
                i!=topologyNodes.end();++i,++vptr) {
            topology::Node* v=*i;
            coords[v->id]=v->rect->getCentreD(dim);
        }
        for(;vptr<coords.size();vptr++) {
            double d = vs[vptr]->finalPosition;
            coords[vptr]=d;
            layout->boundingBoxes[vptr]->moveCentreD(dim,d);
        }
        loopBreaker--;
    } while(interrupted&&loopBreaker>0);
    vpsc::Rectangle::setXBorder(0);
    vpsc::Rectangle::setYBorder(0);
    return layout->computeStress();
}

} // topology namespace

