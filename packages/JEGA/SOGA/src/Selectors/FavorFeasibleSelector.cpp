/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implemenation of class FavorFeasibleSelector.

    NOTES:

        See notes of FavorFeasibleSelector.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Wed Jul 23 10:32:44 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the FavorFeasibleSelector class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <FitnessRecord.hpp>
#include <GeneticAlgorithm.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <../Utilities/include/DesignStatistician.hpp>
#include <../SOGA/include/Selectors/FavorFeasibleSelector.hpp>
#include <../Utilities/include/SingleObjectiveStatistician.hpp>



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
Nested Inner Class Implementations
================================================================================
*/
class FavorFeasibleSelector::FeasibilityPred :
    public GeneticAlgorithmSelector::FitnessPred
{
    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// One of the operators that activates this predicate
        /**
         * \param des1 The first design to compare by fitness value.
         * \param des2 The second design to compare by fitness value.
         * \return True if the fitness value for \a des1 is greater than that
         *         for \a des2 and false otherwise.
         */
        bool
        operator ()(
            const Design* des1,
            const Design* des2
            ) const
        {
            EDDY_FUNC_DEBUGSCOPE
            EDDY_ASSERT(des1 != 0x0);
            EDDY_ASSERT(des2 != 0x0);
            EDDY_ASSERT(&des1->GetDesignTarget() == &des2->GetDesignTarget());

            // First sort on violation.  If there is a tie there, go to fitness.
            double d1Viol = des1->IsFeasible() ? 0.0 :
                DesignStatistician::ComputeTotalPercentageViolation(*des1);
            double d2Viol = des2->IsFeasible() ? 0.0 :
                DesignStatistician::ComputeTotalPercentageViolation(*des2);

            return d1Viol == d2Viol ?
                this->FitnessPred::operator()(des1, des2) : d1Viol < d2Viol;
        }

        inline
        bool
        operator ()(
            const DesignGroupInfo& d1Info,
            const DesignGroupInfo& d2Info
            ) const
        {
            return this->operator ()(*d1Info.second, *d2Info.second);
        }

    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a FitnessPred to sort according to the supplied
         *        fitnesses.
         *
         * \param ftns The record of fitnesses from which to get the fitness
         *             values of the designs to compare.
         */
        inline
        FeasibilityPred(
            const FitnessRecord& ftns
            ) :
                FitnessPred(ftns)
        {}


}; // class FeasibilityPred

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
FavorFeasibleSelector::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("favor_feasible");
    return ret;
}

const string&
FavorFeasibleSelector::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret;
    return ret;
}

GeneticAlgorithmOperator*
FavorFeasibleSelector::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new FavorFeasibleSelector(algorithm);
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
FavorFeasibleSelector::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return FavorFeasibleSelector::Name();
}

string
FavorFeasibleSelector::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return FavorFeasibleSelector::Description();
}

GeneticAlgorithmOperator*
FavorFeasibleSelector::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new FavorFeasibleSelector(*this, algorithm);
}

void
FavorFeasibleSelector::Select(
    DesignGroupVector& from,
    DesignGroup& into,
    size_t count,
    const FitnessRecord& ftns
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing selection.")
        )

    this->GeneticAlgorithmSelector::SelectNBest<FeasibilityPred>(
        from, into, count, ftns, FeasibilityPred(ftns)
        );

    // If we make it here, we are done no matter what.  Whether or not we
    // have made enough selections, there are no more to make.  Any designs
    // left in the from groups have to be taken care of by the algorithm.
}

DesignOFSortSet
FavorFeasibleSelector::SelectNBest(
    DesignGroupVector& from,
    size_t n,
    const FitnessRecord& ftns
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        ostream_entry(ldebug(), this->GetName() + ": identifying the ") <<
        n << " best designs."
        )

    return this->GeneticAlgorithmSelector::SelectNBest<FeasibilityPred>(
        from, n, FeasibilityPred(ftns)
        );
}

bool
FavorFeasibleSelector::CanSelectSameDesignMoreThanOnce(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
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
FavorFeasibleSelector::FavorFeasibleSelector(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(algorithm)
{
    EDDY_FUNC_DEBUGSCOPE
}

FavorFeasibleSelector::FavorFeasibleSelector(
    const FavorFeasibleSelector& copy
    ) :
        GeneticAlgorithmSelector(copy)
{
    EDDY_FUNC_DEBUGSCOPE
}

FavorFeasibleSelector::FavorFeasibleSelector(
    const FavorFeasibleSelector& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmSelector(copy, algorithm)
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
