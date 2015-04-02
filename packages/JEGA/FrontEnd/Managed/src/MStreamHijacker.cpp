/*
================================================================================
    PROJECT:

        CoreSim

    CONTENTS:

        Implementation of managed stream hijacking classes.

    NOTES:

        See notes of MDelegating.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Jan 30 11:51:21 2007 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of managed stream hijacking classes.
 */




/*
================================================================================
Includes
================================================================================
*/
#include <stdafx.h>
#include <MStreamHijacker.hpp>

#pragma unmanaged
#include <utilities/include/EDDY_DebugScope.hpp>
#pragma managed



/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {
        namespace Managed {




namespace detail
{
    class StreamHijackerImpl : public StreamHijacker
    {
        private:

            gcroot<MStreamHijacker MOH> _managed;

        public:

            virtual
            int
            sync(
                )
            {
                int ret = this->StreamHijacker::sync();
                this->_managed->on_hijacked_sync();
                return ret;
            }

            StreamHijackerImpl(
                gcroot<MStreamHijacker MOH> managed,
                std::ostream& toHijack,
                gcroot<System::IO::TextWriter MOH> hijackStr
                ) :
                    StreamHijacker(toHijack, hijackStr),
                    _managed(managed)
            {
            }
    };
}





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

void
MStreamHijacker::detach(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_guts->detach();
}

void
MStreamHijacker::reattach(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_guts->reattach();
}

System::IO::TextWriter MOH
MStreamHijacker::text_writer(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_guts->text_writer();
}

void
MStreamHijacker::on_hijacked_sync(
    )
{
    EDDY_FUNC_DEBUGSCOPE
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

int
StreamHijacker::overflow(
    int c
    )
{
    EDDY_FUNC_DEBUGSCOPE
    wchar_t cc = traits_type::to_char_type(c);
    if (c != EOF) this->_out->Write(cc);
    return traits_type::not_eof(c);
}

int
StreamHijacker::sync(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->streambuf::sync();
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

StreamHijacker::StreamHijacker(
    std::ostream& toHijack,
    gcroot<System::IO::TextWriter MOH> hijackStr
    ) :
        _hijacked(toHijack),
        _out(hijackStr),
        _prevBuf(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->reattach();
}

StreamHijacker::~StreamHijacker(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->detach();
}

MStreamHijacker::MStreamHijacker(
    std::ostream& toHijack,
    System::IO::TextWriter MOH hijackStr
    ) :
        _guts(0x0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->_guts = new detail::StreamHijackerImpl(this, toHijack, hijackStr);
}

MStreamHijacker::~MStreamHijacker(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_guts;
}


MCoutHijacker::MCoutHijacker(
    System::IO::TextWriter MOH hijackStr
    ) :
        MStreamHijacker(std::cout, hijackStr)
{
    EDDY_FUNC_DEBUGSCOPE
}

MCerrHijacker::MCerrHijacker(
    System::IO::TextWriter MOH hijackStr
    ) :
        MStreamHijacker(std::cerr, hijackStr)
{
    EDDY_FUNC_DEBUGSCOPE
}

/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace Managed
    } // namespace FrontEnd
} // namespace JEGA

