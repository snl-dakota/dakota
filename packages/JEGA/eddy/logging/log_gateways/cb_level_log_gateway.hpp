/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class cb_level_log_gateway.

    NOTES:

        See notes under Class Definition section of this file.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Mar 03 13:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the cb_level_log_gateway class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_CB_LEVEL_LOG_MANAGER_HPP
#define EDDY_LOGGING_CB_LEVEL_LOG_MANAGER_HPP


/*
================================================================================
Conditional Inclusion
================================================================================
*/
#ifdef EDDY_LOGGING_ON





/*
================================================================================
Includes
================================================================================
*/
#include "../include/config.hpp"
#include "level_log_gateway.hpp"




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
namespace eddy {
    namespace logging {




/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/




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
 * \brief A log gateway that logs entries based on comparison of logging
 *        levels and may call to callbacks for entries logged at specific
 *        levels.
 *
 * This gateway behaves exactly as the level_log_gateway with the exception
 * that it has the capacity to store callbacks mapped to specific logging
 * levels.
 *
 * \param LogT The type of the log to which this gateway forwards entries.
 * \param LevC The level policy compliant class to use as levels.
 * \param CBArgT The type of arguments that may be passed to callbacks.
 * \param CharT The character type being used for all text in this app.
 * \param Traits The traits type for the character type being used in this app.
 *
 * \ingroup log_gateway_types
 */
template <
          typename LogT,
          typename LevC = EDDY_LOGGING_DEF_LEVEL_CLASS,
          typename CharT = EDDY_LOGGING_DEF_CHAR_TYPE,
          typename Traits = EDDY_LOGGING_DEF_CHAR_TRAITS,
          typename CBArgT = std::basic_string<CharT, Traits>
         >
class EDDY_SL_IEDECL cb_level_log_gateway :
    protected level_log_gateway<LogT, LevC, CharT, Traits>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    public:

        /// The type of the log to which log entries must be sent.
        typedef
        LogT
        log_type;

        /// The class type containing the levels enumeration.
        typedef
        LevC
        level_class;

        /// A synonym for the character type of this log gateway.
        typedef
        CharT
        char_type;

        /// A synonym for the character traits type.
        typedef
        Traits
        traits_type;

        /// The type containing the levels to be used by this logger.
        typedef
        typename level_class::level
        level_type;

        /// The type of the argument sent into callbacks.
        typedef
        CBArgT
        callback_arg_type;

    private:

        /// A shorthand for this type
        typedef
        cb_level_log_gateway<
                             log_type,
                             level_class,
                             callback_arg_type,
                             char_type,
                             traits_type
                            >
        my_type;

    protected:

        /// A shorthand for the base class type of this gateway.
        typedef
        level_log_gateway<log_type, level_class, char_type, traits_type>
        base_type;

    /*
    ============================================================================
    Nested Utility Classes
    ============================================================================
    */
    public:

        /**
         * \brief A required base class for any class that wishes to be mapped
         *        to a level for callback by this log gateway.
         *
         * All subclasses must override the single operator() overload of this
         * base class.
         */
        class callback
        {
            /*
            ====================================================================
            Subclass Overridable Methods
            ====================================================================
            */
            public:

                /// The activation method of this callback.
                /**
                 * All derived classes must implement this method.
                 *
                 * \param arg The argument that will be passed when this
                 *            callback is invoked.
                 */
                virtual
                void
                operator () (
                    const callback_arg_type& arg
                    ) = 0;

            /*
            ====================================================================
            Structors
            ====================================================================
            */
            public:

                /// Destructs this callback object.
                virtual
                ~callback(
                    ) {}

        }; // class callback


    /*
    ============================================================================
    Class Scope Typedefs Continued
    ============================================================================
    */
    protected:

        /// A map of callbacks for individual levels.
        typedef
        MAP_TYPE<level_type, callback*>
        callback_map;

    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// A function object map of callbacks for loggings of certain levels.
        callback_map _callbacks;

        /// A mutex to protect the callback map.
        EDDY_DECLARE_MUTABLE_MUTEX(_callback_map_mutex)

    /*
    ============================================================================
    Public Methods
    ============================================================================
    */
    public:

        /// Logs e to the known log using default levels.
        /**
         * The entry is only passed on to the log if the levels permit.
         * The entry is considered to have been requested at the
         * level_class::Default level and it will be checked against
         * this gateways default level.
         *
         * \param e The entry to be logged to the known log.
         * \return True if e gets logged and false otherwise.
         */
        template <typename ET>
        inline
        bool
        log(
            const ET& e
            )
        {
            return this->my_type::log_if(
                this->get_default_level(), level_class::Default, e
                );
        }

        /// Makes a log request at level elevel for entry e.
        /**
         * The entry is only passed on to the log if the levels permit.
         * elevel will be compared to this gateways default level.
         *
         * \param elevel The level at which the entry logging is being
         *               requested.
         * \param e The entry to be logged to the known log.
         * \return True if e gets logged and false otherwise.
         */
        template <typename ET>
        inline
        bool
        log(
            const level_type& elevel,
            const ET& e
            )
        {
            return this->my_type::log_if(this->get_default_level(), elevel, e);
        }

        /// Makes a log request at level elevel for entry e issued by IssType.
        /**
         * The entry is only passed on to the log if the levels permit.
         * The level elevel will be compared to the level appropriate to the
         * issuers type.
         *
         * \param elevel The level at which the entry logging is being
         *               requested.
         * \param e The entry to be logged to the known log.
         * \return True if e gets logged and false otherwise.
         */
        template <typename IssType, typename ET>
        inline
        bool
        log(
            const level_type& elevel,
            const ET& e
            )
        {
            return this->my_type::log_if(
                this->EDDY_TEMPLATE_KEYWORD_QUALIFIER
                    get_logging_level<IssType>(), elevel, e
                );
        }

        /// Makes a log request at level elevel for entry e issued by issuer.
        /**
         * The entry is only passed on to the log if the levels permit.
         * The level elevel will be compared to the level appropriate to
         * issuer.
         *
         * \param elevel The level at which the entry logging is being
         *               requested.
         * \param issuer The issuer responsible for this log request.
         * \param e The entry to be logged to the known log.
         * \return True if e gets logged and false otherwise.
         */
        template <typename IssType, typename ET>
        inline
        bool
        log(
            const level_type& elevel,
            const IssType* issuer,
            const ET& e
            )
        {
            return this->my_type::log_if(
                this->get_logging_level(issuer), elevel, e
                );
        }

        /// Logs e to the known log without checking levels.
        /**
         * The entry is passed along to the log regardless of
         * logging levels etc.  This is most useful when an explicit
         * check of logging levels is carried out prior to this call
         * in order that the entry creation may be avoided.  See the
         * macro interface for examples of this.
         *
         * Callbacks are still respected by this method.
         *
         * \param elevel The level at which the entry logging is being
         *               requested.
         * \param e The entry to be logged to the known log.
         * \return True no matter what (return is only provided for
         *         consistancy with other logging methods).
         */
        template <typename ET>
        bool
        simple_log(
            const level_type& elevel,
            const ET& e
            )
        {
            this->base_type::simple_log(e);
            this->do_callback(elevel, e);
            return true;
        }

        /**
         * \brief Returns true if the logger will currently log for IssType
         *        at the elevel level.
         *
         * \param elevel The level at which an entry was or would be issued.
         * \return True if an entry at level elevel would be logged against
         *         the level appropriate for IssType.
         */
        template<typename IssType>
        inline
        bool
        will_log(
            const level_type& elevel
            ) const
        {
            return static_cast<const base_type&>(*this).
                EDDY_TEMPLATE_KEYWORD_QUALIFIER will_log<IssType>(elevel);
        }

        /**
         * \brief Returns true if the logger will log for issuer at the
         *        supplied level elevel.
         *
         * \param issuer The issuer who would try to log at level elevel.
         * \param elevel The level at which issuer would try to log.
         * \return True if an entry at level elevel would be logged against
         *         the level appropriate for issuer.
         */
        template<typename IssType>
        inline
        bool
        will_log(
            const IssType* issuer,
            const level_type& elevel
            ) const
        {
            return this->base_type::will_log(issuer, elevel);
        }

        /// Returns true if elevel is greater than or equal to ilevel.
        /**
         * \param ilevel The level at (or above) which the issuer is allowed
         *               to log.
         * \param elevel The level at which the entry was or would be issued.
         * \return True if an entry at level elevel would be logged against
         *         an issuer level if ilevel.
         */
        inline
        bool
        will_log(
            const level_type& ilevel,
            const level_type& elevel
            ) const
        {
            return this->base_type::will_log(ilevel, elevel);
        }

        /// Sets the logging level for the issuer to the supplied level.
        /**
         * \param issuer A pointer to the issuer to set the logging level for.
         * \param ilevel The level at or above which issuer will hence be able
         *               to log.
         */
        template<typename IssType>
        inline
        void
        set_logging_level(
            const IssType* issuer,
            const level_type& ilevel
            )
        {
            this->base_type::set_logging_level(issuer, ilevel);
        }

        /**
         * \brief Sets the logging level for all obects of the supplied type
         * to the supplied level.
         *
         * Any individual objects for whom the level has been explicitly
         * set will not be subject to the type level.
         *
         * \param ilevel The level at or above which objects of IssType will
         *               hence be able to log.
         */
        template<typename IssType>
        inline
        void
        set_logging_level(
            const level_type& ilevel
            )
        {
            // just use the overload with the typeinfo on IssType.
            static_cast<base_type&>(*this).EDDY_TEMPLATE_KEYWORD_QUALIFIER
                set_logging_level<IssType>(ilevel);
        }

        /// Returns the logging level for the IssType.
        /**
         * If the type has been registered with a particular level, that
         * level is returned.  Otherwise the default level is returned.
         *
         * \return The level at (or above) which IssType is allowed to log.
         */
        template<typename IssType>
        inline
        const level_type&
        get_logging_level(
            ) const
        {
            return static_cast<const base_type&>(*this).
                EDDY_TEMPLATE_KEYWORD_QUALIFIER get_logging_level<IssType>();
        }

        /// Returns the logging level for the entity of \a issuer.
        /**
         * If the issuer has a particular entry in the level map, that
         * level is returned.  Otherwise, if the type of the issuer
         * (IssType) has a particular entry, that level is returned.
         * Otherwise the default is returned.
         *
         * \param issuer The issuer whose allowed level is of interest.
         * \return The level at which issuer is allowed to log which may be
         *         the default.
         */
        template<typename IssType>
        inline
        const level_type&
        get_logging_level(
            const IssType* issuer
            ) const
        {
            return this->base_type::get_logging_level(issuer);
        }

        /// Returns the default logging level for this log gateway..
        /**
         * The default logging level is the level at (or above) which all
         * non-mapped entities and types are allowed to log.
         *
         * \return The default level used by this log gateway.
         */
        inline
        const level_type&
        get_default_level(
            ) const
        {
            return this->base_type::get_default_level();
        }

        /// Allows mutation of the default logging level of this gateway.
        /**
         * \param l The new default level for this level gateway.
         */
        inline
        void
        set_default_level(
            const level_type& l
            )
        {
            this->base_type::set_default_level(l);
        }

        /**
         * \brief Returns true if \a issuer is mapped to a particular level of
         * it's own.
         *
         * \param issuer The instance of IssType to be checked for a mapping to
         *               a particular level.
         * \return True if issuer is mapped to a particular level and false
         *         otherwise.
         */
        template<typename IssType>
        inline
        bool
        is_mapped(
            const IssType* issuer
            ) const
        {
            return this->base_type::is_mapped(issuer);
        }

        /**
         * \brief Returns true if IssType is mapped to a particular level of
         *        it's own.
         *
         * \return True if IssType is mapped to a particular level and false
         *         otherwise.
         */
        template<typename IssType>
        inline
        bool
        is_mapped(
            ) const
        {
            return static_cast<const base_type&>(*this).
                EDDY_TEMPLATE_KEYWORD_QUALIFIER is_mapped<IssType>();
        }

        /// Removes issuer from the list of specified logging level entities.
        /**
         * The return is true if \a issuer was found and removed and false
         * otherwise.
         *
         * This is not necessary for safe operation but for large numbers of
         * temporary objects, this can improve performance by keeping the map
         * small.
         *
         * \param issuer The instance of IssType to be unmapped.
         * \return True if issuer was successfully removed from the map.
         */
        template<typename IssType>
        inline
        bool
        clear_logging_level(
            const IssType* issuer
            )
        {
            return this->base_type::clear_logging_level(issuer);
        }

        /// Removes IssType from the list of specified logging level entities.
        /**
         * The return is true if IssType was found and removed and false
         * otherwise.
         *
         * This is not necessary for safe operation but for large numbers of
         * temporary objects, this can improve performance by keeping the map
         * small.
         *
         * \return True if issuer's logging level was successfully removed from
         *         the map.
         */
        template<typename IssType>
        inline
        bool
        clear_logging_level(
            )
        {
            return static_cast<const base_type&>(*this).
                EDDY_TEMPLATE_KEYWORD_QUALIFIER clear_logging_level<IssType>();
        }

        /// Clears the map of types and objects to levels.
        /**
         * After this, any types or objects given a logging level using
         * set_logging_level will no longer be mapped.  Subsequently, any
         * calls to is_mapped will return false and all calls to
         * get_logging_level will return the default level of this gateway.
         */
        inline
        void
        clear_logging_map(
            )
        {
            this->base_type::clear_logging_map();
        }

        /**
         * \brief Sets f as the function to be called back in the case of an
         *        entry at level elevel.
         *
         * Only 1 callback can be associated with each level.  If a set
         * callback request is made for a level that is already mapped to a
         * callback, replacement of the old callback will occur silently.
         *
         * Callback pointers are copied into the callback map.  This class
         * does not assume ownership of these objects, just usage.  Therefore
         * it will not destroy them...ever.  The user is responsible for
         * controlling the lifespan of these objects and if one is to be
         * destroyed before this gateway, the user should call the
         * remove_callback method to ensure that this gateway does not
         * attempt to use it in the future.
         *
         * In order to accomplish this, a reference to this gateway could
         * be stored in your callback and the callback could thus remove
         * itself in its destructor.
         *
         * \param elevel The level to associate with the supplied callback.
         * \param cb The callback to call in case of a logging at level
         *           \a elevel.
         */
        inline
        void
        set_callback(
            const level_type& elevel,
            callback* cb
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_callback_map_mutex)
            this->_callbacks[elevel] = cb;
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Removes all mappings of any levels to the supplied callback
        /**
         * \param cb The callback that will no longer be used.
         */
        inline
        void
        remove_callback(
            callback* cb
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_callback_map_mutex)
            for(typename callback_map::iterator it(this->_callbacks.begin());
                it!=this->_callbacks.end();)
            {
                if((*it).second == cb) it=this->_callbacks.remove(it);
                else ++it;
            }

            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Removes the mapping of the supplied level to a callback.
        /**
         * \param elevel The level that is no longer to be associated with a
         *               callback.
         */
        inline
        void
        remove_callback(
            const level_type& elevel
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_callback_map_mutex)
            typename callback_map::iterator it(this->_callbacks.find(elevel));
            if(it != this->_callbacks.end()) this->_callbacks.erase(it);
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

        /// Allows mutable access to the underlying log for this logger.
        /**
         * \return A reference to the log known by this gateway.
         */
        inline
        log_type&
        get_log(
            )
        {
            return this->base_type::get_log();
        }

        /// Allows immutable access to the underlying log for this logger.
        /**
         * \return A constant reference to the log known by this gateway.
         */
        inline
        const log_type&
        get_log(
            ) const
        {
            return this->base_type::get_log();
        }

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Logs entry e if the levels allow it.
        /**
         * If the issuer level is sufficient to allow logging at the entry
         * level, then e is logged and any relevant callbacks are issued.
         *
         * \param ilevel The level at (or above) which the issuer is allowed
         *               to log.
         * \param elevel The level at which the entry was or would be issued.
         * \param e The actual entry to log based on the supplied levels.
         * \return True if the entry gets logged and false otherwise.
         */
        template <typename ET>
        bool
        log_if(
            const level_type& ilevel,
            const level_type& elevel,
            const ET& e
            )
        {
            bool ret = this->base_type::log_if(ilevel, elevel, e);
            if(ret) this->do_callback(elevel, e);
            return ret;
        }

        /**
         * \brief Issues a callback with e if the entry level is mapped to a
         *        callback.
         *
         * \param elevel The level at which a logging occurred.
         * \param e The entry that was logged at level \a elevel.
         */
        template <typename ET>
        void
        do_callback(
            const level_type& elevel,
            const ET& e
            )
        {
            // find out if a callback is in order and use it.
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_callback_map_mutex)
            if(!this->_callbacks.empty())
            {
                typename callback_map::iterator it(
                    this->_callbacks.find(elevel)
                    );
                if(it != this->_callbacks.end())
                    (*it).second->operator()(e);
            }
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }


    /*
    ============================================================================
    Private Methods
    ============================================================================
    */
    private:



    /*
    ============================================================================
    Structors
    ============================================================================
    */
    public:

        /**
         * \brief Constructs a log gateway to manage log using the supplied
         *        level as the default.
         *
         * \param log The log to associate with this gateway.
         * \param def_level The default level at which to log.
         */
        cb_level_log_gateway(
            log_type& log,
            const level_type& def_level = level_class::Default
            ) :
                base_type(log, def_level),
                _callbacks() EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_callback_map_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
        }

        /// Copy constructs a cb_level_log_gateway.
        /**
         * \param copy The gateway to copy the properties from in this new
         *             gateway object.
         */
        cb_level_log_gateway(
            const my_type& copy
            ) :
                base_type(copy),
                _callbacks() EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_callback_map_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, copy._callback_map_mutex);
            this->_callbacks = copy._callbacks;
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION
        }

}; // class cb_level_log_gateway




/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace eddy
} // namespace logging






/*
================================================================================
Include Inlined Functions File
================================================================================
*/




/*
================================================================================
End of Conditional Inclusion
================================================================================
*/
#endif



/*
================================================================================
End of Multiple Inclusion Check
================================================================================
*/
#endif // EDDY_LOGGING_CB_LEVEL_LOG_MANAGER_HPP
