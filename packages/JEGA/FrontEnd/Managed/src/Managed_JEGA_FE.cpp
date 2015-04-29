/*
================================================================================
    PROJECT:

        John Eddy's Genetic Algorithms (JEGA) Managed Front End

    CONTENTS:



    NOTES:



    PROGRAMMERS:

        John Eddy (jpeddy@sandia.gov) (JE)

    ORGANIZATION:

        Sandia National Laboratories

    COPYRIGHT:

        See the LICENSE file in the top level JEGA directory.

    VERSION:

        1.0.0

    CHANGES:

        Thu Feb 09 10:51:11 2006 - Original Version (JE)

================================================================================
*/
// Managed_JEGA_FE.cpp : main project file.

#include <stdafx.h>

#include <MDriver.hpp>
#include <MDesign.hpp>
#include <MSolution.hpp>
#include <MEvaluator.hpp>
#include <MAlgorithmConfig.hpp>
#include <MParameterDatabase.hpp>
#include <MEvaluationFunctor.hpp>

#pragma unmanaged
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <GeneticAlgorithm.hpp>
// All of the following includes are for the sole purpose of bringing the full
// definitions of these objects into the assembly.  The objects are not needed
// in the code of this file.
#include <FitnessRecord.hpp>
#include <GeneticAlgorithmMutator.hpp>
#include <GeneticAlgorithmCrosser.hpp>
#include <GeneticAlgorithmMainLoop.hpp>
#include <GeneticAlgorithmSelector.hpp>
#include <GeneticAlgorithmConverger.hpp>
#include <GeneticAlgorithmInitializer.hpp>
#include <GeneticAlgorithmPostProcessor.hpp>
#include <GeneticAlgorithmFitnessAssessor.hpp>
#include <../Utilities/include/ConstraintInfo.hpp>
#include <GeneticAlgorithmNichePressureApplicator.hpp>
#include <../Utilities/include/ConstraintTypeBase.hpp>
#include <../Utilities/include/ConstraintNatureBase.hpp>
#include <../Utilities/include/DesignVariableTypeBase.hpp>
#include <../Utilities/include/DesignVariableNatureBase.hpp>
#include <../Utilities/include/ObjectiveFunctionTypeBase.hpp>
#include <../Utilities/include/ObjectiveFunctionNatureBase.hpp>

#pragma managed

using namespace System;
using namespace JEGA::FrontEnd::Managed;
using namespace System::Collections::Generic;

MANAGED_CLASS(public, TestEvaluator) :
    public MEvaluationFunctor
{
    private:

        static const double SQRT3INV = 1.0 / Math::Sqrt(3.0);

    public:

        virtual
        bool
        Evaluate(
            MDesign MOH des,
            List<MDesign MOH> MOH injections
            )
        {
            eddy::utilities::uint64_t ndv = des->GetNDV();
            double f0 = 0.0;
            double f1 = 0.0;

            for(eddy::utilities::uint64_t i=0; i<ndv; ++i) {
                const double xi = des->GetVariable(i);
                f0 += Math::Pow(xi - (SQRT3INV), 2.0);
                f1 += Math::Pow(xi + (SQRT3INV), 2.0);
            }

            des->SetObjective(0, 1.0 - Math::Exp(-f0));
            des->SetObjective(1, 1.0 - Math::Exp(-f1));

            return true;
        }

        virtual
        bool
        Evaluate(
            DesignVector MOH designs,
            System::Collections::Generic::List<MDesign MOH> MOH injections
            )
        {
            return false;
        }

        virtual
        bool
        IsBatchEvaluator(
            )
        {
            return false;
        }

        virtual
        bool
        MayInjectDesigns(
            )
        {
            return false;
        }

};



void
WriteSolution(
    MSolution MOH sol,
    System::IO::TextWriter MOH stream
    )
{
    EDDY_FUNC_DEBUGSCOPE

    try {

        int ndv = System::Convert::ToInt32(sol->GetNDV());

        if(ndv > 0)
        {
            for(int var = 0; var < (ndv - 1); ++var) {
                stream->Write(sol->GetVariable(var).ToString());
                stream->Write("\t");
            }

            stream->Write(sol->GetVariable(ndv - 1));
        }

        if(sol->IsEvaluated() && !sol->IsIllconditioned())
        {
            int nof = System::Convert::ToInt32(sol->GetNOF());
            int ncn = System::Convert::ToInt32(sol->GetNCN());

            if (nof > 0)
                for(int obj = 0; obj<nof; ++obj) {
                    stream->Write("\t");
                    stream->Write(sol->GetObjective(obj).ToString());
                }

            if (ncn > 0)
                for(int con = 0; con<ncn; ++con) {
                    stream->Write("\t");
                    stream->Write(sol->GetConstraint(con).ToString());
                }
        }
    }
    catch(System::Exception MOH ) {

    }
}

void
WriteSolutions(
    SolutionVector MOH sols,
    System::IO::TextWriter MOH stream
    )
{
    EDDY_FUNC_DEBUGSCOPE

    try {

        for(int i=0; i<sols->Count; ++i)
        {
            MSolution MOH sol = NON_GENERIC_CAST(
                MSolution MOH, MANAGED_LIST_ITEM(sols, i)
                );
            WriteSolution(sol, stream);
            stream->WriteLine();
        }
    }
    catch(System::Exception MOH ) {

    }

}



int
main(
#if _MSC_VER < 1400
     int, char*[], char*[]
#else
    array<System::String ^> ^ args
#endif
    )
{
    try {
        MDriver::InitializeJEGA(
            "JEGAGlobal.log", JEGA::Logging::LevelClass::debug, 123456,
            MAlgorithmConfig::FatalBehavior::ABORT
            );

        MProblemConfig MOH pConfig = MANAGED_GCNEW MProblemConfig();
        GC::SuppressFinalize(pConfig);

        pConfig->AddContinuumRealVariable("x1", -4.0, 4.0, 6);
        pConfig->AddContinuumRealVariable("x2", -4.0, 4.0, 6);
        pConfig->AddContinuumRealVariable("x3", -4.0, 4.0, 6);

        pConfig->AddNonlinearMinimizeObjective("F1");
        pConfig->AddNonlinearMinimizeObjective("F2");

        TestEvaluator MOH functor = MANAGED_GCNEW TestEvaluator();
        GC::SuppressFinalize(functor);
        MEvaluator MOH evaluator = MANAGED_GCNEW MEvaluator(functor);
        GC::SuppressFinalize(evaluator);
        MAlgorithmConfig MOH aConfig = MANAGED_GCNEW MAlgorithmConfig(evaluator);
        GC::SuppressFinalize(aConfig);

        MParameterDatabase MOH pdb = aConfig->GetParameterDB();
        aConfig->SetAlgorithmType(MAlgorithmConfig::AlgType::MOGA);
        aConfig->SetLoggingFilename("JEGA_VBRun.log");
        aConfig->SetDefaultLoggingLevel(MAlgorithmConfig::Silent);
        aConfig->SetAlgorithmName("MOGA_1");
        aConfig->SetPrintPopEachGen(false);

        aConfig->SetConvergerName("metric_tracker");
        aConfig->SetCrosserName("shuffle_random");
        aConfig->SetNichePressureApplicatorName("radial");
        aConfig->SetFitnessAssessorName("domination_count");
        aConfig->SetInitializerName("unique_random");
        aConfig->SetMainLoopName("duplicate_free");
        aConfig->SetMutatorName("replace_uniform");
        aConfig->SetSelectorName("below_limit");
        aConfig->SetPostProcessorName("null_postprocessor");

        pdb->AddIntegralParam("method.population_size", 50);
        pdb->AddDoubleParam("method.mutation_rate", 0.1);
        pdb->AddIntegralParam("method.max_iterations", System::Int32::MaxValue);
        pdb->AddIntegralParam("method.max_function_evaluations", 500);
        pdb->AddDoubleParam("method.jega.percent_change", 0.03);
        pdb->AddSizeTypeParam("method.jega.num_generations", 10);
        pdb->AddDoubleParam("method.crossover_rate", 0.8);
        pdb->AddSizeTypeParam("method.jega.num_offspring", 2);
        pdb->AddSizeTypeParam("method.jega.num_parents", 2);
        pdb->AddSizeTypeParam("method.jega.num_cross_points", 3);
        pdb->AddDoubleParam("method.jega.fitness_limit", 5);
        pdb->AddDoubleParam("method.jega.shrinkage_percentage", 0.9);

        DoubleVector MOH nicheVector = MANAGED_GCNEW DoubleVector();
        nicheVector->Add(MANAGED_BOX(0.05));
        nicheVector->Add(MANAGED_BOX(0.05));
        pdb->AddDoubleVectorParam("method.jega.niche_vector", nicheVector);

        MDriver MOH driver = MANAGED_GCNEW MDriver(pConfig);
        GC::SuppressFinalize(driver);
        SolutionVector MOH results = driver->ExecuteAlgorithm(aConfig);

        GC::ReRegisterForFinalize(driver);
        GC::ReRegisterForFinalize(pConfig);
        GC::ReRegisterForFinalize(evaluator);
        GC::ReRegisterForFinalize(aConfig);

        WriteSolutions(results, System::Console::Error);

        return 0;
    }
    catch(System::Exception MOH ex) {
        System::Console::Write("Caught a system exception reading: ");
        System::Console::WriteLine(ex->Message);
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Caught a std exception reading: " << ex.what();
    }
}

