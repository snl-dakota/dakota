/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Definition of class MEvaluator.

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

        Thu Feb 09 10:51:11 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the MEvaluator class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_FRONTEND_MANAGED_MEVALUATOR_HPP
#define JEGA_FRONTEND_MANAGED_MEVALUATOR_HPP

#pragma once


/*
================================================================================
Includes
================================================================================
*/
#include <MConfig.hpp>
#include <MEvaluationFunctor.hpp>







/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/
#pragma unmanaged
namespace JEGA
{
    namespace Algorithms
    {
        class GeneticAlgorithm;
        class GeneticAlgorithmEvaluator;
    }

    namespace Utilities
    {
        class Design;
        class DesignGroup;
    }

    namespace FrontEnd
    {
        class EvaluatorCreator;
    }
}






/*
================================================================================
Namespace Aliases
================================================================================
*/
#pragma managed
#using <mscorlib.dll>








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
MANAGED_CLASS_FORWARD_DECLARE(public, MDesign);
class BaseEvaluator;
class BaseEvaluatorCreator;





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
/**
 * \brief A construct used to get evaluations performed for JEGA through a
 *        project using the managed front end.
 *
 * The MEvaluator does what is necessary to implement the requirements of
 * a JEGA::Algorithms::GeneticAlgorithmEvaluator such that ultimately, a call
 * to an implementation of an MEvaluationFunctor::Evaluate method is issued.
 */
MANAGED_CLASS(public, MEvaluator) :
    public System::IDisposable
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

        /**
         * \brief The creator of the GeneticAlgorihtmEvaluator derivative used
         *        by this class to "hook into" JEGA.
         */
        BaseEvaluatorCreator* _theEvalCreator;

        /// The evaluation functor that will be called by our evaluator.
        /**
         * This implementation uses a custom and non-exposed JEGA evaluator
         * type that further calls ahead to a function delegate to get
         * evaluations performed.  This is the delegate that is called.
         */
        MEvaluationFunctor MOH _theDelegate;

        BaseEvaluator* _evaler;

    /*
    ============================================================================
    Mutators
    ============================================================================
    */
    public:

        /**
         * \brief Allows mutation of the evaluation functor used by the JEGA
         *        evaluator derivative to get evaluations done.
         *
         * \param del The new delegate functor for performing evaluations.
         */
        void
        SetEvaluationDelegate(
            MEvaluationFunctor MOH del
            );

        void
        SetEvaluator(
            BaseEvaluator* evaler
            );

    protected:


    private:


    /*
    ============================================================================
    Accessors
    ============================================================================
    */
    public:

        /**
         * \brief Allows mutation of the evaluation functor used by the JEGA
         *        evaluator derivative to get evaluations done.
         *
         * \param del The new delegate functor for performing evaluations.
         */
        MEvaluationFunctor MOH
        GetEvaluationDelegate(
            );

    protected:


    private:


    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /**
         * \brief Executes the evalution of \a des using the known evaluation
         *        functor.
         *
         * \param des The Design class object to be evaluated.
         * \return True if the evaluation succeeds and false otherwise.
         */
        bool
        PerformEvaluation(
            JEGA::Utilities::Design& des
            );

        bool
        PerformEvaluation(
            JEGA::Utilities::DesignGroup& toEval
            );

        /**
         * \brief Allows access to the creator used to create the actual
         *        evaluator for JEGA.
         *
         * This is used by the algorithm configuration object to get the
         * evaluator creator.
         *
         * \return The evaluator creator stored here for use by those who need
         *         it.
         */
        JEGA::FrontEnd::EvaluatorCreator&
        GetTheEvaluatorCreator(
            );

        MDesign MOH
        InjectDesign(
            DoubleVector MOH X,
            DoubleVector MOH F,
            DoubleVector MOH G
            );

        void
        InjectDesign(
            MDesign MOH mDes
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

        /// Implementation of the Dispose method for the IDisposable interface.
        /**
         * This method destroys the underlying JEGA evaluator creator.
         * It is safe to call this more than once but it is not safe to use
         * the other methods of this class once this has been called.
         *
         * The destructor should call this method.
         */
        virtual
        void
        MANAGED_DISPOSE(
            );

        ///**
        // * \brief Executes the evalution of a Design using the known evaluation
        // *        functor.
        // *
        // * \return True if the evaluation succeeds and false otherwise.
        // */
        //virtual
        //bool
        //PerformEvaluation(
        //    DoubleVector MOH X,
        //    DoubleVector MOH F,
        //    DoubleVector MOH G
        //    );

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
    public:

        /// Default constructs an MEvaluator.
        /**
         * If you use this constructor, you must supply an evaluation functor
         * via the SetEvaluationDelegate method prior to use.
         */
        MEvaluator(
            );

        /// Constructs an MEvaluator to call to the supplied delegate.
        /**
         * \param theDelegate The evaluation functor that will be called back
         *                    when this evaluator is requested to perform an
         *                    evaluation.
         */
        MEvaluator(
            MEvaluationFunctor MOH theDelegate
            );

        //MEvaluator(
        //    JEGA::Algorithms::GeneticAlgorithmEvaluator* evaler
        //    );

        //MEvaluator(
        //    MEvaluationFunctor MOH theDelegate,
        //    JEGA::Algorithms::GeneticAlgorithmEvaluator* evaler
        //    );

        /// Destructs an MEvaluator.
        /**
         * The body of this method calls the Dispose method to destroy the
         * JEGA core evaluator creator object.
         */
        ~MEvaluator(
            );

}; // class MEvaluator



/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA








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
#endif // JEGA_FRONTEND_MANAGED_MEVALUATOR_HPP
