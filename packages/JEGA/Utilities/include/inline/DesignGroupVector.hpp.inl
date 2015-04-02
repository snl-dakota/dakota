/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class DesignGroupVector.

    NOTES:

        See notes of DesignGroupVector.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri Oct 10 10:31:27 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the DesignGroupVector class.
 */




/*
================================================================================
Includes
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {





/*
================================================================================
Inline Mutators
================================================================================
*/








/*
================================================================================
Inline Accessors
================================================================================
*/








/*
================================================================================
Inline Public Methods
================================================================================
*/







/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Inline Private Methods
================================================================================
*/








/*
================================================================================
Inline Structors
================================================================================
*/



template<class InputIterator>
inline
DesignGroupVector::DesignGroupVector(
   InputIterator _First,
   InputIterator _Last
   ) :
      base_type(_First, _Last)
{
}

template<class InputIterator>
inline
DesignGroupVector::DesignGroupVector(
   InputIterator _First,
   InputIterator _Last,
   const allocator_type& _Al
   ) :
      base_type(_First, _Last, _Al)
{
}

inline
DesignGroupVector::DesignGroupVector() :
   base_type()
{
}

inline
DesignGroupVector::DesignGroupVector(
   const allocator_type& _Al
   ) :
      base_type(_Al)
{
}

inline
DesignGroupVector::DesignGroupVector(
   size_type _Count
   ) :
      base_type(_Count)
{
}

inline
DesignGroupVector::DesignGroupVector(
   size_type _Count,
   const value_type& _Val
   ) :
      base_type(_Count, _Val)
{
}

inline
DesignGroupVector::DesignGroupVector(
   size_type _Count,
   const value_type& _Val,
   const allocator_type& _Al
   ) :
      base_type(_Count, _Val, _Al)
{
}

inline
DesignGroupVector::DesignGroupVector(
   const DesignGroupVector& _Right
   ) :
      base_type(_Right)
{
}



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

