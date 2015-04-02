/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Definition of class MessageBoard.

    NOTES:

        See notes under "Document this File" section of this file.

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
 * \brief Contains the definition of the MessageBoard class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef JEGA_UTILITIES_MESSAGEBOARD_HPP
#define JEGA_UTILITIES_MESSAGEBOARD_HPP

#pragma once





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#ifdef JEGA_HAVE_BOOST
#   include <boost/unordered_set.hpp>
#   include <boost/unordered_map.hpp>
#   include <boost/function/function1.hpp>

#define JEGA_MESSAGE_BOARD

#endif

#ifdef JEGA_MESSAGE_BOARD
#   define JEGA_IF_MESSAGE_BOARD(a) a
#   define JEGA_IF_NOT_MESSAGE_BOARD(a)

#   define JEGAPOSTMSG(info, msg) \
        if(info.WillPost()) msg.Post(metric);

#else
#   define JEGA_IF_MESSAGE_BOARD(a)
#   define JEGA_IF_NOT_MESSAGE_BOARD(a) a
#   define JEGAPOSTMSG(info, msg)
#endif

#ifdef JEGA_MESSAGE_BOARD

#include <vector>


#ifdef JEGA_THREADSAFE
#   include <threads/include/ts_vector.hpp>
#else
#   include <vector>
#endif



/*
================================================================================
Pre-Namespace Forward Declares
================================================================================
*/






/*
================================================================================
Namespace Aliases
================================================================================
*/








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {





/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/
class MessageBoard;



#ifdef JEGA_THREADSAFE
#   define MY_VEC_T eddy::threads::ts_vector
#else
#   define MY_VEC_T std::vector
#endif




/*
================================================================================
In-Namespace File Scope Typedefs
================================================================================
*/









/*
================================================================================
Class Definition
================================================================================
*/
/**
 * \brief
 *
 *
 */
//template <typename FirstKeyT, typename SecondKeyT>
class MessageBoard
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        class MessageIdentifier
        {
            friend class MessageBoard;

            private:
                void* _sender;
                std::string _type;
                std::string _tag;

            public:

                inline
                const void*
                Sender(
                    ) const;

                inline
                const std::string&
                SenderType(
                    ) const;

                inline
                const std::string&
                Tag(
                    ) const;

                inline
                bool
                operator ==(
                    const MessageIdentifier& other
                    ) const;

                const MessageIdentifier&
                operator =(
                    const MessageIdentifier& other
                    );

            public:

                inline
                MessageIdentifier(
                    );

                inline
                MessageIdentifier(
                    void* sender,
                    const std::string& type,
                    const std::string& dataTag
                    );

                inline
                MessageIdentifier(
                    const MessageIdentifier& copy
                    );

        };

        class Message
        {
            friend class MessageBoard;

            private:

                const MessageIdentifier& _id;
                const std::string& _msg;

            public:

                inline
                const MessageIdentifier&
                Identifier(
                    ) const;

                inline
                const std::string&
                Text(
                    ) const;

                inline
                bool
                operator ==(
                    const Message& other
                    ) const;

            protected:

                inline
                Message(
                    const MessageIdentifier& identifier,
                    const std::string& msg
                    );

        };

        typedef
        boost::function1<bool, const MessageIdentifier&>
        MessagePredicate;

        typedef
        boost::function1<void, const Message&>
        SubscriberCallback;

        class Subscription
        {
            friend class MessageBoard;

            private:

                MessagePredicate _pred;
                SubscriberCallback _callback;
                std::size_t _id;
                static std::size_t NEXT_ID;

            public:

                inline
                bool
                Check(
                    const Message& msg
                    ) const;

                inline
                bool
                Check(
                    const MessageIdentifier& msgId
                    ) const;

                inline
                void
                Service(
                    const Message& msg
                    ) const;

                inline
                bool
                operator ==(
                    const Subscription& other
                    ) const;

                const Subscription&
                operator =(
                    const Subscription& other
                    );

                inline
                Subscription(
                    );

                inline
                Subscription(
                    const Subscription& copy
                    );

                template <typename PredT, typename CallbackT>
                inline
                Subscription(
                    const PredT& pred,
                    const CallbackT& callback
                    );

                template <typename PredT, typename CallbackT>
                inline
                Subscription(
                    const boost::reference_wrapper<PredT>& pred,
                    const boost::reference_wrapper<CallbackT>& callback
                    );

                template <typename PredT, typename CallbackT>
                inline
                Subscription(
                    const boost::reference_wrapper<PredT const>& pred,
                    const boost::reference_wrapper<CallbackT const>& callback
                    );
        };

        typedef
        boost::function1<void, const Subscription&>
        SubscriptionListenerCallback;

        typedef
        std::size_t
        SubscriberTag;

        typedef
        std::size_t
        SubscriptionListenerTag;

        class IdentifierPredicate :
            public std::unary_function<MessageIdentifier, bool>
        {
            private:

                const void* _who;
                const std::string _type;
                const std::string _tag;

            public:

                inline
                const void*
                Sender(
                    ) const
                {
                    return this->_who;
                }

                inline
                const std::string&
                Type(
                    ) const
                {
                    return this->_type;
                }

                inline
                const std::string&
                Tag(
                    ) const
                {
                    return this->_tag;
                }

                inline
                bool
                operator ()(
                    const MessageIdentifier& msgId
                    ) const
                {
                    return msgId.Sender() == this->_who &&
                           msgId.Tag() == this->_tag &&
                           msgId.SenderType() == this->_type;
                }

                IdentifierPredicate(
                    const void* who,
                    const std::string& type,
                    const std::string& tag
                    ) :
                        _who(who),
                        _type(type),
                        _tag(tag)
                {}

                IdentifierPredicate(
                    const IdentifierPredicate& copy
                    ) :
                        _who(copy._who),
                        _type(copy._type),
                        _tag(copy._tag)
                {}
        };


        class WholessIdentifierPredicate :
            public std::unary_function<MessageIdentifier, bool>
        {
            private:

                const std::string _type;
                const std::string _tag;

            public:

                inline
                const std::string&
                Type(
                    ) const
                {
                    return this->_type;
                }

                inline
                const std::string&
                Tag(
                    ) const
                {
                    return this->_tag;
                }

                inline
                bool
                operator ()(
                    const MessageIdentifier& msgId
                    ) const
                {
                    return msgId.Tag() == this->_tag &&
                           msgId.SenderType() == this->_type;
                }

                WholessIdentifierPredicate(
                    const std::string& type,
                    const std::string& tag
                    ) :
                        _type(type),
                        _tag(tag)
                {}

                WholessIdentifierPredicate(
                    const WholessIdentifierPredicate& copy
                    ) :
                        _type(copy._type),
                        _tag(copy._tag)
                {}

                WholessIdentifierPredicate(
                    const IdentifierPredicate& copy
                    ) :
                        _type(copy.Type()),
                        _tag(copy.Tag())
                {}
        };

    protected:


    private:

        typedef
        MY_VEC_T<Subscription>
        SubscriptionVector;

        typedef
        MY_VEC_T<SubscriptionListenerCallback>
        SubscriptionListenerCallbackVector;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:





    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        static
        Subscription
        Subscribe(
            const MessagePredicate& pred,
            const SubscriberCallback& callback
            );

        template <typename PredT, typename CallbackT>
        static
        Subscription
        Subscribe(
            const PredT& pred,
            const CallbackT& callback
            );

        template <typename PredT, typename CallbackT>
        static
        Subscription
        Subscribe(
            const boost::reference_wrapper<PredT>& pred,
            const boost::reference_wrapper<CallbackT>& callback
            );

        template <typename PredT, typename CallbackT>
        static
        Subscription
        Subscribe(
            const boost::reference_wrapper<PredT const>& pred,
            const boost::reference_wrapper<CallbackT const>& callback
            );

        static
        bool
        Unsubscribe(
            const Subscription& s
            );

        inline static
        SubscriptionListenerTag
        AddSubscriptionListener(
            SubscriptionListenerCallback listener
            );

        inline static
        SubscriptionListenerTag
        AddSubscriptionCanceledListener(
            SubscriptionListenerCallback listener
            );

        inline static
        void
        RemoveSubscriptionListener(
            const SubscriptionListenerTag& tag
            );

        inline static
        void
        RemoveSubscriptionCanceledListener(
            const SubscriptionListenerTag& tag
            );

        static
        void
        PostMessage(
            const Message& msg
            );

        static inline
        void
        PostMessage(
            const MessageIdentifier& msgId,
            const std::string& text
            );

        static
        bool
        HasSubscribers(
            const MessageIdentifier& msgId
            );

        static inline
        bool
        HasSubscribers(
            const Message& msg
            );

        static
        std::size_t
        CountSubscribers(
            const MessageIdentifier& msgId
            );

        static inline
        std::size_t
        CountSubscribers(
            const Message& msg
            );


    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:





    /*
    ============================================================================
    Subclass Overridable Methods
    ============================================================================
    */
    public:


    protected:


    private:


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:

        static inline
        SubscriptionVector&
        SVEC(
            );

        static inline
        SubscriptionListenerCallbackVector&
        SLVEC(
            );

        static inline
        SubscriptionListenerCallbackVector&
        SCLVEC(
            );


    /*
    ============================================================================
    Structors
    ============================================================================
    */
    private:

        MessageBoard(
            );




}; // class MessageBoard


class MessageInfo
{
    private:

        MessageBoard::MessageIdentifier _msgId;
        std::size_t _listenerCt;

        MessageBoard::SubscriptionListenerTag _subTag;
        MessageBoard::SubscriptionListenerTag _subCanceledTag;

    public:

        inline
        const MessageBoard::MessageIdentifier&
        MessageIdentifier(
            ) const;

        inline
        bool
        HasListeners(
            ) const;

        inline
        bool
        WillPost(
            ) const;

        void
        ResetMessageIdentifier(
            const MessageBoard::MessageIdentifier& newId
            );

        void
        Register(
            );

        void
        Unregister(
            );

        inline
        bool
        Post(
            const std::string& msgText
            ) const;

        template <typename T>
        bool
        Post(
            const T& msg
            ) const;

        const MessageInfo&
        operator =(
            const MessageInfo& other
            );

    private:

        void
        Subscribe(
            );

        inline
        void
        OnSubscription(
            MessageBoard::Subscription subsc
            );

        inline
        void
        OnSubscriptionCanceled(
            MessageBoard::Subscription subsc
            );

    public:

        MessageInfo(
            );

        MessageInfo(
            const MessageBoard::MessageIdentifier& msgId
            );

        MessageInfo(
            const MessageInfo& copy
            );

        ~MessageInfo(
            );

};


/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA







/*
================================================================================
Include Inlined Functions File
================================================================================
*/
#include "./inline/MessageBoard.hpp.inl"


#endif


/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // JEGA_UTILITIES_MESSAGEBOARD_HPP
