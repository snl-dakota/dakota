/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class Design.

    NOTES:

        See notes of Design.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 15 11:04:05 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the Design class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <algorithm>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>




/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;




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
std::size_t Design::NEXT_ID(0);

JEGA_IF_THREADSAFE(eddy::threads::mutex Design::ID_MUTEX(PTHREAD_MUTEX_NORMAL);)

void (*Design::DISP_CBK)(Design*) = 0x0;


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
const Design&
Design::operator = (
    const Design& right
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(&right == this) return *this;

    // The targets must be the same for two Designs to equate
    EDDY_ASSERT(&this->_target == &right._target);

    // Allocation should already have taken place so
    // equate this to rhs.

    // equate the variables and responses.
    ::memcpy(
        this->_variables, right._variables,
        this->GetNDV() * sizeof(double)
        );
    ::memcpy(
        this->_objectives, right._objectives,
        this->GetNOF() * sizeof(double)
        );
    ::memcpy(
        this->_constraints, right._constraints,
        this->GetNCN() * sizeof(double)
        );

    // equate the attributes.
    this->_attributes = right._attributes;

    // Tag the two Designs as clones.
    Design::TagAsClones(*this, const_cast<Design&>(right));

    return *this;
}

eddy::utilities::uint8_t
Design::TagAsClones(
    Design& des1,
    Design& des2
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(&des1 != &des2);

    // Check for trivial abort conditions.
    if(&des1 == &des2) return 0;

    // These pointers will be used to iterate the clone lists.
    Design* pdes1 = &des1;
    Design* pdes2 = &des2;

    if(des1.IsCloned())
    {
        if(des2.IsCloned())
        {
            // both are clones already, check to see if they are already known
            // to be clones of one another.  If not, tag'em.
            if(!des1.HasInCloneList(des2))
            {
                // find beginning of des1's list
                for(; pdes1->_pClone!=0x0; pdes1=pdes1->_pClone);
                // find end of des2's list
                for(; pdes2->_nClone!=0x0; pdes2=pdes2->_nClone);
                //  merge the lists.
                pdes2->_nClone = pdes1;
                pdes1->_pClone = pdes2;
            }

            return 0;
        }
        else // if(!des2.IsCloned())
        {
            // find the end of des1's clone list.
            for(; pdes1->_nClone!=0x0; pdes1=pdes1->_nClone);
            // put des2 at the end.
            pdes1->_nClone = pdes2;
            pdes2->_pClone = pdes1;
            return 1;
        }
    }

    if(des2.IsCloned())
    {
        // find the end of des2's list.
        for(; pdes2->_nClone!=0x0; pdes2=pdes2->_nClone);
        // put des1 at the end.
        pdes2->_nClone = pdes1;
        pdes1->_pClone = pdes2;
        return 1;
    }

    // neither of these guys were clones before, start a new list.
    des1._nClone = pdes2;
    des2._pClone = pdes1;
    return 2;
}

void
Design::RemoveAsClone(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // typical linked list removal.
    if(this->_pClone != 0x0)
    {
        if(this->_nClone != 0x0) this->_nClone->_pClone = this->_pClone;
        this->_pClone->_nClone = this->_nClone;
    }
    else if(this->_nClone != 0x0) this->_nClone->_pClone = 0x0;

    this->_pClone = this->_nClone = 0x0;
}

bool
Design::HasInCloneList(
    const Design& des
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    for(const Design* cdes = this->_pClone; cdes!=0x0; cdes=cdes->_pClone)
        if(cdes == &des) return true;

    for(const Design* cdes = this->_nClone; cdes!=0x0; cdes=cdes->_nClone)
        if(cdes == &des) return true;

    return false;
}

eddy::utilities::uint64_t
Design::CountClones(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    eddy::utilities::uint64_t ret = 0;

    // count the nclones.
    for(const Design* clone=this->_nClone; clone!=0x0; clone=clone->_nClone)
        ++ret;

    // now the pclones
    for(const Design* clone=this->_pClone; clone!=0x0; clone=clone->_pClone)
        ++ret;

    return ret;
}

void
Design::CopyResponses(
    const Design& from
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(&from.GetDesignTarget() == &this->GetDesignTarget())

    if(&from == this) return;

    if(this->GetNOF() > 0) ::memcpy(
        this->_objectives, from._objectives,
        this->GetNOF() * sizeof(double)
        );

    if(this->GetNCN() > 0) ::memcpy(
        this->_constraints, from._constraints,
        this->GetNCN() * sizeof(double)
        );
}

void
Design::Dispose(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(DISP_CBK != 0x0) (*DISP_CBK)(this);

    this->_tag = 0x0;
    this->_attributes.clear();
    this->RemoveAsClone();

    EDDY_DEBUGEXEC(
        ::memset(this->_objectives, 0, this->GetNOF() * sizeof(double));
        )
    EDDY_DEBUGEXEC(
        ::memset(this->_variables, 0, this->GetNDV() * sizeof(double));
        )
    EDDY_DEBUGEXEC(
        ::memset(this->_constraints, 0, this->GetNCN() * sizeof(double));
        )
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








/*
================================================================================
Private Methods
================================================================================
*/


void
Design::DoAllocation(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(this->GetNDV() > 0) this->_variables = new double[this->GetNDV()];
    if(this->GetNOF() > 0) this->_objectives = new double[this->GetNOF()];
    if(this->GetNCN() > 0) this->_constraints = new double[this->GetNCN()];
}


/*
================================================================================
Structors
================================================================================
*/
Design::Design(
    DesignTarget& target
    ) :
        _id(NextID()),
        _variables(0x0),
        _objectives(0x0),
        _constraints(0x0),
        _attributes(0),
        _target(target),
        _nClone(0x0),
        _pClone(0x0),
        _tag(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->DoAllocation();
}

Design::Design(
    const Design& copy
    ) :
        _id(NextID()),
        _variables(0x0),
        _objectives(0x0),
        _constraints(0x0),
        _attributes(0),
        _target(copy._target),
        _nClone(0x0),
        _pClone(0x0),
        _tag(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->DoAllocation();
    this->operator =(copy);
}

Design::~Design(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(DISP_CBK != 0x0) (*DISP_CBK)(this);

    this->RemoveAsClone();
    delete [] this->_variables;
    delete [] this->_objectives;
    delete [] this->_constraints;

    this->_variables = 0x0;
    this->_objectives = 0x0;
    this->_constraints = 0x0;
    this->_tag = 0x0;
}







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
