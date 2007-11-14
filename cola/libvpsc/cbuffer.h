/*
 * vim: set cindent 
 * vim: ts=4 sw=4 et tw=0 wm=0
 */
/**
 * @file libvpsc/cbuffer.h
 * @brief 
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
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
