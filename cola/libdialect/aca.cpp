/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libdialect - A library for computing DiAlEcT layouts:
 *                 D = Decompose/Distribute
 *                 A = Arrange
 *                 E = Expand/Emend
 *                 T = Transform
 *
 * Copyright (C) 2018  Monash University
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
 * Author(s):   Steve Kieffer   <http://skieffer.info>
*/

#include <algorithm>
#include <vector>
#include <string>
#include <utility>
#include <map>
#include <set>

#include <sstream>
#include <iostream>
#include <clocale>

#include "libvpsc/constraint.h"
#include "libvpsc/variable.h"
#include "libvpsc/rectangle.h"
#include "libvpsc/solve_VPSC.h"
#include "libvpsc/exceptions.h"

#include "libcola/cola.h"
#include "libcola/cc_nonoverlapconstraints.h"

#include "libdialect/util.h"
#include "libdialect/constraints.h"
#include "libdialect/ortho.h"
#include "libdialect/graphs.h"
#include "libdialect/aca.h"

using std::vector;
using std::string;
using std::pair;
using std::map;
using std::multimap;
using std::set;
using std::sort;

using std::cout;
using std::endl;

using vpsc::XDIM;
using vpsc::YDIM;
using vpsc::Dim;
using vpsc::HORIZONTAL;
using vpsc::VERTICAL;
using vpsc::Constraint;
using vpsc::Constraints;
using vpsc::Rectangle;
using vpsc::Rectangles;
using vpsc::Variable;
using vpsc::Variables;
using vpsc::IncSolver;
using vpsc::UnsatisfiedConstraint;

using cola::Edge;
using cola::ShapePair;
using cola::AlignmentConstraint;
using cola::CompoundConstraints;
using cola::EdgeLengths;
using cola::TestConvergence;
using cola::PreIteration;
using cola::NonOverlapConstraintExemptions;
using cola::UnsatisfiableConstraintInfos;

namespace dialect {

const double ACALayout::BP_PENALTY = 2;
const double ACALayout::LEAF_PENALTY = 5;
const double ACALayout::PENALTY_BOUND = 20;

const double ACALayout::EDGE_SHAPE_HALF_THICKNESS = 4;
const double ACALayout::EDGE_SHAPE_BUFFER = 1;

/*
 * Apply the permutation (13)(24) to the separation flag. In other words,
 * swap N and S, and swap E and W.
 */
ACASepFlag negateSepFlag(ACASepFlag sf)
{
    // Let c = 0000WSEN
    unsigned short c = (unsigned short) sf;
    // Update c to WSENWSEN.
    c += 16*c;
    // Kill first and last bits, obtaining 00ENWS00.
    c &= 60; // 60 is 00111100 in binary
    // Shift right, obtaining 0000ENWS.
    unsigned short b = c >> 2;
    // Recast and return.
    ACASepFlag nf = (ACASepFlag) b;
    return nf;
}

/*
 * Map N and S to Vertical; E and W to Horizontal.
 */
ACAFlag sepToAlignFlag(ACASepFlag sf)
{
    return sf==ACANORTH || sf==ACASOUTH ? ACAVERT : ACAHORIZ;
}

/*
 * Map each dimension to its perpendicular dim; in other words,
 * swap Vert and Horiz.
 */
ACAFlag perpAlignFlag(ACAFlag af)
{
    return af==ACAHORIZ ? ACAVERT : ACAHORIZ;
}

/*
 * Map a vector (dx, dy) to a set of separation flags.
 */
ACASepFlag vectorToSepFlag(double dx, double dy)
{
    int f = 0;
    f |= dx > 0 ? ACAEAST : dx < 0 ? ACAWEST : 0;
    f |= dy > 0 ? ACASOUTH : dy < 0 ? ACANORTH : 0;
    return (ACASepFlag) f;
}

bool propsedSepConflictsWithExistingPosition(ACASepFlag pro, ACASepFlag ex)
{
    // Proposed separation pro conflicts with existing position ex
    // if the union of their bits contains both north and south, or
    // both east and west.
    int u = pro | ex;
    return ( ( (u&ACANORTHSOUTH) == ACANORTHSOUTH ) || ( (u&ACAEASTWEST) == ACAEASTWEST ) );
}

bool sortOrdAlignsByPenalty(const OrderedAlignment *lhs, const OrderedAlignment *rhs)
{
    return lhs->penalty < rhs->penalty;
}

ACALayout::ACALayout(
        const vpsc::Rectangles& rs,
        const std::vector<cola::Edge>& es,
        CompoundConstraints& ccs,
        const double idealLength,
        const EdgeLengths& eLengths,
        TestConvergence *doneTest,
        PreIteration *preIteration)
    : m_graph(nullptr),
      m_n(rs.size()),
      m_m(es.size()),
      m_rs(rs),
      m_es(es),
      m_ccs(ccs),
      m_rc(nullptr),
      m_idealLength(idealLength),
      m_preventOverlaps(false),
      m_edgeLengths(eLengths),
      m_doneTest(doneTest),
      m_preIteration(preIteration),
      m_addBendPointPenalty(true),
      m_favourLongEdges(false),
      m_postponeLeaves(true),
      m_useNonLeafDegree(true),
      m_allAtOnce(false),
      m_aggressiveOrdering(false),
      m_fdlayout(nullptr),
      m_lengthUpperBound(10000),
      m_didLayoutForLastAlignment(false),
      m_doFinalFDLayout(false),
      m_nocExemptions(new NonOverlapConstraintExemptions()),
      m_nocsInitialised(false),
      m_layoutPeriod(1000000)
{
    // By default, no edges are ignored.
    for (int i=0; i<m_m; ++i) m_ignoreEdge.push_back(false);
    // By default, no nodes are ignored.
    for (int i=0; i<m_n; ++i) m_ignoreNodeForOPWithOffsets.push_back(false);
    // Initialise tables.
    computeDegrees();
    generateVPSCConstraints();
    initStateTables();
}

ACALayout::ACALayout(std::shared_ptr<dialect::Graph> G)
    : m_graph(G),
      m_rc(nullptr),
      m_preventOverlaps(false),
      m_addBendPointPenalty(true),
      m_favourLongEdges(false),
      m_postponeLeaves(true),
      m_useNonLeafDegree(true),
      m_allAtOnce(false),
      m_aggressiveOrdering(false),
      m_fdlayout(nullptr),
      m_lengthUpperBound(10000),
      m_didLayoutForLastAlignment(false),
      m_doFinalFDLayout(false),
      m_nocExemptions(new NonOverlapConstraintExemptions()),
      m_nocsInitialised(false),
      m_layoutPeriod(1000000)
{
    // Cola Graph Rep:
    ColaGraphRep &colaRep = G->updateColaGraphRep();
    m_n = colaRep.rs.size();
    m_m = colaRep.es.size();
    m_rs = colaRep.rs;
    m_es = colaRep.es;
    m_ccs.push_back(&G->getSepMatrix());
    // Cola Options:
    m_idealLength = G->getIEL();
    m_edgeLengths = cola::StandardEdgeLengths;
    m_doneTest = nullptr;
    m_preIteration = nullptr;
    // By default, no edges are ignored.
    for (int i=0; i<m_m; ++i) m_ignoreEdge.push_back(false);
    // By default, no nodes are ignored.
    for (int i=0; i<m_n; ++i) m_ignoreNodeForOPWithOffsets.push_back(false);
    // Initialise tables.
    computeDegrees();
    generateVPSCConstraints();
    initStateTables();
}


ACALayout::~ACALayout(void)
{
    // Miscelaneous objects:
    delete m_alignmentState;
    delete m_fdlayout;
    delete m_nocExemptions;
    delete m_xnocs;
    delete m_ynocs;
    // OrderedAlignments. See however the doctext on the OrderedAlignment struct's
    // destructor method, which notes that it deliberately does NOT delete the
    // two cola::CompoundConstraints to which it holds pointers. Thus, these are
    // the client's responsibility.
    for (auto p : m_ordAligns) delete p;
    // Variables:
    for (auto p : m_xvs) delete p;
    for (auto p : m_yvs) delete p;
    // Constraints:
    for (auto p : m_xcs) delete p;
    for (auto p : m_ycs) delete p;
    // Only Rectangles beyond the first m_n were allocated by this ACALayout instance;
    // the first m_n were client-supplied and are therefore NOT deleted.
    for (size_t i = m_n; i < m_xrs.size(); ++i) {
        delete m_xrs.at(i);
    }
    for (size_t i = m_n; i < m_yrs.size(); ++i) {
        delete m_yrs.at(i);
    }
    // We must not delete pointers in any of m_xEqCs, m_yEqCs, m_xIneqCs, m_yIneqCs,
    // since these Contraints are already pointed to in m_xcs and m_ycs.
    // We must not delete pointers in m_extendedRS, since these Rectangles are
    // already pointed to in the portions of m_xrs and m_yrs beyond the first
    // m_n entries.
}

void ACALayout::createAlignments(void)
{
    if (!m_nocsInitialised) initNOCs();
    if (m_allAtOnce) {
        acaLoopAllAtOnce();
    } else {
        acaLoopOneByOne();
    }
}

bool ACALayout::createOneAlignment(void)
{
    if (!m_nocsInitialised) initNOCs();
    return acaLoopOnce();
}

bool ACALayout::applyOAsAllOrNothing(OrderedAlignments oas)
{
    if (!m_nocsInitialised) initNOCs();
    bool b = allOrNothing(oas);
    return b;
}

void ACALayout::layout(void)
{
    if (!m_nocsInitialised) initNOCs();
    layoutWithCurrentConstraints();
    createAlignments();
}

void ACALayout::removeOverlaps(void)
{
    if (!m_nocsInitialised) initNOCs();
    bool pO = m_preventOverlaps;
    m_preventOverlaps = true;
    layoutWithCurrentConstraints();
    m_preventOverlaps = pO;
}

cola::ConstrainedFDLayout *ACALayout::getFDLayout(void)
{
    return m_fdlayout;
}

void ACALayout::addBendPointPenalty(bool b)
{
    m_addBendPointPenalty = b;
}

void ACALayout::favourLongEdges(bool b)
{
    m_favourLongEdges = b;
}

void ACALayout::postponeLeaves(bool b)
{
    m_postponeLeaves = b;
}

void ACALayout::useNonLeafDegree(bool b)
{
    m_useNonLeafDegree = b;
}

void ACALayout::allAtOnce(bool b)
{
    m_allAtOnce = b;
}

void ACALayout::aggressiveOrdering(bool b)
{
    m_aggressiveOrdering = b;
}

void ACALayout::setAvoidNodeOverlaps(bool avoidOverlaps)
{
    m_preventOverlaps = avoidOverlaps;
}

void ACALayout::ignoreEdges(std::vector<bool> ignore)
{
    COLA_ASSERT(ignore.size()==(size_t)m_m); // There should be one boolean for each edge.
    m_ignoreEdge = ignore;
}

void ACALayout::ignoreNodesForOPWithOffsets(std::vector<bool> ignore)
{
    COLA_ASSERT(ignore.size()==(size_t)m_n);
    m_ignoreNodeForOPWithOffsets = ignore;
}

void ACALayout::setNodeAliases(std::map<int, int> aliases)
{
    m_nodeAliases = aliases;
    // Recompute node degrees and neighbours.
    computeDegrees();
    // Add connections in state table.
    for (int j = 0; j < m_m; ++j) {
        cola::Edge e = m_es.at(j);
        int src = e.first, tgt = e.second;
        src = alias(src);
        tgt = alias(tgt);
        (*m_alignmentState)(src,tgt) |= ACACONN;
        (*m_alignmentState)(tgt,src) |= ACACONN;
    }
}

void ACALayout::setClusterHierarchy(cola::RootCluster *rc)
{
    m_rc = rc;
}

bool ACALayout::edgeIsAligned(int j) const
{
    cola::Edge e = m_es.at(j);
    int src = e.first, tgt = e.second;
    int astate = (*m_alignmentState)(src,tgt);
    return (astate & (ACAHORIZ|ACAVERT));
}

bool ACALayout::nodesAreAligned(int i, int j) const
{
    int astate = (*m_alignmentState)(i,j);
    return (astate & (ACAHORIZ|ACAVERT));
}

void ACALayout::layoutPeriod(unsigned p)
{
    m_layoutPeriod = p;
}

void ACALayout::doFinalLayout(bool b)
{
    m_doFinalFDLayout = b;
}

void ACALayout::addGroupOfNonOverlapExemptRectangles(std::vector<unsigned> rs)
{
    // 1. We maintain a vector of vectors, to be added to ConstrainedFDLayout objects
    //    each time they are created.
    m_nocExemptRects.push_back(rs);
    // 2. We maintain a multimap mapping each rectangle index to all the rectangle
    //    indices with which it is exempt. This is used when we add a new rectangle
    //    for each aligned edge; namely, when aligning edge e=(u,v) we make this new
    //    rectangle exempt from the union of the exemption sets of u and v. This is
    //    important when we put port dummy nodes /inside/ of the proper nodes to
    //    which they belong, so that the edge rectangles can be exempt from non-overlap
    //    with those proper nodes as well.
    for (std::vector<unsigned>::iterator it=rs.begin(); it!=rs.end(); ++it) {
        for (std::vector<unsigned>::iterator jt=rs.begin(); jt!=rs.end(); ++jt) {
            if (*it==*jt) continue;
            m_nocExemptionSets.insert(std::pair<unsigned,unsigned>(*it,*jt));
        }
    }
}

std::set<unsigned> ACALayout::exemptionSetForEdge(int j)
{
    cola::Edge e = m_es[j];
    unsigned src = e.first, tgt = e.second;
    std::set<unsigned> exemptions;
    // src set
    std::pair <std::multimap<unsigned,unsigned>::iterator, std::multimap<unsigned,unsigned>::iterator > srcRange;
    srcRange = m_nocExemptionSets.equal_range(src);
    for (std::multimap<unsigned,unsigned>::iterator it=srcRange.first; it!=srcRange.second; ++it) {
        exemptions.insert(it->second);
    }
    // tgt set
    std::pair <std::multimap<unsigned,unsigned>::iterator, std::multimap<unsigned,unsigned>::iterator > tgtRange;
    tgtRange = m_nocExemptionSets.equal_range(tgt);
    for (std::multimap<unsigned,unsigned>::iterator it=tgtRange.first; it!=tgtRange.second; ++it) {
        exemptions.insert(it->second);
    }
    return exemptions;
}

void ACALayout::setAlignmentOffsetsForCompassDirection(ACASepFlag sf, EdgeOffsets offsets)
{
    COLA_ASSERT(offsets.size()==(size_t)m_m); // There should be one offset for each edge.
    m_edgeOffsets.insert( std::pair<ACASepFlag,EdgeOffsets>(sf,offsets) );
}

void ACALayout::setAllowedDirections(ACASepFlags seps)
{
    COLA_ASSERT(seps.size()==(size_t)m_m); // There should be one flag for each edge.
    m_allowedSeps = seps;
}

std::string ACALayout::writeAlignmentTable(void) const
{
    std::string s = m_alignmentState->toString();
    return s;
}

std::string ACALayout::writeStateForNodeIds(id_type id1, id_type id2)
{
    std::ostringstream ss;
    ColaGraphRep cgr = m_graph->getColaGraphRep();
    size_t ix1 = cgr.id2ix.at(id1),
           ix2 = cgr.id2ix.at(id2);
    ss << "Node of ID " << id1 << " has index " << ix1 << std::endl;
    ss << "Node of ID " << id2 << " has index " << ix2 << std::endl;
    ss << "State for indices (" << ix1 << ", " << ix2 << "): " << (*m_alignmentState)(ix1, ix2) << std::endl;
    ss << "State for indices (" << ix2 << ", " << ix1 << "): " << (*m_alignmentState)(ix2, ix1) << std::endl;
    return ss.str();
}

void ACALayout::computeDegrees(void)
{
    m_nbrs.clear();
    m_leaves.clear();
    m_deg2Nodes.clear();
    m_nlnbrs.clear();
    m_nldeg2Nodes.clear();
    // Map node indices to indices of their neighbours.
    for (int j = 0; j < m_m; ++j) {
        cola::Edge e = m_es.at(j);
        m_nbrs.insert(std::pair<int,int>(e.first,e.second));
        m_nbrs.insert(std::pair<int,int>(e.second,e.first));
        m_incidentEdges.insert(std::pair<int,int>(e.first,j));
        m_incidentEdges.insert(std::pair<int,int>(e.second,j));
    }
    // Find the leaves and degree-2 nodes.
    for (int i = 0; i < m_n; ++i) {
        int c = m_nbrs.count(i);
        if (c == 1) m_leaves.insert(i);
        if (c == 2) m_deg2Nodes.insert(i);
    }
    // Compute non-leaf neighbours.
    for (int i = 0; i < m_n; ++i) {
        // Iterate over the neighbours of node i, retaining only the non-leaves.
        std::pair< std::multimap<int,int>::iterator, std::multimap<int,int>::iterator > range;
        range = m_nbrs.equal_range(i);
        for (std::multimap<int,int>::iterator it=range.first; it!=range.second; ++it) {
            int j = it->second;
            if (m_leaves.count(j)==0) {
                m_nlnbrs.insert(std::pair<int,int>(i,j));
            }
        }
    }
    // Compute non-leaf incident edges.
    for (int j = 0; j < m_m; ++j) {
        cola::Edge e = m_es.at(j);
        if (m_leaves.count(e.second)==0) m_nlincidentEdges.insert(std::pair<int,int>(e.first,j));
        if (m_leaves.count(e.first)==0) m_nlincidentEdges.insert(std::pair<int,int>(e.second,j));
    }
    // Find the non-leaf degree-2 nodes.
    for (int i = 0; i < m_n; ++i) {
        if (m_nlnbrs.count(i) == 2) m_nldeg2Nodes.insert(i);
    }
}

void ACALayout::generateVPSCConstraints(void)
{
    // First generate one x-variable and one y-variable for each node.
    // Also create a rectangle vector for each dimension.
    m_xrs.resize(m_n);
    m_yrs.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_xvs.push_back(new vpsc::Variable(i,m_rs[i]->getCentreX()));
        m_yvs.push_back(new vpsc::Variable(i,m_rs[i]->getCentreY()));
        m_xrs[i] = m_rs[i];
        m_yrs[i] = m_rs[i];
    }
    // Generate all VPSC constraints and any auxiliary variables.
    vpsc::Constraints xcs, ycs;
    for (size_t k = 0; k < m_ccs.size(); ++k) {
        cola::CompoundConstraint *cc = m_ccs.at(k);
        cc->generateVariables(vpsc::XDIM, m_xvs);
        cc->generateVariables(vpsc::YDIM, m_yvs);
        cc->generateSeparationConstraints(vpsc::XDIM, m_xvs, xcs, m_rs);
        cc->generateSeparationConstraints(vpsc::YDIM, m_yvs, ycs, m_rs);
    }
    // Compute how many auxiliary vars we got.
    m_numExtraXVars = m_xvs.size() - m_n;
    m_numExtraYVars = m_yvs.size() - m_n;
    // In each dimension, the rectangle vector needs nullptr placeholders to make
    // it the same length as the vector of variables. This is so that when ACA
    // generates guideline variables for alignments, it can create corresponding
    // rectangles for non-overlap constraints involving aligned edges.
    for (int i = 0; i < m_numExtraXVars; ++i) m_xrs.push_back(nullptr);
    for (int i = 0; i < m_numExtraYVars; ++i) m_yrs.push_back(nullptr);
    // Store constraints by dimension and by equality vs. inequality constraints.
    for (size_t k = 0; k < xcs.size(); ++k) {
        vpsc::Constraint *c = xcs.at(k);
        m_xcs.push_back(c);
        if (c->equality) {
            m_xEqCs.push_back(c);
        } else {
            m_xIneqCs.push_back(c);
        }
    }
    for (size_t k = 0; k < ycs.size(); ++k) {
        vpsc::Constraint *c = ycs.at(k);
        m_ycs.push_back(c);
        if (c->equality) {
            m_yEqCs.push_back(c);
        } else {
            m_yIneqCs.push_back(c);
        }
    }
}

void ACALayout::initNOCs(void)
{
    // By the time this method is called, the client should be through adding
    // groups of NOC exempt rectangles. So we can now add all the groups:
    m_nocExemptions->addExemptGroupOfNodes(m_nocExemptRects);
    // And create NOC objects for each dimension:
    m_xnocs = new cola::NonOverlapConstraints(m_nocExemptions);
    m_ynocs = new cola::NonOverlapConstraints(m_nocExemptions);
    // And add all the rectangles:
    for (int i=0; i<m_n; ++i) {
        vpsc::Rectangle *R = m_rs[i];
        double hw = R->width()/2.0, hh = R->height()/2.0;
        m_xnocs->addShape(i,hw,hh);
        m_ynocs->addShape(i,hw,hh);
    }
    // And mark the NOCs as now initialised.
    m_nocsInitialised = true;
}

int ACALayout::alias(int i)
{
    std::map<int,int>::iterator it = m_nodeAliases.find(i);
    if (it!=m_nodeAliases.end()) i = it->second;
    return i;
}

/**
 * Retrieve a rectangle, using the nodeAliases (if any).
 */
vpsc::Rectangle *ACALayout::getRect(int i, bool doAlias)
{
    if (doAlias) i = alias(i);
    vpsc::Rectangle *R = m_rs.at(i);
    return R;
}

/**
 * This is for use only during initialisation of the state tables, when we have
 * to deal with the extra variables generated by the incoming constraints.
 * For that time we arrange the variables like this:
 *
 *     ... rectangles ... | ... extra x-vars ... | ... extra y-vars ...
 *
 * So for example if there were 5 rectangles, 2 extra x-vars, and 3 extra y-vars,
 * then the columns (and rows) in the state table corresponding to the 7 x- and 8 y-vars
 * would be:
 *     x: 0 1 2 3 4 | 5 6
 *     y: 0 1 2 3 4 | 7 8 9
 */
int ACALayout::adjustVarNumForExtraVars(vpsc::Dim dim, int k)
{
    if (dim==vpsc::YDIM && k >= m_n) k += m_numExtraXVars;
    return k;
}

void ACALayout::initStateTables(void)
{
    // Start by building table large enough to handle each rectangle, as
    // well as each extra X-var, and each extra Y-var.
    int N = m_n + m_numExtraXVars + m_numExtraYVars;
    m_alignmentState  = new Matrix2d<int>(N,N);
    // Initialise with zeroes.
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            (*m_alignmentState)(i,j) = 0;
        }
    }
    // Note connections in alignment state table.
    for (int j = 0; j < m_m; ++j) {
        cola::Edge e = m_es.at(j);
        int src = e.first, tgt = e.second;
        (*m_alignmentState)(src,tgt) = ACACONN;
        (*m_alignmentState)(tgt,src) = ACACONN;
    }
    // Consider equality constraints in the x-dimension.
    for (size_t k = 0; k < m_xEqCs.size(); ++k) {
        vpsc::Constraint *c = m_xEqCs.at(k);
        int l = c->left->id, r = c->right->id;
        double gap = c->gap;
        // Adjust for extra variables.
        l = adjustVarNumForExtraVars(vpsc::XDIM,l);
        r = adjustVarNumForExtraVars(vpsc::XDIM,r);
        if (gap==0) {
            // It is an alignment.
            recordAlignmentWithClosure(l,r,ACAVERT,N);
        }
    }
    // Consider equality constraints in the y-dimension.
    for (size_t k = 0; k < m_yEqCs.size(); ++k) {
        vpsc::Constraint *c = m_yEqCs.at(k);
        int l = c->left->id, r = c->right->id;
        double gap = c->gap;
        // Adjust for extra variables.
        l = adjustVarNumForExtraVars(vpsc::YDIM,l);
        r = adjustVarNumForExtraVars(vpsc::YDIM,r);
        if (gap==0) {
            // It is an alignment.
            recordAlignmentWithClosure(l,r,ACAHORIZ,N);
        }
    }

    // Record snapshot for debugging purposes.
    //aStateBeforeChop = m_alignmentState->toString();

    // Now that we have computed the transitive closure of all the passed
    // constraints, we will no longer need the rows and columns for the extra
    // variables, so we chop those off now.
    Matrix2d<int> *aState = new Matrix2d<int>(m_n,m_n);
    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j < m_n; ++j) {
            (*aState)(i,j) = (*m_alignmentState)(i,j);
        }
    }
    delete m_alignmentState;
    m_alignmentState = aState;
}

void ACALayout::recordAlignmentWithClosure(int i, int j, ACAFlag af, int numCols)
{
    if (numCols == 0) numCols = m_n;
    // Get the set Ai of all indices already aligned with i, including i itself.
    // Do likewise for j.
    std::set<int> Ai, Aj;
    Ai.insert(i);
    Aj.insert(j);
    for (int k = 0; k < numCols; ++k) {
        if ( (*m_alignmentState)(i,k) & af ) Ai.insert(k);
        if ( (*m_alignmentState)(j,k) & af ) Aj.insert(k);
    }
    // Record that everything in Ai is aligned with everything in Aj.
    // This is the transitive closure of the new alignment for the alignments table.
    for (std::set<int>::iterator it=Ai.begin(); it!=Ai.end(); ++it) {
        for (std::set<int>::iterator jt=Aj.begin(); jt!=Aj.end(); ++jt) {
            (*m_alignmentState)(*it,*jt) |= af;
            (*m_alignmentState)(*jt,*it) |= af;
        }
    }
}

void ACALayout::addOrderedAlignments(OrderedAlignments oas)
{
    for (OrderedAlignments::const_iterator it=oas.begin(); it!=oas.end(); ++it) {
        OrderedAlignment *oa = *it;
        m_ordAligns.push_back(oa);
        m_ccs.push_back(oa->separation);
        m_ccs.push_back(oa->alignment);
        updateStateTables(oa);
    }
}

void ACALayout::updateStateTables(OrderedAlignment *oa)
{
    int i = oa->src, j = oa->tgt;
    recordAlignmentWithClosure(i, j, oa->af);
}

cola::CompoundConstraints ACALayout::writeAllVPSCConstraintsAsCompound(void)
{
    pushState();
    updateNodeRectsFromVars();
    recomputeEdgeShapes(XDIM);
    recomputeEdgeShapes(YDIM);
    m_xnocs->generateSeparationConstraints(XDIM, m_xvs, m_xcs, m_xrs);
    m_ynocs->generateSeparationConstraints(YDIM, m_yvs, m_ycs, m_yrs);
    cola::CompoundConstraints ccs;
    for (vpsc::Constraints::iterator it=m_xcs.begin(); it!=m_xcs.end(); ++it) {
        Constraint *c = *it;
        int l = c->left->id, r = c->right->id;
        if (l>=m_n) l = m_xAuxRectIndexInExtendedRS.at(l) + m_n;
        if (r>=m_n) r = m_xAuxRectIndexInExtendedRS.at(r) + m_n;
        ccs.push_back(new cola::SeparationConstraint(
            XDIM, l, r, c->gap, c->equality
        ));
    }
    for (vpsc::Constraints::iterator it=m_ycs.begin(); it!=m_ycs.end(); ++it) {
        Constraint *c = *it;
        int l = c->left->id, r = c->right->id;
        if (l>=m_n) l = m_yAuxRectIndexInExtendedRS.at(l) + m_n;
        if (r>=m_n) r = m_yAuxRectIndexInExtendedRS.at(r) + m_n;
        ccs.push_back(new cola::SeparationConstraint(
            YDIM, l, r, c->gap, c->equality
        ));
    }
    popState();
    return ccs;
}

vpsc::Rectangles ACALayout::properAndAuxRects(void)
{
    Rectangles rs;
    for (Rectangles::iterator it=m_rs.begin(); it!=m_rs.end(); ++it) rs.push_back(*it);
    for (Rectangles::iterator it=m_extendedRS.begin(); it!=m_extendedRS.end(); ++it) rs.push_back(*it);
    return rs;
}

void ACALayout::layoutWithCurrentConstraints(void)
{
    // ACA is an experimental method. During development we tried a "new" method for layout
    // which can be reactivating using the following boolean. For most cases we have found the
    // old method preferable, so this is hardcoded to false. The code remains for the sake of
    // future research.
    bool useNewMethod = false;
    // Delete the old FD layout object before creating a new one.
    delete m_fdlayout;
    // Decide which Rectangles to use:
    vpsc::Rectangles rs = useNewMethod ? properAndAuxRects() : m_rs;
    // Form the new FD layout.
    m_fdlayout = new cola::ConstrainedFDLayout(rs, m_es, m_idealLength, m_edgeLengths, m_doneTest, m_preIteration);
    // Node overlap prevention, with possible exemption sets:
    if (useNewMethod) {
        m_fdlayout->setAvoidNodeOverlaps(m_preventOverlaps);
    } else {
        m_fdlayout->setAvoidNodeOverlaps(m_preventOverlaps, m_nocExemptRects);
    }
#ifdef DEBUG_ACA_LAYOUT_STEP
    // For getting info on unsatisfiable constraints:
    UnsatisfiableConstraintInfos *unsatX = new UnsatisfiableConstraintInfos();
    UnsatisfiableConstraintInfos *unsatY = new UnsatisfiableConstraintInfos();
    m_fdlayout->setUnsatisfiableConstraintInfo(unsatX, unsatY);
#endif
    // Constraints:
    cola::CompoundConstraints ccs = useNewMethod ? writeAllVPSCConstraintsAsCompound() : m_ccs;
    m_fdlayout->setConstraints(ccs);
    m_fdlayout->setClusterHierarchy(m_rc);
    // Run the layout.
    m_fdlayout->run();
#ifdef DEBUG_ACA_LAYOUT_STEP
    for (auto p : *unsatX) delete p;
    for (auto p : *unsatY) delete p;
    delete unsatX;
    delete unsatY;
#endif
}

// Only do a layout if some condition is met, e.g. stress has accumulated enough,
// or period has been completed.
// Returned boolean says whether we did a layout.
bool ACALayout::layoutIfAppropriate(void)
{
    m_didLayoutForLastAlignment = false;
    // TODO: Try something more sophisticated; maybe make stress measurement
    // an option.
    unsigned N = m_ordAligns.size();
    if (N > 0 && N % m_layoutPeriod == 0) {
        layoutWithCurrentConstraints();
        m_didLayoutForLastAlignment = true;
    }
    return m_didLayoutForLastAlignment;
}

bool ACALayout::acaLoopOnce(void)
{
    OrderedAlignment *oa = chooseOA();
    if (oa) {
        // Add the new separated alignment constraints.
        m_ccs.push_back(oa->separation);
        m_ccs.push_back(oa->alignment);
        // Redo the layout, with the new constraints.
        layoutIfAppropriate();
        // Update state tables.
        updateStateTables(oa);
        return true;
    } else {
        // We'll come to this case when there are no more alignments to be made.
        // Do a layout if one wasn't done for the last alignment.
        if (m_doFinalFDLayout && !m_didLayoutForLastAlignment) layoutWithCurrentConstraints();
        return false;
    }
}

void ACALayout::acaLoopOneByOne(void)
{
    // Choose a first alignment.
    OrderedAlignment *oa = chooseOA();
    while (oa) {
        // Add the new separated alignment constraints.
        m_ccs.push_back(oa->separation);
        m_ccs.push_back(oa->alignment);
        // Redo the layout, with the new constraints.
        layoutIfAppropriate();
        // Update state tables.
        updateStateTables(oa);
        // Choose next ordered alignment.
        oa = chooseOA();
    }
    // If no layout was done on the final run, do one now.
    if (m_doFinalFDLayout && !m_didLayoutForLastAlignment) {
        layoutWithCurrentConstraints();
    }
}

void ACALayout::acaLoopAllAtOnce(void)
{
    // Choose a first alignment.
    OrderedAlignment *oa = chooseOA();
    while (oa) {
        // Add the new separated alignment constraints.
        m_ccs.push_back(oa->separation);
        m_ccs.push_back(oa->alignment);
        // Update state tables.
        updateStateTables(oa);
        // Choose next ordered alignment.
        oa = chooseOA();
    }
    // Redo the layout, with the new constraints.
    layoutWithCurrentConstraints();
}

bool ACALayout::allOrNothing(OrderedAlignments oas)
{
    bool okay = true;
    pushState();
    pushRectCoords();
    for (OrderedAlignments::const_iterator it=oas.begin(); it!=oas.end(); ++it) {
        OrderedAlignment *oa = *it;
        okay = applyIfFeasible(oa);
        if (!okay) break;
    }
    if (!okay) {
        //perror("All or nothing: nothing.");
        popRectCoords();
        // Must trash any remaining edge shapes before popping the state.
        removeNewEdgeShapesAccordingToStateStack();
        popState();
    } else {
        //perror("All or nothing: all.");
        dropRectCoords();
        dropState();
        addOrderedAlignments(oas);
        layoutIfAppropriate();
    }
    //layoutWithCurrentConstraints();
    return okay;
}

// Record lengths of variable and constraint vectors, so we can restore them later.
void ACALayout::pushState(void)
{
    unsigned nxv=m_xvs.size(), nyv=m_yvs.size(),
             nxc=m_xcs.size(), nyc=m_ycs.size(),
             nxr=m_xrs.size(), nyr=m_yrs.size();
    m_sizeStack.push_back(nxv);
    m_sizeStack.push_back(nyv);
    m_sizeStack.push_back(nxc);
    m_sizeStack.push_back(nyc);
    m_sizeStack.push_back(nxr);
    m_sizeStack.push_back(nyr);
}

// Restore lengths of vectors of Variables, Constraints, and Rectangles, by erasing tails.
// First delete the objects pointed to in those tails.
void ACALayout::popState(void)
{
    COLA_ASSERT(m_sizeStack.size()>=6);
    // Retrieve prior lengths of the vectors.
    unsigned nyr=m_sizeStack.back(); m_sizeStack.pop_back();
    unsigned nxr=m_sizeStack.back(); m_sizeStack.pop_back();
    unsigned nyc=m_sizeStack.back(); m_sizeStack.pop_back();
    unsigned nxc=m_sizeStack.back(); m_sizeStack.pop_back();
    unsigned nyv=m_sizeStack.back(); m_sizeStack.pop_back();
    unsigned nxv=m_sizeStack.back(); m_sizeStack.pop_back();
    // Delete the allocated objects.
    for (auto it = m_xvs.begin()+nxv; it != m_xvs.end(); ++it) delete (*it);
    for (auto it = m_yvs.begin()+nyv; it != m_yvs.end(); ++it) delete (*it);
    for (auto it = m_xcs.begin()+nxc; it != m_xcs.end(); ++it) delete (*it);
    for (auto it = m_ycs.begin()+nyc; it != m_ycs.end(); ++it) delete (*it);
    for (auto it = m_xrs.begin()+nxr; it != m_xrs.end(); ++it) delete (*it);
    for (auto it = m_yrs.begin()+nyr; it != m_yrs.end(); ++it) delete (*it);
    // Truncate the vectors.
    m_xvs.erase(m_xvs.begin()+nxv,m_xvs.end());
    m_yvs.erase(m_yvs.begin()+nyv,m_yvs.end());
    m_xcs.erase(m_xcs.begin()+nxc,m_xcs.end());
    m_ycs.erase(m_ycs.begin()+nyc,m_ycs.end());
    m_xrs.erase(m_xrs.begin()+nxr,m_xrs.end());
    m_yrs.erase(m_yrs.begin()+nyr,m_yrs.end());
}

// Simply pop the last 6 sizes off the size stack.
void ACALayout::dropState(void)
{
    COLA_ASSERT(m_sizeStack.size()>=6);
    for (int i = 0; i < 6; ++i) m_sizeStack.pop_back();
}

void ACALayout::pushRectCoords(void)
{
    for (int i = 0; i < m_n; ++i) {
        Rectangle *R = m_rs[i];
        m_rectXStack.push_back(R->getCentreX());
        m_rectYStack.push_back(R->getCentreY());
    }
}

void ACALayout::popRectCoords(void)
{
    COLA_ASSERT(m_rectXStack.size()>=(size_t)m_n);
    COLA_ASSERT(m_rectYStack.size()>=(size_t)m_n);
    for (int i = 0; i < m_n; ++i) {
        double x = m_rectXStack.back(); m_rectXStack.pop_back();
        double y = m_rectYStack.back(); m_rectYStack.pop_back();
        Rectangle *R = m_rs[m_n-1-i];
        R->moveCentreX(x);
        R->moveCentreY(y);
    }
}

void ACALayout::dropRectCoords(void)
{
    COLA_ASSERT(m_rectXStack.size()>=(size_t)m_n);
    COLA_ASSERT(m_rectYStack.size()>=(size_t)m_n);
    for (int i = 0; i < m_n; ++i) {
        m_rectXStack.pop_back();
        m_rectYStack.pop_back();
    }
}

/**
 * This method is used by the allOrNothing method. It is needed
 * because that method may successfully apply one or more
 * OrderedAlignements before encountering one that is infeasible,
 * and giving up. Ordinarily we use the applyIfFeasible method
 * to apply OrderedAlignments one at a time, and when they prove
 * infeasible that method (near the end) takes responsibility for
 * cleaning up all records of the new edge shape that was added.
 * This method is needed in order to clean up those same records,
 * only for the OrderedAlignments that /were/ feasible, and were
 * adopted, before an infeasible one was encountered.
 *
 * Note: the reason this method need not remove the new edge shapes
 * from either the m_xrs or m_yrs vectors is that those vectors are
 * restored to their prior state by the popState method, invoked
 * by the allOrNothing method.
 */
void ACALayout::removeNewEdgeShapesAccordingToStateStack(void)
{
    // Check how many variables there were in both dimensions,
    // in the previous state.
    size_t N = m_sizeStack.size();
    COLA_ASSERT(N>=6);
    unsigned nyv=m_sizeStack[N-5];
    unsigned nxv=m_sizeStack[N-6];
    // And how many are there now?
    unsigned Nyv = m_yvs.size();
    unsigned Nxv = m_xvs.size();
    // Revert the data structures that grow with addition
    // of edge shapes.
    // x-dimension
    for (unsigned i = Nxv - 1; i >= nxv; i--) {
        m_xGuidelineIndexToEdgeIndex.erase(i);
        m_xnocs->removeShape(i);
        m_xAuxRectIndexInExtendedRS.erase(i);
    }
    // y-dimension
    for (unsigned j = Nyv - 1; j >= nyv; j--) {
        m_yGuidelineIndexToEdgeIndex.erase(j);
        m_ynocs->removeShape(j);
        m_yAuxRectIndexInExtendedRS.erase(j);
    }
    // As for the m_extendedRS vector, there should be one new
    // rectangle at the end of it for each of the edge shapes.
    // We pop these off the vector now. (Could have done so in
    // in the two loops above, but that would put cleverness over
    // clarity.)
    for (unsigned k = 0; k < (Nxv-nxv)+(Nyv-nyv); ++k) {
        m_extendedRS.pop_back();
    }
}

vpsc::Rectangle *ACALayout::makeRectForOA(OrderedAlignment *oa)
{
    return makeRectForEdge(oa->edgeIndex,oa->dim);
}

vpsc::Rectangle *ACALayout::makeRectForEdge(int j, vpsc::Dim dim)
{
    vpsc::Rectangle *R = new vpsc::Rectangle(0,1,0,1);
    updateRectForEdge(R,j,dim);
    return R;
}

void ACALayout::updateRectForEdge(Rectangle *R, int j, Dim dim)
{
    // Here dim should be HORIZONTAL when edge j has been horizontally
    // aligned and R is supposed to be a wide box with a narrow height.
    // Vice versa for VERTICAL.
    cola::Edge e = m_es[j];
    vpsc::Rectangle *srcR = m_rs[e.first], *tgtR = m_rs[e.second];
    double srcZ = srcR->getCentreD(dim), tgtZ = tgtR->getCentreD(dim);
    vpsc::Rectangle *lowR  = srcZ < tgtZ ? srcR : tgtR;
    vpsc::Rectangle *highR = srcZ < tgtZ ? tgtR : srcR;
    double x=0,X=0,y=0,Y=0;
    if (dim==HORIZONTAL) {
        X = highR->getMinX() - EDGE_SHAPE_BUFFER;
        x = lowR->getMaxX()  + EDGE_SHAPE_BUFFER;
        double avgY = ( lowR->getCentreY() + highR->getCentreY() ) / 2;
        y = avgY - EDGE_SHAPE_HALF_THICKNESS;
        Y = avgY + EDGE_SHAPE_HALF_THICKNESS;
    } else {
        Y = highR->getMinY() - EDGE_SHAPE_BUFFER;
        y = lowR->getMaxY()  + EDGE_SHAPE_BUFFER;
        double avgX = ( lowR->getCentreX() + highR->getCentreX() ) / 2;
        x = avgX - EDGE_SHAPE_HALF_THICKNESS;
        X = avgX + EDGE_SHAPE_HALF_THICKNESS;
    }
    R->setMinD(XDIM,x);
    R->setMaxD(XDIM,X);
    R->setMinD(YDIM,y);
    R->setMaxD(YDIM,Y);
}

/*
 * We keep a rectangle for each aligned edge. This method updates the dimensions
 * of those rectangles as the edges move (i.e. their endpoints move) during the layout
 * process.
 */
void ACALayout::recomputeEdgeShapes(Dim dim)
{
    // There is some risk of confusion here.
    // When the dimension passed is XDIM, we will take this to mean that
    // we are interested in computing non-overlap constraints in the
    // x vars. This means that it is the /vertical/ edge shapes that need
    // to be updated, i.e. the shapes representing vertically aligned
    // edges, for which overlap removal means pushing nodes off to the
    // left or right as needed.
    // Conversely, if the dimension passed is YDIM, then we want to
    // update the horizontal edge shapes and prepare for generation of
    // non-overlap constraints on the y variables.
    bool horiz = dim == YDIM;
    int e = horiz ? m_numExtraYVars : m_numExtraXVars;
    int b = m_n + e;
    vpsc::Variables &vs = horiz ? m_yvs : m_xvs;
    int N = vs.size();
    std::map<int,int> &gi2ei = horiz ? m_yGuidelineIndexToEdgeIndex :
                                       m_xGuidelineIndexToEdgeIndex;
    vpsc::Rectangles &rs = horiz ? m_yrs : m_xrs;
    cola::NonOverlapConstraints *nocs = horiz ? m_ynocs : m_xnocs;
    for (int gi = b; gi < N; ++gi) {
        int ei = gi2ei[gi];
        Rectangle *R = rs[gi];
        Dim perpDim = dim == YDIM ? XDIM : YDIM;
        updateRectForEdge(R,ei,perpDim);
        nocs->resizeShape(gi,R->width()/2.0,R->height()/2.0);
    }
}

void ACALayout::updateNodeRectsFromVars(void)
{
    for (int i=0; i<m_n; ++i) {
        m_rs[i]->moveCentreX(m_xvs[i]->finalPosition);
        m_rs[i]->moveCentreY(m_yvs[i]->finalPosition);
    }
}

void ACALayout::updateVarsFromNodeRects(void)
{
    for (int i=0; i<m_n; ++i) {
        Rectangle *r = m_rs[i];
        double x=r->getCentreX(), y=r->getCentreY();
        m_xvs[i]->desiredPosition=x;
        m_yvs[i]->desiredPosition=y;
    }
}

OrderedAlignment *ACALayout::chooseOA(void)
{
    // First form potential alignments and sort by penalty score.
    std::vector<OrderedAlignment*> oas;
    // Consider each edge.
    for (int j = 0; j < m_m; ++j) {
        // If the edge is to be ignored, or is already aligned, then skip it.
        if (m_ignoreEdge[j]) continue;
        if (edgeIsAligned(j)) continue;
        // Otherwise consider each cardinal direction.
        ACASepFlag sf;
        int sn = 1;
        while (sn < 16) {
            sf = (ACASepFlag) sn;
            sn = sn << 1;
            OrderedAlignment *oa = initOrdAlign(j, sf);
            // Skip this one if it would enforce an overlap.
            if (createsOverlap(oa)) continue;
            // Otherwise compute the penalty and store the oa.
            oa->penalty = computePenalty(j,sf);
            oas.push_back(oa);
        }
    }
    // Sort by penalty.
    std::sort(oas.begin(),oas.end(),sortOrdAlignsByPenalty);
    // Now, starting with the lowest-penalty alignments, look for a feasible one.
    OrderedAlignment *nextOA = nullptr;
    for (std::vector<OrderedAlignment*>::iterator it=oas.begin(); it!=oas.end(); ++it) {
        OrderedAlignment *oa = *it;
        bool applied = applyIfFeasible(oa);
        if (applied) {
            nextOA = oa; //needn't call completeOrdAlign, since already done by applyIfFeasible
            m_ordAligns.push_back(oa);
            break;
        }
    }
    // Clean up: delete all OAs that are not being returned.
    for (std::vector<OrderedAlignment*>::iterator it=oas.begin(); it!=oas.end(); ++it) {
        OrderedAlignment *oa = *it;
        if (oa != nextOA) {
            delete oa;
        }
    }
    return nextOA;
}

bool ACALayout::createsOverlap(OrderedAlignment *oa)
{
    int src = oa->src,
        tgt = oa->tgt;
    ACASepFlag sf = oa->sf;
    // Determine which shape is low and which is high in the dimension of interest.
    int lowIndex = sf==ACANORTH || sf==ACAWEST ? tgt : src;
    int highIndex = (lowIndex==tgt ? src : tgt);
    ACAFlag af = sepToAlignFlag(sf);
    // Determine the coordinates in the dimension of interest.
    vpsc::Rectangle *rLow = getRect(lowIndex), *rHigh = getRect(highIndex);
    double lowCoord = af==ACAHORIZ ? rLow->getCentreX() : rLow->getCentreY();
    double highCoord = af==ACAHORIZ ? rHigh->getCentreX() : rHigh->getCentreY();
    // Let L and H be the low and high shapes respectively.
    // We consider each node U which is already aligned with either L or H.
    // Any such node must have lower coord than L if it is connected to L, and
    // higher coord than H if it is connected to H. If either of those conditions
    // fails, then we predict overlap.
    bool overlap = false;
    for (int j = 0; j < m_n; j++) {
        if (j==lowIndex || j==highIndex) continue;
        vpsc::Rectangle *r = getRect(j);
        int lj = (*m_alignmentState)(lowIndex, j);
        int hj = (*m_alignmentState)(highIndex, j);
        if (lj&af || hj&af) {
            double z = af==ACAHORIZ ? r->getCentreX() : r->getCentreY();
            // low shape
            if ( lj&ACACONN && lowCoord < z ) {
                overlap = true; break;
            }
            // high shape
            if ( hj&ACACONN && z < highCoord ) {
                overlap = true; break;
            }
        }
    }
    return overlap;
}

OrderedAlignment *ACALayout::initOrdAlign(int j, ACASepFlag sf) const
{
    OrderedAlignment *oa = new OrderedAlignment();
    cola::Edge e = m_es.at(j);
    int src = e.first, tgt = e.second;
    oa->af = sepToAlignFlag(sf);
    oa->sf = sf;
    oa->dim = oa->af == ACAHORIZ ? HORIZONTAL : VERTICAL;
    oa->src = src;
    oa->tgt = tgt;
    oa->edgeIndex = j;
    return oa;
}

OrderedAlignment *ACALayout::initOrdAlign(int s, int t, ACASepFlag sf, int edgeIndex) const
{
    OrderedAlignment *oa = new OrderedAlignment();
    oa->sf = sf;
    oa->af = sepToAlignFlag(sf);
    oa->dim = oa->af == ACAHORIZ ? HORIZONTAL : VERTICAL;
    oa->src  = s;
    oa->tgt = t;
    oa->edgeIndex = edgeIndex;
    return oa;
}

void ACALayout::completeOrdAlign(OrderedAlignment *oa)
{
    // In order to create a SeparationConstraint, we need to work out which
    // among the src and tgt of the ordalign is to be on the "left" and which
    // on the "right" -- this in the VPSC sense, which is literal when we work in
    // the x-dimension, and where "left" means "above" and "right" means
    // "below" when we work in the y-dimension.
    int src = oa->src, tgt = oa->tgt;
    int l = -1, r = -1;
    if (oa->af == ACAHORIZ) {
        if (oa->sf & ACAEAST) {
            l = src;
            r = tgt;
        } else {
            l = tgt;
            r = src;
        }
    } else {
        if (oa->sf & ACASOUTH) {
            l = src;
            r = tgt;
        } else {
            l = tgt;
            r = src;
        }
    }
    COLA_ASSERT(l >= 0);
    COLA_ASSERT(r >= 0);
    vpsc::Rectangle *rl = getRect(l), *rr = getRect(r);
    // Determine dimensions.
    vpsc::Dim sepDim   = oa->af == ACAHORIZ ? vpsc::XDIM : vpsc::YDIM;
    vpsc::Dim alignDim = oa->af == ACAHORIZ ? vpsc::YDIM : vpsc::XDIM;
    // Create the separation constraint.
    double sep = oa->af == ACAHORIZ ?
                (rl->width()+rr->width())/2.0 : (rl->height()+rr->height())/2.0;
    oa->separation = new cola::SeparationConstraint(sepDim,l,r,sep);
    // Create the alignment constraint.
    double pos = oa->af==ACAHORIZ ?
                (rl->getCentreY()+rr->getCentreY())/2.0 : (rl->getCentreX()+rr->getCentreX())/2.0;
    oa->alignment = new cola::AlignmentConstraint(alignDim,pos);
    oa->alignment->addShape(l,oa->offsetSrc);
    oa->alignment->addShape(r,oa->offsetTgt);
}

void ACALayout::updateGraph(void) {
    COLA_ASSERT(m_graph != nullptr);
    m_graph->updateNodesFromRects();
    updateSepMatrix();
}

void ACALayout::updateSepMatrix(void) {
    COLA_ASSERT(m_graph != nullptr);
    SepMatrix &M = m_graph->getSepMatrix();
    const std::map<size_t, id_type> &ix2id = m_graph->getColaGraphRep().ix2id;
    updateSepMatrix(M, ix2id);
}

void ACALayout::updateSepMatrix(SepMatrix &M, const std::map<size_t, id_type> &ix2id) {
    for (OrderedAlignment *oa : m_ordAligns) {
        id_type id1 = ix2id.at(oa->src), id2 = ix2id.at(oa->tgt);
        CardinalDir dir = CardinalDir::EAST; // initialise to quiet warning about default case we'll never reach
        switch(oa->sf) {
            case ACAEAST: dir = CardinalDir::EAST; break;
            case ACASOUTH: dir = CardinalDir::SOUTH; break;
            case ACAWEST: dir = CardinalDir::WEST; break;
            case ACANORTH: dir = CardinalDir::NORTH; break;
        default: break;
        }
        M.setCardinalOP(id1, id2, dir);
    }
}

OrderedAlignment *ACALayout::mostRecentOA(void)
{
    OrderedAlignment *oa = nullptr;
    if (!m_ordAligns.empty()) {
        oa = m_ordAligns.back();
    }
    return oa;
}

/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/// Feasibility Checking

// This preprocessor directive controls whether ACA does a full VPSC solve, or
// a mere VPSC satisfy, on each of the four checks performed in the applyIfFeasible
// method. If defined, we do a full solve on each check.
// Experimentation has shown better performance when this IS defined.
#define ACA_FULLSOLVE_ON_EACH_CHECK
/**
 * Check using VPSC whether the proposed OA is feasible, and, if so, apply it.
 * Return value says whether or not OA was feasible (and hence applied).
 */
bool ACALayout::applyIfFeasible(OrderedAlignment *oa)
{
    // First check whether it is simply a bad separation.
    if (badSeparation(oa->src, oa->tgt, oa->sf)) return false;
    // If not, then proceed to use VPSC to check feasibility.
    // Save the state before we make any changes.
    pushState();
    // Get dimensions.
    bool horiz = oa->dim==HORIZONTAL;
    Dim sepd = horiz ? XDIM : YDIM;
    Dim alnd = horiz ? YDIM : XDIM;
    // Assign vectors based on dimension.
    Variables   &sepv = horiz ? m_xvs : m_yvs;
    Constraints &sepc = horiz ? m_xcs : m_ycs;
    Rectangles  &sepr = horiz ? m_xrs : m_yrs;
    Variables   &alnv = horiz ? m_yvs : m_xvs;
    Constraints &alnc = horiz ? m_ycs : m_xcs;
    Rectangles  &alnr = horiz ? m_yrs : m_xrs;
    // NOC objects
    cola::NonOverlapConstraints *sepnocs = horiz ? m_xnocs : m_ynocs;
    cola::NonOverlapConstraints *alnnocs = horiz ? m_ynocs : m_xnocs;
    // Get map from guideline indices to edge indices for alignment dimension.
    std::map<int,int> &gi2ei = horiz ? m_yGuidelineIndexToEdgeIndex :
                                       m_xGuidelineIndexToEdgeIndex;
    // Get map from auxiliary variable indices to indices in 'extendedRS' vector, which
    // contains pointers to all auxiliary rectangles.
    std::map<int,int> &auxRectToExtRS = horiz ? m_yAuxRectIndexInExtendedRS :
                                                m_xAuxRectIndexInExtendedRS;
    // Complete the OA, building its Sep and Align compound constraints.
    completeOrdAlign(oa);
    // Extend vectors.
    oa->separation->generateSeparationConstraints(sepd,sepv,sepc,sepr);
    oa->alignment->generateVariables(alnd,alnv);
    oa->alignment->generateSeparationConstraints(alnd,alnv,alnc,alnr);
    // New rectangle for aligned edge:
    Rectangle *newRect = makeRectForOA(oa);
    alnr.push_back(newRect);
    int newRectIndex = alnv.size()-1;
    gi2ei.insert(std::pair<int,int>(newRectIndex,oa->edgeIndex));
    alnnocs->addShape(newRectIndex, newRect->width()/2.0, newRect->height()/2.0, 1, exemptionSetForEdge(oa->edgeIndex));
    m_extendedRS.push_back(newRect);
    auxRectToExtRS.insert(std::pair<int,int>(newRectIndex,m_extendedRS.size()-1));
    // Save this state. We will return to this state if the OrderedAlignment
    // in question turns out to be feasible, and we want to keep it.
    pushState();
    // Save current coordinates of nodes.
    pushRectCoords();
    // Check for satisfiability of the separation and alignment constraints,
    // and of non-overlap constraints in both dimensions.
    bool feasible = true;
    IncSolver *seps = nullptr, *alns = nullptr;
    IncSolver *sepnocsolv = nullptr, *alnnocsolv = nullptr;
    // On previous calls to this function, some constraint(s) may have been
    // marked as unsatisfiable. Here we clear any such flags before beginning
    // the new satisfiability tests.
    for (Constraints::iterator it=sepc.begin(); it!=sepc.end(); ++it) {
        Constraint *c = *it;
        c->unsatisfiable = false;
    }
    for (Constraints::iterator it=alnc.begin(); it!=alnc.end(); ++it) {
        Constraint *c = *it;
        c->unsatisfiable = false;
    }
    // Set variables to represent current node positions.
    updateVarsFromNodeRects();
    // Begin satisfiability tests:
    // 1. The Separation Constraint:
    seps = satisfy(sepv,sepc,feasible);
    // 2. The Alignment Constraint:
    if (feasible) alns = satisfy(alnv,alnc,feasible);

    // 3. Non-overlap in the dimension of the alignment constraint:
    if (feasible) {
        updateNodeRectsFromVars();
        recomputeEdgeShapes(alnd);
        alnnocs->generateSeparationConstraints(alnd,alnv,alnc,alnr);
        alnnocsolv = satisfy(alnv,alnc,feasible);
    }

    // 4. Non-Overlap in the dimension of the separation constraint:
    if (feasible) {
        updateNodeRectsFromVars();
        recomputeEdgeShapes(sepd);
        sepnocs->generateSeparationConstraints(sepd,sepv,sepc,sepr);
        sepnocsolv = satisfy(sepv,sepc,feasible);
    }

    if (!feasible) {
        // Pop state twice to return to state before this function call.
        popState();
        popState();
        // Restore rectangle coordinates.
        popRectCoords();
        // Remove all records of the new edge shape.
        gi2ei.erase(newRectIndex);
        alnnocs->removeShape(newRectIndex);
        auxRectToExtRS.erase(newRectIndex);
        m_extendedRS.pop_back();
    } else { // OA is feasible. We will keep it.
#ifndef ACA_FULLSOLVE_ON_EACH_CHECK
        // So far we have only tried to satisfy the constraints.
        // Now we should actually find an optimal solution.
        alnnocsolv->solve();
        sepnocsolv->solve();
#endif
        // Accept the new node positions.
        updateNodeRectsFromVars();
        // Pop state just once to get rid of the NOCs, but keep the new ordered alignment.
        popState();
        // We do not need the other state and node coords that we saved.
        dropState();
        dropRectCoords();
    }
    delete seps;
    delete alns;
    delete sepnocsolv;
    delete alnnocsolv;
    return feasible;
}

// Constructs a solver and attempts to satisfy the passed constraints on the
// passed vars. Sets the bool passed by reference according to whether it was
// possible to satisfy the constraints.
vpsc::IncSolver *ACALayout::satisfy(Variables &vs, Constraints &cs, bool &sat)
{
    IncSolver *solv = new IncSolver(vs,cs);
    try {
#ifdef ACA_FULLSOLVE_ON_EACH_CHECK
        solv->solve();
#else
        solv->satisfy();
#endif
        sat = true;
    } catch (UnsatisfiedConstraint uc) {
        sat = false;
    }
    for (Constraints::iterator it=cs.begin(); it!=cs.end(); ++it) {
        Constraint *c = *it;
        if (c->unsatisfiable) {
            sat = false;
            break;
        }
    }
    return solv;
}

// Say whether the proposed separation is a bad one.
bool ACALayout::badSeparation(int j, ACASepFlag sf)
{
    // If allowed separations have been set, then check whether sf is allowed.
    if (m_allowedSeps.size()>0) {
        ACASepFlag allowed = m_allowedSeps.at(j);
        if ( (sf&allowed) != sf ) return true; // it is a bad separation
    }
    // If we are /not/ doing aggressive ordering, then
    // check if sf is ruled out for reversing existing order.
    if (!m_aggressiveOrdering) {
        cola::Edge e = m_es.at(j);
        int src = e.first, tgt = e.second;
        vpsc::Rectangle *rs = getRect(src), *rt = getRect(tgt);
        double dx = rt->getCentreX() - rs->getCentreX();
        double dy = rt->getCentreY() - rs->getCentreY();
        ACASepFlag currPos = vectorToSepFlag(dx,dy);
        bool conflict = propsedSepConflictsWithExistingPosition(sf,currPos);
        if (conflict) return true; // it is a bad separation
    }
    return false;
}

bool ACALayout::badSeparation(int l, int r, ACASepFlag sf)
{
    // If we are /not/ doing aggressive ordering, then
    // first check if sf is ruled out for reversing existing order.
    //// Do aliasing
    //l = alias(l);
    //r = alias(r);
    if (!m_aggressiveOrdering) {
        vpsc::Rectangle *rs = getRect(l), *rt = getRect(r);
        double dx = rt->getCentreX() - rs->getCentreX();
        double dy = rt->getCentreY() - rs->getCentreY();
        ACASepFlag currPos = vectorToSepFlag(dx,dy);
        bool conflict = propsedSepConflictsWithExistingPosition(sf,currPos);
        if (conflict) return true; // it is a bad separation
    }
    return false;
}

/// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/// Penalty evaluation

double ACALayout::computePenalty(int j, ACASepFlag sf)
{
    double p = 0;
    cola::Edge e = m_es.at(j);
    int src = e.first, tgt = e.second;
    // Basic penalty:
    if (m_favourLongEdges) {
        // Length:
        p += lengthPenaltyForEdge(j);
    } else {
        // Deflection:
        p += deflectionForEdge(j,sf);
    }
    // Bend points:
    if (m_addBendPointPenalty) p += bendPointPenalty(src,tgt,sf);
    // Leaves:
    if (m_postponeLeaves) p += leafPenalty(src,tgt);
    //
    return p;
}

double ACALayout::lengthPenaltyForEdge(int j)
{
    cola::Edge e = m_es.at(j);
    vpsc::Rectangle *src = m_rs.at(e.first), *tgt = m_rs.at(e.second);
    double sx=src->getCentreX(), sy=src->getCentreY(), tx=tgt->getCentreX(), ty=tgt->getCentreY();
    double dx=tx-sx, dy=ty-sy;
    double l=sqrt(dx*dx+dy*dy);
    return 1 - l/m_lengthUpperBound;
    // FIXME: We really should update the "length upper bound" on each iteration, and make
    // sure that it actually is an upper bound on all edge lengths. We need to do this so
    // that this score can be between 0 and 1.
}

/* Compute a score in [0.0, 1.0] measuring how far edge j is deflected
 * from horizontal or vertical, depending on the passed alignment flag.
 *
 * If t is the angle the edge makes with the positive x-axis, then the
 * score we return is sin^2(t) for horizontal alignments, and cos^2(t)
 * for vertical.
 *
 * So smaller deflection scores mean edges that are closer to axis-aligned.
 */
double ACALayout::deflectionForEdge(int j, ACASepFlag sf)
{
    cola::Edge e = m_es.at(j);
    int src = e.first, tgt = e.second;
    // Do aliasing
    src = alias(src);
    tgt = alias(tgt);
    vpsc::Rectangle *s = getRect(src), *t = getRect(tgt);
    // Start with centre points of the nodes.
    double sx=s->getCentreX(), sy=s->getCentreY(), tx=t->getCentreX(), ty=t->getCentreY();
    // Check offsets.
    EdgeOffset offset = getEdgeOffsetForCompassDirection(j,sf);
    if (sf&ACAEASTWEST) {
        // It is a horizontal alignment, so the offset is to the y-dimension.
        sy += offset.first;
        ty += offset.second;
    } else {
        // It is a vertical alignment, so the offset is to the x-dimension.
        sx += offset.first;
        tx += offset.second;
    }
    return deflection(sx,sy,tx,ty,sf);
}

double ACALayout::deflection(double sx, double sy, double tx, double ty, ACASepFlag sf)
{
    double dx = tx-sx, dy = ty-sy;
    double dx2 = dx*dx, dy2 = dy*dy;
    double l = dx2 + dy2;
    double dfl = sf==ACAWEST || sf==ACAEAST ? dy2/l : dx2/l;
    return dfl;
}

double ACALayout::bendPointPenalty(int src, int tgt, ACASepFlag sf)
{
    double penalty = 0;
    ACAFlag af = sepToAlignFlag(sf);
    ACAFlag op = af==ACAHORIZ ? ACAVERT : ACAHORIZ;
    std::set<int> &deg2Nodes = m_useNonLeafDegree ? m_nldeg2Nodes : m_deg2Nodes;
    std::multimap<int,int> &nbrs = m_useNonLeafDegree ? m_nlnbrs : m_nbrs;
    // First check whether src would be made into a bendpoint.
    if (deg2Nodes.count(src)!=0) {
        // Find neighbour j of src which is different from tgt, by iterating over nbrs of src.
        int j = 0;
        std::pair< std::multimap<int,int>::iterator, std::multimap<int,int>::iterator > range;
        range = nbrs.equal_range(src);
        for (std::multimap<int,int>::iterator it=range.first; it!=range.second; ++it) {
            j = it->second;
            if (j != tgt) break;
        }
        // Now check if they are aligned in the opposite dimension. If so, add penalty.
        int as = (*m_alignmentState)(src,j);
        if (as & op) penalty += BP_PENALTY;
    }
    // Now check whether tgt would be made into a bendpoint.
    if (deg2Nodes.count(tgt)!=0) {
        // Find neighbour j of tgt which is different from src, by iterating over nbrs of tgt.
        int j = 0;
        std::pair< std::multimap<int,int>::iterator, std::multimap<int,int>::iterator > range;
        range = nbrs.equal_range(tgt);
        for (std::multimap<int,int>::iterator it=range.first; it!=range.second; ++it) {
            j = it->second;
            if (j != src) break;
        }
        // Now check if they are aligned in the opposite dimension. If so, add penalty.
        int as = (*m_alignmentState)(tgt,j);
        if (as & op) penalty += BP_PENALTY;
    }
    return penalty;
}

double ACALayout::leafPenalty(int src, int tgt)
{
    double penalty = LEAF_PENALTY;
    return m_leaves.count(src)!=0 || m_leaves.count(tgt)!=0 ? penalty : 0;
}

EdgeOffset ACALayout::getEdgeOffsetForCompassDirection(int j, ACASepFlag sf)
{
    EdgeOffset offset(0,0);
    // If offsets have been specified for this sep flag, then retrieve
    // the offset of the given index.
    std::map<ACASepFlag,EdgeOffsets>::iterator it = m_edgeOffsets.find(sf);
    if (it!=m_edgeOffsets.end()) {
        EdgeOffsets offsets = it->second;
        offset = offsets.at(j);
    }
    return offset;
}



static const double LIMIT = 100000000;

static void reduceRange(double& val)
{
    val = std::min(val, LIMIT);
    val = std::max(val, -LIMIT);
}

void ACALayout::outputInstanceToSVG(std::string instanceName)
{
    // Save current locale
    char* originalLocale = setlocale(LC_NUMERIC, nullptr);
    // Set locale to "C" to enforce decimal point
    setlocale(LC_NUMERIC, "C");

    std::string filename;
    if (!instanceName.empty())
    {
        filename = instanceName;
    }
    else
    {
        filename = "acalayout-debug";
    }
    filename += ".svg";
    FILE *fp = fopen(filename.c_str(), "w");
    if (fp == nullptr)
    {
        return;
    }
    double minX = LIMIT;
    double minY = LIMIT;
    double maxX = -LIMIT;
    double maxY = -LIMIT;
    // Find the bounds of the diagram.
    for (size_t i = 0; i < m_rs.size(); ++i)
    {
        double rMinX = m_rs[i]->getMinX();
        double rMaxX = m_rs[i]->getMaxX();
        double rMinY = m_rs[i]->getMinY();
        double rMaxY = m_rs[i]->getMaxY();
        reduceRange(rMinX);
        reduceRange(rMaxX);
        reduceRange(rMinY);
        reduceRange(rMaxY);
        if (rMinX > -LIMIT)
        {
            minX = std::min(minX, rMinX);
        }
        if (rMaxX < LIMIT)
        {
            maxX = std::max(maxX,rMaxX);
        }
        if (rMinY > -LIMIT)
        {
            minY = std::min(minY, rMinY);
        }
        if (rMaxY < LIMIT)
        {
            maxY = std::max(maxY, rMaxY);
        }
    }
    minX -= 50;
    minY -= 50;
    maxX += 50;
    maxY += 50;

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<svg xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns=\"http://www.w3.org/2000/svg\" width=\"100%%\" height=\"100%%\" viewBox=\"%g %g %g %g\">\n", minX, minY, maxX - minX, maxY - minY);

    // Output source code to generate this ACALayout instance.
    fprintf(fp, "<!-- Source code to generate this instance:\n");
    fprintf(fp, "#include <vector>\n");
    fprintf(fp, "#include <utility>\n");
    fprintf(fp, "#include \"libcola/cola.h\"\n");
    fprintf(fp, "#include \"libdialect/aca.h\"\n");
    fprintf(fp, "using namespace cola;\n");
    fprintf(fp, "using namespace dialect;\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    CompoundConstraints ccs;\n");
    fprintf(fp, "    std::vector<Edge> es;\n");
    fprintf(fp, "    EdgeLengths eLengths;\n");
    fprintf(fp, "    double defaultEdgeLength=%g;\n", m_idealLength);
    fprintf(fp, "    std::vector<vpsc::Rectangle*> rs;\n");
    fprintf(fp, "    vpsc::Rectangle *rect = nullptr;\n\n");
    for (size_t i = 0; i < m_rs.size(); ++i)
    {
        fprintf(fp, "    rect = new vpsc::Rectangle(%g, %g, %g, %g);\n",
               m_rs[i]->getMinX(), m_rs[i]->getMaxX(),
               m_rs[i]->getMinY(), m_rs[i]->getMaxY());
        fprintf(fp, "    rs.push_back(rect);\n\n");
    }
    for(auto e=m_es.begin();e!=m_es.end();++e) {
        fprintf(fp, "    es.push_back(std::make_pair(%u, %u));\n", e->first, e->second);
    }
    
    fprintf(fp, "\n");

    if (m_edgeLengths.size() > 0)
    {
        fprintf(fp, "    eLengths.resize(%d);\n", (int) m_edgeLengths.size());
        for (size_t i = 0; i < m_edgeLengths.size(); ++i)
        {
            fprintf(fp, "    eLengths[%d] = %g;\n", (int) i, m_edgeLengths[i]);
        }
        fprintf(fp, "\n");
    }

    for (cola::CompoundConstraints::iterator c = m_ccs.begin(); 
            c != m_ccs.end(); ++c)
    {
        (*c)->printCreationCode(fp);
    }  
    
    fprintf(fp, "    ACALayout alg(rs, es, ccs, defaultEdgeLength, eLengths);\n");
    fprintf(fp, "    alg.setAvoidNodeOverlaps(%s);\n", (m_preventOverlaps) ? "true" : "false");
    if (m_rc)
    {
        m_rc->printCreationCode(fp);
        fprintf(fp, "    alg.setClusterHierarchy(cluster%llu);\n",
                (unsigned long long) m_rc);
    }
    
    /* OPTIONAL ACA SETTINGS */

    // node aliases
    fprintf(fp, "\n    std::map<int,int> aliases;\n");
    for (std::map<int,int>::iterator alias = m_nodeAliases.begin();
        alias != m_nodeAliases.end(); ++alias) {
        fprintf(fp, "    aliases[%d] = %d;\n", alias->first, alias->second);
    }
    fprintf(fp, "    alg.setNodeAliases(aliases);\n");
    
    // edge ignores
    fprintf(fp, "\n    std::vector<bool> edgeIgnores;\n");
    for (std::vector<bool>::iterator eig = m_ignoreEdge.begin();
        eig != m_ignoreEdge.end(); ++eig) {
        if ((*eig)) {
            fprintf(fp, "    edgeIgnores.push_back(true);\n");
        } else {
            fprintf(fp, "    edgeIgnores.push_back(false);\n");
        }
    }
    fprintf(fp, "    alg.ignoreEdges(edgeIgnores);\n");
    

    // edge ignores
    fprintf(fp, "\n    std::vector<bool> ignoreNodeForOPWithOffsets;\n");
    for (std::vector<bool>::iterator nig = m_ignoreNodeForOPWithOffsets.begin();
        nig != m_ignoreNodeForOPWithOffsets.end(); ++nig) {
        if ((*nig)) {
            fprintf(fp, "    ignoreNodeForOPWithOffsets.push_back(true);\n");
        } else {
            fprintf(fp, "    ignoreNodeForOPWithOffsets.push_back(false);\n");
        }
    }
    fprintf(fp, "    alg.ignoreNodesForOPWithOffsets(ignoreNodeForOPWithOffsets);\n");

    // allowed separations
    fprintf(fp, "\n    std::vector<ACASepFlag> sepFlags;\n");
    for (std::vector<ACASepFlag>::iterator sf = m_allowedSeps.begin();
        sf != m_allowedSeps.end(); ++sf) {
        fprintf(fp, "    sepFlags.push_back((ACASepFlag) %d);\n", (*sf));
    }
    fprintf(fp, "    alg.setAllowedDirections(sepFlags);\n");
    
    // edge offsets 
    fprintf(fp, "\n    EdgeOffset eo;\n");
    for (std::map<ACASepFlag,EdgeOffsets>::iterator pair = m_edgeOffsets.begin();
        pair != m_edgeOffsets.end(); ++pair) {
        
        EdgeOffsets edgeOffs = pair->second;
        unsigned long long rand = (unsigned long long) &edgeOffs;
        fprintf(fp, "\n    EdgeOffsets eoffs%llu;\n", rand);
        
        for (std::vector<EdgeOffset>::iterator offset = edgeOffs.begin();
            offset != edgeOffs.end(); ++offset) {
            fprintf(fp, "    eo = std::make_pair(%g, %g);\n", offset->first, offset->second);
            fprintf(fp, "    eoffs%llu.push_back(eo);\n", rand);
        }
        
        fprintf(fp, "    alg.setAlignmentOffsetsForCompassDirection((ACASepFlag) %d, eoffs%llu);\n", pair->first, rand);
    }
    
    // node exemptions
    //if (m_fdlayout) {
		fprintf(fp, "\n\n    std::vector<unsigned> exempt;\n");
		std::set<ShapePair> pairs = m_nocExemptions->getExemptPairs();
		for (std::set<ShapePair>::iterator p = pairs.begin();
				p != pairs.end(); ++p)
		{
			fprintf(fp, "    exempt.push_back(%d);\n", (*p).index1());
			fprintf(fp, "    exempt.push_back(%d);\n", (*p).index2());
			fprintf(fp, "    alg.addGroupOfNonOverlapExemptRectangles(exempt);\n");
			fprintf(fp, "    exempt.clear();\n");
		}
    //}

    fprintf(fp, "\n\n");
    //fprintf(fp, "    alg.layout();\n");
    fprintf(fp, "    alg.createAlignments();\n");

    // Additional output that can be useful for debugging:
    /*
    fprintf(fp, "    std::string output = \"\";\n");

	fprintf(fp, "    // Show initial state.\n");
    fprintf(fp, "    cola::ConstrainedFDLayout fdl = cola::ConstrainedFDLayout(rs,es,defaultEdgeLength,eLengths);\n");
	fprintf(fp, "    fdl.setConstraints(ccs);\n");
	fprintf(fp, "    //fdl.runOnce(true,true);\n");
	fprintf(fp, "    fdl.outputInstanceToSVG(\"ACA-test0-0000\");\n");

	fprintf(fp, "    // Remove overlaps.\n");
	fprintf(fp, "    alg.removeOverlaps();\n");
	fprintf(fp, "    alg.getFDLayout()->outputInstanceToSVG(\"ACA-test0-0001\");\n");

	fprintf(fp, "    int k = 2;\n");
	fprintf(fp, "    char buf [100];\n");
	fprintf(fp, "    while(alg.createOneAlignment()) {\n");
	fprintf(fp, "    	// Write SVG.\n");
	fprintf(fp, "    	cola::ConstrainedFDLayout *fdlayout = alg.getFDLayout();\n");
	fprintf(fp, "    	sprintf(buf,\"ACA-test0-%%04d\",k);\n");
	fprintf(fp, "    	std::string s(buf);\n");
	fprintf(fp, "    	fdlayout->outputInstanceToSVG(s);\n");
	fprintf(fp, "    	output += \"===================================================================\\n\";\n");
	fprintf(fp, "    	output += s+\"\\n\";\n");
	fprintf(fp, "    	cola::OrderedAlignment *oa = alg.mostRecentOA();\n");
	fprintf(fp, "    	if (oa) {\n");
	fprintf(fp, "    		sprintf(buf,\"Alignment:\\n  Low: %%d, High: %%d, Edge: %%d, Type: %%d\\n\",\n");
    fprintf(fp, "    	    	oa->src,oa->tgt,oa->edgeIndex,oa->af);\n");
	fprintf(fp, "    	    output += std::string(buf);\n");
	fprintf(fp, "       }\n");
	fprintf(fp, "       // Unsat constraints\n");
	fprintf(fp, "       std::set<std::string> xStrings;\n");
	fprintf(fp, "       cola::UnsatisfiableConstraintInfos *unsatX = alg.unsatX();\n");
	fprintf(fp, "       for (UnsatisfiableConstraintInfos::iterator it=unsatX->begin(); it!=unsatX->end(); ++it) {\n");
	fprintf(fp, "           xStrings.insert((**it).toString());\n");
	fprintf(fp, "       }\n");
	fprintf(fp, "       std::set<std::string> yStrings;\n");
	fprintf(fp, "       cola::UnsatisfiableConstraintInfos *unsatY = alg.unsatY();\n");
	fprintf(fp, "       for (UnsatisfiableConstraintInfos::iterator it=unsatY->begin(); it!=unsatY->end(); ++it) {\n");
	fprintf(fp, "           yStrings.insert((**it).toString());\n");
	fprintf(fp, "       }\n");
	fprintf(fp, "       output += \"Unsat X:\\n\";\n");
	fprintf(fp, "       for (std::set<std::string>::iterator it=xStrings.begin(); it!=xStrings.end(); ++it) {\n");
	fprintf(fp, "           output += (*it)+\"\\n\";\n");
	fprintf(fp, "       }\n");
	fprintf(fp, "       output += \"Unsat Y:\\n\";\n");
	fprintf(fp, "       for (std::set<std::string>::iterator it=yStrings.begin(); it!=yStrings.end(); ++it) {\n");
	fprintf(fp, "           output += (*it)+\"\\n\";\n");
	fprintf(fp, "       }\n");
	fprintf(fp, "       k++;\n");
	fprintf(fp, "    }\n");
    */

    // done!
    fprintf(fp, "};\n\n");
    fprintf(fp, "-->\n");

    if (m_rc)
    {
        m_rc->computeBoundingRect(m_rs);
        fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"Clusters\">\n");
        m_rc->outputToSVG(fp);
        fprintf(fp, "</g>\n");
    }

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"Rects\">\n");
    for (size_t i = 0; i < m_rs.size(); ++i)
    {
        double minX = m_rs[i]->getMinX();
        double maxX = m_rs[i]->getMaxX();
        double minY = m_rs[i]->getMinY();
        double maxY = m_rs[i]->getMaxY();
    
        fprintf(fp, "<rect id=\"rect-%u\" x=\"%g\" y=\"%g\" width=\"%g\" "
                "height=\"%g\" style=\"stroke-width: 1px; stroke: black; "
                "fill: %s; fill-opacity: 0.3;\" />\n",
                (unsigned) i, minX, minY, maxX - minX, maxY - minY,
                m_ignoreNodeForOPWithOffsets[i] ? "red" : "blue");
    }
    fprintf(fp, "</g>\n");

    fprintf(fp, "<g inkscape:groupmode=\"layer\" "
            "inkscape:label=\"Edges\">\n");
    // edges
    for(int k = 0; (size_t)k < m_es.size(); ++k) {
        unsigned i = m_es[k].first;
        unsigned j = m_es[k].second;
        fprintf(fp, "<path d=\"M %g %g L %g %g\" "
                "style=\"stroke-width: 1px; stroke: %s;\" />\n",
                m_rs[i]->getCentreX(),
                m_rs[i]->getCentreY(),
                m_rs[j]->getCentreX(),
                m_rs[j]->getCentreY(),
                // if the edge is ignored for alignments, draw it pink
                m_ignoreEdge[k] ? "red" : "black"
        );
    }
    
    // alignments
    for (cola::CompoundConstraints::iterator c = m_ccs.begin(); 
            c != m_ccs.end(); ++c) {
        
        AlignmentConstraint* ac = dynamic_cast<AlignmentConstraint*>((*c));
        if (ac) {
            double gap = ac->position();
            
            // XDIM
            double fstX = gap, fstY = minY, sndX = gap, sndY = maxY;
            if (ac->dimension() == vpsc::YDIM) {
               // YDIM
            	fstX = minX, fstY = gap, sndX = maxX, sndY = gap;
            }
            fprintf(fp, "<path d=\"M %g %g L %g %g\" "
                    "style=\"stroke-width: 1px; stroke: green;\" "
                    "stroke-dasharray=\"10,10\" />\n",
                    fstX, fstY, sndX, sndY);
            
        }
    }
    
    
    fprintf(fp, "</g>\n");

    fprintf(fp, "</svg>\n");
    fclose(fp);
    // Restore locale
    setlocale(LC_NUMERIC, originalLocale);
}

} // namespace dialect
