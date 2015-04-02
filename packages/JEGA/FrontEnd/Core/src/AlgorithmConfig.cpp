/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Front End

    CONTENTS:

        Implementation of class AlgorithmConfig.

    NOTES:

        See notes of AlgorithmConfig.hpp.

    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Tue Feb 07 15:40:43 2006 - Original Version (JE)

================================================================================
*/




/*
================================================================================
Document This File
================================================================================
*/
/** \file
 * \brief Contains the implementation of the AlgorithmConfig class.
 */




/*
================================================================================
Includes
================================================================================
*/
// JEGAConfig.hpp should be the first include in all JEGA files.
#include <../Utilities/include/JEGAConfig.hpp>

#include <utilities/include/EDDY_DebugScope.hpp>
#include <../Utilities/include/ParameterDatabase.hpp>
#include <../Utilities/include/ParameterExtractor.hpp>
#include <../FrontEnd/Core/include/AlgorithmConfig.hpp>








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
    namespace FrontEnd {





/*
================================================================================
Static Member Data Definitions
================================================================================
*/








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
ParameterDatabase&
AlgorithmConfig::GetParameterDB(
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB;
}

const ParameterDatabase&
AlgorithmConfig::GetParameterDB(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB;
}

EvaluatorCreator&
AlgorithmConfig::GetTheEvaluatorCreator(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theEvalCreator;
}


/*
================================================================================
Public Methods
================================================================================
*/
bool
AlgorithmConfig::SetAlgorithmName(
    const string& name
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB.AddStringParam("method.jega.algorithm_name", name);
}

bool
AlgorithmConfig::SetPrintPopEachGen(
    bool doPrint
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB.AddBooleanParam("method.print_each_pop", doPrint);
}

bool
AlgorithmConfig::SetAlgorithmType(
    AlgType algType
    )
{
    EDDY_FUNC_DEBUGSCOPE

    JEGAIFLOG_CF_II_G_F(algType != MOGA && algType != SOGA, this,
        ostream_entry(lfatal(), "Invalid algorithm type ") << algType
        )

    return this->_theParamDB.AddStringParam(
        "method.algorithm", algType == MOGA ? "moga" : "soga"
        );
}

bool
AlgorithmConfig::SetDefaultLoggingLevel(
    const LevelType& defLevel
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB.AddIntegralParam("method.output", defLevel);
}

bool
AlgorithmConfig::SetLoggingFilename(
    const string& filename
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB.AddStringParam("method.log_file", filename);
}

bool
AlgorithmConfig::SetOutputFilenamePattern(
    const std::string& fpattern
    )
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB.AddStringParam(
        "method.jega.final_data_filename", fpattern
        );
}


#define SET_OPNAME_METHOD(opname, keystr) \
    bool \
    AlgorithmConfig::Set##opname##Name( \
        const string& name \
        ) \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return this->_theParamDB.AddStringParam(keystr, name); \
    }

SET_OPNAME_METHOD(Mutator, "method.mutation_type")
SET_OPNAME_METHOD(Converger, "method.jega.convergence_type")
SET_OPNAME_METHOD(Crosser, "method.crossover_type")
SET_OPNAME_METHOD(NichePressureApplicator, "method.jega.niching_type")
SET_OPNAME_METHOD(FitnessAssessor, "method.fitness_type")
SET_OPNAME_METHOD(Selector, "method.replacement_type")
SET_OPNAME_METHOD(Initializer, "method.initialization_type")
SET_OPNAME_METHOD(MainLoop, "method.jega.mainloop_type")
SET_OPNAME_METHOD(PostProcessor, "method.jega.postprocessor_type")



string
AlgorithmConfig::GetAlgorithmName(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB.GetString("method.jega.algorithm_name");
}

bool
AlgorithmConfig::GetPrintPopEachGen(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB.GetBoolean("method.print_each_pop");
}

AlgorithmConfig::AlgType
AlgorithmConfig::GetAlgorithmType(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE

    string algType(this->_theParamDB.GetString("method.algorithm"));

    JEGAIFLOG_CF_II_G_F(algType != "moga" && algType != "soga", this,
        text_entry(lfatal(), "Invalid algorithm type ") << algType
        )

    return algType == "moga" ? MOGA : SOGA;
}

AlgorithmConfig::LevelType
AlgorithmConfig::GetDefaultLoggingLevel(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB.GetIntegral("method.output");
}

string
AlgorithmConfig::GetLoggingFilename(
    ) const
{
    EDDY_FUNC_DEBUGSCOPE
    return this->_theParamDB.GetString("method.log_file");
}


#define GET_OPNAME_METHOD(opname, keystr) \
    string \
    AlgorithmConfig::Get##opname##Name( \
        ) const \
    { \
        EDDY_FUNC_DEBUGSCOPE \
        return this->_theParamDB.GetString(keystr); \
    }

GET_OPNAME_METHOD(Mutator, "method.mutation_type")
GET_OPNAME_METHOD(Converger, "method.jega.convergence_type")
GET_OPNAME_METHOD(Crosser, "method.crossover_type")
GET_OPNAME_METHOD(NichePressureApplicator, "method.jega.niching_type")
GET_OPNAME_METHOD(FitnessAssessor, "method.fitness_type")
GET_OPNAME_METHOD(Selector, "method.replacement_type")
GET_OPNAME_METHOD(Initializer, "method.initialization_type")
GET_OPNAME_METHOD(MainLoop, "method.jega.mainloop_type")
GET_OPNAME_METHOD(PostProcessor, "method.jega.postprocessor_type")





/*
================================================================================
Subclass Visible Methods
================================================================================
*/








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







/*
================================================================================
Structors
================================================================================
*/
AlgorithmConfig::AlgorithmConfig(
    EvaluatorCreator& creator,
    JEGA::Utilities::ParameterDatabase& pdb
    ) :
        _theParamDB(pdb),
        _theEvalCreator(creator)
{
    EDDY_FUNC_DEBUGSCOPE
}







/*
================================================================================
End Namespace
================================================================================
*/
    } // namespace FrontEnd
} // namespace JEGA

