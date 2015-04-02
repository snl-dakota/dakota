/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class SimpleFunctorEvaluator.

    NOTES:

        See notes of SimpleFunctorEvaluator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Mon Jul 17 14:57:43 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the SimpleFunctorEvaluator class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <Evaluators/SimpleFunctorEvaluator.hpp>

#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>






/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
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
Static Member Data Definitions
================================================================================
*/








/*
================================================================================
Mutators
================================================================================
*/
void
SimpleFunctorEvaluator::SetEvaluationFunctor(
    Functor* theFunc
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_theFunctor = theFunc;
}




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

const string&
SimpleFunctorEvaluator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("simple_functor");
    return ret;
}

const string&
SimpleFunctorEvaluator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        ""
        );
    return ret;
}

GeneticAlgorithmOperator*
SimpleFunctorEvaluator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new SimpleFunctorEvaluator(algorithm);
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

string
SimpleFunctorEvaluator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return SimpleFunctorEvaluator::Name();
}

string
SimpleFunctorEvaluator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return SimpleFunctorEvaluator::Description();
}

GeneticAlgorithmOperator*
SimpleFunctorEvaluator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new SimpleFunctorEvaluator(*this, algorithm);
}

bool
SimpleFunctorEvaluator::Evaluate(
    Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_theFunctor != 0x0)

    JEGAIFLOG_CF_II_F(this->_theFunctor == 0x0, this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": Evaluation attempted prior "
            "to setting of the evaluation functor."
            )
        )

    const DesignTarget& target = this->GetDesignTarget();

    const size_t ndv = target.GetNDV();
    const size_t nof = target.GetNOF();
    const size_t ncn = target.GetNCN();

    DoubleVector x(ndv);
    DoubleVector f(nof, 0.0);
    DoubleVector g(ncn, 0.0);

    for(size_t i=0; i<ndv; ++i) x[i] = des.GetVariableValue(i);
    for(size_t i=0; i<nof; ++i) f[i] = des.GetObjective(i);
    for(size_t i=0; i<ncn; ++i) g[i] = des.GetConstraint(i);

    try
    {
        if(!this->_theFunctor->Evaluate(x, f, g)) des.SetIllconditioned(true);

        else
        {
            // make sure that the user hasn't resized f or g.
            JEGAIFLOG_CF_II_F(f.size() != nof, this->GetLogger(), this,
                ostream_entry(lfatal(), this->GetName() + ": Evaluation "
                    "functor has changed the size of the objective function "
                    "vector from ") << nof << " to " << f.size() << ".  This "
                    "is an illegal operation. Please modify your code."
                )

            JEGAIFLOG_CF_II_F(g.size() != ncn, this->GetLogger(), this,
                ostream_entry(lfatal(), this->GetName() + ": Evaluation "
                    "functor has changed the size of the constraint function "
                    "vector from ") << ncn << " to " << g.size() << ".  This "
                    "is an illegal operation. Please modify your code."
                )

            for(size_t i=0; i<nof; ++i) des.SetObjective (i, f[i]);
            for(size_t i=0; i<ncn; ++i) des.SetConstraint(i, g[i]);
        }
    }
    catch(...)
    {
        des.SetIllconditioned(true);
    }

    return this->GeneticAlgorithmEvaluator::PostEvaluate(des);
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
SimpleFunctorEvaluator::SimpleFunctorEvaluator(
    GeneticAlgorithm& algorithm,
    Functor* theFunc
    ) :
        GeneticAlgorithmEvaluator(algorithm),
        _theFunctor(theFunc)
{
    EDDY_FUNC_DEBUGSCOPE
}

SimpleFunctorEvaluator::SimpleFunctorEvaluator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmEvaluator(algorithm),
        _theFunctor(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
}

SimpleFunctorEvaluator::SimpleFunctorEvaluator(
    const SimpleFunctorEvaluator& copy
    ) :
        GeneticAlgorithmEvaluator(copy),
        _theFunctor(copy._theFunctor)
{
    EDDY_FUNC_DEBUGSCOPE
}

SimpleFunctorEvaluator::SimpleFunctorEvaluator(
    const SimpleFunctorEvaluator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmEvaluator(copy, algorithm),
        _theFunctor(copy._theFunctor)
{
    EDDY_FUNC_DEBUGSCOPE
}







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA

