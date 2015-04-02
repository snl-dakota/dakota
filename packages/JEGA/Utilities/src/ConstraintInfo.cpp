/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ConstraintInfo.

    NOTES:

        See notes of ConstraintInfo.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Mon Jun 09 16:16:24 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ConstraintInfo class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/Math.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/ConstraintTypeBase.hpp>
#include <../Utilities/include/ConstraintNatureBase.hpp>
#include <../Utilities/include/InequalityConstraintType.hpp>

#ifdef JEGA_THREADSAFE
#include <threads/include/mutex_lock.hpp>
#endif




/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace eddy::utilities;






/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {








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
ConstraintInfo::SetType(
   ConstraintTypeBase* type
   )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_type;
    this->_type = (type == 0x0) ? new InequalityConstraintType(*this) : type;

} // ConstraintInfo::SetType





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
double
ConstraintInfo::GetViolationPercentage(
   double of
   ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // This will be computed using the violation of "of" and the average
    // violation value.
    const double viol = Math::Abs(this->GetViolationAmount(of));

    // A simple case is if viol is 0.0 in which case the return is an obvious
    // 0% violation.
    if(viol == 0.0) return 0.0;

    JEGA_IF_THREADSAFE(eddy::threads::mutex_lock l(this->_violMutex));

    // Now the only exception is if there are no violations yet recorded.
    // If that is the case, we will return 100%.
    if(this->_numViolations == 0) return 100.0;

    // now get the average violation amount thus far recorded.
    const double avg = this->GetAverageViolation();

    EDDY_ASSERT(avg != 0.0);

    // we will now compute a percentage based on viol and avg.
    return viol/avg * 100.0;

} // ConstraintInfo::GetViolationPercentage

const Design*
ConstraintInfo::GetPreferredDesign(
    const Design& des1,
    const Design& des2
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    double val1 = this->GetViolationAmount(des1);
    double val2 = this->GetViolationAmount(des2);
    return (val1 < val2 ) ? &des1 : (val1 > val2 ) ? &des2 : 0;

} // ConstraintInfo::GetPreferredDesign

void
ConstraintInfo::RecordViolation(
    double rawValue
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // start by getting the magnitude of the violation of the raw value.
    const double viol = Math::Abs(this->GetViolationAmount(rawValue));

    // now, if that violation is > 0, use it.  Otherwise, ignore it.
    if(viol > 0.0)
    {
        JEGA_IF_THREADSAFE(eddy::threads::mutex_lock l(this->_violMutex));

        ++this->_numViolations;
        this->_totalViolation += viol;

        JEGALOG_II_G(ldebug(), this,
            ostream_entry(
                ldebug(), this->GetLabel() + ": recorded violation value of "
                ) << viol << " from raw value " << rawValue
                  << ". Average violation is now "
                  << this->GetAverageViolation()
            )

    }

} // ConstraintInfo::RecordViolation

double
ConstraintInfo::GetAverageViolation(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_THREADSAFE(eddy::threads::mutex_lock l(this->_violMutex));
    return (this->_numViolations == 0) ?
        0.0 : (this->_totalViolation / this->_numViolations);

} // ConstraintInfo::GetAverageViolation



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

ConstraintInfo::ConstraintInfo(
    DesignTarget& target
    ) :
        InfoBase(target),
        _type(0x0),
        _totalViolation(0.0),
        _numViolations(0) JEGA_COMMA_IF_THREADSAFE
        JEGA_IF_THREADSAFE(_violMutex(PTHREAD_MUTEX_RECURSIVE))
{
    EDDY_FUNC_DEBUGSCOPE
    this->_type = new InequalityConstraintType(*this);

} // ConstraintInfo::ConstraintInfo

ConstraintInfo::ConstraintInfo(
    const ConstraintInfo& copy,
    DesignTarget& target
    ) :
        InfoBase(copy, target),
        _type(0x0),
        _totalViolation(copy._totalViolation),
        _numViolations(copy._numViolations) JEGA_COMMA_IF_THREADSAFE
        JEGA_IF_THREADSAFE(_violMutex(PTHREAD_MUTEX_RECURSIVE))
{
    EDDY_FUNC_DEBUGSCOPE
    this->_type = copy.GetType().Clone(*this);

} // ConstraintInfo::ConstraintInfo

ConstraintInfo::ConstraintInfo(
    const ConstraintInfo& copy
    ) :
        InfoBase(copy),
        _type(0x0),
        _totalViolation(copy._totalViolation),
        _numViolations(copy._numViolations) JEGA_COMMA_IF_THREADSAFE
        JEGA_IF_THREADSAFE(_violMutex(PTHREAD_MUTEX_RECURSIVE))
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(copy._type != 0x0);

    this->_type = copy.GetType().Clone(*this);

} // ConstraintInfo::ConstraintInfo

ConstraintInfo::~ConstraintInfo(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_type;
}








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
