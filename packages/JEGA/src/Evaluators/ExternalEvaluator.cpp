/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class ExternalEvaluator.

    NOTES:

        See notes of ExternalEvaluator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        2.1.0

    CHANGES:

        Thu Nov 09 14:54:09 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the ExternalEvaluator class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <string>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <../Utilities/include/Logging.hpp>
#include <Evaluators/ExternalEvaluator.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
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
const string ExternalEvaluator::DEFAULT_OUT_PATTERN("params#.out");
const string ExternalEvaluator::DEFAULT_IN_PATTERN ("results#.in");




/*
================================================================================
Mutators
================================================================================
*/

void
ExternalEvaluator::SetExecutableName(
    const std::string& exeName
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_CF_II(exeName.empty(), this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": Attempt to set the "
            "executable name to an empty string failed.  Keeping the current "
            "value of " + this->_exeName)
        )

    this->_exeName = exeName;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The executable name is "
        "now " + this->_exeName)
        )
}

void
ExternalEvaluator::SetOutputFilenamePattern(
    const std::string& outPattern
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_outPattern = outPattern;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The output filename "
            "pattern is now " + this->_outPattern)
        )
}

void
ExternalEvaluator::SetInputFilenamePattern(
    const std::string& inPattern
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_inPattern = inPattern;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The input filename "
            "pattern is now " + this->_inPattern)
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
ExternalEvaluator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("external");
    return ret;
}

const string&
ExternalEvaluator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This evaluator performs evaluation by calling to an external program "
        "via system calls.  Transmission of data is performed using the file "
        "system.  This evaluator must be provided with the name of the "
        "executable to call and should be provided with patterns on which to "
        "base the names of the input and output files.  Any \"#\"'s in the "
        "pattern will be replaced by the number of the evaluation.  For "
        "example, if using an executable called \"myevaler\", a parameter "
        "file name pattern of \"params#.out\", and a results file pattern of "
        "\"results#.out\", calls to myevaler will look like this:\n\n"
        "     myevaler params0.out results0.out\n\n"
        "params0.out will already exist by the time this call is made and is "
        "where myevaler should look for design variable values (1 per line in "
        "the order in which they were described to the target.  results0.out "
        "is the name of the file that myevaler should create and write "
        "response values into.  The responses must be objectives followed by "
        "constraints each in the order in which they were described to the "
        "target.\n\n"
        "This evaluator respects the potential for evaluation concurrency and "
        "so myevaler may get called multiple times asynchronously.  This will "
        "usually not require any consideration on the part of myevaler.  An "
        "exception would be if it used shared resources that must be "
        "synchronized.  Of course you can always just set the evaluation "
        "concurrency to 1 if that is the case and then not have to worry "
        "about it at all."
        );
    return ret;
}

GeneticAlgorithmOperator*
ExternalEvaluator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new ExternalEvaluator(algorithm);
}










/*
================================================================================
Subclass Visible Methods
================================================================================
*/

string
ExternalEvaluator::GetOutputFilename(
    size_t lbRepl
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return ReplaceAllOccurrances('#', this->_outPattern, lbRepl);
}

string
ExternalEvaluator::GetInputFilename(
    size_t lbRepl
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return ReplaceAllOccurrances('#', this->_inPattern, lbRepl);
}




/*
================================================================================
Subclass Overridable Methods
================================================================================
*/


string
ExternalEvaluator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return ExternalEvaluator::Name();
}

string
ExternalEvaluator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return ExternalEvaluator::Description();
}

GeneticAlgorithmOperator*
ExternalEvaluator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new ExternalEvaluator(*this, algorithm);
}

bool
ExternalEvaluator::Evaluate(
    JEGA::Utilities::DesignGroup& group
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool haveLbs = this->_inPattern.find('#') != string::npos &&
                   this->_outPattern.find('#') != string::npos;

    bool modEC = this->GetEvaluationConcurrency() > 1 && !haveLbs;

    JEGAIFLOG_CF_II(modEC, this->GetLogger(),
        lquiet(), this, text_entry(lquiet(), this->GetName() + ": Evaluation "
            "concurrency is greater than 1 but one or both of the supplied "
            "filenames have no wildcards(#).  The concurrency will be reduced "
            "to 1.")
        )

    if(modEC) this->SetEvaluationConcurrency(1);

    return GeneticAlgorithmEvaluator::Evaluate(group);
}

bool
ExternalEvaluator::Evaluate(
    Design& des
    )
{
    EDDY_FUNC_DEBUGSCOPE

    const DesignTarget& target = this->GetDesignTarget();

    const size_t ndv = target.GetNDV();
    const size_t nof = target.GetNOF();
    const size_t ncn = target.GetNCN();

    // Create the next output and input filenames.
    JEGA_IF_THREADSAFE(this->_currEvalMutex.lock();)
    string outFName(this->GetOutputFilename(this->_evalNum));
    string inFName (this->GetInputFilename (this->_evalNum));
    ++this->_evalNum;
    JEGA_IF_THREADSAFE(this->_currEvalMutex.unlock();)

    // now actually write the output file, 1 variable to a line.
    ofstream ofile(outFName.c_str());

    JEGAIFLOG_CF_II_F(!ofile.is_open(), this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": Unable to open file \""
            + outFName + "\" for writing.  aborting.")
            )

    for(size_t i=0; i<ndv; ++i) ofile << des.GetVariableValue(i) << '\n';
    ofile.close();

    // now issue the system call to perform the evaluation.
    string cmd(this->GetExecutableName() + " " + outFName + " " + inFName);

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": Issuing an "
            "evaluation command line: \"" + cmd + "\".")
            )

    int success = system(cmd.c_str());

    JEGAIFLOG_CF_II_F(success != 0, this->GetLogger(), this,
        ostream_entry(lfatal(), this->GetName() + ": A system call to "
            "perform an evaluation returned a failure code with value ")
            << success << ".  errno is currently " << errno << " ("
            << strerror(success) << ").  The command line was \"" +
               cmd + "\"."
            )

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": \"" +  cmd +
            "\" returned ") << success << "."
            )

    // If we make it here, the input file should exist and have our results
    // in it.  Open it up and get them.
    ifstream ifile(inFName.c_str());

    JEGAIFLOG_CF_II_F(!ifile.is_open(), this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": Unable to open file \""
            + inFName + "\" for reading.")
            )

    // it opened so read it and record the results in the design.  If we
    // don't find enough, issue a logging message and mark this design
    // illconditioned.
    size_t ind = 0;
    for(ind=0; ind<nof && !ifile.eof(); ++ind)
    {
        double temp = 0.0;
        JEGAIFLOG_CF_II_F(!(ifile >> temp), this->GetLogger(), this,
            text_entry(lfatal(), GetName() + ": Failed to extract "
                "a value of type double from \"" + inFName + "\".  Make sure "
                "your results files do not contain anything but the responses "
                "(no extraneous text).")
                )
        des.SetObjective(ind, temp);
    }

    // if we didn't get all objectives, we are illconditioned.
    if(ind != nof)
    {
        des.SetIllconditioned(true);

        JEGALOG_II(this->GetLogger(), lquiet(), this,
            ostream_entry(lquiet(), this->GetName() + ": Found ")
                << ind << " objectives in response file \"" << inFName
                << "\" which is fewer than the required " << nof
                << ".  The current design is being marked illconditioned "
                   "and will be discarded."
                )

    }

    // otherwise, read in the constraints.
    else
    {
        for(ind=0; ind<ncn && !ifile.eof(); ++ind)
        {
            double temp = 0.0;
            JEGAIFLOG_CF_II_F(!(ifile >> temp), this->GetLogger(), this,
                text_entry(lfatal(), this->GetName() + ": Failed to "
                    "extract a value of type double from \"" + inFName + "\". "
                    " Make sure your results files do not contain anything "
                    "but the responses (no extraneous text).")
                    )
            des.SetConstraint(ind, temp);
        }

        // if we didn't get all constraints, we are illconditioned.
        if(ind != ncn)
        {
            des.SetIllconditioned(true);

            JEGALOG_II(this->GetLogger(), lquiet(), this,
                ostream_entry(lquiet(), this->GetName() + ": Found ")
                    << ind << " constraints in response file \"" << inFName
                    << "\" which is fewer than the required " << ncn
                    << ".  The current design is being marked illconditioned "
                       "and will be discarded."
                    )
        }
    }

    return GeneticAlgorithmEvaluator::PostEvaluate(des);
}

bool
ExternalEvaluator::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    bool success = ParameterExtractor::GetStringFromDB(
        db, "method.jega.exe_name", this->_exeName
        );

    // If we did not find the executable name, it is an error.  We cannot
    // proceed without it since there is no reasonable default.
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": The executable name was not "
            "found in the parameter database.  One must be supplied before any "
            "evaluations can occur.")
        )

    this->SetExecutableName(this->_exeName);

    success = ParameterExtractor::GetStringFromDB(
        db, "method.jega.out_file_pat", this->_outPattern
        );

    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The output filename "
            "pattern was not found in the parameter database.  "
            "Using the current value of " + this->_outPattern)
        )

    this->SetOutputFilenamePattern(this->_outPattern);

    success = ParameterExtractor::GetStringFromDB(
        db, "method.jega.in_file_pat", this->_inPattern
        );

    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The input filename pattern "
            "was not found in the parameter database.  "
            "Using the current value of " + this->_inPattern)
        )

    SetInputFilenamePattern(_inPattern);

    // do base class extraction and then do some more sanity checking.
    success = GeneticAlgorithmEvaluator::PollForParameters(db);

    bool haveLbs = this->_inPattern.find('#') != string::npos &&
                   this->_outPattern.find('#') != string::npos;

    JEGAIFLOG_CF_II(this->GetEvaluationConcurrency() > 1 && !haveLbs,
        this->GetLogger(), lquiet(), this,
        text_entry(lquiet(), this->GetName() + ": Evaluation "
            "concurrency is greater than 1 but one or both of the supplied "
            "filenames have no wildcards(#).  If no wildcards are present in "
            "either of the input (" + this->_inPattern + ") or output (" +
            this->_outPattern + ") filenames when a group evaluation is "
            "attempted, the concurrency will be reduced to 1.")
        )

    return true;
}





/*
================================================================================
Private Methods
================================================================================
*/

string
ExternalEvaluator::ReplaceAllOccurrances(
    char of,
    const string& in,
    eddy::utilities::uint64_t with
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ostringstream ostr;

    for(string::const_iterator it(in.begin()); it!=in.end(); ++it)
    {
        if(*it == of) ostr << with;
        else ostr << *it;
    }

    return ostr.str();
}







/*
================================================================================
Structors
================================================================================
*/

ExternalEvaluator::ExternalEvaluator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmEvaluator(algorithm),
        _exeName(),
        _outPattern(DEFAULT_OUT_PATTERN),
        _inPattern(DEFAULT_IN_PATTERN),
        _evalNum(0) EDDY_COMMA_IF_THREADSAFE
        EDDY_INIT_MUTEX(_currEvalMutex, PTHREAD_MUTEX_RECURSIVE)
{
    EDDY_FUNC_DEBUGSCOPE
}

ExternalEvaluator::ExternalEvaluator(
    const ExternalEvaluator& copy
    ) :
        GeneticAlgorithmEvaluator(copy),
        _exeName(copy._exeName),
        _outPattern(copy._outPattern),
        _inPattern(copy._inPattern),
        _evalNum(copy._evalNum) EDDY_COMMA_IF_THREADSAFE
        EDDY_INIT_MUTEX(_currEvalMutex, PTHREAD_MUTEX_RECURSIVE)
{
    EDDY_FUNC_DEBUGSCOPE
}

ExternalEvaluator::ExternalEvaluator(
    const ExternalEvaluator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmEvaluator(copy, algorithm),
        _exeName(copy._exeName),
        _outPattern(copy._outPattern),
        _inPattern(copy._inPattern),
        _evalNum(copy._evalNum) EDDY_COMMA_IF_THREADSAFE
        EDDY_INIT_MUTEX(_currEvalMutex, PTHREAD_MUTEX_RECURSIVE)
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

