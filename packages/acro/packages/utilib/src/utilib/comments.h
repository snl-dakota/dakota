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
 * \file comments.h
 *
 * Defines stream operators for skipping over white space.
 *
 * \author William E. Hart
 */

#ifndef utilib_comments_h
#define utilib_comments_h

#include <utilib_config.h>
#include <utilib/std_headers.h>

namespace utilib {

/**
 * Move the stream counter forward, passing over whitespace and 
 * comment lines, which begin with a '#'.
 * The argument \a line_counter returns the number of lines that were 
 * skipped.
 */
std::istream& comment_lines(std::istream& ins, int& line_counter);

/**
 * Move the stream counter forward, passing over whitespace and 
 * comment lines, which begin with a '#'.
 */
inline std::istream& comment_lines(std::istream& ins)
{
int tmp=0;
return (comment_lines(ins,tmp));
}

/**
 * Move the stream counter forward, passing over whitespace.
 * The argument \a line_counter returns the number of newline symbols
 * that were skipped.
 */
std::istream& whitespace(std::istream& ins, int& line_counter);

/**
 * Move the stream counter forward, passing over whitespace.
 * This serves the same role as 'ws', but this operator does not work
 * under Solaris 6.0 compilers.
 */
inline std::istream& whitespace(std::istream& ins)
{
int tmp=0;
return whitespace(ins,tmp);
}

} // namespace utilib

#endif
