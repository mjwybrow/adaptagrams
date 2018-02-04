/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libvpsc - A solver for the problem of Variable Placement with 
 *           Separation Constraints.
 *
 * Copyright (C) 2009  Monash University
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

#ifndef VPSC_ASSERTIONS_H
#define VPSC_ASSERTIONS_H

#ifdef NDEBUG 

  #define COLA_ASSERT(expr)  static_cast<void>(0)

#else // Not NDEBUG

  // sstream needs ::strcpy_s under MinGW so include cstring.
  #include <cstring>

  #include <sstream>
  #include <cassert>

  #if defined(USE_ASSERT_EXCEPTIONS) 

    // String seems to be missing on MinGW's gcc,
    // so define it here if it is missing.
    #ifndef __STRING
      #define __STRING(x) #x
    #endif

    #if !defined(__ASSERT_FUNCTION)
      #define COLA_ASSERT(expr) \
          if (!(expr)) { \
              throw vpsc::CriticalFailure(__STRING(expr), __FILE__, __LINE__); \
          }
    #else
      #define COLA_ASSERT(expr) \
          if (!(expr)) { \
              throw vpsc::CriticalFailure(__STRING(expr), __FILE__, __LINE__, \
                      __ASSERT_FUNCTION); \
          }
    #endif

  #else
    #define COLA_ASSERT(expr)  assert(expr)
  #endif

namespace vpsc { 

// Critical failure: either something went wrong, or (more likely) there
// was infeasible input.
class CriticalFailure
{
    public:
        CriticalFailure(const char *expr, const char *file, int line, 
                const char *function = nullptr)
            : expr(expr),
              file(file),
              line(line),
              function(function)
        {
        }
        std::string what() const
        {
            std::stringstream s;
            s << "ERROR: Critical assertion failed.\n";
            s << "  expression: " << expr << "\n";
            s << "  at line " << line << " of " << file << "\n";
            if (function)
            {
                s << "  in: " << function << "\n";
            }

            return s.str();
        }
    private:
        const char *expr;
        const char *file;
        int line;
        const char *function;
};

}

#endif // NDEBUG


#endif // VPSC_ASSERTIONS_H

