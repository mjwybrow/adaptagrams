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
 * Author: Michael Wybrow
*/


#ifndef AVOID_ORTHOGLAYOUT_H
#define AVOID_ORTHOGLAYOUT_H

#include "libcola/cola.h"

namespace topology {


class AvoidTopologyAddon : public Avoid::TopologyAddonInterface
{
    public:
        AvoidTopologyAddon(vpsc::Rectangles& rs, cola::CompoundConstraints& cs, 
                cola::RootCluster *ch, cola::VariableIDMap& map);
        ~AvoidTopologyAddon();
        Avoid::TopologyAddonInterface *clone(void) const;

        void improveOrthogonalTopology(Avoid::Router *router);
        bool outputCode(FILE *fp) const;
        bool outputDeletionCode(FILE *fp) const;

    private:
        vpsc::Rectangles rectangles;  // Currently unused, for debugging.

        cola::CompoundConstraints constraints;
        cola::RootCluster *clusterHierarchy;
        cola::VariableIDMap idMap;
};


}

#endif
