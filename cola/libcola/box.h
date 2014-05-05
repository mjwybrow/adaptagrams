/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2014  Monash University
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
 * Author(s):  Michael Wybrow
 *
*/

#ifndef LIBCOLA_BOX_H
#define LIBCOLA_BOX_H

#include <cstdio>

#include "libvpsc/rectangle.h"


namespace cola {

/**
 * @brief A box consisting of four edge values for representing padding or 
 *        margins for rectangles.
 */
class Box
{
    public:
        /**
         * @brief Constructs a padding or margin box consisting of a value for
         *        each edge.
         *
         * Values should be nonnegative.
         *
         * @param[in] xMin  Minimum horizontal value.
         * @param[in] xMax  Maximum horizontal value.
         * @param[in] yMin  Minimum vertical value.
         * @param[in] yMax  Maximum vertical value.
         */
        Box(double xMin, double xMax, double yMin, double yMax);
        /**
         * @brief Constructs a padding or margin box consisting of a value for
         *        each edge.
         *
         * Values should be nonnegative.
         *
         * @param[in] all   The value to be used for all four edges.
         */
        Box(double all);
        /**
         * @brief Constructs an empty padding or margin box.
         */
        Box();
        ~Box();

        bool empty(void) const;
        double min(size_t dim) const;
        double max(size_t dim) const;
        void outputCode(FILE *fp) const;
        vpsc::Rectangle rectangleByApplyingBox(
                const vpsc::Rectangle rectangle) const;

   private:
        static double nonNegative(double value);

        double m_min[2];
        double m_max[2];
};

} // namespace cola
#endif
