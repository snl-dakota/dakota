/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDAdaptiveSampling
//- Description: Implementation code for NonDAdaptiveSampling class
//- Owner:       Dan Maljovec
//- Checked by:  Brian Adams
//- Version:

//- Edited by: Mohamed S. Ebeida on 11/26/2012


#include "NonDAdaptiveSampling.hpp"
#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDLHSSampling.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "pecos_data_types.hpp"
#include "pecos_stat_util.hpp"
#include "DakotaApproximation.hpp"
#include <boost/lexical_cast.hpp>
#include "FSUDesignCompExp.hpp"
#include <sstream>
#include <fstream>

// Options HAVE_MORSE_SMALE and HAVE_DIONYSUS are managed in CMake
#ifdef HAVE_MORSE_SMALE
#include "MorseSmaleComplex.hpp"
#endif

static const char rcsId[]=
  "@(#) $Id: NonDAdaptiveSampling.cpp 7035 2012-6-6 21:45:39Z mseldre $";


namespace Dakota
{
	/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
	NonDAdaptiveSampling::NonDAdaptiveSampling(ProblemDescDB& problem_db, Model& model): NonDSampling(problem_db, model)
	{	
		#pragma region Class Constructor:
		
	        // sampleType default in DataMethod.cpp is SUBMETHOD_DEFAULT (0).
	        // Enforce an LHS default for this method.
	        if (!sampleType)
		  sampleType = SUBMETHOD_LHS;

		AMSC = NULL;

		//Defaults are set before parsing input parameters
		outputValidationData = false;
		numKneighbors = 5;
                numRounds = maxIterations;
                if (numRounds == -1)
                  numRounds = 100;
                
                numEmulEval = probDescDB.get_int("method.nond.samples_on_emulator");
                if (numEmulEval == 0)
                  numEmulEval = 400; 
		batchSize = 1;
		const IntVector& db_refine_samples = 
		  probDescDB.get_iv("method.nond.refinement_samples");
		if (db_refine_samples.length() == 1)
		  batchSize = db_refine_samples[0];
		else if (db_refine_samples.length() > 1) {
		  Cerr << "\nError (NonDAdaptiveSampling): refinement_samples must be "
		       << "length 1 if specified." << std::endl;
		  abort_handler(PARSE_ERROR);
		}
                batchStrategy = probDescDB.get_string("method.batch_selection");
                if (batchStrategy.empty())
		  batchStrategy="naive";
                scoringMetric = probDescDB.get_string("method.fitness_metric");
                if (scoringMetric == "predicted_variance") 
                  scoringMetric = "alm";
                if (scoringMetric.empty())
                  scoringMetric = "alm";
                
                Cout << "numEmulEval " << numEmulEval << '\n';
                Cout << "numRounds " << numRounds << '\n';
                Cout << "batchSize " << batchSize << '\n';
                Cout << "batchStrategy " << batchStrategy  << '\n';
                Cout << "scoringMetric " << scoringMetric  << '\n';
                
      		////***ATTENTION***
		//// So, I hard-coded this directory, it only matters if you set
		//// outputValidationData to true
		////***END ATTENTION***

		outputDir = "adaptive.results";

		//Now parse the inputs
		const StringArray& misc_options = probDescDB.get_sa("method.coliny.misc_options");
                if (misc_options.size() > 0)
                  parse_options();

                Cout << "misc options size " << misc_options.size()  << '\n';
		String sample_reuse;
		UShortArray approx_order; // not used by GP/kriging
		short corr_order = -1, data_order = 1, corr_type = NO_CORRECTION;
		if (probDescDB.get_bool("method.derivative_usage"))
		{
		  if (iteratedModel.gradient_type() != "none") data_order |= 2;
		  if (iteratedModel.hessian_type()  != "none") data_order |= 4;
		}

		bool vary_pattern = false;
		const String& import_pts_file = probDescDB.get_string("method.import_build_points_file");
		int samples = numSamples;
		if (!import_pts_file.empty())
		{
			samples = 0; sample_reuse = "all";
		}
                 
                //**NOTE:  We are hardcoding the sample type to LHS and the approximation type to kriging for now
		//if (sampleDesign == RANDOM_SAMPLING)
		//{
		gpBuild.assign_rep(new NonDLHSSampling(iteratedModel, SUBMETHOD_DEFAULT,
							   samples, randomSeed, rngName,
							   varyPattern, ACTIVE_UNIFORM), false);
		//}
		//else
		//{
		//	gpBuild.assign_rep(new FSUDesignCompExp(iteratedModel, samples, randomSeed,
		//					   sampleDesign));
		//}
                approx_type = "global_kriging";
		ActiveSet gp_set = iteratedModel.current_response().active_set(); // copy
		gp_set.request_values(1); // no surr deriv evals, but GP may be grad-enhanced
		gpModel.assign_rep(new DataFitSurrModel(gpBuild, iteratedModel,
						  gp_set, approx_type, approx_order, corr_type, corr_order, data_order,
						  outputLevel, sample_reuse, import_pts_file,
						  probDescDB.get_ushort("method.import_build_format"),
						  probDescDB.get_bool("method.import_build_active_only"),
						  probDescDB.get_string("method.export_approx_points_file"),
						  probDescDB.get_ushort("method.export_approx_format")), false);

		vary_pattern = true; // allow seed to run among multiple approx sample sets
							 // need to add to input spec

		////***ATTENTION***
		//// Until this starts working, we are forcing the candidates to be selected by
		//// LHS
		////***END ATTENTION***
		//if(sampleDesign == RANDOM_SAMPLING){
		if(true)
		{
			construct_lhs(gpEval, gpModel, SUBMETHOD_DEFAULT, numEmulEval, randomSeed,
						  rngName, vary_pattern);

			numFinalEmulEval = 10000; // may be we should add that as a paramter
			construct_lhs(gpFinalEval, gpModel, SUBMETHOD_DEFAULT, numFinalEmulEval, randomSeed,
						  rngName, vary_pattern);
		}
		else
		{
			construct_fsu_sampler(gpEval, gpModel, numEmulEval, randomSeed,sampleDesign);
			//gpEval.assign_rep(new FSUDesignCompExp(gpModel, numEmulEval, randomSeed, sampleDesign));
		}

		#pragma endregion
	}

  NonDAdaptiveSampling::~NonDAdaptiveSampling()
  { }


  bool NonDAdaptiveSampling::resize()
  {
    bool parent_reinit_comms = NonDSampling::resize();

    Cerr << "\nError: Resizing is not yet supported in method "
         << method_enum_to_string(methodName) << "." << std::endl;
    abort_handler(METHOD_ERROR);

    return parent_reinit_comms;
  }

  void NonDAdaptiveSampling::derived_init_communicators(ParLevLIter pl_iter)
  {
    iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);

    // gpEval and gpFinalEval use NoDBBaseConstructor, so no need to
    // manage DB list nodes at this level
    gpEval.init_communicators(pl_iter);
    gpFinalEval.init_communicators(pl_iter);
  }

  void NonDAdaptiveSampling::derived_set_communicators(ParLevLIter pl_iter)
  {
    NonD::derived_set_communicators(pl_iter);

    // gpEval and gpFinalEval use NoDBBaseConstructor, so no need to
    // manage DB list nodes at this level
    gpEval.set_communicators(pl_iter);
    gpFinalEval.set_communicators(pl_iter);
  }

  void NonDAdaptiveSampling::derived_free_communicators(ParLevLIter pl_iter)
  {
    gpFinalEval.free_communicators(pl_iter);
    gpEval.free_communicators(pl_iter);

    iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
  }



	//This is where all the magic happens
	void NonDAdaptiveSampling::core_run() 
	{
		#pragma region Quantify Uncertainity:
		numPtsTotal = numSamples + numRounds * batchSize;		
 
		// Build initial GP model.  This will be built over the initial LHS sample set
		// defined in the constructor.
		gpModel.build_approximation();

		gpCvars.resize(numEmulEval);
		gpVar.resize(numEmulEval);
		gpMeans.resize(numEmulEval);		

		predictionErrors.resize(numRounds+1);

		
		#ifdef HAVE_MORSE_SMALE
		for (int ifunc = 0; ifunc < numFunctions; ifunc++)
		{
			update_amsc(ifunc);
		}
		#endif

		////***ATTENTION***
		//// I do this all over the place, but there has to be a better way to obtain
		//// the dimensionality of the domain under test search "dim ="
		////***END ATTENTION***

		// BMA: you should just be able to use numContinuousVars for now, or this:
		//  size_t dim = numContinuousVars + numDiscreteIntVars + numDiscreteStringVars + numDiscreteRealVars;
		int dim = 0;
		const Pecos::SurrogateData& gp_data = gpModel.approximation_data(0);

		if(gp_data.points() > 1) dim = gp_data.continuous_variables(0).length();

		int i,j;
 

		// We have built the initial GP.  Now we need to go through, per response 
		// function and response level and calculate the failure probability. 
		// We will need to add error handling:  we will only be calculating 
		// results per response level, not probability level or reliability index.
   
		int iter;
		RealVectorArray new_Xs;

		////***ATTENTION***
		//// This is a bit clunky, but I am writing my own file called improvement.txt
		//// which has just the information I am concerned with and then I post-process
		//// these files with some python scripts
		////***END ATTENTION***
		std::stringstream ss;
		ss << "improvement.txt";
		String improvementFile;
		ss >> improvementFile;
		std::ofstream fout(improvementFile.c_str());
		fout << "Round\tTrue_Min\tTrue_Max\tTrue_Saddle\tModel_Min\tModel_Max"
			 << "\tModel_Saddle\tBottleneck\tRMSPE" << std::endl;

		initialize_level_mappings();


		for (int k = 0; k < numRounds; k++) 
		{ 
			pick_new_candidates();

			score_new_candidates();
					
			new_Xs = drawNewX(k);
			output_round_data(k);

			////***ATTENTION***
			//// The following two lines will write data to improvement.txt a file for
			//// measuring how much improvement your surrogate is making as we progress,
			//// I have disabled a bunch of the output which compares the topologies.
			////***END ATTENTION***
			fout << k << "\t";
			compare_complices(dim, fout);

			// add new_X to the build points and append approximation
			VariablesArray points_to_add;
			IntResponseMap responses_to_add;
			for(int i = 0; i < new_Xs.size(); i++) 
			{
				iteratedModel.continuous_variables(new_Xs[i]);
				iteratedModel.evaluate();
				responses_to_add.insert(IntResponsePair(iteratedModel.evaluation_id(),
										iteratedModel.current_response()));
				points_to_add.push_back(iteratedModel.current_variables());
			}

			gpModel.append_approximation(points_to_add,responses_to_add, true);
			
			#ifdef HAVE_MORSE_SMALE
			for (int ifunc = 0; ifunc < numFunctions; ifunc++)
			{
				update_amsc(ifunc);
			}
			#endif

			Cout << "Done with iteration  " << k << std::endl; 
		}

		// Exploring the final Emulator:
		for (int ifunc = 0; ifunc < numFunctions; ifunc++)
		{
			size_t num_levels = requestedRespLevels[ifunc].length();
			for (int ilevel = 0; ilevel < num_levels; ilevel++) computedProbLevels[ifunc][ilevel] = 0.0;		
		}
	
		// Exploring Final Emulator
		ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
		gpFinalEval.run(pl_iter);
		const IntResponseMap& all_resp = gpFinalEval.all_responses();
		IntRespMCIter resp_it = all_resp.begin();

		for (int icand = 0; icand < numFinalEmulEval; icand++) 
		{
			for (int ifunc = 0; ifunc < numFunctions; ifunc++)
			{
				Real response_value = resp_it->second.function_value(ifunc);

				size_t num_levels = requestedRespLevels[ifunc].length();
				for (int ilevel = 0; ilevel < num_levels; ilevel++) 
				{
					Real z = requestedRespLevels[ifunc][ilevel];						
					if (response_value < z) computedProbLevels[ifunc][ilevel]+=1.0;
				}
			}
			++resp_it;
		}

		double sf = 1.0 / double(numFinalEmulEval);
		for (int ifunc = 0; ifunc < numFunctions; ifunc++)
		{
			size_t num_levels = requestedRespLevels[ifunc].length();
			for (int ilevel = 0; ilevel < num_levels; ilevel++) 
			{
				computedProbLevels[ifunc][ilevel] *= sf; 

				Cout << "Fraction Fail IS " << computedProbLevels[ifunc][ilevel] << '\n';    
			}
		}

		Cout << "Scoring Metric is " << scoringMetric << '\n';

		predictionErrors(numRounds) = compute_rmspe();
		////***ATTENTION***
		//// If you are performing the optimization pipeline this next line is 
		//// uncommented
		////***END ATTENTION***
		//output_for_optimization(dim);
		fout.close();
		#pragma endregion
	}

	void NonDAdaptiveSampling::pick_new_candidates()
	{
		#pragma region Pick New Candidates from Emulator:
		RealVector temp_cvars;

		// generate new set of emulator samples.  Note this will have a different seed  each time.

		ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
		gpEval.run(pl_iter);

		// obtain results 
		const RealMatrix&  all_samples = gpEval.all_samples();
		const IntResponseMap& all_resp = gpEval.all_responses();

		for (int i = 0; i < numEmulEval; i++) 
		{
			temp_cvars = Teuchos::getCol(Teuchos::View,	const_cast<RealMatrix&>(all_samples), i);
			gpCvars[i] = temp_cvars;
      		gpModel.continuous_variables(temp_cvars);
			if(approx_type == "global_kriging")
			{
				gpVar[i] = gpModel.approximation_variances(gpModel.current_variables());
			}
			else
			{
				gpVar[i] = 0;
			}
		}

		IntRespMCIter resp_it = all_resp.begin();
		for (int j = 0; j < numEmulEval; ++j) 
		{
			RealVector temp_resp(numFunctions);
			for (int i = 0; i < numFunctions; i++)
				temp_resp(i) = resp_it->second.function_value(i);
					
			gpMeans[j] = temp_resp; ++resp_it;
		}
		#pragma endregion
	}

	void NonDAdaptiveSampling::score_new_candidates()
	{
		#pragma region Score New Candidates:
		// calculate the scores
		emulEvalScores.resize(0);
		if(scoringMetric == "alm")
			calc_score_alm();
		else if(scoringMetric == "distance")
			calc_score_delta_x( );
		else if(scoringMetric == "gradient")
			calc_score_delta_y( );
		else if(scoringMetric == "bottleneck")
			calc_score_topo_bottleneck( );
		else if(scoringMetric == "avg_persistence")
			calc_score_topo_avg_persistence(0);
		else if(scoringMetric == "highest_persistence")
			calc_score_topo_highest_persistence(0);
		else if(scoringMetric == "alm_topo_hybrid")
			calc_score_topo_alm_hybrid(0);
		#pragma endregion
	}

	void NonDAdaptiveSampling::calc_score_alm( ) 
	{
		#pragma region Score Emultor sample points based on their approximation variance:
		emulEvalScores.resize(numEmulEval);
		for (int i = 0; i < numEmulEval; i++)
		{
			Real max_score;
			for (int respFnCount = 0; respFnCount < numFunctions; respFnCount++) 
			{			
				gpModel.continuous_variables(gpCvars[i]);
				Real score = gpModel.approximation_variances(gpModel.current_variables())[respFnCount];
				if (respFnCount == 0 || score > max_score) max_score = score;
			}
			emulEvalScores(i) = max_score;
		}
		#pragma endregion
	}

	void NonDAdaptiveSampling::calc_score_delta_x( ) 
	{
		#pragma region Score Emulator sample points based on the closest data point:
		emulEvalScores.resize(numEmulEval);
		for (int i = 0; i < numEmulEval; i++) 
		{
			Real max_score;
			for (int respFnCount = 0; respFnCount < numFunctions; respFnCount++) 
			{			
				const Pecos::SurrogateData& gp_data = gpModel.approximation_data(respFnCount);
				double min_sq_dist;
				int min_index;
				bool first = true;
				for (int j = 0; j < gp_data.points(); j++) // This is a Naiive way to retriev closet data point. SHOULD BE RELACED IN THE FUTURE FOR BETTER PERFORMANCE!
				{
					double sq_dist = 0;
					for(int d = 0; d < gp_data.continuous_variables(j).length(); d++)
					{
						sq_dist += pow(gpCvars[i][d] - gp_data.continuous_variables(j)[d],2);
					}
					if(first || sq_dist < min_sq_dist) 
					{
						min_sq_dist = sq_dist;
						min_index = j;
						first = false;
					}
				}
				Real score = sqrt(min_sq_dist);
				if (respFnCount == 0 || score > max_score) max_score = score;
			}
			emulEvalScores(i) = max_score;
		}
		#pragma endregion
	}
	
	void NonDAdaptiveSampling::calc_score_delta_y( ) 
	{
		#pragma region Score Emulator sample points based on the response function difference of closest data point and current candidate:
		emulEvalScores.resize(numEmulEval);
		for (int i = 0; i < numEmulEval; i++) 
		{
			Real max_score;
			for (int respFnCount = 0; respFnCount < numFunctions; respFnCount++) 
			{	
				const Pecos::SurrogateData& gp_data = gpModel.approximation_data(respFnCount);
				double min_sq_dist;
				int min_index;
				bool first = true;

				for (int j = 0; j < gp_data.points(); j++) // This is a Naiive way to retriev closet data point. SHOULD BE RELACED IN THE FUTURE FOR BETTER PERFORMANCE!
				{
					double sq_dist = 0;
					for(int d = 0; d < gp_data.continuous_variables(j).length(); d++)
					{
						sq_dist += pow(gpCvars[i][d] - gp_data.continuous_variables(j)[d],2);
					}
					if(first || sq_dist < min_sq_dist) 
					{
						min_sq_dist = sq_dist;
						min_index = j;
						first = false;
					}
				}					
				Real score = fabs(gpMeans[i][respFnCount] - gp_data.response_function(min_index));
				if (respFnCount == 0 || score > max_score) max_score = score;
			}
			emulEvalScores(i) = max_score;
		}
		#pragma endregion
	}

	
	void NonDAdaptiveSampling::calc_score_topo_bottleneck( )
	{
		#pragma region Score Emulator sample points based on Bottleneck distance:

		#if defined(HAVE_MORSE_SMALE) && defined(HAVE_DIONYSUS)
		emulEvalScores.resize(numEmulEval);
		double *temp_x = NULL;

		if(numEmulEval > 0)
			temp_x = new double[gpCvars[0].length()+1];


		for (int i = 0; i < numEmulEval; i++)
		{

			for(int d = 0; d < gpCvars[i].length(); d++)
			{
				temp_x[d] = gpCvars[i][d];
			}

			Real max_score;
			for (int respFnCount = 0; respFnCount < numFunctions; respFnCount++)
			{
				temp_x[gpCvars[i].length()] = gpMeans[i][respFnCount];
				Real score = ScoreTOPOB((*AMSC), temp_x);
				if (respFnCount == 0 || score > max_score) max_score = score;
			}
			emulEvalScores(i) = max_score;
		}

		delete [] temp_x;

		#else
	  	  #ifdef HAVE_MORSE_SMALE
			Cout << "Dionysus library not enabled, therefore cannot compute the "
				 << "bottleneck distance score, setting all scores to zero" << std::endl;
	  	  #else
			Cout << "ANN library not enabled, therefore cannot compute approximate "
				 << "Morse-Smale complex or bottleneck score, setting all scores to "
	             << "zero" << std::endl;
	  	  #endif
		  abort_handler(-1);
		#endif
		#pragma endregion
	}

	void NonDAdaptiveSampling::update_amsc(int respFnCount) 
	{
		#pragma region Update Morse Smale Complex using ANN
		#ifdef HAVE_MORSE_SMALE
		delete AMSC;
		AMSC = NULL;
 
		const Pecos::SurrogateData& gp_data = gpModel.approximation_data(respFnCount);
		if(gp_data.points() < 1) return;

		int n = gp_data.points();
		int d = gp_data.continuous_variables(0).length();
		double *data_resp_vector = new double[n*(d+1)];

		for (int i = 0; i < n; i++) 
		{
			for(int j = 0; j < d; j++)
			{
				data_resp_vector[i*(d+1)+j] = gp_data.continuous_variables(i)[j];
			}
			data_resp_vector[i*(d+1)+d] = gp_data.response_function(i);
		} 
		AMSC = new MS_Complex(data_resp_vector, d + 1, n, numKneighbors);
		delete [] data_resp_vector;
		#else
			Cout << "ANN library not enabled, therefore cannot compute approximate "
				 << "Morse-Smale complex" << std::endl;
			abort_handler(-1);
		#endif
		#pragma endregion
	}

////***ATTENTION***
//// This function should go away at some point. I use it every now and again
//// for debugging purposes, oh yeah and it is used to compute yModel data which
//// is used to compute the RMSE
////***END ATTENTION***
void NonDAdaptiveSampling::output_round_data(int round, int respFnCount) 
{

  Model& surrogate_model = gpModel.surrogate_model();

  RealVector alm_set;
  RealVector delta_x_set;
  RealVector delta_y_set;
  RealVector topo_b_set;
  RealVector topo_p_set;
  RealVector hybrid_set;

  if(outputValidationData) {
    alm_set.resize(validationSetSize);
    delta_x_set.resize(validationSetSize);
    delta_y_set.resize(validationSetSize);
    topo_b_set.resize(validationSetSize);
    topo_p_set.resize(validationSetSize);
    hybrid_set.resize(validationSetSize);
  }

  for(int i = 0; i < validationSetSize; i++) {
    surrogate_model.continuous_variables(validationSet[i]);
    surrogate_model.evaluate();
    yModel[i] = surrogate_model.current_response().function_value(respFnCount);

    if(outputValidationData) {
      alm_set[i] = (approx_type == "global_kriging") ?
          calc_score_alm(respFnCount, validationSet[i]) : 0;
      delta_x_set[i] = calc_score_delta_x(respFnCount, validationSet[i]);
      delta_y_set[i] = calc_score_delta_y(respFnCount, validationSet[i]);
      topo_b_set[i] = calc_score_topo_bottleneck(respFnCount, validationSet[i]);
      topo_p_set[i] = 
        calc_score_topo_avg_persistence(respFnCount, validationSet[i]);
      hybrid_set[i] =(approx_type == "global_kriging") ? 
          calc_score_topo_alm_hybrid(respFnCount, validationSet[i]) : 0;
    }
  }

  if(outputValidationData)
  {
    std::stringstream cand_stream;
    std::stringstream y_model_stream;
    std::stringstream alm_stream;
    std::stringstream delta_y_stream;
    std::stringstream topob_stream;
    std::stringstream topop_stream;
    std::stringstream hybrid_stream;
    std::stringstream delta_x_stream;

    char temp_str[60];

    cand_stream << outputDir << "/candidate_" << round << ".txt\n"; 
    cand_stream.getline(temp_str,60);
    std::ofstream candidate_out(temp_str);

    y_model_stream << outputDir << "/y_model_" << round << ".txt\n"; 
    y_model_stream.getline(temp_str,60);
    std::ofstream y_model_out(temp_str);

    alm_stream << outputDir << "/alm_" << round << ".txt\n"; 
    alm_stream.getline(temp_str,60);
    std::ofstream alm_out(temp_str);

    delta_y_stream << outputDir << "/delta_y_" << round << ".txt\n"; 
    delta_y_stream.getline(temp_str,60);
    std::ofstream delta_y_out(temp_str);

    topop_stream << outputDir << "/topop_" << round << ".txt\n"; 
    topop_stream.getline(temp_str,60);
    std::ofstream topo_b_out(temp_str);

    topob_stream << outputDir << "/topob_" << round << ".txt\n"; 
    topob_stream.getline(temp_str,60);
    std::ofstream topo_p_out(temp_str);

    hybrid_stream << outputDir << "/hybrid_" << round << ".txt\n"; 
    hybrid_stream.getline(temp_str,60);
    std::ofstream hybrid_out(temp_str);

    delta_x_stream << outputDir << "/delta_x_" << round << ".txt\n"; 
    delta_x_stream.getline(temp_str,60);
    std::ofstream delta_x_out(temp_str);

    candidate_out << std::setprecision(15);

    for(int i = 0; i < gpCvars.size(); i++)
    {
      for(int j = 0; j < gpCvars[i].length(); j++)
      {
        candidate_out << gpCvars[i][j] << " ";
      }
      candidate_out << gpMeans[i][0] << " ";
      candidate_out << emulEvalScores[i] << "\n";
    }

    RealVectorArray new_Xs = drawNewX(round);

    for(int i = 0; i < new_Xs.size(); i++) {
      iteratedModel.continuous_variables(new_Xs[i]);
      iteratedModel.evaluate();

      for(int j = 0; j < new_Xs[i].length(); j++)
        candidate_out << new_Xs[i][j] << " ";
      candidate_out 
        << iteratedModel.current_response().function_value(respFnCount)
        << std::endl;
    }

    //candidate_out << gpCvars;

    y_model_out << std::setprecision(15);
    for(int i = 0; i < yModel.length(); i++)
    {
      y_model_out << yModel[i] << " "; 
      y_model_out << alm_set[i] << "\n"; 
    }
    alm_out << alm_set;
    delta_x_out << delta_x_set;
    delta_y_out << delta_y_set;
    topo_b_out << topo_b_set;
    topo_p_out << topo_p_set;
    hybrid_out << hybrid_set;
    y_model_out.close();
    alm_out.close();
    delta_x_out.close();
    delta_y_out.close();
    topo_b_out.close();
    topo_p_out.close();
    hybrid_out.close();
  }
  predictionErrors(round) = compute_rmspe();
}

RealVectorArray NonDAdaptiveSampling::drawNewX(int this_k, int respFnCount) 
{

  int i,j,temp_length;
  temp_length = gpCvars.size();
  std::set<int> selected_indices;
  RealVectorArray selected_data;

  if(batchSize == 1) {
    int max_index = 0;
    for(i=0; i < temp_length; i++)
      if(emulEvalScores[max_index] < emulEvalScores[i])
        max_index = i;
    selected_data.push_back(gpCvars[max_index]);
  }
  else if(batchStrategy == "naive") {
    //Naive batch selection will grab x highest scored candidates
    // where x=batchSize
    for(i = 0; i < batchSize; i++) {
      int max_index = 0;
      for(j=0; j < temp_length; j++) {
        if(emulEvalScores[max_index] < emulEvalScores[j] &&
           selected_indices.count(j) == 0) {
          max_index = j;
        }
      }
      selected_data.push_back(gpCvars[max_index]);
      selected_indices.insert(max_index);
    }
  }
  else if(batchStrategy == "distance") {
    //Distance penalize point based on what we have already selected

    //This vector will hold the minimum distance from a non-selected point
    // to its nearest selected point
    RealVector min_distances(numEmulEval);

    //First add the highest scoring candidate
    int max_index = 0;
    for(i=0; i < temp_length; i++)
      if(emulEvalScores[max_index] < emulEvalScores[i])
        max_index = i;
    selected_data.push_back(gpCvars[max_index]);
    selected_indices.insert(max_index);

    Real temp_dist = 0;
    for(i = 1; i < batchSize; i++) {
      //Update the minimum distances
      for(j = 0; j < temp_length; j++) {
        temp_dist = 0;
        for(int k = 0; k < gpCvars[j].length(); k++) {
          temp_dist += pow((gpCvars[j][k] - 
            selected_data[selected_data.size()-1][k]),2);
        }
        temp_dist = sqrt(temp_dist);
        //If it is the first iteration i==1, then set the min_distances
        // regardless, otherwise only if the last selected point is closer than
        // all previous selected points
        if(min_distances[j] > temp_dist || i== 1)
          min_distances[j] = temp_dist;
      }

      int max_index;
      Real max_weighted_score;

      //initialize max_weighted_score and max_index to first unused value
      for(j=0; j < temp_length; j++) {
        //Compute distance-penalized scores for each
        Real q_50 = median(min_distances);
        Real rho = 1;
        if(min_distances[j] < q_50)
          rho = 1.5*min_distances[j]-0.5*pow(min_distances[j],3);
        Real temp_weighted_score = emulEvalScores[j] * rho;

        if(selected_indices.count(j) == 0) {
          max_index = j;
          max_weighted_score = temp_weighted_score;
          break;
        }
      }

      j++;
      //No use starting over, we have already looked at the first j entries,
      for( ; j < temp_length; j++) {
        //Compute distance-penalized scores for each
        Real q_50 = median(min_distances);
        Real rho = 1;
        if(min_distances[j] < q_50)
          rho = 1.5*min_distances[j]-0.5*pow(min_distances[j],3);
        Real temp_weighted_score = emulEvalScores[j] * rho;

        if(max_weighted_score < temp_weighted_score &&
           selected_indices.count(j) == 0) {
          max_index = j;
          max_weighted_score = temp_weighted_score;
        }
      }
      selected_data.push_back(gpCvars[max_index]);
      selected_indices.insert(max_index);
    }    
  }
  else if(batchStrategy == "cl") {
////***ATTENTION***
//// We may want to verify that this is doing the correct thing, also TODO:
//// we will want to be able to do const_liar_{max,min,mean}
////***END ATTENTION***
    //Constant Liar strategy

    //Find the mean response value
    const Pecos::SurrogateData& gp_data = gpModel.approximation_data(respFnCount);
    Real mean_value = 0;
    for (i = 0; i < gp_data.points(); i++) {
      mean_value += gp_data.response_function(i);
    } 
    mean_value /= (Real)gp_data.points();

    for(i = 0; i < batchSize; i++) {
      //Rescore the candidates after refitting the gp each time
      emulEvalScores.resize(0);
      if(scoringMetric == "alm")
        calc_score_alm(); // ebeida
      else if(scoringMetric == "distance")
        calc_score_delta_x( ); // Ebeida
      else if(scoringMetric == "gradient")
        calc_score_delta_y( ); // Ebeida
      else if(scoringMetric == "bottleneck")
        calc_score_topo_bottleneck( );
      else if(scoringMetric == "avg_persistence")
        calc_score_topo_avg_persistence(respFnCount);
      else if(scoringMetric == "highest_persistence")
        calc_score_topo_highest_persistence(respFnCount);
      else if(scoringMetric == "alm_topo_hybrid")
        calc_score_topo_alm_hybrid(respFnCount);

      int max_index = 0;
      for(j=0; j < temp_length; j++) {
        if(emulEvalScores[max_index] < emulEvalScores[j] &&
           selected_indices.count(j) == 0) {
          max_index = j;
        }
      }
      selected_data.push_back(gpCvars[max_index]);
      selected_indices.insert(max_index);
      
      Cout << "Updaing surrogate" << std::endl;
      gpModel.continuous_variables(gpCvars[max_index]);
      gpModel.evaluate();
      Response current_response = gpModel.current_response();
      current_response.function_value_view(respFnCount) = mean_value;
      IntResponsePair response_to_add(gpModel.evaluation_id(),current_response);
      Variables point_to_add(gpModel.current_variables());
      gpModel.append_approximation(point_to_add,response_to_add, true);
		#ifdef HAVE_MORSE_SMALE
		for (int ifunc = 0; ifunc < numFunctions; ifunc++)
		{
			update_amsc(ifunc);
		}
		#endif
    }
    gpModel.pop_approximation(false, true/*, batchSize*/);
  }
  else {
#ifdef HAVE_MORSE_SMALE
    //Compute topology of candidates and select only maxima first, and then
    // fall back to distance penalization strategy above
////***ATTENTION***
//// I do this all over the place, but there has to be a better way to obtain
//// the dimensionality of the domain under test
////***END ATTENTION***
    // BMA: See note above
    int dim = 
      gpModel.approximation_data(respFnCount).continuous_variables(0).length();

    double *raw_points = new double[temp_length*(dim+1)];
    for(i = 0; i < temp_length; i++) {
      for(j = 0; j < dim; j++) {
        raw_points[i*(dim+1)+j] = gpCvars[i][j];
      }
      raw_points[i*(dim+1)+dim] = emulEvalScores[i];
    } 

////***ATTENTION***
//// I have hard-coded the number of k-Nearest neighbors to be 25 in any 
//// dimension higher than 2D. Anything higher can be prohibitively expensive 
//// to compute. Thus, the following heuristic only applies in 2D.
////***END ATTENTION***
    //Use this arbitrary heuristic for selecting k: if I have enough points,
    // assume a regular grid-like structure giving (3^d)-1 neighbors, but only 
    // if this is number is less than 40% of the total number of points.
    MS_Complex *score_complex = new MS_Complex(raw_points, dim+1, temp_length, 
                  dim > 2 ? 25 : (int)(std::min(pow(3,dim)-1,0.4*temp_length)));
    delete [] raw_points;

    //Indices here will match indices in complex
    
    //Put the data into a map, where the key is the score and the value is the
    // index.  In terms of coding effort this is probably the laziest way to 
    // sort the maximums. The largest score is at the end of the list, so we
    // can run through the list backwards popping points as we go.
    std::map<double,int> max_values;
    for(i = 0; i < temp_length; i++) {
      if(score_complex->IsMaximum(i)) {
        max_values[score_complex->GetVertex(i)->Value()] = i;
      }
    }
    std::map<double, int>::reverse_iterator mv_iter = max_values.rbegin();
    while(mv_iter != max_values.rend() && selected_data.size() <= batchSize) {
      selected_data.push_back(gpCvars[mv_iter->second]);
      selected_indices.insert(mv_iter->second);
      mv_iter++;
    }

    //Fall back strategy in case we don't have enough maximums
    // Perform a distance metric on the remaining candidates and select
    // the ones farthest from the chosen maxima
    if(selected_data.size() < batchSize) {
      Real temp_dist;
      RealVector min_distances(numEmulEval);
      for(i = 0; i < temp_length; i++) {
        for( j = 0; j < selected_data.size(); j++) {
          temp_dist = 0;
          for(int k = 0; k < gpCvars[i].length(); k++) {
            temp_dist += pow((gpCvars[i][k] - selected_data[j][k]),2);
          }
          temp_dist = sqrt(temp_dist);
          //If it is the first iteration for i, j==1, then set the min_distances
          // regardless, otherwise only if the last selected point is closer 
          // than all previous selected points
          if(min_distances[j] > temp_dist || j == 0)
            min_distances[j] = temp_dist;
        }
      }

      while(selected_data.size() < batchSize) {
        for(j = 0; j < temp_length; j++) {
          temp_dist = 0;
          for(int k = 0; k < gpCvars[j].length(); k++) {
            temp_dist += pow((gpCvars[j][k] - 
              selected_data[selected_data.size()-1][k]),2);
          }
          temp_dist = sqrt(temp_dist);
          //If it is the first iteration i==1, then set the min_distances
          // regardless, otherwise only if the last selected point is closer than
          // all previous selected points
          if(min_distances[j] > temp_dist || i== 1)
            min_distances[j] = temp_dist;
        }

        int max_index;
        Real max_weighted_score;

        //initialize max_weighted_score and max_index to first unused value
        for(j=0; j < temp_length; j++) {
          //Compute distance-penalized scores for each
          Real q_50 = median(min_distances);
          Real rho = 1;
          if(min_distances[j] < q_50)
            rho = 1.5*min_distances[j]-0.5*pow(min_distances[j],3);
          Real temp_weighted_score = emulEvalScores[j] * rho;

          if(selected_indices.count(j) == 0) {
            max_index = j;
            max_weighted_score = temp_weighted_score;
            break;
          }
        }

        j++;
        //No use starting over, we have already looked at the first j entries,
        for( ; j < temp_length; j++) {
          //Compute distance-penalized scores for each
          Real q_50 = median(min_distances);
          Real rho = 1;
          if(min_distances[j] < q_50)
            rho = 1.5*min_distances[j]-0.5*pow(min_distances[j],3);
          Real temp_weighted_score = emulEvalScores[j] * rho;

          if(max_weighted_score < temp_weighted_score &&
             selected_indices.count(j) == 0) {
            max_index = j;
            max_weighted_score = temp_weighted_score;
          }
        }
        selected_data.push_back(gpCvars[max_index]);
        selected_indices.insert(max_index);
      }
    }

    delete score_complex;
#else
  Cout << "Topology-based batch addition is disabled due to the ANN library "
       << " being unavailable, defaulting to naive method" << std::endl;
////***ATTENTION***
//// I copy-and-pasted code from above. A terrible practice, I know, perhaps
//// each of these strategies should be its own function to prevent this?
////***END ATTENTION***
    //Naive batch selection will grab x highest scored candidates
    // where x=batchSize
    for(i = 0; i < batchSize; i++) {
      int max_index = 0;
      for(j=0; j < temp_length; j++) {
        if(emulEvalScores[max_index] < emulEvalScores[j] &&
           selected_indices.count(j) == 0) {
          max_index = j;
        }
      }
      selected_data.push_back(gpCvars[max_index]);
      selected_indices.insert(max_index);
    }
#endif
  }
  return selected_data;
}

	





void NonDAdaptiveSampling::calc_score_topo_avg_persistence(int respFnCount) 
{

#ifdef HAVE_MORSE_SMALE
  emulEvalScores.resize(numEmulEval);

  double *temp_x = NULL;
  if(numEmulEval > 0)
    temp_x = new double[gpCvars[0].length()+1];

  for (int i = 0; i<numEmulEval; i++) {   
    for(int d = 0; d < gpCvars[i].length(); d++)
        temp_x[d] = gpCvars[i][d];
    temp_x[gpCvars[i].length()] = gpMeans[i][respFnCount];
    emulEvalScores(i) = ScoreTOPOP((*AMSC), temp_x);
  }    
  delete [] temp_x;
#else
  Cout << "ANN library not enabled, therefore cannot compute approximate "
       << "Morse-Smale complex or avg_persistence score, setting all scores to " 
       << "zero" << std::endl;
  abort_handler(-1);
#endif
}

void NonDAdaptiveSampling::calc_score_topo_alm_hybrid(int respFnCount) 
{ 
#ifdef HAVE_MORSE_SMALE
  emulEvalScores.resize(numEmulEval);
  double *temp_x = NULL;
  if(numEmulEval > 0)
    temp_x = new double[gpCvars[0].length()+1];

  for (int i = 0; i<numEmulEval; i++) {
    for(int d = 0; d < gpCvars[i].length(); d++)
        temp_x[d] = gpCvars[i][d];
    temp_x[gpCvars[i].length()] = gpMeans[i][respFnCount];

    Real score_val = ScoreTOPOP((*AMSC), temp_x);

	  gpModel.continuous_variables(gpCvars[i]);
    temp_x[gpCvars[i].length()] = gpMeans[i][respFnCount] 
      + sqrt(gpModel.approximation_variances(gpModel.current_variables())[respFnCount]);
    Real score_val_plus_std = ScoreTOPOP((*AMSC), temp_x);
    temp_x[gpCvars[i].length()] = gpMeans[i][respFnCount] 
      - sqrt(gpModel.approximation_variances(gpModel.current_variables())[respFnCount]);
    Real score_val_minus_std = ScoreTOPOP((*AMSC), temp_x);
    emulEvalScores(i) = (score_val + score_val_plus_std + score_val_minus_std) / 3.;
  }    
  delete [] temp_x;
#else
  Cout << "ANN library not enabled, therefore cannot compute approximate "
       << "Morse-Smale complex or hybrid score, setting all scores to " 
       << "zero" << std::endl;
  abort_handler(-1);
#endif
}

void NonDAdaptiveSampling::calc_score_topo_highest_persistence(int respFnCount) 
{

#ifdef HAVE_MORSE_SMALE
  emulEvalScores.resize(numEmulEval);

  //Just in case we don't make it far, at least these will be initialized
  // The scorer will randomly select a point if all scores are equal
  emulEvalScores = 0.0;

  const Pecos::SurrogateData& gp_data = gpModel.approximation_data(respFnCount);
  if(gp_data.points() < 1)
    return;

  int n_train = gp_data.points();
  int n_cand = numEmulEval;
////***ATTENTION***
//// I do this all over the place, but there has to be a better way to obtain
//// the dimensionality of the domain under test
////***END ATTENTION***
  int dim = gp_data.continuous_variables(0).length();

  double *train_x = new double[n_train*dim];
  double *train_y = new double[n_train];
  for (int i = 0; i < n_train; i++) {
    for(int j = 0; j < dim; j++)
      train_x[i*dim+j] = gp_data.continuous_variables(i)[j];
    train_y[i] = gp_data.response_function(i);
  } 

  double *cand_x = new double[n_cand*dim];
  double *cand_y = new double[n_cand];
  for (int i = 0; i < n_cand; i++) {
    for(int j = 0; j < dim; j++)
      cand_x[i*dim+j] = gpCvars[i][j];
    cand_y[i] = gpMeans[i][respFnCount];
  }
  std::vector<int> ordered_indices = ScoreTOPOHP(dim, numKneighbors, 
    train_x, train_y, n_train, cand_x, cand_y, n_cand);

  if(ordered_indices.size() != numEmulEval)
    Cout << "\nWarning: Mismatch in size of ranked Morse-Smale points" 
	 << std::endl;

  //The score will just be the inverse of the ranked order
  // e.g. the first ranked point will be given a score of numEmulEval
  // and the last ranked point will be given a score of 1
  int current_score = numEmulEval;

  for (int i = 0; i < ordered_indices.size(); i++) {
    int index = ordered_indices[i];
    emulEvalScores(index) = current_score--;
  }    
  delete [] train_x;
  delete [] train_y;
  delete [] cand_x;
  delete [] cand_y;
#else
  Cout << "ANN library not enabled, therefore cannot compute approximate "
       << "Morse-Smale complex or highest_persistence score, setting all scores" 
       << " to zero" << std::endl;
  abort_handler(-1);
#endif
}

Real NonDAdaptiveSampling::calc_score_alm(int respFnCount,
  RealVector &test_point) { 
  gpModel.continuous_variables(test_point);
  return gpModel.approximation_variances(
          gpModel.current_variables())[respFnCount];
}

Real NonDAdaptiveSampling::calc_score_delta_y(int respFnCount,
  RealVector &test_point) 
{ 

  const Pecos::SurrogateData& gp_data = gpModel.approximation_data(respFnCount);
  double min_sq_dist;
  int min_index;
  bool first = true;
  for (int j = 0; j < gp_data.points(); j++) {
    double sq_dist = 0;
    for(int d = 0; d < gp_data.continuous_variables(j).length(); d++)
      sq_dist += pow(test_point[d]-gp_data.continuous_variables(j)[d],2);
    if(first || sq_dist < min_sq_dist) {
      min_sq_dist = sq_dist;
      min_index = j;
      first = false;
    }
  }

  Real true_value = gp_data.response_function(min_index);

  Model& surrogate_model = gpModel.surrogate_model();
  surrogate_model.continuous_variables(test_point);
  surrogate_model.evaluate();
  Real guessed_value = 
    surrogate_model.current_response().function_value(respFnCount);

  return fabs(guessed_value - true_value);
}

Real NonDAdaptiveSampling::calc_score_delta_x(int respFnCount,
  RealVector &test_point) 
{ 
  const Pecos::SurrogateData& gp_data = gpModel.approximation_data(respFnCount);
  double min_sq_dist;
  int min_index;
  bool first = true;
  for (int j = 0; j < gp_data.points(); j++) {
    double sq_dist = 0;
    for(int d = 0; d < gp_data.continuous_variables(j).length(); d++)
      sq_dist += pow(test_point[d]-gp_data.continuous_variables(j)[d],2);
    if(first || sq_dist < min_sq_dist) {
      min_sq_dist = sq_dist;
      min_index = j;
      first = false;
    }
  }
  return sqrt(min_sq_dist);
}

Real NonDAdaptiveSampling::calc_score_topo_bottleneck(int respFnCount,
  RealVector &test_point)
{ 
#if defined(HAVE_MORSE_SMALE) && defined(HAVE_DIONYSUS)
  double *temp_x = new double[test_point.length()+1];

  for(int d = 0; d < test_point.length(); d++)
      temp_x[d] = test_point[d];

  Model& surrogate_model = gpModel.surrogate_model();
  surrogate_model.continuous_variables(test_point);
  surrogate_model.evaluate();
  temp_x[test_point.length()] = 
    surrogate_model.current_response().function_value(respFnCount);
  Real ret_value = ScoreTOPOB((*AMSC), temp_x);
  delete [] temp_x;
  return ret_value;
#else
  #ifdef HAVE_MORSE_SMALE
  Cout << "Dionysus library not enabled, therefore cannot compute the "
       << "bottleneck distance score, returning NaN" 
       << std::endl;
  #else
  Cout << "ANN library not enabled, therefore cannot compute approximate "
       << "Morse-Smale complex or bottleneck distance score, returning NaN" 
       << std::endl;
  #endif
  abort_handler(-1);
  return -DBL_MAX;
#endif
}

Real NonDAdaptiveSampling::calc_score_topo_avg_persistence(int respFnCount, 
  RealVector &test_point) 
{ 
#ifdef HAVE_MORSE_SMALE
  double *temp_x = new double[test_point.length()+1];

  for(int d = 0; d < test_point.length(); d++)
      temp_x[d] = test_point[d];

  Model& surrogate_model = gpModel.surrogate_model();
  surrogate_model.continuous_variables(test_point);
  surrogate_model.evaluate();
  temp_x[test_point.length()] = 
    surrogate_model.current_response().function_value(respFnCount);
  Real ret_value = ScoreTOPOP((*AMSC), temp_x);
  delete [] temp_x;
  return ret_value;
#else
  Cout << "ANN library not enabled, therefore cannot compute approximate "
       << "Morse-Smale complex or avg_persistence score, returning NaN" 
       << std::endl;
  abort_handler(-1);
  return -DBL_MAX;
#endif
}



Real NonDAdaptiveSampling::calc_score_topo_alm_hybrid(int respFnCount,
  RealVector &test_point) 
{ 
#ifdef HAVE_MORSE_SMALE
  double *temp_x = new double[test_point.length()+1];

  for(int d = 0; d < test_point.length(); d++)
      temp_x[d] = test_point[d];

  Model& surrogate_model = gpModel.surrogate_model();
  surrogate_model.continuous_variables(test_point);
  surrogate_model.evaluate();
  temp_x[test_point.length()] 
    = surrogate_model.current_response().function_value(respFnCount);
  Real ret_value = ScoreTOPOP((*AMSC), temp_x);
  temp_x[test_point.length()] 
    = surrogate_model.current_response().function_value(respFnCount) 
    + gpModel.approximation_variances(gpModel.current_variables())[respFnCount];
  ret_value += ScoreTOPOP((*AMSC), temp_x);
  temp_x[test_point.length()] 
    = surrogate_model.current_response().function_value(respFnCount) 
    - gpModel.approximation_variances(gpModel.current_variables())[respFnCount];
  ret_value += ScoreTOPOP((*AMSC), temp_x);
  delete [] temp_x;
  return ret_value/3.;
#else
  Cout << "ANN library not enabled, therefore cannot compute approximate "
       << "Morse-Smale complex or hybrid score, returning NaN" << std::endl;
  abort_handler(-1);
  return -DBL_MAX;
#endif
}






////***ATTENTION***
//// Probably want to modify this, but calling this function assumes that the
//// validation sets have been evaluated. Validation sets are evaluated inside
//// the function: output_round_data (so it cannot be deleted yet)
////***END ATTENTION***
Real NonDAdaptiveSampling::compute_rmspe() 
{
  Real rms_prediction_error = 0;
  for(int i = 0; i < yTrue.length(); i++) {
    rms_prediction_error += pow(yTrue(i)-yModel(i),2);
  }
  rms_prediction_error /= ((double)yTrue.length());
  rms_prediction_error = sqrt(rms_prediction_error);
  return rms_prediction_error;
}

	void NonDAdaptiveSampling::parse_options()
	{
		#pragma region Parse Options:
		const StringArray& db_opts = probDescDB.get_sa("method.coliny.misc_options");
		StringArray::const_iterator db_it = db_opts.begin();
		StringArray::const_iterator db_end = db_opts.end();
		String::const_iterator delim;

		for ( ; db_it != db_end; ++db_it)
			if ( (delim = find(db_it->begin(), db_it->end(), '=')) != db_it->end()) 
			{
				String opt(*db_it, 0, distance(db_it->begin(), delim));
				String val(*db_it, distance(db_it->begin(), delim+1), distance(delim, db_it->end()));

				if (opt == "candidate_size")
					numEmulEval = boost::lexical_cast<int,const char*>(val.c_str());
				else if (opt == "batch_size") 
				{
					batchSize = boost::lexical_cast<int,const char*>(val.c_str());
					Cout << "BATCH SIZE: " << batchSize << std::endl;
				}
				else if (opt == "rounds")
					numRounds = boost::lexical_cast<int,const char*>(val.c_str());
				else if (opt == "approx_type") 
				{
					approx_type = val;
				}
				else if (opt == "batch_strategy") 
				{
					batchStrategy = val;
				
					if(!(batchStrategy == "naive" || 
						 batchStrategy == "distance" ||
						 batchStrategy == "topology" || 
						 batchStrategy == "cl")) 
					{
						Cerr << "ERROR (NonDAdaptiveSampling): Bad Value for misc_option " 
							 << opt << ": " << val << std::endl;
						abort_handler(-1);
					}
				}
				else if (opt == "sample_design") 
				{
				  if (val == "sampling_lhs")
				    sampleDesign = RANDOM_SAMPLING;
				  else if (val == "fsu_cvt")
				    sampleDesign = FSU_CVT;
				  else if (val == "fsu_halton")
				    sampleDesign = FSU_HALTON;
				  else if (val == "fsu_hammersley")
				    sampleDesign = FSU_HAMMERSLEY;
				}
				else if (opt == "score_type") 
				{
					scoringMetric = val;
					if(!(scoringMetric == "alm" || 
						 scoringMetric == "distance" ||
						 scoringMetric == "gradient" || 
						 scoringMetric == "bottleneck" ||
						 scoringMetric == "avg_persistence" || 
						 scoringMetric == "highest_persistence" || 
						 scoringMetric == "alm_topo_hybrid")) 
					{
						Cerr << "ERROR (NonDAdaptiveSampling): Bad Value for misc_option " 
							 << opt << ": " << val << std::endl;
						abort_handler(-1);
					}
				}
				else if(opt == "validation_data") 
				{
					outputValidationData =boost::lexical_cast<int,const char*>(val.c_str());
				}
				else if(opt == "knn") 
				{
					numKneighbors = boost::lexical_cast<int,const char*>(val.c_str());
				}
				else 
				{
					Cerr << "ERROR (NonDAdaptiveSampling): Unknown misc_option: " 
						 << opt << std::endl;
					abort_handler(-1);
				}

				if (outputLevel > NORMAL_OUTPUT)
					Cout << "INFO (NonDAdaptiveSampling): User parameter '" << opt << "': " << val << std::endl;
			}
			else 
			{
				Cerr << "ERROR (NonDAdaptiveSampling): Invalid misc_options format." << std::endl;
				abort_handler(-1);
			}

  
		if(approx_type != "global_kriging" && scoringMetric == "alm") 
		{
			Cerr << "ERROR (NonDAdaptiveSampling): Cannot utilize alm scoring with " << approx_type << std::endl;
			abort_handler(-1);  
		}
		if(batchSize > numEmulEval) 
		{
			Cerr << "ERROR (NonDAdaptiveSampling): Cannot use " 
				 << batchSize << " as the batch size with only " << numEmulEval 
				 << " candidates" << std::endl;
			abort_handler(-1);  
		}

		#ifdef HAVE_MORSE_SMALE
		if(numKneighbors > numSamples) 
		{
			Cerr << "ERROR (NonDAdaptiveSampling): Cannot use " 
				 << numKneighbors << " as the number of neighbors in the Morse-Smale " 
				 << " complex when only " << numSamples << " points exist" << std::endl;
			abort_handler(-1);  
		}
		#endif

		#ifndef HAVE_MORSE_SMALE
		if(scoringMetric == "bottleneck" ||
		   scoringMetric == "avg_persistence" || 
		   scoringMetric == "alm_topo_hybrid") 
		{
			Cerr << "ERROR (NonDAdaptiveSampling): Cannot use " 
				 << scoringMetric << " as the scoring metric because ANN is disabled" 
				 << std::endl;
			abort_handler(-1);  
		}
		#endif

		#ifndef HAVE_DIONYSUS
		if(scoringMetric == "bottleneck") 
		{
			Cerr << "ERROR (NonDAdaptiveSampling): Cannot use " 
				 << scoringMetric << " as the scoring metric because Dionysus is " 
				 << "disabled" << std::endl;
			abort_handler(-1);  
		}
		#endif

		#pragma endregion
	}

void NonDAdaptiveSampling::compare_complices(int dim, std::ostream& output) 
{

////***ATTENTION***
//// The majority of this function is commented out because constructing 
//// Morse-Smale Complices can be time-consuming especially in high-dimension
//// Also, I am not sure comparing topologies has proven fruitful to this point
//// so it is a waste of time, instead we will just compute the RMSE between
//// the surrogate and truth model and write zeros for the rest of the values
////***END ATTENTION***

//#ifdef HAVE_MORSE_SMALE
//  double *true_points = new double[validationSetSize*(dim+1)];
//  double *model_points = new double[validationSetSize*(dim+1)];
//  for(int i = 0; i < validationSetSize; i++) {
//    for(int k = 0; k < dim; k++) {
//      true_points[i*(dim+1)+k] = validationSet[i][k];
//      model_points[i*(dim+1)+k] = validationSet[i][k];
//    }
//    true_points[i*(dim+1)+dim] = yTrue[i];
//    model_points[i*(dim+1)+dim] = yModel[i];
//  } 

////***ATTENTION***
//// I have hard-coded the number of k-Nearest neighbors to be 25 in any 
//// dimension higher than 2D. Anything higher can be prohibitively expensive 
//// to compute.
////***END ATTENTION***
//  MS_Complex *trueComplex = new MS_Complex(true_points, dim+1, 
//    validationSetSize, 
//    dim > 2 ? 25 : (int)(std::min(pow(3,dim)-1,0.4*validationSetSize)));

////***ATTENTION***
//// I have hard-coded the number of k-Nearest neighbors to be 25 in any 
//// dimension higher than 2D. Anything higher can be prohibitively expensive 
//// to compute.
////***END ATTENTION***
//  MS_Complex *modelComplex = new MS_Complex(model_points, dim+1, 
//    validationSetSize, 
//    dim > 2 ? 25 : (int)(std::min(pow(3,dim)-1,0.4*validationSetSize)));

//  delete [] true_points;
//  delete [] model_points;

//  int count_model_min = 0;
//  int count_model_max = 0;
//  int count_model_saddle = 0;

//  int count_true_min = 0;
//  int count_true_max = 0;
//  int count_true_saddle = 0;

//  std::map<int,double> true_mins;
//  std::map<int,double> true_maxs;
//  std::map<int,double> true_saddles;

//  std::map<int,double> model_mins;
//  std::map<int,double> model_maxs;
//  std::map<int,double> model_saddles;

//  std::map<int,double> true_min_values;
//  std::map<int,double> model_min_values;

//  for(int i = 0; i < trueComplex->numV; i++) {
//    Vertex *true_vertex = trueComplex->GetVertex(i);
//    Vertex *model_vertex = modelComplex->GetVertex(i);

//    switch(true_vertex->classification)
//    {
//      case 0:
//        count_true_min++;
//        true_mins[i] = true_vertex->persistence;
//        true_min_values[i] = true_vertex->Value();
//      break;
//      case 1:
//        count_true_max++;
//        true_maxs[i] = true_vertex->persistence;
//      break;
//      case 2:
//        count_true_saddle++;
//        true_saddles[i] = true_vertex->persistence;
//      break;
//    }

//    switch(model_vertex->classification)
//    {
//      case 0:
//        count_model_min++;
//        model_mins[i] = model_vertex->persistence;
//        model_min_values[i] = model_vertex->Value();
//      break;
//      case 1:
//        count_model_max++;
//        model_maxs[i] = model_vertex->persistence;
//      break;
//      case 2:
//        count_model_saddle++;
//        model_saddles[i] = model_vertex->persistence;
//      break;
//    }

//  }

//  double bottleneck_distance;
//  #ifdef HAVE_DIONYSUS
//    bottleneck_distance = trueComplex->CompareBottleneck((*modelComplex));
//  #else
//    bottleneck_distance = 0;
//  #endif
//  delete trueComplex;
//  delete modelComplex;
//  std::map<int,double>::iterator iter;
//  Cout << "True Minimum indices and Persistences" << std::endl;
//  for(iter = true_mins.begin(); iter != true_mins.end(); iter++)
//    Cout << (*iter).first << " => " << iter->second << std::endl;
//  Cout << "====================================" << std::endl;
//  Cout << "True Maximum indices and Persistences" << std::endl;
//  for(iter = true_maxs.begin(); iter != true_maxs.end(); iter++)
//    Cout << (*iter).first << " => " << iter->second << std::endl;
//  Cout << "====================================" << std::endl;
//  Cout << "True Saddle indices and Persistences" << std::endl;
//  for(iter = true_saddles.begin(); iter != true_saddles.end(); iter++)
//    Cout << (*iter).first << " => " << iter->second << std::endl;
//  Cout << "====================================" << std::endl;
//  Cout << "====================================" << std::endl;
//  Cout << "Model Minimum indices and Persistences" << std::endl;
//  for(iter = model_mins.begin(); iter != model_mins.end(); iter++)
//    Cout << (*iter).first << " => " << iter->second << std::endl;
//  Cout << "====================================" << std::endl;
//  Cout << "Model Maximum indices and Persistences" << std::endl;
//  for(iter = model_maxs.begin(); iter != model_maxs.end(); iter++)
//    Cout << (*iter).first << " => " << iter->second << std::endl;
//  Cout << "====================================" << std::endl;
//  Cout << "Model Saddle indices and Persistences" << std::endl;
//  for(iter = model_saddles.begin(); iter != model_saddles.end(); iter++)
//    Cout << (*iter).first << " => " << iter->second << std::endl;
//  Cout << "====================================" << std::endl;
//  Cout << "====================================" << std::endl;

//  output << count_true_min;
//  output << "\t" << count_true_max;
//  output << "\t" << count_true_saddle;
//  output << "\t" << count_model_min;
//  output << "\t" << count_model_max;
//  output << "\t" << count_model_saddle;
//  output << "\t" << bottleneck_distance;
//#else
  output << 0 << "\t" << 0 << "\t" << 0 << "\t" << 0 << "\t" << 0 << "\t" << 0 
         << "\t" << 0;
//#endif
  output << "\t" << compute_rmspe() << std::endl;
}

void NonDAdaptiveSampling::output_for_optimization(int dim) 
{
#ifdef HAVE_MORSE_SMALE
  std::ofstream output("opt_topo.in");

  output << "strategy," << std::endl
         << "\ttabular_graphics_data" << std::endl
         << "\tmulti_start" << std::endl
         << "\t\tmethod_pointer = \'Downhill\'" << std::endl
         << "\t\t\tstarting_points" << std::endl;

  double *model_points = new double[validationSetSize*(dim+1)];
  for(int i = 0; i < validationSetSize; i++) {
    for(int k = 0; k < dim; k++) {
      model_points[i*(dim+1)+k] = validationSet[i][k];
    }
    model_points[i*(dim+1)+dim] = yModel[i];
  } 
////***ATTENTION***
//// I have hard-coded the number of k-Nearest neighbors to be 25 in any 
//// dimension higher than 2D. Anything higher can be prohibitively expensive 
//// to compute.
////***END ATTENTION***
  MS_Complex *modelComplex = new MS_Complex(model_points, dim+1, 
    validationSetSize, dim > 2 ? 25 :
    (int)(std::min(pow(3,dim)-1,0.4*validationSetSize)));
  delete [] model_points;

  std::map<int,double> model_saddles;
  std::map<int,double> model_mins;
  int count_model_min = 0;
  int count_model_saddle = 0;

  for(int i = 0; i < modelComplex->numV; i++) {
    Vertex *model_vertex = modelComplex->GetVertex(i);

    switch(model_vertex->classification)
    {
      case 0:
        count_model_min++;
        model_mins[i] = model_vertex->Value();
      break;
      case 2:
        count_model_saddle++;
        model_saddles[i] = model_vertex->persistence;
      break;
    }

  }
  delete modelComplex;

  std::map<int,double>::iterator iter;
  for(iter = model_mins.begin(); iter != model_mins.end(); iter++){
    for(int i = 0; i < dim; i++) {
      output << validationSet[(*iter).first][i] << " ";
    }
    output << "# " << iter->second << std::endl;
  }

  output  << std::endl
          << "method,"<< std::endl
      	  << "\tid_method = \'Downhill\'" << std::endl
	        << "\tconmin_frcg" << std::endl
	        << "\tconvergence_tolerance 1.0e-3" << std::endl
          << std::endl
          << "variables," << std::endl
          << "\tcontinuous_design = " << dim << std::endl
////***ATTENTION***
//// Bounds should not be hard-coded
////***END ATTENTION***
// BMA: SHould be able to do continuous_lower/upper_)bounds from Model
	        << "\t\tlower_bounds\t" << dim << "*-2.0" << std::endl
          << "\t\tupper_bounds\t" << dim << "*2.0" << std::endl
          << std::endl
          << "interface," << std::endl
          << "\tdirect" << std::endl
          << "\t\tanalysis_driver = \'" 
////***ATTENTION***
//// Is there a way to extract the test function used from the database? 
//// Something like, but not exactly the following because it does not work:
//// probDescDB.interface_list().front().interface_id()
//// In lieu of this, I am hard-coding the function name and other things like
//// the domain boundaries, in a general framework this is inadequate
////***END ATTENTION***
    // BMA: Should be able to do
    // iteratedModel.derived_interface().analysis_drivers()[0]\
    // to get the first driver in the list...
          << "herbie\'" << std:: endl
          << std::endl
          << "responses," << std::endl
	        << "\tobjective_functions = 1" << std::endl
	        << "\tanalytic_gradients" << std::endl
	        << "\tno_hessians";

  output.close();
#else
  Cout << "ANN library disabled, unable to build approximate Morse-Smale "
       << "complex" << std::endl;
#endif
}



void NonDAdaptiveSampling::
construct_fsu_sampler(Iterator& u_space_sampler, Model& u_model, 
    int num_samples, int seed, unsigned short sample_type) {
  // sanity checks
  if (num_samples <= 0) {
    Cerr << "Error: bad samples specification (" << num_samples << ") in "
	 << "NonD::construct_fsu_sampler()." << std::endl;
    abort_handler(-1);
  }

  u_space_sampler.assign_rep(new FSUDesignCompExp(u_model, num_samples, seed, 
						  sample_type), false);
}

// Mohamed and Laura
void NonDAdaptiveSampling::print_results(std::ostream& s)
{
  if (statsFlag) {
    s << "\nStatistics based on the adaptive sampling calculations:\n";
    print_level_mappings(s);
  }
}


} // namespace Dakota
