/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class MessageBoard.

    NOTES:

        See notes of MessageBoard.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.7.0

    CHANGES:

        Mon Jan 30 09:57:28 2012 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the MessageBoard class.
 */




/*
================================================================================
Includes
================================================================================
*/
#include "../include/MessageBoard.hpp"

#ifdef JEGA_MESSAGE_BOARD

#include <../Utilities/include/JEGAConfig.hpp>

#include <algorithm>
#include <functional>
#include <utilities/include/EDDY_DebugScope.hpp>






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
    namespace Utilities {





/*
================================================================================
Static Member Data Definitions
================================================================================
*/
size_t MessageBoard::Subscription::NEXT_ID = 0;



/*
================================================================================
Public Methods
================================================================================
*/

const MessageBoard::MessageIdentifier&
MessageBoard::MessageIdentifier::operator =(
    const MessageIdentifier& other
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(this == &other) return other;
    this->_sender = other._sender;
    this->_type = other._type;
    this->_tag = other._tag;
    return *this;
}

const MessageBoard::Subscription&
MessageBoard::Subscription::operator =(
    const Subscription& other
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(this == &other) return other;
    this->_id = other._id;
    this->_pred = other._pred;
    this->_callback = other._callback;
    return *this;
}

void
MessageBoard::PostMessage(
    const Message& msg
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_IF_THREADSAFE(SubscriptionVector::scoped_lock l(SVEC()));
    for(
        SubscriptionVector::const_iterator it(SVEC().begin()); it!=SVEC().end();
        ++it
        ) (*it).Service(msg);
}

MessageBoard::Subscription
MessageBoard::Subscribe(
    const MessagePredicate& pred,
    const SubscriberCallback& callback
    )
{
    Subscription subscr(pred, callback);
    SVEC().push_back(subscr);

    JEGA_IF_THREADSAFE(
        SubscriptionListenerCallbackVector::scoped_lock l(SLVEC())
        );

    for(
        SubscriptionListenerCallbackVector::const_iterator it(SLVEC().begin());
        it!=SLVEC().end(); ++it
        ) (*it)(subscr);

    return subscr;
}

bool
MessageBoard::Unsubscribe(
    const Subscription& s
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_THREADSAFE(SubscriptionVector::scoped_lock l1(SVEC()));

    SubscriptionVector::iterator it(find(SVEC().begin(), SVEC().end(), s));
    bool found = it != SVEC().end();

    if(found)
    {
        JEGA_IF_THREADSAFE(
            SubscriptionListenerCallbackVector::scoped_lock l2(SCLVEC())
            );
        for(
            SubscriptionListenerCallbackVector::iterator lt(SCLVEC().begin());
            lt!=SCLVEC().end(); ++lt
            ) (*lt)(*it);

        SVEC().erase(it);
    }

    return found;
}


bool
MessageBoard::HasSubscribers(
    const MessageIdentifier& msgId
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_IF_THREADSAFE(SubscriptionVector::scoped_lock l(SVEC()));

    for(
        SubscriptionVector::const_iterator it(SVEC().begin()); it!=SVEC().end();
        ++it
        ) if((*it).Check(msgId)) return true;

    return false;
}

size_t
MessageBoard::CountSubscribers(
    const MessageIdentifier& msgId
    )
{
    EDDY_FUNC_DEBUGSCOPE
    size_t ret = 0;

    JEGA_IF_THREADSAFE(SubscriptionVector::scoped_lock l(SVEC()));

    for(
        SubscriptionVector::const_iterator it(SVEC().begin()); it!=SVEC().end();
        ++it
        ) if((*it).Check(msgId)) ++ret;

    return ret;
}












void
MessageInfo::ResetMessageIdentifier(
    const MessageBoard::MessageIdentifier& newId
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_msgId = newId;
    this->_listenerCt = MessageBoard::CountSubscribers(this->_msgId);
}

void
MessageInfo::Register(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_listenerCt = MessageBoard::CountSubscribers(this->_msgId);
    this->Subscribe();
}

void
MessageInfo::Unregister(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    MessageBoard::RemoveSubscriptionListener(this->_subTag);
    MessageBoard::RemoveSubscriptionCanceledListener(this->_subCanceledTag);
    this->_listenerCt = 0;
}

const MessageInfo&
MessageInfo::operator =(
    const MessageInfo& other
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(this == &other) return other;
    this->_msgId = other._msgId;
    this->_listenerCt = other._listenerCt;
    this->_subTag = other._subTag;
    this->_subCanceledTag = other._subCanceledTag;
    return *this;
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
MessageInfo::Subscribe(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_subTag = MessageBoard::AddSubscriptionListener(
        bind1st(mem_fun(&MessageInfo::OnSubscription), this)
        );

    this->_subCanceledTag = MessageBoard::AddSubscriptionCanceledListener(
        bind1st(mem_fun(&MessageInfo::OnSubscriptionCanceled), this)
        );
}







/*
================================================================================
Structors
================================================================================
*/

MessageInfo::MessageInfo(
    ) :
        _msgId(),
        _listenerCt(0),
        _subTag(0),
        _subCanceledTag(0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->Subscribe();
}

MessageInfo::MessageInfo(
    const MessageBoard::MessageIdentifier& msgId
    ) :
        _msgId(msgId),
        _listenerCt(0),
        _subTag(0),
        _subCanceledTag(0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->Register();
}

MessageInfo::MessageInfo(
    const MessageInfo& copy
    ) :
        _msgId(copy._msgId),
        _listenerCt(0),
        _subTag(0),
        _subCanceledTag(0)
{
    EDDY_FUNC_DEBUGSCOPE
    this->Register();
}


MessageInfo::~MessageInfo(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->Unregister();
}



/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA

#else

#pragma message( \
    "NOTE: Message boarding is not available because Boost is not available." \
    )

#endif
