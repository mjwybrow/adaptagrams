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
*/

#ifndef VPSC_EXCEPTIONS_H
#define VPSC_EXCEPTIONS_H

#include <vector>
namespace vpsc {
class Constraint;
struct UnsatisfiableException {
	std::vector<Constraint*> path;
};
struct UnsatisfiedConstraint {
	UnsatisfiedConstraint(Constraint& c):c(c) {}
	Constraint& c;
};
} // namespace vpsc

#endif // VPSC_EXCEPTIONS_H
