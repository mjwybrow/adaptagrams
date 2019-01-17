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

#include <vector>
#include <string>
#include <iostream>

#include "libdialect/quadaction.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;
using dialect::lookupQuadActions;

int main(void) {

    vector<string> qas = lookupQuadActions(2, 2, 2, 2, 15);
    cout << "Number of QAs: " << qas.size() << endl;
    for (auto s : qas) {
        cout << s << endl;
    }

}
