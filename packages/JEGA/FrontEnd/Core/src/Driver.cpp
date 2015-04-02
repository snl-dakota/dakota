/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Implementation of class Driver.

    NOTES:

        See notes of Driver.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Fri Jan 06 07:40:17 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the Driver class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <ctime>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <../FrontEnd/Core/include/Driver.hpp>
#include <../FrontEnd/Core/include/ProblemConfig.hpp>
#include <../FrontEnd/Core/include/AlgorithmConfig.hpp>
#include <../FrontEnd/Core/include/EvaluatorCreator.hpp>

#include <../MOGA/include/MOGA.hpp>
#include <../SOGA/include/SOGA.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterDatabase.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>

#include <GeneticAlgorithmMutator.hpp>
#include <GeneticAlgorithmCrosser.hpp>
#include <GeneticAlgorithmMainLoop.hpp>
#include <GeneticAlgorithmSelector.hpp>
#include <GeneticAlgorithmEvaluator.hpp>
#include <GeneticAlgorithmConverger.hpp>
#include <GeneticAlgorithmInitializer.hpp>
#include <GeneticAlgorithmPostProcessor.hpp>
#include <GeneticAlgorithmFitnessAssessor.hpp>
#include <GeneticAlgorithmNichePressureApplicator.hpp>

#include <OperatorGroups/AllOperators.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;
using namespace eddy::utilities;
using namespace JEGA::Algorithms;




/*
================================================================================
Begin Namespace
================================================================================
*/
namespace JEGA {
    namespace FrontEnd {





/*
================================================================================
Static Member Data Definitions
================================================================================
*/
bool Driver::_initialized = false;
unsigned int Driver::_rSeed = 0;







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

unsigned int
Driver::GetRandomSeed(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return _rSeed;
}

const std::string&
Driver::GetGlobalLogFilename(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_LOGGING_IF_OFF(
        static const std::string ret;
        return ret;
        )
    JEGA_LOGGING_IF_ON(return Logger::Global().GetFilename();)
}

bool
Driver::IsJEGAInitialized(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return _initialized;
}







/*
================================================================================
Public Methods
================================================================================
*/
bool
Driver::InitializeJEGA(
    const string& globalLogFilename,
    const LogLevel& globalLogDefLevel,
    unsigned int rSeed,
    Logger::FatalBehavior onFatal
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(IsJEGAInitialized()) throw runtime_error(
        "Attempt to re-initialize JEGA is illegal.  InitializeJEGA can be "
        "called once and only once."
        );

    // The handle_signal method will give the debugscope project it's chance.
    // So there are no EDDY_DEBUGSIGNAL calls in this method.
    ::signal(SIGSEGV, &Driver::handle_signal);
    EDDY_IF_NO_WINDOWS(::signal(SIGINT, &Driver::handle_signal);)
    ::signal(SIGILL, &Driver::handle_signal);
    ::signal(SIGFPE, &Driver::handle_signal);
    ::signal(SIGTERM, &Driver::handle_signal);
    ::signal(SIGABRT, &Driver::handle_signal);

    // Start by initializing the logger
    JEGA_LOGGING_INIT(globalLogFilename, globalLogDefLevel);
    Logger::SetFatalBehavior(onFatal);

    // make sure the operator groups are instantiated.
    AllOperators::FullInstance();

    _rSeed =
        (rSeed == 0) ? static_cast<unsigned int>(clock() + time(0x0)) : rSeed;

    SeedRandomGenerator(_rSeed);

    return _initialized = true;
}

unsigned int
Driver::ReSeed(
    unsigned int rSeed
    )
{
    EDDY_FUNC_DEBUGSCOPE

    _rSeed =
        (rSeed == 0) ? static_cast<unsigned int>(clock() + time(0x0)) : rSeed;

    SeedRandomGenerator(_rSeed);

    return _rSeed;
}

void
Driver::ResetGlobalLoggingLevel(
    const JEGA::Logging::LogLevel& globalLogDefLevel
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(
        Logger::Global().Gate().set_default_level(globalLogDefLevel);
        )

    JEGA_LOGGING_IF_ON(
        Logger::Global().Gate().get_log().log(
            Logger::Global().GetName() +
            JEGA_LOGGING_AS_STRING_LITERAL(" default level reset to \"") +
            LevelClass::to_ostreamable(
                Logger::Global().Gate().get_default_level()
                ) +
            JEGA_LOGGING_AS_STRING_LITERAL("\".")
            );
        )

}

void
Driver::FlushGlobalLogStreams(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGA_LOGGING_IF_ON(Logger::Global().FlushStreams();)
}

DesignOFSortSet
Driver::ExecuteAlgorithm(
    const AlgorithmConfig& algConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    try
    {
        GeneticAlgorithm* theGA = this->ExtractAllData(algConfig);
        DesignOFSortSet ret(this->PerformIterations(theGA));
        this->DestroyAlgorithm(theGA);
        return ret;
    }
    catch(const exception& e)
    {
        JEGALOG_II_G_F(this,
            text_entry(lfatal(),
                "JEGA Front End Error: Exception caught at application "
                "level reading: \"") << e.what() << "\"."
            )
    }

    return DesignOFSortSet();
}

JEGA::Algorithms::GeneticAlgorithm*
Driver::InitializeAlgorithm(
    const AlgorithmConfig& algConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    GeneticAlgorithm* theGA = this->ExtractAllData(algConfig);

    JEGAIFLOG_CF_II_G_F(theGA == 0x0, this,
        text_entry(lfatal(),
            "JEGA Front End Error: Attempt to initialize a null algorithm.")
        )

    JEGAIFLOG_CF_II_G_F(!theGA->AlgorithmInitialize(), this,
        text_entry(lfatal(),
            "JEGA Front End Error: Unable to initialize the GA " +
            theGA->GetName() + '.')
        )

    return theGA;
}

bool
Driver::PerformNextIteration(
    JEGA::Algorithms::GeneticAlgorithm* theGA
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_CF_II_G_F(theGA == 0x0, this,
        text_entry(lfatal(),
            "JEGA Front End Error: Attempt to perform an iteration on a "
            "null algorithm.")
        )

    return theGA->AlgorithmProcess();
}

DesignOFSortSet
Driver::FinalizeAlgorithm(
    JEGA::Algorithms::GeneticAlgorithm* theGA
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_CF_II_G_F(theGA == 0x0, this,
        text_entry(lfatal(),
            "JEGA Front End Error: Attempt to finalize a null algorithm.")
        )

    theGA->AlgorithmFinalize();
    DesignOFSortSet ret(this->DeepDuplicate(theGA->GetCurrentSolution()));
    this->DestroyAlgorithm(theGA);
    return ret;
}

/*
================================================================================
Subclass Visible Methods
================================================================================
*/

GeneticAlgorithm*
Driver::ExtractAllData(
    const AlgorithmConfig& algConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    EDDY_DEBUG(!this->_initialized, "Must call Driver::InitializeJEGA prior to "
                                    "extracting data for an algorithm.")

    try
    {
        if(!this->_initialized)
        {
            cerr << "JEGA Front End Error: Attempt to run JEGA prior to call "
                    "to Driver::InitializeJEGA.  Please modify your code.\n";
            exit(4);
        }

        JEGAIFLOG_CF_II_G_F(this->_probConfig.GetDesignTarget().GetNOF() == 0,
            this,
            text_entry(lfatal(), "JEGA Front End Error: Attempt to run a JEGA "
                "algorithm with no objective functions defined.  You must "
                "create at least 1 objective.")
            )

        Logger* newLog = 0x0;

#ifdef JEGA_LOGGING_ON

        // prepare the logger to use for this algorithm.  If the logging
        // filename in the algConfig object is non-empty, then the logger used
        // is a new one created to log to the file.  Otherwise, it is
        // the global logger.

        // if we wind up creating a new log, it will have to be named.  See
        // if the algorithm was given a name and if so, use it.  Otherwise
        // call it "unnamed".

        // figure out if we will be creating a new Logger.  To do so, we will
        // need the supplied algorithm name and logging file name.
        string logfile(this->GetAlgorithmLogFilename(algConfig));
        string algname(this->GetAlgorithmName(algConfig));

        if(!logfile.empty())
        {
            string logName(
                (!algname.empty() ? algname : string("Unknown GA")) + " Logger"
                );

            newLog = new Logger(
                logfile, logName, GetAlgorithmDefaultLogLevel(algConfig)
                );
            newLog->Init();
        }

        Logger& tLog = (newLog == 0x0) ? Logger::Global() : *newLog;

#else

        Logger tLog;

#endif

        GeneticAlgorithm* theGA = this->CreateNewAlgorithm(algConfig, tLog);

        JEGAIFLOG_CF_II_G_F(theGA == 0x0, this,
            text_entry(lfatal(),
                "JEGA Front End Error: Unable to create a genetic algorithm.")
            )

        // now make use of the configuration object to load up the GA.
        this->LoadAlgorithm(*theGA, algConfig);

        // Add newLog to the list of logs that we are responsible for
        // destroyting iff it is non-null.
        if(newLog != 0x0)
            this->_myLogs.insert(GALoggerMap::value_type(theGA, newLog));

        // We put newLog in here because it only has a non-null value if it
        // needs to be destroyed at some later time.
        return theGA;
    }
    catch(const exception& e)
    {
        JEGALOG_II_G_F(this,
            text_entry(lfatal(),
                "JEGA Front End Error: Exception caught at application "
                "level reading: \"") << e.what() << "\"."
            )
        return 0x0;
    }
}

DesignOFSortSet
Driver::PerformIterations(
    JEGA::Algorithms::GeneticAlgorithm* theGA
    )
{
    EDDY_FUNC_DEBUGSCOPE

    try
    {
        // prepare to time this run.
        JEGA_LOGGING_IF_ON(clock_t start = clock();)

        theGA->AlgorithmInitialize();
        while(this->PerformNextIteration(theGA));
        theGA->AlgorithmFinalize();

        JEGA_LOGGING_IF_ON(
            double elapsed =
                static_cast<double>((clock() - start)) / CLOCKS_PER_SEC;
            )

        JEGALOG_II(theGA->GetLogger(), lquiet(), this,
            ostream_entry(
                lquiet(), "JEGA Front End: " + theGA->GetName() +
                " execution took ") << elapsed << " seconds."
            )

        JEGALOG_II_G(lquiet(), this,
            ostream_entry(lquiet(), "JEGA Front End: Execution took ")
            << elapsed << " seconds."
            )

        return this->DeepDuplicate(theGA->GetCurrentSolution());
    }
    catch(const exception& e)
    {
        JEGALOG_II_G_F(this,
            text_entry(lfatal(),
                "JEGA Front End Error: Exception caught at application "
                "level reading: \"") << e.what() << "\"."
            )
    }

    return DesignOFSortSet();
}

void
Driver::DestroyAlgorithm(
    GeneticAlgorithm* theGA
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Begin by finding theGA in the logger map.
    GALoggerMap::iterator loc(this->_myLogs.find(theGA));

    // Destroy the GA before destroying the log in case it wants to write
    // from the destructor.
    delete theGA;

    // if we found it, delete the associated log and remove the map entry.
    if(loc != this->_myLogs.end())
    {
        delete (*loc).second;
        this->_myLogs.erase(loc);
    }

}

string
Driver::GetAlgorithmName(
    const AlgorithmConfig& from
    )
{
    EDDY_FUNC_DEBUGSCOPE
    try {
        return from.GetParameterDB().GetString("method.jega.algorithm_name");
    }
    catch(...) { return string(); }
}

string
Driver::GetAlgorithmLogFilename(
    const AlgorithmConfig& from
    )
{
    EDDY_FUNC_DEBUGSCOPE
    try {
        return from.GetParameterDB().GetString("method.log_file");
    }
    catch(...) { return string(); }
}

string
Driver::GetAlgorithmType(
    const AlgorithmConfig& from
    )
{
    EDDY_FUNC_DEBUGSCOPE
    try {
        return from.GetParameterDB().GetString("method.algorithm");
    }
    catch(...) { return string(); }
}

LogLevel
Driver::GetAlgorithmDefaultLogLevel(
    const AlgorithmConfig& from
    )
{
    EDDY_FUNC_DEBUGSCOPE
    try {
        return static_cast<LogLevel>(
            from.GetParameterDB().GetIntegral("method.output")
            );
    }
    catch(...) { return LevelClass::Default; }
}

GeneticAlgorithm*
Driver::CreateNewAlgorithm(
    const AlgorithmConfig& algConfig,
    Logger& logger
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Start by trying to retrieve the algorithm type from the algorithm
    // config.
    AlgorithmConfig::AlgType algType = AlgorithmConfig::MOGA;

    string algTypeStr(this->GetAlgorithmType(algConfig));

    if(algTypeStr == "moga")       algType = AlgorithmConfig::MOGA;
    else if(algTypeStr == "soga")  algType = AlgorithmConfig::SOGA;

    else if(algTypeStr.empty()) JEGALOG_II_G_F(this,
        ostream_entry(lfatal(), "JEGA Front End Error: "
        "Algorithm type not supplied.")
        )
    else JEGALOG_II_G_F(this,
        ostream_entry(lfatal(), "JEGA Front End Error: "
        "Invalid algorithm type supplied: ") << algTypeStr
        )

    GeneticAlgorithm* theGA = 0x0;

    if(algType == AlgorithmConfig::MOGA)
    {
        JEGALOG_II_G(
            lverbose(), this, text_entry(lverbose(),
            "JEGA Front End: Creating a MOGA")
            )
        theGA = new MOGA(this->_probConfig.GetDesignTarget(), logger);
    }

    else if(algType == AlgorithmConfig::SOGA)
    {
        JEGALOG_II_G(
            lverbose(), this, text_entry(lverbose(),
            "JEGA Front End: Creating a SOGA")
            )
        theGA = new SOGA(this->_probConfig.GetDesignTarget(), logger);
    }
    else
        JEGALOG_II_G_F(this,
            ostream_entry(lfatal(), "JEGA Front End Error: "
                "Unknown algorithm type: ") << algType
            )

    return theGA;
}

void
Driver::SeedRandomGenerator(
    unsigned int seed
    )
{
    EDDY_FUNC_DEBUGSCOPE
    RandomNumberGenerator::Seed(seed);

    JEGALOG_IT_G(lquiet(), Driver,
        ostream_entry(lquiet(), "JEGA Front End: Random seed = ") << seed
        )
}

void
Driver::LoadAlgorithm(
    GeneticAlgorithm& theGA,
    const AlgorithmConfig& algConfig
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Store the parameter database for repeated use by this method
    const ParameterDatabase& pdb = algConfig.GetParameterDB();

    // begin by creating the operators and loading them into an operator
    // set.  The set will be validated by the GeneticAlgorithm after it
    // is completely loaded.  The Set<operator> methods put the newly created
    // operators into the supplied set.
    GeneticAlgorithmOperatorSet theOps(theGA);

    SetConverger(pdb.GetString("method.jega.convergence_type"), theOps);
    SetCrosser(pdb.GetString("method.crossover_type"), theOps);
    SetNichePressureApplicator(
        pdb.GetString("method.jega.niching_type"), theOps
        );
    SetFitnessAssessor(pdb.GetString("method.fitness_type"), theOps);
    SetInitializer(pdb.GetString("method.initialization_type"), theOps);
    SetMainLoop(pdb.GetString("method.jega.mainloop_type"), theOps);
    SetMutator(pdb.GetString("method.mutation_type"), theOps);
    SetSelector(pdb.GetString("method.replacement_type"), theOps);
    SetPostProcessor(
        pdb.GetString("method.jega.postprocessor_type"), theOps
        );

    // handle the evaluator specially b/c it is not in a group.
    EvaluatorCreator& ector = algConfig.GetTheEvaluatorCreator();
    GeneticAlgorithmEvaluator* evaler = ector.CreateEvaluator(theGA);
    VerifyValidOperator(evaler, "Evaluator", "Custom Evaluator");
    theOps.SetEvaluator(evaler);

    // now inform the genetic algorithm of the operator set and let it
    // determine if it is a valid set.
    JEGAIFLOG_CF_IT_G_F(!theGA.SetOperatorSet(theOps), Driver,
        text_entry(lfatal(), "JEGA Front End Error: Failed to match a group "
            "to the chosen operators.")
        );

    // Now get the parameters extracted for all operators and the GA.
    theGA.ExtractParameters(algConfig.GetParameterDB());
}

void
Driver::VerifyValidOperator(
    GeneticAlgorithmOperator* op,
    const string& type,
    const string& name
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGAIFLOG_CF_IT_G_F(op == 0x0, Driver,
        text_entry(lfatal(), "JEGA Front End Error: Unable to resolve ")
            << type << " \"" << name << "\""
        );
}

#define SET_OP_METHOD(optype) \
    GeneticAlgorithm##optype* \
    Driver::Set##optype( \
        const string& name, \
        GeneticAlgorithmOperatorSet& into \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        GeneticAlgorithm##optype* op = \
            AllOperators::FullInstance().Get##optype( \
                name, into.GetAlgorithm() \
                ); \
        VerifyValidOperator(op, #optype, name); \
        into.Set##optype(op); \
        return op; \
    }

SET_OP_METHOD(Converger)
SET_OP_METHOD(Crosser)
SET_OP_METHOD(FitnessAssessor)
SET_OP_METHOD(Initializer)
SET_OP_METHOD(MainLoop)
SET_OP_METHOD(Mutator)
SET_OP_METHOD(Selector)
SET_OP_METHOD(PostProcessor)
SET_OP_METHOD(NichePressureApplicator)


DesignOFSortSet
Driver::DeepDuplicate(
    const DesignOFSortSet& from,
    bool moveTags
    )
{
    EDDY_FUNC_DEBUGSCOPE

    DesignOFSortSet ret;
    if(from.empty()) return ret;

    const DesignTarget& target = from.front()->GetDesignTarget();

    DesignOFSortSet::const_iterator it(from.begin());
    DesignOFSortSet::iterator where(ret.end());

    for(; it!=from.end(); ++it)
    {
        const Design& toCopy = **it;
        Design* des = target.GetNewDesign(toCopy);
        if(moveTags)
        {
            des->SetTag(toCopy.GetTag());
            const_cast<Design&>(toCopy).SetTag(0x0);
        }
        where = ret.insert(where, des);
    }

    return ret;
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

void
Driver::handle_signal(
    int val
    ) throw()
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_OFF(return;)

    ostream_entry ose(lsilent(), "JEGA Front End: signal caught: value = ");
    ose << val << ' ';

    switch(val)
    {
        case SIGSEGV: ose << "(SIGSEGV)"; break;
        case SIGINT : ose << "(SIGINT)"; break;
        case SIGFPE : ose << "(SIGFPE)"; break;
        case SIGILL : ose << "(SIGILL)"; break;
        case SIGTERM: ose << "(SIGTERM)"; break;
        case SIGABRT: ose << "(SIGABRT)"; break;
        default: ose << "(UNKNOWN)";
    }

    JEGALOG_G(lsilent(), ose);

    // Now, if we have debugging enabled, let it do it's thing.
#ifdef JEGA_OPTION_DEBUG
    eddy::utilities::EDDY_Debug::_Signal(val);
#endif
}







/*
================================================================================
Structors
================================================================================
*/


Driver::Driver(
    const ProblemConfig& probConfig
    ) :
        _probConfig(probConfig)
{
    EDDY_FUNC_DEBUGSCOPE
}

Driver::~Driver(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If there is anything left in the map, clean it out.  The call to
    // DestroyAlgorithm will remove an entry from the _myLogs map.
    while(!this->_myLogs.empty())
        DestroyAlgorithm((*this->_myLogs.begin()).first);
}




/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace FrontEnd
} // namespace JEGA

