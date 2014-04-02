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
 * Author: Michael Wybrow
*/


#ifndef AVOID_ORTHOGLAYOUT_H
#define AVOID_ORTHOGLAYOUT_H

#include "libcola/cola.h"

namespace topology {

/** 
 *  @brief  This class can be passed to libavoid to extend it to provide
 *          orthogonal topology improvement functionality.
 *
 *  You should instantiate this class with libcola information about 
 *  constraints on objects in the diagram and pass it to 
 *  Avoid::Router::setTopologyAddon().
 */
class AvoidTopologyAddon : public Avoid::TopologyAddonInterface
{
    public:
        /**
         * @brief Constructs a AvoidTopologyAddon instance for a set of COLA
         *        diagram constraints.
         *
         * Passing libavoid an instance of this class with given libcola 
         * rectangles, constraints and cluster hierarchy will cause libavoid
         * to perform a post-processing step after routing where it will 
         * move nodes to unify unnecessary kinks where this doesn't violate 
         * constraints on the diagram.
         *
         * This information is usually available from an existing libcola
         * instance.
         *
         * @param[in] rs   A vector of vpsc::Rectangle objects subject to the
         *                 constraints.
         * @param[in] cs   A set of cola::CompoundConstraint objects to apply
         *                 to rs.
         * @param[in] ch   A cola::RootCluster cluster hierarchy specification.
         * @param[in] map  A cola::VariableIDMap specifying a mapping from
         *                 the Rectangle indexes in rs to ShapeRef IDs in this
         *                 libavoid instance.
         * @param[in] moveLimit  The maximum length of a centre S-bend 
         *                       connector segments to attempt to improve
         *                       (default 120.0).
         */
        AvoidTopologyAddon(vpsc::Rectangles& rs, cola::CompoundConstraints& cs, 
                cola::RootCluster *ch, cola::VariableIDMap& map,
                const double moveLimit = 120);

        ~AvoidTopologyAddon();
        Avoid::TopologyAddonInterface *clone(void) const;

        void improveOrthogonalTopology(Avoid::Router *router);
        bool outputCode(FILE *fp) const;
        bool outputDeletionCode(FILE *fp) const;

    private:
        vpsc::Rectangles m_rectangles;
        cola::CompoundConstraints m_constraints;
        cola::RootCluster *m_cluster_hierarchy;
        cola::VariableIDMap m_id_map;
        double m_move_limit;
};


}

#endif
