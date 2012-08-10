/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2009-2011  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the 
 * library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * Author(s):   Michael Wybrow <mjwybrow@users.sourceforge.net>
*/



#include <cstdlib>
#include <cfloat>
#include <cmath>
#include <set>
#include <list>
#include <algorithm>

#include "libavoid/router.h"
#include "libavoid/geomtypes.h"
#include "libavoid/shape.h"
#include "libavoid/orthogonal.h"
#include "libavoid/connend.h"
#include "libavoid/connector.h"
#include "libavoid/junction.h"
#include "libavoid/vpsc.h"
#include "libavoid/assertions.h"
#include "libavoid/hyperedgetree.h"
#include "libavoid/mtst.h"
#include "libavoid/scanline.h"

//#define NUDGE_DEBUG

namespace Avoid {


static const double CHANNEL_MAX = 100000000;

// IDs:
static const int freeID    = 0;
static const int fixedID   = 1;
// Weights:
static const double freeWeight   = 0.00001;
static const double strongWeight = 0.001;
static const double strongerWeight = 1.0;
static const double fixedWeight  = 100000;


class HyperEdgeShiftSegment : public ShiftSegment
{
    public:
        HyperEdgeShiftSegment(HyperEdgeTreeNode *n1, HyperEdgeTreeNode *n2, 
                const size_t dim, bool immovable)
            : ShiftSegment(dim),
              nodes((dim + 1) % 2),
              isImmovable(immovable),
              m_balance_count(0),
              m_balance_count_set(false),
              m_at_limit(false)
        {
            nodes.insert(n1);
            nodes.insert(n2);
            n1->shiftSegmentNodeSet = &nodes;
            n2->shiftSegmentNodeSet = &nodes;

            minSpaceLimit = -CHANNEL_MAX;
            maxSpaceLimit = CHANNEL_MAX;
        }
        virtual ~HyperEdgeShiftSegment()
        {
            for (OrderedHENodeSet::const_iterator curr = nodes.begin();
                    curr != nodes.end(); ++curr)
            {
                (*curr)->shiftSegmentNodeSet = NULL;
            }
        }

        Point& lowPoint(void)
        {
            return (*nodes.begin())->point;
        }
        Point& highPoint(void) 
        {
            return (*nodes.rbegin())->point;
        }
        const Point& lowPoint(void) const
        {
            return (*nodes.begin())->point;
        }
        const Point& highPoint(void) const
        {
            return (*nodes.rbegin())->point;
        }
        // Counts the number of segments diverging on each side and returns
        // a count: a negative number if there a more on the lower side,
        // a positive number if there are more on the upper side, or zero if
        // there are an equal number of segments.
        void setBalanceCount(void)
        {
            size_t altDim = (dimension + 1) % 2;
            m_next_pos_lower = minSpaceLimit;
            m_next_pos_upper = maxSpaceLimit;
            m_balance_count = 0;
            if ( isImmovable )
            {
                m_balance_count_set = true;
                return;
            }
            for (OrderedHENodeSet::const_iterator curr = nodes.begin();
                    curr != nodes.end(); ++curr)
            {
                const Point& currPoint = (*curr)->point;
                for (std::list<HyperEdgeTreeEdge *>::const_iterator currEdge =
                        (*curr)->edges.begin(); currEdge != (*curr)->edges.end();
                        ++currEdge)
                {
                    const HyperEdgeTreeNode *node = (*currEdge)->followFrom(*curr);
                    const Point& otherPoint = node->point;
                    if (currPoint[altDim] == otherPoint[altDim])
                    {
                        if (otherPoint[dimension] < currPoint[dimension])
                        {
                            m_next_pos_lower = std::max(m_next_pos_lower,
                                    otherPoint[dimension]);
                            --m_balance_count;
                        }
                        else if (otherPoint[dimension] > currPoint[dimension])
                        {
                            m_next_pos_upper = std::min(m_next_pos_upper,
                                    otherPoint[dimension]);
                            ++m_balance_count;
                        }
                    }
                }
            }
            m_balance_count_set = true;
        }
        int balanceCount(void) const
        {
            COLA_ASSERT( m_balance_count_set );
            return m_balance_count;
        }
        void adjustPosition(void)
        {
            COLA_ASSERT(m_balance_count_set);
            COLA_ASSERT(m_balance_count != 0);

            double newPos = (m_balance_count < 0) ?
                    m_next_pos_lower : m_next_pos_upper;
            double limit = (m_balance_count < 0) ?
                    minSpaceLimit : maxSpaceLimit;
            if (lowPoint()[dimension] == newPos)
            {
                // If aren't actually moving this segment, then mark it 
                // as at-limit.
                // XXX This seems to be because of a segment with an 
                //     incorrectly marked limit, possibly due to a 
                //     junction positioned within a shape.
                m_at_limit = true;
            }

            for (OrderedHENodeSet::iterator curr = nodes.begin();
                    curr != nodes.end(); ++curr)
            {
                (*curr)->point[dimension] = newPos;
            }

            if (newPos == limit)
            {
                m_at_limit = true;
            }

            // Add nodes from collapsed segments, in case they are not part of
            // a segment that will be merged.
            for (OrderedHENodeSet::iterator curr = nodes.begin();
                    curr != nodes.end(); ++curr)
            {
                Point& currPoint = (*curr)->point;
                for (std::list<HyperEdgeTreeEdge *>::iterator currEdge =
                        (*curr)->edges.begin(); currEdge != (*curr)->edges.end();
                        ++currEdge)
                {
                    HyperEdgeTreeNode *node = (*currEdge)->followFrom(*curr);
                    Point& otherPoint = node->point;
                    if (currPoint == otherPoint)
                    {
                        nodes.insert(node);
                        node->shiftSegmentNodeSet = &nodes;
                    }
                }
            }
        }
        bool overlapsWith(const ShiftSegment *rhs, const size_t dim) const
        {
            size_t altDim = (dim + 1) % 2;
            const Point& lowPt = lowPoint();
            const Point& highPt = highPoint();
            const Point& rhsLowPt = rhs->lowPoint();
            const Point& rhsHighPt = rhs->highPoint();
            if ( (lowPt[altDim] <= rhsHighPt[altDim]) &&
                    (rhsLowPt[altDim] <= highPt[altDim]))
            {
                // The segments overlap.
                if ( (minSpaceLimit <= rhs->maxSpaceLimit) &&
                        (rhs->minSpaceLimit <= maxSpaceLimit) )
                {
                    return true;
                }
            }
            return false;
        }
        bool immovable(void) const
        {
            return isImmovable;
        }
        bool settled(void) const
        {
            return isImmovable || m_at_limit || (balanceCount() == 0);
        }
        bool mergesWith(HyperEdgeShiftSegment *other)
        {
            size_t altDim = (dimension + 1) % 2;
            const Point& lowPt = lowPoint();
            const Point& highPt = highPoint();
            const Point& otherLowPt = other->lowPoint();
            const Point& otherHighPt = other->highPoint();
            if ( (lowPt[dimension] == otherLowPt[dimension]) &&
                    (lowPt[altDim] <= otherHighPt[altDim]) &&
                    (otherLowPt[altDim] <= highPt[altDim]))
            {
                isImmovable |= other->isImmovable;
                m_at_limit |= m_at_limit;
                minSpaceLimit = std::max(minSpaceLimit, other->minSpaceLimit);
                maxSpaceLimit = std::min(maxSpaceLimit, other->maxSpaceLimit);
                nodes.insert(other->nodes.begin(), other->nodes.end());
                other->nodes.clear();
                for (OrderedHENodeSet::iterator curr = nodes.begin();
                        curr != nodes.end(); ++curr)
                {
                    (*curr)->shiftSegmentNodeSet = &nodes;
                }
                setBalanceCount();
                return true;
            }
            setBalanceCount();
            return false;
        }

        std::set<HyperEdgeTreeNode *, CmpNodesInDim> nodes;
        bool isImmovable;
private:
        int m_balance_count;
        bool m_balance_count_set;
        double m_next_pos_lower;
        double m_next_pos_upper;
        bool m_at_limit;
};

#if 0
// UNUSED
static bool CmpHyperEdgeSegmentDirOrder(const ShiftSegment *lhsSuper,
            const ShiftSegment *rhsSuper)
{
    const HyperEdgeShiftSegment *lhs =
            dynamic_cast<const HyperEdgeShiftSegment *> (lhsSuper);
    const HyperEdgeShiftSegment *rhs =
            dynamic_cast<const HyperEdgeShiftSegment *> (rhsSuper);

    return fabs(lhs->balanceCount()) > fabs(rhs->balanceCount());
}
#endif

// Used to sort points when merging NudgingShiftSegments.
// Sorts the indexes, by point position in one dimension.
class CmpIndexes
{
    public:
        CmpIndexes(ConnRef *conn, size_t dim)
            : connRef(conn),
              dimension(dim)
        {
        }
        bool operator()(size_t lhs, size_t rhs)
        {
            return connRef->displayRoute().ps[lhs][dimension] < 
                    connRef->displayRoute().ps[rhs][dimension];
        }
    private:
        ConnRef *connRef;
        size_t dimension;
};


class NudgingShiftSegment : public ShiftSegment
{
    public:
        // For shiftable segments.
        NudgingShiftSegment(ConnRef *conn, const size_t low, const size_t high, 
                bool isSBend, bool isZBend, const size_t dim, double minLim,
                double maxLim)
            : ShiftSegment(dim),
              connRef(conn),
              variable(NULL),
              fixed(false),
              finalSegment(false),
              endsInShape(false),
              singleConnectedSegment(false),
              containsCheckpoint(false),
              sBend(isSBend),
              zBend(isZBend)
        {
            indexes.push_back(low);
            indexes.push_back(high);
            minSpaceLimit = minLim;
            maxSpaceLimit = maxLim;
        }
        // For fixed segments.
        NudgingShiftSegment(ConnRef *conn, const size_t low, const size_t high, 
                const size_t dim)
            : ShiftSegment(dim),
              connRef(conn),
              variable(NULL),
              fixed(true),
              finalSegment(false),
              endsInShape(false),
              singleConnectedSegment(false),
              containsCheckpoint(false),
              sBend(false),
              zBend(false)
        {
            indexes.push_back(low);
            indexes.push_back(high);
            // This has no space to shift.
            minSpaceLimit = lowPoint()[dim];
            maxSpaceLimit = lowPoint()[dim];
        }
        virtual ~NudgingShiftSegment()
        {
        }
        Point& lowPoint(void)
        {
            return connRef->displayRoute().ps[indexes.front()];
        }
        Point& highPoint(void)
        {
            return connRef->displayRoute().ps[indexes.back()];
        }
        const Point& lowPoint(void) const
        {
            return connRef->displayRoute().ps[indexes.front()];
        }
        const Point& highPoint(void) const 
        {
            return connRef->displayRoute().ps[indexes.back()];
        }
        double nudgeDistance(void) const
        {
            return connRef->router()->routingParameter(idealNudgingDistance);
        }
        bool immovable(void) const
        {
            return ! zigzag();
        }
        void createSolverVariable(const bool justUnifying)
        {
            bool nudgeFinalSegments = connRef->router()->routingOption(
                    nudgeOrthogonalSegmentsConnectedToShapes);
            int varID = freeID;
            double varPos = lowPoint()[dimension];
            double weight = freeWeight;
            if (nudgeFinalSegments && finalSegment)
            {
                weight = strongWeight;
                
                if (singleConnectedSegment && !justUnifying)
                {
                    // This is a single segment connector bridging
                    // two shapes.  So, we want to try to keep it
                    // centred rather than shift it.
                    // Don't do this during Unifying stage, or else 
                    // these connectors could end up at slightly 
                    // different positions and get the wrong ordering
                    // for nudging.
                    weight = strongerWeight;
                }
            }
            else if (containsCheckpoint)
            {
                weight = strongWeight;
            }
            else if (zigzag())
            {
                COLA_ASSERT(minSpaceLimit > -CHANNEL_MAX);
                COLA_ASSERT(maxSpaceLimit < CHANNEL_MAX);
                
                // For zigzag bends, take the middle as ideal.
                varPos = minSpaceLimit + ((maxSpaceLimit - minSpaceLimit) / 2);
            }
            else if (fixed)
            {
                // Fixed segments shouldn't get moved.
                weight = fixedWeight;
                varID = fixedID;
            }
            else if ( ! finalSegment )
            {
                // Set a higher weight for c-bends to stop them sometimes 
                // getting pushed out into channels by more-free connectors
                // to the "inner" side of them.
                weight = strongWeight;
            }

            variable = new Variable(varID, varPos, weight);
        }

        void updatePositionsFromSolver(void)
        {
            if (fixed)
            {
                return;
            }
            double newPos = variable->finalPosition;

            // The solver can sometimes cause variables to be outside their
            // limits by a tiny amount, since all variables are held by
            // weights.  Thus, just make sure they stay in their limits.
            newPos = std::max(newPos, minSpaceLimit);
            newPos = std::min(newPos, maxSpaceLimit);

#ifdef NUDGE_DEBUG
            printf("Pos: %lX, %g\n", (long) connRef, newPos);
#endif
            for (size_t it = 0; it < indexes.size(); ++it)
            {
                size_t index = indexes[it];
                connRef->displayRoute().ps[index][dimension] = newPos;
            }
        }
        int fixedOrder(bool& isFixed) const
        {
            double nudgeDist = nudgeDistance();
            double pos = lowPoint()[dimension];
            bool minLimited = ((pos - minSpaceLimit) < nudgeDist);
            bool maxLimited = ((maxSpaceLimit - pos) < nudgeDist);
            
            if (fixed || (minLimited && maxLimited))
            {
                isFixed = true;
                return 0;
            }
            else if (minLimited)
            {
                return 1;
            }
            else if (maxLimited)
            {
                return -1;
            }
            return 0;
        }
        int order(void) const
        {
            if (lowC())
            {
                return -1;
            }
            else if (highC())
            {
                return 1;
            }
            return 0;
        }
        bool zigzag(void) const
        {
            return sBend || zBend;
        }
        // This counts segments that are collinear and share an endpoint as
        // overlapping.  This allows them to be nudged apart where possible.
        bool overlapsWith(const ShiftSegment *rhsSuper, const size_t dim) const
        {
            const NudgingShiftSegment *rhs = 
                    dynamic_cast<const NudgingShiftSegment *> (rhsSuper);
            size_t altDim = (dim + 1) % 2;
            const Point& lowPt = lowPoint();
            const Point& highPt = highPoint();
            const Point& rhsLowPt = rhs->lowPoint();
            const Point& rhsHighPt = rhs->highPoint();
            if ( (lowPt[altDim] < rhsHighPt[altDim]) &&
                    (rhsLowPt[altDim] < highPt[altDim]))
            {
                // The segments overlap.
                if ( (minSpaceLimit <= rhs->maxSpaceLimit) &&
                        (rhs->minSpaceLimit <= maxSpaceLimit) )
                {
                    return true;
                }
            }
            else if ( (lowPt[altDim] == rhsHighPt[altDim]) || 
                      (rhsLowPt[altDim] == highPt[altDim]) )
            {
                bool nudgeColinearSegments = connRef->router()->routingOption(
                        nudgeOrthogonalTouchingColinearSegments);

                if ( (minSpaceLimit <= rhs->maxSpaceLimit) &&
                        (rhs->minSpaceLimit <= maxSpaceLimit) )
                {
                    // The segments could touch at one end, so count them as 
                    // overlapping for nudging if they are both s-bends 
                    // or both z-bends, i.e., when the ordering would 
                    // matter.
                    if ((rhs->sBend && sBend) || (rhs->zBend && zBend))
                    {
                        return nudgeColinearSegments;
                    }
                    else if ((rhs->finalSegment && finalSegment) &&
                            (rhs->connRef == connRef))
                    {
                        return nudgeColinearSegments;
                    }
                    else if (connRef->router()->routingParameter(
                            fixedSharedPathPenalty) > 0)
                    {
                        // Or if we are routing with a fixedSharedPathPenalty
                        // then we don't want these segments to slide past
                        // each other.
                        return true;
                    }
                }
            }
            return false;
        }
        bool shouldAlignWith(const ShiftSegment *rhsSuper, const size_t dim) const
        {
            const NudgingShiftSegment *rhs = 
                    dynamic_cast<const NudgingShiftSegment *> (rhsSuper);
            if ((connRef == rhs->connRef) && (finalSegment == rhs->finalSegment) &&
                overlapsWith(rhs, dim))
            {
                // If both the segments are in shapes then we know limits
                // and can align.  Otherwise we do this just for segments 
                // that are very close together, since these will often 
                // prevent nudging, or force it to have a tiny separation
                // value.
                if ((endsInShape && rhs->endsInShape) ||
                        (fabs(lowPoint()[dim] - rhs->lowPoint()[dim]) < 10))
                {
                    return true;
                }
            }
            return false;
        }
        // Used for merging segments with end segments that should appear as
        // a single segment.
        void mergeWith(const ShiftSegment *rhsSuper, const size_t dim)
        {
            // Adjust limits.
            minSpaceLimit = std::max(minSpaceLimit, rhsSuper->minSpaceLimit);
            maxSpaceLimit = std::min(maxSpaceLimit, rhsSuper->maxSpaceLimit);
 
            // Find a new position for the segment, taking into account
            // the two original positions and the combined limits.
            double segmentPos = lowPoint()[dimension];
            double segment2Pos = rhsSuper->lowPoint()[dimension];
            if (segment2Pos < segmentPos)
            {
                segmentPos -= ((segmentPos - segment2Pos) / 2.0);
            }
            else if (segment2Pos > segmentPos)
            {
                segmentPos += ((segment2Pos - segmentPos) / 2.0);
            }
            segmentPos = std::max(minSpaceLimit, segmentPos);
            segmentPos = std::min(maxSpaceLimit, segmentPos);

            // Merge the index lists and sort the new list.
            const NudgingShiftSegment *rhs = 
                    dynamic_cast<const NudgingShiftSegment *> (rhsSuper);
            indexes.insert(indexes.end(), rhs->indexes.begin(), rhs->indexes.end());
            size_t altDim = (dim + 1) % 2;
            CmpIndexes compare(connRef, altDim);
            sort(indexes.begin(), indexes.end(), compare);

            // Apply the new positon to all points to keep them constant.
            for (size_t it = 0; it < indexes.size(); ++it)
            {
                size_t index = indexes[it];
                connRef->displayRoute().ps[index][dimension] = segmentPos;
            }
        }

        ConnRef *connRef;
        Variable *variable;
        std::vector<size_t> indexes;
        bool fixed;
        bool finalSegment;
        bool endsInShape;
        bool singleConnectedSegment;
        bool containsCheckpoint;
    private:
        bool sBend;
        bool zBend;
        bool lowC(void) const
        {
            // This is true if this is a cBend and its adjoining points
            // are at lower positions.
            if (!finalSegment && !zigzag() && !fixed && 
                    (minSpaceLimit == lowPoint()[dimension]))
            {
                return true;
            }
            return false;
        }
        bool highC(void) const
        {
            // This is true if this is a cBend and its adjoining points
            // are at higher positions.
            if (!finalSegment && !zigzag() && !fixed && 
                    (maxSpaceLimit == lowPoint()[dimension]))
            {
                return true;
            }
            return false;
        }
};
typedef std::list<ShiftSegment *> ShiftSegmentList;



enum ScanVisDirFlag {
    VisDirNone  = 0,
    VisDirUp    = 1,
    VisDirDown  = 2
};
typedef unsigned int ScanVisDirFlags;


// Returns a bitfield of the directions of visibility in terms of the scanline
// in a particular dimension dimension.  It will return either ConnDirDown 
// (meaning visibility to lower position values) or ConnDirUp (for visibility 
// towards higher position values).
//
static ScanVisDirFlags getPosVertInfDirections(VertInf *v, size_t dim)
{
    if (dim == XDIM) // X-dimension
    {
        unsigned int dirs = v->visDirections & (ConnDirLeft | ConnDirRight);
        if (dirs == (ConnDirLeft | ConnDirRight))
        {
            return (VisDirDown | VisDirUp);
        }
        else if (dirs == ConnDirLeft)
        {
            return VisDirDown;
        }
        else if (dirs == ConnDirRight)
        {
            return VisDirUp;
        }
    }
    else if (dim == YDIM) // Y-dimension
    {
        unsigned int dirs = v->visDirections & (ConnDirDown | ConnDirUp);
        if (dirs == (ConnDirDown | ConnDirUp))
        {
            return (VisDirDown | VisDirUp);
        }
        else if (dirs == ConnDirDown)
        {
            // libavoid's Y-axis points downwards, so where the user has 
            // specified visibility downwards, this results in visibility to
            // higher scanline positition values. 
            return VisDirUp;
        }
        else if (dirs == ConnDirUp)
        {
            // libavoid's Y-axis points downwards, so where the user has 
            // specified visibility upwards, this results in visibility to
            // lower scanline positition values. 
            return VisDirDown;
        }
    }

    // Can occur for ConnDirNone visibility.
    return VisDirNone;
}


struct PosVertInf
{
    PosVertInf(double p, VertInf *vI, ScanVisDirFlags d = VisDirNone)
        : pos(p),
          vert(vI),
          dirs(d)
    {
    }
    
    bool operator<(const PosVertInf& rhs) const 
    {
        if (pos != rhs.pos)
        {
            return pos < rhs.pos;
        }
        if ((vert->id == rhs.vert->id) && (vert->id == dummyOrthogID))
        {
            // Multiple dummy nodes can get placed at the same point for 
            // multiple ShapeConnectionPins on junctions (outside of shapes).
            // We only need one at each position, so multiples can be seen
            // as equal here.
            return false;
        }
        if (vert->id != rhs.vert->id)
        {
            return vert->id < rhs.vert->id;
        }
        return dirs < rhs.dirs;
    }

    double pos;
    VertInf *vert;

    // A bitfield marking the direction of visibility (in this dimension)
    // made up of VisDirDown (for visibility towards lower position values) 
    // and VisDirUp (for visibility towards higher position values).
    //
    ScanVisDirFlags dirs;
};


struct CmpVertInf
{ 
    bool operator()(const VertInf* u, const VertInf* v) const
    {
        // Comparator for VertSet, an ordered set of VertInf pointers.
        // It is assumed vertical sets of points will all have the same
        // x position and horizontal sets all share a y position, so this
        // method can be used to sort both these sets.
        COLA_ASSERT((u->point.x == v->point.x) || (u->point.y == v->point.y));
        if (u->point.x != v->point.x)
        {
            return u->point.x < v->point.x;
        }
        else if (u->point.y != v->point.y)
        {
            return u->point.y < v->point.y;
        }
        return u < v;
    }
};


typedef std::set<VertInf *, CmpVertInf> VertSet;

// A set of points to break the line segment, 
// along with vertices for these points.
typedef std::set<PosVertInf> BreakpointSet;

// Temporary structure used to store the possible horizontal visibility 
// lines arising from the vertical sweep.
class LineSegment 
{
public:
    LineSegment(const double& b, const double& f, const double& p, 
            bool ss = false, VertInf *bvi = NULL, VertInf *fvi = NULL)
        : begin(b),
          finish(f),
          pos(p),
          shapeSide(ss)
    {
        COLA_ASSERT(begin < finish);

        if (bvi)
        {
            vertInfs.insert(bvi);
        }
        if (fvi)
        {
            vertInfs.insert(fvi);
        }
    }
    LineSegment(const double& bf, const double& p, VertInf *bfvi = NULL)
        : begin(bf),
          finish(bf),
          pos(p),
          shapeSide(false)
    {
        if (bfvi)
        {
            vertInfs.insert(bfvi);
        }
    }
 
    // Order by begin, pos, finish.
    bool operator<(const LineSegment& rhs) const 
    {
        if (begin != rhs.begin)
        {
            return begin < rhs.begin;
        }
        if (pos != rhs.pos)
        {
            return pos < rhs.pos;
        }
        if (finish != rhs.finish)
        {
            return finish < rhs.finish;
        }
        COLA_ASSERT(shapeSide == rhs.shapeSide);
        return false;
    }

    bool overlaps(const LineSegment& rhs) const
    {
        if ((begin == rhs.begin) && (pos == rhs.pos) &&
                (finish == rhs.finish))
        {
            // Lines are exactly equal.
            return true;
        }
        
        if (pos == rhs.pos)
        {
            if (((begin >= rhs.begin) && (begin <= rhs.finish)) ||
                ((rhs.begin >= begin) && (rhs.begin <= finish)) )
            {
                // They are colinear and overlap by some amount.
                return true;
            }
        }
        return false;
    }

    void mergeVertInfs(const LineSegment& segment)
    {
        begin = std::min(begin, segment.begin);
        finish = std::max(finish, segment.finish);
        vertInfs.insert(segment.vertInfs.begin(), segment.vertInfs.end());
    }
    
    VertInf *beginVertInf(void) const
    {
        if (vertInfs.empty())
        {
            return NULL;
        }
        VertInf *inf = *vertInfs.begin();
        if ( ((inf->point.y == begin) && (inf->point.x == pos)) ||
             ((inf->point.x == begin) && (inf->point.y == pos)) )
        {
            // Only return the point if it is actually at the begin pos.
            return inf;
        }
        return NULL;
    }
    VertInf *finishVertInf(void) const
    {
        if (vertInfs.empty())
        {
            return NULL;
        }
        VertInf *inf = *vertInfs.rbegin();
        if ( ((inf->point.y == finish) && (inf->point.x == pos)) ||
             ((inf->point.x == finish) && (inf->point.y == pos)) )
        {
            // Only return the point if it is actually at the finish pos.
            return inf;
        }
        return NULL;
    }

    VertInf *commitPositionX(Router *router, double posX)
    {
        VertInf *found = NULL;
        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == posX)
            {
                found = *v;
                break;
            }
        }
        if (!found)
        {
            found = new VertInf(router, dummyOrthogID, Point(posX, pos));
            vertInfs.insert(found);
        }
        return found;
    }
    // Set begin endpoint vertex if none has been assigned.
    void horiCommitBegin(Router *router, VertInf *vert = NULL)
    {
        if (vert)
        {
            vertInfs.insert(vert);
        }

        if (vertInfs.empty() ||
                ((*vertInfs.begin())->point.x != begin))
        {
            if (begin != -DBL_MAX)
            {
                vertInfs.insert(new
                        VertInf(router, dummyOrthogID, Point(begin, pos)));
            }
        }
    }

    // Set begin endpoint vertex if none has been assigned.
    void horiCommitFinish(Router *router, VertInf *vert = NULL)
    {
        if (vert)
        {
            vertInfs.insert(vert);
        }

        if (vertInfs.empty() ||
                ((*vertInfs.rbegin())->point.x != finish))
        {
            if (finish != DBL_MAX)
            {
                vertInfs.insert(new
                        VertInf(router, dummyOrthogID, Point(finish, pos)));
            }
        }
    }

    // Converts a section of the points list to a set of breakPoints.  
    // Returns the first of the intersection points occurring at finishPos.
    VertSet::iterator addSegmentsUpTo(double finishPos)
    {
        VertSet::iterator firstIntersectionPt = vertInfs.end();
        for (VertSet::iterator vert = vertInfs.begin(); 
                vert != vertInfs.end(); ++vert)
        {
            if ((*vert)->point.x > finishPos)
            {
                // We're done.
                break;
            }
            
            breakPoints.insert(PosVertInf((*vert)->point.x, (*vert),
                        getPosVertInfDirections(*vert, XDIM)));

            if ((firstIntersectionPt == vertInfs.end()) && 
                    ((*vert)->point.x == finishPos))
            {
                firstIntersectionPt = vert;
            }
        }
        // Returns the first of the intersection points at finishPos.
        return firstIntersectionPt;
    }

    // Add visibility edge(s) for this segment.  There may be multiple if 
    // one of the endpoints is shared by multiple connector endpoints.
    void addEdgeHorizontal(Router *router)
    {
        horiCommitBegin(router);
        horiCommitFinish(router);
        
        addSegmentsUpTo(finish);
    }

    // Set flags to show what can be passed on this visibility line.
    // This can be used later to disregard some edges in the visibility
    // graph when routing particular connectors.
    void setLongRangeVisibilityFlags(size_t dim)
    {
        // First, travel in one direction
        bool seenConnPt = false;
        bool seenShapeEdge = false;
        for (BreakpointSet::iterator nvert = breakPoints.begin(); 
                nvert != breakPoints.end(); ++nvert)
        {
            VertIDProps mask = 0;
            if (dim == XDIM)
            {
                if (seenConnPt)
                {
                    mask |= XL_CONN;
                }
                if (seenShapeEdge)
                {
                    mask |= XL_EDGE;
                }
            }
            else // if (dim == YDIM)
            {
                if (seenConnPt)
                {
                    mask |= YL_CONN;
                }
                if (seenShapeEdge)
                {
                    mask |= YL_EDGE;
                }
            }
            nvert->vert->orthogVisPropFlags |= mask;

            if (nvert->vert->id.isConnPt())
            {
                seenConnPt = true;
            }
            if (nvert->vert->id.isOrthShapeEdge())
            {
                seenShapeEdge = true;
            }
        }
        // Then in the other direction
        seenConnPt = false;
        seenShapeEdge = false;
        for (BreakpointSet::reverse_iterator rvert = breakPoints.rbegin(); 
                rvert != breakPoints.rend(); ++rvert)
        {
            VertIDProps mask = 0;
            if (dim == XDIM)
            {
                if (seenConnPt)
                {
                    mask |= XH_CONN;
                }
                if (seenShapeEdge)
                {
                    mask |= XH_EDGE;
                }
            }
            else // if (dim == YDIM)
            {
                if (seenConnPt)
                {
                    mask |= YH_CONN;
                }
                if (seenShapeEdge)
                {
                    mask |= YH_EDGE;
                }
            }
            rvert->vert->orthogVisPropFlags |= mask;

            if (rvert->vert->id.isConnPt())
            {
                seenConnPt = true;
            }
            if (rvert->vert->id.isOrthShapeEdge())
            {
                seenShapeEdge = true;
            }
        }
    }

    // Add visibility edge(s) for this segment up until an intersection.
    // Then, move the segment beginning to the intersection point, so we
    // later only consider the remainder of the segment.
    // There may be multiple segments added to the graph if the beginning 
    // endpoint of the segment is shared by multiple connector endpoints.
    VertSet addEdgeHorizontalTillIntersection(Router *router, 
            LineSegment& vertLine)
    {
        VertSet intersectionSet;

        horiCommitBegin(router);

        // Does a vertex already exist for this point.
        commitPositionX(router, vertLine.pos);
   
        // Generate segments and set end iterator to the first point 
        // at the intersection position.
        VertSet::iterator restBegin = addSegmentsUpTo(vertLine.pos);

        // Add the intersections points to intersectionSet.
        VertSet::iterator restEnd = restBegin;
        while ((restEnd != vertInfs.end()) && 
                (*restEnd)->point.x == vertLine.pos)
        {
            ++restEnd;
        }
        intersectionSet.insert(restBegin, restEnd);

        // Adjust segment to remove processed portion.
        begin = vertLine.pos;
        vertInfs.erase(vertInfs.begin(), restBegin);

        return intersectionSet;
    }
                
    // Insert vertical breakpoints.
    void insertBreakpointsBegin(Router *router, LineSegment& vertLine)
    {
        VertInf *vert = NULL;
        if (pos == vertLine.begin && vertLine.beginVertInf())
        {
            vert = vertLine.beginVertInf();
        }
        else if (pos == vertLine.finish && vertLine.finishVertInf())
        {
            vert = vertLine.finishVertInf();
        }
        horiCommitBegin(router, vert);

        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == begin)
            {
                vertLine.breakPoints.insert(PosVertInf(pos, *v, 
                        getPosVertInfDirections(*v, YDIM)));
            }
        }
    }

    // Insert vertical breakpoints.
    void insertBreakpointsFinish(Router *router, LineSegment& vertLine)
    {
        VertInf *vert = NULL;
        if (pos == vertLine.begin && vertLine.beginVertInf())
        {
            vert = vertLine.beginVertInf();
        }
        else if (pos == vertLine.finish && vertLine.finishVertInf())
        {
            vert = vertLine.finishVertInf();
        }
        horiCommitFinish(router, vert);

        for (VertSet::iterator v = vertInfs.begin();
                v != vertInfs.end(); ++v)
        {
            if ((*v)->point.x == finish)
            {
                vertLine.breakPoints.insert(PosVertInf(pos, *v,
                        getPosVertInfDirections(*v, YDIM)));
            }
        }
    }
    void generateVisibilityEdgesFromBreakpointSet(Router *router, size_t dim)
    {
        if (breakPoints.empty() || ((breakPoints.begin())->pos > begin))
        {
            // Add a begin point if there was not already an intersection
            // found at that point. Though, don't do this if the line
            // segment goes off to infinity -- we can't reach anything
            // by going in that direction!
            if (begin == -DBL_MAX)
            {
                // Shorten line to first intersection point.
                COLA_ASSERT(!breakPoints.empty());
                begin = breakPoints.begin()->pos;
            }
            else
            {
                // Add begin point.
                Point point(pos, pos);
                point[dim] = begin;
                VertInf *vert = new VertInf(router, dummyOrthogID, point);
                breakPoints.insert(PosVertInf(begin, vert));
            }
        }
        if (breakPoints.empty() || ((breakPoints.rbegin())->pos < finish))
        {
            // Add a finish point if there was not already an intersection
            // found at that point. Though, don't do this if the line
            // segment goes off to infinity -- we can't reach anything
            // by going in that direction!
            if (finish == DBL_MAX)
            {
                // Shorten line to first intersection point.
                finish = breakPoints.rbegin()->pos;
            }
            else
            {
                // Add begin point.
                Point point(pos, pos);
                point[dim] = finish;
                VertInf *vert = new VertInf(router, dummyOrthogID, point);
                breakPoints.insert(PosVertInf(finish, vert));
            }
        }

        // Set flags for orthogonal routing optimisation.
        setLongRangeVisibilityFlags(dim);

        const bool orthogonal = true;
        BreakpointSet::iterator vert, last;
#if 0
        last = breakPoints.end();
        for (vert = breakPoints.begin(); vert != breakPoints.end();)
        {
            if (vert->vert->id == dummyOrthogID)
            {
                if (last == breakPoints.end() || 
                        (last->vert->point != vert->vert->point))
                {
                    last = vert;
                }
                else
                {
                    // Already seen a dummy orthogonal point at this 
                    // position, delete it.

            }
            else
            {
                ++vert;
            }
        }
#endif
        for (vert = last = breakPoints.begin(); vert != breakPoints.end();)
        {
            BreakpointSet::iterator firstPrev = last;
            while (last->vert->point[dim] != vert->vert->point[dim])
            {
                COLA_ASSERT(vert != last);
                // Assert points are not at the same position.
                COLA_ASSERT(vert->vert->point != last->vert->point);

                if (vert->vert->id.isConnPt() && last->vert->id.isConnPt())
                {
                    // Here we have a pair of two endpoints that are both
                    // connector endpoints and both are inside a shape.
                    
                    // Give vert visibility back to the first non-connector
                    // endpoint vertex (i.e., the side of the shape).
                    BreakpointSet::iterator side = last;
                    while (side->vert->id.isConnPt())
                    {
                        if (side == breakPoints.begin())
                        {
                            break;
                        }
                        --side;
                    }
                    bool canSeeDown = (vert->dirs & VisDirDown);
                    if (canSeeDown && !(side->vert->id.isConnPt()))
                    {
                        EdgeInf *edge = new 
                                EdgeInf(side->vert, vert->vert, orthogonal);
                        edge->setDist(vert->vert->point[dim] - 
                                side->vert->point[dim]);
                    }

                    // Give last visibility back to the first non-connector
                    // endpoint vertex (i.e., the side of the shape).
                    side = vert;
                    while ((side != breakPoints.end()) && 
                            side->vert->id.isConnPt())
                    {
                        ++side;
                    }
                    bool canSeeUp = (last->dirs & VisDirUp);
                    if (canSeeUp && (side != breakPoints.end()))
                    {
                        EdgeInf *edge = new 
                                EdgeInf(last->vert, side->vert, orthogonal);
                        edge->setDist(side->vert->point[dim] - 
                                last->vert->point[dim]);
                    }
                }
                
                // The normal case.
                //
                // Note: It's okay to give two connector endpoints visibility 
                // here since we only consider the partner endpoint as a 
                // candidate while searching if it is the other endpoint of
                // the connector in question.
                //
                bool generateEdge = true;
                if (last->vert->id.isConnPt() && !(last->dirs & VisDirUp))
                {
                    // Don't generate the visibility segment if the ConnEnd
                    // doesn't have visibility in that direction.
                    generateEdge = false;
                }
                else if (vert->vert->id.isConnPt() && !(vert->dirs & VisDirDown))
                {
                    // Don't generate the visibility segment if the ConnEnd
                    // doesn't have visibility in that direction.
                    generateEdge = false;
                }
                if (generateEdge)
                {
                    EdgeInf *edge = 
                            new EdgeInf(last->vert, vert->vert, orthogonal);
                    edge->setDist(vert->vert->point[dim] - 
                            last->vert->point[dim]);
                }

                ++last;
            }

            ++vert;

            if ((vert != breakPoints.end()) &&
                    (last->vert->point[dim] == vert->vert->point[dim]))
            {
                // Still looking at same pair, just reset prev number pointer.
                last = firstPrev;
            }
            else
            {
                // vert has moved to the beginning of a group at a new 
                // position.  Last is now in the right place, so do nothing.
            }
        }
    }

    double begin;
    double finish;
    double pos;

    // XXX shapeSide is unused and could possibly be removed?
    bool shapeSide;
    
    VertSet vertInfs;
    BreakpointSet breakPoints;
private:
	// MSVC wants to generate the assignment operator and the default 
	// constructor, but fails.  Therefore we declare them private and 
	// don't implement them.
    LineSegment & operator=(LineSegment const &);
    LineSegment();
};

typedef std::list<LineSegment> SegmentList;

class SegmentListWrapper
{
    public:
        LineSegment *insert(LineSegment segment)
        {
            SegmentList::iterator found = _list.end();
            for (SegmentList::iterator curr = _list.begin();
                    curr != _list.end(); ++curr)
            {
                if (curr->overlaps(segment))
                {
                    if (found != _list.end())
                    {
                        // This is not the first segment that overlaps,
                        // so we need to merge and then delete an existing
                        // segment.
                        curr->mergeVertInfs(*found);
                        _list.erase(found);
                        found = curr;
                    }
                    else
                    {
                        // This is the first overlapping segment, so just 
                        // merge the new segment with this one.
                        curr->mergeVertInfs(segment);
                        found = curr;
                    }
                }
            }

            if (found == _list.end())
            {
                // Add this line.
                _list.push_back(segment);
                return &(_list.back());
            }

            return &(*found);
        }
        SegmentList& list(void)
        {
            return _list;
        }
    private:
        SegmentList _list;
};


// Given a router instance and a set of possible horizontal segments, and a
// possible vertical visibility segment, compute and add edges to the
// orthogonal visibility graph for all the visibility edges.
static void intersectSegments(Router *router, SegmentList& segments, 
        LineSegment& vertLine)
{
    // XXX: It seems that this case can sometimes occur... maybe when
    // there are many overlapping rectangles.
    //COLA_ASSERT(vertLine.beginVertInf() == NULL);
    //COLA_ASSERT(vertLine.finishVertInf() == NULL);

    COLA_ASSERT(!segments.empty());
    for (SegmentList::iterator it = segments.begin(); it != segments.end(); )
    {
        LineSegment& horiLine = *it;

        bool inVertSegRegion = ((vertLine.begin <= horiLine.pos) &&
                                (vertLine.finish >= horiLine.pos));

        if (vertLine.pos < horiLine.begin)
        {
            // We've yet to reach this segment in the sweep, so ignore.
            ++it;
            continue;
        }
        else if (vertLine.pos == horiLine.begin)
        {
            if (inVertSegRegion)
            {
                horiLine.insertBreakpointsBegin(router, vertLine);
            }
        }
        else if (vertLine.pos == horiLine.finish)
        {
            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                horiLine.addEdgeHorizontal(router);
            
                horiLine.insertBreakpointsFinish(router, vertLine);
                
                size_t dim = XDIM; // x-dimension
                horiLine.generateVisibilityEdgesFromBreakpointSet(router, dim);

                // And we've now finished with the segment, so delete.
                it = segments.erase(it);
                continue;
            }
        }
        else if (vertLine.pos > horiLine.finish)
        {
            // Add horizontal visibility segment.
            horiLine.addEdgeHorizontal(router);

            size_t dim = XDIM; // x-dimension
            horiLine.generateVisibilityEdgesFromBreakpointSet(router, dim);

            // We've now swept past this horizontal segment, so delete.
            it = segments.erase(it);
            continue;
        }
        else
        {
            COLA_ASSERT(vertLine.pos > horiLine.begin);
            COLA_ASSERT(vertLine.pos < horiLine.finish);

            if (inVertSegRegion)
            {
                // Add horizontal visibility segment.
                VertSet intersectionVerts = 
                        horiLine.addEdgeHorizontalTillIntersection(
                            router, vertLine);

                for (VertSet::iterator v = intersectionVerts.begin();
                        v != intersectionVerts.end(); ++v)
                {
                    vertLine.breakPoints.insert(PosVertInf(horiLine.pos, *v,
                            getPosVertInfDirections(*v, YDIM)));
                }
            }
        }
        ++it;
    }

    // Split breakPoints set into visibility segments.
    size_t dimension = YDIM; // y-dimension
    vertLine.generateVisibilityEdgesFromBreakpointSet(router, dimension);
}


// Processes an event for the vertical sweep used for computing the static 
// orthogonal visibility graph.  This adds possible visibility segments to 
// the segments list.
// The first pass is adding the event to the scanline, the second is for
// processing the event and the third for removing it from the scanline.
static void processEventVert(Router *router, NodeSet& scanline, 
        SegmentListWrapper& segments, Event *e, unsigned int pass)
{
    Node *v = e->v;
    
    if ( ((pass == 1) && (e->type == Open)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        std::pair<NodeSet::iterator, bool> result = scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        NodeSet::iterator it = v->iter;
        // Work out neighbours
        if (it != scanline.begin()) 
        {
            Node *u = *(--it);
            v->firstAbove = u;
            u->firstBelow = v;
        }
        it = v->iter;
        if (++it != scanline.end()) 
        {
            Node *u = *it;
            v->firstBelow = u;
            u->firstAbove = v;
        }
    }
    
    if (pass == 2)
    {
        if ((e->type == Open) || (e->type == Close))
        {
            // Only difference between Open and Close is whether the line
            // segments are at the top or bottom of the shape.  Decide here.
            double lineY = (e->type == Open) ? v->min[YDIM] : v->max[YDIM];

            // Shape edge positions.
            double minShape = v->min[XDIM];
            double maxShape = v->max[XDIM];
            // As far as we can see.
            double minLimit, maxLimit;
            double minLimitMax, maxLimitMin;
            v->findFirstPointAboveAndBelow(XDIM, lineY, minLimit, maxLimit,
                    minLimitMax, maxLimitMin);

            // Insert possible visibility segments.
            if (minLimitMax >= maxLimitMin)
            {
                // These vertices represent the shape corners.
                VertInf *vI1 = new VertInf(router, dummyOrthogShapeID, 
                            Point(minShape, lineY));
                VertInf *vI2 = new VertInf(router, dummyOrthogShapeID, 
                            Point(maxShape, lineY));
                
                // There are no overlapping shapes, so give full visibility.
                if (minLimit < minShape)
                {
                    segments.insert(LineSegment(minLimit, minShape, lineY,
                                true, NULL, vI1));
                }
                segments.insert(LineSegment(minShape, maxShape, lineY, 
                            true, vI1, vI2));
                if (maxShape < maxLimit)
                {
                    segments.insert(LineSegment(maxShape, maxLimit, lineY,
                                true, vI2, NULL));
                }
            }
            else
            {
                // There are overlapping shapes along this shape edge.

                if ((minLimitMax > minLimit) && (minLimitMax >= minShape))
                {
                    LineSegment *line = segments.insert(
                            LineSegment(minLimit, minLimitMax, lineY, true));
                    // Shape corner:
                    VertInf *vI1 = new VertInf(router, dummyOrthogShapeID, 
                                Point(minShape, lineY));
                    line->vertInfs.insert(vI1);
                }
                if ((maxLimitMin < maxLimit) && (maxLimitMin <= maxShape))
                {
                    LineSegment *line = segments.insert(
                            LineSegment(maxLimitMin, maxLimit, lineY, true));
                    // Shape corner:
                    VertInf *vI2 = new VertInf(router, dummyOrthogShapeID, 
                                Point(maxShape, lineY));
                    line->vertInfs.insert(vI2);
                }
            }
        }
        else if (e->type == ConnPoint)
        {
            // Connection point.
            VertInf *centreVert = e->v->c;
            Point& cp = centreVert->point;

            // As far as we can see.
            double minLimit = v->firstPointAbove(XDIM);
            double maxLimit = v->firstPointBelow(XDIM);
            bool inShape = v->isInsideShape(XDIM);
            
            // Insert if we have visibility in that direction and the segment
            // length is greater than zero.
            LineSegment *line1 = NULL, *line2 = NULL;
            if ((centreVert->visDirections & ConnDirLeft) && (minLimit < cp.x))
            {
                line1 = segments.insert(LineSegment(minLimit, cp.x, e->pos, 
                        true, NULL, centreVert));
            }
            if ((centreVert->visDirections & ConnDirRight) && (cp.x < maxLimit))
            {
                line2 = segments.insert(LineSegment(cp.x, maxLimit, e->pos, 
                        true, centreVert, NULL));
                // If there was a line1, then we just merged with it, so
                // that pointer will be invalid (and now unnecessary).
                line1 = NULL;
            }
            if (!line1 && !line2)
            {
                // Add a point segment for the centre point.
                segments.insert(LineSegment(cp.x, e->pos, centreVert));
            }
            
            if (!inShape)
            {
                // This is not contained within a shape so add a normal
                // visibility graph point here too (since paths won't route
                // *through* connector endpoint vertices).
                if (line1 || line2)
                {
                    VertInf *cent = new VertInf(router, dummyOrthogID, cp);
                    if (line1)
                    {
                        line1->vertInfs.insert(cent);
                    }
                    if (line2)
                    {
                        line2->vertInfs.insert(cent);
                    }
                }
            }
        }
    }
    
    if ( ((pass == 3) && (e->type == Close)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != NULL) 
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != NULL)
        {
            r->firstAbove = v->firstAbove;
        }

        if (e->type == ConnPoint)
        {
            scanline.erase(v->iter);
            delete v;
        }
        else  // if (e->type == Close)
        {
            size_t result;
            result = scanline.erase(v);
            COLA_ASSERT(result == 1);
            COLA_UNUSED(result);  // Avoid warning.
            delete v;
        }
    }
}


// Processes an event for the vertical sweep used for computing the static 
// orthogonal visibility graph.  This adds possible visibility segments to 
// the segments list.
// The first pass is adding the event to the scanline, the second is for
// processing the event and the third for removing it from the scanline.
static void processEventHori(Router *router, NodeSet& scanline, 
        SegmentListWrapper& segments, Event *e, unsigned int pass)
{
    Node *v = e->v;
    
    if ( ((pass == 1) && (e->type == Open)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        std::pair<NodeSet::iterator, bool> result = scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        NodeSet::iterator it = v->iter;
        // Work out neighbours
        if (it != scanline.begin()) 
        {
            Node *u = *(--it);
            v->firstAbove = u;
            u->firstBelow = v;
        }
        it = v->iter;
        if (++it != scanline.end()) 
        {
            Node *u = *it;
            v->firstBelow = u;
            u->firstAbove = v;
        }
    }
    
    if (pass == 2)
    {
        if ((e->type == Open) || (e->type == Close))
        {
            // Only difference between Open and Close is whether the line
            // segments are at the left or right of the shape.  Decide here.
            double lineX = (e->type == Open) ? v->min[XDIM] : v->max[XDIM];

            // Shape edge positions.
            double minShape = v->min[YDIM];
            double maxShape = v->max[YDIM];
            // As far as we can see.
            double minLimit, maxLimit;
            double minLimitMax, maxLimitMin;
            v->findFirstPointAboveAndBelow(YDIM, lineX, minLimit, maxLimit,
                    minLimitMax, maxLimitMin);

            if (minLimitMax >= maxLimitMin)
            {
                LineSegment *line = segments.insert(
                        LineSegment(minLimit, maxLimit, lineX));

                // Shape corners:
                VertInf *vI1 = new VertInf(router, dummyOrthogShapeID, 
                        Point(lineX, minShape));
                VertInf *vI2 = new VertInf(router, dummyOrthogShapeID, 
                        Point(lineX, maxShape));
                line->vertInfs.insert(vI1);
                line->vertInfs.insert(vI2);
            }
            else
            {
                if ((minLimitMax > minLimit) && (minLimitMax >= minShape))
                {
                    LineSegment *line = segments.insert(
                            LineSegment(minLimit, minLimitMax, lineX));

                    // Shape corner:
                    VertInf *vI1 = new VertInf(router, dummyOrthogShapeID, 
                                Point(lineX, minShape));
                    line->vertInfs.insert(vI1);
                }
                if ((maxLimitMin < maxLimit) && (maxLimitMin <= maxShape))
                {
                    LineSegment *line = segments.insert(
                            LineSegment(maxLimitMin, maxLimit, lineX));

                    // Shape corner:
                    VertInf *vI2 = new VertInf(router, dummyOrthogShapeID, 
                                Point(lineX, maxShape));
                    line->vertInfs.insert(vI2);
                }
            }
        }
        else if (e->type == ConnPoint)
        {
            // Connection point.
            VertInf *centreVert = e->v->c;
            Point& cp = centreVert->point;

            // As far as we can see.
            double minLimit = v->firstPointAbove(YDIM);
            double maxLimit = v->firstPointBelow(YDIM);
            
            // Insert if we have visibility in that direction and the segment
            // length is greater than zero.
            if ((centreVert->visDirections & ConnDirUp) && (minLimit < cp.y))
            {
                segments.insert(LineSegment(minLimit, cp.y, e->pos));
            }

            if ((centreVert->visDirections & ConnDirDown) && (cp.y < maxLimit))
            {
                segments.insert(LineSegment(cp.y, maxLimit, e->pos));
            }
        }
    }
    
    if ( ((pass == 3) && (e->type == Close)) ||
         ((pass == 2) && (e->type == ConnPoint)) )
    {
        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != NULL) 
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != NULL)
        {
            r->firstAbove = v->firstAbove;
        }

        if (e->type == ConnPoint)
        {
            scanline.erase(v->iter);
            delete v;
        }
        else  // if (e->type == Close)
        {
            size_t result;
            result = scanline.erase(v);
            COLA_ASSERT(result == 1);
            COLA_UNUSED(result);  // Avoid warning.
            delete v;
        }
    }
}


extern void generateStaticOrthogonalVisGraph(Router *router)
{
    const size_t n = router->m_obstacles.size();
    const unsigned cpn = router->vertices.connsSize();
    // Set up the events for the vertical sweep.
    size_t totalEvents = (2 * n) + cpn;
    Event **events = new Event*[totalEvents];
    unsigned ctr = 0;
    ObstacleList::iterator obstacleIt = router->m_obstacles.begin();
    for (unsigned i = 0; i < n; i++)
    {
        Obstacle *obstacle = *obstacleIt;
#ifndef PAPER
        JunctionRef *junction = dynamic_cast<JunctionRef *> (obstacle);
        if (junction && ! junction->positionFixed())
        {
            // Junctions that are free to move are not treated as obstacles.
            ++obstacleIt;
            totalEvents -= 2;
            continue;
        }
#endif

        Box bbox = obstacle->routingBox();
        double midX = bbox.min.x + ((bbox.max.x - bbox.min.x) / 2);
        Node *v = new Node(obstacle, midX);
        events[ctr++] = new Event(Open, v, bbox.min.y);
        events[ctr++] = new Event(Close, v, bbox.max.y);

        ++obstacleIt;
    }
    for (VertInf *curr = router->vertices.connsBegin(); 
            curr && (curr != router->vertices.shapesBegin()); 
            curr = curr->lstNext)
    {
        if (curr->visDirections == ConnDirNone)
        {
            // This is a connector endpoint that is attached to a connection
            // pin on a shape, so it doesn't need to be given visibility.
            // Thus, skip it and record that there is one less total event.
            --totalEvents;
            continue;
        }
        Point& point = curr->point;

        Node *v = new Node(curr, point.x);
        events[ctr++] = new Event(ConnPoint, v, point.y);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Process the vertical sweep.
    // We do multiple passes over sections of the list so we can add relevant
    // entries to the scanline that might follow, before process them.
    SegmentListWrapper segments;
    NodeSet scanline;
    double thisPos = (totalEvents > 0) ? events[0]->pos : 0;
    unsigned int posStartIndex = 0;
    unsigned int posFinishIndex = 0;
    for (unsigned i = 0; i <= totalEvents; ++i)
    {
        // If we have finished the current scanline or all events, then we
        // process the events on the current scanline in a couple of passes.
        if ((i == totalEvents) || (events[i]->pos != thisPos))
        {
            posFinishIndex = i;
            for (int pass = 2; pass <= 3; ++pass)
            {
                for (unsigned j = posStartIndex; j < posFinishIndex; ++j)
                {
                    processEventVert(router, scanline, segments, 
                            events[j], pass);
                }
            }

            if (i == totalEvents)
            {
                // We have cleaned up, so we can now break out of loop.
                break;
            }

            thisPos = events[i]->pos;
            posStartIndex = i;
        }

        // Do the first sweep event handling -- building the correct 
        // structure of the scanline.
        const int pass = 1;
        processEventVert(router, scanline, segments, events[i], pass);
    }
    COLA_ASSERT(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }

    segments.list().sort();

    // Set up the events for the horizontal sweep.
    SegmentListWrapper vertSegments;
    ctr = 0;
    obstacleIt = router->m_obstacles.begin();
    for (unsigned i = 0; i < n; i++)
    {
        Obstacle *obstacle = *obstacleIt;
#ifndef PAPER
        JunctionRef *junction = dynamic_cast<JunctionRef *> (obstacle);
        if (junction && ! junction->positionFixed())
        {
            // Junctions that are free to move are not treated as obstacles.
            ++obstacleIt;
            continue;
        }
#endif
        Box bbox = obstacle->routingBox();
        double midY = bbox.min.y + ((bbox.max.y - bbox.min.y) / 2);
        Node *v = new Node(obstacle, midY);
        events[ctr++] = new Event(Open, v, bbox.min.x);
        events[ctr++] = new Event(Close, v, bbox.max.x);

        ++obstacleIt;
    }
    for (VertInf *curr = router->vertices.connsBegin(); 
            curr && (curr != router->vertices.shapesBegin()); 
            curr = curr->lstNext)
    {
        if (curr->visDirections == ConnDirNone)
        {
            // This is a connector endpoint that is attached to a connection
            // pin on a shape, so it doesn't need to be given visibility.
            // Thus, skip it.
            continue;
        }
        Point& point = curr->point;

        Node *v = new Node(curr, point.y);
        events[ctr++] = new Event(ConnPoint, v, point.x);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Process the horizontal sweep
    thisPos = (totalEvents > 0) ? events[0]->pos : 0;
    posStartIndex = 0;
    posFinishIndex = 0;
    for (unsigned i = 0; i <= totalEvents; ++i)
    {
        // If we have finished the current scanline or all events, then we
        // process the events on the current scanline in a couple of passes.
        if ((i == totalEvents) || (events[i]->pos != thisPos))
        {
            posFinishIndex = i;
            for (int pass = 2; pass <= 3; ++pass)
            {
                for (unsigned j = posStartIndex; j < posFinishIndex; ++j)
                {
                    processEventHori(router, scanline, vertSegments, 
                            events[j], pass);
                }
            }
            
            // Process the merged line segments.
            vertSegments.list().sort();
            for (SegmentList::iterator curr = vertSegments.list().begin();
                    curr != vertSegments.list().end(); ++curr)
            {
                intersectSegments(router, segments.list(), *curr);
            }
            vertSegments.list().clear();

            if (i == totalEvents)
            {
                // We have cleaned up, so we can now break out of loop.
                break;
            }

            thisPos = events[i]->pos;
            posStartIndex = i;
        }

        // Do the first sweep event handling -- building the correct 
        // structure of the scanline.
        const int pass = 1;
        processEventHori(router, scanline, vertSegments, events[i], pass);
    }
    COLA_ASSERT(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }
    delete [] events;

    // Add portions of the horizontal line that are after the final vertical
    // position we considered.
    for (SegmentList::iterator it = segments.list().begin(); 
            it != segments.list().end(); )
    {
        LineSegment& horiLine = *it;

        horiLine.addEdgeHorizontal(router);
        
        size_t dim = XDIM; // x-dimension
        horiLine.generateVisibilityEdgesFromBreakpointSet(router, dim);

        it = segments.list().erase(it);
    }
}


//============================================================================
//                           Path Adjustment code
//============================================================================




// Processes sweep events used to determine each horizontal and vertical 
// line segment in a connector's channel of visibility.  
// Four calls to this function are made at each position by the scanline:
//   1) Handle all Close event processing.
//   2) Remove Close event objects from the scanline.
//   3) Add Open event objects to the scanline.
//   4) Handle all Open event processing.
//
static void processShiftEvent(NodeSet& scanline, Event *e, size_t dim,
        unsigned int pass)
{
    Node *v = e->v;
    
    if ( ((pass == 3) && (e->type == Open)) ||
         ((pass == 3) && (e->type == SegOpen)) )
    {
        std::pair<NodeSet::iterator, bool> result = scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        NodeSet::iterator it = v->iter;
        // Work out neighbours
        if (it != scanline.begin()) 
        {
            Node *u = *(--it);
            v->firstAbove = u;
            u->firstBelow = v;
        }
        it = v->iter;
        if (++it != scanline.end()) 
        {
            Node *u = *it;
            v->firstBelow = u;
            u->firstAbove = v;
        }
    }
    
    if ( ((pass == 4) && (e->type == Open)) ||
         ((pass == 4) && (e->type == SegOpen)) ||
         ((pass == 1) && (e->type == SegClose)) ||
         ((pass == 1) && (e->type == Close)) )
    {
        if (v->ss)
        {
            // As far as we can see.
            double minLimit = v->firstObstacleAbove(dim);
            double maxLimit = v->firstObstacleBelow(dim);

            v->ss->minSpaceLimit = 
                    std::max(minLimit, v->ss->minSpaceLimit);
            v->ss->maxSpaceLimit = 
                    std::min(maxLimit, v->ss->maxSpaceLimit);
        }
        else
        {
            v->markShiftSegmentsAbove(dim);
            v->markShiftSegmentsBelow(dim);
        }
    }
    
    if ( ((pass == 2) && (e->type == SegClose)) ||
         ((pass == 2) && (e->type == Close)) )
    {
        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != NULL) 
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != NULL)
        {
            r->firstAbove = v->firstAbove;
        }

        size_t result;
        result = scanline.erase(v);
        COLA_ASSERT(result == 1);
        COLA_UNUSED(result);  // Avoid warning.
        delete v;
    }
}

typedef std::pair<Point, Point> RectBounds;

static bool insideRectBounds(const Point& point, const RectBounds& rectBounds)
{
    Point zero(0, 0);
    if ((rectBounds.first == zero) && (rectBounds.second == zero))
    {
        // We can't be inside the invalid rectangle.
        return false;
    }

    for (size_t i = 0; i < 2; ++i)
    {
        if (point[i] < rectBounds.first[i])
        {
            return false;
        }
        if (point[i] > rectBounds.second[i])
        {
            return false;
        }
    }
    return true;
}


static void buildOrthogonalNudgingSegments(Router *router, 
        const size_t dim, ShiftSegmentList& segmentList)
{
    if (router->routingParameter(segmentPenalty) == 0)
    {
        // The nudging code assumes the routes are pretty optimal.  This will
        // only be true if a segment penalty is set, so just return if this 
        // is not the case.
        return;
    }
    bool nudgeFinalSegments = 
            router->routingOption(nudgeOrthogonalSegmentsConnectedToShapes);
    std::vector<RectBounds> shapeLimits;
    if (nudgeFinalSegments)
    {
        // If we're going to nudge final segments, then cache the shape 
        // rectangles to save us rebuilding them multiple times.
        const size_t n = router->m_obstacles.size();
        shapeLimits = std::vector<RectBounds>(n);

        double zeroBufferDist = 0.0;

        ObstacleList::iterator obstacleIt = router->m_obstacles.begin();
        for (unsigned i = 0; i < n; i++)
        {
            ShapeRef *shape = dynamic_cast<ShapeRef *> (*obstacleIt);
            JunctionRef *junction = dynamic_cast<JunctionRef *> (*obstacleIt);
            if (shape)
            {
                // Take the real bounds of the shape
                Box bBox = shape->polygon().offsetBoundingBox(zeroBufferDist);
                shapeLimits[i] = std::make_pair(bBox.min, bBox.max);
            }
            else if (junction)
            {
                // Don't nudge segments attached to junctions,
                // so just use the junction position here.
                Point pos = junction->position();
                shapeLimits[i] = std::make_pair(pos, pos);
            }
            ++obstacleIt;
        }
    }

    size_t altDim = (dim + 1) % 2;
    // For each connector.
    for (ConnRefList::const_iterator curr = router->connRefs.begin(); 
            curr != router->connRefs.end(); ++curr) 
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        Polygon& displayRoute = (*curr)->displayRoute();
        std::vector<Point> checkpoints = (*curr)->routingCheckpoints();
        // Determine all line segments that we are interested in shifting. 
        // We don't consider the first or last segment of a path.
        for (size_t i = 1; i < displayRoute.size(); ++i)
        {
            if (displayRoute.ps[i - 1][dim] == displayRoute.ps[i][dim])
            {
                // It's a segment in the dimension we are processing,
                size_t indexLow = i - 1;
                size_t indexHigh = i;
                if (displayRoute.ps[i - 1][altDim] == 
                        displayRoute.ps[i][altDim])
                {
                    // This is a zero length segment, so ignore it.
                    continue;
                }
                else if (displayRoute.ps[i - 1][altDim] > 
                        displayRoute.ps[i][altDim])
                {
                    indexLow = i;
                    indexHigh = i - 1;
                }

                bool containsCheckpoint = (displayRoute.segmentHasCheckpoint.empty()) ?
                        false : displayRoute.segmentHasCheckpoint[i - 1];
                
                if (containsCheckpoint && !nudgeFinalSegments)
                {
                    // This segment includes one of the routing
                    // checkpoints so we shouldn't shift it.
                    segmentList.push_back(new NudgingShiftSegment(
                            *curr, indexLow, indexHigh, dim));
                    continue;
                }

                double thisPos = displayRoute.ps[i][dim];
                
                if ((i == 1) || ((i + 1) == displayRoute.size()))
                {
                    if (nudgeFinalSegments)
                    {
                        // Determine available space for nudging these
                        // final segments.
                        double minLim = -CHANNEL_MAX;
                        double maxLim = CHANNEL_MAX;
                        
                        // Limit their movement by the length of 
                        // adjoining segments.
                        bool first = (i == 1) ? true : false;
                        bool last = ((i + 1) == displayRoute.size()) ? 
                                true : false;
                        // If the position of the opposite end of the
                        // attached segment is within the shape boundaries
                        // then we want to use this as an ideal position
                        // for the segment.
                        if (!first)
                        {
                            double prevPos = displayRoute.ps[i - 2][dim];
                            if (prevPos < thisPos)
                            {
                                minLim = std::max(minLim, prevPos);
                            }
                            else if (prevPos > thisPos)
                            {
                                maxLim = std::min(maxLim, prevPos);
                            }
                        }
                        if (!last)
                        {
                            double nextPos = displayRoute.ps[i + 1][dim];
                            if (nextPos < thisPos)
                            {
                                minLim = std::max(minLim, nextPos);
                            }
                            else if (nextPos > thisPos)
                            {
                                maxLim = std::min(maxLim, nextPos);
                            }
                        }

                        // Bitflags indicating whether this segment starts 
                        // and/or ends in a shape.
                        unsigned int endsInShapes = 0;
                        // Also limit their movement to the edges of the 
                        // shapes they begin or end within.
                        for (size_t k = 0; k < shapeLimits.size(); ++k)
                        {
                            double shapeMin = shapeLimits[k].first[dim];
                            double shapeMax = shapeLimits[k].second[dim];
                            if (insideRectBounds(displayRoute.ps[i - 1], 
                                        shapeLimits[k]))
                            {
                                minLim = std::max(minLim, shapeMin);
                                maxLim = std::min(maxLim, shapeMax);
                                endsInShapes |= 0x01;
                            }
                            if (insideRectBounds(displayRoute.ps[i], 
                                        shapeLimits[k]))
                            {
                                minLim = std::max(minLim, shapeMin);
                                maxLim = std::min(maxLim, shapeMax);
                                endsInShapes |= 0x10;
                            }
                        }

                        if ( endsInShapes == 0 )
                        {
                            // If the segment is not within a shape, then we
                            // should limit it's nudging buffer so we don't
                            // combine many unnecessary regions.
                            double pos = displayRoute.ps[i - 1][dim];
                            double freeConnBuffer = 15;
                            minLim = std::max(minLim, pos - freeConnBuffer);
                            maxLim = std::min(maxLim, pos + freeConnBuffer);
                        }

                        if (minLim == maxLim)
                        {
                            // Fixed.
                            segmentList.push_back(new NudgingShiftSegment(*curr, 
                                    indexLow, indexHigh, dim));
                        }
                        else
                        {
                            // Shiftable.
                            NudgingShiftSegment *segment = new NudgingShiftSegment(
                                    *curr, indexLow, indexHigh, false, false, dim, 
                                    minLim, maxLim);
                            segment->finalSegment = true;
                            segment->endsInShape = (endsInShapes > 0);
                            if ((displayRoute.size() == 2) && 
                                    (endsInShapes = 0x11))
                            {
                                // This is a single segment connector bridging
                                // two shapes.  So, we want to try to keep the
                                // segment centred rather than shift it.
                                segment->singleConnectedSegment = true;
                            }
                            segmentList.push_back(segment);
                        }
                    }
                    else
                    {
                        // The first and last segment of a connector can't be 
                        // shifted.  We call them fixed segments.  
                        segmentList.push_back(new NudgingShiftSegment(*curr, 
                               indexLow, indexHigh, dim));
                    }
                    continue;
                }
                

                // The segment probably has space to be shifted.
                double minLim = -CHANNEL_MAX;
                double maxLim = CHANNEL_MAX;

                bool isSBend = false;
                bool isZBend = false;

                double prevPos = displayRoute.ps[i - 2][dim];
                double nextPos = displayRoute.ps[i + 1][dim];
                if ( ((prevPos < thisPos) && (nextPos > thisPos)) ||
                     ((prevPos > thisPos) && (nextPos < thisPos)) )
                {

                    // Determine limits if the s-bend is not due to an 
                    // obstacle.  In this case we need to limit the channel 
                    // to the span of the adjoining segments to this one.
                    if ((prevPos < thisPos) && (nextPos > thisPos))
                    {
                        minLim = std::max(minLim, prevPos);
                        maxLim = std::min(maxLim, nextPos);
                        isZBend = true;
                    }
                    else
                    {
                        minLim = std::max(minLim, nextPos);
                        maxLim = std::min(maxLim, prevPos);
                        isSBend = true;
                    }
                }
                else
                {
                    // isCBend: Both adjoining segments are in the same
                    // direction.  We indicate this for later by setting 
                    // the maxLim or minLim to the segment position.
                    if (prevPos < thisPos)
                    {
                        minLim = thisPos;
                    }
                    else
                    {
                        maxLim = thisPos;
                    }
                }

                NudgingShiftSegment *nss = new NudgingShiftSegment(*curr,
                        indexLow, indexHigh, isSBend, isZBend, dim,
                        minLim, maxLim);
                nss->containsCheckpoint = containsCheckpoint;
                segmentList.push_back(nss);
            }
        }
    }
}

static void buildOrthogonalChannelInfo(Router *router, 
        const size_t dim, ShiftSegmentList& segmentList)
{
    if (segmentList.empty())
    {
        // There are no segments, so we can just return now.
        return;
    }
    
    // Do a sweep to determine space for shifting segments.
    size_t altDim = (dim + 1) % 2;
    const size_t n = router->m_obstacles.size();
    const size_t cpn = segmentList.size();
    // Set up the events for the sweep.
    size_t totalEvents = 2 * (n + cpn);
    Event **events = new Event*[totalEvents];
    unsigned ctr = 0;
    ObstacleList::iterator obstacleIt = router->m_obstacles.begin();
    for (unsigned i = 0; i < n; i++)
    {
        Obstacle *obstacle = *obstacleIt;
        JunctionRef *junction = dynamic_cast<JunctionRef *> (obstacle);
        if (junction && ! junction->positionFixed())
        {
            // Junctions that are free to move are not treated as obstacles.
            ++obstacleIt;
            totalEvents -= 2;
            continue;
        }
        Box bBox = obstacle->routingBox();
        Point min = bBox.min;
        Point max = bBox.max;
        double mid = min[dim] + ((max[dim] - min[dim]) / 2);
        Node *v = new Node(obstacle, mid);
        events[ctr++] = new Event(Open, v, min[altDim]);
        events[ctr++] = new Event(Close, v, max[altDim]);

        ++obstacleIt;
    }
    for (ShiftSegmentList::iterator curr = segmentList.begin(); 
            curr != segmentList.end(); ++curr)
    {
        const Point& lowPt = (*curr)->lowPoint();
        const Point& highPt = (*curr)->highPoint();

        COLA_ASSERT(lowPt[dim] == highPt[dim]);
        COLA_ASSERT(lowPt[altDim] < highPt[altDim]);
        Node *v = new Node(*curr, lowPt[dim]);
        events[ctr++] = new Event(SegOpen, v, lowPt[altDim]);
        events[ctr++] = new Event(SegClose, v, highPt[altDim]);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Process the sweep.
    // We do multiple passes over sections of the list so we can add relevant
    // entries to the scanline that might follow, before process them.
    NodeSet scanline;
    double thisPos = (totalEvents > 0) ? events[0]->pos : 0;
    unsigned int posStartIndex = 0;
    unsigned int posFinishIndex = 0;
    for (unsigned i = 0; i <= totalEvents; ++i)
    {
        // If we have finished the current scanline or all events, then we
        // process the events on the current scanline in a couple of passes.
        if ((i == totalEvents) || (events[i]->pos != thisPos))
        {
            posFinishIndex = i;
            for (int pass = 2; pass <= 4; ++pass)
            {
                for (unsigned j = posStartIndex; j < posFinishIndex; ++j)
                {
                    processShiftEvent(scanline, events[j], dim, pass);
                }
            }

            if (i == totalEvents)
            {
                // We have cleaned up, so we can now break out of loop.
                break;
            }

            thisPos = events[i]->pos;
            posStartIndex = i;
        }

        // Do the first sweep event handling -- building the correct 
        // structure of the scanline.
        const int pass = 1;
        processShiftEvent(scanline, events[i], dim, pass);
    }
    COLA_ASSERT(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }
    delete [] events;
}


static void simplifyOrthogonalRoutes(Router *router)
{
    // Simplify routes.
    for (ConnRefList::const_iterator curr = router->connRefs.begin(); 
            curr != router->connRefs.end(); ++curr) 
    {
        if ((*curr)->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        (*curr)->set_route((*curr)->displayRoute().simplify());
    }
}

typedef std::vector<ConnRef *> ConnRefVector;
typedef std::vector<Polygon> RouteVector;

static void buildOrthogonalNudgingOrderInfo(Router *router, 
        PtOrderMap& pointOrders)
{
    // Simplify routes.
    simplifyOrthogonalRoutes(router);

    int crossingsN = 0;

    // Make a vector of the ConnRefList, for convenience.
    ConnRefVector connRefs(router->connRefs.begin(), router->connRefs.end());
    
    // Make a temporary copy of all the connector displayRoutes.
    RouteVector connRoutes(connRefs.size());
    for (size_t ind = 0; ind < connRefs.size(); ++ind)
    {
        connRoutes[ind] = connRefs[ind]->displayRoute();
    }

    // Do segment splitting.
    for (size_t ind1 = 0; ind1 < connRefs.size(); ++ind1)
    {
        ConnRef *conn = connRefs[ind1];
        if (conn->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        
        for (size_t ind2 = 0; ind2 < connRefs.size(); ++ind2)
        {
            if (ind1 == ind2)
            {
                continue;
            }
            
            ConnRef *conn2 = connRefs[ind2];
            if (conn2->routingType() != ConnType_Orthogonal)
            {
                continue;
            }
            
            Avoid::Polygon& route = connRoutes[ind1];
            Avoid::Polygon& route2 = connRoutes[ind2];
            splitBranchingSegments(route2, true, route);
        }
    }

    for (size_t ind1 = 0; ind1 < connRefs.size(); ++ind1)
    {
        ConnRef *conn = connRefs[ind1];
        if (conn->routingType() != ConnType_Orthogonal)
        {
            continue;
        }
        
        for (size_t ind2 = ind1 + 1; ind2 < connRefs.size(); ++ind2)
        {
            ConnRef *conn2 = connRefs[ind2];
            if (conn2->routingType() != ConnType_Orthogonal)
            {
                continue;
            }
            
            Avoid::Polygon& route = connRoutes[ind1];
            Avoid::Polygon& route2 = connRoutes[ind2];
            int crossings = 0;
            ConnectorCrossings cross(route2, true, route, conn2, conn);
            cross.pointOrders = &pointOrders;
            for (size_t i = 1; i < route.size(); ++i)
            {
                const bool finalSegment = ((i + 1) == route.size());
                cross.countForSegment(i, finalSegment);

                crossings += cross.crossingCount;
            }
            if (crossings > 0)
            {
                crossingsN += crossings;
            }
        }
    }
}


class CmpLineOrder 
{
    public:
        CmpLineOrder(PtOrderMap& ord, const size_t dim)
            : orders(ord),
              dimension(dim)
        {
        }
        bool operator()(const ShiftSegment *lhsSuper, 
                const ShiftSegment *rhsSuper,
                bool *comparable = NULL) const
        {
            const NudgingShiftSegment *lhs = 
                    dynamic_cast<const NudgingShiftSegment *> (lhsSuper);
            const NudgingShiftSegment *rhs = 
                    dynamic_cast<const NudgingShiftSegment *> (rhsSuper);
            if (comparable)
            {
                *comparable = true;
            }
            Point lhsLow  = lhs->lowPoint(); 
            Point rhsLow  = rhs->lowPoint(); 
            size_t altDim = (dimension + 1) % 2;
#ifndef NDEBUG
            const Point& lhsHigh = lhs->highPoint(); 
            const Point& rhsHigh = rhs->highPoint(); 
            COLA_ASSERT(lhsLow[dimension] == lhsHigh[dimension]);
            COLA_ASSERT(rhsLow[dimension] == rhsHigh[dimension]);
#endif

            // We consider things at effectively the same position to 
            // be ordered based on their order and fixedOrder, so only 
            // compare segments further apart than the nudgeDistance.
            if (lhsLow[dimension] != rhsLow[dimension])
            {
                return lhsLow[dimension] < rhsLow[dimension];
            }

            // If one of these is fixed, then determine order based on
            // fixed segment, that is, order so the fixed segment doesn't 
            // block movement.
            bool oneIsFixed = false;
            const int lhsFixedOrder = lhs->fixedOrder(oneIsFixed);
            const int rhsFixedOrder = rhs->fixedOrder(oneIsFixed);
            if (oneIsFixed && (lhsFixedOrder != rhsFixedOrder))
            {
                return lhsFixedOrder < rhsFixedOrder;
            }

            // C-bends that did not have a clear order with s-bends might 
            // not have a good ordering here, so compare their order in 
            // terms of C-bend direction and S-bends and use that if it
            // differs for the two segments.
            const int lhsOrder = lhs->order();
            const int rhsOrder = rhs->order();
            if (lhsOrder != rhsOrder)
            {
                return lhsOrder < rhsOrder;
            }
            
            // Need to index using the original point into the map, so find it.
            Point& unchanged = (lhsLow[altDim] > rhsLow[altDim]) ?
                    lhsLow : rhsLow;

            PtOrder& lowOrder = orders[unchanged];
            int lhsPos = lowOrder.positionFor(dimension, lhs->connRef);
            int rhsPos = lowOrder.positionFor(dimension, rhs->connRef);
            if ((lhsPos == -1) || (rhsPos == -1))
            {
                // A value for rhsPos or lhsPos mean the points are not directly
                // comparable, meaning they are at the same position but cannot
                // overlap (they are just collinear.  The relative order for 
                // these segments is not important since we do not constrain
                // them against each other.
                //COLA_ASSERT(lhs->overlapsWith(rhs, dimension) == false);
                // We do need to be consistent though.
                if (comparable)
                {
                    *comparable = false;
                }
                return lhsLow[altDim] < rhsLow[altDim];
            }
            return lhsPos < rhsPos;
        }

        PtOrderMap& orders;
        const size_t dimension;
};


// We can't use the normal sort algorithm for lists since it is not possible 
// to compare all elements, but there will be an ordering defined between 
// most of the elements.  Hence we order these, using insertion sort, and 
// the case of them not being able to be compared is handled by not setting 
// up any constraints between such segments when doing the nudging.
//
static ShiftSegmentList linesort(bool nudgeFinalSegments, 
        ShiftSegmentList origList, CmpLineOrder& comparison)
{
    // Cope with end segments that are getting moved and will line up with
    // other segments of the same connector.  We do this by merging them into
    // a single NudgingShiftSegment.
    if (nudgeFinalSegments)
    {
        for (ShiftSegmentList::iterator currSegIt = origList.begin();
                currSegIt != origList.end(); ++currSegIt)
        {
            for (ShiftSegmentList::iterator otherSegIt = currSegIt;
                    otherSegIt != origList.end(); )
            {
                NudgingShiftSegment *currSeg = 
                        dynamic_cast<NudgingShiftSegment *> (*currSegIt);
                NudgingShiftSegment *otherSeg = 
                        dynamic_cast<NudgingShiftSegment *> (*otherSegIt);
                if ((currSegIt != otherSegIt) && currSeg && otherSeg && 
                        currSeg->shouldAlignWith(otherSeg, comparison.dimension))
                {
                    currSeg->mergeWith(otherSeg, comparison.dimension);
                    delete otherSeg;
                    otherSegIt = origList.erase(otherSegIt);
                }
                else
                {
                    ++otherSegIt;
                }
            }
        }
    }

    ShiftSegmentList resultList;

    size_t origListSize = origList.size();
    size_t deferredN = 0; 
    while (!origList.empty())
    {
        // Get and remove the first element from the origList.
        ShiftSegment *segment = origList.front();
        origList.pop_front();

        // Find the insertion point in the resultList.
        bool allComparable = true;
        ShiftSegmentList::iterator curr;
        for (curr = resultList.begin(); curr != resultList.end(); ++curr)
        {
            bool comparable = false;
            bool lessThan = comparison(segment, *curr, &comparable);
            allComparable &= comparable;

            if (comparable && lessThan)
            {
                // If it is comparable and lessThan, then we have found the
                // insertion point.
                break;
            }
        }
    
        if (resultList.empty() || allComparable || (deferredN >= origListSize))
        {
            // Insert the element into the resultList at the required point.
            resultList.insert(curr, segment);
            // Reset the origListSize and deferred counter.
            deferredN = 0;
            origListSize = origList.size();
        }
        else
        {
            // This wasn't comparable to anything in the sorted list, 
            // so defer addition of the segment till later.
            origList.push_back(segment);
            deferredN++;
        }
    }

    return resultList;
}


typedef std::list<ShiftSegment *> ShiftSegmentPtrList;

class PotentialSegmentConstraint
{
    public:
        PotentialSegmentConstraint(unsigned index1, unsigned index2, 
                const Variables& vs)
            : index1(index1),
              index2(index2),
              vs(vs)
        {
        }

        bool operator<(const PotentialSegmentConstraint rhs) const
        {
            return sepDistance() < rhs.sepDistance();
        }
        double sepDistance(void) const
        {
            if (!stillValid())
            {
                return 0;
            }
            return fabs(vs[index1]->finalPosition - vs[index2]->finalPosition);
        }
        bool stillValid(void) const
        {
            return (index1 != index2);
        }
        void rewriteIndex(unsigned oldIndex, unsigned newIndex)
        {
            if (index1 == oldIndex)
            {
                index1 = newIndex;
            }

            if (index2 == oldIndex)
            {
                index2 = newIndex;
            }
        }

        unsigned index1;
        unsigned index2;

    private:
        const Variables& vs;
};

static void nudgeOrthogonalRoutes(Router *router, size_t dimension, 
        PtOrderMap& pointOrders, ShiftSegmentList& segmentList, 
        bool justUnifying = false)
{
    bool nudgeFinalSegments = router->routingOption(
            nudgeOrthogonalSegmentsConnectedToShapes);
    double baseSepDist = router->routingParameter(idealNudgingDistance);
    COLA_ASSERT(baseSepDist >= 0);
    // If we can fit things with the desired separation distance, then
    // we try 10 times, reducing each time by a 10th of the original amount.
    double reductionSteps = 10.0;

    // Do the actual nudging.
    ShiftSegmentList currentRegion;
    while (!segmentList.empty())
    {
        // Take a reference segment
        ShiftSegment *currentSegment = segmentList.front();
        // Then, find the segments that overlap this one.
        currentRegion.clear();
        currentRegion.push_back(currentSegment);
        segmentList.erase(segmentList.begin());
        for (ShiftSegmentList::iterator curr = segmentList.begin();
                curr != segmentList.end(); )
        {
            bool overlaps = false;
            for (ShiftSegmentList::iterator curr2 = currentRegion.begin();
                    curr2 != currentRegion.end(); ++curr2)
            {
                if ((*curr)->overlapsWith(*curr2, dimension))
                {
                    overlaps = true;
                    break;
                }
            }
            if (overlaps)
            {
                currentRegion.push_back(*curr);
                segmentList.erase(curr);
                // Consider segments from the beginning, since we may have
                // since passed segments that overlap with the new set.
                curr = segmentList.begin();
            }
            else
            {
                ++curr;
            }
        }

        if (! justUnifying)
        {
            CmpLineOrder lineSortComp(pointOrders, dimension);
            currentRegion = linesort(nudgeFinalSegments, currentRegion,
                    lineSortComp);
        }

        if (currentRegion.size() == 1)
        {
            // Save creating the solver instance if there is just one
            // immovable segment, or we're nudging a single segment.
            if ( !justUnifying || currentRegion.front()->immovable() )
            {
                delete currentRegion.front();
                continue;
            }
        }

        // Process these segments.
        std::list<unsigned> freeIndexes;
        Variables vs;
        Constraints cs;
        Constraints gapcs;
        ShiftSegmentPtrList prevVars;
        double sepDist = baseSepDist;
#ifdef NUDGE_DEBUG 
        printf("-------------------------------------------------------\n");
        printf("Nudge -- size: %d\n", (int) currentRegion.size());
#endif
#ifdef NUDGE_DEBUG_SVG
        printf("\n\n");
#endif
        for (ShiftSegmentList::iterator currSegmentIt = currentRegion.begin();
                currSegmentIt != currentRegion.end(); ++currSegmentIt )
        {
            NudgingShiftSegment *currSegment = dynamic_cast<NudgingShiftSegment *> (*currSegmentIt);
            
            // Create a solver variable for the position of this segment.
            currSegment->createSolverVariable(justUnifying);
            
            vs.push_back(currSegment->variable);
            size_t index = vs.size() - 1;
#ifdef NUDGE_DEBUG
            fprintf(stderr,"line(%d)  %.15f  dim: %d pos: %g   min: %g  max: %g\n"
                   "minEndPt: %g  maxEndPt: %g weight: %g cc: %d\n",
                    currSegment->connRef->id(),
                    currSegment->lowPoint()[dimension], (int) dimension, 
                    currSegment->variable->desiredPosition, 
                    currSegment->minSpaceLimit, currSegment->maxSpaceLimit,
                    currSegment->lowPoint()[!dimension], currSegment->highPoint()[!dimension], 
                    currSegment->variable->weight, 
                    (int) currSegment->containsCheckpoint);
#endif
#ifdef NUDGE_DEBUG_SVG
            // Debugging info:
            double minP = std::max(currSegment->minSpaceLimit, -5000.0);
            double maxP = std::min(currSegment->maxSpaceLimit, 5000.0);
            fprintf(stdout, "<rect style=\"fill: #f00; opacity: 0.2;\" "
                    "x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" />\n",
                    currSegment->lowPoint()[XDIM], minP, 
                    currSegment->highPoint()[XDIM] - currSegment->lowPoint()[XDIM], 
                    maxP - minP);
            fprintf(stdout, "<line style=\"stroke: #000;\" x1=\"%g\" "
                    "y1=\"%g\" x2=\"%g\" y2=\"%g\" />\n",
                    currSegment->lowPoint()[XDIM], currSegment->lowPoint()[YDIM],
                    currSegment->highPoint()[XDIM], currSegment->highPoint()[YDIM]);
#endif

            // Constrain to channel boundary.
            if (!currSegment->fixed)
            {
                // If this segment sees a channel boundary to its left,
                // then constrain its placement as such.
                if (currSegment->minSpaceLimit > -CHANNEL_MAX)
                {
                    vs.push_back(new Variable(fixedID,
                                currSegment->minSpaceLimit, fixedWeight));
                    cs.push_back(new Constraint(vs[vs.size() - 1], vs[index],
                                0.0));
                }

                // If this segment sees a channel boundary to its right,
                // then constrain its placement as such.
                if (currSegment->maxSpaceLimit < CHANNEL_MAX)
                {
                    vs.push_back(new Variable(fixedID,
                                currSegment->maxSpaceLimit, fixedWeight));
                    cs.push_back(new Constraint(vs[index], vs[vs.size() - 1],
                                0.0));
                }
            }

            if (justUnifying)
            {
                // Just doing centring, not nudging.
                // Record the index of the variable so we can use it as 
                // a segment to potentially constrain to other segments.
                if (currSegment->variable->weight == freeWeight)
                {
                    freeIndexes.push_back(index);
                }
                // Thus, we don't need to constrain position against other
                // segments.
                prevVars.push_back(&(*currSegment));
                continue;
            }

            // Constrain position in relation to previously seen segments,
            // if necessary (i.e. when they could overlap).
            for (ShiftSegmentPtrList::iterator prevVarIt = prevVars.begin();
                    prevVarIt != prevVars.end(); ++prevVarIt)
            {
                NudgingShiftSegment *prevSeg =
                        dynamic_cast<NudgingShiftSegment *> (*prevVarIt);
                Variable *prevVar = prevSeg->variable;
                
                if (currSegment->overlapsWith(prevSeg, dimension) &&
                        (!(currSegment->fixed) || !(prevSeg->fixed)))
                {
                    // If there is a previous segment to the left that 
                    // could overlap this in the shift direction, then 
                    // constrain the two segments to be separated.
                    // Though don't add the constraint if both the 
                    // segments are fixed in place.
                    double thisSepDist = sepDist;
                    bool equality = false;
                    if (currSegment->shouldAlignWith(prevSeg, dimension))
                    {
                        // Handles the case where the two end segments can
                        // be brought together to make a single segment. This
                        // can help in situations where having the small kink
                        // can restrict other kinds of nudging.
                        thisSepDist = 0;
                        equality = true;
                    }
                    else if (currSegment->connRef == prevSeg->connRef)
                    {
                        // We need to address the problem of two neighbouring
                        // segments of the same connector being kept separated
                        // due only to a kink created in the other dimension.
                        // Here, we let such segments drift back together.
                        thisSepDist = 0;
                    }
                    
                    Constraint *constraint = new Constraint(prevVar, 
                            vs[index], thisSepDist, equality);
                    cs.push_back(constraint);
                    if (thisSepDist)
                    {
                        // Add to the list of gap constraints so we can 
                        // rewrite the separation distance later.
                        gapcs.push_back(constraint);
                    }
                }
            }

            prevVars.push_back(&(*currSegment));
        }

        std::list<PotentialSegmentConstraint> potentialConstraints;
        if (justUnifying)
        {
            for (std::list<unsigned>::iterator curr = freeIndexes.begin();
                    curr != freeIndexes.end(); ++curr)
            {
                for (std::list<unsigned>::iterator curr2 = curr;
                        curr2 != freeIndexes.end(); ++curr2)
                {
                    if (curr == curr2)
                    {
                        continue;
                    }
                    potentialConstraints.push_back(
                            PotentialSegmentConstraint(*curr, *curr2, vs));
                }
            }
        }
#ifdef NUDGE_DEBUG
        for (unsigned i = 0;i < vs.size(); ++i)
        {
            printf("-vs[%d]=%f\n", i, vs[i]->desiredPosition);
        }
#endif
        // Repeatedly try solving this.  There are two cases:
        //  -  When Unifying, we greedily place as many free segments as 
        //     possible at the same positions, that way they have more 
        //     accurate nudging orders determined for them in the Nudging
        //     stage.
        //  -  When Nudging, if we can't fit all the segments with the 
        //     default nudging distance we try smaller separation 
        //     distances till we find a solution that is satisfied.
        bool justAddedConstraint = false;
        bool satisfied;
        do 
        {
            IncSolver f(vs, cs);
            f.solve();
            satisfied = true;
            for (size_t i = 0; i < vs.size(); ++i) 
            {
                if (vs[i]->id == fixedID)
                {
                    if (fabs(vs[i]->finalPosition - 
                            vs[i]->desiredPosition) > 0.01)
                    {
                        satisfied = false;
                        break;
                    }
                }
            }

            if (justUnifying)
            {
                // When we're centring, we'd like to greedily place as many
                // segments as possible at the same positions, that way they
                // have more accurate nudging orders determined for them.
                // 
                // We do this by taking pairs of adjoining free segments and 
                // attempting to constrain them to have the same position, 
                // starting from the closest up to the furthest.

                if (justAddedConstraint)
                {
                    COLA_ASSERT(potentialConstraints.size() > 0);
                    if (!satisfied)
                    {
                        // We couldn't satisfy the problem with the added
                        // potential constraint, so we can't position these
                        // segments together.  Roll back.
                        potentialConstraints.pop_front();
                        delete cs.back();
                        cs.pop_back();
                    }
                    else
                    {
                        // We could position these two segments together.
                        PotentialSegmentConstraint& pc =
                                potentialConstraints.front();

                        // Rewrite the indexes of these two variables to 
                        // one, so we need not worry about redundant 
                        // equality constraints.
                        for (std::list<PotentialSegmentConstraint>::iterator
                                it = potentialConstraints.begin();
                                it != potentialConstraints.end(); ++it)
                        {
                            it->rewriteIndex(pc.index1, pc.index2);
                        }
                        potentialConstraints.pop_front();
                    }
                }
                potentialConstraints.sort();
                justAddedConstraint = false;

                // Remove now invalid potential segment constraints.
                // This could have been caused by the variable rewriting.
                while (!potentialConstraints.empty() && 
                       !potentialConstraints.front().stillValid())
                {
                    potentialConstraints.pop_front();
                }

                if (!potentialConstraints.empty())
                {
                    // We still have more possibilities to consider.
                    // Create a constraint for this, add it, and mark as
                    // unsatisfied, so the problem gets re-solved.
                    PotentialSegmentConstraint& pc =
                            potentialConstraints.front();
                    COLA_ASSERT(pc.index1 != pc.index2);
                    cs.push_back(new Constraint(vs[pc.index1], vs[pc.index2],
                            0, true));
                    satisfied = false;
                    justAddedConstraint = true;
                }
            }
            else
            {
                if (!satisfied)
                {
                    // Reduce the separation distance.
                    sepDist -= (baseSepDist / reductionSteps);
                    // And rewrite all the gap constraints to have the new 
                    // reduced separation distance.
                    for (Constraints::iterator cIt = gapcs.begin(); 
                            cIt != gapcs.end(); ++cIt)
                    {
                        Constraint *constraint = *cIt;
                        constraint->gap = sepDist;
                    }
                }
            }
        }
        while (!satisfied && (sepDist > 0.0001));

        if (satisfied)
        {
            for (ShiftSegmentList::iterator currSegment = currentRegion.begin();
                    currSegment != currentRegion.end(); ++currSegment)
            {
                NudgingShiftSegment *segment =
                        dynamic_cast<NudgingShiftSegment *> (*currSegment);

                segment->updatePositionsFromSolver();
            }
        }
#ifdef NUDGE_DEBUG
        for(unsigned i=0;i<vs.size();i++) {
            printf("+vs[%d]=%f\n",i,vs[i]->finalPosition);
        }
#endif
#ifdef NUDGE_DEBUG_SVG
        for (ShiftSegmentList::iterator currSegment = currentRegion.begin();
                currSegment != currentRegion.end(); ++currSegment)
        {
            NudgingShiftSegment *segment =
                    dynamic_cast<NudgingShiftSegment *> (*currSegment);

            fprintf(stdout, "<line style=\"stroke: #00F;\" x1=\"%g\" "
                    "y1=\"%g\" x2=\"%g\" y2=\"%g\" />\n",
                    segment->lowPoint()[XDIM], segment->variable->finalPosition,
                    segment->highPoint()[XDIM], segment->variable->finalPosition);
        }
#endif
        for_each(currentRegion.begin(), currentRegion.end(), delete_object());
        for_each(vs.begin(), vs.end(), delete_object());
        for_each(cs.begin(), cs.end(), delete_object());
    }
}

extern void improveOrthogonalRoutes(Router *router)
{
    router->timers.Register(tmOrthogNudge, timerStart);

    // Simplify routes.
    simplifyOrthogonalRoutes(router);

    // Build a cache that denotes whether a certain segment of a connector
    // contains a checkpoint.  We can't just compare positions, since routes
    // can be moved away from their original positions during nudging.
    buildConnectorRouteCheckpointCache(router);

    // Do Unifying first, by itself.  This greedily tries to position free
    // segments in overlaping channels at the same position.  This way they
    // have correct nudging orders determined for them since they will form
    // shared paths, rather than segments just positioned as an results of
    // the routing process.  Of course, don't do this when rerouting with
    // a fixedSharedPathPenalty since these routes include extra segments 
    // we want to keep apart which prevent some shared paths.
    if (router->routingParameter(fixedSharedPathPenalty) == 0)
    {
        PtOrderMap pointOrders;
        for (size_t dimension = 0; dimension < 2; ++dimension)
        {
            // Just perform Unifying operation.
            bool justUnifying = true;
            ShiftSegmentList segmentList;
            buildOrthogonalNudgingSegments(router, dimension, segmentList);
            buildOrthogonalChannelInfo(router, dimension, segmentList);
            nudgeOrthogonalRoutes(router, dimension, pointOrders, segmentList,
                    justUnifying);
        }
    }

    // Do the Nudging and centring.
    for (size_t dimension = 0; dimension < 2; ++dimension)
    {
        // Build nudging info.
        // XXX Needs to be rebuilt for each dimension, cause of shifting
        //     points.  Maybe we could modify the point orders.
        PtOrderMap pointOrders;
        buildOrthogonalNudgingOrderInfo(router, pointOrders);

        // Do the centring and nudging.
        ShiftSegmentList segmentList;
        buildOrthogonalNudgingSegments(router, dimension, segmentList);
        buildOrthogonalChannelInfo(router, dimension, segmentList);
        nudgeOrthogonalRoutes(router, dimension, pointOrders, segmentList);
    }

    // Resimplify all the display routes that may have been split.
    simplifyOrthogonalRoutes(router);

    router->improveOrthogonalTopology();
 
    // Clear the segment-checkpoint cache for connectors.
    clearConnectorRouteCheckpointCache(router);

    router->timers.Stop();
}


typedef std::map<JunctionRef *, ShiftSegmentList> RootSegmentsMap;

struct ImproveHyperEdges
{
    // Constructor.
    ImproveHyperEdges(Router *router)
        : router(router),
          debug_count(0)
    {
    }

    // Helper method for buildHyperEdgeSegments() for hyperedge tree nodes.
    void createShiftSegmentsForDimensionExcluding(HyperEdgeTreeNode *node,
            const size_t dim, HyperEdgeTreeEdge *ignore, 
            ShiftSegmentList& segments)
    {
        for (std::list<HyperEdgeTreeEdge *>::iterator curr = node->edges.begin();
                curr != node->edges.end(); ++curr)
        {
            HyperEdgeTreeEdge *edge = *curr;
            if (edge != ignore)
            {
                createShiftSegmentsForDimensionExcluding(edge, dim,
                        node, segments);
            }
        }
    }

    // Helper method for buildHyperEdgeSegments() for hyperedge tree edges.
    void createShiftSegmentsForDimensionExcluding(HyperEdgeTreeEdge *edge,
            const size_t dim, HyperEdgeTreeNode *ignore, 
            ShiftSegmentList& segments)
    {
        if (edge->hasOrientation(dim) && ! edge->zeroLength())
        {
            bool immovable = (edge->ends.first->edges.size() == 1) || 
                    (edge->ends.second->edges.size() == 1);
            HyperEdgeShiftSegment *newSegment =
                    new HyperEdgeShiftSegment(edge->ends.first,
                    edge->ends.second, dim, immovable);
            segments.push_back(newSegment);
        }

        if (edge->ends.first && (edge->ends.first != ignore))
        {
            createShiftSegmentsForDimensionExcluding(edge->ends.first, dim,
                    edge, segments);
        }

        if (edge->ends.second && (edge->ends.second != ignore))
        {
            createShiftSegmentsForDimensionExcluding(edge->ends.second, dim,
                    edge, segments);
        }
    }

    // During creation and nudging of shift segments it is often necessary
    // to merge collinear or overlapping segments.  This method does the
    // merging for these cases.  Effectively merging is done by adding
    // additional vertex pointers to the shift segment.
    void mergeOverlappingSegments(ShiftSegmentList& segments)
    {
        for (ShiftSegmentList::iterator curr = segments.begin();
                curr != segments.end(); ++curr)
        {
            HyperEdgeShiftSegment *edge1 =
                    dynamic_cast<HyperEdgeShiftSegment *> (*curr);
            for (ShiftSegmentList::iterator curr2 = segments.begin();
                    curr2 != segments.end(); )
            {
                if (curr2 == curr)
                {
                    ++curr2;
                    continue;
                }
                HyperEdgeShiftSegment *edge2 =
                        dynamic_cast<HyperEdgeShiftSegment *> (*curr2);
                if (edge1->mergesWith(edge2))
                {
                    delete edge2;
                    curr2 = segments.erase(curr2);
                }
                else
                {
                    ++curr2;
                }
            }
        }
    }

    // Given a hyperedge tree and a dimension, this method creates shift
    // segments for all edges in that orientation.  These segments are the
    // objects on which the local improvement nudging operates, and they
    // in turn make changes back to the hyperedge tree.
    void buildHyperEdgeSegments(const size_t dim)
    {
        for (JunctionSet::iterator curr = hyperEdgeTreeRoots.begin();
                curr != hyperEdgeTreeRoots.end(); ++curr)
        {
            ShiftSegmentList& segments = rootShiftSegments[*curr];
            
            HyperEdgeTreeNode *node = hyperEdgeTreeJunctions[*curr];
            createShiftSegmentsForDimensionExcluding(node, dim, NULL, segments);

            // Merge overlapping segment.
            mergeOverlappingSegments(segments);

            allShiftSegments.insert(allShiftSegments.begin(), 
                    segments.begin(), segments.end());    
        }
    }

    // This method looks for and corrects situations where the middle section
    // of a zigzag is optimised away by bringing the outside segments in line
    // and leading to the middle segment being zero length.  These zero length
    // edges are removed.
    void removeZeroLengthEdges(void)
    {
        for (JunctionSet::iterator curr = hyperEdgeTreeRoots.begin();
                curr != hyperEdgeTreeRoots.end(); ++curr)
        {
            HyperEdgeTreeNode *node = hyperEdgeTreeJunctions[*curr];

            node->removeZeroLengthEdges(NULL);
        }
    }

    // This method looks for and correct situations where multiple overlapping
    // edges lead to a junction and one or more of these segments could be
    // removed by moving the junction (and thus divergence point) along the
    // edge.
    void moveJunctionsAlongCommonEdges(void)
    {
        for (JunctionHyperEdgeTreeNodeMap::iterator curr = 
                hyperEdgeTreeJunctions.begin(); 
                curr != hyperEdgeTreeJunctions.end(); ++curr)
        {
            HyperEdgeTreeNode *node = curr->second;

            // For each junction, try and move it.
            while ((node = HyperEdgeTreeNode::moveJunctionAlongCommonEdge(node)))
            {
                if (node)
                {
                    // Junction has moved, rewrite the pointer in
                    // the hyperEdgeTreeJunctions map.
                    curr->second = node;
                }
            }
        }
    }

    // Given a set of hyperedge shift segments in a particular dimension,
    // with limits and balance values precomputed, this method shifts and
    // merges segments to improve the overall cost (length + bend penalties)
    // for the hyperedge.
    void nudgeHyperEdgeSegments(size_t dimension, unsigned int& versionNumber)
    {
        // For each hyperedge...
        for (JunctionSet::iterator curr = hyperEdgeTreeRoots.begin();
                curr != hyperEdgeTreeRoots.end(); ++curr)
        {
            ++debug_count;
            versionNumber = dimension * 10000;
            versionNumber += debug_count * 1000;

            // Calculate the balance for each shift segment.
            ShiftSegmentList& segmentList = rootShiftSegments[*curr];
            for (ShiftSegmentList::iterator currSeg = segmentList.begin();
                    currSeg != segmentList.end(); )
            {
                HyperEdgeShiftSegment *segment =
                        dynamic_cast<HyperEdgeShiftSegment *> (*currSeg);
                segment->setBalanceCount();

                ++currSeg;
            }

            //segmentList.sort(CmpHyperEdgeSegmentDirOrder);

            bool change = false;
            ShiftSegmentList::iterator currSeg = segmentList.begin();
            while (currSeg != segmentList.end())
            {
                // While we haven't considered every segment...

                HyperEdgeShiftSegment *segment =
                        dynamic_cast<HyperEdgeShiftSegment *> (*currSeg);

                if ( ! segment->settled() )
                {
                    // The segment is not settled, so move it to the next
                    // ideal position and then merge it with overlapping
                    // segments.  Note, the merged segment will have a new
                    // balance value calculated for it.
                    segment->adjustPosition();
                    outputHyperEdgesToSVG(++versionNumber, segment);
                    mergeOverlappingSegments(segmentList);
                    change = true;
                }

                if (change)
                {
                    // We made a change, so start again from the beginning
                    // of the list of segments.
                    change = false;
                    currSeg = segmentList.begin();
                }
                else
                {
                    // Consider the next segment.
                    ++currSeg;
                }
            }
        }
    }

    // Write the paths from an improved hyperedgetree object back as routes
    // to the component connectors that form the hyperedge.
    void writeHyperEdgeSegmentsBackToConnPaths(void)
    {
        // Write segments in two passes.  The first to clear the existing
        // connector routes and the second to build and set new routes.
        for (size_t pass = 0; pass < 2; ++pass)
        {
            for (JunctionSet::iterator curr = hyperEdgeTreeRoots.begin();
                    curr != hyperEdgeTreeRoots.end(); ++curr)
            {
                HyperEdgeTreeNode *node = hyperEdgeTreeJunctions[*curr];

                node->writeEdgesToConns(NULL, pass);
            }
        }
    }

    // Output the hyperedge tree to an SVG file, optionally highlighting
    // a segment of interest (usually the segment being moved).
    void outputHyperEdgesToSVG(unsigned int pass,
            HyperEdgeShiftSegment *activeSegment = NULL)
    {
#ifndef HYPEREDGE_DEBUG
        return;
#endif

        // Reasonable initial limit for diagram bounds.
        const double LIMIT = 100000000;

        char filename[50];
        sprintf(filename, "DEBUG/hyperedges-%05u.svg", pass);
        FILE *fp = fopen(filename, "w");

        double minX = LIMIT;
        double minY = LIMIT;
        double maxX = -LIMIT;
        double maxY = -LIMIT;

        VertInf *curr = router->vertices.connsBegin();
        while (curr)
        {
            Point p = curr->point;

            if (p.x > -LIMIT)
            {
                minX = std::min(minX, p.x);
            }
            if (p.x < LIMIT)
            {
                maxX = std::max(maxX, p.x);
            }
            if (p.y > -LIMIT)
            {
                minY = std::min(minY, p.y);
            }
            if (p.y < LIMIT)
            {
                maxY = std::max(maxY, p.y);
            }
            curr = curr->lstNext;
        }
        minX -= 50;
        minY -= 50;
        maxX += 50;
        maxY += 50;


        fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(fp, "<svg xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\" width=\"100%%\" height=\"100%%\" viewBox=\"%g %g %g %g\">\n", minX, minY, maxX - minX, maxY - minY);

        fprintf(fp, "<g inkscape:groupmode=\"layer\" "
                "inkscape:label=\"ShapesRect\">\n");
        ObstacleList::iterator obstacleIt = router->m_obstacles.begin();
        while (obstacleIt != router->m_obstacles.end())
        {
            Obstacle *obstacle = *obstacleIt;
            bool isShape = (NULL != dynamic_cast<ShapeRef *> (obstacle));

            if ( ! isShape )
            {
                // Don't output obstacles here, for now.
                ++obstacleIt;
                continue;
            }

            Box bBox = obstacle->polygon().offsetBoundingBox(0.0);

            fprintf(fp, "<rect id=\"rect-%u\" x=\"%g\" y=\"%g\" width=\"%g\" "
                    "height=\"%g\" style=\"stroke-width: 1px; stroke: %s; "
                    "fill: blue; fill-opacity: 0.3;\" />\n",
                    obstacle->id(), bBox.min.x, bBox.min.y,
                    bBox.max.x - bBox.min.x, bBox.max.y - bBox.min.y,
                    (isShape) ? "blue" : "red");
            ++obstacleIt;
        }
        fprintf(fp, "</g>\n");

        fprintf(fp, "<g inkscape:groupmode=\"layer\" "
                "inkscape:label=\"HyperEdge-%u\">\n", pass);
        if (activeSegment)
        {
            fprintf(fp, "<path d=\"M %g %g L %g %g\" "
                "style=\"fill: none; stroke: %s; stroke-width: 12px; "
                "stroke-opacity: 0.5;\" />\n",
                activeSegment->lowPoint().x, activeSegment->lowPoint().y,
                activeSegment->highPoint().x, activeSegment->highPoint().y,
                activeSegment->settled() ? "red" : "orange");
        }

        for (JunctionSet::iterator curr = hyperEdgeTreeRoots.begin();
                curr != hyperEdgeTreeRoots.end(); ++curr)
        {
            HyperEdgeTreeNode *node = hyperEdgeTreeJunctions[*curr];

            node->outputEdgesExcept(fp, NULL);
        }
        fprintf(fp, "</g>\n");
        fprintf(fp, "</svg>\n");

        fclose(fp);
    }

    // Given a junction, this method follows the attached connectors and
    // junctions to determine a hyperedge and returns the set of vertices
    // representing its endpoints.
    void getEndpoints(JunctionRef *junction, JunctionRef *ignore,
            std::set<VertInf *>& endpoints)
    {
        for (std::set<ConnEnd *>::iterator curr =
                junction->m_following_conns.begin();
                curr != junction->m_following_conns.end(); ++curr)
        {
            ConnEnd *connEnd = *curr;
            COLA_ASSERT(connEnd->m_conn_ref != NULL);
            ConnRef *connRef = connEnd->m_conn_ref;
            std::pair<Obstacle *, Obstacle *> anchors =
                    connRef->endpointAnchors();

            JunctionRef *junction1 =
                    dynamic_cast<JunctionRef *> (anchors.first);
            if (junction1)
            {
                if (junction1 != junction && junction1 != ignore)
                {
                    getEndpoints(junction1, junction, endpoints);
                }
            }
            else
            {
                endpoints.insert(connRef->m_src_vert);
            }

            JunctionRef *junction2 =
                    dynamic_cast<JunctionRef *> (anchors.second);
            if (junction2)
            {
                if (junction2 != junction && junction2 != ignore)
                {
                    getEndpoints(junction2, junction, endpoints);
                }
            }
            else
            {
                endpoints.insert(connRef->m_dst_vert);
            }
        }
    }

    // Execute local improvement process.
    void execute(void)
    {
        // Build HyperEdge trees.
        ConnRefList::iterator connRefIt = router->connRefs.begin();
        while (connRefIt != router->connRefs.end())
        {
            ConnRef *connRef = *connRefIt;
            JunctionRef *jFront = NULL;
            JunctionRef *jBack = NULL;

            if (connRef->m_src_connend)
            {
                jFront = connRef->m_src_connend->junction();
            }

            if (connRef->m_dst_connend)
            {
                jBack = connRef->m_dst_connend->junction();
            }

            if (jFront && jFront->positionFixed())
            {
                jFront = NULL;
            }

            if (jBack && jBack->positionFixed())
            {
                jBack = NULL;
            }

            if ( ! jFront && ! jBack )
            {
                ++connRefIt;
                continue;
            }

            bool seenFront = (hyperEdgeTreeJunctions.find(jFront) != 
                    hyperEdgeTreeJunctions.end());
            bool seenBack = (hyperEdgeTreeJunctions.find(jBack) !=
                    hyperEdgeTreeJunctions.end());
                
            HyperEdgeTreeNode *nodeFront = NULL;
            HyperEdgeTreeNode *nodeBack = NULL;

            if (jFront)
            {
                if ( ! seenFront)
                {
                    nodeFront = new HyperEdgeTreeNode();
                    nodeFront->point = jFront->position();
                    nodeFront->junction = jFront;

                    hyperEdgeTreeJunctions[jFront] = nodeFront;
                }
                else
                {
                    nodeFront = hyperEdgeTreeJunctions[jFront];
                }
            }
            else
            {
                nodeFront = new HyperEdgeTreeNode();
            }

            if (jBack)
            {
                if ( ! seenBack)
                {
                    nodeBack = new HyperEdgeTreeNode();
                    nodeBack->point = jBack->position();
                    nodeBack->junction = jBack;

                    hyperEdgeTreeJunctions[jBack] = nodeBack;
                }
                else
                {
                    nodeBack = hyperEdgeTreeJunctions[jBack];
                }
            }
            else
            {
                nodeBack = new HyperEdgeTreeNode();
            }

            PolyLine& route = connRef->displayRoute();
            HyperEdgeTreeNode *prev = NULL;
            for (unsigned int i = 1; i < route.size(); ++i)
            {
                HyperEdgeTreeNode *node;
                if (i + 1 == route.size())
                {
                    node = nodeBack;
                }
                else
                {
                    node = new HyperEdgeTreeNode();
                }
                node->point = route.at(i);
                if (i == 1)
                {
                    prev = nodeFront;
                    nodeFront->point = route.at(0);
                }
                new HyperEdgeTreeEdge(prev, node, connRef);
                prev = node;
            }
            ++connRefIt;
        }

        // Make a list that contains a single junction from each tree.
        for (JunctionHyperEdgeTreeNodeMap::iterator curr = 
                hyperEdgeTreeJunctions.begin(); 
                curr != hyperEdgeTreeJunctions.end(); ++curr)
        {
            HyperEdgeTreeNode *node = curr->second;
            hyperEdgeTreeRoots.insert(node->junction); 
        }
        for (JunctionSet::iterator curr = hyperEdgeTreeRoots.begin();
                curr != hyperEdgeTreeRoots.end(); ++curr)
        {
            HyperEdgeTreeNode *node = hyperEdgeTreeJunctions[*curr];
            node->removeOtherJunctionsFrom(NULL, hyperEdgeTreeRoots);
        }

        router->timers.Register(tmHyperedgeImprove, timerStart);

        // Debug output.
        unsigned int versionNumber = 1;
        outputHyperEdgesToSVG(versionNumber);

        // Move junctions to divergence points.
        moveJunctionsAlongCommonEdges();

        // Debug output.
        outputHyperEdgesToSVG(++versionNumber);

        for (size_t count = 0; count < 4; ++count)
        {
            size_t dimension = count % 2;

            // Set a version number for debug output.
            versionNumber = 100 * (dimension + 1);

            // Build shift segments.
            buildHyperEdgeSegments(dimension);
            // Calculate channel information for this dimension.
            buildOrthogonalChannelInfo(router, dimension, allShiftSegments);
            // Nudge hyperedge segments to locally improve the route.
            nudgeHyperEdgeSegments(dimension, versionNumber);
            // Remove resulting zero length edges.
            removeZeroLengthEdges();
            // Move junctions to divergence points.
            moveJunctionsAlongCommonEdges();
            // Debug output.
            outputHyperEdgesToSVG(++versionNumber);

            // Clean up shift segments.
            for (JunctionSet::iterator curr = hyperEdgeTreeRoots.begin();
                    curr != hyperEdgeTreeRoots.end(); ++curr)
            {
                ShiftSegmentList& segmentList = rootShiftSegments[*curr];
                for_each(segmentList.begin(), segmentList.end(),
                        delete_object());
            }
            rootShiftSegments.clear();
            allShiftSegments.clear();
        }

        // Write back final recommended positions to junctions.
        for (JunctionHyperEdgeTreeNodeMap::iterator curr = 
                hyperEdgeTreeJunctions.begin(); 
                curr != hyperEdgeTreeJunctions.end(); ++curr)
        {
            HyperEdgeTreeNode *node = curr->second;

            node->junction->setRecommendedPosition(node->point);
        }

        // Write paths from the hyperedge tree back into individual
        // connector routes.
        writeHyperEdgeSegmentsBackToConnPaths();

        // Free HyperEdgeTree structure.
        for (JunctionSet::iterator curr = hyperEdgeTreeRoots.begin();
                curr != hyperEdgeTreeRoots.end(); ++curr)
        {
            HyperEdgeTreeNode *node = hyperEdgeTreeJunctions[*curr];

            node->deleteEdgesExcept(NULL);
            delete node;
        }
        router->timers.Stop();
    }

    Router *router;
    JunctionHyperEdgeTreeNodeMap hyperEdgeTreeJunctions;
    JunctionSet hyperEdgeTreeRoots;
    RootSegmentsMap rootShiftSegments;
    ShiftSegmentList allShiftSegments;
    int debug_count;
};


// Convenience function to create a ImproveHyperEdges object and to
// execute the process.
void improveHyperedgeRoutes(Router *router)
{
    ImproveHyperEdges method(router);
    method.execute();
}


}
