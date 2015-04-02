/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of utility generator functions.

    NOTES:

        See notes under various sections of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Apr 03 09:31:49 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains generator specific utility functions.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_GENERATOR_HPP
#define EDDY_UTILITIES_GENERATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
#include "config.hpp"







/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/








/*
================================================================================
Namespace Aliases
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace utilities {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/







/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/
/**
 * \brief A generator of copies of the elements in a range.
 *
 * The usage of this generator is a bit nonintuitive.  Because functions like
 * std::generate do not pass arguments into their generators, this class keeps
 * a copy of the iterator and increments it at each use.  It does not check
 * to see that that iterator is valid in any way or that it has not yet reached
 * the end.  That must be handled by the function using the generate.  An
 * example of the usage would be:
 * \code
  vector<double> v1(10, 0.0);
  vector<double> v2(10, 1.0);
  std::generate(v1.begin(), v1.end(), copy_gen(v2.begin()));
   \endcode
 *
 * after which v1 will have duplicate contents of v2.
 *
 * \param T The data type pointed to by iterator type FwdIt.
 * \param FwdIt The forward iterator type that dereferences to a T.
 */
template <typename T, typename FwdIt>
class copy_gen
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The current element to be copied.
        FwdIt _curr;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// The activation operator of this generator.
        /**
         * \return A duplicate of the item pointed to by the current iterator
         *         and also increment that iterator.
         */
        T
        operator ()(
            )
        {
            return T(*(_curr++));
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a copy generator to start with the supplied
         *        iterator and increment it as many times as it gets called.
         *
         * \param b The iterator at which to begin.
         */
        copy_gen(
            const FwdIt& b
            ) :
                _curr(b)
        {
        }

}; // copy_gen





/*
================================================================================
Functions
================================================================================
*/


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
// Not using an Inlined Functions File.



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_GENERATOR_HPP
