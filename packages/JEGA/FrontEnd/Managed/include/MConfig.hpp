/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        General configuration for the managed project.

    NOTES:



    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Sep 25 10:51:11 2006 - Original Version (JE)

================================================================================
*/
#pragma once


/// An empty argument for macros.
#define EMPTY_ARG()

#pragma unmanaged
#include <../Utilities/include/Logging.hpp>
#include <cstddef>
#include <utilities/include/int_types.hpp>
#pragma managed

#if _MSC_VER < 1400

/**
 * \brief A macro for declaring a managed, garbage collected, reference type
 *        class.
 *
 * This resolves to the leading qualifiers followed by the name of the class.
 *
 * \param vis The visibility of the class.  One of public, private, or
 *            protected.
 * \param cname The name of the class being declared.
 */
#define MANAGED_CLASS(vis, cname) __gc vis class cname

/**
 * \brief A macro for declaring a managed, garbage collected, reference type
 *        interface.
 *
 * This resolves to the leading qualifiers followed by the name of the
 * interface.
 *
 * \param vis The visibility of the interface.  One of public, private, or
 *            protected.
 * \param iname The name of the interface being declared.
 */
#define MANAGED_INTERFACE(vis, iname) __gc vis __interface iname

/**
 * \brief A macro for declaring a managed, garbage collected, reference type,
 *        abstract class.
 *
 * This resolves to the entire class declaration including the class name.
 *
 * \param vis The visibility of the class.  One of public, private, or
 *            protected.
 * \param cname The name of the class being declared.
 */
#define MANAGED_ABSTRACT_CLASS(vis, cname) __abstract MANAGED_CLASS(vis, cname)

/**
 * \brief A macro for forward declaring a managed, garbage collected, reference
 *        type class.
 *
 * \param vis The visibility of the class.  One of public, private, or
 *            protected.
 * \param cname The name of the class being forward declared.
 */
#define MANAGED_CLASS_FORWARD_DECLARE(vis, cname) MANAGED_CLASS(vis, cname);

/**
 * \brief A macro for forward declaring a managed interface.
 *
 * \param vis The visibility of the interface.  One of public, private, or
 *            protected.
 * \param cname The name of the interface being forward declared.
 */
#define MANAGED_INTERFACE_FORWARD_DECLARE(vis, iname) \
    MANAGED_INTERFACE(vis, iname);

/**
 * \brief A macro that resolves to the operator used to allocate objects on
 *        the managed heap.
 */
#define MANAGED_GCNEW __gc new

/**
 * \brief A macro for declaring enumeration types.
 *
 * This resolves to all leading qualifiers and the name of the enumeration.
 *
 * \param ename The name of the enumeration.
 */
#define MANAGED_ENUM(ename) __value enum ename

/**
 * \brief A shorthand for the character sequence used to denote the declaration
 *        of an object handle.
 */
#define MOH __gc *

/**
 * \brief A macro to expand to the managed extension for indicating a method
 *        override.
 */
#define MANAGED_OVERRIDE

/**
 * \brief A macro that expands to an explicit cast in the case where generics
 *        are not supported and thus a downcast is necessary and no cast at
 *        all when they are supported.
 *
 * \param totype The type to which the \a arg is to be casted.
 * \param arg The item that is to be casted or left as is depending on whether
 *            or not generics are supported.
 */
#define NON_GENERIC_CAST(totype, arg) static_cast< totype >(arg)

/**
 * \brief A macro to perform a safe_cast or equivalent.
 *
 * \param totype The destination type of the cast.
 * \param arg The item to cast to a \a totype.
 */
#define SAFE_CAST(totype, arg) __try_cast< totype >(arg)

/**
 * \brief A macro to retrieve an item from a list using an index.
 *
 * \param list The list from which to retrieve the item.
 * \param index The integral index of the item of interest.
 */
#define MANAGED_LIST_ITEM(list, index) list->Item[index]

/**
 * \brief A macro to explicitely box a value type into a reference type object.
 *
 * \param item The item to box.
 */
#define MANAGED_BOX(item) __box(item)

/**
 * \brief A macro to expand to the declaration of an enumerator that can be
 *        used to iterate a list.
 *
 * \param dtype The type of the items held in the list.  This is only used if
 *              using a version of the project that supports generics.
 */
#define MANAGED_IENUMERATOR(dtype) System::Collections::IEnumerator

/**
 * \brief A macro to name the Dispose methods.
 *
 * This is necessary because in 2005, Dispose is a reserved name and cannot
 * be used directly.  So the macro resolves to a name is actually not Dispose.
 */
#define MANAGED_DISPOSE Dispose

/**
 * \brief A macro to resolve to the managed version of a null object handle.
 *
 * This is the null pointer value (0, NULL, 0x0, etc.) in 2003 and the
 * nullptr keyword in >= 2005.
 */
#define MANAGED_NULL_HANDLE 0x0

#else

#define MANAGED_CLASS(vis, cname) vis ref class cname

#define MANAGED_INTERFACE(vis, iname) vis interface class iname

#define MANAGED_ABSTRACT_CLASS(vis, cname) \
    MANAGED_CLASS(vis, cname) abstract

#define MANAGED_CLASS_FORWARD_DECLARE(vis, cname) \
    MANAGED_CLASS(EMPTY_ARG(), cname);

#define MANAGED_INTERFACE_FORWARD_DECLARE(vis, iname) \
    MANAGED_INTERFACE(EMPTY_ARG(), iname);

#define MANAGED_GCNEW gcnew

#define MANAGED_ENUM(ename) enum class ename

#define MOH ^

#define MANAGED_OVERRIDE override

#define NON_GENERIC_CAST(totype, arg) arg

#define SAFE_CAST(totype, arg) safe_cast< totype >(arg)

#define MANAGED_LIST_ITEM(list, index) list->default[index]

#define MANAGED_IENUMERATOR(dtype) \
    System::Collections::Generic::IEnumerator< dtype >

#define MANAGED_BOX(item) item

#define MANAGED_DISPOSE DoDispose

#define MANAGED_NULL_HANDLE nullptr

#endif



/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {
        namespace Managed {



/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
// Forward declare the MDesign for creation of the DesignVector below.
MANAGED_CLASS_FORWARD_DECLARE(public, MDesign)

// Do the same for the MSolution
MANAGED_CLASS_FORWARD_DECLARE(public, MSolution)

#if _MSC_VER < 1400

#define VECTOR_CLASS(dtype, name) \
    MANAGED_CLASS(public, name) : \
        public System::Collections::ArrayList \
    { \
        public: \
            name() : System::Collections::ArrayList() {} \
            name(System::Int32 initCap) : \
                System::Collections::ArrayList(initCap) {} \
            name(System::Collections::ICollection MOH copy) : \
                System::Collections::ArrayList(copy) {} \
    };

#else

#define VECTOR_CLASS(dtype, name) \
    MANAGED_CLASS(public, name) : \
        public System::Collections::Generic::List< dtype > \
    { \
        public: \
            name() : System::Collections::Generic::List< dtype >() {} \
            name(System::Int32 initCap) : \
                System::Collections::Generic::List< dtype >(initCap) {} \
            name(System::Collections::Generic::IEnumerable< dtype > MOH copy) : \
                System::Collections::Generic::List< dtype >(copy) {} \
    };

#endif

VECTOR_CLASS(System::Double, DoubleVector)
VECTOR_CLASS(DoubleVector MOH, DoubleMatrix)
VECTOR_CLASS(System::Int32, IntVector)
VECTOR_CLASS(System::String MOH, StringVector)
VECTOR_CLASS(MDesign MOH, DesignVector)
VECTOR_CLASS(MSolution MOH, SolutionVector)


/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA
