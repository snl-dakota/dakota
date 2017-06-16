/*
===============================================================================
    PROJECT:

        Genetic Algorithm for Sandia National Laboratories

    CONTENTS:

        Implementation of class JEGAOptimizer.

    NOTES:

        See notes of JEGAOptimizer.hpp.

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
        Wed Mar 29 12:00:00 2006 - Adopted use of JEGA front end project to
                                   reduce code duplication and keep Dakota
                                   up to speed with the latest in JEGA.

===============================================================================
*/




/*
===============================================================================
Document This File
===============================================================================
*/
/** \file
 * \brief Contains the implementation of the JEGAOptimizer class.
 */




/*
===============================================================================
Includes
===============================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>
#include <../Utilities/include/Logging.hpp>

// Standard includes.

// JEGA core includes.
#include <GeneticAlgorithm.hpp>
#include <GeneticAlgorithmEvaluator.hpp>
#include <GeneticAlgorithmInitializer.hpp>
#include <OperatorGroups/AllOperators.hpp>

// SOGA-specific API
#include <../SOGA/include/SOGA.hpp>

// JEGA front end includes.
#include <../FrontEnd/Core/include/Driver.hpp>
#include <../FrontEnd/Core/include/ProblemConfig.hpp>
#include <../FrontEnd/Core/include/AlgorithmConfig.hpp>
#include <../FrontEnd/Core/include/EvaluatorCreator.hpp>

// Dakota includes.
#include <JEGAOptimizer.hpp>
#include <ProblemDescDB.hpp>

// Eddy utility includes.
#include <utilities/include/EDDY_DebugScope.hpp>

// JEGA utility includes.
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <../Utilities/include/BasicParameterDatabaseImpl.hpp>
#include <../Utilities/include/MultiObjectiveStatistician.hpp>
#include <../Utilities/include/SingleObjectiveStatistician.hpp>

#include <algorithm>
#include <sstream>

/*
===============================================================================
Namespace Using Directives
===============================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::FrontEnd;
using namespace eddy::utilities;
using namespace JEGA::Utilities;
using namespace JEGA::Algorithms;

/*
===============================================================================
Begin Namespace
===============================================================================
*/
namespace Dakota {




/*
===============================================================================
File Scope Helper Functions
===============================================================================
*/
/// Creates a string from the argument \a val using an ostringstream.
/**
 * This only gets used in this file and is only ever called with ints so no
 * error checking is in place.
 *
 * \param val The value of type T to convert to a string.
 * \return The string representation of \a val created using an ostringstream.
 */
template <typename T>
string
asstring(
    const T& val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ostringstream ostr;
    ostr << val;
    return ostr.str();
}


/// An evaluator specialization that knows how to interact with Dakota.
/**
 * This evaluator knows how to use the model to do evaluations both in
 * synchronous and asynchronous modes.
 */
class JEGAOptimizer::Evaluator :
    public GeneticAlgorithmEvaluator
{
    /*
    ===========================================================================
    Member Data Declarations
    ===========================================================================
    */
    private:

        /// The Model known by this evaluator.
        /**
         * It is through this model that evaluations will take place.
         */
        Model& _model;

    /*
    ===========================================================================
    Public Methods
    ===========================================================================
    */
    public:

         /// Returns the proper name of this operator.
        /**
         * \return The string "DAKOTA JEGA Evaluator".
         */
        static
        const std::string&
        Name(
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            static const string ret("DAKOTA JEGA Evaluator");
            return ret;
        }

        /// Returns a full description of what this operator does and how.
        /**
         * The returned text is:
         * \verbatim
            This evaluator uses Sandia's DAKOTA optimization
            software to evaluate the passed in Designs.  This
            makes it possible to take advantage of the fact that
            DAKOTA is designed to run on massively parallel machines.
           \endverbatim.
         *
         * \return A description of the operation of this operator.
         */
        static
        const std::string&
        Description(
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            static const string ret(
                "This evaluator uses Sandia's DAKOTA optimization software to "
                "evaluate the passed in Designs.  This makes it possible to "
                "take advantage of the fact that DAKOTA is designed to run on "
                "massively parallel machines."
                );
            return ret;
        }

    /*
    ===========================================================================
    Subclass Visible Methods
    ===========================================================================
    */
    protected:

        /**
         * \brief This method fills \a intoCont, \a intoDiscInt and
         * \a intoDiscReal appropriately using the values of \a from.
         *
         * The discrete integer design variable values are placed in
         * \a intoDiscInt, the discrete real design variable values are placed
         * in \a intoDiscReal, and the continuum are placed into \a intoCont.
         * The values are written into the vectors from the beginning so any
         * previous contents of the vectors will be overwritten.
         *
         * \param from The Design class object from which to extract the
         *             discrete design variable values.
         * \param intoDiscInt The vector into which to place the extracted
         *             discrete integer values.
         * \param intoDiscReal The vector into which to place the extracted
         *             discrete real values.
         * \param intoCont The vector into which to place the extracted
         *             continuous values.
         */
        void
        SeparateVariables(
            const Design& from,
            RealVector& intoCont,
            IntVector&  intoDiscInt,
            RealVector& intoDiscReal,
	    StringMultiArray& intoDiscString
            ) const;

        /**
         * \brief Records the computed objective and constraint function values
         *        into \a into.
         *
         * This method takes the response values stored in \a from and properly
         * transfers them into the \a into design.
         *
         * The response vector \a from is expected to contain values for each
         * objective function followed by values for each non-linear constraint
         * in the order in which the info objects were loaded into the target
         * by the optimizer class.
         *
         * \param from The vector of responses to install into \a into.
         * \param into The Design to which the responses belong and into which
         *             they must be written.
         */
        void
        RecordResponses(
            const RealVector& from,
            Design& into
            ) const;

        /// Returns the number of non-linear constraints for the problem.
        /**
         * This is computed by adding the number of non-linear equality
         * constraints to the number of non-linear inequality constraints.
         * These values are obtained from the model.
         *
         * \return The total number of non-linear constraints.
         */
        std::size_t
        GetNumberNonLinearConstraints(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return this->_model.num_nonlinear_eq_constraints() +
                   this->_model.num_nonlinear_ineq_constraints();
        }

        /// Returns the number of linear constraints for the problem.
        /**
         * This is computed by adding the number of linear equality
         * constraints to the number of linear inequality constraints.
         * These values are obtained from the model.
         *
         * \return The total number of linear constraints.
         */
        std::size_t
        GetNumberLinearConstraints(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return this->_model.num_linear_eq_constraints() +
                   this->_model.num_linear_ineq_constraints();
        }

    /*
    ===========================================================================
    Subclass Overridable Methods
    ===========================================================================
    */
    public:

        /// Does evaluation of each design in \a group.
        /**
         * This method uses the Model known by this class to get Designs
         * evaluated.  It properly formats the Design class information in a
         * way that Dakota will understand and then interprets the Dakota
         * results and puts them back into the Design class object.  It
         * respects the asynchronous flag in the Model so evaluations may
         * occur synchronously or asynchronously.
         *
         * Prior to evaluating a Design, this class checks to see if it
         * is marked as already evaluated.  If it is, then the evaluation
         * of that Design is not carried out.  This is not strictly
         * necessary because Dakota keeps track of evaluated designs and
         * does not re-evaluate.  An exception is the case of a population
         * read in from a file complete with responses where Dakota is
         * unaware of the evaluations.
         *
         * \param group The group of Design class objects to be evaluated.
         * \return true if all evaluations completed and false otherwise.
         */
        virtual
        bool
        Evaluate(
            DesignGroup& group
            );

        /// This method cannot be used!!
        /**
         * This method does nothing and cannot be called.  This is because in
         * the case of asynchronous evaluation, this method would be unable
         * to conform.  It would require that each evaluation be done in a
         * synchronous fashion.
         *
         * \param des A Design that would be evaluated if this
         *            method worked.
         * \return Would return true if the Design were evaluated and false
         *         otherwise.  Never actually returns here.  Issues a fatal
         *         error.  Otherwise, it would always return false.
         */
        virtual
        bool
        Evaluate(
            Design& des
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            JEGALOG_II_F(this->GetLogger(), this,
                text_entry(lfatal(),
                    this->GetName() + 
                    ": You cannot use Evaluate(Design&) with this "
                    "evaluator...ever.")
                )
            return false;
        }

        /// Returns the proper name of this operator.
        /**
         * \return See Name().
         */
        virtual
        std::string
        GetName(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return Evaluator::Name();
        }

        /// Returns a full description of what this operator does and how.
        /**
         * \return See Description().
         */
        virtual
        std::string
        GetDescription(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return Evaluator::Description();
        }

        /**
         * \brief Creates and returns a pointer to an exact duplicate of this
         *        operator.
         *
         * \param algorithm The GA for which the clone is being created.
         * \return A clone of this operator.
         */
        virtual
        GeneticAlgorithmOperator*
        Clone(
            GeneticAlgorithm& algorithm
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return new Evaluator(*this, algorithm, _model);
        }


    /*
    ===========================================================================
    Structors
    ===========================================================================
    */
    public:

        /**
         * \brief Constructs a Evaluator for use by \a  algorithm.
         *
         * The optimizer is needed for purposes of variable scaling.
         *
         * \param algorithm The GA for which the new evaluator is to be used.
         * \param model The model through which evaluations will be done.
         */
        Evaluator(
            GeneticAlgorithm& algorithm,
            Model& model
            ) :
                GeneticAlgorithmEvaluator(algorithm),
                _model(model)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

        /**
         * \brief Copy constructs a Evaluator.
         *
         * \param copy The evaluator from which properties are to be duplicated
         *             into this.
         */
        Evaluator(
            const Evaluator& copy
            ) :
                GeneticAlgorithmEvaluator(copy),
                _model(copy._model)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

        /**
         * \brief Copy constructs a Evaluator for use by \a algorithm.
         *
         * The optimizer is needed for purposes of variable scaling.
         *
         * \param copy The existing Evaluator from which to retrieve
         *             properties.
         * \param algorithm The GA for which the new evaluator is to be used.
         * \param model The model through which evaluations will be done.
         */
        Evaluator(
            const Evaluator& copy,
            GeneticAlgorithm& algorithm,
            Model& model
            ) :
                GeneticAlgorithmEvaluator(copy, algorithm),
                _model(model)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

    private:

        /// This constructor has no implementation and cannot be used.
        /**
         * This constructor can never be used.  It is provided so that this
         * operator can still be registered in an operator registry even though
         * it can never be instantiated from there.
         *
         * \param algorithm The GA for which the new evaluator is to be used.
         */
        Evaluator(
            GeneticAlgorithm& algorithm
            );


}; // class JEGAOptimizer::Evaluator

/**
 * \brief A specialization of the JEGA::FrontEnd::EvaluatorCreator that
 *        creates a new instance of a Evaluator.
 */
class JEGAOptimizer::EvaluatorCreator :
    public JEGA::FrontEnd::EvaluatorCreator
{
    /*
    ===========================================================================
    Member Data Declarations
    ===========================================================================
    */
    private:

        /**
         * \brief The user defined model to be passed to the constructor of the
         *        Evaluator.
         */
        Model& _theModel;

    /*
    ===========================================================================
    Subclass Overridable Methods
    ===========================================================================
    */
    public:

        /// Overriden to return a newly created Evaluator.
        /**
         * The GA will assume ownership of the evaluator so we needn't worry
         * about keeping track of it for destruction.  The additional
         * parameters needed by the Evaluator are stored as members of this
         * class at construction time.
         *
         * \param alg The GA for which the evaluator is to be created.
         * \return A pointer to a newly created Evaluator.
         */
        virtual
        GeneticAlgorithmEvaluator*
        CreateEvaluator(
            GeneticAlgorithm& alg
            )
        {
            EDDY_FUNC_DEBUGSCOPE
	      return new Evaluator(alg, _theModel);
        }

    /*
    ===========================================================================
    Structors
    ===========================================================================
    */
    public:

        /**
         * \brief Constructs an EvaluatorCreator using the supplied model.
         *
         * \param theModel The Dakota::Model this creator will pass to the
         *                 created evaluator.
         */
        EvaluatorCreator(
            Model& theModel
            ) :
                _theModel(theModel)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

}; // class JEGAOptimizer::EvaluatorCreator

/**
 * \brief A subclass of the JEGA front end driver that exposes the
 *        individual protected methods to execute the algorithm.
 *
 * This is necessary because DAKOTA requires that all problem
 * information be extracted from the problem description DB at the
 * time of Optimizer construction and the front end does it all in
 * the execute algorithm method which must be called in core_run.
 */
class JEGAOptimizer::Driver :
    public JEGA::FrontEnd::Driver
{
    /*
    ===========================================================================
    Member Data Declarations
    ===========================================================================
    */
    private:

    /*
    ===========================================================================
    Public Methods
    ===========================================================================
    */
    public:

        /**
         * \brief Reads all required data from the problem description database
         *        stored in the supplied algorithm config.
         *
         * The returned GA is fully configured and ready to be run.  It must
         * also be destroyed at some later time.  You MUST call
         * DestroyAlgorithm for this purpose.  Failure to do so could result
         * in a memory leak and an eventual segmentation fault!  Be sure to
         * call DestroyAlgorithm prior to destroying the algorithm config that
         * was used to create it!
         *
         * This is just here to expose the base class method to users.
         *
         * \param algConfig The fully loaded configuration object containing
         *                  the database of parameters for the algorithm to be
         *                  run on the known problem.
         * \return The fully configured and loaded GA ready to be run using
         *         the PerformIterations method.
         */
        GeneticAlgorithm*
        ExtractAllData(
            const AlgorithmConfig& algConfig
            )
        {
            return JEGA::FrontEnd::Driver::ExtractAllData(algConfig);
        }

        /**
         * \brief Performs the required iterations on the supplied GA.
         *
         * This includes the calls to AlgorithmInitialize and AlgorithmFinalize
         * and logs some information if appropriate.
         *
         * This is just here to expose the base class method to users.
         *
         * \param theGA The GA on which to perform iterations.  This parameter
         *              must be non-null.
         * \return The final solutions reported by the supplied GA after all
         *         iterations and call to AlgorithmFinalize.
         */
        DesignOFSortSet
        PerformIterations(
            GeneticAlgorithm* theGA
            )
        {
            return JEGA::FrontEnd::Driver::PerformIterations(theGA);
        }

        /**
         * \brief Deletes the supplied GA.
         *
         * Use this method to destroy a GA after all iterations have been run.
         * This method knows if the log associated with the GA was created here
         * and needs to be destroyed as well or not.
         *
         * This is just here to expose the base class method to users.
         *
         * Be sure to use this prior to destoying the algorithm config object
         * which contains the target.  The GA destructor needs the target to
         * be in tact.
         *
         * \param theGA The algorithm that is no longer needed and thus must be
         *              destroyed.
         */
        void
        DestroyAlgorithm(
            GeneticAlgorithm* theGA
            )
        {
            JEGA::FrontEnd::Driver::DestroyAlgorithm(theGA);
        }

    /*
    ===========================================================================
    Structors
    ===========================================================================
    */
    public:

        /// Default constructs a Driver
        /**
         * \param probConfig The definition of the problem to be solved by this
         *                   Driver whenever ExecuteAlgorithm is called.
         *
         * The problem can be solved in multiple ways by multiple algorithms
         * even using multiple different evaluators by issuing multiple calls
         * to ExecuteAlgorithm with different AlgorithmConfigs.
         */
        Driver(
            const ProblemConfig& probConfig
            ) :
                JEGA::FrontEnd::Driver(probConfig)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

}; // class JEGAOptimizer::Driver


/*
===============================================================================
Static Member Data Definitions
===============================================================================
*/





/*
===============================================================================
Mutators
===============================================================================
*/








/*
===============================================================================
Accessors
===============================================================================
*/





/*
===============================================================================
Public Methods
===============================================================================
*/
void
JEGAOptimizer::core_run(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Load up an algorithm config and a problem config.
    ProblemConfig pConfig;
    LoadProblemConfig(pConfig);

    AlgorithmConfig aConfig(*this->_theEvalCreator, *this->_theParamDB);
    this->LoadAlgorithmConfig(aConfig);

    // retrieve parameter database for repeated use (is actaully _theParamDB)
    ParameterDatabase& pdb = aConfig.GetParameterDB();

    // Create a new driver for JEGA.
    JEGAOptimizer::Driver driver(pConfig);

    // Get the algorithm separately (rather than simply running the current
    // configuration) in case we need to change the initializer.
    GeneticAlgorithm* theGA = driver.ExtractAllData(aConfig);

    // Get the name of the GA for repeated use below.  We need this regardless
    // of whether or not logging b/c it is used in a fatal error.
    const string& name = theGA->GetName();

    // The initializer requires some additional logic to account for the
    // possibility that JEGA is being used in a Dakota strategy.  If that is
    // the case, the _initPts array will be non-empty and we will use them
    // instead of whatever initialization has been specified by the user.
    if(!this->_initPts.empty())
    {
        const GeneticAlgorithmInitializer& oldInit =
            theGA->GetOperatorSet().GetInitializer();

        JEGALOG_II_G(lquiet(), this,
            text_entry(lquiet(), name + ": discovered multiple initial "
                "points presumably supplied by a previous iterator in a "
                "strategy.  The \"" + oldInit.GetName() + "\" initializer "
                "will not be used and instead will be replaced with the "
                "double_matrix initializer which will read the supplied "
                "initial points."
                )
            )

        pdb.AddIntegralParam(
            "method.population_size", static_cast<int>(oldInit.GetSize())
            );

        pdb.AddDoubleMatrixParam(
            "method.jega.design_matrix", ToDoubleMatrix(initial_points())
            );

        GeneticAlgorithmInitializer* newInit =
            AllOperators::FullInstance().GetInitializer(
                "double_matrix", *theGA
                );

        JEGAIFLOG_II_G_F(newInit == 0x0, this,
            text_entry(lfatal(), name + ": Unable to resolve "
                "Initializer \"double_matrix\".")
            );

        JEGAIFLOG_II_F(!theGA->SetInitializer(newInit),
            theGA->GetLogger(), this,
            text_entry(lfatal(), name + ": Unable to set the initializer to "
                "double_matrix because it is incompatible with the other "
                "operators."
                )
            )

        JEGAIFLOG_II_F(
            !newInit->ExtractParameters(pdb), theGA->GetLogger(), this,
            text_entry(lfatal(),
                name + ": Failed to retrieve the parameters for \"" +
                newInit->GetName() + "\".")
            );

    }

    JEGALOG_II_G(lverbose(), this,
        text_entry(lverbose(),
            name + ": About to perform algorithm execution.")
            )

    DesignOFSortSet bests(driver.PerformIterations(theGA));

    JEGALOG_II_G(lverbose(), this,
        ostream_entry(lverbose(), name + ": algorithm execution completed. ")
            << bests.size() << " solutions found. Passing them back to DAKOTA."
        )

    // Return up to numBest solutions to DAKOTA, sorted first by L2
    // constraint violation, then (utopia distance or weighted sum
    // objective value).  So the single "best" will be at the front.
    //
    // Load up to numBest solutions into the arrays of best responses
    // and variables.  If this is MOGA, the array will then contain
    // the Pareto set.  If it is SOGA, it will contain all the
    // solutions with the same best "fitness".
      
    // populate the sorted map of best solutions (fairly lightweight
    // map) key is pair<constraintViolation, fitness>, where fitness
    // is either utopia distance (MOGA) or objective value (SOGA)
    std::multimap<RealRealPair, Design*> designSortMap;
    this->GetBestSolutions(bests, *theGA, designSortMap);

    JEGAIFLOG_II_G(designSortMap.size() == 0, lquiet(), this,
        text_entry(lquiet(), name + ": was unable to identify at least one "
            "best solution.  The Dakota best variables and best responses "
            "objects will be empty.\n\n")
        )

    // load the map into the DAKOTA vectors
    resize_best_resp_array(designSortMap.size());
    resize_best_vars_array(designSortMap.size());
    
    std::multimap<RealRealPair, Design*>::const_iterator best_it = 
        designSortMap.begin(); 
    const std::multimap<RealRealPair, Design*>::const_iterator best_end = 
        designSortMap.end(); 
    ResponseArray::size_type index = 0;
    for( ; best_it != best_end; ++best_it, ++index)
    {
        this->LoadDakotaResponses(
        *(best_it->second),
            this->bestVariablesArray[index],
            this->bestResponseArray[index]
            );
    }

    // now we are done with our solution set so we can flush it
    // per Driver rules.
    bests.flush();

    JEGALOG_II_G(lquiet(), this,
        text_entry(lquiet(), name + ": find optimum completed and all "
            "results have been passed back to DAKOTA.\n\n")
        )

    // We can not destroy our GA.
    driver.DestroyAlgorithm(theGA);
}

bool
JEGAOptimizer::accepts_multiple_points(
    ) const
{
    return true;
}

bool
JEGAOptimizer::returns_multiple_points(
    ) const
{
    return true;
}

void
JEGAOptimizer::initial_points(
    const VariablesArray& pts
    )
{
    this->_initPts = pts;
}

const VariablesArray&
JEGAOptimizer::initial_points(
    ) const
{
    return this->_initPts;
}


/*
===============================================================================
Subclass Visible Methods
===============================================================================
*/

void
JEGAOptimizer::LoadDakotaResponses(
    const JEGA::Utilities::Design& des,
    Dakota::Variables& vars,
    Dakota::Response& resp
    ) const
{
    RealVector c_vars(this->numContinuousVars);
    IntVector  di_vars(this->numDiscreteIntVars);
    RealVector dr_vars(this->numDiscreteRealVars);

//PDH: JEGA variables to Dakota variables.
//     Don't know what the JEGA data structure is.  These are all
//     mapped on entry at a time.

    // The first numContinuousVars of a design will be all the continuous
    // variables of the problem (see LoadTheDesignVariables).
    for(size_t i=0; i<this->numContinuousVars; ++i)
        c_vars[i] = des.GetVariableValue(i);

    // The next set of variables represent the discrete integer variables.
    for(size_t i=0; i<this->numDiscreteIntVars; ++i)
        di_vars[i] = static_cast<int>(
            des.GetVariableValue(i + this->numContinuousVars));

    // The next set of variables represent the discrete real variables.
    for(size_t i=0; i<this->numDiscreteRealVars; ++i)
        dr_vars[i] = des.GetVariableValue(
            i + this->numContinuousVars + this->numDiscreteIntVars);

    // Finally, set the discrete string vars. These are mapped to discrete
    // integers in JEGA, so they must be unmapped. Here, they also are set in
    // vars using the single value setter to avoid creating a 
    // StringMultiArrayConstView

//PDH: JEGA variables to Dakota variables.
//     Don't know what the JEGA data structure is.  These are all
//     mapped on entry at a time.
//     String variables also need to be remapped.

    const StringSetArray& dssv_values = 
      iteratedModel.discrete_design_set_string_values();
    for(size_t i=0; i<this->numDiscreteStringVars; ++i) {
      const int &element_index = static_cast<int>(des.GetVariableValue(i +
	    this->numContinuousVars + this->numDiscreteIntVars +
	    this->numDiscreteRealVars));
      const String &ds_var = set_index_to_value(element_index, dssv_values[i]);
      vars.discrete_string_variable(ds_var, i);
    }
    vars.continuous_variables(c_vars);
    vars.discrete_int_variables(di_vars);
    vars.discrete_real_variables(dr_vars);

//PDH: JEGA responses to Dakota responses.
//     Don't know what the JEGA data structure is.  These are all
//     mapped on entry at a time.
//     Need to respect constraint ordering.

    // BMA TODO: Could always populate constraints and just get
    // primary responses from the DB, as in SNLL
    if (!localObjectiveRecast) {  // else local_recast_retrieve
      RealVector fn_vals(this->numFunctions);
      for(size_t i=0; i<this->numObjectiveFns; i++)
	fn_vals[i]= des.GetObjective(i);

      // JEGA constraint ordering is nonlinear inequality, nonlinear equality,
      // linear inequality, linear equality
      // (see JEGAOptimizer::LoadTheConstraints()).
      for(size_t i=0; i<static_cast<size_t>(this->numNonlinearConstraints); ++i)
        fn_vals[i+this->numObjectiveFns] = des.GetConstraint(i);

      resp.function_values(fn_vals);
    }
}

void
JEGAOptimizer::ReCreateTheParameterDatabase(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_theParamDB;
    this->_theParamDB = new BasicParameterDatabaseImpl();
}

void
JEGAOptimizer::LoadTheParameterDatabase(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->ReCreateTheParameterDatabase();

    // Duplicate in all the integral parameters.
    const int& random_seed = probDescDB.get_int("method.random_seed");
    if (random_seed != 0)
      this->_theParamDB->AddIntegralParam(
	  "method.random_seed", random_seed
        );
    else
      this->_theParamDB->AddIntegralParam(
	  "method.random_seed", -1
        );

    // now all the reals
    const Real& constraint_penalty = probDescDB.get_real("method.constraint_penalty");
    if (constraint_penalty >= 0.)
      this->_theParamDB->AddDoubleParam(
	  "method.constraint_penalty", constraint_penalty
        );
    else
       this->_theParamDB->AddDoubleParam(
	   "method.constraint_penalty", 1.0
        );
    const Real& crossover_rate = probDescDB.get_real("method.crossover_rate");
    if (crossover_rate >= 0.)
      this->_theParamDB->AddDoubleParam(
          "method.crossover_rate", crossover_rate
        );
    else
      this->_theParamDB->AddDoubleParam(
          "method.crossover_rate", 0.75
        );
    if (this->probDescDB.get_string("method.mutation_type") != "")
      this->_theParamDB->AddDoubleParam(
	  "method.mutation_rate",
	  probDescDB.get_real("method.mutation_rate")
	  );
    else
      this->_theParamDB->AddDoubleParam(
	  "method.mutation_rate", 0.1
	  );
    this->_theParamDB->AddDoubleParam(
        "method.mutation_scale",
        probDescDB.get_real("method.mutation_scale")
        );
    this->_theParamDB->AddDoubleParam(
        "method.jega.percent_change",
        probDescDB.get_real("method.jega.percent_change")
        );
    this->_theParamDB->AddDoubleParam(
        "method.convergence_tolerance",
        probDescDB.get_real("method.convergence_tolerance")
        );
    this->_theParamDB->AddDoubleParam(
        "method.jega.shrinkage_percentage",
        probDescDB.get_real("method.jega.shrinkage_percentage")
        );
    this->_theParamDB->AddDoubleParam(
        "method.jega.fitness_limit",
        probDescDB.get_real("method.jega.fitness_limit")
        );

    // now get all the size_t's
    this->_theParamDB->AddSizeTypeParam(
        "method.jega.num_cross_points",
        probDescDB.get_sizet("method.jega.num_cross_points")
        );
    this->_theParamDB->AddSizeTypeParam(
        "method.jega.num_parents",
        probDescDB.get_sizet("method.jega.num_parents")
        );
    this->_theParamDB->AddSizeTypeParam(
        "method.jega.num_offspring",
        probDescDB.get_sizet("method.jega.num_offspring")
        );
    this->_theParamDB->AddSizeTypeParam(
        "method.jega.num_generations",
        probDescDB.get_sizet("method.jega.num_generations")
        );
    this->_theParamDB->AddSizeTypeParam(
        "method.jega.max_designs",
        probDescDB.get_sizet("method.jega.num_designs")
        );
    // Note that the population size, max evals, and max gens are in as ints.
    // Do a conversion for each here.
    this->_theParamDB->AddSizeTypeParam(
        "method.population_size",
        static_cast<size_t>(probDescDB.get_int("method.population_size"))
        );
    this->_theParamDB->AddSizeTypeParam(
        "method.max_iterations",
        static_cast<size_t>(probDescDB.get_int("method.max_iterations"))
        );
    this->_theParamDB->AddSizeTypeParam(
        "method.max_function_evaluations",
        static_cast<size_t>(
            probDescDB.get_int("method.max_function_evaluations")
            )
        );

    // Dakota does not currently expose the input that indicates that there is
    // a minimum allowable population size for the below limit selector.  Add
    // a default explicitly here to prevent any problems.
    this->_theParamDB->AddSizeTypeParam("method.jega.minimum_selections", 2);

    // Dakota does not currently expose the evaluation concurrency flag
    // through the interface nor should it b/c Dakota handles evaluations.
    this->_theParamDB->AddSizeTypeParam("method.jega.eval_concurrency", 1);

    // Now get all the booleans
    this->_theParamDB->AddBooleanParam(
        "method.print_each_pop",
        probDescDB.get_bool("method.print_each_pop")
        );

    // Dakota does not currently expose the flag to instruct the GA whether or
    // not to write the final data file and discards file.  Put those in here
    // to avoid warnings about them missing.
    this->_theParamDB->AddBooleanParam("method.print_final_data", true);
    this->_theParamDB->AddBooleanParam("method.print_discards", true);

    // Dakota does not currently expose the flag to instruct a nicher
    // as to whether or not to cache niched designs.  So put that flag in here
    // with a default true value.  It won't hurt anything if it is not needed.
    this->_theParamDB->AddBooleanParam("method.jega.cache_niched_designs", true);

    // now get all the strings.
    
    // Dakota does not expose the ability to specify the weighted sum
    // only fitness assessor b/c it is only for use with the favor
    // feasible selector.  Likewise, the favor feasible selector can
    // only be used with the weighted sum fitness asessor.  Because of
    // this, we will detect use of the the favor feasible and enforce
    // the use of the weighted sum only.  We will write a log message
    // about it.
    const string& selector =
        this->probDescDB.get_string("method.replacement_type");
    if (selector != "")
      this->_theParamDB->AddStringParam(
	  "method.replacement_type", selector);
    else if (this->methodName == SOGA)
      this->_theParamDB->AddStringParam(
	  "method.replacement_type", "elitist");
    else if (this->methodName == MOGA)
      this->_theParamDB->AddStringParam(
	  "method.replacement_type", "below_limit");
    
    const string& fitness = this->probDescDB.get_string("method.fitness_type");
    if(selector == "favor_feasible")
    {
        JEGALOG_II_G(lquiet(), this,
            text_entry(lquiet(),
            "Use of the favor_feasible selector has been detected.  Use of "
            "this selector type requires use of the \"weighted_sum_only\" "
            "fitness assessor.  Therefore, use of the \"" + fitness +
            "\" will be changed to use of the \"weighted_sum_only\".")
            )

        this->_theParamDB->AddStringParam(
            "method.fitness_type", "weighted_sum_only"
            );
    }
    else
    {
      if (fitness != "")
        this->_theParamDB->AddStringParam("method.fitness_type", fitness);
      else if (this->methodName == SOGA)
        this->_theParamDB->AddStringParam("method.fitness_type", "merit_function");
      else if (this->methodName == MOGA)
        this->_theParamDB->AddStringParam("method.fitness_type", "domination_count");
    }

    const string& crossover_operator =
        this->probDescDB.get_string("method.crossover_type");
    if (crossover_operator != "")
      this->_theParamDB->AddStringParam(
	  "method.crossover_type", crossover_operator);
    else
      this->_theParamDB->AddStringParam(
	  "method.crossover_type", "shuffle_random");

    const string& mutation_operator =
        this->probDescDB.get_string("method.mutation_type");
    if (mutation_operator != "")
      this->_theParamDB->AddStringParam(
	  "method.mutation_type", mutation_operator);
    else
      this->_theParamDB->AddStringParam(
	  "method.mutation_type", "replace_uniform");
 
   this->_theParamDB->AddIntegralParam(
        "method.output",
        this->probDescDB.get_short("method.output")
        );
    this->_theParamDB->AddStringParam(
        "method.initialization_type",
        this->probDescDB.get_string("method.initialization_type")
        );
    this->_theParamDB->AddStringParam(
        "method.flat_file",
        this->probDescDB.get_string("method.flat_file")
        );

    // Dakota does not currently expose the input that allows one to specify
    // the location of any data files written by JEGA.  Use the default but
    // specify it explicitly here to prevent any problems.
    this->_theParamDB->AddStringParam( 
        "method.jega.data_directory", "./" 
        );

    // Dakota does not currently expose the final data file name pattern
    // through the interface.
    this->_theParamDB->AddStringParam(
        "method.jega.final_data_filename", "finaldata#.dat"
        );

    // Dakota does not currently expose the main loop operator selection
    // through the interface.
    this->_theParamDB->AddStringParam(
        "method.jega.mainloop_type", "duplicate_free"
        );

    // The log file gets special attention.  If it is the default global log
    // file name, we will replace it with an empty string b/c we don't want the
    // created GA to think it owns the global log file.
    string log_file = probDescDB.get_string("method.log_file");
    this->_theParamDB->AddStringParam(
        "method.log_file",
        log_file == "JEGAGlobal.log" ? "" : log_file
        );
    const string& convergence_operator =
        this->probDescDB.get_string("method.jega.convergence_type");

    if (convergence_operator != "")
      this->_theParamDB->AddStringParam(
	  "method.jega.convergence_type", convergence_operator);
    else if (this->methodName == SOGA)
      this->_theParamDB->AddStringParam(
	  "method.jega.convergence_type", "average_fitness_tracker");
    else if (this->methodName == MOGA)
      this->_theParamDB->AddStringParam(
	  "method.jega.convergence_type", "metric_tracker");

    this->_theParamDB->AddStringParam(
        "method.jega.niching_type",
        this->probDescDB.get_string("method.jega.niching_type")
        );
    this->_theParamDB->AddStringParam(
        "method.jega.postprocessor_type",
        this->probDescDB.get_string("method.jega.postprocessor_type")
        );

    // Dakota does not expose a flat file delimiter for the case where we
    // are using the flat file initializer but the initializer is going to
    // be looking for one if it is in use.
    this->_theParamDB->AddStringParam("method.jega.initializer_delimiter", "");

    // now get all vector of doubles.
    const RealVector *dak_rv
        = &this->probDescDB.get_rv("method.jega.niche_vector");

    JEGA::DoubleVector niche_vector(
        dak_rv->values(),
        dak_rv->values() + dak_rv->length()
        );

    this->_theParamDB->AddDoubleVectorParam(
        "method.jega.niche_vector",
        niche_vector
        );

    dak_rv = &this->probDescDB.get_rv("method.jega.distance_vector");
    JEGA::DoubleVector distance_vector(
        dak_rv->values(),
        dak_rv->values() + dak_rv->length()
        );

    this->_theParamDB->AddDoubleVectorParam(
        "method.jega.distance_vector",
        distance_vector
        );
 
    // when recasting is active, the weights may be transformed; get off Model
    dak_rv = &iteratedModel.primary_response_fn_weights();
    JEGA::DoubleVector mow_vector(
        dak_rv->values(),
        dak_rv->values() + dak_rv->length()
        );

    this->_theParamDB->AddDoubleVectorParam(
        "responses.multi_objective_weights",
        mow_vector
        );

    // Dakota does not expose a capability to enter multiple flat file names
    // as a vector (can delimit them in the single string and JEGA will parse
    // them).  We will add an empty vector to prevent the warning from JEGA.
    this->_theParamDB->AddStringVectorParam(
        "method.flat_files", std::vector<std::string>()
        );



    // now get all int vector params.
    // nothing to do here.

    // now get all the double matrices.
    // nothing to do here.

    // now get all integer list params.
    // nothing to do here.

    // now get all string vectors
    // nothing to do here.

    // now get all string lists
    // nothing to do here.
}

void
JEGAOptimizer::LoadAlgorithmConfig(
    JEGA::FrontEnd::AlgorithmConfig& aConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    ParameterDatabase& pdb = aConfig.GetParameterDB();

    // Determine what kind of algorithm we are creating (MOGA or SOGA)
    // based on the methodName base class variable.
    AlgorithmConfig::AlgType algType;

    if(this->methodName == MOGA)
        algType = AlgorithmConfig::MOGA;

    else if(this->methodName == SOGA)
        algType = AlgorithmConfig::SOGA;

    else
        JEGALOG_II_G_F(this,
            text_entry(lfatal(), "JEGA Error: \"" +
		       method_enum_to_string(this->methodName) +
		       "\" is an invalid method specification.")
            )

    aConfig.SetAlgorithmType(algType);

    // We will use the method id as the algorithm name if it is non-empty and
    // we will otherwise use the method name.
    aConfig.SetAlgorithmName(
        this->method_id().empty() ?
	  method_enum_to_string(this->methodName) : this->method_id()
        );
}

void
JEGAOptimizer::LoadProblemConfig(
    JEGA::FrontEnd::ProblemConfig& pConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // This is carried out by loading the design variables, objective
    // functions, and constraints.
    this->LoadTheDesignVariables(pConfig);
    this->LoadTheObjectiveFunctions(pConfig);
    this->LoadTheConstraints(pConfig);
}

void
JEGAOptimizer::LoadTheDesignVariables(
    JEGA::FrontEnd::ProblemConfig& pConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // The information needed to create the design variable infos
    // is contained in the data structures of the base classes.
    // In particular, the Model (iteratedModel) has most of the
    // info.  We will create a shorthand for it here to ease syntax.
    Model& m = this->iteratedModel;
    size_t i, j, dsi_cntr;

    // Loop over all continuous variables and add an info object.  Don't worry
    // about the precision so much.  It is only considered by the operators
    // that do binary encoding such as the NPointParameterizedBinaryCrosser and
    // the RandomBitMutator.  Other than that, it is largely ignored by this
    // implementation.  It can have a fairly profound effect on the preformance
    // of those operators that use it to encode to binary.

//PDH: Should be able to simplify all of this code quite a bit.  As
//far as I can tell, the JEGA vectors are all just std vectors of the
//corresponding type.  Not sure what exactly pConfig is, though.

    const RealVector& clbs = m.continuous_lower_bounds();
    const RealVector& cubs = m.continuous_upper_bounds();
    StringMultiArrayConstView clabels = m.continuous_variable_labels();
    for(i=0; i<this->numContinuousVars; ++i)
      pConfig.AddContinuumRealVariable(clabels[i], clbs[i], cubs[i], 6);

    // now move on to the discrete variables.  The data for those is in the
    // Model as discrete_lower_bounds, discrete_upper_bounds, and
    // discrete_variable_labels.
    const IntVector& dilbs = m.discrete_int_lower_bounds();
    const IntVector& diubs = m.discrete_int_upper_bounds();
    StringMultiArrayConstView dilabels = m.discrete_int_variable_labels();
    const BitArray& di_set_bits = m.discrete_int_sets();
    const IntSetArray& dsiv = m.discrete_set_int_values();
    for(i=0, dsi_cntr=0; i<this->numDiscreteIntVars; ++i)
    {
      if (di_set_bits[i]) { // discrete set variables
        const IntSet& dak_set = dsiv[dsi_cntr];
        pConfig.AddDiscreteIntegerVariable(dilabels[i],
	  JEGA::IntVector(dak_set.begin(), dak_set.end()) );
	++dsi_cntr;
      }
      else // discrete range variables
        pConfig.AddContinuumIntegerVariable(dilabels[i], dilbs[i], diubs[i]);
    }

    // Next, load in the "discrete set of real" variables.
    const RealSetArray& dsrv = m.discrete_set_real_values();
    StringMultiArrayConstView drlabels = m.discrete_real_variable_labels();
    for(i=0; i<this->numDiscreteRealVars; ++i)
    {
      const RealSet& dak_set = dsrv[i];
      pConfig.AddDiscreteRealVariable(drlabels[i],
	JEGA::DoubleVector(dak_set.begin(), dak_set.end()) );
    }

//PDH: Have to map the string variables to indices.

    // Finally, load in the "discrete set of string" variables. These must
    // be mapped to discrete integer variables.
    StringMultiArrayConstView dslabels = m.discrete_string_variable_labels();
    const StringSetArray& dssv_values = m.discrete_set_string_values();
    for (i=0; i<this->numDiscreteStringVars; ++i) {
      const size_t &num_elements = dssv_values[i].size(); //assume > 0
      IntArray element_index(num_elements);
      for (j=0; j<num_elements; ++j)
	element_index[j] = j;
      pConfig.AddDiscreteIntegerVariable(dslabels[i],
	            JEGA::IntVector(element_index.begin(), 
		      element_index.end()) );
    }

    // Now make sure that an info was created for each variable.
    EDDY_ASSERT(pConfig.GetDesignTarget().GetNDV() == (this->numContinuousVars +
		this->numDiscreteIntVars + this->numDiscreteRealVars + 
		this->numDiscreteStringVars));
}

void
JEGAOptimizer::LoadTheObjectiveFunctions(
    JEGA::FrontEnd::ProblemConfig& pConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // For now, all objectives will be of type minimize.  Hopefully,
    // Dakota will soon support mixed extremization schemes.
    // Dakota does not support labeling objectives.  Until it does,
    // we will create a label that looks like "Nature Type Index".
    const StringArray&  labels = iteratedModel.response_labels();
    const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
    bool use_sense = !max_sense.empty();
    for(size_t i=0; i<this->numObjectiveFns; ++i)
      if (use_sense && max_sense[i])
        pConfig.AddNonlinearMaximizeObjective(
	    "Non-Linear Maximize " + labels[i]
            );
      else
        pConfig.AddNonlinearMinimizeObjective(
            "Non-Linear Minimize " + labels[i]
            );

    // see to it that the numbers match up.
    EDDY_ASSERT(pConfig.GetDesignTarget().GetNOF() == this->numObjectiveFns);
}

void
JEGAOptimizer::LoadTheConstraints(
    JEGA::FrontEnd::ProblemConfig& pConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // The information needed to create the constraint infos
    // is contained in the data structures of the base classes.
    // In particular, the Model (iteratedModel) has most of the
    // info.  We will create a shorthand for it here to ease syntax.
    const Model& m = this->iteratedModel;

    /**************************************************************************

    Note the order in which these are created.  Do not change this order.  It
    is this way because of the order in which responses are returned out of the
    Model.  Technically, it only involves the first two blocks which create the
    non-linear constraints.  But don't mess with any of it anyway.

    **************************************************************************/


    // start with non-linear (2-sided) inequality constraints.
    // The information we need for these is in
    // nonlinear_ineq_constraint_lower_bounds and
    // nonlinear_ineq_constraint_upper_bounds.  As with objective
    // functions, Dakota does not allow labeling of constraints.
    // we will create a label that looks like "Nature Type Index".
    const RealVector& nln_ineq_lwr_bnds
        = m.nonlinear_ineq_constraint_lower_bounds();
    const RealVector& nln_ineq_upr_bnds
        = m.nonlinear_ineq_constraint_upper_bounds();

//PDH: Dakota nonlinear constraints to JEGA nonlinear constraints.
//     Don't know what the JEGA data structure is.  These are all
//     mapped on entry at a time.
//     Looks like we don't have to worry about (JEGA) order.  Need to
//     determine if they have to be two-sided for JEGA.

    // Loop over all two sided non linear inequality constraitns and add an
    // info object for each.
    for(size_t i=0; i<this->numNonlinearIneqConstraints; ++i)
        pConfig.AddNonlinearTwoSidedInequalityConstraint(
            "Non-Linear Two-Sided Inequality " + asstring(i),
            nln_ineq_lwr_bnds[i], nln_ineq_upr_bnds[i]
            );

    // now do non-linear equality constraints.  The information we need for
    // these is in nonlinear_eq_constraint_targets.
    const RealVector& nln_eq_targets = m.nonlinear_eq_constraint_targets();
    for(size_t i=0; i<this->numNonlinearEqConstraints; ++i)
        pConfig.AddNonlinearEqualityConstraint(
            "Non-Linear Equality " + asstring(i), nln_eq_targets[i]
            );

//PDH: Dakota linear constraints to JEGA linear constraints.
//     Don't know what the JEGA data structure is.  These are all
//     mapped on entry at a time.
//     Looks like we don't have to worry about (JEGA) order.  Need to
//     determine if they have to be two-sided for JEGA.

    // now do linear (2-sided) inequality constraints  The information we need
    // for these is in linear_ineq_constraint_lower_bounds and
    // linear_ineq_constraint_upper_bounds.
    // In addition to bounds, these accept coefficients for possible shortcut
    // evaluation.  That information is in linear_ineq_constraint_coeffs.
    const RealVector& lin_ineq_lwr_bnds
        = m.linear_ineq_constraint_lower_bounds();
    const RealVector& lin_ineq_upr_bnds
        = m.linear_ineq_constraint_upper_bounds();
    const RealMatrix& lin_ineq_coeffs
        = m.linear_ineq_constraint_coeffs();

    JEGA::DoubleVector lin_ineq_coeffs_row(lin_ineq_coeffs.numCols());

//PDH: RealMatrix -> set of std::vector
//     Just need the individual rows.  Check copy_row_vector to see if
//     transpose is also needed.

    for(size_t i=0; i<numLinearIneqConstraints; ++i) {
        copy_row_vector(lin_ineq_coeffs, i, lin_ineq_coeffs_row);

        pConfig.AddLinearTwoSidedInequalityConstraint(
            "Linear Two-Sided Inequality " + asstring(i),
            lin_ineq_lwr_bnds[i], lin_ineq_upr_bnds[i],
            lin_ineq_coeffs_row
            );
    }

    // now do linear equality constraints.  The information we need for these
    // is in lin_eq_targets. In addition to targets, these accept coefficients
    // for possible shortcut evaluation.  That information is in
    // linear_eq_constraint_coeffs.
    const RealVector& lin_eq_targets = m.linear_eq_constraint_targets();
    const RealMatrix& lin_eq_coeffs = m.linear_eq_constraint_coeffs();

    JEGA::DoubleVector lin_eq_coeffs_row(lin_eq_coeffs.numCols());

//PDH: RealMatrix -> set of std::vector
//     Just need the individual rows.  Check copy_row_vector to see if
//     transpose is also needed.

    for(size_t i=0; i<numLinearEqConstraints; ++i) {
        copy_row_vector(lin_eq_coeffs, i, lin_eq_coeffs_row);

        pConfig.AddLinearEqualityConstraint(
            "Linear Equality " + asstring(i),
            lin_eq_targets[i], 0.0, lin_eq_coeffs_row
            );
    }

    // see to it that the numbers match up.
    EDDY_ASSERT(pConfig.GetDesignTarget().GetNCN() ==
        (this->numNonlinearIneqConstraints + this->numLinearIneqConstraints +
        this->numNonlinearEqConstraints + this->numLinearEqConstraints));
}

void
JEGAOptimizer::GetBestSolutions(
    const JEGA::Utilities::DesignOFSortSet& from,
    const JEGA::Algorithms::GeneticAlgorithm& theGA,
    std::multimap<RealRealPair, JEGA::Utilities::Design*>& designSortMap
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(this->methodName == MOGA)
        this->GetBestMOSolutions(from, theGA, designSortMap);

    else if(this->methodName == SOGA)
        this->GetBestSOSolutions(from, theGA, designSortMap);

    else
    {
        JEGALOG_II_G_F(this,
            text_entry(lfatal(), "JEGA Error: \"" +
		       method_enum_to_string(this->methodName) +
		       "\" is an invalid method specification.")
            )
    }
}


void
JEGAOptimizer::GetBestMOSolutions(
    const JEGA::Utilities::DesignOFSortSet& from,
    const JEGA::Algorithms::GeneticAlgorithm& theGA,
    std::multimap<RealRealPair, JEGA::Utilities::Design*>& designSortMap
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(from.empty())
        return;

    // Start by removing any infeasible from "from".
    DesignOFSortSet feasible(DesignStatistician::GetFeasible(from));

    // We need the extremes of the feasible solutions; if no feasible
    // designs, this will be empty and unused below.
    DoubleExtremes extremeSet(
        MultiObjectiveStatistician::FindParetoExtremes(feasible)
        );

    const DesignTarget& target = from.front()->GetDesignTarget();
    const ConstraintInfoVector& cnInfos = target.GetConstraintInfos();

    // get number of objective functions
    const eddy::utilities::uint64_t nof = target.GetNOF();

    // get total number of constraints (nonlinear and linear)
    const eddy::utilities::uint64_t noc = target.GetNCN();

    // iterate the designs and sort first by constraint violation,
    // then objective function
    DesignOFSortSet::const_iterator design_it(from.begin());
    const DesignOFSortSet::const_iterator design_end(from.end());

    for(; design_it != design_end; ++design_it)
    {
        // L2 constraint violation for this design
        double constraintViolation = 0.0;

        for(size_t i=0; i<noc; ++i)
            constraintViolation +=
                Math::Pow(cnInfos[i]->GetViolationAmount(**design_it), 2);

        // sum-of-squared distance for this Design over objective functions.
        double utopiaDistance = 0.0;
        if(constraintViolation > 0.0)
            utopiaDistance = DBL_MAX;
        else 
        {
            // compute the sum-of-squares between the current point
            // and the utopia point; if the feasible set is empty,
            // we should never reach this block,
            for(size_t i=0; i<nof; ++i)
                utopiaDistance += Math::Pow(
                    (*design_it)->GetObjective(i) - extremeSet.get_min(i), 2
                    );
        }

        // insert the design into the map, keeping only numBest
        RealRealPair metrics(constraintViolation, utopiaDistance);

        if(designSortMap.size() < this->numFinalSolutions)
            designSortMap.insert(std::make_pair(metrics, *design_it));
        else 
        {
            // if this Design is better than the worst, remove worst
            // and insert this one
            std::multimap<RealRealPair, Design*>::iterator worst_it = 
                --designSortMap.end();

            if(metrics < worst_it->first)
            {
                designSortMap.erase(worst_it);
                designSortMap.insert(std::make_pair(metrics, *design_it));
            }
        }
    }
}


void
JEGAOptimizer::GetBestSOSolutions(
    const JEGA::Utilities::DesignOFSortSet& from,
    const JEGA::Algorithms::GeneticAlgorithm& theGA,
    std::multimap<RealRealPair, JEGA::Utilities::Design*>& designSortMap
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(from.empty()) return;

    const DesignTarget& target = from.front()->GetDesignTarget();
    const ConstraintInfoVector& cnInfos = target.GetConstraintInfos();

    // get number of objective functions
    const eddy::utilities::uint64_t nof = target.GetNOF();

    // get total number of constraints (nonlinear and linear)
    const eddy::utilities::uint64_t noc = target.GetNCN();

    // in order to order the points, need the weights; get them from
    // the GA to ensure solver/final results consistency
    JEGA::DoubleVector weights;
    try 
    {
        const JEGA::Algorithms::SOGA& 
	    the_ga = dynamic_cast<const JEGA::Algorithms::SOGA&>(theGA);
	weights = the_ga.GetWeights();
    }
    catch(const std::bad_cast& bc_except) 
    {
      Cerr << "\nError: could not cast GeneticAlgorithm to SOGA; exception:\n" 
	   << bc_except.what() << std::endl;
      abort_handler(-1);
    }

    // iterate the designs and sort first by constraint violation,
    // then objective function
    DesignOFSortSet::const_iterator design_it(from.begin());
    const DesignOFSortSet::const_iterator design_end(from.end());

    for(; design_it != design_end; ++design_it)
    {
        // L2 constraint violation for this design
        double constraintViolation = 0.0;

        for(size_t i=0; i<noc; ++i)
            constraintViolation +=
                Math::Pow(cnInfos[i]->GetViolationAmount(**design_it), 2);

        // Multi-objective sum for this Design over objective functions.
        // In the single objective case we can store
        // objective even if there's a constraint violation.
        double objectiveFunction = 
            SingleObjectiveStatistician::ComputeWeightedSum(
                **design_it, weights
                );

        // insert the design into the map, keeping only numBest
        RealRealPair metrics(constraintViolation, objectiveFunction);

        if(designSortMap.size() < this->numFinalSolutions)
            designSortMap.insert(std::make_pair(metrics, *design_it));
        else 
        {
            // if this Design is better than the worst, remove worst
            // and insert this one
            std::multimap<RealRealPair, Design*>::iterator worst_it =
                --designSortMap.end();

            if(metrics < worst_it->first)
            {
                designSortMap.erase(worst_it);
                designSortMap.insert(std::make_pair(metrics, *design_it));
            }
        }
    }
}


JEGA::DoubleMatrix
JEGAOptimizer::ToDoubleMatrix(
    const VariablesArray& variables
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // Prepare the resultant matrix with proper initial capacity
    JEGA::DoubleMatrix ret(variables.size());

    // Iterate the variables objects and create entries in the new matrix
    size_t i = 0;
    for(VariablesArray::const_iterator it(variables.begin());
        it!=variables.end(); ++it, ++i)
    {
        // Store the continuous and discrete variables arrays for use below.
        const RealVector& cvs  = (*it).continuous_variables();
        const IntVector&  divs = (*it).discrete_int_variables();
        const RealVector& drvs = (*it).discrete_real_variables();

        // Prepare the row we are working with to hold all variable values.
        ret[i].reserve(cvs.length() + divs.length() + drvs.length());

        // Copy in first the continuous followed by the discrete values.
        ret[i].insert(ret[i].end(), cvs.values(), cvs.values()+cvs.length());
        ret[i].insert(ret[i].end(), divs.values(), divs.values()+divs.length());
        ret[i].insert(ret[i].end(), drvs.values(), drvs.values()+drvs.length());
    }

    return ret;
}


/*
===============================================================================
Subclass Overridable Methods
===============================================================================
*/




/*
===============================================================================
Private Methods
===============================================================================
*/




/*
===============================================================================
Structors
===============================================================================
*/
JEGAOptimizer::JEGAOptimizer(
    ProblemDescDB& problem_db, Model& model
    ) :
        Optimizer(problem_db, model),
        _theParamDB(0x0),
        _theEvalCreator(0x0)
{
    EDDY_FUNC_DEBUGSCOPE

    // JEGAOptimizer now makes use of the JEGA front end core project to run
    // an algorithm.  In order to do this, it creates and loads a DesignTarget,
    // a ProblemConfig, and an AlgorithmConfig.

    // The first step is to initialize JEGA via the front end Driver
    // class.  The data needed is available from the problem description
    // database.  This should only happen once in any run of Dakota regardless
    // of how many JEGAOptimizers are used.
    if(!JEGA::FrontEnd::Driver::IsJEGAInitialized())
    {
        // The random seed must be handled separately because the sentry value
        // for JEGA (0) is not the same as the sentry value for Dakota (-1).
        int rseed_temp = this->probDescDB.get_int("method.random_seed");

        // if the rseed is negative, it is the sentry value and we will use the
        // JEGA sentry value of 0.
        unsigned int rSeed = (rseed_temp < 0) ? 0 :
            static_cast<unsigned int>(rseed_temp);

        // For now, we will use the level of the first instance of an optimizer
        // as the level for the global log.  This is only potentially not ideal
        // in the case of strategies.  The 4 - below is to account for the fact
        // that the actual dakota levels count upwards by increasing amount of
        // output while the dakota_levels must count downwards in order to be
        // compatable with the logging library code.
        short dakLev = this->probDescDB.get_short("method.output");
        LogLevel jegaLev;

        switch (dakLev)
        {
            case SILENT_OUTPUT: jegaLev = lsilent(); break;
            case NORMAL_OUTPUT: jegaLev = lnormal(); break;
            case DEBUG_OUTPUT: jegaLev = ldebug(); break;
            case QUIET_OUTPUT: jegaLev = lquiet(); break;
            case VERBOSE_OUTPUT: jegaLev = lverbose(); break;
            default: jegaLev = ldefault();
        }

        JEGA::FrontEnd::Driver::InitializeJEGA(
            "JEGAGlobal.log", jegaLev, rSeed
            );
    }

    // If we failed to init, we cannot continue.
    JEGAIFLOG_II_G_F(!JEGA::FrontEnd::Driver::IsJEGAInitialized(), this,
        text_entry(lfatal(), "JEGAOptimizer Error: Unable to initialize JEGA")
        );

    // we only need to load up the parameter database at this point.
    this->LoadTheParameterDatabase();

    // population_size is extracted by JEGA in
    // GeneticAlgorithmInitializer::PollForParameters(), but it is
    // also needed here to specify the algorithmic concurrency.  Note
    // that the JEGA population size may grow or shrink during its
    // iterations, so this is only an initial estimate.
    int pop_size = this->probDescDB.get_int("method.population_size");
    this->maxEvalConcurrency *= pop_size;

    // Assign iterator-specific default for numFinalSolutions
    if (methodName == MOGA && !this->numFinalSolutions)
      this->numFinalSolutions
	= std::numeric_limits<std::size_t>::max(); // moga returns all Pareto

    // We only ever need one EvaluatorCreator so we can create it now.
    this->_theEvalCreator = new EvaluatorCreator(iteratedModel);
}

JEGAOptimizer::~JEGAOptimizer(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    delete this->_theEvalCreator;
    delete this->_theParamDB;
}


/*
===============================================================================
Inner Class Implementations
===============================================================================
*/
void
JEGAOptimizer::Evaluator::SeparateVariables(
    const Design& from,
    RealVector& intoCont,
    IntVector&  intoDiscInt,
    RealVector& intoDiscReal,
    StringMultiArray& intoDiscString
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    size_t num_cv  = this->_model.cv(), num_div = this->_model.div(),
           num_drv = this->_model.drv(), num_dsv = this->_model.dsv();

    // "into" containers may not yet be sized. If not, size them.  If they are,
    // don't size them b/c it will be a lot of wasted effort.
    if(intoCont.length()     != num_cv)  intoCont.size(num_cv);
    if(intoDiscInt.length()  != num_div) intoDiscInt.size(num_div);
    if(intoDiscReal.length() != num_drv) intoDiscReal.size(num_drv);
    // Strings are multi_arrays, not vectors
    if(intoDiscString.num_elements() != num_dsv) {
      StringMultiArray::extent_gen extents;
      intoDiscString.resize(extents[num_dsv]);
    }
    
    // Because we cannot easily distinguish real from integral variables
    // (true of both continuum and discrete), we will rely on the fact that
    // the infos are stored in the order in which we added them which is the
    // order laid out in JEGAOptimizer::LoadTheDesignVariables.  We will split
    // them up accordingly.  We need the design variable infos to get variable
    // values out of the design.
    const DesignTarget& target = from.GetDesignTarget();
    const DesignVariableInfoVector& dvis = target.GetDesignVariableInfos();

//PDH: I think there's something that can be done here with regard to
//     mapping discrete variables, but I don't know what the JEGA data
//     structures are at the moment.

    // We will be marching through the dvis and need to keep track of were we
    // are from loop to loop.
    size_t i, dvi_cntr = 0;

    // Start with the DAKOTA continuous variables.
    for(i=0; i<num_cv; ++i, ++dvi_cntr)
    {
      EDDY_ASSERT(dvis[dvi_cntr]->IsContinuum());
      intoCont[i] = dvis[dvi_cntr]->WhichValue(from);
    }

    // Move on to the DAKOTA discrete integer {range,set} variables.
    const BitArray& di_set_bits = this->_model.discrete_int_sets();
    for(i=0; i<num_div; ++i, ++dvi_cntr)
    {
      if (di_set_bits[i]) // set variables are discrete nature in JEGA
        EDDY_ASSERT(dvis[dvi_cntr]->IsDiscrete());
      else // range variables are continuum nature in JEGA
        EDDY_ASSERT(dvis[dvi_cntr]->IsContinuum());
      intoDiscInt[i] = static_cast<int>(dvis[dvi_cntr]->WhichValue(from));
    }

    // Next process the "discrete set of real" variables.
    // These will also be discrete nature in JEGA.
    for(i=0; i<num_drv; ++i, ++dvi_cntr)
    {
      EDDY_ASSERT(dvis[dvi_cntr]->IsDiscrete());
      intoDiscReal[i] = dvis[dvi_cntr]->WhichValue(from);
    }
    // Finally, process the "discrete set of string" variables.
    // These will also be discrete in JEGA, and must be mapped
    // back to their associated string values.
    const StringSetArray& dssv_values = _model.discrete_set_string_values();
    for(i=0; i<num_dsv; ++i, ++dvi_cntr)
    {
      EDDY_ASSERT(dvis[dvi_cntr]->IsDiscrete());
      const int &element_index = static_cast<int>(dvis[dvi_cntr]->WhichValue(from));
      intoDiscString[i] = set_index_to_value(element_index, dssv_values[i]);
    }
}

void
JEGAOptimizer::Evaluator::RecordResponses(
    const RealVector& from,
    Design& into
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // get the target for information.
    const DesignTarget& target = this->GetDesignTarget();

    // get the information about the constraints.
    const ConstraintInfoVector& cnis = target.GetConstraintInfos();

    // prepare to store the location in the responses vector.
    RealVector::ordinalType loc = 0;

//PDH: I think this is going from Dakota responses to JEGA responses,
//     e.g., after a function evaluation.

    // find out how many objective and constraint functions there are.
    const size_t nof = target.GetNOF();
    const size_t ncn = target.GetNCN();

    // record the objective functions first.
    for(size_t i=0; i<nof; ++i, ++loc)
        into.SetObjective(i, from[loc]);

    // now record the nonlinear constraints.  To do this,
    // we will need to know how many there are.  They will be the
    // first of the constraints in the design.
    const size_t num_nonlin_cn = this->GetNumberNonLinearConstraints();
    for(size_t cn=0; cn<num_nonlin_cn && cn<ncn; ++cn, ++loc)
    {
        into.SetConstraint(cn, from[loc]);
        cnis[cn]->RecordViolation(into);
    }
}

bool
JEGAOptimizer::Evaluator::Evaluate(
    DesignGroup& group
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing group evaluation.")
        )

    // check for trivial abort conditions
    if(group.IsEmpty()) return true;

    // first, let's see if we can avoid any evaluations.
    ResolveClones(group);

    // we'll prepare containers for repeated use without re-construction
    RealVector       contVars;
    IntVector        discIntVars;
    RealVector       discRealVars;
    StringMultiArray discStringVars;

    // prepare to iterate over the group
    DesignDVSortSet::const_iterator it(group.BeginDV());
    const DesignDVSortSet::const_iterator e(group.EndDV());

    // these quantities will be used below
    const DesignTarget& target = this->GetDesignTarget();

    // Find out the counts on the different types of constraints
    const size_t num_nonlin_cn = this->GetNumberNonLinearConstraints();
    // const size_t num_lin_cn = this->GetNumberLinearConstraints();

    // Get the information about the constraints.
    const ConstraintInfoVector& cninfos = target.GetConstraintInfos();

    // Prepare to store the number of requests in order to avoid overshooting
    // the limit.
    const eddy::utilities::uint64_t priorReqs = this->GetNumberEvaluations();
    eddy::utilities::uint64_t numEvalReqs = 0;

    // store an iterator to the first linear constraint so that
    // we can evaluate it using the cinfo objects.

    // prepare to iterate
    ConstraintInfoVector::const_iterator flincn(
        cninfos.begin() + num_nonlin_cn
        );
    ConstraintInfoVector::const_iterator cit;

    // prepare to return the success of this.  Success occurs only if all
    // designs wind up evaluated and non-illconditioned.
    bool ret = true;

    for(; it!=e; ++it)
    {
        // If this Design is evaluated, let's skip it.
        if((*it)->IsEvaluated()) continue;

        // If we've reached our maximum allowable number of
        // evaluations, tag remaining as evaluated and illconditioned.
        // By doing so, they will be flushed from the algorithm.
        if((priorReqs + numEvalReqs) >= this->GetMaxEvaluations())
        {
            (*it)->SetEvaluated(true);
            (*it)->SetIllconditioned(true);
            ret = false;
            continue;
        }

        // extract the real and continuous variables
        // from the current Design
        this->SeparateVariables(**it, contVars, discIntVars, discRealVars,
	    discStringVars);

        // send this guy out for evaluation using the _model.

        // first, set the current values of the variables in the model
        this->_model.continuous_variables(contVars);
        this->_model.discrete_int_variables(discIntVars);
        this->_model.discrete_real_variables(discRealVars);
	// Strings set by calling single value setter for each
	for (size_t i=0; i<discStringVars.num_elements(); ++i)
	  this->_model.discrete_string_variable(discStringVars[i],i);
	// Could use discrete_string_varables to avoid overhead of repeated 
	// function calls, but it takes a StringMultiArrayConstView, which
	// must be created from discStringVars. Maybe there's a simpler way,
	// but...
	// const size_t &dsv_len = discStringVars.num_elements();
	// StringMultiArrayConstView dsv_view = discStringVars[ 
	//   boost::indices[idx_range(0,dsv_len)]];
        // this->_model.discrete_string_variables(dsv_view);
	
        // now request the evaluation in synchronous or asyncronous mode.
        if(this->_model.asynch_flag())
        {
            // The following method call will use the default
            // Active set vector which is to just compute
            // function values, no gradients or hessians.
            this->_model.evaluate_nowait();
        }
        else
        {
            // The following method call will use the default
            // Active set vector which is to just compute
            // function values, no gradients or hessians.
            this->_model.evaluate();

            // increment the number of performed evaluations by 1
            this->IncrementNumberEvaluations();

            // Record the responses back into the Design
            const RealVector& ftn_vals =
                this->_model.current_response().function_values();

            this->RecordResponses(ftn_vals, **it);

            // Label this guy as now being evaluated.
            (*it)->SetEvaluated(true);

            // now check the feasibility of this design
            target.CheckFeasibility(**it);
        }

        // no matter what, we want to increment the number of evaluations
        // or evaluation requests.
        ++numEvalReqs;

        // The responses do not (or will not) include the linear
        // constraint values. We have to compute them ourselves.
        // we can do it using the "EvaluateConstraint" method which
        // will only succeed for linear constraints for which
        // coefficients have been supplied.
        for(cit=flincn; cit!=cninfos.end(); ++cit)
        {
            (*cit)->EvaluateConstraint(**it);
            (*cit)->RecordViolation(**it);
        }
    }

    // If we did our evaluations asynchronously, we did not yet record
    // the results (because they were not available).  We need to do so
    // now.  The call to _model.synchronize causes the program to block
    // until all the results are available.  We can then record the
    // responses in the same fashion as above.  Note that the linear
    // constraints have already been computed!!
    if(this->_model.asynch_flag())
    {
        // Wait for the responses.
        const IntResponseMap& response_map = this->_model.synchronize();
        size_t num_resp = response_map.size();

        // increment the number of evaluations by the number of responses
        this->IncrementNumberEvaluations(num_resp);

        EDDY_ASSERT(num_resp == numEvalReqs);

        // prepare to access the elements of the response_map by iterator.
        IntRespMCIter r_cit = response_map.begin();

        // Record the set of responses in the DesignGroup
        for(it=group.BeginDV(); it!=e; ++it, ++r_cit)
        {
            // we didn't send already-evaluated Designs out for evaluation
            // so skip them here as well.
            if((*it)->IsEvaluated()) continue;

            // Put the responses into the Design properly.
            this->RecordResponses(r_cit->second.function_values(), **it);

            // Label this guy as now being evaluated.
            (*it)->SetEvaluated(true);

            // now check the feasibility of this design
            target.CheckFeasibility(**it);
        }
    }

    return ret;
}

/*
===============================================================================
End Namespace
===============================================================================
*/
} // namespace Dakota
