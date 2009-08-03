/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 *
 * Copyright (C) 2009  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the 
 * library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * Author(s):   Michael Wybrow <mjwybrow@users.sourceforge.net>
*/

#ifndef AVOID_ASSERTIONS_H
#define AVOID_ASSERTIONS_H

#include <sstream>
#include <cassert>

#ifndef ASSERT

#ifdef NDEBUG 

  #define ASSERT(expr)  static_cast<void>(0)

#else // Not NDEBUG

  #if defined(USE_CASSERT) 
    #define ASSERT(expr)  assert(expr)
  #else

    // String seems to be missing on MinGW's gcc,
    // so define it here if it is missing.
    #ifndef __STRING
      #define __STRING(x) #x
    #endif

    #if !defined(__ASSERT_FUNCTION)
      #define ASSERT(expr) \
          if (!(expr)) { \
              throw Avoid::AssertionFailure(__STRING(expr), __FILE__, __LINE__); \
          }
    #else
      #define ASSERT(expr) \
          if (!(expr)) { \
              throw Avoid::AssertionFailure(__STRING(expr), __FILE__, __LINE__, \
                      __ASSERT_FUNCTION); \
          }
    #endif

    #define ASSERTION_EXCEPTIONS

  #endif

#endif // NDEBUG

#endif // ASSERT

namespace Avoid { 


// Asserion failure: This will be thrown when an assertion fails.
class AssertionFailure
{
    public:
        AssertionFailure(const char *expr, const char *file, int line, 
                const char *function = NULL)
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


#endif // AVOID_ASSERTIONS_H

