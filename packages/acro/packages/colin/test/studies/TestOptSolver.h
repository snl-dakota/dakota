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
 * \file TestOptSolver.h
 *
 * Defines TestOptSolver, a class that defines a simple solver
 * for exercising COLIN.
 */

#include <acro_config.h>
#include <colin/solver/ColinSolver.h>
#include <utilib/PM_LCG.h>
#include <boost/bind.hpp>
#include <vector>

using namespace colin;


class TestOptSolver
   : public ColinSolver<std::vector<double>, NLP0_problem>
{
public:
   
   TestOptSolver() 
   { 
      reset_signal.connect
         ( boost::bind(&TestOptSolver::reset_TestOptSolver, this) ); 
   }
     
    virtual ~TestOptSolver() 
      {}

    void optimize();

  private:
    void reset_TestOptSolver()
      {
      pm_rng.set_seed(100); 
      set_rng(&pm_rng);
      if (initial_point_flag)
        { curr = initial_point; }
      }

  protected:
   virtual std::string define_solver_type() const
   { return "TestOptSolver"; }

    void initialize_best_point()
      {
      initial_point.resize(problem->num_real_vars);
      }

    std::vector<double> curr;

    utilib::PM_LCG pm_rng;

  };


class TestOptSolver_g 
  : public ColinSolver<std::vector<double>,NLP1_problem >
  {
  public:
    TestOptSolver_g() 
   { 
      reset_signal.connect
         ( boost::bind(&TestOptSolver_g::reset_TestOptSolver_g, this) ); 
   }

    virtual ~TestOptSolver_g() 
      {}

    void optimize();

    void reset_TestOptSolver_g()
      {
      pm_rng.set_seed(100); 
      set_rng(&pm_rng);
      if (initial_point_flag)
        { curr = initial_point; }
      }

  protected:
   virtual std::string define_solver_type() const
   { return "TestOptSolver_g"; }

    void initialize_best_point()
      { initial_point.resize(problem->num_real_vars); }

    std::vector<double> curr;
    utilib::PM_LCG pm_rng;
  };


class AnotherOptSolver 
  : public ColinSolver<utilib::BasicArray<double>,NLP0_problem>
  {
  public:

    AnotherOptSolver() 
   { 
      reset_signal.connect
         ( boost::bind(&AnotherOptSolver::reset_AnotherOptSolver, this) ); 
   }

    virtual ~AnotherOptSolver() 
      {}

    void optimize();

    void reset_AnotherOptSolver()
      {
      pm_rng.set_seed(100); 
      set_rng(&pm_rng);
      if (initial_point_flag)
        { curr = initial_point; }
      }


  protected:
   virtual std::string define_solver_type() const
   { return "AnotherOptSolver"; }

    void initialize_best_point()
      { initial_point.resize(problem->num_real_vars); }

    utilib::BasicArray<double> curr;
    utilib::PM_LCG pm_rng;
  };


#if 0
class TestOptSolver_mf 
  : public ConcreteOptSolver<std::vector<double>,MONLP0_problem >
  {
  public:

    TestOptSolver_mf() 
      {}

    virtual ~TestOptSolver_mf() 
      {}

    void minimize();

    void reset()
      {
      ConcreteOptSolver<std::vector<double>,MONLP0_problem>::reset();
      }

  protected:

    void initialize_best_point()
      { initial_point.resize(problem->num_real_vars); }
  };
#endif


