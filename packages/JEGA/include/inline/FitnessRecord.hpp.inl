/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class FitnessRecord.

    NOTES:

        See notes of FitnessRecord.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Sep 15 10:11:58 2009 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the FitnessRecord class.
 */




/*
================================================================================
Includes
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
Inline Mutators
================================================================================
*/








/*
================================================================================
Inline Accessors
================================================================================
*/








/*
================================================================================
Inline Public Methods
================================================================================
*/

inline
void
FitnessRecord::SuspendStatistics(
    )
{
    this->_data.SuspendStatistics();
}

inline
void
FitnessRecord::ResumeStatistics(
    bool performUpdate
    )
{
    this->_data.ResumeStatistics(performUpdate);
}

inline
bool
FitnessRecord::UpdatingStatistics(
    )
{
    return this->_data.UpdatingStatistics();
}

inline
void
FitnessRecord::UpdateTotals(
    )
{
    this->_data.UpdateTotals();
}

inline
double
FitnessRecord::GetAverageFitness(
    ) const
{
    double avg = this->_data.GetAverageValue();
    return avg == JEGA::Utilities::DesignDoubleMap::MIN_POSSIBLE ?
                  JEGA::Utilities::DesignDoubleMap::MAX_POSSIBLE : avg;
}

inline
bool
FitnessRecord::AddFitness(
    const JEGA::Utilities::Design* des,
    double fitness
    )
{
    return this->_data.AddValue(des, fitness);
}

inline
double
FitnessRecord::GetFitness(
    const JEGA::Utilities::Design& des
    ) const
{
    // if the fitness comes back unknown, make it large negative so
    // that it keeps with the idea of lower fitness is worse.
    double ftns = this->_data.GetValue(des);
    return ftns == JEGA::Utilities::DesignDoubleMap::MAX_POSSIBLE ?
                   JEGA::Utilities::DesignDoubleMap::MIN_POSSIBLE : ftns;
}

inline
double
FitnessRecord::GetMaxFitness(
    ) const
{
    return this->_data.GetMaxValue();
}

inline
double
FitnessRecord::GetMinFitness(
    ) const
{
    return this->_data.GetMinValue();
}

inline
double
FitnessRecord::GetTotalFitness(
    ) const
{
    return this->_data.GetTotalValue();
}

inline
std::size_t
FitnessRecord::GetSize(
    ) const
{
    return this->_data.size();
}

inline
FitnessRecord::const_iterator
FitnessRecord::begin(
    ) const
{
    return this->_data.begin();
}

inline
FitnessRecord::const_iterator
FitnessRecord::end(
    ) const
{
    return this->_data.end();
}


/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/








/*
================================================================================
Inline Private Methods
================================================================================
*/








/*
================================================================================
Inline Structors
================================================================================
*/








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
