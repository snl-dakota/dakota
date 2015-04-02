/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Implementation of class RandomNumberGenerator.

    NOTES:

        See notes of RandomNumberGenerator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jan 29 19:16:21 2004 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the RandomNumberGenerator class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <cstdlib>
#include "../include/Math.hpp"
#include "../include/RandomNumberGenerator.hpp"





/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;






/*
================================================================================
Begin Namespace
================================================================================
*/
namespace eddy {
    namespace utilities {





/*
================================================================================
Static Member Data Definitions
================================================================================
*/








/*
================================================================================
Mutators
================================================================================
*/








/*
================================================================================
Accessors
================================================================================
*/








/*
================================================================================
Public Methods
================================================================================
*/
void
RandomNumberGenerator::Seed(
    unsigned int seed
    )
{
    srand(seed);

} // RandomNumberGenerator::Seed

int
RandomNumberGenerator::Rand(
    )
{
    return rand();
}


double
RandomNumberGenerator::GaussianReal(
    double mu,
    double sigma
    )
{
    double x1, x2, w, y;

    do
    {
        x1 = UniformReal(-1.0, 1.0);
        x2 = UniformReal(-1.0, 1.0);

        w = x1 * x1 + x2 * x2;

    } while ( (w >= 1.0) || (w == 0.0) );

    w = Math::Sqrt( (-2.0 * Math::Ln( w ) ) / w );
    y = x1 * w;
    return mu + y*sigma;

} // RandomNumberGenerator::GaussianReal

double
RandomNumberGenerator::CauchyReal(
    double mu,
    double sigma
    )
{
    return mu + sigma * Math::Tan(Math::Pi()*UniformReal(-0.5, 0.5));

} // RandomNumberGenerator::CauchyReal


double
RandomNumberGenerator::UniformReal(
    double lo,
    double hi
    )
{
    return static_cast<double>(rand()) / RAND_MAX * (hi-lo) + lo;

} // RandomNumberGenerator::UniformReal

/*
================================================================================
Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Subclass Overridable Methods
================================================================================
*/








/*
================================================================================
Private Methods
================================================================================
*/








/*
================================================================================
Structors
================================================================================
*/








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace utilities
} // namespace eddy

