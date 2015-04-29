/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA)

    CONTENTS:

        Implementation of class LocalDesignVariableMutator.

    NOTES:

        See notes of LocalDesignVariableMutator.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Nov 10 12:07:44 2009 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the LocalDesignVariableMutator class.
 */


/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <fstream>
#include <../Utilities/include/Logging.hpp>
#include <../Utilities/include/DesignGroup.hpp>
#include <../Utilities/include/DesignTarget.hpp>
#include <utilities/include/EDDY_DebugScope.hpp>
#include <Mutators/LocalDesignVariableMutator.hpp>
#include <../Utilities/include/DesignVariableInfo.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <utilities/include/RandomNumberGenerator.hpp>

#ifdef JEGA_HAVE_BOOST
#include <boost/lexical_cast.hpp>
#else
#include <sstream>
#endif

/*
================================================================================
Namespace Using Directives
================================================================================
*/
using namespace std;
using namespace JEGA::Logging;
using namespace JEGA::Utilities;
using namespace eddy::utilities;





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
template <typename T>
T
AsT(
    const string& conv
    )
{
    EDDY_FUNC_DEBUGSCOPE

#ifdef JEGA_HAVE_BOOST
	return boost::lexical_cast<T>(conv);
#else
    istringstream istr(conv);
    T res;
    istr >> res;
    return res;
#endif
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
const string&
LocalDesignVariableMutator::Name(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    static const string ret("local_uniform");
    return ret;
}

const string&
LocalDesignVariableMutator::Description(
    )
{
    EDDY_FUNC_DEBUGSCOPE

    static const string ret(
        "This mutator does mutation by first randomly selecting a Design.  "
        "It then chooses a random design variable and reassigns that variable "
        "to a random valid value.  It then looks to the left and right of that "
        "variable and performs the exact change to any values that have the "
        "same old value.\n\nThe net result would be that a string such as "
        "1 2 2 2 2 3 3 4 5 6 could become something like 1 7 7 7 7 3 3 4 5 6"
        "\n\nThe number of mutations is the rate times the size of the group "
        "passed in rounded to the nearest whole number.\n\nThis mutator is "
        "built specifically for technology management optimization and is "
        "aware of the concepts of variables, options, and suboptions.  It "
        "requires that the design space map be written without specific names "
        "into a text file that can in turn be read.  The location of that "
        "file must be provided via the parameters database as a string "
        "argument named method.design_space_map."
        );
    return ret;
}

GeneticAlgorithmOperator*
LocalDesignVariableMutator::Create(
    GeneticAlgorithm& algorithm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return new LocalDesignVariableMutator(algorithm);
}









/*
================================================================================
Subclass Visible Methods
================================================================================
*/
void
LocalDesignVariableMutator::ReadDesignSpaceFile(
    const string& fileName
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // We need to read the design space map file.  It is somewhat complicated in
    // that it is built in 4 sections.  The first is just statistics on the
    // total space.  The second is the details of the single choice option
    // space.  The third is the details of the multiple choice option space and
    // the last is the variable space details.
    //
    // We need to identify and treat each section differently.  So, we will
    // read lines and identify the sections as we go.  Each time we come to a
    // new section, we will branch off and process it.
    ifstream iFile(fileName.c_str());

    // If we could not open the space map file, then we cannot continue.
    JEGAIFLOG_CF_II_F(!iFile, this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": The design space file \"" +
            fileName + "\" was not found or could not be opened.")
        )

    this->ReadDesignSpaceFile(iFile, fileName);
}

void
LocalDesignVariableMutator::ReadDesignSpaceFile(
    istream& iFile,
    const string& fileName
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // We can skip lines until we find a section.
    string line;
    line.reserve(1024);

    while(!iFile.eof())
    {
        getline(iFile, line);
        if(line == "Single Choice Option Space")
        {
            this->_roadmaps.push_back(roadmap());

            this->ReadSingleChoiceOptionSection(
                iFile, this->_roadmaps.back(), fileName
                );
        }
        else if(line == "Multiple Choice Option Space")
        {
            this->ReadMultipleChoiceOptionSection(
                iFile, this->_roadmaps.back(), fileName
                );
        }
        else if(line == "Variable Space")
        {
            this->ReadVariableSection(
                iFile, this->_roadmaps.back(), fileName
                );
        }
    }
}

void
LocalDesignVariableMutator::ReadSingleChoiceOptionSection(
    istream& iFile,
    roadmap& rm,
    const string& fileName
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // We can skip lines until we find a section.
    string line;
    line.reserve(1024);

    while(!iFile.eof())
    {
        getline(iFile, line);

        if(!line.empty())
        {
            if(line[0] == 'O')
                this->ReadSingleChoiceOptionData(iFile, line, rm, fileName);

            else if(line[0] == '=') return;
        }
    }
}

void
LocalDesignVariableMutator::ReadMultipleChoiceOptionSection(
    istream& iFile,
    roadmap& rm,
    const string& fileName
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // We can skip lines until we find a section.
    string line;
    line.reserve(1024);

    while(!iFile.eof())
    {
        if(line.empty() || line[0] != 'O')
            getline(iFile, line);

        if(!line.empty() && line[0] == 'O')
            this->ReadMultipleChoiceOptionData(iFile, line, rm, fileName);

        if(!line.empty() && line[0] == '=')
            return;
    }
}

void
LocalDesignVariableMutator::ReadVariableSection(
    istream& iFile,
    roadmap& rm,
    const string& fileName
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // We can skip lines until we find a section.
    string line;
    line.reserve(1024);

    while(!iFile.eof())
    {
        if(line.empty() || line[0] != 'V')
            getline(iFile, line);

        if(!line.empty() && line[0] == 'V')
            this->ReadVariableData(iFile, line, rm, fileName);

        if(!line.empty() && line[0] == '=')
            return;
    }
}

string
LocalDesignVariableMutator::TrimWhitespace(
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

    if(ret.empty()) return ret;

    size_t first = ret.size() - 1;
    while(isspace(ret[first])) --first;
    ++first;
    if(first < ret.size()) ret.erase(first);

    return ret;
}

void
LocalDesignVariableMutator::PerformMoveBy1Mutation(
    JEGA::Utilities::Design& des,
    const std::size_t dv
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // store the design target for repeated use
    const DesignTarget& target = des.GetDesignTarget();

    // extract the design variable information
    const DesignVariableInfoVector& dvis = target.GetDesignVariableInfos();
    const DesignVariableInfo& dvi = *dvis[dv];
    const double currRep = des.GetVariableRep(dv);

    const bool up =
		(RandomNumberGenerator::RandomBoolean() &&
         currRep < dvi.GetMaxDoubleRep()) ||
         currRep <= dvi.GetMinDoubleRep();

    des.SetVariableRep(
        dv, dvi.GetNearestValidDoubleRep(currRep + (up ? 1 : -1))
        );
}

void
LocalDesignVariableMutator::PerformRandomReassignMutation(
    JEGA::Utilities::Design& des,
    const std::size_t dv
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // store the design target for repeated use
    const DesignTarget& target = des.GetDesignTarget();

    // extract the design variable information
    const DesignVariableInfoVector& dvis = target.GetDesignVariableInfos();

    des.SetVariableRep(dv, dvis[dv]->GetRandomDoubleRep());
}

void
LocalDesignVariableMutator::PerformFullBlockChangeMutation(
    Design& des,
    const roadmap& rm,
    const size_t dv,
    const size_t pndv
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // store the design target for repeated use
    const DesignTarget& target = des.GetDesignTarget();

    // extract the design variable information
    const DesignVariableInfoVector& dvis = target.GetDesignVariableInfos();

    // if the chosen dv is higher than the hi Opt, then it is a variable.
    // If that is the case, then we use the variable map and not the
    // option suboption maps.
    const bool dvIsVar = static_cast<int>(dv-pndv) > rm._hiOptVar;

    // perform the mutation.  In order to identify locally equal variables,
    // store the old rep.  To replace properly, store the new rep.
    const double oldRep = des.GetVariableRep(dv);
    const double newRep = this->GenerateNewRep(des, rm, dv, pndv);

    // If we could not find a new value different from the old, don't bother
    // with the rest of this loop.
    if(newRep != oldRep)
        this->PerformFullBlockChangeMutation(des, rm, dv, pndv, oldRep, newRep);
}

void
LocalDesignVariableMutator::PerformVerticalPairFullBlockChangeMutation(
    Design& des,
    const roadmap& rm,
    const size_t pndv
    )
{
    EDDY_FUNC_DEBUGSCOPE

    const DesignTarget& target = des.GetDesignTarget();
    const DesignVariableInfoVector& dvis = target.GetDesignVariableInfos();

    // We will choose two variables, each from a different option or variable
    // but aligned in time.  Start by choosing the time.
    const size_t time = RandomNumberGenerator::UniformInt<size_t>(
        0, rm._dateDVs.size() - 1
        );

    // Now, extract the vector of allowable variables for the chosen time.
    // Note that each one must come from a different option or variable.
    const map<string, vector<size_t> >::const_iterator mit =
        eddy::utilities::advance(rm._dateDVs.begin(), time);

    const vector<size_t>& vvec = mit->second;

    size_t ov1 = RandomNumberGenerator::UniformInt<size_t>(0, vvec.size() - 1);
    size_t ov2 = RandomNumberGenerator::UniformInt<size_t>(0, vvec.size() - 2);

    if(ov2 >= ov1) ++ov2;

    const size_t dv1 = vvec[ov1] + pndv;
    const size_t dv2 = vvec[ov2] + pndv;

    const double oldRep1 = des.GetVariableRep(dv1);
    const double newRep1 = this->GenerateNewRep(des, rm, dv1, pndv);

    const double oldRep2 = des.GetVariableRep(dv2);
    const double newRep2 = this->GenerateNewRep(des, rm, dv2, pndv);

    this->PerformFullBlockChangeMutation(
        des, rm, dv1, pndv, oldRep1, newRep1
        );
    this->PerformFullBlockChangeMutation(
        des, rm, dv2, pndv, oldRep2, newRep2
        );
}

void
LocalDesignVariableMutator::PerformBlockExtensionMutation(
    Design& des,
    const roadmap& rm,
    const size_t dv,
    const size_t pndv
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // if there is only 1 time period, then there is nothing to do.
    if(rm._dateDVs.size() < 2) return;

    // store the design target for repeated use
    const DesignTarget& target = des.GetDesignTarget();

    // extract the design variable information
    const DesignVariableInfoVector& dvis = target.GetDesignVariableInfos();

    const size_t ldv = dv - pndv;

    // if the chosen dv is higher than the hi Opt, then it is a variable.
    // If that is the case, then we use the variable map and not the
    // option suboption maps.
    const bool dvIsVar = (int)ldv > rm._hiOptVar;

    // Figure out if we want to grow right or left.  This is only the
    // preferred.  If we can't go the preferred direction, we will check
    // the other direction.  If we can't do that either, then we skip.
    const bool left = RandomNumberGenerator::RandomBoolean();

    // perform the mutation.  In order to identify locally equal variables,
    // store the old rep.
    const double oldRep = des.GetVariableRep(dv);

    // If we are dealing with a variable, figure out which one.
    if(dvIsVar)
    {
        const size_t var =
            lower_bound(rm._varEnds.begin(), rm._varEnds.end(), ldv) -
            rm._varEnds.begin();

        const pair<size_t, size_t>& allDVs = rm._varMap[var];

        bool madeChange = false;

        // If left, start looking before dv for a different variable value
        // and if found, try to change it.
        if(left)
        {
            for(int i=static_cast<int>(ldv)-1;
                i>=static_cast<int>(allDVs.first); --i)
            {
                size_t si = static_cast<size_t>(i + pndv);
                if(des.GetVariableRep(si) == oldRep) continue;

                // Found one!  See if we can change it.
                if(!dvis[si]->IsValidDoubleRep(oldRep)) break;
                des.SetVariableRep(si, oldRep);
                madeChange = true;
                break;
            }

            // If we didn't make a change, try the right.
            if(!madeChange)
            {
                for(size_t i=ldv+1; i<=allDVs.second; ++i)
                {
                    size_t si = static_cast<size_t>(i+pndv);
                    if(des.GetVariableRep(si) == oldRep) continue;

                    // Found one!  See if we can change it.
                    if(!dvis[si]->IsValidDoubleRep(oldRep)) break;
                    des.SetVariableRep(si, oldRep);
                    madeChange = true;
                    break;
                }
            }
        }
        else
        {
            for(size_t i=ldv+1; i<=allDVs.second; ++i)
            {
                const size_t si = static_cast<size_t>(i+pndv);
                if(des.GetVariableRep(si) == oldRep) continue;

                // Found one!  See if we can change it.
                if(!dvis[si]->IsValidDoubleRep(oldRep)) break;
                des.SetVariableRep(si, oldRep);
                madeChange = true;
                break;
            }

            // If we didn't make a change, try the left.
            if(!madeChange)
            {
                for(int i=static_cast<int>(ldv)-1;
                    i>=static_cast<int>(allDVs.first); --i)
                {
                    const size_t si = static_cast<size_t>(i+pndv);
                    if(des.GetVariableRep(si) == oldRep) continue;

                    // Found one!  See if we can change it.
                    if(!dvis[si]->IsValidDoubleRep(oldRep)) break;
                    des.SetVariableRep(si, oldRep);
                    madeChange = true;
                    break;
                }
            }
        }
    }

    // If we are dealing with an option, figure out which one.
    else
    {
        const size_t opt =
            lower_bound(rm._optEnds.begin(), rm._optEnds.end(), ldv) -
            rm._optEnds.begin();

        const pair<size_t, size_t>& allDVs = rm._optMap[opt];

        bool madeChange = false;

        const string& oldSO =
            rm._dvSuboptList[ldv - rm._numSCOpts][(int)oldRep];

        // If left, start looking before dv for a different suboption
        // and if found, try to change it.
        if(left)
        {
            for(int i=static_cast<int>(ldv)-1;
                i>=static_cast<int>(allDVs.first); --i)
            {
                const size_t si = static_cast<size_t>(i+pndv);

                const string& currSO =
                    rm._dvSuboptList[i - rm._numSCOpts][
                        (int)des.GetVariableRep(si)
                        ];

                if(currSO == oldSO) continue;

                const map<string, size_t>& varIndexMap =
                    rm._dvSuboptIndexMap[i - rm._numSCOpts];

                map<string, size_t>::const_iterator it(
                    varIndexMap.find(oldSO)
                    );
                if(it == varIndexMap.end()) break;

                const double newRep = static_cast<double>(it->second);

                if(!dvis[si]->IsValidDoubleRep(newRep)) break;
                des.SetVariableRep(si, newRep);
                madeChange = true;
                break;
            }

            if(!madeChange)
            {
                for(size_t i=ldv+1; i<=allDVs.second; ++i)
                {
                    const size_t si = static_cast<size_t>(i+pndv);

                    const string& currSO =
                        rm._dvSuboptList[i - rm._numSCOpts][
                            (int)des.GetVariableRep(si)
                            ];

                    if(currSO == oldSO) continue;

                    const map<string, size_t>& varIndexMap =
                        rm._dvSuboptIndexMap[i - rm._numSCOpts];

                    map<string, size_t>::const_iterator it(
                        varIndexMap.find(oldSO)
                        );
                    if(it == varIndexMap.end()) break;

                    const double newRep = static_cast<double>(it->second);

                    if(!dvis[si]->IsValidDoubleRep(newRep)) break;
                    des.SetVariableRep(si, newRep);
                    madeChange = true;
                    break;
                }
            }
        }
        else
        {
            for(size_t i=ldv+1; i<=allDVs.second; ++i)
            {
                const size_t si = static_cast<size_t>(i+pndv);

                const string& currSO =
                    rm._dvSuboptList[i - rm._numSCOpts][
                        (int)des.GetVariableRep(si)
                        ];

                if(currSO == oldSO) continue;

                const map<string, size_t>& varIndexMap =
                    rm._dvSuboptIndexMap[i - rm._numSCOpts];

                map<string, size_t>::const_iterator it(
                    varIndexMap.find(oldSO)
                    );
                if(it == varIndexMap.end()) break;

                const double newRep = static_cast<double>(it->second);

                if(!dvis[si]->IsValidDoubleRep(newRep)) break;
                des.SetVariableRep(si, newRep);
                madeChange = true;
                break;
            }

            if(!madeChange)
            {
                for(int i=static_cast<int>(ldv)-1;
                    i>=static_cast<int>(allDVs.first); --i)
                {
                    const size_t si = static_cast<size_t>(i+pndv);

                    const string& currSO =
                        rm._dvSuboptList[i - rm._numSCOpts][
                            (int)des.GetVariableRep(si)
                            ];

                    if(currSO == oldSO) continue;

                    const map<string, size_t>& varIndexMap =
                        rm._dvSuboptIndexMap[i - rm._numSCOpts];

                    map<string, size_t>::const_iterator it(
                        varIndexMap.find(oldSO)
                        );
                    if(it == varIndexMap.end()) break;

                    const double newRep = static_cast<double>(it->second);

                    if(!dvis[si]->IsValidDoubleRep(newRep)) break;
                    des.SetVariableRep(si, newRep);
                    madeChange = true;
                    break;
                }
            }
        }
    }
}







/*
================================================================================
Subclass Overridable Methods
================================================================================
*/
string
LocalDesignVariableMutator::GetName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return LocalDesignVariableMutator::Name();
}

string
LocalDesignVariableMutator::GetDescription(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return LocalDesignVariableMutator::Description();
}

GeneticAlgorithmOperator*
LocalDesignVariableMutator::Clone(
    GeneticAlgorithm& algorithm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return new LocalDesignVariableMutator(*this, algorithm);
}

void
LocalDesignVariableMutator::Mutate(
    DesignGroup& pop,
    DesignGroup& cldrn
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGALOG_II(this->GetLogger(), ldebug(), this,
        text_entry(ldebug(), this->GetName() + ": Performing mutation.")
        )

    // determine the number of mutation operations;
    const size_t nMutate =
        static_cast<size_t>(Math::Round(this->GetRate() * pop.GetSize()));

    // if there will be no mutations, don't continue.
    if(nMutate < 1)
    {
        JEGALOG_II(this->GetLogger(), ldebug(), this,
            text_entry(ldebug(), this->GetName() + ": Rate and group size are "
                "such that no mutation will occur.")
            )
        return;
    }

    const DesignTarget& target = this->GetDesignTarget();

    // The DesignGroup sort containers do not support random access.  Therefore,
    // for efficiency, we will pre-select the designs to mutate and iterate
    // through to collect them.  Then we will mutate them.
    vector<DesignDVSortSet::iterator> toMutate(
        ChooseDesignsToMutate(nMutate, pop)
        );

    for(vector<DesignDVSortSet::iterator>::iterator it(toMutate.begin());
        it!=toMutate.end(); ++it)
    {
        Design* chosen = target.GetNewDesign(***it);
        chosen->SetEvaluated(false);
        chosen->SetIllconditioned(false);
        chosen->RemoveAsClone();

        const size_t dv =
            RandomNumberGenerator::UniformInt<size_t>(0, target.GetNDV()-1);

        const pair<const roadmap*, size_t> rmpndv =
			this->GetRoadmapAndPriorNDV(dv);

        // The probabilities for fancy mutation types depend on how many
        // multiple change options, variables, and time periods there are.
        const size_t tcVars = rmpndv.first->_dateDVs.size() < 2 ? 0 :
            rmpndv.first->_optMap.size() + rmpndv.first->_varMap.size();

        const size_t ldv = dv - rmpndv.second;

        // If there are no such variables, then we do single option type
        // mutation all the time.
        //
        // If there are multiple such variables, then:
        // We will do block extension 40% when one is to be mutated.
        // We will do block change 40% when one is to be mutated.
        // We will do vertical block change 20% when one is to be mutated.
        //
        // If there is 1 such variable:
        // We will do block extension 50% when it is to be mutated.
        // We will do block change 50% when it is to be mutated.
        //
        // For single change option types:
        // We will do move 1 mutation 40% of the time.
        // We will do random reassign 60% of the time.
        const double val = RandomNumberGenerator::UniformReal();

        const double blockExtProb = tcVars > 1 ? 0.4 : 0.5;
        const double blockChngProb = tcVars > 1 ? 0.8 : 1.1; // 1.1 to be sure.

        // if dv indicates a single option variable, then do single option
        // mutation types.
        if(tcVars == 0 || ldv < rmpndv.first->_numSCOpts)
        {
            if(val <= 0.4)
                this->PerformMoveBy1Mutation(*chosen, dv);
            else
                this->PerformRandomReassignMutation(*chosen, dv);
        }
        else
        {
            if(val <= blockExtProb)
                this->PerformBlockExtensionMutation(
                    *chosen, *rmpndv.first, dv, rmpndv.second
                    );
            else if(val <= blockChngProb)
                this->PerformFullBlockChangeMutation(
                    *chosen, *rmpndv.first, dv, rmpndv.second
                    );
            else
                this->PerformVerticalPairFullBlockChangeMutation(
                    *chosen, *rmpndv.first, rmpndv.second
                    );
        }

        cldrn.Insert(chosen);
    }

    JEGALOG_II(this->GetLogger(), lverbose(), this,
        ostream_entry(lverbose(), this->GetName() + ": Performed ") << nMutate
            << " total mutations."
        )
}

bool
LocalDesignVariableMutator::CanProduceInvalidVariableValues(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return false;
}


bool
LocalDesignVariableMutator::PollForParameters(
    const ParameterDatabase& db
    )
{
    EDDY_FUNC_DEBUGSCOPE

    string spaceMap;
    const bool success = ParameterExtractor::GetStringFromDB(
        db, "method.design_space_map", spaceMap
        );

    // If we did not find the space map file, then we cannot continue.
    JEGAIFLOG_CF_II_F(!success, this->GetLogger(), this,
        text_entry(lfatal(), this->GetName() + ": The design space file name "
            "was not found in the parameter database.  This is a required "
            "input.")
        )

    this->ReadDesignSpaceFile(spaceMap);

    return this->GeneticAlgorithmMutator::PollForParameters(db);
}






/*
================================================================================
Private Methods
================================================================================
*/
string&
LocalDesignVariableMutator::ReadSingleChoiceOptionData(
    istream& iFile,
    string& line,
    roadmap& rm,
    const string& fileName
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(!line.empty() && line[0] == 'O')

    // The only thing we need to know about this kind of variable is that it
    // it is this kind and that it is not a constant.  We will do fairly simple
    // mutation operations on it.  In order to determine that it is not a
    // constant, we need to read the next line.  If it starts with an 'x' then
    // there are values.  Otherwise if it starts with a 'c', then it is a const.
    getline(iFile, line);
    line = TrimWhitespace(line);
    if(!line.empty() && line[0] == 'x') ++rm._numSCOpts;
    return line;
}

string&
LocalDesignVariableMutator::ReadMultipleChoiceOptionData(
    istream& iFile,
    string& line,
    roadmap& rm,
    const string& fileName
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(!line.empty() && line[0] == 'O')

    // Get the option number.  We assume they are sequential but not including
    // any single change options.
    const size_t optNum = rm._optMap.size();

    getline(iFile, line);
    const bool wasSpace = !line.empty() && isspace(line[0]) != 0;
    line = TrimWhitespace(line);

    bool haveLoDV = false;
    size_t loDV = 0;
    size_t hiDV = 0;

    // prepare a vector of strings for the suboption names and a map of
    // names to indices.
    while(wasSpace && !iFile.eof())
    {
        // Skip constants.  They are of no use to us.  They do count as a time
        // increment though!
        if(line[0] == 'c')
        {
            this->ReadDateDVInfo(line, rm);
            getline(iFile, line);
            line = TrimWhitespace(line);
            continue;
        }
        else if(line[0] != 'x') break;

        // We have a variable line that looks like:
        //      x# - # levels (L) @ m[m]/d[d]/yyyy
        // This counts as an increment.
        this->ReadDateDVInfo(line, rm);

        // Since the line has been trimmed, this gets the integer after "x".
        const size_t mcDV = AsT<size_t>(
            line.substr(1, line.find_first_of(' ') - 1)
            );

        const size_t dvNum = mcDV + rm._numSCOpts;

        if(static_cast<int>(dvNum) > rm._hiOptVar)
			rm._hiOptVar = static_cast<int>(dvNum);

        vector<string> names;
        names.reserve(5);
        map<string, size_t> indices;

        while(!iFile.eof())
        {
            getline(iFile, line);
            if(!isspace(line[0])) break;

            line = TrimWhitespace(line);
            if(line[0] != 'L') break;

            // Since the line has been trimmed, this gets the integer after "L".
            size_t lNum = AsT<size_t>(
                line.substr(1, line.find_first_of(' ') - 1)
                );

            string soName = line.substr(line.find_first_of('S'));

            names.push_back(soName);
            indices.insert(make_pair(soName, lNum));
        }

        loDV = haveLoDV ? loDV : dvNum;
        hiDV = dvNum;
        haveLoDV = true;

        if(rm._dvSuboptList.size() < (mcDV + 1))
            rm._dvSuboptList.resize(mcDV + 1);
        if(rm._dvSuboptIndexMap.size() < (mcDV + 1))
            rm._dvSuboptIndexMap.resize(mcDV + 1);

        rm._dvSuboptList[mcDV] = names;
        rm._dvSuboptIndexMap[mcDV] = indices;
    }

    if(rm._optMap.size() < (optNum + 1))
        rm._optMap.resize(optNum + 1);

    if(rm._optEnds.size() < (optNum + 1))
        rm._optEnds.resize(optNum + 1);

    rm._optMap[optNum] = make_pair(loDV, hiDV);
    rm._optEnds[optNum] = rm._hiOptVar;

    // Before we exit, if the number of increments we encountered is greater
    // than the current number on record, use the current number.
    return line;
}

string&
LocalDesignVariableMutator::ReadVariableData(
    istream& iFile,
    string& line,
    roadmap& rm,
    const string& fileName
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(!line.empty() && line[0] == 'V')

    // Get the variable number.  We assume they are sequential.
    const size_t varNum = rm._varMap.size();

    bool haveLoDV = false;
    size_t loDV = 0;
    size_t hiDV = 0;

    while(!iFile.eof())
    {
        getline(iFile, line);
        if(!isspace(line[0])) break;

        line = TrimWhitespace(line);

        // All lines should be constants or variables (c or x).
        this->ReadDateDVInfo(line, rm);

        // Skip constants.  They are of no use to us.
        if(line[0] == 'c') { continue; }

        JEGAIFLOG_CF_II_F(line[0] != 'x', this->GetLogger(), this,
            text_entry(lfatal(), this->GetName() + ": The design space "
                "file \"" + fileName + "\" is not in the required format at "
                "or near line reading \"" + line + '\"')
            )

        const size_t dvNum =
			AsT<size_t>(line.substr(1, line.find_first_of(' ') - 1));

        loDV = haveLoDV ? loDV : dvNum + rm._hiOptVar + 1;
        hiDV = dvNum + rm._hiOptVar + 1;
        haveLoDV = true;
    }

    if(rm._varEnds.size() < (varNum + 1)) rm._varEnds.resize(varNum + 1);
    rm._varEnds[varNum] = hiDV;

    if(rm._varMap.size() < (varNum + 1)) rm._varMap.resize(varNum + 1);
    rm._varMap[varNum] = make_pair(loDV, hiDV);

    return line;
}

string
LocalDesignVariableMutator::ReadDateDVInfo(
    const string& line,
    roadmap& rm
    )
{
    EDDY_FUNC_DEBUGSCOPE
    EDDY_ASSERT(!line.empty() && (line[0] == 'c' || line[0] == 'x'))

    if(line[0] != 'c' && line[0] != 'x') return string();

    const size_t dvNum =
		AsT<size_t>(line.substr(1, line.find_first_of(' ') - 1));

    string::size_type atpos(line.find_last_of('@'));
    string date = line.substr(atpos + 2);

    // See if there is already a vector for these values.
    map<string, vector<size_t> >::iterator mit = rm._dateDVs.find(date);

    if(mit == rm._dateDVs.end())
        mit = rm._dateDVs.insert(
            map<string, vector<size_t> >::value_type(date, vector<size_t>())
            ).first;

    mit->second.push_back(dvNum + rm._numSCOpts);

    return date;
}

void
LocalDesignVariableMutator::PerformFullBlockChangeMutation(
    Design& des,
    const roadmap& rm,
    const size_t dv,
    const size_t pndv,
    double oldRep,
    double newRep
    )
{
    EDDY_FUNC_DEBUGSCOPE

    // Start by changing the variable at dv.
    des.SetVariableRep(dv, newRep);

    // If there is only 1 time period, then there is nothing more to do.
    if(rm._dateDVs.size() < 2) return;

    const size_t ldv = dv - pndv;
    const DesignTarget& target = this->GetDesignTarget();
    const DesignVariableInfoVector& dvis = target.GetDesignVariableInfos();
    const bool dvIsVar = static_cast<int>(ldv) > rm._hiOptVar;

    // If we are dealing with a variable, figure out which one.
    if(dvIsVar)
    {
        const size_t var =
            lower_bound(rm._varEnds.begin(), rm._varEnds.end(), ldv) -
            rm._varEnds.begin();

        const pair<size_t, size_t>& allDVs = rm._varMap[var];

        // Now start looking before dv and changing any that are the same
        // until one that is different is encountered.
        for(int i=static_cast<int>(ldv)-1;
            i>=static_cast<int>(allDVs.first); --i)
        {
            const size_t si = static_cast<size_t>(i+pndv);
            if(des.GetVariableRep(si) != oldRep) break;
            if(!dvis[si]->IsValidDoubleRep(newRep)) break;
            des.SetVariableRep(si, newRep);
        }

        // Now start looking after dv and changing any that are the same
        // until one that is different is encountered.
        for(size_t i=ldv+1; i<=allDVs.second; ++i)
        {
            const size_t si = static_cast<size_t>(i+pndv);
            if(des.GetVariableRep(si) != oldRep) break;
            if(!dvis[si]->IsValidDoubleRep(newRep)) break;
            des.SetVariableRep(si, newRep);
        }
    }

    // If we are dealing with an option, figure out which one.
    else
    {
        const size_t opt =
            lower_bound(rm._optEnds.begin(), rm._optEnds.end(), ldv) -
            rm._optEnds.begin();

        const pair<size_t, size_t>& allDVs = rm._optMap[opt];

        // We have to figure out which suboption is actually in use so we
        // can change those that are the same.
        const string& oldSO =
            rm._dvSuboptList[ldv - rm._numSCOpts][(int)oldRep];
        const string& newSO =
            rm._dvSuboptList[ldv - rm._numSCOpts][(int)newRep];

        // Now start looking before dv and changing any that are the same
        // until one that is different is encountered.
        for(int i=static_cast<int>(ldv)-1;
            i>=static_cast<int>(allDVs.first); --i)
        {
            const size_t si = static_cast<size_t>(i+pndv);

            const string& currSO =
                rm._dvSuboptList[i - rm._numSCOpts][
                    (int)des.GetVariableRep(si)
                    ];
            if(currSO != oldSO) break;

            const map<string, size_t>& varIndexMap =
                rm._dvSuboptIndexMap[i - rm._numSCOpts];
            map<string, size_t>::const_iterator it(varIndexMap.find(newSO));
            if(it == varIndexMap.end()) break;

            const double newRep = static_cast<double>(it->second);

            if(!dvis[si]->IsValidDoubleRep(newRep)) break;
            des.SetVariableRep(si, newRep);
        }

        // Now start looking after dv and changing any that are the same
        // until one that is different is encountered.
        for(size_t i=ldv+1; i<=allDVs.second; ++i)
        {
            const size_t si = static_cast<size_t>(i+pndv);

            const string& currSO =
                rm._dvSuboptList[i - rm._numSCOpts][
                    (int)des.GetVariableRep(si)
                    ];
            if(currSO != oldSO) break;

            const map<string, size_t>& varIndexMap =
                rm._dvSuboptIndexMap[i - rm._numSCOpts];
            map<string, size_t>::const_iterator it(varIndexMap.find(newSO));
            if(it == varIndexMap.end()) break;

            const double newRep = static_cast<double>(it->second);

            if(!dvis[si]->IsValidDoubleRep(newRep)) break;
            des.SetVariableRep(si, newRep);
        }
    }
}

double
LocalDesignVariableMutator::GenerateNewRep(
    Design& des,
    const roadmap& rm,
    const size_t dv,
    const size_t pndv
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    const DesignTarget& target = des.GetDesignTarget();
    const DesignVariableInfoVector& dvis = target.GetDesignVariableInfos();

    const bool dvIsVar = static_cast<int>(dv-pndv) > rm._hiOptVar;

    const double oldRep = des.GetVariableRep(dv);
    double newRep = oldRep;

    // We will do random reassignment for an option.  For a variable, we will
    // choose between random reassignment and gaussian offset.
    // See to it that the old and new reps are not the same if possible.
    for(int i=0; i<100 && oldRep == newRep; ++i)
    {
        if(dvIsVar || RandomNumberGenerator::RandomBoolean())
            newRep = dvis[dv]->GetRandomDoubleRep();
        else
            newRep = dvis[dv]->GetNearestValidDoubleRep(
                oldRep + RandomNumberGenerator::GaussianReal(
                    0.0, 0.20*dvis[dv]->GetDoubleRepRange()
                    )
                );
    }

    return newRep;
}

std::size_t
LocalDesignVariableMutator::GetTotalNumTecOpts(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    size_t ret = 0;
    for(size_t i=0; i<this->_roadmaps.size(); ++i)
        ret += this->_roadmaps[i]._optMap.size();
    return ret;
}

std::size_t
LocalDesignVariableMutator::GetTotalNumTecVars(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    size_t ret = 0;
    for(size_t i=0; i<this->_roadmaps.size(); ++i)
        ret += this->_roadmaps[i]._varMap.size();
    return ret;
}

std::size_t
LocalDesignVariableMutator::GetPriorNDV(
    const roadmap& rm
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    size_t ndv = 0;
    for(size_t i=0; i<this->_roadmaps.size(); ++i)
    {
        const roadmap& crm = this->_roadmaps[i];
        if(&crm == &rm) return ndv;
        ndv += crm.GetNumDVs();
    }
    return ndv;
}

std::pair<const LocalDesignVariableMutator::roadmap*, size_t>
LocalDesignVariableMutator::GetRoadmapAndPriorNDV(
    size_t dvNum
    )
{
    EDDY_FUNC_DEBUGSCOPE

    size_t ndv = 0;
    for(size_t i=0; i<this->_roadmaps.size(); ++i)
    {
        const roadmap& crm = this->_roadmaps[i];
        const size_t cndv = crm.GetNumDVs();
        if(dvNum < (ndv+cndv))
            return std::pair<const roadmap*, size_t>(&crm, ndv);
        ndv += cndv;
    }

    throw logic_error(
        "It should not be possible to request a variable that is beyond the "
        "end of the roadmaps."
        );
}

/*
================================================================================
Structors
================================================================================
*/
LocalDesignVariableMutator::LocalDesignVariableMutator(
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMutator(algorithm),
        _roadmaps()
{
    EDDY_FUNC_DEBUGSCOPE
}

LocalDesignVariableMutator::LocalDesignVariableMutator(
    const LocalDesignVariableMutator& copy
    ) :
        GeneticAlgorithmMutator(copy),
        _roadmaps(copy._roadmaps)
{
    EDDY_FUNC_DEBUGSCOPE
}

LocalDesignVariableMutator::LocalDesignVariableMutator(
    const LocalDesignVariableMutator& copy,
    GeneticAlgorithm& algorithm
    ) :
        GeneticAlgorithmMutator(copy, algorithm),
        _roadmaps(copy._roadmaps)
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
