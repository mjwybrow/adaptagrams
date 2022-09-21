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


#include <cstdlib>
#include <cfloat>
#include <cmath>
#include <set>
#include <list>
#include <algorithm>

#ifdef ORTHOG_TOPOLOGY_DEBUG
#include <sstream>
#include <iomanip>
#endif

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

#include "libtopology/orthogonal_topology.h"
#include "libvpsc/solve_VPSC.h"
#include "libvpsc/variable.h"
#include "libvpsc/constraint.h"

//#define ORTHOG_TOPOLOGY_DEBUG

#define CONSTRAIN_CHECKPOINTS  false


namespace topology {

using namespace Avoid;

static const double CHANNEL_MAX = 100000000;

// Used to sort points when merging LayoutEdgeSegments.
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


class LayoutEdgeSegmentSeparation
{
    public:
    double distance;
    vpsc::Variable *var1;
    vpsc::Variable *var2;
    ConnRef *connRef;

    bool operator<(const LayoutEdgeSegmentSeparation& rhs) const
    {
        COLA_ASSERT((var1 != rhs.var1) || (var2 != rhs.var2));
        if (distance == rhs.distance)
        {
            return (var1 < rhs.var1) || (var2 < rhs.var2);
        }
        return distance < rhs.distance;
    }
};
typedef std::set<LayoutEdgeSegmentSeparation> LayoutEdgeSegmentSeparations;

class LayoutEdgeSegment : public ShiftSegment
{
    public:
        // For shiftable segments.
        LayoutEdgeSegment(ConnRef *conn, const size_t low, const size_t high,
                bool isSBend, bool isZBend, const size_t dim, double minLim,
                double maxLim)
            : ShiftSegment(dim),
              connRef(conn),
              variable(nullptr),
              fixed(false),
              finalSegment(false),
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
        LayoutEdgeSegment(ConnRef *conn, const size_t low, const size_t high,
                const size_t dim)
            : ShiftSegment(dim),
              connRef(conn),
              variable(nullptr),
              fixed(true),
              finalSegment(false),
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
        virtual ~LayoutEdgeSegment()
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
        void createSolverVariable(void)
        {
            int varID = connRef->id();
            double varPos = lowPoint()[dimension];

            // Use a low weight for segments.
            // (Constraints will hold them in place on shapes.)
            double weight = 0.001;

            variable = new vpsc::Variable(varID, varPos, weight);
        }
        bool overlapsWith(const ShiftSegment *rhs, const size_t dim) const
        {
            COLA_UNUSED(rhs);
            COLA_UNUSED(dim);

            return false;
        }
        void updatePositionsFromSolver(void)
        {
            if (fixed)
            {
                return;
            }
            double newPos = variable->finalPosition;
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

        ConnRef *connRef;
        vpsc::Variable *variable;
        std::vector<size_t> indexes;
        bool fixed;
        bool finalSegment;
        std::list<int> endsInShapeIndexes;
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
typedef std::list<LayoutEdgeSegment *> LayoutEdgeSegmentList;

class LayoutObstacle
{
    public:
    LayoutObstacle()
        : dimension(0),
          obstacle(nullptr),
          variable(nullptr)
    {
    }
    LayoutObstacle(ShapeRef *shape, const size_t dim)
        : dimension(dim),
          obstacle(shape),
          variable(nullptr)
    {
        // Take the bounds of the shape
        Avoid::Box bBox = shape->routingBox();
        min = bBox.min;
        max = bBox.max;
    }
    LayoutObstacle(JunctionRef *junction, const size_t dim)
        : dimension(dim),
          obstacle(junction),
          variable(nullptr)
    {
        // Don't nudge segments attached to junctions,
        // so just use the junction position here.
        Point pos = junction->position();

        min = pos;
        max = pos;
    }
    ShapeRef *shape(void) const
    {
        return (dynamic_cast<ShapeRef *> (obstacle));
    }
    JunctionRef *junction(void) const
    {
        return (dynamic_cast<JunctionRef *> (obstacle));
    }
    double halfSizeInDim(const size_t dim)
    {
        return (max[dim] - min[dim]) / 2;
    }
    double centreX(void) const
    {
        return min.x + ((max.x - min.x) / 2);
    }
    double centreY(void) const
    {
        return min.y + ((max.y - min.y) / 2);
    }
    Point centre(void) const
    {
        return Point(centreX(), centreY());
    }
    Point shapeCentre(void) const
    {
        COLA_ASSERT(shape());
        return shape()->position();
    }
    void createSolverVariable(void)
    {
        int varID = obstacle->id();

        // Use higher weight than alignment variables (0.001) so we don't
        // get moved by them.
        double weight = 1.0;

        variable = new vpsc::Variable(varID, centre()[dimension], weight);
    }
    void updatePositionsFromSolver(void)
    {
        double varVal = variable->finalPosition;
#ifdef ORTHOG_TOPOLOGY_DEBUG
        fprintf(stderr, "Obstacle Pos: %d, %g\n", obstacle->id(), varVal);
#endif
        ShapeRef *shapeRef = shape();
        if (shapeRef)
        {
            Point newPos = shapeRef->position();
            newPos[dimension] = varVal;
            shapeRef->setCentrePos(newPos);
        }
    }

    size_t dimension;

    Point min;
    Point max;

    Obstacle *obstacle;

    vpsc::Variable *variable;
};
typedef std::vector<LayoutObstacle> LayoutObstacleVector;

static const int SIDE_RIGHT = -1;
static const int SIDE_LEFT  = 1;

class LayoutNode : public Node
{
    public:
    LayoutNode(LayoutObstacle *layObstacle, const double position)
        : Node(layObstacle->obstacle, position),
          layoutObstacle(layObstacle),
          side(0)
    {
    }

    LayoutObstacle *layoutObstacle;
    int side;
};



enum ScanVisDirFlag {
    VisDirNone  = 0,
    VisDirUp    = 1,
    VisDirDown  = 2
};
typedef unsigned int ScanVisDirFlags;


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
        if ((vert->id == rhs.vert->id) && (vert->id == Avoid::dummyOrthogID))
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


// Temporary structure used to store the possible horizontal visibility
// lines arising from the vertical sweep.
class LineSegment
{
public:
    LineSegment(const double& b, const double& f, const double& p,
            bool ss = false, VertInf *bvi = nullptr, VertInf *fvi = nullptr)
        : begin(b),
          finish(f),
          pos(p),
          shapeSide(ss)
    {
        COLA_UNUSED(bvi);
        COLA_UNUSED(fvi);
        COLA_ASSERT(begin < finish);
    }
    LineSegment(const double& bf, const double& p, VertInf *bfvi = nullptr)
        : begin(bf),
          finish(bf),
          pos(p),
          shapeSide(false)
    {
        COLA_UNUSED(bfvi);
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

    double begin;
    double finish;
    double pos;

    // XXX shapeSide is unused and could possibly be removed?
    bool shapeSide;

private:
	// MSVC wants to generate the assignment operator and the default
	// constructor, but fails.  Therefore we declare them private and
	// don't implement them.
    LineSegment & operator=(LineSegment const &);
    LineSegment();
};

typedef std::list<LineSegment> SegmentList;


static bool insideLayoutObstacleBounds(const Point& point,
        const LayoutObstacle& anchor)
{
    Point zero(0, 0);
    if ((anchor.min == zero) && (anchor.max == zero))
    {
        // We can't be inside the invalid rectangle.
        return false;
    }

    for (size_t i = 0; i < 2; ++i)
    {
        if (point[i] < anchor.min[i])
        {
            return false;
        }
        if (point[i] > anchor.max[i])
        {
            return false;
        }
    }
    return true;
}


// When processing the scanline, it may be possible that shapes share edges,
// or a connector segment shares a position with a shape edge.  Thus, we
// order objects in the scanline list so that shape right sides come before
// connector segments which come before shape left sides at the same position.
struct LayoutScanlineCmpNodePos
{
    bool operator()(const Node* u, const Node* v) const
    {
        if (fabs(u->pos - v->pos) > 0.000000001)
        {
            return u->pos < v->pos;
        }

        // Use the pointers to the base objects to differentiate them.
        void *up = (u->v) ? (void *) u->v :
                ((u->c) ? (void *) u->c : (void *) u->ss);
        void *vp = (v->v) ? (void *) v->v :
                ((v->c) ? (void *) v->c : (void *) v->ss);

        const LayoutNode *layoutNodeU = dynamic_cast<const LayoutNode *> (u);
        const LayoutNode *layoutNodeV = dynamic_cast<const LayoutNode *> (v);

        // For things occurring at the same position on the scanline,
        // order shape SIDE_RIGHT (-1) < segments (0) < shape SIDE_LEFT (1).
        int valU = (layoutNodeU) ? layoutNodeU->side : 0;
        int valV = (layoutNodeV) ? layoutNodeV->side : 0;
        if (valU != valV)
        {
            return valU < valV;
        }

        return up < vp;
    }
};

typedef std::set<Node*,LayoutScanlineCmpNodePos> LayoutScanlineNodeSet;

// Processes sweep events to determine orthogonal constraints affecting each
// shape and line segment.  This is done in three passes to cater for events
// that occur at the same position.  First pass adds all obejcts at position
// to the scanline.  The second pass processes the scanline and generates
// constraints.  The third pass removes any objects ending at that position
// from the scanline.
static void processLayoutConstraintEvent(LayoutScanlineNodeSet& scanline,
        Event *e, size_t dim,  LayoutObstacleVector& obstacleVector,
        LineReps *lineReps, vpsc::Constraints& cs,
        LayoutEdgeSegmentSeparations& less, unsigned int pass,
        const double moveLimit, const double idealNudgeDist)
{
    COLA_UNUSED(lineReps);

    //size_t altDim = (dim + 1) % 2;
    Node *v = e->v;

    bool Equality = true;
    bool NonEquality = false;

    // Scanline housekeeping.
    if ( (pass == 1) && ((e->type == Open) || (e->type == SegOpen)) )
    {
        // Opening events.  First pass.

        std::pair<LayoutScanlineNodeSet::iterator, bool> result =
                scanline.insert(v);
        v->iter = result.first;
        COLA_ASSERT(result.second);

        LayoutScanlineNodeSet::iterator it = v->iter;
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
    else if ( (pass == 3) &&  ((e->type == SegClose) || (e->type == Close)) )
    {
        // Closing events.  Third pass.

        // Clean up neighbour pointers.
        Node *l = v->firstAbove, *r = v->firstBelow;
        if (l != nullptr)
        {
            l->firstBelow = v->firstBelow;
        }
        if (r != nullptr)
        {
            r->firstAbove = v->firstAbove;
        }

        size_t result;
        result = scanline.erase(v);
        COLA_ASSERT(result == 1);
        delete v;
    }

    if (pass != 2)
    {
        // Do remaining stuff only if we are in the second pass.
        return;
    }

    if ((e->type == SegClose) || (e->type == SegOpen))
    {
        // This is an edge segment from a connector.

        // Constraint edge segments that end in shapes to be fixed to those
        // shapes in the shift dimension.
        LayoutEdgeSegment *les = dynamic_cast<LayoutEdgeSegment *> (v->ss);
        for (std::list<int>::iterator it = les->endsInShapeIndexes.begin();
                it != les->endsInShapeIndexes.end(); ++it)
        {
            assert(les->lowPoint()[dim] == les->highPoint()[dim]);
            Point endpoint = (e->type == SegOpen) ? les->lowPoint() :
                    les->highPoint();
            if (insideLayoutObstacleBounds(endpoint, obstacleVector[*it]))
            {
                double sepDist = obstacleVector[*it].centre()[dim] -
                        les->lowPoint()[dim];
                vpsc::Constraint *constraint = new vpsc::Constraint(
                        les->variable, obstacleVector[*it].variable,
                        sepDist, Equality);
                cs.push_back(constraint);
            }

            /*
            if (lineReps)
            {
                LineRep line;
                line.begin[dim] = constraint->left->desiredPosition;
                line.end[dim] = constraint->right->desiredPosition;
                line.begin[altDim] = e->pos;
                line.end[altDim] = e->pos;
                lineReps->push_back(line);
            }
            */
        }

        // Constrain edge segment to separate from left neighbouring shape.
        Node *beforeV = v->firstAbove;
        LayoutNode *beforeLayoutV = dynamic_cast<LayoutNode *> (beforeV);
        if (beforeLayoutV && (beforeLayoutV->side == SIDE_RIGHT))
        {
            double sepDist =
                    beforeLayoutV->layoutObstacle->halfSizeInDim(dim);

            COLA_ASSERT(sepDist >= 0);
            vpsc::Constraint *constraint = new vpsc::Constraint(
                    beforeLayoutV->layoutObstacle->variable,
                    les->variable, sepDist, NonEquality);
            cs.push_back(constraint);

        }

        // Constrain edge segment to separate from right neighbouring shape.
        Node *afterV = v->firstBelow;
        LayoutNode *afterLayoutV = dynamic_cast<LayoutNode *> (afterV);
        if (afterLayoutV && (afterLayoutV->side == SIDE_LEFT))
        {
            double sepDist =
                    afterLayoutV->layoutObstacle->halfSizeInDim(dim);

            COLA_ASSERT(sepDist >= 0);
            vpsc::Constraint *constraint = new vpsc::Constraint(
                    les->variable, afterLayoutV->layoutObstacle->variable,
                    sepDist, NonEquality);
            cs.push_back(constraint);

        }

        // Constrain edge segment to separate from left neighbouring segment.
        if (beforeV && beforeV->ss)
        {
            LayoutEdgeSegment *beforeLes =
                dynamic_cast<LayoutEdgeSegment *> (beforeV->ss);
            assert(beforeLes);

            if (les->connRef != beforeLes->connRef)
            {
                // If not segments from same edge.
                if (CONSTRAIN_CHECKPOINTS && les->containsCheckpoint &&
                        beforeLes->containsCheckpoint)
                {
                    // Both segments are passing a checkpoint.
                    // Constrain them together.
                    double sepDist =
                            (les->lowPoint()[les->dimension] -
                                    beforeLes->lowPoint()[les->dimension]);

                    vpsc::Constraint *constraint = new vpsc::Constraint(
                            beforeLes->variable, les->variable,
                            sepDist, Equality);
                    cs.push_back(constraint);
                }
                else
                {
                    // Normal segments.  Constrain them with Nudging distance
                    // or the current distance apart, if this is smalled.
                    double sepDist = std::min(idealNudgeDist,
                            (les->lowPoint()[les->dimension] -
                             beforeLes->lowPoint()[les->dimension]));
                    sepDist = std::max(sepDist, 0.0);
                    COLA_ASSERT(sepDist >= 0);
                    vpsc::Constraint *constraint = new vpsc::Constraint(
                            beforeLes->variable, les->variable,
                            sepDist, NonEquality);
                    cs.push_back(constraint);
                }
            }
        }

        // Constrain edge segment to separate from right neighbouring segment.
        if (afterV && afterV->ss)
        {
            LayoutEdgeSegment *afterLes =
                dynamic_cast<LayoutEdgeSegment *> (afterV->ss);
            assert(afterLes);

            if (les->connRef != afterLes->connRef)
            {
                // If not segments from same edge.
                if (CONSTRAIN_CHECKPOINTS && les->containsCheckpoint &&
                        afterLes->containsCheckpoint)
                {
                    // Both segments are passing a checkpoint.a
                    // Constrain them together.
                    double sepDist =
                            (afterLes->lowPoint()[les->dimension] -
                             les->lowPoint()[les->dimension]);

                    vpsc::Constraint *constraint = new vpsc::Constraint(
                            les->variable, afterLes->variable,
                            sepDist, Equality);
                    cs.push_back(constraint);
                }
                else
                {
                    // Normal segments.  Constrain them with Nudging distance
                    // or the current distance apart, if this is smalled.
                    double sepDist = std::min(idealNudgeDist,
                            (afterLes->lowPoint()[les->dimension] -
                                    les->lowPoint()[les->dimension]));
                    sepDist = std::max(sepDist, 0.0);
                    COLA_ASSERT(sepDist >= 0);

                    vpsc::Constraint *constraint = new vpsc::Constraint(
                            les->variable, afterLes->variable,
                            sepDist, NonEquality);
                    cs.push_back(constraint);
                }
            }
        }
    }

    if ((e->type == Close) || (e->type == Open))
    {
        // This is a left or right shape side, beginning or ending.

        LayoutNode *ln = dynamic_cast<LayoutNode *> (v);

        if (ln->side == SIDE_LEFT)
        {
            Node *beforeV = v->firstAbove;
            LayoutNode *beforeLayoutV = dynamic_cast<LayoutNode *> (beforeV);
            LayoutEdgeSegment *beforeLes = (beforeV == nullptr) ? nullptr :
                    dynamic_cast<LayoutEdgeSegment *> (beforeV->ss);

            if (beforeLayoutV && (beforeLayoutV->side == SIDE_RIGHT))
            {
                // Constrain this left shape side to be separated from
                // a neighbouring shape right side to the left.
                double sepDist =
                        ln->layoutObstacle->halfSizeInDim(dim) +
                        beforeLayoutV->layoutObstacle->halfSizeInDim(dim);
                COLA_ASSERT(sepDist >= 0);

                vpsc::Constraint *constraint = new vpsc::Constraint(
                        beforeLayoutV->layoutObstacle->variable,
                        ln->layoutObstacle->variable, sepDist, NonEquality);
                cs.push_back(constraint);

            }
            else if (beforeLes)
            {
                // Constrain this left shape side to be separated from
                // a neighbouring edge segment to the left.
                double sepDist =
                        ln->layoutObstacle->halfSizeInDim(dim);
                COLA_ASSERT(sepDist >= 0);

                vpsc::Constraint *constraint = new vpsc::Constraint(
                        beforeLes->variable, ln->layoutObstacle->variable,
                        sepDist, NonEquality);
                cs.push_back(constraint);
            }
        }
        else if (ln->side == SIDE_RIGHT)
        {
            Node *afterV = v->firstBelow;
            LayoutNode *afterLayoutV = dynamic_cast<LayoutNode *> (afterV);
            LayoutEdgeSegment *afterLes = (afterV == nullptr) ? nullptr :
                    dynamic_cast<LayoutEdgeSegment *> (afterV->ss);

            if (afterLayoutV && (afterLayoutV->side == SIDE_LEFT))
            {
                // Constrain this right shape side to be separated from
                // a neighbouring shape left side to the right.
                double sepDist =
                        ln->layoutObstacle->halfSizeInDim(dim) +
                        afterLayoutV->layoutObstacle->halfSizeInDim(dim);
                COLA_ASSERT(sepDist >= 0);

                vpsc::Constraint *constraint = new vpsc::Constraint(
                        ln->layoutObstacle->variable,
                        afterLayoutV->layoutObstacle->variable,
                        sepDist, NonEquality);
                cs.push_back(constraint);

            }
            else if (afterLes)
            {
                // Constrain this right shape side to be separated from
                // a neighbouring edge segment to the right.
                double sepDist =
                        ln->layoutObstacle->halfSizeInDim(dim);
                COLA_ASSERT(sepDist >= 0);

                vpsc::Constraint *constraint = new vpsc::Constraint(
                        ln->layoutObstacle->variable, afterLes->variable,
                        sepDist, NonEquality);
                cs.push_back(constraint);
            }
        }
    }

    // Compute all possible shifts that can be made from a segment ending,
    // to straighten it to a sibling segment.
    if (e->type == SegClose)
    {
        // Possible points of movement.
        LayoutEdgeSegment *les = dynamic_cast<LayoutEdgeSegment *> (v->ss);
        for(LayoutScanlineNodeSet::iterator it = scanline.begin();
                it != scanline.end(); ++it)
        {
            size_t altDim = (les->dimension + 1) % 2;
            Node *u = *it;
            LayoutEdgeSegment *otherLes =
                    dynamic_cast<LayoutEdgeSegment *> (u->ss);
            if (otherLes && (les->connRef == otherLes->connRef) &&
                    !(les->containsCheckpoint) &&
                    !(otherLes->containsCheckpoint) &&
                    (les->highPoint()[altDim] == otherLes->lowPoint()[altDim]))
            {
                // We consider moving segments from the same connector
                // separated by a single segment, where neither contains a
                // checkpoint.
                LayoutEdgeSegmentSeparation newLess;
                newLess.distance = fabs(les->highPoint()[les->dimension] -
                        otherLes->lowPoint()[les->dimension]);
                newLess.var1 = les->variable;
                newLess.var2 = otherLes->variable;
                newLess.connRef = les->connRef;
                if (newLess.distance < moveLimit)
                {
                    less.insert(newLess);
                }

                /*
                if (lineReps)
                {
                    LineRep line;
                    line.begin[dim] = les->highPoint()[les->dimension];
                    line.end[dim] = otherLes->highPoint()[les->dimension];
                    line.begin[altDim] = e->pos;
                    line.end[altDim] = e->pos;
                    lineReps->push_back(line);
                }
                */
            }
        }
    }
}

// This class is used to record and update endpoints of segments in the same
// dimension that shapes are being moved in the optimisation step.  this is
// done only for the first and last endpoints of connectors for the shape they
// terminate inside of.
class EndpointAnchorInMoveDir
{
    public:
        EndpointAnchorInMoveDir(size_t dimension, ConnRef *connRef,
                size_t endptIndex, size_t obstacleIndex,
                double obstacleOffset)
            : dimension(dimension),
              connRef(connRef),
              endptIndex(endptIndex),
              obstacleIndex(obstacleIndex),
              obstacleOffset(obstacleOffset)
        {
        }

        void updatePosition(const LayoutObstacleVector& obstacleVector)
        {
            Point obstacleCentre = obstacleVector[obstacleIndex].shapeCentre();
            connRef->displayRoute().ps[endptIndex][dimension] =
                    obstacleCentre[dimension] + obstacleOffset;
        }

        size_t dimension;
        ConnRef *connRef;
        size_t endptIndex;
        size_t obstacleIndex;
        double obstacleOffset;
};
typedef std::list<EndpointAnchorInMoveDir> EndpointAnchorList;


static void buildOrthogonalLayoutSegments(Router *router,
        const size_t dim, LayoutEdgeSegmentList& segmentList,
        LayoutObstacleVector& obstacleVector,
        EndpointAnchorList& extraTerminalsList)
{
    // We can handle bends that occur as a result of checkpoints as intentional.
    // In this case we don't try to remove those bends.  Also, we keep a fixed
    // (equality) spacing between nudges paths at the checkpoint, so part of
    // the shared path doesn't get ripped away.
    const bool preserveCheckpointBends = true;

    const size_t n = router->m_obstacles.size();

    obstacleVector = LayoutObstacleVector(n);

    // If we're going to nudge final segments, then cache the shape
    // rectangles to save us rebuilding them multiple times.
    ObstacleList::iterator obstacleIt = router->m_obstacles.begin();
    for (unsigned i = 0; i < n; i++)
    {
        ShapeRef *shape = dynamic_cast<ShapeRef *> (*obstacleIt);
        JunctionRef *junction = dynamic_cast<JunctionRef *> (*obstacleIt);
        if (shape)
        {
            obstacleVector[i] = LayoutObstacle(shape, dim);
        }
        else if (junction)
        {
            obstacleVector[i] = LayoutObstacle(junction, dim);
        }
        ++obstacleIt;
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
        Avoid::Polygon& displayRoute = (*curr)->displayRoute();
        bool routeHasCheckpointInfo = !displayRoute.checkpointsOnRoute.empty();
        // Determine all line segments that we are interested in shifting.
        // We don't consider the first or last segment of a path.
        for (size_t i = 1; i < displayRoute.size(); ++i)
        {
            bool containsCheckpoint = false;
            if (preserveCheckpointBends && routeHasCheckpointInfo)
            {
                containsCheckpoint =
                        (displayRoute.checkpointsOnSegment(i - 1).size() > 0);
            }

            if (displayRoute.ps[i - 1][dim] == displayRoute.ps[i][dim])
            {
                // It's a segment in the dimension we are processing,
                size_t indexLow = i - 1;
                size_t indexHigh = i;
                COLA_ASSERT(displayRoute.ps[i - 1][altDim] !=
                        displayRoute.ps[i][altDim]);
                if (displayRoute.ps[i - 1][altDim] >
                        displayRoute.ps[i][altDim])
                {
                    indexLow = i;
                    indexHigh = i - 1;
                }

                double thisPos = displayRoute.ps[i][dim];

                if ((i == 1) || ((i + 1) == displayRoute.size()))
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

                    std::list<int> endsInShapeIndexes;
                    if (first || last)
                    {
                    // Also limit their movement to the edges of the
                    // shapes they begin or end within.
                    for (size_t k = 0; k < obstacleVector.size(); ++k)
                    {
                        double shapeMin = obstacleVector[k].min[dim];
                        double shapeMax = obstacleVector[k].max[dim];
                        if (first && insideLayoutObstacleBounds(displayRoute.ps[i - 1],
                                    obstacleVector[k]))
                        {
                            minLim = std::max(minLim, shapeMin);
                            maxLim = std::min(maxLim, shapeMax);
                            endsInShapeIndexes.push_back(k);
                            continue;
                        }
                        if (last && insideLayoutObstacleBounds(displayRoute.ps[i],
                                    obstacleVector[k]))
                        {
                            minLim = std::max(minLim, shapeMin);
                            maxLim = std::min(maxLim, shapeMax);
                            endsInShapeIndexes.push_back(k);
                        }
                    }
                    }

                    // Shiftable.
                    LayoutEdgeSegment *segment = new LayoutEdgeSegment(
                            *curr, indexLow, indexHigh, false, false, dim,
                            minLim, maxLim);
                    segment->endsInShapeIndexes = endsInShapeIndexes;
                    segment->containsCheckpoint = containsCheckpoint;
                    segment->finalSegment = true;
                    segmentList.push_back(segment);
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

                LayoutEdgeSegment *nss = new LayoutEdgeSegment(*curr,
                        indexLow, indexHigh, isSBend, isZBend, dim,
                        minLim, maxLim);
                nss->containsCheckpoint = containsCheckpoint;
                segmentList.push_back(nss);
            }
            else if (displayRoute.ps[i - 1][altDim] ==
                    displayRoute.ps[i][altDim])
            {
                // It's a segment in the dimension opposite to the one we
                // are processing.  For these we need to record their position
                // relative to the shape they terminate in, so if that shape
                // is moved along the direction of these segments, their
                // endpoint is updated.
                if (i == 1)
                {
                    // This is the first segment of the connector.

                    // Also limit their movement to the edges of the
                    // shapes they begin or end within.
                    for (size_t k = 0; k < obstacleVector.size(); ++k)
                    {
                        if (insideLayoutObstacleBounds(displayRoute.ps[i - 1],
                                    obstacleVector[k]))
                        {
                            double offset = displayRoute.ps[i - 1][dim] -
                                    obstacleVector[k].centre()[dim];
                            EndpointAnchorInMoveDir terminal(
                                    dim, *curr, i - 1, k, offset);
                            extraTerminalsList.push_back(terminal);
                        }
                    }
                }
                if ((i + 1) == displayRoute.size())
                {
                    // This is the final segment of the connector.

                    // Also limit their movement to the edges of the
                    // shapes they begin or end within.
                    for (size_t k = 0; k < obstacleVector.size(); ++k)
                    {
                        if (insideLayoutObstacleBounds(displayRoute.ps[i],
                                    obstacleVector[k]))
                        {
                            double offset = displayRoute.ps[i][dim] -
                                    obstacleVector[k].centre()[dim];
                            EndpointAnchorInMoveDir terminal(
                                    dim, *curr, i, k, offset);
                            extraTerminalsList.push_back(terminal);
                        }
                    }
                }
            }
        }
    }
}

static void setupOrthogonalLayoutConstraints(Router *router,
        const size_t dim, LayoutEdgeSegmentList& segmentList,
        LayoutObstacleVector& obstacleVector,
        EndpointAnchorList& extraTerminalsList,
        cola::CompoundConstraints& ccs, cola::VariableIDMap& idMap,
        cola::RootCluster *clusterHierarchy, vpsc::Rectangles& rs,
        LineReps *lineReps, const double moveLimit)
{
    COLA_UNUSED(router);

    int count = 1;
    if (segmentList.empty())
    {
        // There are no segments, so we can just return now.
        return;
    }


    vpsc::Variables vs;
    vpsc::Constraints cs;
    vpsc::Rectangles boundingBoxes;
    LayoutEdgeSegmentSeparations less;

    cola::VariableIDMap colaToCurrMap;
    // Do a sweep to determine space for shifting segments.
    size_t altDim = (dim + 1) % 2;
    const size_t ovn = obstacleVector.size();
    const size_t cpn = segmentList.size();
    std::vector<double> offsetsVector(ovn, 0.0);
    // Set up the events for the sweep.
    size_t totalEvents = (4 * ovn) + (2 * cpn);
    Event **events = new Event*[totalEvents];
    unsigned ctr = 0;
    for (unsigned i = 0; i < ovn; i++)
    {
        LayoutObstacle& obstacle = obstacleVector[i];
        JunctionRef *junction = obstacle.junction();
        if (junction && ! junction->positionFixed())
        {
        }
        Point min = obstacle.min;
        Point max = obstacle.max;
        boundingBoxes.push_back(new vpsc::Rectangle(min.x, max.x, min.y, max.y));

        LayoutNode *v = new LayoutNode(&obstacle, min[dim]);
        v->side = SIDE_LEFT;
        events[ctr++] = new Event(Open, v, min[altDim]);
        events[ctr++] = new Event(Close, v, max[altDim]);

        v = new LayoutNode(&obstacle, max[dim]);
        v->side = SIDE_RIGHT;
        events[ctr++] = new Event(Open, v, min[altDim]);
        events[ctr++] = new Event(Close, v, max[altDim]);

        obstacle.createSolverVariable();
        vs.push_back(obstacle.variable);
        unsigned index = vs.size() - 1;
        bool reverse = false;
        unsigned colaIndex = idMap.mappingForVariable(obstacle.shape()->id(),
                reverse);
        if (colaIndex != obstacle.shape()->id())
        {
            colaToCurrMap.addMappingForVariable(colaIndex, index);
        }

        // Calculate offsets in case the cola and libavoid shapes have
        // different sizes.
        if (colaIndex < rs.size())
        {
            double offset = rs[colaIndex]->getCentreD(dim) -
                    obstacleVector[i].centre()[dim];
            if (fabs(offset) > 0.001)
            {
                offsetsVector[i] = offset;
            }
#ifdef ORTHOG_TOPOLOGY_DEBUG
            fprintf(stderr, "Shape %03d (cola: %03d) centre offset %g "
                    "(from %g, %g)\n", index, colaIndex,
                    offsetsVector[i], obstacleVector[i].centre()[dim],
                    rs[colaIndex]->getCentreD(dim));
#endif
        }
    }
    for (LayoutEdgeSegmentList::iterator curr = segmentList.begin();
            curr != segmentList.end(); ++curr)
    {
        const Point& lowPt = (*curr)->lowPoint();
        const Point& highPt = (*curr)->highPoint();

        (*curr)->createSolverVariable();
        vs.push_back((*curr)->variable);

        COLA_ASSERT(lowPt[dim] == highPt[dim]);
        COLA_ASSERT(lowPt[altDim] < highPt[altDim]);
        Node *v = new Node(*curr, lowPt[dim]);
        events[ctr++] = new Event(SegOpen, v, lowPt[altDim]);
        events[ctr++] = new Event(SegClose, v, highPt[altDim]);
    }
    qsort((Event*)events, (size_t) totalEvents, sizeof(Event*), compare_events);

    // Update the variable Ids on the existing compound constraints
    for (size_t i = 0; i < ccs.size(); ++i)
    {
        cola::CompoundConstraint *cc = ccs[i];
        cc->updateVarIDsWithMapping(colaToCurrMap);
        cc->updateShapeOffsetsForDifferentCentres(offsetsVector);
    }

    double idealNudgeDist = router->routingParameter(idealNudgingDistance);
    // Process the sweep.
    LayoutScanlineNodeSet scanline;
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
                    processLayoutConstraintEvent(scanline, events[j], dim,
                            obstacleVector, lineReps, cs, less, pass,
                            moveLimit, idealNudgeDist);
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
        processLayoutConstraintEvent(scanline, events[i], dim,
                obstacleVector, lineReps, cs, less, pass, moveLimit,
                idealNudgeDist);
    }
    COLA_ASSERT(scanline.size() == 0);
    for (unsigned i = 0; i < totalEvents; ++i)
    {
        delete events[i];
    }
    delete [] events;

    vpsc::Constraints valid = cs;
    std::vector<double> priorPos(vs.size());

    // Apply existing constraints:
    if (clusterHierarchy && !clusterHierarchy->clusters.empty())
    {
        // Create variables for clusters
        clusterHierarchy->computeBoundingRect(boundingBoxes);
        clusterHierarchy->createVars((vpsc::Dim) dim, boundingBoxes, vs);
    }
    for (cola::CompoundConstraints::const_iterator c = ccs.begin();
            c != ccs.end(); ++c)
    {
        (*c)->generateVariables((vpsc::Dim) dim, vs);
    }
    for (cola::CompoundConstraints::const_iterator c = ccs.begin();
            c != ccs.end(); ++c)
    {
        (*c)->generateSeparationConstraints((vpsc::Dim) dim, vs,
                valid, boundingBoxes);
    }

    vpsc::Constraints nonRedundantCs =
            constraintsRemovingRedundantEqualities(vs, valid);

    vpsc::IncSolver vpscInstance(vs, nonRedundantCs);
    vpscInstance.satisfy();

    bool subConstraintSatisfiable = true;
    while (!less.empty())
    {
        LayoutEdgeSegmentSeparations::iterator frontIt = less.begin();
        LayoutEdgeSegmentSeparation sepChoice = *frontIt;
        less.erase(frontIt);

        // Reset subConstraintSatisfiable for new solve.
        subConstraintSatisfiable = true;

        // Store current values for variables.
        for (unsigned int i = 0; i < priorPos.size(); ++i)
        {
            priorPos[i] = vs[i]->finalPosition;
        }

        for (size_t i = 0; i < valid.size(); ++i)
        {
            valid[i]->unsatisfiable = false;
        }

        // Add the constraint from this alternative to the
        // valid constraint set.
        valid.push_back(new vpsc::Constraint(sepChoice.var1,
                sepChoice.var2, 0, true));

#ifdef ORTHOG_TOPOLOGY_DEBUG
        fprintf(stderr, "Try removing sep in %c on connector %03d...",
                (dim == XDIM) ? 'X' : 'Y', sepChoice.connRef->id());
#endif

        bool needsSolving = true;
        while (needsSolving)
        {
            // Solve with this constraint set.
            vpsc::Constraints nonRedundantCs =
                    constraintsRemovingRedundantEqualities(vs, valid);
            vpsc::IncSolver vpscInstance(vs, nonRedundantCs);
#ifdef ORTHOG_TOPOLOGY_DEBUG
            fprintf(stderr, "Solving...!\n");
#endif
            vpscInstance.satisfy();
            needsSolving = false;

            for (vpsc::Constraints::iterator it = valid.begin();
                    it != valid.end(); )
            {
                vpsc::Constraint *constraint = *it;
                if (constraint->unsatisfiable &&
                    (fabs((constraint->left->finalPosition + constraint->gap) -
                          constraint->right->finalPosition) < 0.01))
                {
                    // This is an almost satisfiable constraint.
#ifdef ORTHOG_TOPOLOGY_DEBUG
                    fprintf(stderr, "%s constraint: "
                            "(%d)%.12g + %g %s (%d)%.12g\n",
                            constraint->equality ? "Dropped redundant" :
                            "Relaxed", constraint->left->id,
                            constraint->left->finalPosition, constraint->gap,
                            constraint->equality ? "==" : "<=",
                            constraint->right->id,
                            constraint->right->finalPosition);
#endif

                    if (constraint->equality)
                    {
                        // Drop redundant equality constraint.
                        // We know these occur due to cycles of equalities.
                        it = valid.erase(it);
                    }
                    else
                    {
                        // Relax seemingly unsatisfiable inequality.
                        constraint->gap -= 0.01;
                        constraint->unsatisfiable = false;
                        ++it;
                    }
                    // And now we need to resatisfy the problem..
                    needsSolving = true;
                }
                else
                {
                    ++it;
                }
            }
        }
        for (size_t i = 0; i < valid.size(); ++i)
        {
            if (valid[i]->unsatisfiable)
            {
#ifdef ORTHOG_TOPOLOGY_DEBUG
                fprintf(stderr, "Unsatisfiable: (%d)%.12g + %g %s (%d)%.12g\n",
                        valid[i]->left->id,
                        valid[i]->left->finalPosition, valid[i]->gap,
                        valid[i]->equality ? "==" : "<=",
                        valid[i]->right->id,
                        valid[i]->right->finalPosition);
#endif
                // It might have made one of the earlier added
                // constraints unsatisfiable, so we mark that one
                // as okay since we will be reverting the most
                // recent one.
                valid[i]->unsatisfiable = false;

                subConstraintSatisfiable = false;
            }
        }

        if (!subConstraintSatisfiable)
        {
#ifdef ORTHOG_TOPOLOGY_DEBUG
            fprintf(stderr, "\tInvalid\n");
#endif
            // Restore previous values for variables.
            for (unsigned int i = 0; i < priorPos.size(); ++i)
            {
                vs[i]->finalPosition = priorPos[i];
            }

            // Delete the newly added (and unsatisfiable)
            // constraint from the valid constraint set.
            delete valid.back();
            valid.pop_back();
        }
        else
        {
#ifdef ORTHOG_TOPOLOGY_DEBUG
            fprintf(stderr, "Valid %d\n", count);
#endif
            for (unsigned i = 0; i < ovn; i++)
            {
                LayoutObstacle& obstacle = obstacleVector[i];

                obstacle.updatePositionsFromSolver();
            }
            for (LayoutEdgeSegmentList::iterator curr = segmentList.begin();
                    curr != segmentList.end(); ++curr)
            {
                (*curr)->updatePositionsFromSolver();
            }
            for (EndpointAnchorList::iterator curr = extraTerminalsList.begin();
                    curr != extraTerminalsList.end(); ++curr)
            {
                curr->updatePosition(obstacleVector);
            }
#ifdef ORTHOG_TOPOLOGY_DEBUG
            std::stringstream filename;
            filename << "layout-" << dim << "-" << std::setfill('0') << std::setw(3) << count;
            router->outputDiagramSVG(filename.str().c_str(), lineReps);
#endif
            ++count;
        }
    }

    // Reverse the variable Ids on the existing compound constraints
    bool reversed = false;
    for (size_t i = 0; i < ccs.size(); ++i)
    {
        cola::CompoundConstraint *cc = ccs[i];
        cc->updateShapeOffsetsForDifferentCentres(offsetsVector,
                reversed);
        cc->updateVarIDsWithMapping(colaToCurrMap, reversed);
    }
    for_each(valid.begin(), valid.end(), cola::delete_object());
    for_each(vs.begin(), vs.end(), cola::delete_object());
    for_each(segmentList.begin(), segmentList.end(), cola::delete_object());
    for_each(boundingBoxes.begin(), boundingBoxes.end(), cola::delete_object());
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
typedef std::vector<Avoid::Polygon> RouteVector;

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
                bool *comparable = nullptr) const
        {
            const LayoutEdgeSegment *lhs =
                    dynamic_cast<const LayoutEdgeSegment *> (lhsSuper);
            const LayoutEdgeSegment *rhs =
                    dynamic_cast<const LayoutEdgeSegment *> (rhsSuper);
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

AvoidTopologyAddon::AvoidTopologyAddon(vpsc::Rectangles& rs,
        cola::CompoundConstraints& cs, cola::RootCluster *ch,
        cola::VariableIDMap& map, const double moveLimit)
    : Avoid::TopologyAddonInterface(),
      m_rectangles(rs),
      m_constraints(cs),
      m_cluster_hierarchy(ch),
      m_id_map(map),
      m_move_limit(moveLimit)
{
}

AvoidTopologyAddon::~AvoidTopologyAddon()
{
}

Avoid::TopologyAddonInterface *AvoidTopologyAddon::clone(void) const
{
    return new AvoidTopologyAddon(*this);
}

void AvoidTopologyAddon::improveOrthogonalTopology(Router *router)
{
    //router->timers.Register(???, timerStart);

    // We depend on the checkpoint cache built earlier.

#ifdef ORTHOG_TOPOLOGY_DEBUG
    router->outputDiagramSVG("layout-0-000");
#endif

    for (size_t dimension = 0; dimension < 2; ++dimension)
    {
        LayoutEdgeSegmentList segmentList;
        LayoutObstacleVector obstacleVector;
        EndpointAnchorList extraTerminalsList;
        LineReps lineReps;

        buildOrthogonalLayoutSegments(router, dimension, segmentList,
                obstacleVector, extraTerminalsList);
        setupOrthogonalLayoutConstraints(router, dimension, segmentList,
                obstacleVector, extraTerminalsList, m_constraints, m_id_map,
                m_cluster_hierarchy, m_rectangles, &lineReps, m_move_limit);

        simplifyOrthogonalRoutes(router);
    }

    //router->timers.Stop();
}


bool AvoidTopologyAddon::outputCode(FILE *fp) const
{
    if (fp)
    {
        fprintf(fp, "    CompoundConstraints ccs;\n");
        fprintf(fp, "    std::vector<vpsc::Rectangle*> rs;\n");
        fprintf(fp, "    vpsc::Rectangle *rect = nullptr;\n\n");
        for (size_t i = 0; i < m_rectangles.size(); ++i)
        {
            fprintf(fp, "    rect = new vpsc::Rectangle(%g, %g, %g, %g);\n",
                   m_rectangles[i]->getMinX(), m_rectangles[i]->getMaxX(),
                   m_rectangles[i]->getMinY(), m_rectangles[i]->getMaxY());
            fprintf(fp, "    rs.push_back(rect);\n\n");
        }

        for (cola::CompoundConstraints::const_iterator c =
                m_constraints.begin(); c != m_constraints.end(); ++c)
        {
            (*c)->printCreationCode(fp);
        }

        if (m_cluster_hierarchy)
        {
            m_cluster_hierarchy->printCreationCode(fp);
        }
        else
        {
            fprintf(fp, "    RootCluster *cluster%llu = nullptr;\n\n",
                    (unsigned long long) m_cluster_hierarchy);
        }

        m_id_map.printCreationCode(fp);

        fprintf(fp, "    topology::AvoidTopologyAddon topologyAddon(rs, ccs, "
                "cluster%llu, idMap);\n",
                (unsigned long long) m_cluster_hierarchy);
        fprintf(fp, "    router->setTopologyAddon(&topologyAddon);\n");
    }
    return true;
}


bool AvoidTopologyAddon::outputDeletionCode(FILE *fp) const
{
    if (fp)
    {
        if (m_cluster_hierarchy)
        {
            fprintf(fp, "\n    delete cluster%llu;\n",
                    (unsigned long long) m_cluster_hierarchy);
        }
        fprintf(fp, "    for_each(rs.begin(), rs.end(), cola::delete_object());\n");
        fprintf(fp, "    for_each(ccs.begin(), ccs.end(), cola::delete_object());\n\n");
    }
    return true;
}


}
