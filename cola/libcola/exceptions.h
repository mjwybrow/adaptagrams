/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2008  Monash University
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
*/

#ifndef SEEN_LIBCOLA_EXCEPTIONS_H
#define SEEN_LIBCOLA_EXCEPTIONS_H
#include <string>
#include <sstream>

namespace cola {
class CompoundConstraint;

struct InvalidConstraint {
    InvalidConstraint(CompoundConstraint *c):constraint(c) {}
    CompoundConstraint *constraint;
};


class InvalidVariableIndexException
{
public:
    InvalidVariableIndexException(CompoundConstraint *c, unsigned i) 
        : constraint(c),
          index(i)
    { }
    std::string what() const throw()
    {
        std::stringstream s;
        s << "Invalid variable index: " << index;
        return s.str();
    }
    CompoundConstraint *constraint;
    unsigned index;
};


} // namespace cola
#endif //SEEN_LIBCOLA_EXCEPTIONS_H
