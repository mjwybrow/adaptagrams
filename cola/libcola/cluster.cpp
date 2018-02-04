/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2014  Monash University
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
 * Author(s):  Tim Dwyer
 *             Michael Wybrow
*/

#include "libvpsc/assertions.h"
#include "libcola/commondefs.h"
#include "libcola/cola.h"
#include "libcola/convex_hull.h"
#include "libcola/cluster.h"

using vpsc::generateXConstraints;
using vpsc::generateYConstraints;
using namespace std;

namespace cola {

Cluster::Cluster()
    : bounds(),
      clusterVarId(0),
      varWeight(0.0001),
      internalEdgeWeightFactor(1.), 
      desiredBoundsSet(false), 
      desiredBounds()
{
    // XXX We use a really low weight here until we properly set the source 
    //     of the variable value back in updatePositions() type manner.
    varWeight = 0.0000001;
}

Cluster::~Cluster()
{
    for_each(clusters.begin(), clusters.end(), delete_object());
    clusters.clear();
}

void Cluster::setDesiredBounds(const vpsc::Rectangle db) 
{
    desiredBoundsSet = true;
    desiredBounds = db;
}

void Cluster::unsetDesiredBounds(void)
{
    desiredBoundsSet=false;
}


// Checks to see if the shape at the given index is contained within this
// cluster or its child clusters.
//
void Cluster::countContainedNodes(std::vector<unsigned>& counts)
{
    vector<unsigned> invalidNodes;
    for (set<unsigned>::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        unsigned nodeIndex = *it;
        if (nodeIndex < counts.size())
        {
            // Node index is valid, increase count.
            counts[*it] += 1;
        }
        else
        {
            fprintf(stderr, "Warning: Invalid node index %u specified in "
                    "cluster. Ignoring...\n", nodeIndex);
            invalidNodes.push_back(nodeIndex);
        }
    }
    for (size_t i = 0; i < invalidNodes.size(); ++i)
    {
        nodes.erase(invalidNodes[i]);
    }

    for (vector<Cluster*>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->countContainedNodes(counts);
    }
}

void Cluster::computeBoundingRect(const vpsc::Rectangles& rs) 
{
    bounds = vpsc::Rectangle();
    for (vector<Cluster*>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->computeBoundingRect(rs);
        vpsc::Rectangle rectangle = 
                (*i)->margin().rectangleByApplyingBox((*i)->bounds);
        bounds = bounds.unionWith(rectangle);
    }
    for (set<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        vpsc::Rectangle* r=rs[*i];
        bounds = bounds.unionWith(*r);
    }
    bounds = padding().rectangleByApplyingBox(bounds);
}

void Cluster::computeVarRect(vpsc::Variables& vars, size_t dim)
{
    if ((clusterVarId > 0) && (vars.size() > clusterVarId))
    {
        varRect.setMinD(dim, vars[clusterVarId]->finalPosition);
        varRect.setMaxD(dim, vars[clusterVarId + 1]->finalPosition);
    }

    for (vector<Cluster*>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->computeVarRect(vars, dim);
    }
}

bool Cluster::clusterIsFromFixedRectangle(void) const
{
    return false;
}

void ConvexCluster::computeBoundary(const vpsc::Rectangles& rs) 
{
    unsigned n = 4 * nodes.size();
    valarray<double> X(n);
    valarray<double> Y(n);
    unsigned pctr = 0;
    vector<unsigned> nodesVector(nodes.begin(), nodes.end());
    for (size_t i = 0; i < nodesVector.size(); ++i)
    {
        vpsc::Rectangle* r=rs[nodesVector[i]];
        // Bottom Right
        X[pctr]=r->getMaxX();
        Y[pctr++]=r->getMinY();
        // Top Right
        X[pctr]=r->getMaxX();
        Y[pctr++]=r->getMaxY();
        // Top Left
        X[pctr]=r->getMinX();
        Y[pctr++]=r->getMaxY();
        // Bottom Left
        X[pctr]=r->getMinX();
        Y[pctr++]=r->getMinY();
    }
    /*
    for(unsigned i=0;i<n;i++) {
        printf("X[%d]=%f, Y[%d]=%f;\n",i,X[i],i,Y[i]);
    }
    */
    vector<unsigned> hull;
    hull::convex(X,Y,hull);
    hullX.resize(hull.size());
    hullY.resize(hull.size());
    hullRIDs.resize(hull.size());
    hullCorners.resize(hull.size());
    for (unsigned j=0;j<hull.size();j++) 
    {
        hullX[j]=X[hull[j]];
        hullY[j]=Y[hull[j]];
        hullRIDs[j]=nodesVector[hull[j]/4];
        hullCorners[j]=hull[j]%4;
    }
}


void ConvexCluster::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    ConvexCluster *cluster%llu = new ConvexCluster();\n",
            (unsigned long long) this);
    for(set<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        fprintf(fp, "    cluster%llu->addChildNode(%u);\n",
                (unsigned long long) this, *i);
    }
    for(vector<Cluster *>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->printCreationCode(fp);
        fprintf(fp, "    cluster%llu->addChildCluster(cluster%llu);\n",
                (unsigned long long) this, (unsigned long long) *i);
    }
}

void ConvexCluster::outputToSVG(FILE *fp) const
{
    for(vector<Cluster *>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->outputToSVG(fp);
    }
}


RectangularCluster::RectangularCluster()
    : Cluster(),
      m_rectangle_index(-1),
      m_margin(0),
      m_padding(0)
{
    minEdgeRect[vpsc::XDIM] = nullptr;
    minEdgeRect[vpsc::YDIM] = nullptr;
    maxEdgeRect[vpsc::XDIM] = nullptr;
    maxEdgeRect[vpsc::YDIM] = nullptr;
}

RectangularCluster::RectangularCluster(unsigned rectIndex)
    : Cluster(),
      m_rectangle_index(rectIndex),
      m_margin(0),
      m_padding(0)
{
    minEdgeRect[vpsc::XDIM] = nullptr;
    minEdgeRect[vpsc::YDIM] = nullptr;
    maxEdgeRect[vpsc::XDIM] = nullptr;
    maxEdgeRect[vpsc::YDIM] = nullptr;
}

RectangularCluster::~RectangularCluster()
{
    for (size_t dim = 0; dim < 2; ++dim)
    {
        if (minEdgeRect[dim])
        {
            delete minEdgeRect[dim];
            minEdgeRect[dim] = nullptr;
        }
        if (maxEdgeRect[dim])
        {
            delete maxEdgeRect[dim];
            maxEdgeRect[dim] = nullptr;
        }
    }
}
 
void RectangularCluster::setMargin(const Box margin)
{
    m_margin = margin;
}


void RectangularCluster::setMargin(double margin)
{
    m_margin = Box(margin);
}


Box RectangularCluster::margin(void) const
{
    return m_margin;
}


void RectangularCluster::setPadding(const Box padding)
{
    m_padding = padding;
}


void RectangularCluster::setPadding(double padding)
{
    m_padding = Box(padding);
}


Box RectangularCluster::padding(void) const
{
    return m_padding;
}


void RectangularCluster::countContainedNodes(std::vector<unsigned>& counts)
{
    if (m_rectangle_index >= 0)
    {
        // This cluster is the shape in question.
        counts[m_rectangle_index] += 1;
    }
    Cluster::countContainedNodes(counts);
}


void RectangularCluster::generateFixedRectangleConstraints(
        cola::CompoundConstraints& idleConstraints,
        vpsc::Rectangles& rc, vpsc::Variables (&vars)[2]) const
{
    COLA_UNUSED(vars);

    if (m_rectangle_index < 0)
    {
        // Not based on a Rectangle.
        return;
    }

    double halfWidth = rc[m_rectangle_index]->width() / 2;
    double halfHeight = rc[m_rectangle_index]->height() / 2;

    cola::SeparationConstraint *sc = 
            new cola::SeparationConstraint(vpsc::XDIM, clusterVarId, 
                    m_rectangle_index, halfWidth, true);
    idleConstraints.push_back(sc);
    sc = new cola::SeparationConstraint(vpsc::XDIM, 
                    m_rectangle_index, clusterVarId + 1, halfWidth, true);
    idleConstraints.push_back(sc);
    
    sc = new cola::SeparationConstraint(vpsc::YDIM, 
                    clusterVarId, m_rectangle_index, halfHeight, true);
    idleConstraints.push_back(sc);
    sc = new cola::SeparationConstraint(vpsc::YDIM, 
                    m_rectangle_index, clusterVarId + 1, halfHeight, true);
    idleConstraints.push_back(sc);
}


void RectangularCluster::computeBoundary(const vpsc::Rectangles& rs)
{
    double xMin=DBL_MAX, xMax=-DBL_MAX, yMin=DBL_MAX, yMax=-DBL_MAX;
    for (std::set<unsigned>::iterator it = nodes.begin();
            it != nodes.end(); ++it)
    {
        xMin=std::min(xMin,rs[*it]->getMinX());
        xMax=std::max(xMax,rs[*it]->getMaxX());
        yMin=std::min(yMin,rs[*it]->getMinY());
        yMax=std::max(yMax,rs[*it]->getMaxY());
    }
    hullX.resize(4);
    hullY.resize(4);
    hullX[3]=xMin;
    hullY[3]=yMin;
    hullX[2]=xMin;
    hullY[2]=yMax;
    hullX[1]=xMax;
    hullY[1]=yMax;
    hullX[0]=xMax;
    hullY[0]=yMin;
}


void RectangularCluster::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    RectangularCluster *cluster%llu = "
            "new RectangularCluster(",
            (unsigned long long) this);
    if (m_rectangle_index != -1)
    {
        fprintf(fp, "%d", m_rectangle_index);
    }
    fprintf(fp, ");\n");
    if (!m_margin.empty())
    {
        fprintf(fp, "    cluster%llu->setMargin(",
                (unsigned long long) this);
        m_margin.outputCode(fp);
        fprintf(fp, ");\n");
    }
    if (!m_padding.empty())
    {
        fprintf(fp, "    cluster%llu->setPadding(",
                (unsigned long long) this);
        m_padding.outputCode(fp);
        fprintf(fp, ");\n");
    }
    for(set<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        fprintf(fp, "    cluster%llu->addChildNode(%u);\n",
                (unsigned long long) this, *i);
    }
    for(vector<Cluster *>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->printCreationCode(fp);
        fprintf(fp, "    cluster%llu->addChildCluster(cluster%llu);\n",
                (unsigned long long) this, (unsigned long long) *i);
    }
}

void RectangularCluster::outputToSVG(FILE *fp) const
{
    double rounding = 4;
    if (varRect.isValid())
    {
        fprintf(fp, "<rect id=\"cluster-%llu-r\" x=\"%g\" y=\"%g\" width=\"%g\" "
                "height=\"%g\" style=\"stroke-width: 1px; stroke: black; "
                "fill: green; fill-opacity: 0.3;\" rx=\"%g\" ry=\"%g\" />\n",
                (unsigned long long) this, varRect.getMinX(), varRect.getMinY(), 
                varRect.getMaxX() - varRect.getMinX(), 
                varRect.getMaxY() - varRect.getMinY(), rounding, rounding);
    }
    else
    {
        fprintf(fp, "<rect id=\"cluster-%llu\" x=\"%g\" y=\"%g\" width=\"%g\" "
            "height=\"%g\" style=\"stroke-width: 1px; stroke: black; "
            "fill: red; fill-opacity: 0.3;\" rx=\"%g\" ry=\"%g\" />\n",
            (unsigned long long) this, bounds.getMinX(), bounds.getMinY(), 
            bounds.getMaxX() - bounds.getMinX(), 
            bounds.getMaxY() - bounds.getMinY(), rounding, rounding);
    }

    for(vector<Cluster *>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->outputToSVG(fp);
    }
}



int RectangularCluster::rectangleIndex(void) const
{
    return m_rectangle_index;
}

bool RectangularCluster::clusterIsFromFixedRectangle(void) const
{
    return (m_rectangle_index >= 0);
}

void RectangularCluster::computeBoundingRect(const vpsc::Rectangles& rs) 
{
    if (clusterIsFromFixedRectangle())
    {
        // For bounds, just use this shape's rectangle.
        bounds = *(rs[m_rectangle_index]);
    }
    else
    {
        Cluster::computeBoundingRect(rs);
    }
}
void RectangularCluster::addChildNode(unsigned index)
{
    if ((m_rectangle_index == (int) index) && (m_rectangle_index > 0))
    {
        fprintf(stderr, "Warning: ignoring cluster (%u) added as child of "
                "itself.\n", index);
        return;
    }
    Cluster::addChildNode(index);
}


RootCluster::RootCluster()
    : m_allows_multiple_parents(false)
{
}

void Cluster::recPathToCluster(RootCluster *rootCluster, Clusters currentPath)
{
    // Reset cluster-cluster overlap exceptions.
    m_cluster_cluster_overlap_exceptions.clear();
    m_nodes_replaced_with_clusters.clear();
    m_overlap_replacement_map.clear();

    // Add this cluster to the path.
    currentPath.push_back(this);

    // Recusively all on each child cluster.
    for (unsigned i = 0; i < clusters.size(); ++i) 
    {
        clusters[i]->recPathToCluster(rootCluster, currentPath);
    }

    // And store the path to each child node.
    for (std::set<unsigned>::iterator it = nodes.begin();
            it != nodes.end(); ++it)
    {
        rootCluster->m_cluster_vectors_leading_to_nodes[*it].
                push_back(currentPath);
    }
}


void RootCluster::calculateClusterPathsToEachNode(size_t nodesCount)
{
    m_cluster_vectors_leading_to_nodes.clear();
    m_cluster_vectors_leading_to_nodes.resize(nodesCount);

    recPathToCluster(this, Clusters());

    for (unsigned i = 0; i < m_cluster_vectors_leading_to_nodes.size(); ++i) 
    {
        size_t paths = m_cluster_vectors_leading_to_nodes[i].size();
        for (size_t j = 1; j < paths; ++j)
        {
            for (size_t k = 0; k < j; ++k)
            {
                // For each pair of paths.

                // Find the lowest common ancestor by finding where the two
                // paths from the root cluster to node i diverge.
                Clusters pathJ = m_cluster_vectors_leading_to_nodes[i][j];
                Clusters pathK = m_cluster_vectors_leading_to_nodes[i][k];
                size_t lcaIndex = 0;
                while ((lcaIndex < pathJ.size()) && 
                       (lcaIndex < pathK.size()) &&
                        (pathJ[lcaIndex] == pathK[lcaIndex]))
                {
                    ++lcaIndex;
                }
                COLA_ASSERT(lcaIndex > 0);

                // lcaIndex will be the clusters/nodes that need to overlap
                // due to these two paths to node i.
                size_t lcaChildJIndex = i;
                size_t lcaChildKIndex = i;
                Cluster *lcaChildJCluster = nullptr;
                Cluster *lcaChildKCluster = nullptr;
                
                // lcaIndex < path{J,K}.size() means the child J or K of 
                // the lca is a Cluster.   At least one of them will always
                // be a cluster.
                COLA_ASSERT((lcaIndex < pathJ.size()) ||
                        (lcaIndex < pathK.size()));
                if (lcaIndex < pathJ.size())
                {
                    lcaChildJCluster = pathJ[lcaIndex];
                    lcaChildJIndex = lcaChildJCluster->clusterVarId;
                }
                if (lcaIndex < pathK.size())
                {
                    lcaChildKCluster = pathK[lcaIndex];
                    lcaChildKIndex = lcaChildKCluster->clusterVarId;
                }

                // We want to exclude the overlapping children of the lca 
                // from having non-overlap constraints generated for them
                // (siblings of a particular cluster usually have 
                // non-overlap constraints generated for them).
                Cluster *lcaCluster = pathJ[lcaIndex - 1];
                lcaCluster->m_cluster_cluster_overlap_exceptions.insert(
                        ShapePair(lcaChildJIndex, lcaChildKIndex));

                if (lcaChildJCluster)
                {
                    // In cluster J, replace node i with cluster K for the 
                    // purpose of non-overlap with siblings, and remember 
                    // this replacement so we can still generate non-overlap 
                    // constraints between multiple nodes that are children
                    // of the same overlapping clusters.
                    lcaChildJCluster->m_overlap_replacement_map[i] =
                            lcaChildKCluster;
                    lcaChildJCluster->m_nodes_replaced_with_clusters.insert(i);
                }

                if (lcaChildKCluster)
                {
                    // In cluster K, replace node i with cluster J for the 
                    // purpose of non-overlap with siblings, and remember 
                    // this replacement so we can still generate non-overlap 
                    // constraints between multiple nodes that are children
                    // of the same overlapping clusters.
                    lcaChildKCluster->m_overlap_replacement_map[i] =
                            lcaChildJCluster;
                    lcaChildKCluster->m_nodes_replaced_with_clusters.insert(i);
                }
            }
        }
    }
}


void RootCluster::computeBoundary(const vpsc::Rectangles& rs) 
{
    for (unsigned i = 0; i < clusters.size(); ++i) 
    {
        clusters[i]->computeBoundary(rs);
    }
}

void RootCluster::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    RootCluster *cluster%llu = new RootCluster();\n",
            (unsigned long long) this);
    for(set<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        fprintf(fp, "    cluster%llu->addChildNode(%u);\n",
                (unsigned long long) this, *i);
    }
    for(vector<Cluster *>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->printCreationCode(fp);
        fprintf(fp, "    cluster%llu->addChildCluster(cluster%llu);\n",
                (unsigned long long) this, (unsigned long long) *i);
    }
}

void RootCluster::outputToSVG(FILE *fp) const
{
    for(vector<Cluster *>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->outputToSVG(fp);
    }

}


bool RootCluster::allowsMultipleParents(void) const
{
    return m_allows_multiple_parents;
}

void RootCluster::setAllowsMultipleParents(const bool value)
{
    m_allows_multiple_parents = value;
}

void Cluster::updateBounds(const vpsc::Dim dim) 
{
    if (dim == vpsc::HORIZONTAL)
    {
        bounds = vpsc::Rectangle(vMin->finalPosition, vMax->finalPosition, 
                bounds.getMinY(), bounds.getMaxY());
    } 
    else
    {
        bounds = vpsc::Rectangle(bounds.getMinX(), bounds.getMaxX(), 
                vMin->finalPosition, vMax->finalPosition);
    }
    for (unsigned i=0; i < clusters.size(); ++i)
    {
        clusters[i]->updateBounds(dim);
    }
}


void Cluster::createVars(const vpsc::Dim dim, const vpsc::Rectangles& rs, 
        vpsc::Variables& vars) 
{
    for (vector<Cluster*>::iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->createVars(dim, rs, vars);
    }
    if (dim==vpsc::HORIZONTAL)
    {
        double desiredMinX = bounds.getMinX(), desiredMaxX = bounds.getMaxX();
        if (desiredBoundsSet)
        {
            desiredMinX = desiredBounds.getMinX();
            desiredMaxX = desiredBounds.getMaxX();
        }
        clusterVarId = vars.size(); 
        vars.push_back(vXMin = new vpsc::Variable(
                    vars.size(), desiredMinX, varWeight));
        vars.push_back(vXMax = new vpsc::Variable(
                    vars.size(), desiredMaxX, varWeight));
    } 
    else 
    {
        double desiredMinY = bounds.getMinY(), desiredMaxY = bounds.getMaxY();
        if (desiredBoundsSet)
        {
            desiredMinY = desiredBounds.getMinY();
            desiredMaxY = desiredBounds.getMaxY();
        }
        clusterVarId = vars.size(); 
        vars.push_back(vYMin = new vpsc::Variable(
                    vars.size(), desiredMinY, varWeight));
        vars.push_back(vYMax = new vpsc::Variable(
                    vars.size(), desiredMaxY, varWeight));
    }
}

// Returns the total area covered by contents of this cluster (not
// including space between nodes/clusters).
//
double Cluster::area(const vpsc::Rectangles& rs)
{
    double a = 0;
    for (set<unsigned>::iterator i = nodes.begin(); i != nodes.end(); ++i)
    {
        vpsc::Rectangle *r = rs[*i];
        a += r->width() * r->height();
    }
    for (Clusters::iterator i = clusters.begin(); i!= clusters.end(); ++i)
    {
        a += (*i)->area(rs);
    }
    return a;
}

void Cluster::addChildNode(unsigned index)
{
    this->nodes.insert(index);
}

void Cluster::addChildCluster(Cluster *cluster)
{
    if (cluster == this)
    {
        fprintf(stderr, "Warning: ignoring cluster added as child of itself.\n");
        return;
    }
    this->clusters.push_back(cluster);
}


} // namespace cola
