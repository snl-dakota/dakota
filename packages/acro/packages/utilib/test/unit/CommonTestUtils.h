/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

#ifndef TEST_UNIT_TESTUTILS_h
#define TEST_UNIT_TESTUTILS_h

#include <iostream>

#include <cxxtest/TestSuite.h>
#include <cxxtest/ValueTraits.h>
#include <cxxtest/StdValueTraits.h>

#define TEST_WHAT_LIMIT(EXCEPTION, STRING, LIMIT)       \
   if ( true ) {                                        \
      std::string msg(EXCEPTION.what());                \
      std::string test(STRING);                         \
                                                        \
      /* remove the "File: line#: " */                  \
      size_t pos = msg.find(":");                       \
      if ( pos != std::string::npos )                   \
         pos = msg.find(":", pos+1);                    \
      if ( pos == std::string::npos )                   \
         pos = 0;                                       \
      else                                              \
         pos += 2;                                      \
                                                        \
      size_t end = 0;                                   \
      if ( LIMIT )                                      \
         end = test.size();                             \
      else                                              \
      {                                                 \
         /* strip trailing whitespace */                \
         msg.find_last_not_of(" \n\t");                 \
         if ( end < pos )                               \
            end = std::string::npos;                    \
         else                                           \
            end = end - pos + 1;                        \
      }                                                 \
                                                        \
      TS_ASSERT_EQUALS(test, msg.substr(pos,end));      \
      if ( test != msg.substr(pos, end) )               \
         TS_WARN(msg);                                  \
   } else static_cast<void>(0)

#define TEST_WHAT(EXCEPTION, STRING)  TEST_WHAT_LIMIT(EXCEPTION, STRING, true)

#endif // define TEST_UNIT_TESTUTILS_h
