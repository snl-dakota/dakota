/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Configuration File Front End

    CONTENTS:

        Implementation of class ConfigFileFrontEndApp.

    NOTES:

        See notes of ConfigFileFrontEndApp.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.1.0

    CHANGES:

        Mon Oct 30 14:23:13 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ConfigFileFrontEndApp class.
 */





/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <map>
#include <memory>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <Evaluators/ExternalEvaluator.hpp>
#include <../Utilities/include/JEGATypes.hpp>
#include <../FrontEnd/Core/include/Driver.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/DesignMultiSet.hpp>
#include <../FrontEnd/Core/include/ConfigHelper.hpp>
#include <../FrontEnd/Core/include/ProblemConfig.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../FrontEnd/Core/include/AlgorithmConfig.hpp>
#include <../Utilities/include/BasicParameterDatabaseImpl.hpp>
#include <../FrontEnd/Core/include/GenericEvaluatorCreator.hpp>
#include <../FrontEnd/ConfigFile/include/ConfigFileFrontEndApp.hpp>

#include <boost/mpl/at.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/mpl/map/map40.hpp>
#include <boost/program_options.hpp>

#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/repetition/for.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;

using namespace boost;
using namespace boost::tuples;
using namespace boost::program_options;

using namespace JEGA;
using namespace JEGA::Logging;
using namespace JEGA::FrontEnd;
using namespace JEGA::Utilities;
using namespace JEGA::Algorithms;

namespace bt = boost::tuples;

/*
================================================================================
Options Map Macros
================================================================================
*/
/// A macro to resolve to the type tags used in the lookup map of option types.
/**
 * Currently, this is done using character arrays of varying size.
 *
 * \param tag The tag for the size value for the character array to be created.
 */
#define CFFE_CHVEC(tag) char[IXS::tag]

/// A macro to shorten the syntax of a retrieval from the all_opt_types map.
/**
 * \param tag The tag the type in the all_opt_types map.
 * \return Resolves to the type associated with the supplied index.
 */
#define CFFE_TYPEOF(tag) mpl::at<all_opt_types, CFFE_CHVEC(tag)>::type

/// A macro to retrieve a variable value from the variables map.
/**
 * This returns the actual reference to a value, not a
 * boost::program_options::variable_value.
 *
 * \param tag The tag used to get the index in the all_opt_types map and the
 *            string name of the option value of interest.
 * \return The value provided on the command line and/or in a config file for
 *         option identified by \a tag.
 */
#define CFFE_GETVAR(tag) GetValueOf<CFFE_TYPEOF(tag)>(ITS::tag)



/*
================================================================================
Pre-Namespace File Scope Typedefs
================================================================================
*/






/*
================================================================================
Global Utility Functions
================================================================================
*/

// This just might take the stupid cake.  It seems that in order for the vc80
// compiler to handle these stream insertion/extraction operators properly,
// they must be put in the std namespace.  Global scope does not work and
// neither does inside the JEGA namespaces.
namespace std {

/// Eats all leading whitespace off the supplied input stream and returns it.
/**
 * \param stream The stream from which to remove all leading whitespace.
 * \return The stream after all leading whitespace has been removed.
 */
inline
istream&
eat_white(
    istream& stream
    )
{
    EDDY_FUNC_DEBUGSCOPE
    while(!stream.eof() && ::isspace(stream.peek())) stream.ignore();
    return stream;
}

/// Replaces all occurances of "&lb" with the pound (#) character.
/**
 * \param in The string in which to do the replacing.
 * \return The supplied \a in string is returned for convenience.
 */
string&
process_pounds(
    string& in
    )
{
    static const char* lbstr = "&lb";
    string::size_type next(in.find(lbstr));

    while(next != string::npos) {
        in.replace(next, 3, "#");
        next = in.find(lbstr, next+1);
    }

    return in;
}

/// Reads a variable in from an input stream and stores it in \a into.
/**
 * The default implementation is to simply use the stream extraction operator
 * >> to read and store the value.  See any specializations provided for any
 * different behaviors.
 *
 * \param stream The input stream from which to read a value.
 * \param into The variable in which to store the read in value.
 * \return The supplied stream after extraction of the variable.
 */
template <typename T>
inline
istream&
read(
    istream& stream,
    T& into
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return stream >> into;
}

/// A specialization of the read template for std::strings.
/**
 * This specialization will account for the possibility that the string may
 * contains spaces if enclosed in quotation marks.  It also allows for the
 * inclusion of quotation marks if they are escaped (\").
 *
 * \param stream The input stream from which to read a value.
 * \param into The string in which to store the read in value.
 * \return The supplied stream after extraction of the string.
 */
template <>
istream&
read<string>(
    istream& stream,
    string& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

    eat_white(stream);

    // if the first char is a ", then read until the next ".  Otherwise,
    // do a regular string read.
    if(stream.peek() != '\"') stream >> into;

    else
    {
        // if we get here, we found a ".  Now we must read up to and pop off
        // the next ".  start by ignoring the current "
        stream.ignore();

        // now start reading.  Watch for the \" sequence which is a literal ".
        char last = '\0';
        while(!stream.eof())
        {
            char next = stream.peek();

            // if we found a " and it is not escaped, we are done.
            if(next == '\"' && last != '\\') { stream.ignore(); break; }

            // if we make it here and last is an escape, then we will put it in
            // since we only heed it in front of ".
            if(last == '\\') into.push_back(last);

            // no matter what at this point, we put in next which we will obtain
            // with a get to move past it.
            into.push_back(stream.get());

            // now update our last.
            last = next;
        }
    }

    // process any # indicators.
    process_pounds(into);

    return stream;
}

/// A specialization of the read template for bools.
/**
 * This specialization will read the input as a string (not using the read
 * specialization for strings, just using the std::operator >>) and then see
 * if it is a valid representation for a boolean.  Valid representations for
 * true include "t", "true", and "1" (case insensitive).  Valid representations
 * for false include "f", "false", and "0", also case insensitive.  If a
 * representation other than any of these is provided, then an exception is
 * thrown.
 *
 * \param stream The input stream from which to read a value.
 * \param into The boolean in which to store the read in value.
 * \return The supplied stream after extraction of the boolean.
 */
template <>
istream&
read<bool>(
    istream& stream,
    bool& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // do this the brute force way.
    string temp;
    stream >> temp;
    string orig(temp);
    transform(temp.begin(), temp.end(), temp.begin(), &::tolower);
    if     (temp == "t" || temp == "true"  || temp == "1") into = true;
    else if(temp == "f" || temp == "false" || temp == "0") into = false;
    else throw runtime_error(
        "Unrecognized input for boolean \"" + orig + "\"."
        );
    return stream;
}

/// Writes a variable into a supplied output stream.
/**
 * The default implementation is to simply use the stream insertion operator
 * << to write the value.  See any specializations provided for any
 * different behaviors.
 *
 * \param stream The output stream into which to write a value.
 * \param t The value to write into the \a stream.
 * \return The supplied stream after insertion of the variable.
 */
template <typename T>
inline
ostream&
write(
      ostream& stream,
      const T& t
      )
{
    EDDY_FUNC_DEBUGSCOPE
    return stream << t;
}

/// A specialization of the write template for strings.
/**
 * This specialization writes the string wrapped in quotation marks and any
 * quotation marks that are already in the string are escaped.
 *
 * \param stream The output stream into which to write a string.
 * \param str The string to write into the \a stream.
 * \return The supplied stream after insertion of the string.
 */
template<>
ostream&
write<string>(
    ostream& stream,
    const string& str
    )
{
    EDDY_FUNC_DEBUGSCOPE
    stream << '\"';
    // replace all " with \" and write it out wrapped in quotes.
    for(string::const_iterator it(str.begin()); it!=str.end(); ++it) {
        if(*it == '\"') stream << '\\';
        stream << *it;
    }

    return stream << '\"';
}

/// A specialization of the write template for bools.
/**
 * This specialization writes bools as alpha (boolalpha) but returns the stream
 * to non-boolalpha if that is what it was prior to this call.
 *
 * \param stream The output stream into which to write a bool.
 * \param str The bool to write into the \a stream.
 * \return The supplied stream after insertion of the bool.
 */
template<>
ostream&
write<bool>(
    ostream& stream,
    const bool& str
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // see if the supplied stream is boolalpha.
    bool isba = (stream.flags() & ios_base::boolalpha) != 0;

    // if it is not, make it for now.
    if(!isba) boolalpha(stream);

    // now write our data.
    stream << str;

    // now set it back to noboolalpha if appropriate.
    if(!isba) noboolalpha(stream);

    // now return our stream
    return stream;
}

/**
 * \brief A predicate for preprocessor enumerations that progress from some
 *        lower bound integral value to an upper bound such as when using
 *        BOOST_PP_FOR with a tuple.
 *
 * \param r Not Used.
 * \param state The current enumeration value.
 */
#define ENUM_PRED(r, state) \
   BOOST_PP_NOT_EQUAL( \
      BOOST_PP_TUPLE_ELEM(2, 0, state), \
      BOOST_PP_TUPLE_ELEM(2, 1, state) \
   )

/**
 * \brief An operator for preprocessor enumerations that progress from some
 *        lower bound integral value to an upper bound such as when using
 *        BOOST_PP_FOR with a tuple.
 *
 * \param r Not Used.
 * \param state The current enumeration value.
 */
#define ENUM_OP(r, state) \
   ( \
      BOOST_PP_INC(BOOST_PP_TUPLE_ELEM(2, 0, state)), \
      BOOST_PP_TUPLE_ELEM(2, 1, state) \
   )

#define WRITE(r, state) \
    write(stream, p.get< BOOST_PP_TUPLE_ELEM(2, 0, state) >()) \
    BOOST_PP_IF(BOOST_PP_EQUAL( \
        BOOST_PP_INC(BOOST_PP_TUPLE_ELEM(2, 0, state)), \
        BOOST_PP_TUPLE_ELEM(2, 1, state) \
        ), ;, << " "; )

#define READ(r, state) \
    read (stream, p.get< BOOST_PP_TUPLE_ELEM(2, 0, state) >()) \
    BOOST_PP_IF(BOOST_PP_EQUAL( \
        BOOST_PP_INC(BOOST_PP_TUPLE_ELEM(2, 0, state)), \
        BOOST_PP_TUPLE_ELEM(2, 1, state) \
        ), ;, >> eat_white; )

#define TUPLE_STREAM_INSERT(N) \
    template < BOOST_PP_ENUM_PARAMS(N, typename T) > \
    inline ostream& operator << ( \
        ostream& stream, \
        const bt::tuple< BOOST_PP_ENUM_PARAMS(N, T) >& p \
        ) { \
        EDDY_FUNC_DEBUGSCOPE \
        BOOST_PP_FOR((0, N), ENUM_PRED, ENUM_OP, WRITE) \
        return stream; \
    }

#define TUPLE_STREAM_EXTRACT(N) \
    template < BOOST_PP_ENUM_PARAMS(N, typename T) > \
    inline istream& operator >> ( \
        istream& stream, \
        bt::tuple< BOOST_PP_ENUM_PARAMS(N, T) >& p \
        ) { \
        EDDY_FUNC_DEBUGSCOPE \
        BOOST_PP_FOR((0, N), ENUM_PRED, ENUM_OP, READ) \
        return stream; \
    }

/// The stream insertion for a 5-tuple
TUPLE_STREAM_INSERT(5)
/// The stream insertion for a 4-tuple
TUPLE_STREAM_INSERT(4)
/// The stream insertion for a 3-tuple
TUPLE_STREAM_INSERT(3)
/// The stream insertion for a 2-tuple
TUPLE_STREAM_INSERT(2)

/// The stream extraction for a 5-tuple
TUPLE_STREAM_EXTRACT(5)
/// The stream extraction for a 4-tuple
TUPLE_STREAM_EXTRACT(4)
/// The stream extraction for a 3-tuple
TUPLE_STREAM_EXTRACT(3)
/// The stream extraction for a 2-tuple
TUPLE_STREAM_EXTRACT(2)

// Keep the pred and op for further use.
//#undef ENUM_PRED
//#undef ENUM_OP
#undef WRITE
#undef READ
#undef TUPLE_STREAM_INSERT
#undef TUPLE_STREAM_EXTRACT

/**
 * \brief This stream insertion operator writes each element of a vector of
 *        T on a separate line.
 *
 * \param stream The stream into which to write the vector elements.
 * \param vec The vector whose elements are to be written into the stream.
 */
template <typename T>
ostream&
operator << (
    ostream& stream,
    const vector<T>& vec
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(vec.empty()) return stream;
    write(stream, *vec.begin());
    for(typename vector<T>::const_iterator it(++vec.begin());
        it!=vec.end(); ++it) write(stream << '\n', *it);
    return stream;
}

/**
 * \brief This stream insertion operator reads each element of a vector of T.
 *
 * The elements must be delimited by whitespace.
 *
 * \param stream The stream from which to read the vector elements.
 * \param vec The vector into which to store the read elements.
 */
template <typename T>
istream&
operator >> (
    istream& stream,
    vector<T>& vec
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(stream.eof()) return stream;
    for(T temp; !stream.eof(); eat_white(stream)) {
        if(!(read(stream, temp))) return stream;
        vec.push_back(temp);
    }
    return stream;
}

} // namespace std


/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {
        namespace ConfigFile {



class ConfigFileFrontEndApp::InputDescriptors
{
    /*
    ============================================================================
    Nested Inner Class Forward Declares
    ============================================================================
    */
    public:

        class Indices;
        class InputTags;
        class DefaultValues;

}; // class ConfigFileFrontEndApp::InputDescriptors

/// A shorthand for the input description type map indices.
typedef ConfigFileFrontEndApp::InputDescriptors::Indices IXS;

/// A shorthand for the input description name tags.
typedef ConfigFileFrontEndApp::InputDescriptors::InputTags ITS;

/// A shorthand for the default values of the inputs.
typedef ConfigFileFrontEndApp::InputDescriptors::DefaultValues DVS;

class ConfigFileFrontEndApp::InputDescriptors::Indices
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        /**
         * \brief A general purpose constant for the (very common) case where
         *        the input is a simple, single string.
         */
        static const char _plainString;

        /// The constant used to identify the configuation filename input.
        static const char _cfgFile;

        /// The constant used to identify the help request input.
        static const char _help;

        /// The constant used to identify the version request input.
        static const char _version;

        static const char _assessor     ;
        static const char _initializer  ;
        static const char _mutator      ;
        static const char _crosser      ;
        static const char _nicher       ;
        static const char _postprocessor;
        static const char _mainloop     ;
        static const char _selector     ;
        static const char _converger    ;

        static const char _intParams    ;
        static const char _dblParams    ;
        static const char _sztParams    ;
        static const char _boolParams   ;
        static const char _strParams    ;
        static const char _intVecParams ;
        static const char _dblVecParams ;
        static const char _strVecParams ;
        static const char _dblMatParams ;

        static const char _rndSeed      ;
        static const char _globLogFile  ;
        static const char _globLogLevel ;
        static const char _algType      ;

        static const char _contRealVar  ;
        static const char _discRealVar  ;
        static const char _contIntVar   ;
        static const char _discIntVar   ;
        static const char _boolVar      ;
        static const char _varBounds    ;
        static const char _varDiscVals  ;

        static const char _linMinObj        ;
        static const char _nonLinMinObj     ;
        static const char _linMaxObj        ;
        static const char _nonLinMaxObj     ;
        static const char _linSkValObj      ;
        static const char _nonLinSkValObj   ;
        static const char _linSkRngObj      ;
        static const char _nonLinSkRngObj   ;

        static const char _linInEqCon           ;
        static const char _nonLinInEqCon        ;
        static const char _linEqCon             ;
        static const char _nonLinEqCon          ;
        static const char _linTwoSideIneqCon    ;
        static const char _nonLinTwoSideIneqCon ;
        static const char _linNotEqCon          ;
        static const char _nonLinNotEqCon       ;


}; // class ConfigFileFrontEndApp::InputDescriptors::Indices

const char IXS::_plainString (0);

const char IXS::_cfgFile    (IXS::_plainString);
const char IXS::_help       (IXS::_plainString);
const char IXS::_version    (IXS::_plainString);

const char IXS::_assessor       (IXS::_plainString);
const char IXS::_initializer    (IXS::_plainString);
const char IXS::_mutator        (IXS::_plainString);
const char IXS::_crosser        (IXS::_plainString);
const char IXS::_nicher         (IXS::_plainString);
const char IXS::_postprocessor  (IXS::_plainString);
const char IXS::_selector       (IXS::_plainString);
const char IXS::_mainloop       (IXS::_plainString);
const char IXS::_converger      (IXS::_plainString);

const char IXS::_intParams      (IXS::_plainString  + 1);
const char IXS::_dblParams      (IXS::_intParams    + 1);
const char IXS::_sztParams      (IXS::_dblParams    + 1);
const char IXS::_boolParams     (IXS::_sztParams    + 1);
const char IXS::_strParams      (IXS::_boolParams   + 1);
const char IXS::_intVecParams   (IXS::_strParams    + 1);
const char IXS::_dblVecParams   (IXS::_intVecParams + 1);
const char IXS::_strVecParams   (IXS::_dblVecParams + 1);
const char IXS::_dblMatParams   (IXS::_strVecParams + 1);

const char IXS::_rndSeed        (IXS::_dblMatParams + 1 );
const char IXS::_globLogFile    (IXS::_plainString      );
const char IXS::_globLogLevel   (IXS::_plainString      );
const char IXS::_algType        (IXS::_plainString      );

const char IXS::_contRealVar    (IXS::_rndSeed      + 1);
const char IXS::_discRealVar    (IXS::_contRealVar  + 1);
const char IXS::_contIntVar     (IXS::_discRealVar  + 1);
const char IXS::_discIntVar     (IXS::_contIntVar   + 1);
const char IXS::_boolVar        (IXS::_discIntVar   + 1);

const char IXS::_linMinObj      (IXS::_boolVar          + 1);
const char IXS::_nonLinMinObj   (IXS::_linMinObj        + 1);
const char IXS::_linMaxObj      (IXS::_nonLinMinObj     + 1);
const char IXS::_nonLinMaxObj   (IXS::_linMaxObj        + 1);
const char IXS::_linSkValObj    (IXS::_nonLinMaxObj     + 1);
const char IXS::_nonLinSkValObj (IXS::_linSkValObj      + 1);
const char IXS::_linSkRngObj    (IXS::_nonLinSkValObj   + 1);
const char IXS::_nonLinSkRngObj (IXS::_linSkRngObj      + 1);

const char IXS::_linInEqCon           (IXS::_nonLinSkRngObj       + 1);
const char IXS::_nonLinInEqCon        (IXS::_linInEqCon           + 1);
const char IXS::_linEqCon             (IXS::_nonLinInEqCon        + 1);
const char IXS::_nonLinEqCon          (IXS::_linEqCon             + 1);
const char IXS::_linTwoSideIneqCon    (IXS::_nonLinEqCon          + 1);
const char IXS::_nonLinTwoSideIneqCon (IXS::_linTwoSideIneqCon    + 1);
const char IXS::_linNotEqCon          (IXS::_nonLinTwoSideIneqCon + 1);
const char IXS::_nonLinNotEqCon       (IXS::_linNotEqCon          + 1);

/**
 *\brief The map that stores input option types keyed on character arrays of
 *       the size specified by the index tag of the option.
 */
typedef mpl::map32<

    // A reusable type entry for all inputs that are simple, unnamed strings.
    mpl::pair<CFFE_CHVEC(_plainString),
        // string - The string value of the corresponding input.
        string
        >,

    // An entry for all inputs that are named integral parameters.
    mpl::pair<CFFE_CHVEC(_intParams),
        // string - The name of the integer parameter
        // int - The value of the integer parameter
        vector<bt::tuple<string, int> >
        >,

    // An entry for all inputs that are named double parameters.
    mpl::pair<CFFE_CHVEC(_dblParams),
        // string - The name of the double parameter
        // double - The value of the double parameter
        vector<bt::tuple<string, double> >
        >,

    // An entry for all inputs that are named size-type parameters.
    mpl::pair<CFFE_CHVEC(_sztParams),
        // string - The name of the size_t parameter
        // size_t - The value of the size_t parameter
        vector<bt::tuple<string, size_t> >
        >,

    // An entry for all inputs that are named boolean parameters.
    mpl::pair<CFFE_CHVEC(_boolParams),
        // string - The name of the boolean parameter
        // bool - The value of the boolean parameter
        vector<bt::tuple<string, bool> >
        >,                                                                // 5

    // An entry for all inputs that are named string parameters.
    mpl::pair<CFFE_CHVEC(_strParams),
        // string - The name of the string parameter
        // string - The value of the string parameter
        vector<bt::tuple<string, string> >
        >,

    // An entry for all inputs that are named integer vector parameters.
    mpl::pair<CFFE_CHVEC(_intVecParams),
        // string - The name of the int vector parameter
        // IntVector - The vector of values of the int vector parameter
        vector<bt::tuple<string, IntVector> >
        >,

    // An entry for all inputs that are named double vector parameters.
    mpl::pair<CFFE_CHVEC(_dblVecParams),
        // string - The name of the double vector parameter
        // DoubleVector - The vector of values of the double vector parameter
        vector<bt::tuple<string, DoubleVector> >
        >,

    // An entry for all inputs that are named string vector parameters.
    mpl::pair<CFFE_CHVEC(_strVecParams),
        // string - The name of the string vector parameter
        // StringVector - The vector of values of the string vector parameter
        vector<bt::tuple<string, StringVector> >
        >,

    // An entry for all inputs that are named double matrix parameters.
    mpl::pair<CFFE_CHVEC(_dblMatParams),
        // string - The name of the double matrix parameter for which this is
        //          a row
        // DoubleVector - The vector of values of the row of this double matrix
        //                parameter
        vector<bt::tuple<string, DoubleVector> >
        >,                                                                // 10

    // An entry for the type of the random seed input.
    mpl::pair<CFFE_CHVEC(_rndSeed),
        // size_t - The value for the random number generator seed.
        size_t
        >,

    // An entry for all inputs that describe a continuous real variable.
    mpl::pair<CFFE_CHVEC(_contRealVar),
        // size_t - The zero based index of this variable.
        // string - The label for the new variable.
        // double - The lower bound on the range of this variable.
        // double - The upper bound on the range of this variable.
        // int - The desired decimal precision of this real variable.
        vector<bt::tuple<size_t, string, double, double, int> >
        >,

    // An entry for all inputs that describe a discrete real variable.
    mpl::pair<CFFE_CHVEC(_discRealVar),
        // size_t - The zero based index of this variable.
        // string - The label for the new variable.
        // DoubleVector - The vector of discrete values for this variable.
        vector<bt::tuple<size_t, string, DoubleVector> >
        >,

    // An entry for all inputs that describe a continuous integer variable.
    mpl::pair<CFFE_CHVEC(_contIntVar),
        // size_t - The zero based index of this variable.
        // string - The label for the new variable.
        // int - The lower bound on the range of this variable.
        // int - The upper bound on the range of this variable.
        vector<bt::tuple<size_t, string, int, int> >
        >,

    // An entry for all inputs that describe a discrete integer variable.
    mpl::pair<CFFE_CHVEC(_discIntVar),
        // size_t - The zero based index of this variable.
        // string - The label for the new variable.
        // IntVector - The vector of discrete values for this variable.
        vector<bt::tuple<size_t, string, IntVector> >
        >,                                                                // 15

    // An entry for all inputs that describe a boolean variable.
    mpl::pair<CFFE_CHVEC(_boolVar),
        // size_t - The zero based index of this variable.
        // string - The label for the new variable.
        vector<bt::tuple<size_t, string> >
        >,

    // An entry for all inputs that describe a linear minimization objective.
    mpl::pair<CFFE_CHVEC(_linMinObj),
        // size_t - The zero based index of this objective.
        // string - The label for the new objective.
        // DoubleVector - The vector of variable coefficients used to evaluate
        //                this linear function.
        vector<bt::tuple<size_t, string, DoubleVector> >
        >,

    // An entry for all inputs that describe a nonlinear minimization
    // objective.
    mpl::pair<CFFE_CHVEC(_nonLinMinObj),
        // size_t - The zero based index of this objective.
        // string - The label for the new objective.
        vector<bt::tuple<size_t, string> >
        >,

    // An entry for all inputs that describe a linear maximization objective.
    mpl::pair<CFFE_CHVEC(_linMaxObj),
        // size_t - The zero based index of this objective.
        // string - The label for the new objective.
        // DoubleVector - The vector of variable coefficients used to evaluate
        //                this linear function.
        vector<bt::tuple<size_t, string, DoubleVector> >
        >,

    // An entry for all inputs that describe a nonlinear maximization
    // objective.
    mpl::pair<CFFE_CHVEC(_nonLinMaxObj),
        // size_t - The zero based index of this objective.
        // string - The label for the new objective.
        vector<bt::tuple<size_t, string> >
        >,                                                                // 20

    // An entry for all inputs that describe a linear seek value objective.
    mpl::pair<CFFE_CHVEC(_linSkValObj),
        // size_t - The zero based index of this objective.
        // string - The label for the new objective.
        // double - The value being sought by the objective.
        // DoubleVector - The vector of variable coefficients used to evaluate
        //                this linear function.
        vector<bt::tuple<size_t, string, double, DoubleVector> >
        >,

    // An entry for all inputs that describe a nonlinear seek value objective.
    mpl::pair<CFFE_CHVEC(_nonLinSkValObj),
        // size_t - The zero based index of this objective.
        // string - The label for the new objective.
        // double - The value being sought by the objective.
        vector<bt::tuple<size_t, string, double> >
        >,

    // An entry for all inputs that describe a linear seek range objective.
    mpl::pair<CFFE_CHVEC(_linSkRngObj),
        // size_t - The zero based index of this objective.
        // string - The label for the new objective.
        // double - The lower bound on the range sought by this objective.
        // double - The upper bound on the range sought by this objective.
        // DoubleVector - The vector of variable coefficients used to evaluate
        //                this linear function.
        vector<bt::tuple<size_t, string, double, double, DoubleVector> >
        >,

    // An entry for all inputs that describe a nonlinear seek range objective.
    mpl::pair<CFFE_CHVEC(_nonLinSkRngObj),
        // size_t - The zero based index of this objective.
        // string - The label for the new objective.
        // double - The lower bound on the range sought by this objective.
        // double - The upper bound on the range sought by this objective.
        vector<bt::tuple<size_t, string, double, double> >
        >,

    // An entry for all inputs that describe a linear inequality constraint.
    mpl::pair<CFFE_CHVEC(_linInEqCon),
        // size_t - The zero based index of this constraint.
        // string - The label for the new constraint.
        // double - The upper limiting value for this constraint function.
        // DoubleVector - The vector of variable coefficients used to evaluate
        //                this linear function.
        vector<bt::tuple<size_t, string, double, DoubleVector> >
        >,                                                                // 25

    // An entry for all inputs that describe a nonlinear inequality constraint.
    mpl::pair<CFFE_CHVEC(_nonLinInEqCon),
        // size_t - The zero based index of this constraint.
        // string - The label for the new constraint.
        // double - The upper limiting value for this constraint function.
        vector<bt::tuple<size_t, string, double> >
        >,

    // An entry for all inputs that describe a linear equality constraint.
    mpl::pair<CFFE_CHVEC(_linEqCon),
        // size_t - The zero based index of this constraint.
        // string - The label for the new constraint.
        // double - The target value for this constraint function.
        // double - The allowable violation from target value for this
        //          constraint function.
        // DoubleVector - The vector of variable coefficients used to evaluate
        //                this linear function.
        vector<bt::tuple<size_t, string, double, double, DoubleVector> >
        >,

    // An entry for all inputs that describe a nonlinear equality constraint.
    mpl::pair<CFFE_CHVEC(_nonLinEqCon),
        // size_t - The zero based index of this constraint.
        // string - The label for the new constraint.
        // double - The target value for this constraint function.
        // double - The allowable violation from target value for this
        //          constraint function.
        vector<bt::tuple<size_t, string, double, double> >
        >,

    // An entry for all inputs that describe a linear 2-sided inequality
    // constraint.
    mpl::pair<CFFE_CHVEC(_linTwoSideIneqCon),
        // size_t - The zero based index of this constraint.
        // string - The label for the new constraint.
        // double - The lower limiting value for this constraint function.
        // double - The upper limiting value for this constraint function.
        // DoubleVector - The vector of variable coefficients used to evaluate
        //                this linear function.
        vector<bt::tuple<size_t, string, double, double, DoubleVector> >
        >,

    // An entry for all inputs that describe a nonlinear 2-sided inequality
    // constraint.
    mpl::pair<CFFE_CHVEC(_nonLinTwoSideIneqCon),
        // size_t - The zero based index of this constraint.
        // string - The label for the new constraint.
        // double - The lower limiting value for this constraint function.
        // double - The upper limiting value for this constraint function.
        vector<bt::tuple<size_t, string, double, double> >
        >,                                                                // 30

    // An entry for all inputs that describe a linear not-equality constraint.
    mpl::pair<CFFE_CHVEC(_linNotEqCon),
        // size_t - The zero based index of this constraint.
        // string - The label for the new constraint.
        // double - The taboo value for this constraint function.
        // DoubleVector - The vector of variable coefficients used to evaluate
        //                this linear function.
        vector<bt::tuple<size_t, string, double, DoubleVector> >
        >,

    // An entry for all inputs that describe a nonlinear not-equality
    // constraint.
    mpl::pair<CFFE_CHVEC(_nonLinNotEqCon),
        // size_t - The zero based index of this constraint.
        // string - The label for the new constraint.
        // double - The taboo value for this constraint function.
        vector<bt::tuple<size_t, string, double> >
        >
>
all_opt_types;


class ConfigFileFrontEndApp::InputDescriptors::InputTags
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        static const string _cfgFile;
        static const string _help   ;
        static const string _version;

        static const string _assessor     ;
        static const string _initializer  ;
        static const string _mutator      ;
        static const string _crosser      ;
        static const string _nicher       ;
        static const string _postprocessor;
        static const string _mainloop     ;
        static const string _selector     ;
        static const string _converger    ;

        static const string _intParams   ;
        static const string _dblParams   ;
        static const string _sztParams   ;
        static const string _boolParams  ;
        static const string _strParams   ;
        static const string _intVecParams;
        static const string _dblVecParams;
        static const string _strVecParams;
        static const string _dblMatParams;

        static const string _rndSeed     ;
        static const string _globLogFile ;
        static const string _globLogLevel;
        static const string _algType     ;

        static const string _contRealVar ;
        static const string _discRealVar ;
        static const string _contIntVar  ;
        static const string _discIntVar  ;
        static const string _boolVar     ;
        static const string _varBounds   ;
        static const string _varDiscVals ;

        static const string _linMinObj     ;
        static const string _nonLinMinObj  ;
        static const string _linMaxObj     ;
        static const string _nonLinMaxObj  ;
        static const string _linSkValObj   ;
        static const string _nonLinSkValObj;
        static const string _linSkRngObj   ;
        static const string _nonLinSkRngObj;

        static const string _linInEqCon          ;
        static const string _nonLinInEqCon       ;
        static const string _linEqCon            ;
        static const string _nonLinEqCon         ;
        static const string _linTwoSideIneqCon   ;
        static const string _nonLinTwoSideIneqCon;
        static const string _linNotEqCon         ;
        static const string _nonLinNotEqCon      ;

        static const string _dummyTag;

}; // class ConfigFileFrontEndApp::InputDescriptors::InputTags

class ConfigFileFrontEndApp::InputDescriptors::DefaultValues
{
    /*
    ============================================================================
    Member Data Declarations
    ============================================================================
    */
    public:

        static const CFFE_TYPEOF(_cfgFile       ) _cfgFile  ;
        static const CFFE_TYPEOF(_help          ) _help     ;
        static const CFFE_TYPEOF(_version       ) _version  ;

        static const CFFE_TYPEOF(_assessor      ) _assessor     ;
        static const CFFE_TYPEOF(_initializer   ) _initializer  ;
        static const CFFE_TYPEOF(_mutator       ) _mutator      ;
        static const CFFE_TYPEOF(_crosser       ) _crosser      ;
        static const CFFE_TYPEOF(_nicher        ) _nicher       ;
        static const CFFE_TYPEOF(_postprocessor ) _postprocessor;
        static const CFFE_TYPEOF(_mainloop      ) _mainloop     ;
        static const CFFE_TYPEOF(_selector      ) _selector     ;
        static const CFFE_TYPEOF(_converger     ) _converger    ;

        static const CFFE_TYPEOF(_intParams     ) _intParams   ;
        static const CFFE_TYPEOF(_dblParams     ) _dblParams   ;
        static const CFFE_TYPEOF(_sztParams     ) _sztParams   ;
        static const CFFE_TYPEOF(_boolParams    ) _boolParams  ;
        static const CFFE_TYPEOF(_strParams     ) _strParams   ;
        static const CFFE_TYPEOF(_intVecParams  ) _intVecParams;
        static const CFFE_TYPEOF(_dblVecParams  ) _dblVecParams;
        static const CFFE_TYPEOF(_strVecParams  ) _strVecParams;
        static const CFFE_TYPEOF(_dblMatParams  ) _dblMatParams;

        static const CFFE_TYPEOF(_rndSeed       ) _rndSeed     ;
        static const CFFE_TYPEOF(_globLogFile   ) _globLogFile ;
        static const CFFE_TYPEOF(_globLogLevel  ) _globLogLevel;
        static const CFFE_TYPEOF(_algType       ) _algType     ;

        static const CFFE_TYPEOF(_contRealVar   ) _contRealVar;
        static const CFFE_TYPEOF(_discRealVar   ) _discRealVar;
        static const CFFE_TYPEOF(_contIntVar    ) _contIntVar ;
        static const CFFE_TYPEOF(_discIntVar    ) _discIntVar ;
        static const CFFE_TYPEOF(_boolVar       ) _boolVar    ;

        static const CFFE_TYPEOF(_linMinObj     ) _linMinObj     ;
        static const CFFE_TYPEOF(_nonLinMinObj  ) _nonLinMinObj  ;
        static const CFFE_TYPEOF(_linMaxObj     ) _linMaxObj     ;
        static const CFFE_TYPEOF(_nonLinMaxObj  ) _nonLinMaxObj  ;
        static const CFFE_TYPEOF(_linSkValObj   ) _linSkValObj   ;
        static const CFFE_TYPEOF(_nonLinSkValObj) _nonLinSkValObj;
        static const CFFE_TYPEOF(_linSkRngObj   ) _linSkRngObj   ;
        static const CFFE_TYPEOF(_nonLinSkRngObj) _nonLinSkRngObj;

        static const CFFE_TYPEOF(_linInEqCon          ) _linInEqCon          ;
        static const CFFE_TYPEOF(_nonLinInEqCon       ) _nonLinInEqCon       ;
        static const CFFE_TYPEOF(_linEqCon            ) _linEqCon            ;
        static const CFFE_TYPEOF(_nonLinEqCon         ) _nonLinEqCon         ;
        static const CFFE_TYPEOF(_linTwoSideIneqCon   ) _linTwoSideIneqCon   ;
        static const CFFE_TYPEOF(_nonLinTwoSideIneqCon) _nonLinTwoSideIneqCon;
        static const CFFE_TYPEOF(_linNotEqCon         ) _linNotEqCon         ;
        static const CFFE_TYPEOF(_nonLinNotEqCon      ) _nonLinNotEqCon      ;

}; // class ConfigFileFrontEndApp::InputDescriptors::DefaultValues







/*
================================================================================
Static Member Data Definitions
================================================================================
*/
const string ITS::_cfgFile       ("cfgfile"  );
const string ITS::_help          ("help"     );
const string ITS::_version       ("version"  );

const string ITS::_assessor      ("FITNESS_ASSESSOR");
const string ITS::_initializer   ("INITIALIZER"     );
const string ITS::_mutator       ("MUTATOR"         );
const string ITS::_crosser       ("CROSSER"         );
const string ITS::_nicher        ("NICHER"          );
const string ITS::_postprocessor ("POST_PROCESSOR"  );
const string ITS::_mainloop      ("MAIN_LOOP"       );
const string ITS::_selector      ("SELECTOR"        );
const string ITS::_converger     ("CONVERGER"       );

const string ITS::_intParams     ("INT_PARAMS"       );
const string ITS::_dblParams     ("DBL_PARAMS"       );
const string ITS::_sztParams     ("SIZE_T_PARAMS"    );
const string ITS::_boolParams    ("BOOL_PARAMS"      );
const string ITS::_strParams     ("STRING_PARAMS"    );
const string ITS::_intVecParams  ("INT_VEC_PARAMS"   );
const string ITS::_dblVecParams  ("DBL_VEC_PARAMS"   );
const string ITS::_strVecParams  ("STRING_VEC_PARAMS");
const string ITS::_dblMatParams  ("DBL_MAT_PARAMS"   );

const string ITS::_rndSeed       ("RANDOM_SEED"     );
const string ITS::_globLogFile   ("GLOBAL_LOG_FILE" );
const string ITS::_globLogLevel  ("GLOBAL_LOG_LEVEL");
const string ITS::_algType       ("ALGORITHM_TYPE"  );

const string ITS::_contRealVar   ("CONT_REAL_VAR");
const string ITS::_discRealVar   ("DISC_REAL_VAR");
const string ITS::_contIntVar    ("CONT_INT_VAR" );
const string ITS::_discIntVar    ("DISC_INT_VAR" );
const string ITS::_boolVar       ("BOOL_VAR"     );

const string ITS::_linMinObj     ("LIN_MIN_OBJ"      );
const string ITS::_nonLinMinObj  ("NONLIN_MIN_OBJ"   );
const string ITS::_linMaxObj     ("LIN_MAX_OBJ"      );
const string ITS::_nonLinMaxObj  ("NONLIN_MAX_OBJ"   );
const string ITS::_linSkValObj   ("LIN_SK_VAL_OBJ"   );
const string ITS::_nonLinSkValObj("NONLIN_SK_VAL_OBJ");
const string ITS::_linSkRngObj   ("LIN_SK_RNG_OBJ"   );
const string ITS::_nonLinSkRngObj("NONLIN_SK_RNG_OBJ");

const string ITS::_linInEqCon          ("LIN_INEQ_CON"          );
const string ITS::_nonLinInEqCon       ("NONLIN_INEQ_CON"       );
const string ITS::_linEqCon            ("LIN_EQ_CON"            );
const string ITS::_nonLinEqCon         ("NONLIN_EQ_CON"         );
const string ITS::_linTwoSideIneqCon   ("LIN_2_SIDE_INEQ_CON"   );
const string ITS::_nonLinTwoSideIneqCon("NONLIN_2_SIDE_INEQ_CON");
const string ITS::_linNotEqCon         ("LIN_NOT_EQ_CON"        );
const string ITS::_nonLinNotEqCon      ("NONLIN_NOT_EQ_CON"     );

const string ITS::_dummyTag      ("DUMMY");



const CFFE_TYPEOF(_cfgFile) DVS::_cfgFile ("RUNJEGA");
const CFFE_TYPEOF(_help   ) DVS::_help    (""       );

const CFFE_TYPEOF(_assessor)
    DVS::_assessor("domination_count");

const CFFE_TYPEOF(_initializer)
    DVS::_initializer("unique_random");

const CFFE_TYPEOF(_mutator)
    DVS::_mutator("replace_uniform");

const CFFE_TYPEOF(_crosser)
    DVS::_crosser("shuffle_random");

const CFFE_TYPEOF(_nicher)
    DVS::_nicher("distance");

const CFFE_TYPEOF(_postprocessor)
    DVS::_postprocessor("null_postprocessor");

const CFFE_TYPEOF(_mainloop)
    DVS::_mainloop("duplicate_free");

const CFFE_TYPEOF(_selector)
    DVS::_selector("below_limit");

const CFFE_TYPEOF(_converger)
    DVS::_converger("metric_tracker");

const CFFE_TYPEOF(_intParams)
    DVS::_intParams(1, bt::make_tuple(ITS::_dummyTag, 0));

const CFFE_TYPEOF(_dblParams)
    DVS::_dblParams (1, bt::make_tuple(ITS::_dummyTag, 0.0));

const CFFE_TYPEOF(_sztParams)
    DVS::_sztParams (1, bt::make_tuple(ITS::_dummyTag, 0));

const CFFE_TYPEOF(_boolParams)
    DVS::_boolParams(1, bt::make_tuple(ITS::_dummyTag, false));

const CFFE_TYPEOF(_strParams)
    DVS::_strParams (1, bt::make_tuple(ITS::_dummyTag, string()));

const CFFE_TYPEOF(_intVecParams)
    DVS::_intVecParams(1, bt::make_tuple(ITS::_dummyTag, IntVector(1, 0)));

const CFFE_TYPEOF(_dblVecParams)
    DVS::_dblVecParams(1, bt::make_tuple(ITS::_dummyTag, DoubleVector(1, 0.0)));

const CFFE_TYPEOF(_strVecParams)
    DVS::_strVecParams(1,
        bt::make_tuple(ITS::_dummyTag, StringVector(1, ITS::_dummyTag))
        );

const CFFE_TYPEOF(_dblMatParams)
    DVS::_dblMatParams(1, bt::make_tuple(ITS::_dummyTag, DoubleVector(1, 0.0)));

const CFFE_TYPEOF(_rndSeed)
    DVS::_rndSeed     (0);

const CFFE_TYPEOF(_globLogFile)
    DVS::_globLogFile ("JEGAGlobal.log");

const CFFE_TYPEOF(_globLogLevel)
    DVS::_globLogLevel("Default");

const CFFE_TYPEOF(_algType)
    DVS::_algType     ("MOGA");

const CFFE_TYPEOF(_contRealVar)
    DVS::_contRealVar(1, bt::make_tuple(0, ITS::_dummyTag, 0.0, 0.0, 0));

const CFFE_TYPEOF(_discRealVar)
    DVS::_discRealVar(1, bt::make_tuple(0, ITS::_dummyTag, DoubleVector(1, 0.0)));

const CFFE_TYPEOF(_contIntVar)
    DVS::_contIntVar(1, bt::make_tuple(0, ITS::_dummyTag, 0, 0));

const CFFE_TYPEOF(_discIntVar)
    DVS::_discIntVar(1, bt::make_tuple(0, ITS::_dummyTag, IntVector(1, 0)));

const CFFE_TYPEOF(_boolVar)
    DVS::_boolVar(1, bt::make_tuple(0, ITS::_dummyTag));

const CFFE_TYPEOF(_linMinObj)
    DVS::_linMinObj(1, bt::make_tuple(0, ITS::_dummyTag, DoubleVector(1, 0.0)));

const CFFE_TYPEOF(_nonLinMinObj)
    DVS::_nonLinMinObj(1, bt::make_tuple(0, ITS::_dummyTag));

const CFFE_TYPEOF(_linMaxObj)
    DVS::_linMaxObj(1, bt::make_tuple(0, ITS::_dummyTag, DoubleVector(1, 0.0)));

const CFFE_TYPEOF(_nonLinMaxObj)
    DVS::_nonLinMaxObj(1, bt::make_tuple(0, ITS::_dummyTag));

const CFFE_TYPEOF(_linSkValObj)
    DVS::_linSkValObj(
        1,
        bt::make_tuple(0, ITS::_dummyTag, 0.0, DoubleVector(1, 0.0))
        );

const CFFE_TYPEOF(_nonLinSkValObj)
    DVS::_nonLinSkValObj(1, bt::make_tuple(0, ITS::_dummyTag, 0.0));

const CFFE_TYPEOF(_linSkRngObj)
    DVS::_linSkRngObj(
        1,
        bt::make_tuple(0, ITS::_dummyTag, 0.0, 0.0, DoubleVector(1, 0.0))
        );

const CFFE_TYPEOF(_nonLinSkRngObj)
    DVS::_nonLinSkRngObj(1, bt::make_tuple(0, ITS::_dummyTag, 0.0, 0.0));

const CFFE_TYPEOF(_linInEqCon)
    DVS::_linInEqCon(
        1,
        bt::make_tuple(0, ITS::_dummyTag, 0.0, DoubleVector(1, 0.0))
        );

const CFFE_TYPEOF(_nonLinInEqCon)
    DVS::_nonLinInEqCon(1, bt::make_tuple(0, ITS::_dummyTag, 0.0));

const CFFE_TYPEOF(_linEqCon)
    DVS::_linEqCon(
        1,
        bt::make_tuple(0, ITS::_dummyTag, 0.0, 0.0, DoubleVector(1, 0.0))
        );

const CFFE_TYPEOF(_nonLinEqCon)
    DVS::_nonLinEqCon(1, bt::make_tuple(0, ITS::_dummyTag, 0.0, 0.0));

const CFFE_TYPEOF(_linTwoSideIneqCon)
    DVS::_linTwoSideIneqCon(1,
        bt::make_tuple(0, ITS::_dummyTag, 0.0, 0.0, DoubleVector(1, 0.0))
        );

const CFFE_TYPEOF(_nonLinTwoSideIneqCon)
    DVS::_nonLinTwoSideIneqCon(1, bt::make_tuple(0, ITS::_dummyTag, 0.0, 0.0));

const CFFE_TYPEOF(_linNotEqCon)
    DVS::_linNotEqCon(
        1,
        bt::make_tuple(0, ITS::_dummyTag, 0.0, DoubleVector(1, 0.0))
        );

const CFFE_TYPEOF(_nonLinNotEqCon)
    DVS::_nonLinNotEqCon(1, bt::make_tuple(0, ITS::_dummyTag, 0.0));


/*
================================================================================
Private Template Method Implementations
================================================================================
*/
template <typename T>
const T&
ConfigFileFrontEndApp::GetValueOf(
    const string& param
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    // Keep this broken up into two statements for compilation with gcc.
    const variable_value& vv = GetVariableValue(param);
    return vv.as<T>();
}

template <typename T, const char I>
vector<T>
ConfigFileFrontEndApp::ConcatenateVector(
    const string& tag
    )
{
    typedef typename mpl::at<all_opt_types, char[I]>::type BVT;
    const BVT& all = GetValueOf<BVT>(tag);

    // if all is empty, there is nothing to do.
    if(all.empty()) return vector<T>();

    // !!!!!what about the dummy entry!!!!!
    //// if all is size 1, return that 1 entry.
    //if(all.size() == 1) return all[0].get<1>();

    // Otherwise, we have to gather them up and append them in order.
    vector<T> gathered;

    for(typename BVT::const_iterator it(all.begin()); it!=all.end(); ++it)
    {
        // Skip the default dummy entry(ies).
        if((*it).get<0>() == ITS::_dummyTag) continue;

        const vector<T>& curr = (*it).get<1>();

        // add the values in "it" to our gathered.
        gathered.reserve(gathered.size() + curr.size());
        gathered.insert(gathered.end(), curr.begin(), curr.end());
    }

    return gathered;
}

template <typename InfoT>
void
ConfigFileFrontEndApp::AddInfoToMap(
    map<size_t, InfoT*>& theMap,
    size_t index,
    InfoT* theInfo,
    const string& type
    )
{
    EDDY_FUNC_DEBUGSCOPE

    typename map<size_t, InfoT*>::iterator loc(theMap.find(index));

    if(loc != theMap.end()) throw runtime_error(
        "Attempt to add multiple " + type + " with index " +
        lexical_cast<string, size_t>(index) + " is illegal. "
        "Please correct your file at inputs \"" +
        (*loc).second->GetLabel() + "\" and \"" + theInfo->GetLabel() + "\"."
        );

    theMap.insert(typename map<size_t, InfoT*>::value_type(index, theInfo));
}







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

int
ConfigFileFrontEndApp::Run(
    int argc,
    char* argv[]
    )
{
    EDDY_FUNC_DEBUGSCOPE

    try
    {
        // Begin by retrieving all user input.
        RetrieveAllInput(argc, argv);

        ValidateAllInput();

        // All programs must initialize JEGA once and only once.
        Driver::InitializeJEGA(
            CFFE_GETVAR(_globLogFile),
            ResolveLogLevel(CFFE_GETVAR(_globLogLevel)),
            static_cast<unsigned int>(CFFE_GETVAR(_rndSeed))
            );

        this->LoadParameterDatabase();
        this->LoadProblemConfig();

        // Now that the parameter database and the problem configuration
        // are loaded, we can solve our problem!

        // start by creating our algorithm config.  To do that, we need an
        // evaluator creator
        auto_ptr<EvaluatorCreator> evalCreator(this->GetEvaluatorCreator());
        AlgorithmConfig aConfig(*evalCreator, *this->_theParamDB);
        this->LoadAlgorithmParameters(aConfig);

        Driver app(*this->_theProbConfig);
        DesignOFSortSet res(app.ExecuteAlgorithm(aConfig));

        // WE MUST FLUSH THE RETURNED SET OF SOLUTIONS
        // TO AVOID A MEMORY LEAK!!
        res.flush();
    }
    catch(const std::exception& e)
    {
        cerr << "JEGA Configuration file front end caught an exception at "
                "the application level reading:\n\n"
             << e.what() << "\n\nAborting with value of 1.\n";
        exit(1);
    }
    catch(int e)
    {
        cerr << "JEGA Configuration file front end caught an integer "
                "exception at the application level with value " << e
             << ".  Aborting with that code.\n";
        exit(e);
    }
    catch(...)
    {
        cerr << "JEGA Configuration file front end caught an unknown "
                "exception at the application level.  Aborting with value of "
                "1.\n";
        exit(1);
    }

    return 0;
}

const options_description&
ConfigFileFrontEndApp::CommandLineOnlyInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const options_description options(
        CreateCommandLineOnlyInputOptions()
        );
    return options;
}

const options_description&
ConfigFileFrontEndApp::AllCommandLineInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const options_description options(
        CreateAllCommandLineInputOptions()
        );

    return options;
}

const options_description&
ConfigFileFrontEndApp::ConfigFileOnlyInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const options_description options(
       CreateConfigFileOnlyInputOptions()
       );
    return options;
}

const options_description&
ConfigFileFrontEndApp::AllConfigFileInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const options_description options(
       CreateAllConfigFileInputOptions()
       );
    return options;
}

const options_description&
ConfigFileFrontEndApp::SharedInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const options_description options(CreateSharedInputOptions());
    return options;
}

const options_description&
ConfigFileFrontEndApp::AllInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static options_description options(CreateAllInputOptions());
    return options;
}

bool
ConfigFileFrontEndApp::HasInputValueFor(
    const string& param
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_vMap->count(param) != 0;
}

void
ConfigFileFrontEndApp::PrintAllInputArgs(
    ostream& stream
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    this->PrintSuppliedCommandLineArgs(stream);
    this->PrintSuppliedConfigFileArgs(stream);
}

#define PRINT_CFI(stream, tag) \
    if(this->HasInputValueFor(ITS::tag)) \
        stream << ITS::tag << ": " << CFFE_GETVAR(tag) << "\n";

void
ConfigFileFrontEndApp::PrintSuppliedCommandLineArgs(
    ostream& stream
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    stream << "Command line supplied arguments:\n"
           << "================================\n";

    PRINT_CFI(stream, _cfgFile);
    PRINT_CFI(stream, _help);
    PRINT_CFI(stream, _version);

    stream << "\n";
}

void
ConfigFileFrontEndApp::PrintSuppliedConfigFileArgs(
    ostream& stream
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    stream << "Configuration file supplied arguments:\n"
           << "======================================\n";

    PRINT_CFI(stream, _assessor     );
    PRINT_CFI(stream, _initializer  );
    PRINT_CFI(stream, _mutator      );
    PRINT_CFI(stream, _crosser      );
    PRINT_CFI(stream, _nicher       );
    PRINT_CFI(stream, _postprocessor);
    PRINT_CFI(stream, _mainloop     );
    PRINT_CFI(stream, _selector     );
    PRINT_CFI(stream, _converger    );

    PRINT_CFI(stream, _intParams    );
    PRINT_CFI(stream, _dblParams    );
    PRINT_CFI(stream, _sztParams    );
    PRINT_CFI(stream, _boolParams   );
    PRINT_CFI(stream, _strParams    );
    PRINT_CFI(stream, _intVecParams );
    PRINT_CFI(stream, _dblVecParams );
    PRINT_CFI(stream, _strVecParams );

    PRINT_CFI(stream, _rndSeed      );
    PRINT_CFI(stream, _globLogFile  );
    PRINT_CFI(stream, _globLogLevel );
    PRINT_CFI(stream, _algType      );

    PRINT_CFI(stream, _contRealVar  );
    PRINT_CFI(stream, _discRealVar  );
    PRINT_CFI(stream, _contIntVar   );
    PRINT_CFI(stream, _discIntVar   );
    PRINT_CFI(stream, _boolVar      );

    PRINT_CFI(stream, _linMinObj     );
    PRINT_CFI(stream, _nonLinMinObj  );
    PRINT_CFI(stream, _linMaxObj     );
    PRINT_CFI(stream, _nonLinMaxObj  );
    PRINT_CFI(stream, _linSkValObj   );
    PRINT_CFI(stream, _nonLinSkValObj);
    PRINT_CFI(stream, _linSkRngObj   );
    PRINT_CFI(stream, _nonLinSkRngObj);

    PRINT_CFI(stream, _linInEqCon          );
    PRINT_CFI(stream, _nonLinInEqCon       );
    PRINT_CFI(stream, _linEqCon            );
    PRINT_CFI(stream, _nonLinEqCon         );
    PRINT_CFI(stream, _linTwoSideIneqCon   );
    PRINT_CFI(stream, _nonLinTwoSideIneqCon);
    PRINT_CFI(stream, _linNotEqCon         );
    PRINT_CFI(stream, _nonLinNotEqCon      );

    stream << "\n";
}

#undef PRINT_CFI



/*
================================================================================
Subclass Visible Methods
================================================================================
*/

const variable_value&
ConfigFileFrontEndApp::GetVariableValue(
    const string& param
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_vMap->operator[](param);
}

void
ConfigFileFrontEndApp::PrintHelpMessage(
    ostream& into
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // print a message indicating what is being shown and how to use
    // the configuration file utility.
    into << "\n\nThe following is a listing of the allowable command "
            "line input arguments for JEGA.  Some arguments are reserved "
            "only for the command line and others are reserved for input "
            "through the configuration file only.  See the listings below for "
            "a complete reference of available inputs.\n\n"

            "Inputs to the command line are preceeded with two hyphens when "
            "no abbreviation is used and with a single hyphen if using an "
            "accepted abbreviation.  For example:\n\n"

            "\t>> jega --help\n"
            "\t>> jega -h\n\n"

            "are equivolent and will print this message.\n\n"

            "Inputs to the configuration file are quite different.  Each is "
            "identified using an all-caps identifier.  See the listing below "
            "for a complete reference of available identifiers.  The majority "
            "of the configuration file inputs are tuples meaning that "
            "multiple pieces of information are supplied on a single line.  "
            "Some of them are also vector valued meaning that one of the "
            "inputs is a vector.  Consider for example the input for "
            "declaring a discrete real variable.  The required inputs in "
            "order are the index of the variable, the name of the variable, "
            "and the values that it may have.  The values that it may have is "
            "read in as a vector.  It has no predefined size and so each of "
            "the following is perfectly legal input:\n\n"

            "\tDISC_REAL_VAR = 0 X1 0.0\n"
            "\tDISC_REAL_VAR = 0 X1 0.0 1.0\n"
            "\tDISC_REAL_VAR = 0 X1 0.0 1.0 3.4 5.6 7.1\n\n"

    // output the allowable command line input arguments for this application.
         << "The following are the COMMAND LINE ONLY input options:\n"
            "=======================================================\n"
         << this->CommandLineOnlyInputOptions()
         << "\n\n"

         << "The following are the CONFIGURATION FILE ONLY input options:\n"
            "=======================================================\n"
         << this->ConfigFileOnlyInputOptions()
         << "\n\n"

         << "There are currently no SHARED input options.\n\n";

    //     << "The following are the SHARED input options:\n"
    //        "=======================================================\n"
    //     << SharedInputOptions()
    //     << "\n\n";
}

void
ConfigFileFrontEndApp::PrintVersionMessage(
    ostream& into
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // This date stuff is very temporary since it only gets updated when this
    // file gets built.  Need a more robust and correct solution.
    into << JEGA_PACKAGE << " built on " << __DATE__ << '\n';
}

#define LOAD_DB_PARAMS(tag, meth) { \
        const CFFE_TYPEOF(tag)& prms = CFFE_GETVAR(tag); \
        for(CFFE_TYPEOF(tag)::const_iterator it(prms.begin()); \
            it!=prms.end(); ++it) if((*it).get<0>() != ITS::_dummyTag) \
                this->_theParamDB->meth((*it).get<0>(), (*it).get<1>()); \
    }

void
ConfigFileFrontEndApp::LoadParameterDatabase(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Start by loading in all the singular valued parameters.
    LOAD_DB_PARAMS(_intParams   , AddIntegralParam      );
    LOAD_DB_PARAMS(_dblParams   , AddDoubleParam        );
    LOAD_DB_PARAMS(_sztParams   , AddSizeTypeParam      );
    LOAD_DB_PARAMS(_boolParams  , AddBooleanParam       );
    LOAD_DB_PARAMS(_strParams   , AddStringParam        );
    LOAD_DB_PARAMS(_intVecParams, AddIntVectorParam     );
    LOAD_DB_PARAMS(_dblVecParams, AddDoubleVectorParam  );
    LOAD_DB_PARAMS(_strVecParams, AddStringVectorParam  );

    // There is an additional requirement that we make the output an integral
    // parameter since that is what the driver will be looking for.
    // Fortunately, this should be quick and easy since it is already in as
    // a string parameter labeled "method.log_level" (if at all).
    if(this->_theParamDB->HasStringParam("method.log_level"))
        this->_theParamDB->AddIntegralParam(
            "method.output",
            ResolveLogLevel(this->_theParamDB->GetString("method.log_level"))
            );

    LoadDoubleMatrices();
}

#undef LOAD_DB_PARAMS

void
ConfigFileFrontEndApp::LoadProblemConfig(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    LoadDesignVariables();
    LoadObjectiveFunctions();
    LoadConstraints();

    // Find out if we should store discards or not.
    bool doStore = true;

    try {
        doStore = this->_theParamDB->GetBoolean("method.jega.store_discards");
    }
    catch(...) { doStore = true; }

    this->_theProbConfig->SetDiscardTracking(doStore);
}

#define GET(r, state) \
    (*it).get< BOOST_PP_TUPLE_ELEM(2, 0, state) >() \
    BOOST_PP_IF( \
        BOOST_PP_EQUAL( \
            BOOST_PP_INC(BOOST_PP_TUPLE_ELEM(2, 0, state)), \
            BOOST_PP_TUPLE_ELEM(2, 1, state) \
        ), BOOST_PP_EMPTY, BOOST_PP_COMMA )()

#define ADD_INFO(tag, mapName, infoType, infoTypeDesc, getFunc, N) \
{ \
    const CFFE_TYPEOF(tag)& vname = CFFE_GETVAR(tag); \
    for(CFFE_TYPEOF(tag)::const_iterator it(vname.begin()); \
        it!=vname.end(); ++it) \
            if((*it).get<1>() != ITS::_dummyTag) \
                AddInfoToMap<infoType>(mapName, (*it).get<0>(), \
                    ConfigHelper::getFunc( \
                        target, BOOST_PP_FOR((1, N), ENUM_PRED, ENUM_OP, GET) \
                    ), infoTypeDesc \
                ); \
}

#define ADD_DV_INFO(tag, mapName, getFunc, N) \
    ADD_INFO(tag, mapName, DesignVariableInfo, "design variables", getFunc, N)

void
ConfigFileFrontEndApp::LoadDesignVariables(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // The design variables must all be given unique indices. We will sort by
    // index and add them in the order in which they end up.

    // In order to sort them by index, we will use a map.  We will map indices
    // to newly created design variable infos.  If any index is repeated,
    // throw an error.
    typedef std::map<size_t, DesignVariableInfo*> DVMap;
    DVMap dvMap;

    // We are going to need the design target for all of this.
    DesignTarget& target = this->_theProbConfig->GetDesignTarget();

    ADD_DV_INFO(_contRealVar, dvMap, GetContinuumRealVariable, 5)
    ADD_DV_INFO(_discRealVar, dvMap, GetDiscreteRealVariable, 3)
    ADD_DV_INFO(_contIntVar, dvMap, GetContinuumIntegerVariable, 4)
    ADD_DV_INFO(_discIntVar, dvMap, GetDiscreteIntegerVariable, 3)
    ADD_DV_INFO(_boolVar, dvMap, GetBooleanVariable, 2)

    // If we make it here, all our infos loaded up.  Now stick them into the
    // target.
    for(DVMap::const_iterator it(dvMap.begin()); it!=dvMap.end(); ++it)
        target.AddDesignVariableInfo(*(*it).second);
}

#define ADD_OF_INFO(tag, mapName, getFunc, N) \
    ADD_INFO(tag, mapName, ObjectiveFunctionInfo, \
             "objective functions", getFunc, N)

void
ConfigFileFrontEndApp::LoadObjectiveFunctions(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // The objective functions must all be given unique indices. We will sort
    // by index and add them in the order in which they end up.

    // In order to sort them by index, we will use a map.  We will map indices
    // to newly created objective function infos.  If any index is repeated,
    // throw an error.
    typedef std::map<size_t, ObjectiveFunctionInfo*> OFMap;
    OFMap ofMap;

    // We are going to need the design target for all of this.
    DesignTarget& target = this->_theProbConfig->GetDesignTarget();

    // Start with the linear minimize objectives.
    ADD_OF_INFO(_linMinObj, ofMap, GetLinearMinimizeObjective, 3)
    ADD_OF_INFO(_nonLinMinObj, ofMap, GetNonlinearMinimizeObjective, 2)
    ADD_OF_INFO(_linMaxObj, ofMap, GetLinearMaximizeObjective, 3)
    ADD_OF_INFO(_nonLinMaxObj, ofMap, GetNonlinearMaximizeObjective, 2)
    ADD_OF_INFO(_linSkValObj, ofMap, GetLinearSeekValueObjective, 4)
    ADD_OF_INFO(_nonLinSkValObj, ofMap, GetNonlinearSeekValueObjective, 3)
    ADD_OF_INFO(_linSkRngObj, ofMap, GetLinearSeekRangeObjective, 5)
    ADD_OF_INFO(_nonLinSkRngObj, ofMap, GetNonlinearSeekRangeObjective, 4)

    // If we make it here, all our infos loaded up.  Now stick them into the
    // target.
    for(OFMap::const_iterator it(ofMap.begin()); it!=ofMap.end(); ++it)
        target.AddObjectiveFunctionInfo(*(*it).second);
}

#define ADD_CN_INFO(tag, mapName, getFunc, N) \
    ADD_INFO(tag, mapName, ConstraintInfo, "constraints", getFunc, N)

void
ConfigFileFrontEndApp::LoadConstraints(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // The constraint functions must all be given unique indices. We will sort
    // by index and add them in the order in which they end up.

    // In order to sort them by index, we will use a map.  We will map indices
    // to newly created constraint function infos.  If any index is repeated,
    // throw an error.
    typedef std::map<size_t, ConstraintInfo*> CNMap;
    CNMap cnMap;

    // We are going to need the design target for all of this.
    DesignTarget& target = this->_theProbConfig->GetDesignTarget();

    // Start with the linear inequality constraints.
    ADD_CN_INFO(_linInEqCon, cnMap, GetLinearInequalityConstraint, 4)
    ADD_CN_INFO(_nonLinInEqCon, cnMap, GetNonlinearInequalityConstraint, 3)
    ADD_CN_INFO(_linEqCon, cnMap, GetLinearEqualityConstraint, 5)
    ADD_CN_INFO(_nonLinEqCon, cnMap, GetNonlinearEqualityConstraint, 4)

    ADD_CN_INFO(_linTwoSideIneqCon, cnMap,
        GetLinearTwoSidedInequalityConstraint, 5
        )

    ADD_CN_INFO(
        _nonLinTwoSideIneqCon, cnMap,
        GetNonlinearTwoSidedInequalityConstraint, 4
        )

    ADD_CN_INFO(_linNotEqCon, cnMap, GetLinearNotEqualityConstraint, 4)
    ADD_CN_INFO(_nonLinNotEqCon, cnMap, GetNonlinearNotEqualityConstraint, 3)

    // If we make it here, all our infos loaded up.  Now stick them into the
    // target.
    for(CNMap::const_iterator it(cnMap.begin()); it!=cnMap.end(); ++it)
        target.AddConstraintInfo(*(*it).second);
}

#undef ADD_CN_INFO
#undef ADD_OF_INFO
#undef ADD_DV_INFO
#undef ADD_INFO
#undef GET

void
ConfigFileFrontEndApp::LoadDoubleMatrices(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // to do this, we must gather all vectors input with the same tag via
    // DBL_MAT_PARAM into matrices.
    typedef CFFE_TYPEOF(_dblMatParams) StrVecPairVec;
    const StrVecPairVec& all = CFFE_GETVAR(_dblMatParams);

    // create a map of strings to matrices to gather these up.
    typedef std::map<string, DoubleMatrix> MatMap;

    MatMap matrices;

    for(StrVecPairVec::const_iterator it(all.begin()); it!=all.end(); ++it)
    {
        // Skip the default dummy entry(ies).
        if((*it).get<0>() == ITS::_dummyTag) continue;

        MatMap::iterator curr(matrices.find((*it).get<0>()));

        // if we have a running matrix entry, then add on to it.  Otherwise,
        // put in a new entry.
        if(curr != matrices.end()) (*curr).second.push_back((*it).get<1>());

        else matrices[(*it).get<0>()] = DoubleMatrix(1, (*it).get<1>());
    }

    // now we've gathered up and created all our matrices, now put them in
    // the database.
    for(MatMap::const_iterator it(matrices.begin()); it!=matrices.end(); ++it)
        this->_theParamDB->AddDoubleMatrixParam((*it).first, (*it).second);

}

void
ConfigFileFrontEndApp::ValidateAlgorithmType(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    const string& algType = CFFE_GETVAR(_algType);
    if(algType != "MOGA" && algType != "SOGA") throw runtime_error(
        "Invalid algorithm type supplied \"" + algType +
        "\".  Must be one of MOGA or SOGA."
        );
}

void
ConfigFileFrontEndApp::LoadAlgorithmParameters(
    AlgorithmConfig& aConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Start by getting the algorithm type.
    const CFFE_TYPEOF(_algType)& aType = CFFE_GETVAR(_algType);

    if(aType == "MOGA") aConfig.SetAlgorithmType(AlgorithmConfig::MOGA);
    else if(aType == "SOGA") aConfig.SetAlgorithmType(AlgorithmConfig::SOGA);
    else throw runtime_error(
        "Invalid algorithm type supplied \"" + aType + "\""
        );

    aConfig.SetMutatorName(CFFE_GETVAR(_mutator));
    aConfig.SetFitnessAssessorName(CFFE_GETVAR(_assessor));
    aConfig.SetInitializerName(CFFE_GETVAR(_initializer));
    aConfig.SetCrosserName(CFFE_GETVAR(_crosser));
    aConfig.SetNichePressureApplicatorName(CFFE_GETVAR(_nicher));
    aConfig.SetPostProcessorName(CFFE_GETVAR(_postprocessor));
    aConfig.SetSelectorName(CFFE_GETVAR(_selector));
    aConfig.SetMainLoopName(CFFE_GETVAR(_mainloop));
    aConfig.SetConvergerName(CFFE_GETVAR(_converger));
}


/*
================================================================================
Subclass Overridable Methods
================================================================================
*/


void
ConfigFileFrontEndApp::RetrieveAllInput(
    int argc,
    char* argv[]
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // start with the command line input.
    RetrieveCommandLineInput(argc, argv);

    // now, if a configuration file was not supplied, we are screwed!
    if(!HasInputValueFor(ITS::_cfgFile)) throw runtime_error(
        "The JEGA configuration file front end must be supplied an "
        "input file via the command line.  Use --help to see how."
        );

    RetrieveConfigFileInput();
}


void
ConfigFileFrontEndApp::RetrieveCommandLineInput(
    int argc,
    char* argv[]
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Start by parsing and storing the input.
    store(parse_command_line(
        argc, argv, AllCommandLineInputOptions()
        ), *this->_vMap);

    // now notify all callback handlers for the variables.
    notify(*this->_vMap);

    // under certain circumstances, we needn't go any further in
    // the program than this.

    // The help flag is the first such instance we will look for.
    if(HasInputValueFor(ITS::_help))
    {
        PrintHelpMessage(cerr);
        exit(0);
    }
    // The version flag is the second such instance we will look for.
    if(HasInputValueFor(ITS::_version))
    {
        PrintVersionMessage(cerr);
        exit(0);
    }
}

void
ConfigFileFrontEndApp::RetrieveConfigFileInput(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // otherwise, open the file.
    string cfgFileName(GetValueOf<string>(ITS::_cfgFile));
    ifstream ifile(cfgFileName.c_str());

    // if it didn't open, there is some other sort of problem that
    // we cannot resolve.
    if(!ifile.is_open()) throw runtime_error(
        "Unable to open configuration file \"" + cfgFileName +
        "\" for parsing."
        );

    // now parse the file and store the input.
    store(parse_config_file(ifile, AllConfigFileInputOptions()), *this->_vMap);

    // now close the file for good measure.
    ifile.close();

    // now notify all callback handlers for the variable map.
    notify(*this->_vMap);
}

void
ConfigFileFrontEndApp::ValidateAllInput(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ValidateAlgorithmType();
}

EvaluatorCreator*
ConfigFileFrontEndApp::GetEvaluatorCreator(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new GenericEvaluatorCreator<ExternalEvaluator>();
}

LogLevel
ConfigFileFrontEndApp::ResolveLogLevel(
    const string& input
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // Use the LevelClass to resolve.  any unrecognized input will result in
    // the use of the Default level.
    return LevelClass::get_level_of(input);
}



/*
================================================================================
Private Methods
================================================================================
*/


options_description
ConfigFileFrontEndApp::CreateAllCommandLineInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    options_description options("All Command Line Input Options");
    options.add(CommandLineOnlyInputOptions()).add(SharedInputOptions());
    return options;
}

#define OPTION_ND(id, d) ( ITS::id, value<CFFE_TYPEOF(id)>(), d )
#define OPTION_WD(id, d) \
    ( ITS::id.c_str(), value<CFFE_TYPEOF(id)>()->default_value(DVS::id), d )

options_description
ConfigFileFrontEndApp::CreateCommandLineOnlyInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    options_description options("Command Line Only Input Options");

    options.add_options()
        (
            (ITS::_cfgFile + ",c").c_str(),
            value<CFFE_TYPEOF(_cfgFile)>()->default_value(DVS::_cfgFile),
            "The file from which to retrieve the configuration settings."
        )
        (
            (ITS::_help + ",h").c_str(),
            "Prints the help message to the standard output stream."
        )
        (
            (ITS::_version + ",v").c_str(),
            "Prints the current JEGA version and build date."
        )
        ;

    return options;
}

options_description
ConfigFileFrontEndApp::CreateConfigFileOnlyInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    options_description options("Configuration File Only Input Options");

    options.add_options()

        OPTION_WD(_assessor,
            "The name of the fitness assessment operator to use (string)."
            )

        OPTION_WD(_initializer,
            "The name of the initialization operator to use (string)."
            )

        OPTION_WD(_mutator,
            "The name of the mutation operator to use (string)."
            )

        OPTION_WD(_crosser,
            "The name of the crossover operator to use (string)."
            )

        OPTION_WD(_nicher,
            "The name of the niche pressure operator to use (string)."
            )

        OPTION_WD(_postprocessor,
            "The name of the post processing operator to use (string)."
            )

        OPTION_WD(_mainloop,
            "The name of the main loop operator to use (string)."
            )

        OPTION_WD(_selector,
            "The name of the selection operator to use (string)."
            )

        OPTION_WD(_converger,
            "The name of the convergence operator to use (string)."
            )

        OPTION_WD(_intParams,
            "Integral parameters as name value pairs (string, integer)."
            )

        OPTION_WD(_dblParams,
            "Floating point parameters as name value pairs (string, real)."
            )

        OPTION_WD(_sztParams,
            "Size type parameters as name value pairs "
            "(string, unsigned integer)."
            )

        OPTION_WD(_boolParams,
            "Boolean parameters as name value pairs (string, boolean)."
            )

        OPTION_WD(_strParams,
            "String parameters as name value pairs (string, string)."
            )

        OPTION_WD(_intVecParams,
            "Integral vector parameters as name vector-value pairs "
            "(string, vector of integer)."
            )

        OPTION_WD(_dblVecParams,
            "Double vector parameters as name vector-value pairs "
            "(string, vector of real)."
            )

        OPTION_WD(_strVecParams,
            "String vector parameters as name vector-value pairs "
            "(string, vector of string)."
            )

        OPTION_WD(_dblMatParams,
            "Double matrix parameters as multiple name vector-value pairs "
            "(multiple (string, vector of real) each with the same string)."
            )

        OPTION_WD(_rndSeed,
            "The seed for the random number generator (unsigned integer)."
            )

        OPTION_WD(_globLogFile,
            "The name of the file for the global logger to write to (string)."
            )

        OPTION_WD(_globLogLevel,
            "The level at which to log entries to the global log "
            "(string-one of debug, verbose, quiet, silent, or fatal)."
            )

        OPTION_WD(_algType,
            "The type of the algorithm to run (string-one of MOGA or SOGA)."
            )

        OPTION_WD(_contRealVar,
            "Declaration of a continuous real variable "
            "(unsigned integer, string, real, real, integer)."
            )

        OPTION_WD(_discRealVar,
            "Declaration of a discrete real variable "
            "(unsigned integer, string, vector of real)."
            )

        OPTION_WD(_contIntVar,
            "Declaration of a continuous integer variable "
            "(unsigned integer, string, integer, integer)."
            )

        OPTION_WD(_discIntVar,
            "Declaration of a discrete integer variable "
            "(unsigned integer, string, vector of integer)."
            )

        OPTION_WD(_boolVar,
            "Declaration of a boolean variable "
            "(unsigned integer, string-one of t, f, true, false, 1, 0)."
            )

        OPTION_WD(_linMinObj,
            "Declaration of a linear minimization objective "
            "(unsigned integer, string, vector of real)."
            )

        OPTION_WD(_nonLinMinObj,
            "Declaration of a non-linear minimization objective "
            "(unsigned integer, string)."
            )

        OPTION_WD(_linMaxObj,
            "Declaration of a linear maximization objective "
            "(unsigned integer, string, vector of real)."
            )

        OPTION_WD(_nonLinMaxObj,
            "Declaration of a non-linear maximization objective "
            "(unsigned integer, string)."
            )

        OPTION_WD(_linSkValObj,
            "Declaration of a linear seek value objective "
            "(unsigned integer, string, real, vector of real)."
            )

        OPTION_WD(_nonLinSkValObj,
            "Declaration of a non-linear seek value objective "
            "(unsigned integer, string, real)."
            )

        OPTION_WD(_linSkRngObj,
            "Declaration of a linear seek range objective "
            "(unsigned integer, string, real, real, vector of real)."
            )

        OPTION_WD(_nonLinSkRngObj,
            "Declaration of a non-linear seek range objective "
            "(unsigned integer, string, real, real)."
            )

        OPTION_WD(_linInEqCon,
            "Declaration of a linear inequality constraint "
            "(unsigned integer, string, real, vector of real)."
            )

        OPTION_WD(_nonLinInEqCon,
            "Declaration of a non-linear inequality constraint "
            "(unsigned integer, string, real)."
            )

        OPTION_WD(_linEqCon,
            "Declaration of a linear equality constraint "
            "(unsigned integer, string, real, real, vector of real)."
            )

        OPTION_WD(_nonLinEqCon,
            "Declaration of a non-linear equality constraint "
            "(unsigned integer, string, real, real)."
            )

        OPTION_WD(_linTwoSideIneqCon,
            "Declaration of a linear 2-sided inequality constraint "
            "(unsigned integer, string, real, real, vector of real)."
            )

        OPTION_WD(_nonLinTwoSideIneqCon,
            "Declaration of a non-linear 2-sided inequality constraint "
            "(unsigned integer, string, real, real)."
            )

        OPTION_WD(_linNotEqCon,
            "Declaration of a linear not-equality constraint "
            "(unsigned integer, string, real, vector of real)."
            )

        OPTION_WD(_nonLinNotEqCon,
            "Declaration of a non-linear not-equality constraint "
            "(unsigned integer, string, real)."
            )
        ;

    return options;
}

#undef ADD_OPTION_ND
#undef ADD_OPTION_WD

program_options::options_description
ConfigFileFrontEndApp::CreateAllConfigFileInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    options_description options("All Configuration File Input Options");
    options.add(ConfigFileOnlyInputOptions()).add(SharedInputOptions());
    return options;
}

options_description
ConfigFileFrontEndApp::CreateSharedInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    options_description options("Shared Input Options");
    return options;
}

options_description
ConfigFileFrontEndApp::CreateAllInputOptions(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // create the options_description object with the desired caption.
    options_description options("All Input Options");

    // now add the comman line only, config file only, and shared options.
    options.add(CommandLineOnlyInputOptions());
    options.add(ConfigFileOnlyInputOptions());
    options.add(SharedInputOptions());

    // finally, return our options object.
    return options;
}

/*
================================================================================
Structors
================================================================================
*/


ConfigFileFrontEndApp::ConfigFileFrontEndApp(
    ) :
        _vMap(new variables_map()),
        _theParamDB(new BasicParameterDatabaseImpl()),
        _theProbConfig(new ProblemConfig())
{
    EDDY_FUNC_DEBUGSCOPE
}

ConfigFileFrontEndApp::~ConfigFileFrontEndApp(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    delete this->_vMap;
    delete this->_theParamDB;
    delete this->_theProbConfig;
}






/*
================================================================================
End Namespace
================================================================================
*/
        } // namespace ConfigFile
    } // namespace FrontEnd
} // namespace JEGA
