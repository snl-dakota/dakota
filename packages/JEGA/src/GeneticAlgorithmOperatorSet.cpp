/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithmOperatorSet.

    NOTES:

        See notes of GeneticAlgorithmOperatorSet.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed May 28 16:03:18 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithmOperatorSet class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Mutators/NullMutator.hpp>
#include <Crossers/NullCrosser.hpp>
#include <MainLoops/NullMainLoop.hpp>
#include <Selectors/NullSelector.hpp>
#include <Convergers/NullConverger.hpp>
#include <Evaluators/NullEvaluator.hpp>
#include <GeneticAlgorithmOperatorSet.hpp>
#include <Initializers/NullInitializer.hpp>
#include <../Utilities/include/Logging.hpp>
#include <PostProcessors/NullPostProcessor.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <FitnessAssessors/NullFitnessAssessor.hpp>
#include <NichePressureApplicators/NullNichePressureApplicator.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace JEGA::Logging;
using namespace JEGA::Utilities;





/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {





/*
================================================================================
Nested Utility Class Definitions
================================================================================
*/
class GeneticAlgorithmOperatorSet::DefaultOperators
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /// The default mutator.
        NullMutator _mutator;

        /// The default converger
        NullConverger _converger;

        /// The default crosser
        NullCrosser _crosser;

        /// The default niche pressure applicator
        NullNichePressureApplicator _nicher;

        /// The default fitness assessor
        NullFitnessAssessor _ftnsAssessor;

        /// The default initializer
        NullInitializer _initializer;

        /// The default selector
        NullSelector _selector;

        /// The default post processor
        NullPostProcessor _postProcessor;

        /// The default evaluator
        NullEvaluator _evaluator;

        /// The default main loop
        NullMainLoop _mainLoop;


    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /// Constructs a DefaultOperators object using the supplied algorithm.
        /**
         * \param algorithm The GA for which the default operators are to be
         *                  created.
         */
        DefaultOperators(
            GeneticAlgorithm& algorithm
            ) :
                _mutator(algorithm),
                _converger(algorithm),
                _crosser(algorithm),
                _nicher(algorithm),
                _ftnsAssessor(algorithm),
                _initializer(algorithm),
                _selector(algorithm),
                _postProcessor(algorithm),
                _evaluator(algorithm),
                _mainLoop(algorithm)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

}; // class GeneticAlgorithmOperatorSet::DefaultOperators






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
#define SET_METHOD(optype, opname) \
    void \
    GeneticAlgorithmOperatorSet::Set##optype( \
        GeneticAlgorithm##optype* op \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        EDDY_ASSERT((op == 0x0) || (&op->GetAlgorithm() == &this->_algorithm)); \
        opname = (op == 0x0) ? &this->_defaults->opname : op; \
        JEGALOG_II_G(ldebug(), this, text_entry(ldebug(), #optype " is now " \
            "set to \"" + opname->GetName() + "\" in the operator set.") \
            ) \
    }

SET_METHOD(Mutator, _mutator)
SET_METHOD(Crosser, _crosser)
SET_METHOD(Converger, _converger)
SET_METHOD(NichePressureApplicator, _nicher)
SET_METHOD(Evaluator, _evaluator)
SET_METHOD(FitnessAssessor, _ftnsAssessor)
SET_METHOD(MainLoop, _mainLoop)
SET_METHOD(Initializer, _initializer)
SET_METHOD(Selector, _selector)
SET_METHOD(PostProcessor, _postProcessor)

#undef SET_METHOD




/*
================================================================================
Accessors
================================================================================
*/
#define GET_BODY(opname) \
    EDDY_FUNC_DEBUGSCOPE \
    EDDY_ASSERT(this->opname != 0x0) \
    return *this->opname;

#define GET_METHOD(optype, opname) \
    GeneticAlgorithm##optype& \
    GeneticAlgorithmOperatorSet::Get##optype( \
        ) \
    { \
        GET_BODY(opname) \
    }

#define CONST_GET_METHOD(optype, opname) \
    const GeneticAlgorithm##optype& \
    GeneticAlgorithmOperatorSet::Get##optype( \
        ) const \
    { \
        GET_BODY(opname) \
    }

GET_METHOD(Mutator, _mutator)
GET_METHOD(Crosser, _crosser)
GET_METHOD(Converger, _converger)
GET_METHOD(NichePressureApplicator, _nicher)
GET_METHOD(Evaluator, _evaluator)
GET_METHOD(FitnessAssessor, _ftnsAssessor)
GET_METHOD(MainLoop, _mainLoop)
GET_METHOD(Initializer, _initializer)
GET_METHOD(Selector, _selector)
GET_METHOD(PostProcessor, _postProcessor)

#undef GET_METHOD

CONST_GET_METHOD(Mutator, _mutator)
CONST_GET_METHOD(Crosser, _crosser)
CONST_GET_METHOD(Converger, _converger)
CONST_GET_METHOD(NichePressureApplicator, _nicher)
CONST_GET_METHOD(Evaluator, _evaluator)
CONST_GET_METHOD(FitnessAssessor, _ftnsAssessor)
CONST_GET_METHOD(MainLoop, _mainLoop)
CONST_GET_METHOD(Initializer, _initializer)
CONST_GET_METHOD(Selector, _selector)
CONST_GET_METHOD(PostProcessor, _postProcessor)

#undef CONST_GET_METHOD
#undef GET_BODY


/*
================================================================================
Public Methods
================================================================================
*/
const GeneticAlgorithmOperatorSet&
GeneticAlgorithmOperatorSet::operator = (
    const GeneticAlgorithmOperatorSet& rhs
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(&rhs._algorithm == &this->_algorithm);

    if(this == &rhs) return *this;

    // Setting to null will have the effect of using the default.

    SetMutator(rhs.HasDefaultMutator() ? 0x0 : rhs._mutator);
    SetCrosser(rhs.HasDefaultCrosser() ? 0x0 : rhs._crosser);
    SetConverger(rhs.HasDefaultConverger() ? 0x0 : rhs._converger);
    SetFitnessAssessor(
        rhs.HasDefaultFitnessAssessor() ? 0x0 : rhs._ftnsAssessor
        );
    SetInitializer(rhs.HasDefaultInitializer() ? 0x0 : rhs._initializer);
    SetNichePressureApplicator(
        rhs.HasDefaultNichePressureApplicator() ? 0x0 : rhs._nicher
        );
    SetSelector(rhs.HasDefaultSelector() ? 0x0 : rhs._selector);
    SetPostProcessor(rhs.HasDefaultPostProcessor() ? 0x0 : rhs._postProcessor);
    SetEvaluator(rhs.HasDefaultEvaluator() ? 0x0 : rhs._evaluator);
    SetMainLoop(rhs.HasDefaultMainLoop() ? 0x0 : rhs._mainLoop);

    return *this;
}

void
GeneticAlgorithmOperatorSet::Clear(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_mutator = &this->_defaults->_mutator;
    this->_converger = &this->_defaults->_converger;
    this->_crosser = &this->_defaults->_crosser;
    this->_ftnsAssessor = &this->_defaults->_ftnsAssessor;
    this->_initializer = &this->_defaults->_initializer;
    this->_nicher = &this->_defaults->_nicher;
    this->_selector = &this->_defaults->_selector;
    this->_postProcessor = &this->_defaults->_postProcessor;
    this->_evaluator = &this->_defaults->_evaluator;
    this->_mainLoop = &this->_defaults->_mainLoop;
}


GeneticAlgorithmOperatorSet
GeneticAlgorithmOperatorSet::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    GeneticAlgorithmOperatorSet ret(algorithm);

    // We do not clone default operators.  We just set leave them
    // as default for the new set.

#define ASSIGN_CLONE(optype, opname) \
    if(!this->HasDefault##optype()) \
        ret.opname = static_cast<GeneticAlgorithm##optype*>( \
            this->Get##optype().Clone(algorithm) \
            );

    ASSIGN_CLONE(Mutator, _mutator)
    ASSIGN_CLONE(Crosser, _crosser)
    ASSIGN_CLONE(Converger, _converger)
    ASSIGN_CLONE(FitnessAssessor, _ftnsAssessor)
    ASSIGN_CLONE(Initializer, _initializer)
    ASSIGN_CLONE(NichePressureApplicator, _nicher)
    ASSIGN_CLONE(Selector, _selector)
    ASSIGN_CLONE(Evaluator, _evaluator)
    ASSIGN_CLONE(MainLoop, _mainLoop)
    ASSIGN_CLONE(PostProcessor, _postProcessor)

#undef ASSIGN_CLONE

    return ret;
}

#define HAS_DEFAULT_METHOD(optype, opname) \
    bool \
    GeneticAlgorithmOperatorSet::HasDefault##optype( \
        ) const \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return this->opname == &this->_defaults->opname; \
    }

HAS_DEFAULT_METHOD(Mutator, _mutator)
HAS_DEFAULT_METHOD(Crosser, _crosser)
HAS_DEFAULT_METHOD(Converger, _converger)
HAS_DEFAULT_METHOD(NichePressureApplicator, _nicher)
HAS_DEFAULT_METHOD(Evaluator, _evaluator)
HAS_DEFAULT_METHOD(FitnessAssessor, _ftnsAssessor)
HAS_DEFAULT_METHOD(MainLoop, _mainLoop)
HAS_DEFAULT_METHOD(Initializer, _initializer)
HAS_DEFAULT_METHOD(Selector, _selector)
HAS_DEFAULT_METHOD(PostProcessor, _postProcessor)

#undef HAS_DEFAULT_METHOD

void
GeneticAlgorithmOperatorSet::DestroyOperators(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    //DestroyEvaluator();
    this->DestroyFitnessAssessor();
    this->DestroyMainLoop();
    this->DestroySelector();
    this->DestroyCrosser();
    this->DestroyNichePressureApplicator();
    this->DestroyInitializer();
    this->DestroyConverger();
    this->DestroyMutator();
    this->DestroyPostProcessor();
}


#define DESTROY_METHOD(optype, opname) \
    void \
    GeneticAlgorithmOperatorSet::Destroy##optype( \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        if(!this->HasDefault##optype()) { \
            delete this->opname; this->Set##optype(0x0); \
        } \
    }

DESTROY_METHOD(Mutator, _mutator)
DESTROY_METHOD(Crosser, _crosser)
DESTROY_METHOD(Converger, _converger)
DESTROY_METHOD(NichePressureApplicator, _nicher)
DESTROY_METHOD(Evaluator, _evaluator)
DESTROY_METHOD(FitnessAssessor, _ftnsAssessor)
DESTROY_METHOD(MainLoop, _mainLoop)
DESTROY_METHOD(Initializer, _initializer)
DESTROY_METHOD(Selector, _selector)
DESTROY_METHOD(PostProcessor, _postProcessor)

#undef DESTROY_METHOD

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
GeneticAlgorithmOperatorSet::GeneticAlgorithmOperatorSet(
    GeneticAlgorithm& algorithm
    ) :
        _defaults(new DefaultOperators(algorithm)),
        _mutator(0x0),
        _converger(0x0),
        _crosser(0x0),
        _ftnsAssessor(0x0),
        _initializer(0x0),
        _selector(0x0),
        _postProcessor(0x0),
        _evaluator(0x0),
        _nicher(0x0),
        _mainLoop(0x0),
        _algorithm(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
    Clear();
}

GeneticAlgorithmOperatorSet::GeneticAlgorithmOperatorSet(
    const GeneticAlgorithmOperatorSet& copy
    ) :
        _defaults(new DefaultOperators(copy._algorithm)),
        _mutator(copy._mutator),
        _converger(copy._converger),
        _crosser(copy._crosser),
        _ftnsAssessor(copy._ftnsAssessor),
        _initializer(copy._initializer),
        _selector(copy._selector),
        _postProcessor(copy._postProcessor),
        _evaluator(copy._evaluator),
        _nicher(copy._nicher),
        _mainLoop(copy._mainLoop),
        _algorithm(copy._algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

GeneticAlgorithmOperatorSet::GeneticAlgorithmOperatorSet(
    const GeneticAlgorithmOperatorSet& copy,
    GeneticAlgorithm& algorithm
    ) :
        _defaults(new DefaultOperators(algorithm)),
        _mutator(0x0),
        _converger(0x0),
        _crosser(0x0),
        _ftnsAssessor(0x0),
        _initializer(0x0),
        _selector(0x0),
        _postProcessor(0x0),
        _evaluator(0x0),
        _nicher(0x0),
        _mainLoop(0x0),
        _algorithm(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
    this->operator =(copy.Clone(algorithm));
}

GeneticAlgorithmOperatorSet::~GeneticAlgorithmOperatorSet(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_defaults;
}







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
