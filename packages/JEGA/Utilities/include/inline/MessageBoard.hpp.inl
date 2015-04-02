/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Inline methods of class MessageBoard.

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
 * \brief Contains the inline methods of the MessageBoard class.
 */

#ifdef JEGA_MESSAGE_BOARD


/*
================================================================================
Includes
================================================================================
*/
#include <sstream>







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {





/*
================================================================================
Inline Public Methods
================================================================================
*/

inline
const void*
MessageBoard::MessageIdentifier::Sender(
    ) const
{
    return this->_sender;
}

inline
const std::string&
MessageBoard::MessageIdentifier::SenderType(
    ) const
{
    return this->_type;
}

inline
const std::string&
MessageBoard::MessageIdentifier::Tag(
    ) const
{
    return this->_tag;
}

inline
bool
MessageBoard::MessageIdentifier::operator ==(
    const MessageIdentifier& other
    ) const
{
    if(this == &other) return true;

    return this->_sender == other._sender &&
           this->_tag == other._tag &&
           this->_type == other._type;
}

inline
MessageBoard::MessageIdentifier::MessageIdentifier(
    ) :
        _sender(0x0),
        _type(),
        _tag()
{}

inline
MessageBoard::MessageIdentifier::MessageIdentifier(
    void* sender,
    const std::string& type,
    const std::string& dataTag
    ) :
        _sender(sender),
        _type(type),
        _tag(dataTag)
{}

inline
MessageBoard::MessageIdentifier::MessageIdentifier(
    const MessageIdentifier& copy
    ) :
        _sender(copy._sender),
        _type(copy._type),
        _tag(copy._tag)
{}







inline
const MessageBoard::MessageIdentifier&
MessageBoard::Message::Identifier(
    ) const
{
    return this->_id;
}

inline
const std::string&
MessageBoard::Message::Text(
    ) const
{
    return this->_msg;
}

inline
bool
MessageBoard::Message::operator ==(
    const Message& other
    ) const
{
    if(this == &other) return true;
    return this->_id == other._id && this->_msg == other._msg;
}

inline
MessageBoard::Message::Message(
    const MessageIdentifier& identifier,
    const std::string& msg
    ) :
        _id(identifier),
        _msg(msg)
{}







inline
bool
MessageBoard::Subscription::Check(
    const Message& msg
    ) const
{
    return this->Check(msg.Identifier());
}

inline
bool
MessageBoard::Subscription::Check(
    const MessageIdentifier& msgId
    ) const
{
    return this->_pred(msgId);
}

inline
void
MessageBoard::Subscription::Service(
    const Message& msg
    ) const
{
    if(this->Check(msg)) this->_callback(msg);
}

inline
bool
MessageBoard::Subscription::operator ==(
    const Subscription& other
    ) const
{
    return this->_id == other._id;
}

inline
MessageBoard::Subscription::Subscription(
    ) :
        _pred(),
        _callback(),
        _id(NEXT_ID++)
{}

inline
MessageBoard::Subscription::Subscription(
    const Subscription& copy
    ) :
        _pred(copy._pred),
        _callback(copy._callback),
        _id(copy._id)
{}

template <typename PredT, typename CallbackT>
inline
MessageBoard::Subscription::Subscription(
    const PredT& pred,
    const CallbackT& callback
    ) :
        _pred(pred),
        _callback(callback),
        _id(NEXT_ID++)
{}

template <typename PredT, typename CallbackT>
inline
MessageBoard::Subscription::Subscription(
    const boost::reference_wrapper<PredT>& pred,
    const boost::reference_wrapper<CallbackT>& callback
    ) :
        _pred(pred),
        _callback(callback),
        _id(NEXT_ID++)
{}

template <typename PredT, typename CallbackT>
inline
MessageBoard::Subscription::Subscription(
    const boost::reference_wrapper<PredT const>& pred,
    const boost::reference_wrapper<CallbackT const>& callback
    ) :
        _pred(pred),
        _callback(callback),
        _id(NEXT_ID++)
{}






















template <typename PredT, typename CallbackT>
inline
MessageBoard::Subscription
MessageBoard::Subscribe(
    const PredT& pred,
    const CallbackT& callback
    )
{
    return Subscribe(MessagePredicate(pred), SubscriberCallback(callback));
}

template <typename PredT, typename CallbackT>
inline
MessageBoard::Subscription
MessageBoard::Subscribe(
    const boost::reference_wrapper<PredT>& pred,
    const boost::reference_wrapper<CallbackT>& callback
    )
{
    return Subscribe(MessagePredicate(pred), SubscriberCallback(callback));
}

template <typename PredT, typename CallbackT>
inline
MessageBoard::Subscription
MessageBoard::Subscribe(
    const boost::reference_wrapper<PredT const>& pred,
    const boost::reference_wrapper<CallbackT const>& callback
    )
{
    return Subscribe(MessagePredicate(pred), SubscriberCallback(callback));
}

inline
MessageBoard::SubscriptionListenerTag
MessageBoard::AddSubscriptionListener(
    SubscriptionListenerCallback listener
    )
{
    JEGA_IF_THREADSAFE(
        SubscriptionListenerCallbackVector::scoped_lock l(SLVEC())
        );
    SLVEC().push_back(listener);
    return SLVEC().size() - 1;
}

inline
MessageBoard::SubscriptionListenerTag
MessageBoard::AddSubscriptionCanceledListener(
    SubscriptionListenerCallback listener
    )
{
    JEGA_IF_THREADSAFE(
        SubscriptionListenerCallbackVector::scoped_lock l(SCLVEC())
        );
    SCLVEC().push_back(listener);
    return SCLVEC().size() - 1;
}

inline
void
MessageBoard::RemoveSubscriptionListener(
    const SubscriptionListenerTag& tag
    )
{
    JEGA_IF_THREADSAFE(
        SubscriptionListenerCallbackVector::scoped_lock l(SLVEC())
        );
    SLVEC().erase(SLVEC().begin() + tag);
}

inline
void
MessageBoard::RemoveSubscriptionCanceledListener(
    const SubscriptionListenerTag& tag
    )
{
    JEGA_IF_THREADSAFE(
        SubscriptionListenerCallbackVector::scoped_lock l(SCLVEC())
        );
    SCLVEC().erase(SCLVEC().begin() + tag);
}

inline
void
MessageBoard::PostMessage(
    const MessageIdentifier& msgId,
    const std::string& text
    )
{
    PostMessage(Message(msgId, text));
}

inline
bool
MessageBoard::HasSubscribers(
    const Message& msg
    )
{
    return HasSubscribers(msg.Identifier());
}

inline
std::size_t
MessageBoard::CountSubscribers(
    const Message& msg
    )
{
    return CountSubscribers(msg.Identifier());
}






inline
bool
MessageInfo::Post(
    const std::string& msgText
    ) const
{
    if(this->WillPost())
    {
        MessageBoard::PostMessage(this->_msgId, msgText);
        return true;
    }
    return false;
}

template <typename T>
bool
MessageInfo::Post(
    const T& msg
    ) const
{
    std::ostringstream ostr;
    ostr << msg;
    return this->Post(ostr.str());
}

inline
const MessageBoard::MessageIdentifier&
MessageInfo::MessageIdentifier(
    ) const
{
    return this->_msgId;
}

inline
bool
MessageInfo::HasListeners(
    ) const
{
    return this->_listenerCt > 0;
}

inline
bool
MessageInfo::WillPost(
    ) const
{
    return this->HasListeners();
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
inline
void
MessageInfo::OnSubscription(
    MessageBoard::Subscription subsc
    )
{
    if(subsc.Check(this->_msgId)) ++this->_listenerCt;
}

inline
void
MessageInfo::OnSubscriptionCanceled(
    MessageBoard::Subscription subsc
    )
{
    if(this->_listenerCt > 0 && subsc.Check(this->_msgId))
        --this->_listenerCt;
}

inline
MessageBoard::SubscriptionVector&
MessageBoard::SVEC(
    )
{
    static SubscriptionVector ret;
    return ret;
}

inline
MessageBoard::SubscriptionListenerCallbackVector&
MessageBoard::SLVEC(
    )
{
    static SubscriptionListenerCallbackVector ret;
    return ret;
}

inline
MessageBoard::SubscriptionListenerCallbackVector&
MessageBoard::SCLVEC(
    )
{
    static SubscriptionListenerCallbackVector ret;
    return ret;
}







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
    } // namespace Utilities
} // namespace JEGA


#endif