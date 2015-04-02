/*
================================================================================
    PROJECT:

        Eddy C++ Utilities Project

    CONTENTS:

        Definition of class RandomNumberGenerator.

    NOTES:

        See notes under Class Definition section of this file.

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
 * \brief Contains the definition of the RandomNumberGenerator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_UTILITIES_RANDOMNUMBERGENERATOR_HPP
#define EDDY_UTILITIES_RANDOMNUMBERGENERATOR_HPP







/*
================================================================================
Includes
================================================================================
*/
#include "config.hpp"
#include "int_types.hpp"






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
class RandomNumberGenerator;







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
/// A class providing random number generation capabilities.
/**
 * The methods of this class are all static and can be used to retrieve random
 * numbers of varying types (uniform, gaussian, cauchy, etc.).
 */
class EDDY_SL_IEDECL RandomNumberGenerator
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:





    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Seeds the generator(s) used to produce numbers.
        /**
         * \param seed The seed to supply to the random number generator.
         */
        static
        void
        Seed(
            unsigned int seed
            );

        /// Returns the next result of a call to the ANSI rand() function.
        /**
         * \return The next raw random number of the ANSI rand() function.
         */
        static
        int
        Rand(
            );

        /// Returns a uniform random integer in the range [\a lo, \a hi].
        /**
         * INT_T can be a signed or unsigned integer type.
         * The range is inclusive meaning that hi and lo are legitimate
         * return values.
         *
         * \param lo The smallest integral value that may be returned.
         * \param hi The largest integral value that may be returned.
         * \return A uniform random integer in the range [\a lo, \a hi].
         */
        template <typename INT_T>
        static inline
        INT_T
        UniformInt(
            const INT_T& lo,
            const INT_T& hi
            );

        /// Returns a uniform random number in the range of [\a lo, \a hi].
        /**
         * The range is inclusive meaning that hi and lo are legitimate
         * return values.
         *
         * The default is to generate a number in the range of [0.0, 1.0].
         *
         * \param lo The smallest real number that may be returned.
         * \param hi The largest real number that may be returned.
         * \return A uniform random real number in the range [\a lo, \a hi].
         */
        static
        double
        UniformReal(
            double lo = 0.0,
            double hi = 1.0
            );

        /**
         * \brief Returns a Gaussian random number from a distribution
         *        with mean \a mu and standard deviation \a sigma.
         *
         * The default is a number in a distribution with mean = 0.0 and
         * a standard deviation of 1.0 (the so called "standard normal
         * distribution").
         *
         * \param mu The average of the distribution from which to select
         *           a random number.
         * \param sigma The standard deviation of the distribution from which
         *              to select a random number.
         * \return A random number chosen from a gaussian distribution with
         *         the supplied mean and standard deviation.
         */
        static
        double
        GaussianReal(
            double mu = 0.0,
            double sigma = 1.0
            );

        /**
         * \brief Returns a Cauchy random number from a distribution
         * with median \a mu and standard deviation \a sigma.
         *
         * The default is a number in a distribution with mean = 0.0 and
         * a standard deviation of 1.0.
         *
         * \param mu The average of the distribution from which to select
         *           a random number.
         * \param sigma The standard deviation of the distribution from which
         *              to select a random number.
         * \return A random number chosen from a cauchy distribution with
         *         the supplied mean and standard deviation.
         */
        static
        double
        CauchyReal(
            double mu = 0.0,
            double sigma = 1.0
            );

        /// Provides a random boolean value.
        /**
         * \return Either true or false with equal probability.
         */
        static inline
        bool
        RandomBoolean(
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





    /*
    ============================================================================
    Structors
    ============================================================================
    */
    // Make this class non-instantiable.
    private:

        /// This constructor is private and has no implementation.
        /**
         * This class has nothing but static methods and is not meant to be
         * instantiated.
         */
        RandomNumberGenerator(
            );


}; // class RandomNumberGenerator



/*
================================================================================
STL Style Generators
================================================================================
*/
/**
 * \brief A generator object for generating uniform random integers.
 *
 * \param INT_T A signed or unsigned integer type to generate.
 */
template <typename INT_T>
class uniform_int_generator
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The lowest possible return value of this generator.
        INT_T _lo;

        /// The highest possible return value of this generator.
        INT_T _hi;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// The operator that causes return of the next random value.
        /**
         * \return The next uniform random integral value in the range
         *         [\a _lo, \a _hi].
         */
        inline
        INT_T
        operator ()(
            ) const
        {
            return RandomNumberGenerator::UniformInt<INT_T>(_lo, _hi);
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a uniform_int_generator to generate values in the
         *        range [\a lo, \a hi].
         *
         * \param lo The lowest value that this generator can return.
         * \param hi The highest value that this generator can return.
         */
        uniform_int_generator(
            INT_T lo,
            INT_T hi
            ) :
                _lo(lo),
                _hi(hi)
        {
        }

}; // class uniform_int_generator


/**
 * \brief A generator object for generating uniform real numbers.
 */
class uniform_real_generator
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The lowest possible return value of this generator.
        double _lo;

        /// The highest possible return value of this generator.
        double _hi;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// The operator that causes return of the next random value.
        /**
         * \return The next uniform random real value in the range
         *         [\a _lo, \a _hi].
         */
        inline
        double
        operator ()(
            ) const
        {
            return RandomNumberGenerator::UniformReal(_lo, _hi);
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a uniform_real_generator to generate values in the
         *        range [\a lo, \a hi].
         *
         * \param lo The lowest value that this generator can return.
         * \param hi The highest value that this generator can return.
         */
        uniform_real_generator(
            double lo = 0.0,
            double hi = 1.0
            ) :
                _lo(lo),
                _hi(hi)
        {
        }

}; // class uniform_real_generator


/**
 * \brief A generator object for generating gaussian real numbers.
 */
class gaussian_real_generator
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The mean of the distribution from which to generate numbers.
        double _mu;

        /**
         * \brief The standard deviation of the distribution from which to
         *        generate numbers.
         */
        double _sigma;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// The operator that causes return of the next random value.
        /**
         * \return The next gaussian random real value with mean \a _mu and
         *         standard deviation \a _sigma.
         */
        inline
        double
        operator ()(
            ) const
        {
            return RandomNumberGenerator::GaussianReal(_mu, _sigma);
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a gaussian_real_generator to generate values with
         *        a mean of \a mu and a standard deviation of \a sigma.
         *
         * \param mu The mean of the distribution from which to generate
         *           numbers.
         * \param sigma The standard deviation of the distribution from which
         *              to generate numbers.
         */
        gaussian_real_generator(
            double mu = 0.0,
            double sigma = 1.0
            ) :
                _mu(mu),
                _sigma(sigma)
        {
        }

}; // class gaussian_real_generator

/**
 * \brief A generator object for generating cauchy real numbers.
 */
class cauchy_real_generator
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The mean of the distribution from which to generate numbers.
        double _mu;

        /**
         * \brief The standard deviation of the distribution from which to
         *        generate numbers.
         */
        double _sigma;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// The operator that causes return of the next random value.
        /**
         * \return The next cauchy random real value with mean \a _mu and
         *         standard deviation \a _sigma.
         */
        inline
        double
        operator ()(
            ) const
        {
            return RandomNumberGenerator::CauchyReal(_mu, _sigma);
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a cauchy_real_generator to generate values with
         *        a mean of \a mu and a standard deviation of \a sigma.
         *
         * \param mu The mean of the distribution from which to generate
         *           numbers.
         * \param sigma The standard deviation of the distribution from which
         *              to generate numbers.
         */
        cauchy_real_generator(
            double mu = 0.0,
            double sigma = 1.0
            ) :
                _mu(mu),
                _sigma(sigma)
        {
        }

}; // class cauchy_real_generator

/// A generator that generates random boolean values.
class random_bool_generator
{
    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// The operator that causes return of the next random value.
        /**
         * true and false are equally likely.
         *
         * \return The next random boolean (true or false).
         */
        inline
        bool
        operator ()(
            ) const
        {
            return RandomNumberGenerator::RandomBoolean();
        }

}; // random_bool_generator


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
#include "./inline/RandomNumberGenerator.hpp.inl"



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_UTILITIES_RANDOMNUMBERGENERATOR_HPP
