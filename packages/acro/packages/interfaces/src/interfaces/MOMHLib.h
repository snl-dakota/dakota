/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file MOMHLib.h
 *
 * Defines the interfaces::MOMHLib class, which is a wrapper for MOMHLib++
 * multi-objective solvers
 */

#ifndef interfaces_MOMHLib_h
#define interfaces_MOMHLib_h

#include <acro_config.h>
#ifdef ACRO_USING_MOMHLIB
#include <colin/Solver.h>
#include <colin/CommonOptions.h>
#include <utilib/Normal.h>
#include <utilib/Uniform.h>

#include <problem.h>
#include <mosa.h>
#include <psa.h>
#include <smosa.h>
#include <mogls.h>
#include <pma.h>
#include <immogls.h>
#include <momsls.h>
#include <spea.h>
#include <nsga.h>
#include <nsgaii.h>
#include <nsgaiic.h>
#include <momhsolution.h>



namespace interfaces
{

/// Problem class used to communicate with the MOMHLib solver
template <class ProblemT>
class MOMHLibProblem : public TProblem
{
public:

   //! Constructor
   MOMHLibProblem() {}

   //! Destructor
   virtual ~MOMHLibProblem() { }

   ///
   bool Load(char*)
   { return false; }

   ///
   virtual void reset()
   {
      NumberOfObjectives = problem->num_objectives;
      Objectives.resize(NumberOfObjectives);
      std::vector<colin::optimizationSense> sense = problem->sense;
      for (size_t i = 0; i < NumberOfObjectives; i++)
      {
         Objectives[0].bActive = true;
         if ( sense[i] == colin::minimization )
         {
            Objectives[0].ObjectiveType = _Min;
         }
         else
         {
            Objectives[0].ObjectiveType = _Max;
         }
      }
   }

   /// The COLIN problem
   colin::Problem<ProblemT> problem;

};


/** COLIN wrapper class for MOMHLib multi-objective optimizers.
 *
 *  NOTE: this is a template class to enable the application of this wrapper
 *  for other COLIN-related applications.
*/
template <class MProblemT, class MSolutionT, class ProblemT>
class MOMHLib 
   : virtual public colin::Solver<ProblemT>, 
     public colin::CommonOptions
{
public:

   /// Constructor
   MOMHLib()
        : colin::Solver<ProblemT>(), CommonOptions(this->properties)
        { 
        this->reset_signal.connect( boost::bind( &MOMHLib::reset_MOMHLib, this ) );
        solver_type="unknown";
        }

   ///
   void initialize(std::string name = "NSGAII")
   {

        solver_type = name;

        T_init=-1.0;
        if ((solver_type == "PSA") || (solver_type == "MOSA") || (solver_type == "SMOSA")) {
           this->properties.declare
              ( "T-init", 
                "Initial temperature.",
                utilib::Privileged_Property(T_init) );
           //this->option.add("T-init",T_init, "Initial temperature.");
           //this->option.categorize("T-init","MOMHLib");
        }

        T_fini=-1.0;
        if ((solver_type == "PSA") || (solver_type == "MOSA") || (solver_type == "SMOSA")) {
           this->properties.declare
              ( "T-fini", 
                "Final temperature.",
                utilib::Privileged_Property(T_fini) );
           //this->option.add("T-fini",T_fini, "Final temperature.");
           //this->option.categorize("T-fini","MOMHLib");
        }

        T_decrease=-1.0;
        if ((solver_type == "MOSA") || (solver_type == "SMOSA") || (solver_type == "PSA")) {
           this->properties.declare
              ( "T-decrease", 
                "Temperature rate of decrease.",
                utilib::Privileged_Property(T_decrease) );
           //this->option.add("T-decrease",T_decrease, "Temperature rate of decrease.");
           //this->option.categorize("T-decrease","MOMHLib");
        }
   
        moves_per_level=-1;
        if ((solver_type == "PSA") || (solver_type == "MOSA") || (solver_type == "SMOSA")) {
           this->properties.declare
              ( "moves-per-level", 
                "Moves per level in the SA method.",
                utilib::Privileged_Property(moves_per_level) );
           //this->option.add("moves-per-level",moves_per_level,"Moves per level in the SA method.");
           //this->option.categorize("moves-per-level","MOMHLib");
        }

        weight_coef=-1.0;
        if (solver_type == "PSA") {
           this->properties.declare
              ( "weight-coef", 
                "Weight coefficient.",
                utilib::Privileged_Property(weight_coef) );
           //this->option.add("weight-coef",weight_coef, "Weight coefficient.");
           //this->option.categorize("weight-coef","MOMHLib");
        }

        generating_pop_size=-1;
        if ((solver_type == "PSA") || (solver_type == "MOSA")) {
           this->properties.declare
              ( "generating-pop-size", 
                "Subpopulation size used to generate new points.",
                utilib::Privileged_Property(generating_pop_size) );
           //this->option.add("generating-pop-size",generating_pop_size,
           //     "Subpopulation size used to generate new points.");
           //this->option.categorize("generating-pop-size","MOMHLib");
        }

        tmp_pop_size=-1;
        if ((solver_type == "MOGLS") || (solver_type == "PMA")) {
           this->properties.declare
              ( "tmp-pop-size", 
                "Temporary population size.",
                utilib::Privileged_Property(tmp_pop_size) );
           //this->option.add("tmp-pop-size",tmp_pop_size, "Temporary population size.");
           //this->option.categorize("tmp-pop-size","MOMHLib");
        }

        pop_size=-1;
        if ((solver_type == "MOGLS") || (solver_type == "PMA") || (solver_type == "IMMOGLS") ||
            (solver_type == "SPEA") || (solver_type == "NSGAII") || (solver_type == "NSGAIIC") ||
            (solver_type == "NSGA")) {
           this->properties.declare
              ( "pop-size", 
                "Population size",
                utilib::Privileged_Property(pop_size) );
           //this->option.add("pop-size",pop_size, "Population size");
           //this->option.categorize("pop-size","MOMHLib");
        }

        ftype_str="linear";
        if ((solver_type == "MOGLS") || (solver_type == "IMMOGLS") || (solver_type == "PMA") ||
            (solver_type == "MOMSLS")) {
           this->properties.declare
              ( "ftype", 
                "The function type used to combine the objectives?",
                utilib::Privileged_Property(ftype_str) );
           //this->option.add("ftype",ftype_str,"The function type used to combine the objectives?");
           //this->option.categorize("ftype","MOMHLib");
        }

        elite_size=-1;
        if (solver_type == "IMMOGLS") {
           this->properties.declare
              ( "elite-size", 
                "Number of elite members to keep in the next population",
                utilib::Privileged_Property(elite_size) );
           //this->option.add("elite-size",elite_size,
           //     "Number of elite members to keep in the next population");
           //this->option.categorize("elite-size","MOMHLib");
        }

        mutation_prob=-1.0;
        if ((solver_type == "SPEA") || (solver_type == "NSGAII") || (solver_type == "NSGAIIC") || (solver_type == "NSGA")) {
           this->properties.declare
              ( "mutation-prob", 
                "Mutation probability.",
                utilib::Privileged_Property(mutation_prob) );
           //this->option.add("mutation-prob",mutation_prob, "Mutation probability.");
           //this->option.categorize("mutation-prob","MOMHLib");
        }

        nondominated_pop_size=-1;
        if (solver_type == "SPEA") {
           this->properties.declare
              ( "nondominated-pop-size", 
                "Population size of nondominated points that is used.",
                utilib::Privileged_Property(nondominated_pop_size) );
           //this->option.add("nondominated-pop-size",nondominated_pop_size,
           //     "Population size of nondominated points that is used.");
           //this->option.categorize("nondominated-pop-size","MOMHLib");
        }

        cluster_size=-1;
        if (solver_type == "SPEA") {
           this->properties.declare
              ( "cluster-size", 
                "Cluster size.",
                utilib::Privileged_Property(cluster_size) );
           //this->option.add("cluster-size",cluster_size, "Cluster size.");
           //this->option.categorize("cluster-size","MOMHLib");
        }

        scalarize=false;
        if ((solver_type == "SPEA") || (solver_type == "NSGAII") || (solver_type == "NSGAIIC") || (solver_type == "NSGA")) {
           this->properties.declare
              ( "scalarize", 
                "If true, then scalarize the search problem.",
                utilib::Privileged_Property(scalarize) );
           //this->option.add("scalarize",scalarize,
           //                 "If true, then scalarize the search problem.");
           //this->option.categorize("scalarize","MOMHLib");
        }

        geom_factor=-1.0;
        if (solver_type == "NSGAIIC") {
           this->properties.declare
              ( "geom-factor", 
                "Geometric factor.",
                utilib::Privileged_Property(geom_factor) );
           //this->option.add("geom-factor",geom_factor,
           //                 "Geometric factor.");
           //this->option.categorize("geom-factor","MOMHLib");
        }

        nbhd_dist=-1.0;
        if (solver_type == "NSGA") {
           this->properties.declare
              ( "nbhd-dist", 
                "Neighborhood distance.",
                utilib::Privileged_Property(nbhd_dist) );
           //this->option.add("nbhd-dist",nbhd_dist, "Neighborhood distance.");
           //this->option.categorize("nbhd-dist","MOMHLib");
        }

        //this->option.merge_options(momhlib_problem.options);
   }

   /// Destructor
   virtual ~MOMHLib()
        {}

   /// Setup MOMLib for optimization.  Call immediately before \ref{minimize}
   void reset_MOMHLib()
        {
        if (this->problem.empty()) return;

        if (ftype_str == "linear") {
           ftype = _Linear;
        }
        if (ftype_str == "Chebycheff") {
           ftype = _Chebycheff;
        }
           momhlib_problem.problem = this->problem;
           momhlib_problem.reset();
        }

   /// Perform minimization
   void optimize()
        {
        //
        // Create the MOMHLib solver
        //
        TMOMethod<MSolutionT>* momlib_solver = create_solver(solver_type);
        //
        // TODO - initialize initial population
        //
        //
        // Perform optimization
        //
        momlib_solver->Run();
        this->solver_status.termination_info = "Max-Iters";
        //
        // Save results to a file ... for now.
        // 
        momlib_solver->pNondominatedSet->Save("MOMHLib_results");
        //
        // Delete solver
        //
        delete momlib_solver;
        }

   ///
   static MProblemT momhlib_problem;

protected:

   ///
   std::string solver_type;

   ///
   std::string define_solver_type() const
            {
            std::string tmp = "momhlib:";
            tmp += solver_type;
            return tmp;
            }

   ///
   int pop_size;

   ///
   int elite_size;

   ///
   int tmp_pop_size;

   ///
   double mutation_prob;

   ///
   double nbhd_dist;

   ///
   bool scalarize;

   ///
   double geom_factor;

   ///
   int nondominated_pop_size;

   ///
   int cluster_size;

   ///
   double T_init;

   ///
   double T_fini;

   ///
   double T_decrease;

   ///
   int moves_per_level;

   ///
   int generating_pop_size;

   ///
   double weight_coef;

   ///
   TScalarizingFunctionType ftype;

   ///
   std::string ftype_str;

   ///
   TMOMethod<MSolutionT>* create_solver(const std::string s_type)
   {
      if (s_type == "PSA") {
         TPSA<MSolutionT>* tmp = new TPSA <MSolutionT>();
         tmp->SetParameters(T_init, T_fini, T_decrease, weight_coef, moves_per_level, generating_pop_size);
         return tmp;
         }

      if (s_type == "MOSA") {
         TMOSA<MSolutionT>* tmp = new TMOSA <MSolutionT>();
         tmp->SetParameters(T_init, T_fini, T_decrease, moves_per_level, generating_pop_size);
         return tmp;
         }

      if (s_type == "SMOSA") {
         TSMOSA<MSolutionT>* tmp =new TSMOSA <MSolutionT>();
         tmp->SetParameters(T_init, T_fini, T_decrease, moves_per_level);
         return tmp;
         }

      if (s_type == "MOGLS") {
         TMOGLS<MSolutionT>* tmp = new TMOGLS <MSolutionT>();
         tmp->SetParameters(tmp_pop_size, pop_size, this->max_iters, ftype);
         }

      if (s_type == "IMMOGLS") {
         TIMMOGLS<MSolutionT>* tmp = new TIMMOGLS <MSolutionT>();
         tmp->SetParameters(pop_size, this->max_iters, elite_size, ftype);
         return tmp;
         }

      if (s_type == "PMA") {
         TPMA<MSolutionT>* tmp = new TPMA <MSolutionT>();
         tmp->SetParameters(tmp_pop_size, pop_size, this->max_iters, ftype);
         return tmp;
         }

      if (s_type == "MOMSLS") {
         TMOMSLS<MSolutionT>* tmp = new TMOMSLS <MSolutionT>();
         tmp->SetParameters(this->max_iters, ftype);
         return tmp;
         }

      if (s_type == "SPEA") {
         TSPEA<MSolutionT>* tmp = new TSPEA <MSolutionT>();
         tmp->SetParameters(pop_size, this->max_iters, mutation_prob, nondominated_pop_size, cluster_size, scalarize);
         return tmp;
         }

      if (s_type == "NSGAII") {
         TNSGAII<MSolutionT>* tmp = new TNSGAII <MSolutionT>();
         tmp->SetParameters(pop_size, this->max_iters, mutation_prob, scalarize);
         return tmp;
         }

      if (s_type == "NSGAIIC") {
         TNSGAIIC<MSolutionT>* tmp = new TNSGAIIC <MSolutionT>();
         tmp->SetParameters(pop_size, this->max_iters, mutation_prob, geom_factor, scalarize);
         return tmp;
         }

      if (s_type == "NSGA") {
         TNSGA<MSolutionT>* tmp = new TNSGA <MSolutionT>();
         tmp->SetParameters(pop_size, this->max_iters, mutation_prob, nbhd_dist, scalarize);
         return tmp;
         }

      return 0;
   }
};

template <class MProblemT, class MSolutionT, class ProblemT>
MProblemT MOMHLib<MProblemT,MSolutionT,ProblemT>::momhlib_problem;

} // namespace interfaces
#endif

#endif


