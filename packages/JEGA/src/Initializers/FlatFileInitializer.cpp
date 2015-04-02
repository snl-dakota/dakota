/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implemenation of class FlatFileInitializer.

    NOTES:

        See notes of FlatFileInitializer.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Jun 26 12:47:50 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the FlatFileInitializer class.
 */



/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <../Utilities/include/Logging.hpp>
#include <Initializers/FlatFileInitializer.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Initializers/RandomUniqueInitializer.hpp>
#include <../Utilities/include/DesignFileReader.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>





/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA;
using namespace JEGA::Logging;
using namespace eddy::utilities;
using namespace JEGA::Utilities;








/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace Algorithms {








/*
================================================================================
Static Member Data Definitions
================================================================================
*/
const std::string FlatFileInitializer::DEFAULT_DELIM;







/*
================================================================================
Mutators
================================================================================
*/
void
FlatFileInitializer::SetFileNames(
    const JEGA::StringSet& fileNames
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_fileNames = fileNames;
    JEGA_LOGGING_IF_ON(this->LogFilenames());
}

void
FlatFileInitializer::SetDelimiter(
    const std::string& delim
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_delim = delim;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(lverbose(),
            this->GetName() + ": Delimiter now = " +
            (this->_delim.empty() ? "<EMPTY STRING>" : this->_delim))
        )

    JEGAIFLOG_CF_II(this->_delim.empty(), this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": Empty delimiter received. "
            "As a result, the automatic delimiter discovery algorithm "
            "will be employed."
            )
        )
}









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

const string&
FlatFileInitializer::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("flat_file");
    return ret;
}

const string&
FlatFileInitializer::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This initializer attempts to read from a delimited file with the "
        "following format:\n\n"
        "dv0<delim>dv1...dvN[<delim>of0<delim>of1...ofM<delim>con0<delim>con1"
        "...conK]\n\n"
        "The delimeter can be any string.  It will continue to read until "
        "the end of the file.  It will discard any configurations for which "
        "it was unable to retrieve at least the number of design variables.  "
        "The objective and constraint entries are not required but if all are "
        "present, they will be recorded and the Design will be tagged as "
        "evaluated so that evaluators may choose not to re-evaluate them.\n\n"
        "Setting the size for this initializer has the effect of requiring a "
        "minimum number of Designs to create.  If this minimum number has not "
        "been created once the files are all read, the rest are created using "
        "the random unique initializer."
        );
    return ret;
}

GeneticAlgorithmOperator*
FlatFileInitializer::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new FlatFileInitializer(algorithm);
}

bool
FlatFileInitializer::AddFileName(
    const string& fileName
    )
{
    EDDY_FUNC_DEBUGSCOPE
    bool ret = this->_fileNames.insert(fileName).second;
    JEGA_LOGGING_IF_ON(this->LogFilenames());
    return ret;
}








/*
================================================================================
Subclass Visible Methods
================================================================================
*/
JEGA::StringSet
FlatFileInitializer::ParseFileNames(
    const string& from
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA::StringSet ret;

    // if from is empty, there is nothing we can do.
    if(from.empty()) return ret;

    // place to store each filename as we encounter it
    string fname;

    // stores our current location in "from"
    string::size_type strt = 0;

    // extract the first value as a string.
    fname.assign(this->GetNextField(from, "\t", strt));

    // go for as long as we can get filenames.
    while(!fname.empty())
    {
        // otherwise record the value
        JEGA_LOGGING_IF_ON(bool inserted = ret.insert(fname).second;)
        JEGA_LOGGING_IF_OFF(ret.insert(fname);)

        JEGAIFLOG_CF_II(!inserted, this->GetLogger(), lquiet(), this,
            text_entry(lquiet(), this->GetName() + ": Filename \"" + fname +
                        "\" found more than once.  Repetitions ignored.")
            )

        // update the start to one past the most recent end
        strt += fname.size() + 1;

        // extract the next value as a string.
        fname.assign(this->GetNextField(from, strt));
    }

    return ret;
}

JEGA::StringSet
FlatFileInitializer::ParseFileNames(
    const JEGA::StringSet& from
    )
{
    EDDY_FUNC_DEBUGSCOPE
    StringSet ret;

    for(StringSet::const_iterator it(from.begin()); it!=from.end(); ++it)
    {
        StringSet curr(this->ParseFileNames(*it));
        ret.insert(curr.begin(), curr.end());
    }

    return ret;
}

string
FlatFileInitializer::GetNextField(
    const string& from,
    const string& delim,
    const string::size_type off
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Check for trivial abort condition.
    if(off >= from.size()) return string();

    // Find the next occurance of the delimiter.
    string::size_type end = from.find(delim, off);

    // return the substring consisting of everything from
    // the offset to the delimiter.
    return from.substr(off, end-off);
}

string
FlatFileInitializer::GetNextField(
    const string& from,
    const string::size_type off
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetNextField(from, this->_delim, off);
}


/*
================================================================================
Subclass Overridable Methods
================================================================================
*/

string
FlatFileInitializer::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return FlatFileInitializer::Name();
}

string
FlatFileInitializer::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return FlatFileInitializer::Description();
}

GeneticAlgorithmOperator*
FlatFileInitializer::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new FlatFileInitializer(*this, algorithm);
}


void
FlatFileInitializer::Initialize(
    DesignGroup& into
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing initialization.")
        )

    // get the target for the designs we create.
    DesignTarget& target = into.GetDesignTarget();

    // find out the requested minimum number of Designs
    const std::size_t minsize = this->GetSize();

    JEGA_LOGGING_IF_ON(const std::size_t prevSize = into.GetSize();)

    // read in each file one at a time
    for(StringSet::const_iterator it(this->_fileNames.begin());
        it!=this->_fileNames.end(); ++it)
    {
        DesignFileReader::Result readResults(
            DesignFileReader::ReadFlatFile(*it, this->_delim, target)
            );

        if(readResults.GetErrorFlag())
        {
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(lquiet(), this->GetName() + ": Encountered fatal "
                    "error while attempting to read file \"" + *it + "\".  "
                    "Make sure the file exists and is a JEGA Design flat file."
                    )
                )
            continue;
        }
        else into.AbsorbDesigns(readResults.GetDesigns());

        // indicate what happened
        JEGALOG_II(this->GetLogger(), lverbose(), this,
            text_entry(
                lverbose(), this->GetName() + ": \n" +
                readResults.GetResultsString()
                )
            )

    }

    JEGAIFLOG_CF_II(into.IsEmpty(), this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": No designs were read from "
            "initialization files.  All initial designs will be "
            "generated by the random unique initializer.")
        )

    // now we have to make sure that we have enough according to the
    // requested minimum size.
    if(into.GetSize() < minsize)
    {
        // we will use the random unique initializer to fill
        // in the rest.

        JEGALOG_II(this->GetLogger(), lverbose(), this,
            text_entry(lverbose(), this->GetName() + ": The desired number of "
                "designs were not found in the supplied file(s).  Using the "
                "random unique initializer to generate the remaining designs."
                )
            )

        RandomUniqueInitializer subiniter(this->GetAlgorithm());
        subiniter.SetSize(minsize - into.GetSize());
        subiniter.Initialize(into);
    }

    JEGAIFLOG_CF_II_F(into.IsEmpty(), this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": Unable to generate any "
            "initial designs even with the help of the random unique "
            "initializer.")
        )

    JEGAIFLOG_CF_II_F(into.GetSize() < 2, this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": Unable to generate minimum "
            "of 2 initial designs even with the help of the random "
            "unique initializer.")
        )

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": ")
            << (into.GetSize() - prevSize)
            << " total designs read or created."
        )

    JEGALOG_II(this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(),
            this->GetName() + ": Final initial population size: ")
            << into.GetSize() << "."
        )

    // Set the size to the number of successful reads.
    this->SetSize(into.GetSize());
}

bool
FlatFileInitializer::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Get the delimiter
    bool success = ParameterExtractor::GetStringFromDB(
        db, "method.jega.initializer_delimiter", _delim
        );

    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The field delimiter was "
            "not found in the parameter database.  Using the current " +
            "delimiter " +
            (this->_delim.empty() ? "<EMPTY STRING>" : this->_delim) + ".")
        )

    this->SetDelimiter(this->_delim);

    // The user may have given us a single filename or multiple or both!  First
    // extract the single name if present.
    string tstr;
    success = ParameterExtractor::GetStringFromDB(db, "method.flat_file", tstr);

    // Create a temporary set and initialize it with our filename.  Don't parse
    // it yet.  That will happen in one step when we have collected all our
    // filenames.
    JEGA::StringSet fnames;
    if(success) fnames.insert(tstr);

    // Now get the multiple names and merge them with our existing name.
    JEGA::StringVector sfnames;
    success = ParameterExtractor::GetStringVectorFromDB(
        db, "method.flat_files", sfnames
        );

    if(success) fnames.insert(sfnames.begin(), sfnames.end());

    // Now parse all the items in fnames and make that our new list of files.
    fnames = this->ParseFileNames(fnames);

    JEGAIFLOG_CF_II(fnames.empty(), this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": No filenames obtained for "
            "initialization.")
        )

    // Even if no filenames have been supplied, continue.  The initialization
    // will just use the random unique for all of them.
    this->SetFileNames(fnames);

    return this->GeneticAlgorithmInitializer::PollForParameters(db);
}






/*
================================================================================
Private Methods
================================================================================
*/
void
FlatFileInitializer::LogFilenames(
    ) const
{
#ifdef JEGA_LOGGING_ON
    if(this->GetLogger().Gate().will_log(this, lverbose()))
    {
        string allFiles;

        StringSet::const_iterator e(this->_fileNames.end());
        if(!this->_fileNames.empty()) --e;
        for(StringSet::const_iterator it(this->_fileNames.begin()); it!=e; ++it)
            allFiles.append(*it + ",");

        if(!this->_fileNames.empty()) allFiles.append(*e);

        JEGALOG_II(this->GetLogger(), lverbose(), this,
            text_entry(lverbose(), this->GetName() +
                ": Initialization file(s) now = " + allFiles)
            )
    }
#endif

}








/*
================================================================================
Structors
================================================================================
*/

FlatFileInitializer::FlatFileInitializer(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmInitializer(algorithm),
        _delim(DEFAULT_DELIM),
        _fileNames()
{
    EDDY_FUNC_DEBUGSCOPE

    // override the default requested initial size to zero.
    // For this initializer, this is the min size.  A value of
    // 0 will result in using only the Designs found in the file.
    this->SetSize(0);
}

FlatFileInitializer::FlatFileInitializer(
    const FlatFileInitializer& copy
    ) :
        GeneticAlgorithmInitializer(copy),
        _delim(copy._delim),
        _fileNames(copy._fileNames)
{
    EDDY_FUNC_DEBUGSCOPE
}

FlatFileInitializer::FlatFileInitializer(
    const FlatFileInitializer& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmInitializer(copy, algorithm),
        _delim(copy._delim),
        _fileNames(copy._fileNames)
{
    EDDY_FUNC_DEBUGSCOPE
}








/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
