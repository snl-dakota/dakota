/*
================================================================================
    PROJECT:

        Eddy C++ Logging

    CONTENTS:

        Definition of class level_log_gateway.

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

        Mon Jan 24 13:33:44 2005 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the definition of the level_log_gateway class.
 */




/*
================================================================================
Prevent Multiple Inclusions
================================================================================
*/
#ifndef EDDY_LOGGING_LEVEL_LOG_MANAGER_HPP
#define EDDY_LOGGING_LEVEL_LOG_MANAGER_HPP


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
#include <typeinfo>
#include "../include/config.hpp"
#include "basic_log_gateway.hpp"

#ifdef EDDY_HAVE_BOOST
#   include <boost/unordered_map.hpp>
#else
#   include <map>
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
namespace eddy {
    namespace logging {




/*
================================================================================
In-Namespace Forward Declares
================================================================================
*/


#ifdef EDDY_HAVE_BOOST
#define MAP_TYPE boost::unordered_map
#else
#define MAP_TYPE std::map
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
 * \brief A log gateway that logs entries based on comparison of logging
 *        levels.
 *
 * With most applications, there is a need to log information about many
 * possible exceptional circumstances.  It is often desired to reflect the
 * severity of those circumstances in the logs and even discriminate against
 * entries with an insufficient severity level.
 *
 * It is also common to want to be able to control the "verbocity" of the
 * logs created without disabling logging all together.  This gateway and
 * any similar were created in order to accomplish that.  Entries that pass
 * through this gateway are subject to rejection based on logging levels.
 *
 * Some other methods of accomplishing a similar differentiation of entry
 * severity levels include the creation of multiple logs and or the creation
 * of multiple logging "channels".  This method allows creation of a single
 * log image with all information optionally restricted using a minimum
 * level.
 *
 * By supplying a level for a given entry, this gateway can decide whether
 * or not that entry should be logged.  So for example, if the level is set
 * such that this gateway will accept only log entries of the highest
 * importance, then all less important entries are not sent to the underlying
 * log.
 *
 * This class accepts a level policy compliant class as a template argument.
 * It may use this policy class to decide what to use as an initial default
 * logging level for all entries.  That level policy class is where a user can
 * define what levels exist in their system.  It must adhere to certain
 * restrictions (See the \ref levels "level policies" section on the
 * <a href=./index.html>Main Page</a>) in order for use with this gateway such
 * as supplying a Default level.
 *
 * In addition to having a default logging level, this gateway allows the
 * setting of different levels for different program entities.  Specifically,
 * any particular instance of any type (except a std::type_info) can be set to
 * log at or above a specific level.  Likewise, any type as a whole  (except
 * std::type_info) can be set to do the same.  A setting on a particular
 * instance takes precedence over a setting of its type.
 *
 * Why support this type of behavior?  Presumably, during for example the
 * debugging process, a programmer will have some idea where to start looking
 * for a problem and will "hone-in" on it using whatever means available
 * (a debugger for instance).  If the problem is known to be coming from
 * a particular object or type of object, then the logging level for that
 * object or type can be decreased for a more verbose output while logging
 * by the rest of the application can occur at a higher level.
 *
 * Also, even during normal operation there may be components of an
 * application whose workings are of greater interest than those of the rest.
 * In that case, it would be nice to be able to configure the logging
 * levels for the various components or modules to reflect those levels of
 * interest (using a configuration file for example).
 *
 * There are a number of ways to implement this behavior.  The approach here
 * is to keep a map of void* to logging levels.  In order to set the level
 * for an instance, a void* to it is mapped to a level.  In order to set
 * the level for a type, a void* to the type_info object describing that
 * type is kept in the same map.  This is why the type type_info and instances
 * of it cannot be mapped to logging levels the way other program types and
 * entities can.  These pointers are never cast to anything else and of
 * course never dereferenced.
 *
 * There are also a number of ways to indicate to this gateway the level at
 * which you want an entry to be logged and the cutoff level that should be
 * applied to it.  In order to accomodate the various ways, there are a few
 * overloads of the log function.  Some of them accept the entry level and
 * issuer level, some of them make default assumptions, and some accept the
 * identity of the issuer (type or instance).
 *
 * \param LogT The type of the log to which this gateway forwards entries.
 * \param LevC The level policy compliant class to use as levels.
 * \param CharT The character type being used for all text in this app.
 * \param Traits The traits type for the character type being used in this app.
 *
 * \ingroup log_gateway_types
 */
template <
          typename LogT,
          typename LevC = EDDY_LOGGING_DEF_LEVEL_CLASS,
          typename CharT = EDDY_LOGGING_DEF_CHAR_TYPE,
          typename Traits = EDDY_LOGGING_DEF_CHAR_TRAITS
         >
class EDDY_SL_IEDECL level_log_gateway :
    protected basic_log_gateway<LogT, CharT, Traits>
{
    /*
    ============================================================================
    Class Scope Typedefs
    ============================================================================
    */
    protected:

        /// A shorthand for the base class type of this gateway.
        typedef
        basic_log_gateway<LogT, CharT, Traits>
        base_type;

    public:

        /// The type of the log to which log entries must be sent.
        typedef
        typename base_type::log_type
        log_type;

        /// The class type containing the levels enumeration.
        typedef
        LevC
        level_class;

        /// A synonym for the character type of this log gateway.
        typedef
        typename base_type::char_type
        char_type;

        /// A synonym for the character traits type.
        typedef
        typename base_type::traits_type
        traits_type;

        /// The type containing the levels to be used by this logger.
        typedef
        typename level_class::level
        level_type;

    protected:

        /// A shorthand for the type of the pointers stored in the level map.
        typedef
        const void*
        map_ptr_t;

        /// A shorthand for the type of the pointers stored in the level map.
        typedef
        const std::type_info*
        type_ptr_t;

    private:

        /// A shorthand for this type
        typedef
        level_log_gateway<log_type, level_class, char_type, traits_type>
        my_type;

        /// A map of individual entities to logging levels.
        typedef
        MAP_TYPE<map_ptr_t, level_type>
        level_map;


    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    private:

        /// The default logging level for this logger.
        /**
         * This level applies to all logging requests sent anonymously or
         * by an entity that is not or whose type is not otherwise configured.
         */
        level_type _default;

        /// The map in which entities are mapped to logging levels.
        level_map _level_map;

        /// A mutex to protect the default logging level.
        EDDY_DECLARE_MUTABLE_MUTEX(_default_mutex)

        /// A mutex to protect the logging level map.
        EDDY_DECLARE_MUTABLE_MUTEX(_level_map_mutex)


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
            return this->log_if(
                this->get_default_level(), level_class::Default, e
                );

        } // log(ET)

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
            return this->log_if(this->get_default_level(), elevel, e);

        } // log(level_type, ET)

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
            return this->my_type::log_if(this->EDDY_TEMPLATE_KEYWORD_QUALIFIER
                get_logging_level<IssType>(), elevel, e
                );

        } // log(level_type, ET)

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
            return this->log_if(this->get_logging_level(issuer), elevel, e);

        } // log(level_type, IssType, ET)

        /// Logs e to the known log without checking levels.
        /**
         * The entry is passed along to the log regardless of
         * logging levels etc.  This is most useful when an explicit
         * check of logging levels is carried out prior to this call
         * in order that the entry creation may be avoided.  See the
         * macro interface for examples of this.
         *
         * \param e The entry to be logged to the known log.
         * \return True no matter what (return is only provided for
         *         consistancy with other logging methods).
         */
        template <typename ET>
        inline
        bool
        simple_log(
            const ET& e
            )
        {
            return base_type::log(e);

        } // simple_log(level_type, ET)

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
            return this->will_log(
                this->get_logging_level(&typeid(IssType), elevel)
                );

        } // will_log(level_type)

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
            return this->will_log(this->get_logging_level(issuer), elevel);

        } // will_log(IssType, level_type)

        /**
         * \brief Indicates whether an issuer using ilevel is allowed to log at
         *        elevel.
         *
         * \param ilevel The level at (or above) which the issuer is allowed
         *               to log.
         * \param elevel The level at which an entry was or would be issued.
         * \return True if ilevel >= elevel.
         */
        inline
        bool
        will_log(
            const level_type& ilevel,
            const level_type& elevel
            ) const
        {
            return ilevel <= elevel;

        } // will_log(level_type, level_type)

        /// Sets the logging level for the issuer to the supplied level.
        /**
         * If the logging level for issuer is already set, it will be changed
         * to the supplied elevel.
         *
         * \param issuer A pointer to the issuer to set the logging level for.
         * \param elevel The entry level at or above which issuer will hence be
         *               able to log.
         */
        template<typename IssType>
        void
        set_logging_level(
            const IssType* issuer,
            const level_type& elevel
            )
        {
            if(issuer != 0x0)
            {
                EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_level_map_mutex)
                this->_level_map[as_map_ptr(issuer)] = elevel;
                EDDY_LOGGING_CATCH_THREAD_EXCEPTION
            }
            else this->EDDY_TEMPLATE_KEYWORD_QUALIFIER
                    set_logging_level<IssType>(elevel);

        } // set_logging_level(IssType, level_type)

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
            this->set_logging_level(&typeid(IssType), ilevel);

        } // set_logging_level(level_type)

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
            return this->get_logging_level_for(&typeid(IssType));

        } // get_logging_level

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
        const level_type&
        get_logging_level(
            const IssType* issuer
            ) const
        {
            bool found = false;

            const level_type& lt = this->get_logging_level_for(
                as_map_ptr(issuer), found
                );

            return found ? lt : this->get_logging_level_for(&typeid(IssType));

        } // get_logging_level

        /// Returns the default logging level for this log gateway..
        /**
         * The default logging level is the level at (or above) which all
         * non-mapped entities and types are allowed to log.
         *
         * \return The default level used by this log gateway.
         */
        EDDY_NOTHREAD_INLINE
        const level_type&
        get_default_level(
            ) const
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_default_mutex)
            return this->_default;
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION

        } // get_default_level

        /// Allows mutation of the default logging level of this gateway.
        /**
         * \param l The new default level for this level gateway.
         */
        EDDY_NOTHREAD_INLINE
        void
        set_default_level(
            const level_type& l
            )
        {
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_default_mutex)
            this->_default = l;
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION

        } // set_default_level

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
        EDDY_NOTHREAD_INLINE
        bool
        is_mapped(
            const IssType* issuer
            ) const
        {
            // begin a try block and lock our mutex if thread safe is in use.
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_level_map_mutex)

            // Return true if issuer is in the list and false otherwise.
            return this->_level_map.empty() ? false :
                        this->_level_map.find(as_map_ptr(issuer)) !=
                        this->_level_map.end();

            // close the try block and handle any error if necessary
            // if thread safe is in use.
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION

        } // is_mapped(IssType)

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
            return this->is_mapped(&typeid(IssType));

        } // is_mapped(void)

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
         * \return True if issuer was successfully removed from the map. A
         *         return of false would generally mean that the issuer was
         *         not in the map to begin with.
         */
        template<typename IssType>
        EDDY_NOTHREAD_INLINE
        bool
        clear_logging_level(
            const IssType* issuer
            )
        {
            // begin a try block and lock our mutex if thread safe is in use.
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_level_map_mutex)

            // use erase to get issuer out of our map and return true if it
            // was removed and false if it wasn't found.
            return this->_level_map.empty() ? false :
                (this->_level_map.erase(as_map_ptr(issuer)) > 0);

            // close the try block and handle any error if necessary
            // if thread safe is in use.
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION

        } // clear_logging_level(IssType)

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
            return clear_logging_level(&typeid(IssType));

        } // clear_logging_level(void)

        /// Clears the map of types and objects to levels.
        /**
         * After this, any types or objects that had previously been given a
         * logging level using set_logging_level will no longer be mapped.
         * Subsequently, any calls to is_mapped will return false and all calls
         * to get_logging_level will return the default level of this gateway.
         */
        inline
        void
        clear_logging_map(
            )
        {
            this->_level_map.clear();
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
            return base_type::get_log();

        } // get_log

        /// Allows immutable access to the underlying log for this logger.
        /**
         * \return A constant reference to the log known by this gateway.
         */
        inline
        const log_type&
        get_log(
            ) const
        {
            return base_type::get_log();

        } // get_log, const

    /*
    ============================================================================
    Subclass Visible Methods
    ============================================================================
    */
    protected:

        /// Logs entry e if the levels allow it.
        /**
         * If the issuer level is sufficient to allow logging at the entry
         * level, then e is logged.
         *
         * \param ilevel The level at (or above) which the issuer is allowed
         *               to log.
         * \param elevel The level at which the entry was or would be issued.
         * \param e The actual entry to log based on the supplied levels.
         * \return True if the entry gets logged and false otherwise.
         */
        template <typename ET>
        inline
        bool
        log_if(
            const level_type& ilevel,
            const level_type& elevel,
            const ET& e
            )
        {
            // find out if we are going to log for this entry or not.
            return will_log(ilevel, elevel) ? base_type::log(e) : false;

        } // log_if

        /// Converts a pointer from the IssType to a map pointer type.
        /**
         * \param issuer The issuer pointer to convert to a map pointer type.
         * \return The issuer pointer statically casted to a map_ptr_t.
         */
        template <typename IssType>
        inline static
        map_ptr_t
        as_map_ptr(
            const IssType* issuer
            )
        {
            return static_cast<map_ptr_t>(issuer);

        } // as_map_ptr

        /// Returns the level at which \a issuer is permitted to log.
        /**
         * \a issuer may be a type_ptr_t in which case the logging level
         * for that type will be returned if found.  If instead issuer is an
         * instance of anything else, the logging level specific to that
         * instance will be returned if there is one. if there isn't, the
         * default logging level will be returned and \a found will be set to
         * false.  If a level is found for whatever \a issuer is, found will
         * be set to true.
         *
         * \param issuer A pointer to the instance to find in the level map.
         * \param found A boolean used to indicate whether or not a specific
         *        level was found for \a issuer.
         * \return The level at which issuer or type is allowed to log or the
         *         default level if no specific level is found.
         */
        const level_type&
        get_logging_level_for(
            map_ptr_t issuer,
            bool& found
            ) const
        {
            // Default found to false.  Set it true only if a specific level
            // is found.
            found = false;

            if(issuer != 0x0)
            {
                // lock the level map mutex for protected access.
                EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_level_map_mutex)

                // find our map_ptr_t issuer in the level map if we can.
                if(!this->_level_map.empty())
                {
                    typename level_map::const_iterator i(
                        this->_level_map.find(issuer)
                        );

                    // if the issuer was found, return the level associated.
                    if(found = (i!=this->_level_map.end())) return (*i).second;
                }

                // close the try block and handle any error (if thread safe).
                EDDY_LOGGING_CATCH_THREAD_EXCEPTION
            }

            // otherwise, return the logging level of the type which may
            // be default.
            return this->get_default_level();

        } // get_logging_level_for

        /**
         * \brief Returns the level at which items of type \a type are
         *        permitted to log.
         *
         * If no specific level is designated for items of type \a type, the
         * default level is returned.
         *
         * \param type A pointer to the type information object to determine
         *             the logging level of.
         * \return The logging level for type if found or the default level if
         *         not.
         */
        const level_type&
        get_logging_level_for(
            type_ptr_t type
            ) const
        {
            // begin a try block and lock our mutex if thread safe is in use.
            EDDY_LOGGING_TRY_SCOPEDLOCK(lock, this->_level_map_mutex)

            // find our map_ptr_t type in the level map if we can.
            if(!this->_level_map.empty())
            {
                typename level_map::const_iterator i(
                    this->_level_map.find(as_map_ptr(type))
                    );

                // if the type was found, return the level associated.
                // Otherwise return the default level.
                if(i != this->_level_map.end()) return (*i).second;
            }

            // if we make it here, we are returning the default level.
            return this->get_default_level();

            // close the try block and handle any error if necessary
            // if thread safe is in use.
            EDDY_LOGGING_CATCH_THREAD_EXCEPTION

        } // get_logging_level_for

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
        level_log_gateway(
            log_type& log,
            const level_type& def_level = level_class::Default
            ) :
                base_type(log),
                _default(def_level),
                _level_map() EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_default_mutex, PTHREAD_MUTEX_RECURSIVE)
                    EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_level_map_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
        } // level_log_gateway(log_type, level_type)

        /// Copy constructs a level_log_gateway.
        /**
         * \param copy The gateway to copy the properties from in this new
         *             gateway object.
         */
        level_log_gateway(
            const my_type& copy
            ) :
                base_type(copy),
                _default(copy._default),
                _level_map(copy._level_map) EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_default_mutex, PTHREAD_MUTEX_RECURSIVE)
                    EDDY_COMMA_IF_THREADSAFE
                EDDY_INIT_MUTEX(_level_map_mutex, PTHREAD_MUTEX_RECURSIVE)
        {
        } // level_log_gateway(my_type)

}; // class level_log_gateway




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
#endif // EDDY_LOGGING_LEVEL_LOG_MANAGER_HPP
