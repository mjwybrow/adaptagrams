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

#include <string>
#include <vector>
#include <iostream>

#include "libdialect/io.h"
#include "libdialect/graphs.h"
#include "libdialect/logging.h"

using namespace dialect;

using std::string;
using std::vector;

Logger::Logger(std::string outputDir, std::string prefix, bool verbose, bool addSVG)
    : outputDir(outputDir), prefix(prefix), verbose(verbose), addSVG(addSVG)
{
    if (outputDir.size() > 0) hasOutputDir = true;
    if (prefix.size() > 0) {
        hasPrefix = true;
        prefix += "_";
    }
}

void Logger::log(std::string content, std::string name) {
    // At least we record the content.
    contents.push_back(content);
    // Was a name given?
    if (name.size() > 0) {
        names.push_back(name);
        // Write to stdout?
        if (verbose) std::cout << "Log: " << name << std::endl;
        // Do we have an output directory?
        if (hasOutputDir) {
            // Set prefix if provided.
            if (hasPrefix) name = prefix + name;
            // Write.
            string path = outputDir + name;
            writeStringToFile(content, path);
        }
    }
}

void Logger::log(Graph &G, std::string name) {
    string tglf = G.writeTglf();
    log(tglf, name+".tglf");
    if (addSVG) {
        string svg = G.writeSvg();
        log(svg, name+".svg");
    }
}

string Logger::writeFullPathForFilename(std::string name) {
    if (hasPrefix) name = prefix + name;
    string path = outputDir + name;
    return path;
}
