/*
===============================================================================
    PROJECT:

        Genetic Algorithm for Sandia National Laboratories

    CONTENTS:

        Definition of class JEGAOptimizer.

    NOTES:

        See notes under section "Class Definition" of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)
        Brian Adams (briadam@sandia.gov) (BA)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        This library is free software; you can redistribute it and/or
        modify it under the terms of the GNU Lesser General Public
        License as published by the Free Software Foundation; either
        version 2.1 of the License, or (at your option) any later version.

        This library is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
        GNU Lesser General Public License for more details.

        For a copy of the GNU Lesser General Public License, write to:
            Free Software Foundation, Inc.
            59 Temple Place, Suite 330
            Boston, MA 02111-1307 USA

    VERSION:

        2.0.0

    CHANGES:

        Mon Jun 09 09:48:34 2003 - Original Version (JE)
        Wed Dec 07 15:00:00 2005 - Added ParameterDatabase subclass to wrap
                                   ProblemDescDB for dependency removal (JE)

===============================================================================
*/



/*
===============================================================================
Document This File
===============================================================================
*/
/** \file
 * \brief Contains the definition of the JEGAOptimizer class.
 */






/*
===============================================================================
Prevent Multiple Inclusions
===============================================================================
*/
#ifndef DAKOTA_JEGAOPTIMIZER_H
#define DAKOTA_JEGAOPTIMIZER_H







/*
===============================================================================
Includes
===============================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <DakotaOptimizer.hpp>
#include <../Utilities/include/JEGATypes.hpp>






/*
===============================================================================
Pre-Namespace Forward Declares
===============================================================================
*/
namespace JEGA
{
    namespace Utilities
    {
        class Design;
        class DesignOFSortSet;
        class ParameterDatabase;
    }

    namespace FrontEnd
    {
        class ProblemConfig;
        class AlgorithmConfig;
    }

    namespace Algorithms
    {
        class GeneticAlgorithm;
    }
}

/*
===============================================================================
Namespace Using Directives - DISCOURAGED!!
===============================================================================
*/






/*
===============================================================================
Begin Namespace
===============================================================================
*/
namespace Dakota {







/*
===============================================================================
Forward Declares
===============================================================================
*/
class JEGAOptimizer;





/*
===============================================================================
Class Definition
===============================================================================
*/

/**
 * \brief A version of Dakota::Optimizer for instantiation of John Eddy's
 *        Genetic Algorithms (JEGA).
 *
 * This class encapsulates the necessary functionality for creating and
 * properly initializing the JEGA algorithms (MOGA and SOGA).
 */
class JEGAOptimizer :
    public Optimizer
{
    /*
    ===========================================================================
    Inner Class Forward Declares
    ===========================================================================
    */
    private:

        /// This evaluator uses Sandia National Laboratories Dakota software.
        /**
         * Evaluations are carried out using a Model which is known by
         * reference to this class.  This provides the advantage of execution
         * on massively parallel computing architectures.
         */
        class Evaluator;

        /**
         * \brief A specialization of the JEGA::FrontEnd::EvaluatorCreator that
         *        creates a new instance of a JEGAEvaluator.
         */
        class EvaluatorCreator;

        /**
         * \brief A subclass of the JEGA front end driver that exposes the
         *        individual protected methods to execute the algorithm.
         *
         * This is necessary because DAKOTA requires that all problem
         * information be extracted from the problem description DB at the
         * time of Optimizer construction and the front end does it all in
         * the execute algorithm method which must be called in core_run.
         */
        class Driver;

    /*
    ===========================================================================
    Member Data Declarations
    ===========================================================================
    */
    private:

        /**
         * \brief A pointer to an EvaluatorCreator used to create the evaluator
         *        used by JEGA in Dakota (a JEGAEvaluator).
         */
        EvaluatorCreator* _theEvalCreator;

        /**
         * \brief A pointer to the ParameterDatabase from which all parameters
         *        are retrieved by the created algorithms.
         */
        JEGA::Utilities::ParameterDatabase* _theParamDB;

        /// An array of initial points to use as an initial population.
        /**
         * This member is here to help support the use of JEGA algorithms in
         * Dakota strategies.  If this array is populated, then whatever
         * initializer is specified will be ignored and the DoubleMatrix
         * initializer will be used instead on a matrix created from the data
         * in this array.
         */
        VariablesArray _initPts;
 

    /*
    ===========================================================================
    Mutators
    ===========================================================================
    */
    public:





    /*
    ===========================================================================
    Accessors
    ===========================================================================
    */
    public:







    /*
    ===========================================================================
    Public Methods
    ===========================================================================
    */

    public:






    /*
    ===========================================================================
    Subclass Visible Methods
    ===========================================================================
    */
    protected:

        /**
         * \brief Loads the JEGA-style Design class into equivalent
         *        Dakota-style Variables and Response objects.
         *
         * This version is meant for the case where a Variables and a Response
         * object exist and just need to be loaded.
         *
         * \param from The JEGA Design class object from which to extract the
         *             variable and response information for Dakota.
         * \param vars The Dakota::Variables object into which to load the
         *             design variable values of \a from.
         * \param resp The Dakota::Response object into which to load the
         *             objective function and constraint values of \a from.
         */
        void
        LoadDakotaResponses(
            const JEGA::Utilities::Design& from,
            Variables& vars,
            Response& resp
            ) const;

        /**
         * \brief Destroys the current parameter database and creates a new
         *        empty one.
         */
        void
        ReCreateTheParameterDatabase(
            );

        /**
         * \brief Reads information out of the known Dakota::ProblemDescDB and
         *        puts it into the current parameter database.
         *
         * This should be called from the JEGAOptimizer constructor since it
         * is the only time when the problem description database is certain to
         * be configured to supply data for this optimizer.
         */
        void
        LoadTheParameterDatabase(
            );

        /**
         * \brief Completely initializes the supplied algorithm configuration.
         *
         * This loads the supplied configuration object with appropriate data
         * retrieved from the parameter database.
         *
         * \param aConfig The algorithm configuration object to load.
         */
        void
        LoadAlgorithmConfig(
            JEGA::FrontEnd::AlgorithmConfig& aConfig
            );

        /**
         * \brief Completely initializes the supplied problem configuration.
         *
         * This loads the fresh configuration object using the
         * LoadTheDesignVariables, LoadTheObjectiveFunctions, and
         * LoadTheConstraints methods.
         *
         * \param pConfig The problem configuration object to load.
         */
        void
        LoadProblemConfig(
            JEGA::FrontEnd::ProblemConfig& pConfig
            );

        /**
         * \brief Adds DesignVariableInfo objects into the problem
         *        configuration object.
         *
         * This retrieves design variable information from the
         * ParameterDatabase and creates DesignVariableInfo's from it.
         *
         * \param pConfig The problem configuration object to load.
         */
        void
        LoadTheDesignVariables(
            JEGA::FrontEnd::ProblemConfig& pConfig
            );

        /**
         * \brief Adds ObjectiveFunctionInfo objects into the problem
         *        configuration object.
         *
         * This retrieves objective function information from the
         * ParameterDatabase and creates ObjectiveFunctionInfo's from it.
         *
         * \param pConfig The problem configuration object to load.
         */
        void
        LoadTheObjectiveFunctions(
            JEGA::FrontEnd::ProblemConfig& pConfig
            );

        /**
         * \brief Adds ConstraintInfo objects into the problem
         *        configuration object.
         *
         * This retrieves constraint function information from the
         * ParameterDatabase and creates ConstraintInfo's from it.
         *
         * \param pConfig The problem configuration object to load.
         */
        void
        LoadTheConstraints(
            JEGA::FrontEnd::ProblemConfig& pConfig
            );

        /**
         * \brief Returns up to _numBest designs sorted by DAKOTA's
         *        fitness (L2 constraint violation, then utopia or
         *        objective), taking into account the algorithm type.
         *        The front of the returned map can be viewed as a
         *        single "best".
         *
         * \param from The full set of designs returned by the solver.
         *
         * \param theGA The GA used to generate this set; needed for
         *              its weights in the SO case, provided to both
         *              for consistency
         *
	     * \param designSortMap Map of best solutions with key
	     *                      pair<constraintViolation, fitness>
	     * 
         * eventually this functionality must be moved into a separate
         * post-processing application for MO datasets.
         */
        void
        GetBestSolutions(
  	    const JEGA::Utilities::DesignOFSortSet& from,
            const JEGA::Algorithms::GeneticAlgorithm& theGA,
            std::multimap<RealRealPair, JEGA::Utilities::Design*>& designSortMap
            );

        /**
         * \brief Retreive the best Designs from a set of solutions assuming
         *        that they are generated by a multi objective algorithm.
         *
         * eventually this functionality must be moved into a separate
         * post-processing application for MO datasets.
         */
        void
        GetBestMOSolutions(
	    const JEGA::Utilities::DesignOFSortSet& from,
            const JEGA::Algorithms::GeneticAlgorithm& theGA,
	    std::multimap<RealRealPair, JEGA::Utilities::Design*>& designSortMap
            );

        /**
         * \brief Retreive the best Designs from a set of solutions assuming
         *        that they are generated by a single objective algorithm.
         *
         * eventually this functionality must be moved into a separate
         * post-processing application for MO datasets.
         */
        void
        GetBestSOSolutions(
	    const JEGA::Utilities::DesignOFSortSet& from,
            const JEGA::Algorithms::GeneticAlgorithm& theGA,
	    std::multimap<RealRealPair, JEGA::Utilities::Design*>& designSortMap
            );

        /**
         * \brief Converts the items in a VariablesArray into a DoubleMatrix
         *        whereby the items in the matrix are the design variables.
         *
         * The matrix will not contain responses but when being used by Dakota,
         * this doesn't matter.  JEGA will attempt to re-evaluate these points
         * but Dakota will recognize that they do not require re-evaluation and
         * thus it will be a cheap operation.
         *
         * \param variables The array of DakotaVariables objects to use as the
         *                  contents of the returned matrix.
         * \return The matrix created using the supplied VariablesArray.
         */
        JEGA::DoubleMatrix
        ToDoubleMatrix(
            const VariablesArray& variables
            ) const;
            

    /*
    ===========================================================================
    Subclass Overridable Methods
    ===========================================================================
    */
    public:

        /// Performs the iterations to determine the optimal set of solutions.
        /**
         * Override of pure virtual method in Optimizer base class.
         *
         * The extraction of parameter values actually occurs in this method
         * when the JEGA::FrontEnd::Driver::ExecuteAlgorithm is called.  Also
         * the loading of the problem and algorithm configurations occurs in
         * this method.  That way, if it is called more than once and the
         * algorithm or problem has changed, it will be accounted for.
         */
        virtual
        void
        core_run(
            );

        /**
         * \brief Overridden to return true since JEGA algorithms can accept
         *        multiple initial points.
         *
         * \return true, always.
         */
        virtual
        bool
        accepts_multiple_points(
            ) const;

        /**
         * \brief Overridden to return true since JEGA algorithms can return
         *        multiple final points.
         *
         * \return true, always.
         */
        virtual
        bool
        returns_multiple_points(
            ) const;

        /**
         * \brief Overridden to assign the _initPts member variable to the
         *        passed in collection of Dakota::Variables.
         *
         * \param pts The array of initial points for the JEGA algorithm created
         *            and run by this JEGAOptimizer.
         */
        virtual
        void
        initial_points(
            const VariablesArray& pts
            );

        /**
         * \brief Overridden to return the collection of initial points for the
         *        JEGA algorithm created and run by this JEGAOptimizer.
         *
         * \return The collection of initial points for the JEGA algorithm
         *         created and run by this JEGAOptimizer.
         */
        virtual
        const VariablesArray&
        initial_points(
            ) const;

    protected:


    private:



    /*
    ===========================================================================
    Private Methods
    ===========================================================================
    */
    private:




    /*
    ===========================================================================
    Structors
    ===========================================================================
    */
    public:

        /// Constructs a JEGAOptimizer class object.
        /**
         * This method does some of the initialization work for the algorithm.
         * In particular, it initialized the JEGA core.
         *
	 * \param problem_db The Dakota::ProblemDescDB with information on how the 
	 *                   algorithm controls should be set.
	 *
         * \param model The Dakota::Model that will be used by this optimizer
         *              for problem information, etc.
         */
        JEGAOptimizer(
            ProblemDescDB& problem_db, Model& model
            );

        /// Destructs a JEGAOptimizer
        ~JEGAOptimizer(
            );

}; // class JEGAOptimizer



/*
===============================================================================
Include Inlined Methods File
===============================================================================
*/
// Not using an Inlined Functions File.




/*
===============================================================================
End Namespace
===============================================================================
*/
} // namespace Dakota






/*
===============================================================================
End of Multiple Inclusion Check
===============================================================================
*/
#endif // DAKOTA_JEGAOPTIMIZER_H
