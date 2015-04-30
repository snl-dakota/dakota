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

//
// NSGA2.cpp
//

#include <acro_config.h>
#ifdef ACRO_USING_NSGA2

#include <colin/Solver.h>
#include <colin/SolverMngr.h>

#include <utilib/PropertyValidators.h>

extern "C" {
#include <nsga2/global.h>
#include <nsga2/rand.h>
   
   int nreal = 0;
   int nbin = 0;
   int nobj = 0;
   int ncon = 0;
   int popsize = 100;
   double pcross_real = 0.9;
   double pcross_bin = 0.9;
   double pmut_real = 0.1;
   double pmut_bin = 0.1;
   double eta_c = 10;
   double eta_m = 20;
   int ngen = 100;
   int *nbits = NULL;
   double *min_realvar = NULL;
   double *max_realvar = NULL;
   double *min_binvar = NULL;
   double *max_binvar = NULL;

   int bitlength = 0; // calculated within optimize()

   // The following is used by the algorithm and reset for each call to
   // optimize()
   int nbinmut = 0;
   int nrealmut = 0;
   int nbincross = 0;
   int nrealcross = 0;

   // The following are used by NSGA for plotting (which we disable)
   int choice = 0;  // do not plot / display anything during optimization
   int obj1 = 0;
   int obj2 = 0;
   int obj3 = 0;
   int angle1 = 0;
   int angle2 = 0;
}

using utilib::Any;
using utilib::ReadOnly_Property;

using std::cout;
using std::cerr;
using std::endl;
using std::printf;

namespace interfaces {

class NSGA2 : public colin::Solver<colin::MO_MINLP0_problem>
{
   // WORKAROUND: pgCC 9 apparently cannot automatically identify the
   // type of the pointer to a global template function.  As a result,
   // we will explicitly cast the validation callback pointers to their
   // actual function pointer type.  Note that this does not appear to
   // be a problem with pgCC 10, and when we deprecate support for pgCC
   // 9, we should also remove these explicit casts.
   typedef bool(*validate_fcn_t)( const utilib::ReadOnly_Property&, 
                                  const utilib::Any& );

public:
   NSGA2()
      : run_number(0),
        defaultSeed(0.5),
        name("")
   {
      if ( instance )
         EXCEPTION_MNGR(std::runtime_error, "NSGA2: Instantiating "
                        "multiple instances of the NSGA2 solver wrapper.  "
                        "The NSGA2 solver is not threadsafe and can only "
                        "support one instance at a time.");
      instance = this;

      {
         utilib::Privileged_Property p(popsize);
         p.validate().connect( &multiple_of_4 );
         properties.declare( "popsize", 
                             "The population size (a multiple of 4)", p );
      }
      {
         utilib::Privileged_Property p(ngen);
         p.validate().connect
            ( static_cast<validate_fcn_t>
              (&utilib::PropertyValidators::Nonnegative<int>) );
         properties.declare( "ngen", "The number of generations", p );
      }

      {
         utilib::Privileged_Property p(pcross_real);
         p.validate().connect( &NSGA2::inrange_0_1 );
         properties.declare( "pcross_real", "The probability of crossover "
                             "of real variables (0.6-1.0)", p );
      }
      {
         utilib::Privileged_Property p(pmut_real);
         p.validate().connect( &NSGA2::inrange_0_1 );
         properties.declare( "pmut_real", "The probability of mutation "
                             "of real variables (1/nreal)", p );
      }
      {
         utilib::Privileged_Property p(eta_c);
         p.validate().connect
            ( static_cast<validate_fcn_t>
              (&utilib::PropertyValidators::Nonnegative<int>) );
         properties.declare( "eta_c", "The value of distribution index "
                             "for crossover (5-20)", p );
      }
      {
         utilib::Privileged_Property p(eta_m);
         p.validate().connect
            ( static_cast<validate_fcn_t>
              (&utilib::PropertyValidators::Nonnegative<int>) );
         properties.declare( "eta_m", "The value of distribution index "
                             "for mutation (5-50)", p );
      }

      {
         utilib::Privileged_Property p(discrete_bits);
         properties.declare( "nbits", "The number of bits to use to store "
                             "each binary (discrete) variable", p );
      }
      {
         utilib::Privileged_Property p(pcross_bin);
         p.validate().connect( &NSGA2::inrange_0_1 );
         properties.declare( "pcross_bin", "The probability of crossover "
                             "of binary (discrete) variables (0.6-1.0)", p );
      }
      {
         utilib::Privileged_Property p(pmut_bin);
         p.validate().connect( &NSGA2::inrange_0_1 );
         properties.declare( "pmut_bin", "The probability of mutation "
                             "of binary (discrete) variables (1/nbits)", p );
      }

      seed = 0;
      {
         utilib::Privileged_Property p(seed);
         p.validate().connect( &NSGA2::rng_range );
         properties.declare( "seed", "The random number generator seed "
                             "(in (0,1))", p );
      }
   }

   ~NSGA2()
   {
      if ( this == instance )
         instance = NULL;
   }

   void optimize()
   {
      ++run_number;
      generation = 1;
      std::stringstream ss;
      ss << "nsga-" << run_number;
      name = ss.str();

      domain.resize( problem->num_binary_vars, 
		     problem->num_int_vars, 
		     problem->num_real_vars );

      nobj = problem->num_objectives;
      ncon = problem->num_constraints;
      nreal = problem->num_real_vars;
      real_lb = problem->real_lower_bounds;
      real_ub = problem->real_upper_bounds;
      min_realvar = real_lb.data();
      max_realvar = real_ub.data();
      nbin = 
         problem->num_int_vars.as<int>() + problem->num_binary_vars.as<int>();
      discrete_lb = problem->int_lower_bounds;
      discrete_lb.resize(nbin);
      discrete_ub = problem->int_upper_bounds;
      discrete_ub.resize(nbin);
      for( size_t i = problem->num_int_vars; i < nbin; ++i )
      {
         discrete_lb[i] = 0;
         discrete_ub[i] = 1;
      }
      min_binvar = discrete_lb.data();
      max_binvar = discrete_ub.data();

      if ( discrete_bits.empty() )
      {
         discrete_bits.resize(nbin);
         for( size_t i = 0; i < discrete_bits.size(); ++i )
            discrete_bits[i] = 1 +  static_cast<int>
	       ( std::floor( std::log(discrete_ub[i]-discrete_lb[i])
			     / std::log(2) ) );
      }
      else if ( discrete_bits.size() != nbin )
         EXCEPTION_MNGR(std::runtime_error, "NSGA2::optimize(): specified "
                        "binary variable bitsize vector does not match the "
                        "number of binary variables (" 
                        << discrete_bits.size() << " != " << nbin << ").");
      else
         for( size_t i = 0; i < discrete_bits.size(); ++i )
            if ( discrete_bits[i] <
                 1 + std::floor( std::log(discrete_ub[i]-discrete_lb[i]) / 
                                 std::log(2) ) )
               cerr << "WARNING: insufficient bit storage for binary variable "
                    << i << ": specified " << discrete_bits[i] 
                    << " bits but need "
                    << (1 + std::floor( std::log(discrete_ub[i]-discrete_lb[i])
                                        / std::log(2) )) << " bits." << endl;

      nbits = discrete_bits.data();
      bitlength = 0;
      for( size_t i = 0; i < discrete_bits.size(); ++i )
         bitlength += nbits[i];

      print_solver_params();

      // Here is the actual NSGA2 algorithm
      nbinmut = 0;
      nrealmut = 0;
      nbincross = 0;
      nrealcross = 0;
      population* parent_pop = new population;
      population* child_pop = new population;
      population* mixed_pop = new population;
      allocate_memory_pop (parent_pop, popsize);
      allocate_memory_pop (child_pop, popsize);
      allocate_memory_pop (mixed_pop, 2*popsize);
      if ( run_number == 1 || seed != 0 )
      {
         // the seed *must* be in (0,1); so if the user didn't specify a
         // seed (or specified 0!), we need to temporarilt set it to a
         // valid value in order to warmup the random generator
         double real_seed = seed;
         if ( seed == 0 )
            seed = defaultSeed;
         randomize();
         seed = real_seed;
      }
      initialize_pop (parent_pop);

      // cheat here: overwrite the initial population with our actual
      // initial population
      if ( initial_points->size( problem ) > popsize )
         EXCEPTION_MNGR(std::runtime_error, "NSGA2::optimize(): " 
                        "passed more initial points than population size ("
                        << initial_points->size( problem )
                        << " > " << popsize << ")");
      int individial_id = 0;
      for( colin::Cache::iterator it = initial_points->begin( problem );
           it != initial_points->end();
           ++it )
      {
         it->second.asResponse(problem).get_domain(domain);
         from_MixedInt( parent_pop->ind[individial_id].xreal,
                        parent_pop->ind[individial_id].gene,
                        domain );
         ++individial_id;
      }

      cout << "Initialization complete." << endl;
      decode_pop(parent_pop);
      evaluate_pop (parent_pop);
      assign_rank_and_crowding_distance (parent_pop);
      cout << "gen = 1" << endl;
      for (generation=2; generation<=ngen; generation++)
      {
         selection (parent_pop, child_pop);
         mutation_pop (child_pop);
         decode_pop(child_pop);
         evaluate_pop(child_pop);
         merge (parent_pop, child_pop, mixed_pop);
         fill_nondominated_sort (mixed_pop, parent_pop);
         cout << "gen = " << generation << endl;
      }

      for( int i = 0; i < popsize; ++i )
      {
         to_MixedInt(parent_pop->ind[i].xreal, parent_pop->ind[i].gene, domain);
         final_points.add_point(problem, domain);
      }

      deallocate_memory_pop (parent_pop, popsize);
      deallocate_memory_pop (child_pop, popsize);
      deallocate_memory_pop (mixed_pop, 2*popsize);
      delete parent_pop;
      delete child_pop;
      delete mixed_pop;
   }


protected:
   ///
   std::string define_solver_type() const
   { return "NSGA2"; }

private:
   friend void ::test_problem( double *xreal, double *xbin, int **gene, 
                               double *obj, double *constr );

   static bool multiple_of_4( const ReadOnly_Property&, const Any& new_value )
   {
      int val;
      utilib::TypeManager()->lexical_cast(new_value, val);
      return (( val > 0 ) && ( val % 4 == 0 ));
   }

   static bool inrange_0_1( const ReadOnly_Property&, const Any& new_value )
   {
      double val;
      utilib::TypeManager()->lexical_cast(new_value, val);
      return (( val >= 0. ) && ( val <= 1. ));
   }
   static bool rng_range( const ReadOnly_Property&, const Any& new_value )
   {
      double val;
      utilib::TypeManager()->lexical_cast(new_value, val);
      return (( val >= 0. ) && ( val < 1. ));
   }

   void evaluate( double *xreal, int **gene, 
                  double *obj, double *constr )
   {
      to_MixedInt(xreal, gene, domain);

      std::vector<double> obj_val;
      std::vector<double> con_viol;

      colin::AppRequest req = problem->set_domain(domain);
      problem->Request_MF(req, obj_val);
      problem->Request_CFViol(req, con_viol);
      colin::AppResponse response = eval_mngr().perform_evaluation(req);

      colin::CacheHandle cache = colin::CacheFactory().evaluation_cache();
      if ( ! cache.empty() )
      {
         colin::Cache::iterator it = 
            cache->find(problem, response.get_domain()).first;
         if ( it != cache->end() )
         {
            std::string key = name + ".generated";
            if ( it->second.annotations.find(key) == it->second.annotations.end() )
               cache->annotate(it, key, generation);
         }
      }

      for( size_t i = 0; i < obj_val.size(); ++i )
         obj[i] = obj_val[i];

      // NSGA assumes all constraints are 1-sided inequalities.
      // However, since it only really cares about constraint violation
      // (assumed to be negative), we can be lazy here and just return
      // violation (instead of splitting the constraints)
      for( size_t i = 0; i < con_viol.size(); ++i )
         constr[i] = con_viol[i] > 0 ? -1.0*con_viol[i] : con_viol[i];
   }

   void to_MixedInt(double* xreal, int** gene, utilib::MixedIntVars &ans)
   {
      for( int i = 0; i < nreal; ++i )
         ans.Real()[i] = xreal[i];
      for( int i = 0; i < nbin; ++i )
      {
         int val = 0;
         for( int j = 0; j < discrete_bits[i]; ++j )
         {
            val <<= 1;
            if ( gene[i][j] )
               val += 1;
         }
         if ( i < ans.Integer().size() )
            ans.Integer()[i] = val;
         else
            ans.Binary()[i] = val;
      }
   }

   void from_MixedInt(double* xreal, int** gene, utilib::MixedIntVars &ans)
   {
      for( size_t i = 0; i < domain.Real().size(); ++i )
         xreal[i] = domain.Real()[i];
      for( size_t i = 0; i < nbin; ++i )
         for( size_t j = 0; j < nbits[i]; ++j )
            gene[i][j] = 0;
      for( size_t i = 0; i < domain.Integer().size(); ++i )
      {
         int val = domain.Integer()[i] - 
            static_cast<int>(std::floor(discrete_lb[i]));
         for( size_t j = nbits[i]; j > 0; )
         {
            --j;
            gene[i][j] = val & 1;
            val >>= 1;
         }
      }
      for( size_t i = domain.Integer().size(); i < nbin; ++i )
      {
         gene[i][nbits[i]-1] = domain.Binary()[i - domain.Integer().size()];
      }
   }

   void print_solver_params()
   {
      std::ostream &os = std::cout;
      os << "Population size = " << popsize << endl;
      os << "Number of generations = " << ngen << endl;
      os << "Number of objective functions = " << nobj << endl;
      os << "Number of constraints = " << ncon << endl;
      os << "Number of real variables = " << nreal << endl;
      if ( nreal != 0 )
      {
	 for ( int i=0; i<nreal; i++ )
            os << "   Real variable " << i 
	       << ": bounds = [ " << min_realvar[i] 
	       << ", " << max_realvar[i] << " ]" << endl;

	 os << "Probability of crossover of real variable = " << pcross_real 
	    << endl;
	 os << "Probability of mutation of real variable = " << pmut_real 
	    << endl;
	 os << "Distribution index for crossover = " << eta_c << endl;
	 os << "Distribution index for mutation = " << eta_m << endl;
      }
      os << "Number of binary variables = " << nbin << endl;
      if (nbin!=0)
      {
	 for (int i=0; i<nbin; i++)
            os << "   Binary variable " << i 
	       << ": bounds = [ " << min_binvar[i] 
	       << ", " << max_binvar[i] 
	       << " ],  nbits = " << nbits[i] << endl;

	 os << "Probability of crossover of binary variable = " << pcross_bin
	    << endl;
	 os << "Probability of mutation of binary variable = " << pmut_bin
	    << endl;
      }
      os << "Seed for random number generator = " << seed;
      if ( seed == 0. )
      {
         if ( run_number == 1 )
            os << " (using " << defaultSeed << " to warmup the generator)";
         else
            os << " (using the current generator state)";
      }
      os << endl;
   }

private:

   static NSGA2* instance;

   int run_number;
   int generation;
   double defaultSeed;
   std::string name;

   std::vector<double> real_lb;
   std::vector<double> real_ub;
   std::vector<int> discrete_bits;
   std::vector<double> discrete_lb;
   std::vector<double> discrete_ub;

   utilib::MixedIntVars domain;
};

NSGA2* NSGA2::instance = NULL;

REGISTER_COLIN_SOLVER_WITH_ALIAS(NSGA2, "NSGA2", "nsga2", 
                                 "The Non-Dominated Sorting Genetic Algorithm "
                                 "(NSGA-II) multi-objective optimizer");

} // namespace interfaces

void test_problem ( double *xreal, double *xbin, int **gene, 
                    double *obj, double *constr )
{
   interfaces::NSGA2::instance->evaluate(xreal, gene, obj, constr);
}

#endif // defined ACRO_USING_NSGA2
