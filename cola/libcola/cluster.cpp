/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2010  Monash University
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
 * Author(s):  Tim Dwyer
 *             Michael Wybrow
*/

#include "libvpsc/assertions.h"
#include "commondefs.h"
#include "cola.h"
#include "convex_hull.h"
#include "cluster.h"

using vpsc::generateXConstraints;
using vpsc::generateYConstraints;
using namespace std;

namespace cola {

Cluster::Cluster()
    : varWeight(0.0001), 
      internalEdgeWeightFactor(1.), 
      bounds(),
      clusterVarId(0),
      rectBuffer(0),
      desiredBoundsSet(false), 
      desiredBounds(-1,1,-1,1)
{ }

void Cluster::setDesiredBounds(const vpsc::Rectangle db) 
{
    desiredBoundsSet = true;
    desiredBounds = db;
}

void Cluster::unsetDesiredBounds(void)
{
    desiredBoundsSet=false;
}

void Cluster::setRectBuffers(const double buffer)
{
    for (vector<Cluster*>::iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->setRectBuffers(buffer);
    }
    this->rectBuffer = buffer;
}


// Checks to see if the shape at the given index is contained within this
// cluster or its child clusters.
//
bool Cluster::containsShape(unsigned index) const
{
    for (vector<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        if (*i == index)
        {
            return true;
        }
    }
    for (vector<Cluster*>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        if ((*i)->containsShape(index))
        {
            return true;
        }
    }
    return false;
}

void Cluster::computeBoundingRect(const vpsc::Rectangles& rs) 
{
    bounds = vpsc::Rectangle();
    for (vector<Cluster*>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->computeBoundingRect(rs);
        bounds = bounds.unionWith((*i)->bounds);
    }
    for (vector<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        vpsc::Rectangle* r=rs[*i];
        bounds = bounds.unionWith(*r);
    }
}


void ConvexCluster::computeBoundary(const vpsc::Rectangles& rs) 
{
    unsigned n = 4 * nodes.size();
    valarray<double> X(n);
    valarray<double> Y(n);
    unsigned pctr = 0;
    for (vector<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        vpsc::Rectangle* r=rs[*i];
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
        hullRIDs[j]=hull[j]/4;
        hullCorners[j]=hull[j]%4;
    }
}


void ConvexCluster::printCreationCode(FILE *fp) const
{
    fprintf(fp, "    ConvexCluster *cluster%llu = new ConvexCluster();\n",
            (unsigned long long) this);
    for(vector<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        fprintf(fp, "    cluster%llu->nodes.push_back(%u);\n",
                (unsigned long long) this, *i);
    }
    for(vector<Cluster *>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->printCreationCode(fp);
        fprintf(fp, "    cluster%llu->clusters.push_back(cluster%llu);\n",
                (unsigned long long) this, (unsigned long long) *i);
    }
}


RectangularCluster::RectangularCluster()
    : Cluster()
{
    minEdgeRect[vpsc::XDIM] = NULL;
    minEdgeRect[vpsc::YDIM] = NULL;
    maxEdgeRect[vpsc::XDIM] = NULL;
    maxEdgeRect[vpsc::YDIM] = NULL;
}

RectangularCluster::~RectangularCluster()
{
    for (size_t dim = 0; dim < 2; ++dim)
    {
        if (minEdgeRect[dim])
        {
            delete minEdgeRect[dim];
            minEdgeRect[dim] = NULL;
        }
        if (maxEdgeRect[dim])
        {
            delete maxEdgeRect[dim];
            maxEdgeRect[dim] = NULL;
        }
    }
}
 
vpsc::Rectangle *RectangularCluster::getMinEdgeRect(const vpsc::Dim dim)
{
    if (minEdgeRect[dim])
    {
        delete minEdgeRect[dim];
    }
    minEdgeRect[dim] = new vpsc::Rectangle(bounds);
    
    // Set the Min and Max positions to be the min minus an offset.
    double edgePosition = minEdgeRect[dim]->getMinD(dim);
    minEdgeRect[dim]->setMinD(dim, edgePosition - rectBuffer);
    minEdgeRect[dim]->setMaxD(dim, edgePosition);
    
    return minEdgeRect[dim];
}

vpsc::Rectangle *RectangularCluster::getMaxEdgeRect(const vpsc::Dim dim)
{
    if (maxEdgeRect[dim])
    {
        delete maxEdgeRect[dim];
    }
    maxEdgeRect[dim] = new vpsc::Rectangle(bounds);

    // Set the Min and Max positions to be the max plus an offset.
    double edgePosition = maxEdgeRect[dim]->getMaxD(dim);
    maxEdgeRect[dim]->setMaxD(dim, edgePosition + rectBuffer);
    maxEdgeRect[dim]->setMinD(dim, edgePosition);

    return maxEdgeRect[dim];
}


void RectangularCluster::computeBoundary(const vpsc::Rectangles& rs)
{
    double xMin=DBL_MAX, xMax=-DBL_MAX, yMin=DBL_MAX, yMax=-DBL_MAX;
    for (unsigned i=0;i<nodes.size();i++)
    {
        xMin=std::min(xMin,rs[nodes[i]]->getMinX());
        xMax=std::max(xMax,rs[nodes[i]]->getMaxX());
        yMin=std::min(yMin,rs[nodes[i]]->getMinY());
        yMax=std::max(yMax,rs[nodes[i]]->getMaxY());
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
            "new RectangularCluster();\n",
            (unsigned long long) this);
    for(vector<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        fprintf(fp, "    cluster%llu->nodes.push_back(%u);\n",
                (unsigned long long) this, *i);
    }
    for(vector<Cluster *>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->printCreationCode(fp);
        fprintf(fp, "    cluster%llu->clusters.push_back(cluster%llu);\n",
                (unsigned long long) this, (unsigned long long) *i);
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
    for(vector<unsigned>::const_iterator i = nodes.begin(); 
            i != nodes.end(); ++i)
    {
        fprintf(fp, "    cluster%llu->nodes.push_back(%u);\n",
                (unsigned long long) this, *i);
    }
    for(vector<Cluster *>::const_iterator i = clusters.begin(); 
            i != clusters.end(); ++i)
    {
        (*i)->printCreationCode(fp);
        fprintf(fp, "    cluster%llu->clusters.push_back(cluster%llu);\n",
                (unsigned long long) this, (unsigned long long) *i);
    }
    fprintf(fp, "    alg.setClusterHierarchy(cluster%llu);\n",
            (unsigned long long) this);
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

/** recursively delete all clusters */
void Cluster::clear(void) 
{
    for_each(clusters.begin(), clusters.end(), delete_object());
    clusters.clear();
}

/**
 * @return the total area covered by contents of this cluster (not
 * including space between nodes/clusters)
 */
double Cluster::area(const vpsc::Rectangles& rs)
{
    double a = 0;
    for (vector<unsigned>::iterator i = nodes.begin(); i != nodes.end(); ++i)
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

} // namespace cola
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
