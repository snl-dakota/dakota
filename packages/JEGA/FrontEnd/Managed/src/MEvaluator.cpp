/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:

        Implementation of class MEvaluator.

    NOTES:

        See notes of MEvaluator.hpp.

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
 * \brief Contains the implementation of the MEvaluator class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <stdafx.h>
#include <vcclr.h>
#include <MDesign.hpp>
#include <MEvaluator.hpp>
#include <ManagedUtils.hpp>

#pragma unmanaged
#include <GeneticAlgorithmEvaluator.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../FrontEnd/Core/include/EvaluatorCreator.hpp>
#pragma managed





/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::FrontEnd;
using namespace JEGA::Utilities;
using namespace JEGA::Algorithms;
using namespace System::Collections;
using namespace System::Collections::Generic;



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
Nested Utility Class Implementations
================================================================================
*/

class BaseEvaluator :
    public GeneticAlgorithmEvaluator
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        gcroot<MEvaluator MOH> _managedEvaluator;


    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        virtual
        bool
        Evaluate(
            Design& des
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            bool ret = false;

            try {
                ret = this->_managedEvaluator->PerformEvaluation(des);

                // if PerformEvaluation returned badly, we mark the
                // design illconditioned.
                if(!ret) des.SetIllconditioned(true);
            }
            catch(...) {
                des.SetIllconditioned(true);
            }

            // Do base class evaluation finalization no matter what.
            // It knows enough to properly handle illconditioned designs.
            return this->GeneticAlgorithmEvaluator::PostEvaluate(des) && ret;
        }

        virtual
        bool
        Evaluate(
            JEGA::Utilities::DesignGroup& group
            )
        {
            EDDY_FUNC_DEBUGSCOPE

            bool ret = false;

            try {
                ret = this->_managedEvaluator->PerformEvaluation(group);

                // If the functor is a batch evaluator, then we have to do
                // post evaluation now b/c the overload will not have been
                // used.
                MEvaluationFunctor MOH del =
                    this->_managedEvaluator->GetEvaluationDelegate();

                if(del->IsBatchEvaluator())
                {
                    DesignDVSortSet::const_iterator e(group.EndDV());
                    for(DesignDVSortSet::const_iterator it(group.BeginDV());
                        it!=e; ++it) ret &= this->PostEvaluate(**it);
                }
            }
            catch(...) {
                ret = false;
            }

            return ret;
        }

        /// Returns the proper name of this operator.
        /**
         * \return The string "null_evaluation".
         */
        virtual
        string
        GetName(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return "managed_evaluator";
        }

        /// Returns a full description of what this operator does and how.
        /**
         * \verbatim
            This evaluator is a derivative of the
            GeneticAlgorithmEvaluator and uses managed extensions to
            hold and call-forward to a managed evaluator (MEvaluator)
            class instance's PerformEvaluation method.
           \endverbatim.
         *
         * \return A description of the operation of this operator.
         */
        virtual
        string
        GetDescription(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return "This evaluator is a derivative of the "
                "GeneticAlgorithmEvaluator and uses managed extensions to "
                "hold and call-forward to a managed evaluator (MEvaluator) "
                "class instance's PerformEvaluation method.";
        }

        /**
         * \brief Returns a new instance of this operator class for use by
         *        \a algorithm.
         *
         * \param algorithm The GA for which the new evaluator is to be used.
         * \return A new, default instance of a NullEvaluator.
         */
        virtual
        GeneticAlgorithmOperator*
        Clone(
            GeneticAlgorithm& algorithm
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return new BaseEvaluator(algorithm, this->_managedEvaluator);
        }

        virtual
        bool
        PostEvaluate(
            JEGA::Utilities::Design& des
            )
        {
            return this->GeneticAlgorithmEvaluator::PostEvaluate(des);
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        BaseEvaluator(
            GeneticAlgorithm& algorithm,
            gcroot<MEvaluator MOH> managedEvaluator
            ) :
                GeneticAlgorithmEvaluator(algorithm),
                _managedEvaluator(managedEvaluator)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

}; // class BaseEvaluator


class BaseEvaluatorCreator :
    public JEGA::FrontEnd::EvaluatorCreator
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        gcroot<MEvaluator MOH> _managedEvaluator;

        BaseEvaluator* _theEvaler;

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        BaseEvaluator*
        CurrentEvaluator(
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            return this->_theEvaler;
        }

    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:

        virtual
        JEGA::Algorithms::GeneticAlgorithmEvaluator*
        CreateEvaluator(
            JEGA::Algorithms::GeneticAlgorithm& algorithm
            )
        {
            EDDY_FUNC_DEBUGSCOPE
            if((this->_theEvaler == 0x0) ||
               (&this->_theEvaler->GetAlgorithm() != &algorithm))
            {
                BaseEvaluator* evaler = new BaseEvaluator(
                    algorithm, this->_managedEvaluator
                    );
                this->_managedEvaluator->SetEvaluator(evaler);
                return evaler;
            }
            return this->CurrentEvaluator();
        };

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        BaseEvaluatorCreator(
            gcroot<MEvaluator MOH> managedEvaluator
            ) :
                _managedEvaluator(managedEvaluator),
                _theEvaler(0x0)
        {
            EDDY_FUNC_DEBUGSCOPE
        }

}; // class BaseEvaluatorCreator


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

void
MEvaluator::SetEvaluationDelegate(
    MEvaluationFunctor MOH del
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_theDelegate = del;
}

void
MEvaluator::SetEvaluator(
    BaseEvaluator* evaler
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_evaler = evaler;
}





/*
================================================================================
Accessors
================================================================================
*/

MEvaluationFunctor MOH
MEvaluator::GetEvaluationDelegate(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theDelegate;
}






/*
================================================================================
Public Methods
================================================================================
*/

bool
MEvaluator::PerformEvaluation(
    Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE
    try
    {
        const bool needInjs = this->_theDelegate->MayInjectDesigns();

        List<MDesign MOH> MOH injections =
            needInjs ? MANAGED_GCNEW List<MDesign MOH>(10) : MANAGED_NULL_HANDLE;

        MDesign MOH toEval = MANAGED_GCNEW MDesign(&des);
        bool ret = this->_theDelegate->Evaluate(toEval, injections);

        if(needInjs) for each(MDesign MOH mDes in injections)
            this->InjectDesign(mDes);

        delete toEval;
        return ret;
    }
    catch(System::Exception MOH JEGA_LOGGING_IF_ON(ex))
    {
        JEGALOG_G(lquiet(),
            text_entry(lverbose(),
                "Managed Evaluator caught an exception thrown by the "
                "evaluation functor reading \"" + ToStdStr(ex->Message) +
                "\".  The design being evaluated will be considered "
                "illconditioned."
                )
            )

        return false;
    }
}

bool
MEvaluator::PerformEvaluation(
    JEGA::Utilities::DesignGroup& toEval
    )
{
    EDDY_FUNC_DEBUGSCOPE
    try
    {
        if(toEval.SizeDV() > 0 && this->_theDelegate->IsBatchEvaluator())
        {
            // Create a design vector from the designs in "toEval" and send
            // them off for evaluation.
            DesignVector MOH mdv = MANAGED_GCNEW DesignVector(
				static_cast<int>(toEval.SizeDV())
				);
            for(DesignDVSortSet::const_iterator it=toEval.BeginDV();
                it!=toEval.EndDV(); ++it) mdv->Add(MANAGED_GCNEW MDesign(*it));

            const bool needInjs = this->_theDelegate->MayInjectDesigns();

            List<MDesign MOH> MOH injections =
                needInjs ? MANAGED_GCNEW List<MDesign MOH>(10) : MANAGED_NULL_HANDLE;

            bool ret = this->_theDelegate->Evaluate(mdv, injections);

            if(needInjs) for each(MDesign MOH mDes in injections)
                this->InjectDesign(mDes);

            for(int i=0; i<mdv->Count; ++i) delete MANAGED_LIST_ITEM(mdv, i);
            delete mdv;

            return ret;
        }
        else
        {
            return this->_evaler->GeneticAlgorithmEvaluator::Evaluate(toEval);
        }
    }
    catch(System::Exception MOH JEGA_LOGGING_IF_ON(ex))
    {
        JEGALOG_G(lquiet(),
            text_entry(lverbose(),
                "Managed Evaluator caught an exception thrown by the "
                "evaluation functor reading \"" + ToStdStr(ex->Message) +
                "\".  The designs being evaluated will be considered "
                "illconditioned."
                )
            )

        return false;
    }
}

EvaluatorCreator&
MEvaluator::GetTheEvaluatorCreator(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return *this->_theEvalCreator;
}

void
MEvaluator::InjectDesign(
    MDesign MOH mDes
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA::Utilities::Design& des = mDes->Manifest();
    this->_evaler->PostEvaluate(des);
    //const DesignTarget& target = des.GetDesignTarget();
    //target.CheckFeasibility(des);
    //des.SetEvaluated(true);
    this->_evaler->InjectDesign(des);
}

MDesign MOH
MEvaluator::InjectDesign(
    DoubleVector MOH X,
    DoubleVector MOH F,
    DoubleVector MOH G
    )
{
    EDDY_FUNC_DEBUGSCOPE
    const GeneticAlgorithmEvaluator& cevaler = *this->_evaler;
    const DesignTarget& target = cevaler.GetDesignTarget();
    Design* des = target.GetNewDesign();
    MDesign MOH ret = MANAGED_GCNEW MDesign(des);
    const size_t xct = static_cast<std::size_t>(X->Count);
    const size_t fct = static_cast<std::size_t>(F->Count);
    const size_t gct = static_cast<std::size_t>(G->Count);

    if(xct != target.GetNDV()) throw MANAGED_GCNEW System::Exception(
        "Injection design has incorrect number of design variables (" +
        xct.ToString() + "). expected " + target.GetNDV().ToString() + '.'
        );

    if(fct != target.GetNOF()) throw MANAGED_GCNEW System::Exception(
        "Injection design has incorrect number of objective functions (" +
        fct.ToString() + "). expected " + target.GetNOF().ToString() + '.'
        );

    if(gct != target.GetNCN()) throw MANAGED_GCNEW System::Exception(
        "Injection design has incorrect number of constraints (" +
        gct.ToString() + "). expected " + target.GetNCN().ToString() + '.'
        );

    for(size_t i=0; i<xct; ++i)
        des->SetVariableValue(i, MANAGED_LIST_ITEM(X, static_cast<int>(i)));

    for(size_t i=0; i<fct; ++i)
        des->SetObjective(i, MANAGED_LIST_ITEM(F, static_cast<int>(i)));

    for(size_t i=0; i<gct; ++i)
        des->SetConstraint(i, MANAGED_LIST_ITEM(G, static_cast<int>(i)));

    this->_evaler->PostEvaluate(*des);
    //target.CheckFeasibility(*des);
    //des->SetEvaluated(true);
    this->_evaler->InjectDesign(*des);

    return ret;
}






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
void
MEvaluator::MANAGED_DISPOSE(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_theEvalCreator;
    this->_theEvalCreator = 0x0;
    this->_evaler = 0x0;
}




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

MEvaluator::MEvaluator(
    ) :
        _theDelegate(MANAGED_NULL_HANDLE),
        _theEvalCreator(MANAGED_NULL_HANDLE),
        _evaler(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_theEvalCreator = new BaseEvaluatorCreator(this);
}


MEvaluator::MEvaluator(
    MEvaluationFunctor MOH theDelegate
    ) :
        _theDelegate(theDelegate),
        _theEvalCreator(MANAGED_NULL_HANDLE),
        _evaler(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_theEvalCreator = new BaseEvaluatorCreator(this);
}
//
//MEvaluator::MEvaluator(
//    JEGA::Algorithms::GeneticAlgorithmEvaluator* evaler
//    ) :
//        _theDelegate(MANAGED_NULL_HANDLE),
//        _theEvalCreator(MANAGED_NULL_HANDLE),
//        _evaler(evaler)
//{
//    EDDY_FUNC_DEBUGSCOPE
//    this->_theEvalCreator = new BaseEvaluatorCreator(this);
//}
//
//
//MEvaluator::MEvaluator(
//    MEvaluationFunctor MOH theDelegate,
//    JEGA::Algorithms::GeneticAlgorithmEvaluator* evaler
//    ) :
//        _theDelegate(theDelegate),
//        _theEvalCreator(MANAGED_NULL_HANDLE),
//        _evaler(evaler)
//{
//    EDDY_FUNC_DEBUGSCOPE
//    this->_theEvalCreator = new BaseEvaluatorCreator(this);
//}

MEvaluator::~MEvaluator(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->MANAGED_DISPOSE();
}







/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA


