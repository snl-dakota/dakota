/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class GeneticAlgorithm.

    NOTES:

        See notes of GeneticAlgorithm.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu May 15 08:25:23 2003 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the GeneticAlgorithm class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <ctime>
#include <cfloat>
#include <fstream>
#include <FitnessRecord.hpp>
#include <GeneticAlgorithm.hpp>
#include <GeneticAlgorithmMutator.hpp>
#include <GeneticAlgorithmCrosser.hpp>
#include <GeneticAlgorithmMainLoop.hpp>
#include <GeneticAlgorithmSelector.hpp>
#include <GeneticAlgorithmConverger.hpp>
#include <GeneticAlgorithmEvaluator.hpp>
#include <GeneticAlgorithmInitializer.hpp>
#include <../Utilities/include/Logging.hpp>
#include <GeneticAlgorithmFitnessAssessor.hpp>
#include <PostProcessors/NullPostProcessor.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/DesignGroupVector.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <GeneticAlgorithmNichePressureApplicator.hpp>


/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
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
File Scope Helper Functions
================================================================================
*/
/// Creates a string from the argument \a val using an ostringstream.
/**
 * \param val The value of type T to convert to a string.
 * \return The string representation of \a val created using an ostringstream.
 */
template <typename T>
string
asstring(
    const T& val
    )
{
    EDDY_FUNC_DEBUGSCOPE
    ostringstream ostr;
    ostr << val;
    return ostr.str();
}

/**
 * \brief Replaces all occurances of some search string within a source string
 *        with a supplied replacement string.
 *
 * \param of The string to search for.
 * \param in The string in which to search.
 * \param with The string with which to replace all occurances of \a of in
 *             \a in.
 */
string
ReplaceAllOccurances(
    const string& of,
    string in,
    const string& with
    )
{
    EDDY_FUNC_DEBUGSCOPE

    string::size_type cpos = in.find(of);

    while(cpos != string::npos)
    {
        in.replace(cpos, of.size(), with);
        cpos = in.find(of, cpos + with.size());
    }

    return in;
}



/*
================================================================================
Private Template Method Implementations
================================================================================
*/

template<typename Op_T>
bool
GeneticAlgorithm::SetOperator(
    Op_T* op,
    Op_T& (GeneticAlgorithmOperatorSet::*getFunc)(), // Get
    void (GeneticAlgorithmOperatorSet::*setFunc)(Op_T* op),
    bool groupHasOp,
    const std::string& JEGA_LOGGING_IF_ON(opType)
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // See if the current group has the operator.  If it does, we can go ahead
    // and use the supplied operator without any futher considerations.
    if(groupHasOp)
    {
        (this->GetOperatorSet().*setFunc)(op);
        return true;
    }

    // if it doesn't, we have to see if we can find a group that does.
    // Begin by storing the current converger.
    Op_T& current = (this->GetOperatorSet().*getFunc)();

    // now put the new converger in the set so we can match the it.
    (this->GetOperatorSet().*setFunc)(op);

    const GeneticAlgorithmOperatorGroup* match =
        this->MatchGroup(this->GetOperatorSet());

    // now, warn if we had no success matching a group.
    if(match == 0x0)
    {
        JEGALOG_II(this->GetLogger(), lquiet(), this, text_entry(lquiet(),
            "Cannot set " + opType + " to " + op->GetName() +
            ".  It is incompatable with the other existing operators "
            "according to the known groups.  Retaining current " + opType +
            " of " + current.GetName())
            )
        (this->GetOperatorSet().*setFunc)(&current);
        return false;
    }

    // we matched a group.  Adopt the new group and warn about it.
    JEGALOG_II(this->GetLogger(), lquiet(), this, text_entry(lquiet(),
        this->GetName() + ": Matched the modified set to group " +
        match->GetName() + " in response to a set " + opType + " call."))

    this->SetOperatorGroup(*match);

    return true;
}



/*
================================================================================
Static Member Data Definitions
================================================================================
*/
eddy::utilities::uint64_t GeneticAlgorithm::_instanceCt = 0;







/*
================================================================================
Mutators
================================================================================
*/

void
GeneticAlgorithm::SetCurrentFitnesses(
    const FitnessRecord* ftns
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->_lastFtns.reset(ftns);
}

void
GeneticAlgorithm::SetName(
    const std::string& name
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(name.empty())
    {
        if(this->_name.empty())
        {
            this->_name = GetDefaultName();
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(lquiet(), this->_name + ": Empty name supplied to "
                    "unnamed GA.  Using default name of " + this->_name + "."
                    )
                )
        }
        else
        {
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(lquiet(), this->_name + ": Attempt to rename GA "
                    "from " + this->_name + " to an empty string failed.  "
                    "Keeping the existing name"
                    )
                )
        }
    }
    else this->_name = name;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(
            lverbose(), this->_name + ": The name of this GA is now " +
            this->_name
            )
        )
}

void
GeneticAlgorithm::SetFinalDataFilename(
    const std::string& name
    )
{
    EDDY_FUNC_DEBUGSCOPE

    if(name.empty())
    {
        if(this->_finalDataFile.empty())
        {
            this->_finalDataFile = "finaldata#.dat";
            JEGALOG_II(this->GetLogger(), lquiet(), this,
                text_entry(lquiet(), this->_name + ": Empty name supplied for "
                    "final data filename.  Using default name of " +
                    this->_finalDataFile + "."
                    )
                )
        }
    }
    else this->_finalDataFile = name;

    this->_finalDataFile.assign(
        ReplaceAllOccurances(
            "#", this->_finalDataFile, asstring(this->_instanceNum)
            )
        );

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The name of the file to "
            "which final data will be written is now " + this->_finalDataFile)
        )
}

void
GeneticAlgorithm::SetPrintFinalData(
    bool print
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_printFinalData = print;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() +
            ": The print final data flag is now set to ")
            << this->_printFinalData
        )
}

void
GeneticAlgorithm::SetDataDirectory(
    const std::string& dir
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_dataDir = dir;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() +
            ": The data directory is now set to ")
            << this->_dataDir
        )
}

void
GeneticAlgorithm::SetPrintDiscards(
    bool print
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_printDiscards = print;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() +
            ": The print discards flag is now set to ")
            << this->_printDiscards
        )
}


void
GeneticAlgorithm::SetPrintEachPopulation(
    bool print
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_printPopEachGen = print;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() +
            ": The print each population flag is now set to ")
            << this->_printPopEachGen
        )
}






/*
================================================================================
Accessors
================================================================================
*/
const FitnessRecord&
GeneticAlgorithm::GetCurrentFitnesses(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    if(this->_lastFtns.get() == 0x0)
    {
        DesignGroupVector dgv(2, &this->_pop);
        dgv[1] = &this->_cldrn;
        this->SetCurrentFitnesses(
            this->GetFitnessAssessor().AssessFitness(dgv)
            );
    }
    return *this->_lastFtns;
}







/*
================================================================================
Public Methods
================================================================================
*/

bool
GeneticAlgorithm::ExtractParameters(
    const ParameterDatabase& pdb
    )
{
    EDDY_FUNC_DEBUGSCOPE
    bool ret = this->PollForParameters(pdb);
    this->ExtractOperatorParameters(this->GetConverger(), pdb);
    this->ExtractOperatorParameters(this->GetCrosser(), pdb);
    this->ExtractOperatorParameters(this->GetEvaluator(), pdb);
    this->ExtractOperatorParameters(this->GetFitnessAssessor(), pdb);
    this->ExtractOperatorParameters(this->GetInitializer(), pdb);
    this->ExtractOperatorParameters(this->GetNichePressureApplicator(), pdb);
    this->ExtractOperatorParameters(this->GetMainLoop(), pdb);
    this->ExtractOperatorParameters(this->GetMutator(), pdb);
    this->ExtractOperatorParameters(this->GetSelector(), pdb);
    this->ExtractOperatorParameters(this->GetPostProcessor(), pdb);
    return ret;
}


eddy::utilities::uint64_t
GeneticAlgorithm::GetNumberEvaluations(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetEvaluator().GetNumberEvaluations();
}

eddy::utilities::uint64_t
GeneticAlgorithm::GetGenerationNumber(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetMainLoop().GetCurrentGeneration();
}

bool
GeneticAlgorithm::SetOperatorGroup(
    const GeneticAlgorithmOperatorGroup& to
    )
{
    EDDY_FUNC_DEBUGSCOPE

    GeneticAlgorithmOperatorGroupRegistry& ogReg =
        this->GetOperatorGroupRegistry();

    bool ret = ogReg.is_registered(to.GetName());
    if(ret)
    {
        if(!to.ContainsSet(this->GetOperatorSet()))
        {
            JEGALOG_II(this->GetLogger(), ldebug(), this,
                text_entry(ldebug(), this->GetName() + ": Current operator set "
                    "is not compatible with new operator group \"" +
                    to.GetName() + "\".  Clearing the set and "
                    "adopting the group.")
                )
            this->GetOperatorSet().Clear();
        }
        this->_opGroup = &to;
    }
    return ret;
}

#define SET_OP_METHOD(optype) \
    bool \
    GeneticAlgorithm::Set##optype( \
        GeneticAlgorithm##optype* to \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return this->SetOperator<GeneticAlgorithm##optype>( \
                to, \
                &GeneticAlgorithmOperatorSet::Get##optype, \
                &GeneticAlgorithmOperatorSet::Set##optype, \
                this->GetOperatorGroup().Has##optype(*to), \
                #optype \
                ); \
    }

SET_OP_METHOD(Converger)
SET_OP_METHOD(Crosser)
SET_OP_METHOD(Evaluator)
SET_OP_METHOD(FitnessAssessor)
SET_OP_METHOD(Initializer)
SET_OP_METHOD(MainLoop)
SET_OP_METHOD(Mutator)
SET_OP_METHOD(Selector)
SET_OP_METHOD(PostProcessor)
SET_OP_METHOD(NichePressureApplicator)

bool
GeneticAlgorithm::SetOperatorSet(
    const GeneticAlgorithmOperatorSet& to
    )
{
    EDDY_FUNC_DEBUGSCOPE

    const GeneticAlgorithmOperatorGroup* gp = this->MatchGroup(to);

    // first, warn if we had no success matching a group.
    if(gp == 0x0)
    {
        JEGALOG_II(this->GetLogger(), lquiet(), this, text_entry(lquiet(),
            this->GetName() + ": Attempt to use an operator set which cannot "
            "be matched to a group failed.  Retaining current operator set."))
        return false;
    }

    // start by accepting our new set b/c we have success.
    this->_opSet->operator =(to);

    // we matched a group.  now we must see if it is our current group.
    // if it is, fine.  If not, adopt the new group and warn about it.
    if(gp != this->_opGroup)
    {
        JEGALOG_II(this->GetLogger(), lquiet(), this, text_entry(lquiet(),
            this->GetName() + ": Matched new set to a group that is not the "
            "current group.  Adopting the new group."))
        this->SetOperatorGroup(*gp);
    }

    // return our success
    return true;
}

Design*
GeneticAlgorithm::GetNewDesign(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    Design* ret = this->_target.GetNewDesign();
    EDDY_ASSERT(ret != 0x0);
    return ret;
}

Design*
GeneticAlgorithm::GetNewDesign(
    const Design& copy
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    Design* ret = this->_target.GetNewDesign(copy);
    EDDY_ASSERT(ret != 0x0);
    return ret;
}

size_t
GeneticAlgorithm::ValidateVariableValues(
    const DesignGroupVector& groups
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    size_t numIll = 0;

    // iterate over the "groups" and call the overload
    for(size_t i=0; i<groups.size(); ++i)
        numIll += this->ValidateVariableValues(*groups[i]);

    return numIll;
}

size_t
GeneticAlgorithm::LogIllconditionedDesigns(
    const JEGA::Utilities::DesignGroup& from
    ) const
{
    size_t nLogged = 0;

#if defined(JEGA_LOGGING_ON)
    if(this->GetLogger().Gate().will_log(
        this->GetLogger().Gate().get_default_level(), lquiet()
        ))
    {
        ostream_entry ent(lquiet(), this->GetName());

        ent << ": Design Variable Values for Illconditioned "
               "Designs:\n";
        
        for(
            DesignDVSortSet::const_iterator dvit(from.BeginDV());
            dvit!=from.EndDV(); ++dvit
            )
        {
            const Design* des = *dvit;
            if(des->IsIllconditioned())
            {
                ent << des->GetVariableValue(0);
                for(size_t i=1; i<des->GetNDV(); ++i)
                    ent << ' ' << des->GetVariableValue(i);
                ent << '\n';
                ++nLogged;
            }
        }
        this->GetLogger().Gate().simple_log(lquiet(), ent);
    }
#endif

    return nLogged;
}

size_t
GeneticAlgorithm::ValidateVariableValues(
    DesignGroup& group
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // get the design variable info objects.
    const DesignVariableInfoVector& dvInfos =
        this->_target.GetDesignVariableInfos();

    size_t ndv = dvInfos.size();

    // keep a temporary list of the altered designs.
    vector<Design*> changedDesigns;
    changedDesigns.reserve(group.SizeDV());

    size_t numIll = 0;

    // go through the Designs and then the infos
    const DesignDVSortSet::const_iterator de(group.EndDV());

    for(DesignDVSortSet::iterator dit(group.BeginDV()); dit!=de;)
    {
        // keep track of whether or not the design gets changed.
        bool changed = false;

        Design* des = *dit;

        for(size_t dv = 0; dv<ndv; ++dv)
        {
            double rep = des->GetVariableRep(dv);

            const DesignVariableInfo& dvi = *dvInfos[dv];

            if(dvi.IsValidDoubleRep(rep)) continue;

            // get a corrected variable value.
            double corrected = dvi.IsRepInBounds(rep) ?
                dvi.GetNearestValidDoubleRep(rep) : dvi.GetRandomDoubleRep();

            // a return of -DBL_MAX means that the rep could
            // not be corrected.
            if(corrected == -DBL_MAX)
            {
                JEGALOG_II(this->GetLogger(), lquiet(), this,
                    ostream_entry(lquiet(), this->GetName() + ": "
                        "Noncorrectable invalid value of ")
                        << rep << " found for variable " << dvi.GetLabel()
                    )
                des->SetIllconditioned(true);
                ++numIll;
                break;
            }
            // otherwise, install the corrected value and
            // set the flag so that we know to remove and
            // re-insert the Design.
            else if(corrected != rep)
            {
                JEGALOG_II(this->GetLogger(), lquiet(), this,
                    ostream_entry(
                        lquiet(), this->GetName() + ": Invalid value of "
                        )
                        << rep << " found for variable " << dvi.GetLabel()
                        << ".  Corrected to " << corrected << "."
                    )
                des->SetVariableRep(dv, corrected);
                des->RemoveAsClone();
                changed = true;
            }
        }

        // if the design was changed, we have to remove it temporarily.
        if(changed)
        {
            changedDesigns.push_back(des);
            dit = group.EraseRetDV(dit);
        }
        else ++dit;
    }

    // now put back all the changed Designs.
    for(size_t i=0; i<changedDesigns.size(); ++i)
    {
        changedDesigns[i]->SetEvaluated(false);
        changedDesigns[i]->SetIllconditioned(false);
        group.Insert(changedDesigns[i]);
    }

    // this will varify that all variables are now valid.
#if defined(JEGA_LOGGING_ON) && defined(JEGA_OPTION_DEBUG)

    for(DesignDVSortSet::iterator dit(group.BeginDV()); dit!=de; ++dit)
    {
        for(DesignVariableInfoVector::const_iterator dvit(dvInfos.begin());
            dvit != dvInfos.end(); ++dvit)
        {
            JEGAIFLOG_CF_F(!(*dit)->IsIllconditioned() &&
                        !(*dvit)->IsValidDoubleRep(
                            (*dvit)->WhichDoubleRep(**dit)
                            ),
                        this->GetLogger(),
                        ostream_entry(lfatal(),
                            "Invalid variable representation found in "
                            "non-illcondintioned design after variable "
                            "correction operation.  Variable ")
                                << (*dvit)->GetLabel() << ", Representation "
                                << (*dvit)->WhichDoubleRep(**dit)
                        )
        }
    }
#endif

    return numIll;
}

double
GeneticAlgorithm::GetElapsedTime(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return double(clock() - this->_startTime) / CLOCKS_PER_SEC;
}

/*
================================================================================
Subclass Visible Methods
================================================================================
*/

string
GeneticAlgorithm::GetDefaultName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    ostringstream ostr;
    ostr << this->GetAlgorithmTypeName() << " #" << this->GetInstanceNumber();
    return ostr.str();
}

void
GeneticAlgorithm::ExtractOperatorParameters(
    GeneticAlgorithmOperator& op,
    const ParameterDatabase& pdb
    )
{
    EDDY_FUNC_DEBUGSCOPE
    JEGAIFLOG_CF_II_F(!op.ExtractParameters(pdb), this->GetLogger(), this,
        text_entry(lfatal(),
            this->GetName() + ": Failed to retrieve the parameters for \""
            ) << op.GetName() << "\"."
        );
}

const GeneticAlgorithmOperatorGroupRegistry&
GeneticAlgorithm::GetOperatorGroupRegistry(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return const_cast<GeneticAlgorithm*>(this)->GetOperatorGroupRegistry_FWD();
}

bool
GeneticAlgorithm::WritePopulationToFile(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    ostringstream fname;
    fname << "population_" << this->GetGenerationNumber() << ".dat";
    return this->WritePopulationToFile(
        this->GetDataDirectory() + "/" + fname.str()
        );
}

bool
GeneticAlgorithm::WritePopulationToFile(
    const std::string& fname
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->WriteGroupToFile(this->_pop.GetOFSortContainer(), fname);
}

bool
GeneticAlgorithm::WriteGroupToFile(
    const JEGA::Utilities::DesignDVSortSet& group,
    const std::string& fname
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    ofstream ofile(fname.c_str());

    if(!ofile.is_open())
    {
        JEGALOG_II(this->GetLogger(), lquiet(), this, text_entry(lquiet(),
            this->GetName() + ": Unable to open file " + fname +
            " for writing design group.  No data written.")
             )
        return false;
    }

    group.stream_out(ofile);
    ofile.close();

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(lverbose(),
            this->GetName() + ": Wrote designs file \"" + fname + "\".")
        )

    return true;
}

bool
GeneticAlgorithm::WriteGroupToFile(
    const JEGA::Utilities::DesignOFSortSet& group,
    const std::string& fname
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    ofstream ofile(fname.c_str());

    if(!ofile.is_open())
    {
        JEGALOG_II(this->GetLogger(), lquiet(), this, text_entry(lquiet(),
            this->GetName() + ": Unable to open file " + fname +
            " for writing design group.  No data written.")
             )
        return false;
    }

    group.stream_out(ofile);
    ofile.close();

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(lverbose(),
            this->GetName() + ": Wrote designs file \"" + fname + "\".")
        )

    return true;
}





/*
================================================================================
Subclass Overridable Methods
================================================================================
*/
bool
GeneticAlgorithm::PollForParameters(
    const JEGA::Utilities::ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    string tname;
    bool success = ParameterExtractor::GetStringFromDB(
        db, "method.jega.algorithm_name", tname
        );

    // If we did not find the name, warn about it and use the default
    // value.  Note that if !success, then tname is unaltered
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The algorithm name string "
            "was not found in the parameter database.  A default name will "
            "be created.")
        )

    SetName(tname);

    bool tmp = false;

    // Begin by extracting the population printing flag.
    success = ParameterExtractor::GetBooleanFromDB(
        db, "method.print_each_pop", tmp
        );

    if(success) this->SetPrintEachPopulation(tmp);

    // If we did not find the flag, warn about it and use the default
    // value.  Note that if !success, then _printPopEachGen is unaltered
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The population printing "
            "flag was not found in the parameter database.  Using the current "
            "value of ") << (this->_printPopEachGen ? "true." : "false.")
        )

    success = ParameterExtractor::GetBooleanFromDB(
        db, "method.print_final_data", tmp
        );

    if(success) this->SetPrintFinalData(tmp);

    // If we did not find the flag, warn about it and use the default
    // value.  Note that if !success, then _printFinalData is unaltered
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The final data printing "
            "flag was not found in the parameter database.  Using the current "
            "value of ") << (this->_printFinalData ? "true." : "false.")
        )

    success = ParameterExtractor::GetBooleanFromDB(
        db, "method.print_discards", tmp
        );

    if(success) this->SetPrintDiscards(tmp);

    // If we did not find the flag, warn about it and use the default
    // value.  Note that if !success, then _printDiscards is unaltered
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": The discards printing "
            "flag was not found in the parameter database.  Using the current "
            "value of ") << (this->_printDiscards ? "true." : "false.")
        )

    success = ParameterExtractor::GetStringFromDB(
        db, "method.jega.final_data_filename", tname
        );

    if(success) this->SetFinalDataFilename(tname);

    // If we did not find the name, warn about it and use the default
    // value.  Note that if !success, then tname is unaltered
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The final data "
            "filename/pattern was not found in the parameter database.  Using "
            "the current pattern of " + this->_finalDataFile + ".")
        )

    success = ParameterExtractor::GetStringFromDB(
        db, "method.jega.data_directory", tname
        );

    if(success) this->SetDataDirectory(tname);

    // If we did not find the data directory, warn about it and use the default
    // value.  Note that if !success, then tname is unaltered
    JEGAIFLOG_CF_II(!success, this->GetLogger(), lverbose(), this,
        text_entry(lverbose(), this->GetName() + ": The data directory "
            "was not found in the parameter database.  Using the current "
            "value of " + this->_dataDir + ".")
        )

    return success;
}


void
GeneticAlgorithm::DoCrossover(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // it should be unnecessary to do this but we will anyway.
    this->_cldrn.FlushAll();

    // all is set to use the crossover operator.
    // cross members of _pop and put the results in _cldrn.
    this->GetCrosser().Crossover(this->_pop, this->_cldrn);
}

const FitnessRecord*
GeneticAlgorithm::DoFitnessAssessment(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // create a vector of groups to be simultaneously considered
    // for fitness assessment.
    DesignGroupVector gpvec(2, &this->_pop);
    gpvec[1] = &this->_cldrn;

    // now use the fitness assessor operator on the collected groups.
    return this->GetFitnessAssessor().AssessFitness(gpvec);
}

void
GeneticAlgorithm::DoMutation(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Everything should be all set to go right into the mutation operator.
    this->GetMutator().Mutate(this->_pop, this->_cldrn);
}

void
GeneticAlgorithm::DoPreSelection(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->GetNichePressureApplicator().PreSelection(this->_pop);
}

void
GeneticAlgorithm::AbsorbEvaluatorInjections(
    bool allowDuplicates
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Merge in any designs injected via the evaluator.  They get merged
    // into the children and must already be evaluated.
    GeneticAlgorithmEvaluator& evaler = this->GetEvaluator();
    if(evaler.GetInjections().empty()) return;

    evaler.MergeInjectedDesigns(this->_cldrn, false, false, allowDuplicates);

    // Now verify that there are no children that duplicate population members
    // if disallowing duplicates.
    if(!allowDuplicates)
    {
        std::size_t nrem = 
            this->GetPopulation().GetDVSortContainer().test_for_clones(
                this->_cldrn.GetDVSortContainer()
                );

        if(nrem > 0)
        {
            nrem = this->_cldrn.FlushCloneDesigns();

            JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(),
                lverbose(), this,
                ostream_entry(lverbose(), this->GetName() + ": flushed ")
                    << nrem << " designs from the children set (possibly "
                    "injections) that duplicated existing population members."
                )
        }
    }

    // now that we've absorbed them all, dispose of them in the evalutor.
    evaler.ClearInjectedDesigns();
}

void
GeneticAlgorithm::DoSelection(
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // create a vector of all groups to be selected from.
    DesignGroupVector gpvec(2, &this->_pop);
    gpvec[1] = &this->_cldrn;

    JEGA_LOGGING_IF_ON(const std::size_t iPopSize = this->_pop.GetSize());
    JEGA_LOGGING_IF_ON(const std::size_t iCldrnSize = this->_cldrn.GetSize());

    // prepare a group to put the results into.
    DesignGroup into(this->_target);

    // actually perform the selection
    this->GetSelector().Select(gpvec, into, this->_pop.GetSize(), fitnesses);

    // expel whatever is left in the Population and Children after indicating
    // what is gone and what remains.
    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": ")
            << this->_pop.GetSize() << " of " << iPopSize
            << " population members were not selected to continue.  "
            << this->_cldrn.GetSize() << " of " << iCldrnSize
            << " offspring were immediately rejected."
        )

    this->_pop.FlushAll();
    this->_cldrn.FlushAll();

    // set the new population to be the new group
    this->_pop = into;
}

void
GeneticAlgorithm::ApplyNichePressure(
    JEGA::Utilities::DesignGroup& group,
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->GetNichePressureApplicator().ApplyNichePressure(group, fitnesses);
}

void
GeneticAlgorithm::InitializePopulation(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Everything should be all set to go right into the initialization
    // operator.
    this->GetInitializer().Initialize(this->_pop);
}

bool
GeneticAlgorithm::DoEvaluation(
    DesignGroup& group
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Everything should be all set to go right into the evaluator operator.
    bool success = this->GetEvaluator().Evaluate(group);

    // now refresh the DesignOFSortSet of group so that it reflects
    // the fact that it's Designs should now be evaluated.
    group.SynchronizeOFAndDVContainers();
    return success;
}

bool
GeneticAlgorithm::TestForConvergence(
    const FitnessRecord& fitnesses
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // all should be set to use the convergence operator.
    // recall that the CheckConvergence operator returns
    // whether or not convergence has occurred.
    return this->GetConverger().CheckConvergence(this->_pop, fitnesses);
}

void
GeneticAlgorithm::DoPostProcessing(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    this->GetPostProcessor().PostProcess(this->_pop);
}

bool
GeneticAlgorithm::AlgorithmInitialize(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_startTime = clock();

    // initialize the populations design variable values.
    this->InitializePopulation();

    if(this->GetInitializer().CanProduceInvalidVariableValues())
    {
        // verify the validity of each design variable of eahc Design.
        size_t numIll = this->ValidateVariableValues(this->_pop);

        // any designs that could not be evaluated must be removed.
        if(numIll > 0)
        {
            this->LogIllconditionedDesigns(this->_pop);

            JEGA_LOGGING_IF_ON(DesignDVSortSet::size_type nrem =)
                this->_pop.FlushIllconditionedDesigns();

            JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(), lquiet(), this,
                ostream_entry(lquiet(), this->GetName() + ": flushed ") << nrem
                    << " designs from the initial population whose variables "
                       "could not be corrected."
                    )
        }
    }

    // check for clones in the initial population to avoid re-evaluation
    this->_pop.GetDVSortContainer().test_within_list_for_clones();

    // check for duplicates in the discarded designs to avoid re-evaluations.
    const DesignDVSortSet& discards =
        this->GetDesignTarget().CheckoutDiscards();
    discards.test_for_clones(this->_pop.GetDVSortContainer());
    this->GetDesignTarget().CheckinDiscards();

    // perform evaluation.
    bool evald = this->DoEvaluation(this->_pop);

    JEGAIFLOG_CF_II(!evald, this->GetLogger(), lquiet(), this,
        text_entry(
            lquiet(), this->GetName() + ": Errors were encountered while "
            "evaluating the offspring designs"
            )
        )

    // any designs that could not be evaluated must be removed.
    if(!evald)
    {
        this->LogIllconditionedDesigns(this->_pop);

        JEGA_LOGGING_IF_ON(DesignDVSortSet::size_type nrem =)
            this->_pop.FlushIllconditionedDesigns();

        JEGAIFLOG_CF_II(nrem > 0, this->GetLogger(), lquiet(), this,
            ostream_entry(
                lquiet(), this->GetName() + ": encountered and flushed "
                )
                << nrem
                << " illconditioned designs after evaluation of the initial "
                   "population."
                )
    }

    // load the designs into the objective function sorted list.
    this->_pop.SynchronizeOFAndDVContainers();

    // initialize the generation number to 0.
    this->GetMainLoop().SetCurrentGeneration(0);

    if(this->_printPopEachGen) this->WritePopulationToFile();

    this->_isInitialized = true;

    return true;
}

bool
GeneticAlgorithm::AlgorithmProcess(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // If this algorithm has already reported itself converged, don't do
    // anything.
    // Or, similary, if we test it for convergence using the parameterless
    // convergence method and it says it is converged, then do nothing.
    if(this->GetConverger().GetConverged() ||
       this->GetConverger().CheckConvergence())
        return false;

    const bool ret = this->GetMainLoop().RunGeneration();

    if(this->_printPopEachGen) this->WritePopulationToFile();

    JEGA_LOGGING_IF_ON(
        this->GetLogger().FlushStreams();
        if(&this->GetLogger() != &Logger::Global())
            Logger::Global().FlushStreams();
        )

    return ret;
}

bool
GeneticAlgorithm::AlgorithmFinalize(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGA_LOGGING_IF_ON(std::size_t prevSize = this->_pop.GetSize();)

    // make sure that the children structure is empty.
    this->_cldrn.FlushAll();
    // prepare a return value.
    bool ret = true;

    // Finalize all the operators with the exception of the post processor
    // and the fitness assessor both of which will be needed below.
    ret &= this->GetConverger().Finalize();
    ret &= this->GetCrosser().Finalize();
    ret &= this->GetEvaluator().Finalize();
    ret &= this->GetInitializer().Finalize();
    ret &= this->GetMainLoop().Finalize();
    ret &= this->GetMutator().Finalize();
    ret &= this->GetNichePressureApplicator().Finalize();
    ret &= this->GetSelector().Finalize();

    // reclaim any optimal from the discards.
    this->ReclaimOptimal();

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lquiet(), this->GetName() + ": Reclaimed ")
            << (this->_pop.GetSize() - prevSize) << " optimal designs that "
               "had been selected out."
        )

    // now flush out all non-optimal designs.
    this->FlushNonOptimal();

    // Now run the post-processor.
    this->DoPostProcessing();

    // now we can finalize our fitness assessor and post processor.
    ret &= this->GetFitnessAssessor().Finalize();
    ret &= this->GetPostProcessor().Finalize();

    JEGALOG_II(this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": Ran ")
            << this->GetGenerationNumber() << " total generations."
        )

    // let the user know how many final Designs are left
    JEGALOG_II(this->GetLogger(), lquiet(), this,
        ostream_entry(lquiet(), this->GetName() + ": Final population size = ")
            << this->_pop.GetSize()
        )

    // write out the final set of optimal designs to a tab delimited file.
    if(this->_printFinalData &&
       !this->WritePopulationToFile(
            this->GetDataDirectory() + "/" + this->_finalDataFile
        )) ret = false;

    // now write all other Designs considered to another tab delimited file.
    if(this->_printDiscards)
    {
        const DesignDVSortSet& discards =
            this->GetDesignTarget().CheckoutDiscards();
        if(!this->WriteGroupToFile(
            discards, this->GetDataDirectory() + "/" + "discards.dat"
            )) ret = false;
        this->GetDesignTarget().CheckinDiscards();
    }

    this->_isFinalized = true;

    return ret;
}

const Design*
GeneticAlgorithm::GetBestDesign(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // store the ofsort of the population for repeated use.
    DesignGroupVector gps(1, &this->GetPopulation());

    DesignOFSortSet bests(this->GetSelector().SelectNBest(
        gps, 1, this->GetCurrentFitnesses()
        ));

    return bests.empty() ? 0x0 : *bests.begin();
}

/*
================================================================================
Private Methods
================================================================================
*/
const GeneticAlgorithmOperatorGroup*
GeneticAlgorithm::MatchGroup(
    const GeneticAlgorithmOperatorSet& set
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    // go through all the possible groups until one is found that matches.
    // when one is found, make it the new group and return true.
    // if none are found, return false.
    const GeneticAlgorithmOperatorGroupRegistry& gReg =
        GetOperatorGroupRegistry();

    GeneticAlgorithmOperatorGroupRegistry::const_iterator it(gReg.begin());

    for(; it!=gReg.end(); ++it)
    {
        const GeneticAlgorithmOperatorGroup& cgp = (*it).second();

        JEGALOG_II(this->GetLogger(), lverbose(), this,
            text_entry(
                ldebug(), this->GetName() + " Matching Group: Trying \"" +
                cgp.GetName() + "\"."
                )
            )

        if(cgp.ContainsSet(set))
        {
            JEGALOG_II(this->GetLogger(), lverbose(), this,
                text_entry(ldebug(),
                    this->GetName() + " Matching Group: \"" + cgp.GetName() +
                    "\" match succeeded.")
                )
            return &cgp;
        }
    }

    return 0x0;
}

const GeneticAlgorithmOperatorGroupRegistry&
GeneticAlgorithm::GetOperatorGroupRegistry_FWD(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->GetOperatorGroupRegistry();
}






/*
================================================================================
Structors
================================================================================
*/
GeneticAlgorithm::GeneticAlgorithm(
    DesignTarget& target,
    Logger& logger
    ) :
        _opGroup(0x0),
        _opSet(0x0),
        _pop(target),
        _cldrn(target),
        _target(target),
        _log(logger),
        _name(),
        _finalDataFile("finaldata#.dat"),
        _instanceNum(++_instanceCt),
        _printPopEachGen(false),
        _printFinalData(true),
        _printDiscards(true),
        _myDesignSpace(target.GetDesignSpace()),
        _isFinalized(false),
        _isInitialized(false),
        _lastFtns(0x0),
        _dataDir("./"),
        _startTime(std::numeric_limits<clock_t>::max())
{
    EDDY_FUNC_DEBUGSCOPE
    this->_opSet = new GeneticAlgorithmOperatorSet(*this);
}

GeneticAlgorithm::~GeneticAlgorithm(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    this->_pop.FlushAll();
    this->_cldrn.FlushAll();
    this->_opSet->DestroyOperators();
    delete this->_opSet;

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        text_entry(lquiet(), this->GetName() + ": goodbye!\n\n")
        )

    this->GetLogger().FlushStreams();
}





/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace Algorithms
} // namespace JEGA
