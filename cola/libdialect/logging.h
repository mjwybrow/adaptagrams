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

#ifndef DIALECT_LOGGING_H
#define DIALECT_LOGGING_H

#include <string>
#include <vector>

#include "libdialect/commontypes.h"

namespace dialect {

struct Logger {
    std::string outputDir = "";
    std::string prefix = "";
    std::vector<std::string> contents;
    std::vector<std::string> names;
    bool hasOutputDir = false;
    bool hasPrefix = false;
    //! Control whether the logger writes to stdout the name of each file recorded:
    bool verbose = false;
    //! If true, then when logging TGLF for a graph, also log SVG:
    bool addSVG = false;

    //! Sometimes it is useful to be able to record a number that should be used in a
    //! prefix by various processes. Helps when separate processes are being logged.
    //! This is not used by the Logger itself. It is only a convenience for use by clients.
    unsigned nextLoggingIndex = 0;

    //! @brief  Standard constructor.
    //! @param[in] outputDir  Optional name of output directory in which files should be written.
    //! @param[in] prefix  Optional prefix to put at start of all file names.
    //! @param[in] verbose  Set true if you want the logger to write each recorded name to stdout.
    //! @param[in] addSVG  Set true if you want the logger to also write an SVG file, each time it
    //!     writes a TGLF file.
    //!
    //! @note  If no outputDir is given, then strings are only saved in this object.
    //! @note  If a prefix is given, an underscore will be automatically appended to it.
    Logger(std::string outputDir = "", std::string prefix = "",
        bool verbose = false, bool addSVG = false);

    //! @brief  Record a string, optionally to a named file in the output dir.
    //! @param[in] content  The string to be recorded.
    //! @param[in] name  The name under which to record it.
    void log(std::string content, std::string name="");

    //! @brief  Convenience method to log the TGLF for a Graph.
    //! @param[in] G  The Graph whose TGLF is to be logged.
    //! @param[in] name  The filename for the TGLF file WITHOUT the ".tglf" suffix. The suffix
    //!                  is automatically added for you.
    void log(Graph &G, std::string name="");

    //! @brief  Given a filename, prepend the output directory and prefix (if given) in order
    //!         to write the full path.
    //! @return The full path.
    std::string writeFullPathForFilename(std::string name);

};


} // namespace dialect

#endif // DIALECT_LOGGING_H
