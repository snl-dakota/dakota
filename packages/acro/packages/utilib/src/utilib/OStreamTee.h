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

/**
 * \file OStreamTee.h
 *
 * Defines the utilib::OStreamTee class.
 */

#ifndef utilib_OStreamTee_h
#define utilib_OStreamTee_h

#include <iostream>
#include <sstream>
#include <cstdio>

namespace utilib {

class OStreamTee
{
private:
   class teebuffer : public std::basic_streambuf<char>
   {
      typedef std::basic_streambuf<char> streambuf_t;
   public:
      teebuffer(streambuf_t * buf1, streambuf_t * buf2)
         : buffer1(buf1), buffer2(buf2)
      {}

      virtual int overflow(int c)
      {
         if (c == EOF)
            return !EOF;
         else
         {
            int const ans1 = buffer1->sputc(c);
            int const ans2 = buffer2->sputc(c);
            return ans1 == EOF || ans2 == EOF ? EOF : c;
         }
      }

      virtual int sync()
      {
         int ans1 = buffer1->pubsync();
         int ans2 = buffer2->pubsync();
         return ans1 || ans2 ? -1 : 0;
      }

      streambuf_t * buffer1;
      streambuf_t * buffer2;
   };

public:
   OStreamTee(std::ostream& _original)
      : out(),
        original(_original),
        tee(out.rdbuf(), _original.rdbuf())
   {
      original.rdbuf(&tee);
   }

   ~OStreamTee()
   {
      original.rdbuf(tee.buffer2);
   }

   std::stringstream out;

private:
   std::ostream&  original;
   teebuffer      tee;
};

} // namespace utilib

#endif // utilib_OStreamTee_h
