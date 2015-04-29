/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class GeneticAlgorithm.

    NOTES:

        See notes of GeneticAlgorithm.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 15 08:25:23 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the inline methods of the GeneticAlgorithm class.
 */



/*
================================================================================
Includes
================================================================================
*/
#include <utilities/include/EDDY_DebugScope.hpp>




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
inline
GeneticAlgorithmOperatorSet&
GeneticAlgorithm::GetOperatorSet(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_opSet != 0x0);
    return *this->_opSet;
}

inline
const GeneticAlgorithmOperatorSet&
GeneticAlgorithm::GetOperatorSet(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(this->_opSet != 0x0);
    return *this->_opSet;
}

inline
eddy::utilities::uint64_t
GeneticAlgorithm::GetInstanceNumber(
    ) const
{
    return this->_instanceNum;
}

inline
JEGA::Utilities::DesignTarget&
GeneticAlgorithm::GetDesignTarget(
    )
{
    return this->_target;
}

inline
const JEGA::Utilities::DesignTarget&
GeneticAlgorithm::GetDesignTarget(
    ) const
{
    return this->_target;
}

inline
JEGA::Utilities::DesignGroup&
GeneticAlgorithm::GetPopulation(
    )
{
    return this->_pop;
}

inline
const JEGA::Utilities::DesignGroup&
GeneticAlgorithm::GetPopulation(
    ) const
{
    return this->_pop;
}

inline
JEGA::Utilities::DesignGroup&
GeneticAlgorithm::GetOffspring(
    )
{
    return this->_cldrn;
}

inline
const JEGA::Utilities::DesignGroup&
GeneticAlgorithm::GetOffspring(
    ) const
{
    return this->_cldrn;
}

inline
const std::size_t&
GeneticAlgorithm::GetStartTime(
    ) const
{
    return this->_startTime;
}

inline
bool
GeneticAlgorithm::IsWritingPopEachGen(
    ) const
{
    return this->_printPopEachGen;
}

inline
bool
GeneticAlgorithm::IsWritingFinalData(
    ) const
{
    return this->_printFinalData;
}

inline
bool
GeneticAlgorithm::IsWritingDiscards(
    ) const
{
    return this->_printDiscards;
}




/*
================================================================================
Inline Public Methods
================================================================================
*/
inline
const std::string&
GeneticAlgorithm::GetName(
    ) const
{
   return this->_name;
}

inline
const std::string&
GeneticAlgorithm::GetFinalDataFilename(
    ) const
{
    return this->_finalDataFile;
}

inline
const std::string&
GeneticAlgorithm::GetDataDirectory(
    ) const
{
    return this->_dataDir;
}

inline
const GeneticAlgorithmMutator&
GeneticAlgorithm::GetMutator(
    ) const
{
    return this->GetOperatorSet().GetMutator();
}

inline
const GeneticAlgorithmConverger&
GeneticAlgorithm::GetConverger(
    ) const
{
    return this->GetOperatorSet().GetConverger();
}

inline
const GeneticAlgorithmCrosser&
GeneticAlgorithm::GetCrosser(
    ) const
{
    return this->GetOperatorSet().GetCrosser();
}

inline
const GeneticAlgorithmFitnessAssessor&
GeneticAlgorithm::GetFitnessAssessor(
    ) const
{
    return this->GetOperatorSet().GetFitnessAssessor();
}

inline
const GeneticAlgorithmInitializer&
GeneticAlgorithm::GetInitializer(
    ) const
{
    return this->GetOperatorSet().GetInitializer();
}

inline
const GeneticAlgorithmSelector&
GeneticAlgorithm::GetSelector(
    ) const
{
    return this->GetOperatorSet().GetSelector();
}

inline
const GeneticAlgorithmEvaluator&
GeneticAlgorithm::GetEvaluator(
    ) const
{
    return this->GetOperatorSet().GetEvaluator();
}

inline
const GeneticAlgorithmNichePressureApplicator&
GeneticAlgorithm::GetNichePressureApplicator(
    ) const
{
    return this->GetOperatorSet().GetNichePressureApplicator();
}

inline
const GeneticAlgorithmMainLoop&
GeneticAlgorithm::GetMainLoop(
    ) const
{
    return this->GetOperatorSet().GetMainLoop();
}

inline
const GeneticAlgorithmOperatorGroup&
GeneticAlgorithm::GetOperatorGroup(
    ) const
{
    return *this->_opGroup;
}

inline
const JEGA::Utilities::RegionOfSpace&
GeneticAlgorithm::GetMyRegion(
    ) const
{
    return this->_myDesignSpace;
}

inline
bool
GeneticAlgorithm::IsFinalized(
    ) const
{
    return this->_isFinalized;
}

inline
bool
GeneticAlgorithm::IsInitialized(
    ) const
{
    return this->_isInitialized;
}




/*
================================================================================
Inline Subclass Visible Methods
================================================================================
*/
inline
JEGA::Logging::Logger&
GeneticAlgorithm::GetLogger(
    ) const
{
    return this->_log;
}

inline
GeneticAlgorithmMutator&
GeneticAlgorithm::GetMutator(
    )
{
    return this->GetOperatorSet().GetMutator();
}

inline
GeneticAlgorithmConverger&
GeneticAlgorithm::GetConverger(
    )
{
    return this->GetOperatorSet().GetConverger();
}

inline
GeneticAlgorithmCrosser&
GeneticAlgorithm::GetCrosser(
    )
{
    return this->GetOperatorSet().GetCrosser();
}

inline
GeneticAlgorithmFitnessAssessor&
GeneticAlgorithm::GetFitnessAssessor(
    )
{
    return this->GetOperatorSet().GetFitnessAssessor();
}

inline
GeneticAlgorithmInitializer&
GeneticAlgorithm::GetInitializer(
    )
{
    return this->GetOperatorSet().GetInitializer();
}

inline
GeneticAlgorithmSelector&
GeneticAlgorithm::GetSelector(
    )
{
    return this->GetOperatorSet().GetSelector();
}

inline
GeneticAlgorithmPostProcessor&
GeneticAlgorithm::GetPostProcessor(
    )
{
    return this->GetOperatorSet().GetPostProcessor();
}

inline
GeneticAlgorithmEvaluator&
GeneticAlgorithm::GetEvaluator(
    )
{
    return this->GetOperatorSet().GetEvaluator();
}

inline
GeneticAlgorithmMainLoop&
GeneticAlgorithm::GetMainLoop(
    )
{
    return this->GetOperatorSet().GetMainLoop();
}

inline
GeneticAlgorithmNichePressureApplicator&
GeneticAlgorithm::GetNichePressureApplicator(
    )
{
    return this->GetOperatorSet().GetNichePressureApplicator();
}








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
