/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class BitManipulator.

    NOTES:

        See notes under section "Class Definition" of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jun 12 10:11:58 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the BitManipulator class.
 */



/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_ALGORITHMS_BITMANIPULATOR_HPP
#define JEGA_ALGORITHMS_BITMANIPULATOR_HPP






/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <iosfwd>
#include <vector>
#include <utilities/include/bit_mask.hpp>
#include <../Utilities/include/JEGATypes.hpp>







/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class Design;
        class DesignTarget;
    }
}


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
namespace JEGA {
    namespace Algorithms {







/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class BitManipulator;




/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/








/*
================================================================================
Class Definition
================================================================================
*/

/// A class providing functionality for doing bitwise operations.
/**
 * This class serves as a base class or member for any operator that wishes to
 * manipulate the bits of design variables and Designs in general.
 *
 * This class provides functionality for binary encoding.  It can convert
 * between reals and integers, provide information about the encodings, and
 * do a few other things.
 */
class BitManipulator
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The data type used to represent a binary string by this encoder.
        typedef
        eddy::utilities::dynamic_bit_mask<>
        BinaryString;

    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// Bit counts for each design variable.
        /**
         * This vector contains the counts of how many bits are required to
         * represent the respective variable.
         */
        std::vector<eddy::utilities::uint16_t> _nbits;

        eddy::utilities::uint32_t _tnBits;

        /// Conversion multipliers for each design variable.
        /**
         * This vector contains the numbers needed to multiply the variable
         * values in order to create an integer representation (dependent on
         * decimal precision).
         */
        JEGA::DoubleVector _mults;

        /// Minimum allowable values for each design variable.
        /**
         * This vector contains the minimum representation for each variable.
         * It is here to avoid having to extract it every time.
         */
        JEGA::DoubleVector _mins;

        /// The target known by this manipulator.
        /**
         * The target is used to compute the data stored in the above vectors
         * and to retrieve problem specific information.
         */
        const JEGA::Utilities::DesignTarget& _target;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:





    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /// Returns the target known by this object (const).
        /**
         * The target must have been supplied to the constructor.  This allows
         * const-qualified access to it.
         *
         * \return The DesignTarget known and used by this manipulator.
         */
        inline
        const JEGA::Utilities::DesignTarget&
        GetDesignTarget(
            ) const;




    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Creates an integral representation of a variable of \a des.
        /**
         * This method converts the \a dv'th design variable value of \a des
         * into an integer by multiplying it out according to the precision
         * (x 10^prec)
         *
         * The precision is obtained from the DesignVariableInfo object stored
         * for design variable #\a dv with the known DesignTarget.
         *
         * \param des The design whose \a dv'th variable is to be converted.
         * \param dv The index of the variable to convert.
         * \return The \a dv'th variable of \a des multiplied by 10^prec.
         */
        eddy::utilities::int64_t
        ConvertToShiftedInt(
            const JEGA::Utilities::Design& des,
            std::size_t dv
            ) const;

        /// Creates an integral representation of a real number.
        /**
         * This method converts \a val, which should be the \a dv'th design
         * variable value of some design into an integer by multiplying it out
         * according to the precision.
         *
         * The precision is obtained from the DesignVariableInfo object stored
         * for design variable #\a dv with the known DesignTarget.
         *
         * \param val The design variable value that is to be converted.
         * \param dv The index of the variable to convert.
         * \return The \a dv'th variable of \a des multiplied by 10^prec.
         */
        eddy::utilities::int64_t
        ConvertToShiftedInt(
            double val,
            std::size_t dv
            ) const;

        /// Returns a shifted integral value back into a double.
        /**
         * This method converts \a val, which should be the integer
         * representation of the \a dv'th design variable value of some design
         * (probably created using ConvertToShiftedInt) back into a double
         * representation by dividing it out according to the precision
         * (/ 10^prec).
         *
         * The precision is obtained from the DesignVariableInfo object stored
         * for design variable #\a dv with the known DesignTarget.
         *
         * \param val The integral design variable value that is to be
         *            returned to double format.
         * \param dv The index of the variable to convert.
         * \return The \a dv'th variable of \a des divided by 10^prec.
         */
        inline
        double
        ConvertFromShiftedInt(
            eddy::utilities::int64_t val,
            std::size_t dv
            ) const;

        /// Returns the number of bits in \a of whose value is \a withVal
        /**
         * \a withVal of true will be interpreted as 1 and false as 0.
         *
         * \param of The integral value whose bits are to be counted.
         * \param withVal true if counting 1's and false if counting 0's.
         * \return The number of bits in \a of with the desired value.
         */
        static
        eddy::utilities::uint8_t
        CountBits(
            eddy::utilities::int64_t of,
            bool withVal = true
            );

        /**
         * \brief Returns the number of bits required to represent the \a dv'th
         *        design variable.
         *
         * The value returned is read directly from the _nbits container.
         *
         * \param dv The Design variable of interest.
         * \return The number of bits required to represent the \a dv'th
         *         design variable as an integer.
         */
        inline
        eddy::utilities::uint16_t
        GetNumberOfBits(
            std::size_t dv
            ) const;

        /// Compares the contents of this object to recomputed values.
        /**
         * This method compares this objects current contents to what
         * they should be according to the target.  It returns true
         * if they match up and false otherwise.  It does not update
         * the values either way.
         *
         * \return true if the data held in here is current and false
         *         otherwise.
         */
        bool
        AreContentsCurrent(
            ) const;

        /// Refreshes the data stored in this object.
        /**
         * This method recomputes the \a _mults, \a _nbits, and \a _mins data
         * according to the target.
         */
        void
        ReValidateContents(
            );

        /**
         * \brief Returns the total number of bits required to represent an
         *        entire genome.
         *
         * This is equivolent to summing the return values of
         * GetNumberOfBits(dv) for all dv (0 to ndv-1).
         *
         * \return The number of bits needed to represent an entire design in
         *         integer form.
         */
        inline
        eddy::utilities::uint32_t
        GetTotalNumOfBits(
            ) const;

        /**
         * \brief Prints all the bits of all of the design variables to
         *        \a stream with spaces between the chromosomes.
         *
         * \param stream The stream into which to print the bit string.
         * \param des The Design whose design variable bits are to be printed.
         */
        void
        PrintAllBits(
            std::ostream& stream,
            const JEGA::Utilities::Design& des
            ) const;

        /// Prints the significant bits of \a dv in \a des to \a stream.
        /**
         * This will convert the \a dv'th design variable of \a des into an
         * integer according to it's precision and then print as many bits
         * as are required to represent the full range of the shifted variable.
         *
         * \param stream The stream into which to print the bits.
         * \param des The Design whose \a dv bits are to be printed.
         * \param dv The Design variable of interest.
         */
        void
        PrintBits(
            std::ostream& stream,
            const JEGA::Utilities::Design& des,
            std::size_t dv
            ) const;

        /// Prints the requested bits of \a val to \a stream.
        /**
         * The resulting output from this for the value 10 with default lobit
         * and hibit parameters would be:
         * \verbatim
           00000000000000000000000000000000000000000000000000000000000000001010
           \endverbatim
         * The result for the value 10 with lobit=0 and hibit=8 would be
         * \verbatim
            00001010
           \endverbatim
         *
         * \param stream The stream into which to print the bits of \a val
         * \param val The integer whose bits are to be printed to \a stream
         * \param lobit The index of the least significant bit to be printed.
         * \param hibit The index of the most significant bit to be printed.
         */
        static
        void
        PrintBits(
            std::ostream& stream,
            eddy::utilities::int64_t val,
            eddy::utilities::uint16_t lobit = 0,
            eddy::utilities::uint16_t hibit = 63
            );

        /// Converts bit # \a bit of \a val from a 1 to a 0 or visa versa.
        /**
         * For example, ToggleBit(0, 8) would return 9 as 100 became 101.
         *
         * \param bit The index of the bit to be toggled.
         * \param val The value in which the bit is to be toggled.
         * \return The resulting value with the toggled bit.
         */
        inline static
        eddy::utilities::int64_t
        ToggleBit(
            eddy::utilities::uint16_t bit,
            eddy::utilities::int64_t val
            );


    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


    protected:


    private:





    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        /**
         * \brief Converts the supplied double to a binary string.
         *
         * The double is truncated prior to conversion.
         *
         * \param v The double value to represent as a binary string.
         * \return The binary string representing the whole number portion of
         *         \a v.
         */
        static
        BinaryString
        ToBinaryString(
            double v
            );

        /// Converts the supplied binary string into a double.
        /**
         * The resulting double will be a whole number.
         *
         * \param bstr The binary string to convert to a double.
         * \return The double value of the supplied binary string.
         */
        static
        double
        ToDouble(
            const BinaryString& bstr
            );




    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs and initializes a BitManipulator using \a target.
        /**
         * \param target The target used by this manipulator to compute all
         *               required conversion data, etc.
         */
        BitManipulator(
            const JEGA::Utilities::DesignTarget& target
            );

}; // class BitManipulator


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA







/*
================================================================================
Include Inlined Methods File
================================================================================
*/
#include "./inline/BitManipulator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_ALGORITHMS_BITMANIPULATOR_HPP
