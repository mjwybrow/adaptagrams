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

#include <iostream>
#include <cmath>

#include "libvpsc/assertions.h"

using std::string;
using std::cout;
using std::endl;

void test(string msg, bool test) {
    cout << msg << ": ";
    if (test) {
        cout << "true" << endl;
    } else {
        cout << "false" << endl;
    }
}

int main(void) {
    double nz = -0.0;
    double pz = 0.0;

    test("-0.0 < 0.0", nz < pz);
    test("-0.0 signbit", std::signbit(nz));
    test("0.0 signbit", std::signbit(pz));
    test("-pz signbit", std::signbit(-pz));
    test("-nz signbit", std::signbit(-nz));
    test("-0.0 == 0", -0.0 == 0);
    test("0.0 == 0", 0.0 == 0);
    test("-0.0 == 0.0", -0.0 == 0.0);
    test("-30 signbit", std::signbit(-30));
    test("60 signbit", std::signbit(60));

    COLA_ASSERT(std::signbit(-0.0));
    COLA_ASSERT(!std::signbit(0.0));

}
