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

#include "libvpsc/assertions.h"
#include "libdialect/quadaction.h"

using namespace dialect;

int main(void) {

    Nbr n0(0, 1, 0);
    COLA_ASSERT(n0.octalCode() == 0);
    COLA_ASSERT(n0.deflection() == 0);
    Nbr n1(0, 1, 1);
    COLA_ASSERT(n1.octalCode() == 1);
    COLA_ASSERT(n0.deflection() == 1/2);
    Nbr n2(0, 0, 1);
    COLA_ASSERT(n2.octalCode() == 2);
    COLA_ASSERT(n0.deflection() == 0);
    Nbr n3(0, -1, 1);
    COLA_ASSERT(n3.octalCode() == 3);
    COLA_ASSERT(n0.deflection() == 1/2);
    Nbr n4(0, -1, 0);
    COLA_ASSERT(n4.octalCode() == 4);
    COLA_ASSERT(n0.deflection() == 0);
    Nbr n5(0, -1, -1);
    COLA_ASSERT(n5.octalCode() == 5);
    COLA_ASSERT(n0.deflection() == 1/2);
    Nbr n6(0, 0, -1);
    COLA_ASSERT(n6.octalCode() == 6);
    COLA_ASSERT(n0.deflection() == 0);
    Nbr n7(0, 1, -1);
    COLA_ASSERT(n7.octalCode() == 7);
    COLA_ASSERT(n0.deflection() == 1/2);

    return 0;
}
