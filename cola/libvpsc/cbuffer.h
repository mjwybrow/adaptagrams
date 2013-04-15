/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2005-2008  Monash University
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
 *
 */
#ifndef VPSC_CBUFFER_H
#define VPSC_CBUFFER_H

#include <vector>

namespace vpsc {
    class Constraint;
    class CBuffer {
    public:
        CBuffer(std::vector<Constraint*>& l,
                const unsigned maxsize=5) 
                 : master_list(l), maxsize(maxsize), size(0) {
            buffer.resize(maxsize);
            load();
        }
        void reset() { size=0; }
        void load();
        Constraint* mostViolated();
    private:
        std::vector<Constraint*>& master_list;
        std::vector<Constraint*> buffer;
        const unsigned maxsize;
        unsigned size;
    };
}

#endif // VPSC_CBUFFER_H

