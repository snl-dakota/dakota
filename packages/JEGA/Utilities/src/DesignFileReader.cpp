/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class DesignFileReader.

    NOTES:

        See notes of DesignFileReader.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.0.0

    CHANGES:

        Fri Mar 03 13:23:45 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the DesignFileReader class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <sstream>
#include <fstream>
#include <utilities/include/Math.hpp>
#include <../Utilities/include/Design.hpp>
#include <../Utilities/include/Logging.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <../Utilities/include/DesignFileReader.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>







/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Logging;
using namespace eddy::utilities;







/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Utilities {



string
DesignFileReader::Result::GetResultsString(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // use a string stream to get this job done.
    ostringstream ostrm;

    // start pumping information into the stream.
    ostrm << "Read File: " << this->GetFilename()
          << "\nCatastrophic Error: "
          << boolalpha
          << this->GetErrorFlag()
          << "\nTotal Attempts: "
          << this->GetTotalAttemptedReads()
          << "\nFull Design Reads: "
          << this->GetNumFullReads()
          << "\nPartial Design Reads: "
          << this->GetNumPartialReads()
          << "\nFailed Reads: "
          << this->GetNumFailures()
          << "\nEmpty Lines: "
          << this->GetNumEmptyLines();

    return ostrm.str();

} // DesignFileReader::Result::GetResultsString



/*
================================================================================
Static Member Data Definitions
================================================================================
*/

const string DesignFileReader::ALL_WHITESPACE(
    DesignFileReader::InitializeWhiteString()
    );

const string DesignFileReader::WHITESPACE_DELIMITER("d__WHITE__d");




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

DesignFileReader::Result
DesignFileReader::ReadFlatFile(
    const string& file,
    const DesignTarget& target
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return ReadFlatFile(file, "", target);
}

DesignFileReader::Result
DesignFileReader::ReadFlatFile(
    const string& file,
    const string& delim,
    const DesignTarget& target
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // prepare a default constructed return object
    Result ret;

    // record the name of the file with the results.
    ret._filename.assign(file);

    // open the file
    ifstream ifile(file.c_str(), ios::in);

    // if we couldn't open the file, set the error flag and return.
    if(!ifile.is_open()) { ret._error = true; return ret; }

    // prepare a Design pointer for repeated use.
    Design* newdes = static_cast<Design*>(0);

    // get the number of design variables per Design
    const size_t ndv = target.GetNDV();

    // get the number of objective functions per Design
    const size_t nof = target.GetNOF();

    // get the number of constraints per Design
    const size_t ncn = target.GetNCN();

    // store the number of responses for repeated use.
    const size_t totparams = ndv + ncn + nof;

    // prepare to store the file line-at-a-time for parsing.
    string line;

    // the line should be no larget than 1KB
    line.reserve(1024);

    // Prepare to store the parsed response values in case they exist for any of
    // the designs in the file.  The initial size is just a best guess.
    // It will not be a problem if there are more or less for a particular read.
    DoubleVector values;
    values.reserve(static_cast<DoubleVector::size_type>(totparams));

    // loop through and create the Designs (keep counts)
    while(!ifile.eof())
    {
        // count this as an attempt.
        ++ret._attempts;

        // Get the next line of the file.
        getline(ifile, line);

        // strip whitespace off the ends of the line.
        line = TrimWhitespace(line);

        // if we have an empty line, note that.
        if(line.empty()) { ++ret._emptyLines; continue; }

        // Parse the line into doubles
        std::string usedDelim(delim);

        if(delim.empty()) usedDelim = ParseValues(line, values);
        else ParseValues(line, delim, values);

        // if reps is not the size ndv, we have a failed attempt.
        if(values.size() < ndv)
        {
            JEGALOG_IT_G(
                ldebug(), DesignFileReader,
                ostream_entry(ldebug(),
                    "Design File Reader: Failed to read a usable design from " +
                    line + " with delimiter = " + usedDelim +
                    ".  Needed ") << ndv << " variables.  Got " << values.size()
                    << '.'
                )

            ++ret._failures;
            continue;
        }

        // otherwise we can have a new design.
        newdes = target.GetNewDesign();

        // record the values into newdes
        RecordValues(values, *newdes);

        // put the new Design into the initialization group.
        ret._designs.insert(newdes);

        // We know we have at least ndv values read in but
        // if we have fewer than "totparams" values
        // in "into", we can have a Design but it will not be
        // considered evaluated
        if(values.size() < totparams)
        {
            ++ret._partReads;
            newdes->SetEvaluated(false);
        }
        // otherwise we have a full Design.
        else
        {
            // increment our full read count.
            ++ret._fullReads;
            newdes->SetEvaluated(true);
            target.CheckFeasibility(*newdes);
            RecordViolations(*newdes, target);
        }
    }

    // close the file prior to returning.
    ifile.close();

    // finally, return our read results.
    return ret;
}







/*
================================================================================
Subclass Visible Methods
================================================================================
*/
std::string
DesignFileReader::ParseValues(
    const string& from,
    DoubleVector& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

#ifdef JEGA_LOGGING_ON
    static std::string ret;
    ret = DetermineDelimeter(from, ret);
#else
    std::string ret(DetermineDelimeter(from));
#endif
    ParseValues(from, ret, into);
    return ret;
}

void
DesignFileReader::ParseValues(
    const string& from,
    const string& delim,
    DoubleVector& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // clear the "into" vector.
    into.clear();

    // this string will store the values prior to conversion to doubles.
    string valstr;

    // the beginning of a substring containing a single double.
    string::size_type strt = 0;

    // extract the first value as a string.
    valstr.assign(GetNextField(from, delim, strt));

    // now eat all whitespace.
    while((strt < from.size()) && isspace(from[strt])) ++strt;

    // go for as long as we can get values.
    while(!valstr.empty())
    {
        // convert the string into a double
        pair<bool, double> conv(ToDouble(valstr));

        if(!conv.first) return;

        // otherwise record the value
        into.push_back(conv.second);

        // extract the next value as a string.
        valstr.assign(GetNextField(from, delim, strt));

        // now eat all whitespace.
        while((strt < from.size()) && isspace(from[strt])) ++strt;
    }
}

string
DesignFileReader::DetermineDelimeter(
    const string& from
#ifdef JEGA_LOGGING_ON
    , const std::string& prev
#endif
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(static const std::string AWS("All Whitespace");)

    // we are looking for a pattern whereby we have double precision values
    // separated by some delimeter.  For this to be the case, the very first
    // thing we need to find is a double precision value.  If it is not the
    // first thing, then we have a problem.

    // the dumb but sure-fire way to do this is to consider the input 1
    // character at a time until we no longer have a double precision number.
    // Call that the first value, then traverse from there until we run
    // into the next dp value and call everything in between the delimeter.
    string::size_type off = 0;
    string delimCandidate(FindCandidateDelimiter(from, off));
    if(delimCandidate.empty()) return delimCandidate;

    while(off < from.size())
    {
        string nextCandidate(FindCandidateDelimiter(from, off));
        if(nextCandidate.empty())
        {
            JEGA_LOGGING_IF_ON(
                const std::string& delim =
                    delimCandidate == WHITESPACE_DELIMITER ?
                    AWS : delimCandidate;
                )
            JEGAIFLOG_IT_G(
                delimCandidate != prev, ldebug(), DesignFileReader,
                text_entry(
                    ldebug(),
                    "Design File Reader: Delimiter determined to be " + delim
                    )
                )

            return delimCandidate;
        }

        if(nextCandidate != delimCandidate)
        {
            JEGALOG_IT_G(
                ldebug(), DesignFileReader,
                text_entry(
                    ldebug(), "Design File Reader: Failed to find a delimiter "
                    "due to mismatch  of " + delimCandidate + " followed by " +
                    nextCandidate + '.'
                    )
                )

            return string();
        }
    }

    JEGA_LOGGING_IF_ON(
        const std::string& delim =
            delimCandidate == WHITESPACE_DELIMITER ?
            AWS : delimCandidate;
        )

    JEGAIFLOG_IT_G(
        delimCandidate != prev, ldebug(), DesignFileReader,
        text_entry(
            ldebug(),
            "Design File Reader: Delimiter determined to be " + delim
            )
        )

    return delimCandidate;
}

string
DesignFileReader::FindCandidateDelimiter(
    const string& in,
    string::size_type& off
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // to distinguish a double precision value, we will test to see that is it
    // a legitimate character and then to see that we can convert to a double.
    // Note that whitespace will generally not cause us to fail to convert.
    static const string legitChars("+-dDeE."); // as well as digits.
    static const string dStartChars("+-."); // as well as digits.

    string::size_type vpos = off;
    bool afterSci = false;

    for(; off < in.length(); ++off)
    {
        // If we are immediately following a scientific notation character and
        // this is a sign, skip it.
        if(afterSci && (in[off] == '+' || in[off] == '-'))
        { afterSci = false; continue; }

        // If we have a digit, we can continue.
        if(isdigit(in[off])) continue;

        // If we don't have a legit character, we can break
        if(legitChars.find(in[off]) == string::npos) break;

        // If the current character is one that might start a double and we
        // are at the beginning of a double, continue.
        if((vpos == off) && (dStartChars.find(in[off]) != string::npos))
            continue;

        // If we are at a scientific notation indicator, then ToDouble will
        // fail.  Continue onto the next value.
        if(in[off] == 'e' || in[off] == 'E' || in[off] == 'd' || in[off] == 'D')
        { afterSci = true; continue; }

        // Otherwise, we see if we are no longer a double and break if so.
        if(!ToDouble(in.substr(vpos, off-vpos+1)).first) break;
    }

    // If we were unable to obtain any kind of advance, then we cannot find
    // a delimiter and will return the empty string.  This may indicate the
    // end of the input string as well.
    if(off == vpos) return string();

    // by here, we have some double value between vpos and off.  so start
    // looking for the delimiter starting at off.
    vpos = off;

    for(; off < in.length(); ++off)
        if(isdigit(in[off]) ||
           (legitChars.find(in[off]) != string::npos)) break;

    // by here we should have some delimeter.  Store it as our potential
    // delimeter
    return FormatDelimiter(in.substr(vpos, off-vpos));
}


pair<bool, double>
DesignFileReader::ToDouble(
    const string& str
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // convert the string into a double
    double value;
    istringstream strm(str);
    strm >> value;
    return pair<bool, double>(!strm.fail() && !strm.bad(), value);
}

string
DesignFileReader::FormatDelimiter(
    const string& unformatted
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if the unformatted is nothing but whitespace, then we will make it
    // a sentinal string that tells the reader to consider any sized
    // block of whitespace to be a delimiter.
    if(IsAllWhite(unformatted)) return WHITESPACE_DELIMITER;

    // otherwise, strip the whitespace off the ends only and return the result.
    return TrimWhitespace(unformatted);

}

void
DesignFileReader::RecordValues(
    const DoubleVector& from,
    Design& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Get the target for information.
    const DesignTarget& target = into.GetDesignTarget();

    // Get the counts of variables and responses.
    const size_t ndv = target.GetNDV();
    const size_t nof = target.GetNOF();
    const size_t ncn = target.GetNCN();

    // stop at the first of the number of needed values
    // and the number of values available.
    size_t end =
        Math::Min<size_t>(ndv+nof+ncn, from.size());

    // index will keep track of where we are in the "from"
    // vector.
    DoubleVector::size_type index = 0;

    // Interpret the first "ndv" entries as Design variables.
    DesignVariableInfoVector::const_iterator it(
        target.GetDesignVariableInfos().begin()
        );

    for(size_t i=0; index<end && i<ndv; ++i, ++it)
    {
        // The file contains values but the Design class stores
        // double representations.  So we must convert here.
        double rep = (*it)->GetNearestValidDoubleRep(from.at(index++));

        // if the rep is not valid, this design is illconditioned
        if(!(*it)->IsValidDoubleRep(rep)) into.SetIllconditioned(true);

        // continue on anyway and fill up the Design.
        into.SetVariableRep(i, rep);
    }

    // Next are the objective functions
    for(size_t i=0; index<end && i<nof; ++i)
        into.SetObjective(i, from.at(index++));

    // Finish with constraints.
    for(size_t i=0; index<end && i<ncn; ++i)
        into.SetConstraint(i, from.at(index++));
}

string
DesignFileReader::GetNextField(
    const string& from,
    const string& delim,
    string::size_type& off
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Check for trivial abort condition.
    if(off >= from.size()) return string();

    string::size_type beg = off;

    bool whiteDelim = delim == WHITESPACE_DELIMITER;
    // Find the next occurance of the delimiter.
    // If the delimiter is the whitespace delimiter, then search for the
    // next whitespace.
    off = whiteDelim ?
        from.find_first_of(ALL_WHITESPACE, off) : from.find(delim, off);

    string::size_type end = (off == string::npos) ? from.size() : off;
    off += (off == string::npos) ? 0 : whiteDelim ? 1 : delim.size();

    // return the substring consisting of everything from
    // the offset to the delimiter.
    return from.substr(beg, end-beg);
}


void
DesignFileReader::RecordViolations(
    const Design& des,
    const DesignTarget& target
    )
{
    EDDY_FUNC_DEBUGSCOPE

    ConstraintInfoVector cninfos = target.GetConstraintInfos();

    for(ConstraintInfoVector::const_iterator it(cninfos.begin());
        it!=cninfos.end(); ++it)
            (*it)->RecordViolation(des);
}

bool
DesignFileReader::IsAllWhite(
    const string& str
    )
{
    EDDY_FUNC_DEBUGSCOPE

    for(string::const_iterator it(str.begin()); it!=str.end(); ++it)
        if(!isspace(*it)) return false;

    return !str.empty();
}

string
DesignFileReader::TrimWhitespace(
    const string& str
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(str.empty() || (!isspace(str[0]) && !isspace(str[str.size()-1])))
        return str;

    string ret(str);
    size_t last = 0;
    while(isspace(ret[last])) ++last;
    if(last > 0) ret.erase(ret.begin(), ret.begin() + last);

    return ret.empty() ? ret :
        ret.erase(ret.find_last_not_of(ALL_WHITESPACE)+1);
}



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


string
DesignFileReader::InitializeWhiteString(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    string white(6, '0');
    white[0] = 0x09; // Horizontal Tab
    white[1] = 0x0A; // NL Line Feed, New Line
    white[2] = 0x0B; // vertical tab
    white[3] = 0x0C; // NP Form Feed, New Page
    white[4] = 0x0D; // Carriage Return
    white[5] = 0x20; // Space
    return white;

} // ExplodePredicate






/*
================================================================================
Structors
================================================================================
*/

DesignFileReader::Result::Result(
    ) :
        _filename("Unknown"),
        _designs(),
        _fullReads(0),
        _partReads(0),
        _attempts(0),
        _failures(0),
        _emptyLines(0),
        _error(false)
{
    EDDY_FUNC_DEBUGSCOPE

} // DesignFileReader::Result::Result

DesignFileReader::Result::Result(
    const DesignFileReader::Result& copy
    ) :
        _filename(copy._filename),
        _designs(copy._designs),
        _fullReads(copy._fullReads),
        _partReads(copy._partReads),
        _attempts(copy._attempts),
        _failures(copy._failures),
        _emptyLines(copy._emptyLines),
        _error(copy._error)
{
    EDDY_FUNC_DEBUGSCOPE

} // DesignFileReader::Result::Result

DesignFileReader::Result::~Result(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    _designs.clear();

} // DesignFileReader::Result::~Result







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Utilities
} // namespace JEGA
