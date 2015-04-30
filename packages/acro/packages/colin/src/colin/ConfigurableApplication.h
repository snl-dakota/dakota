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
 * \file ConfigurableApplication.h
 *
 * Defines the colin::ConfigurableApplication class.
 */

#ifndef colin_ConfigurableApplication_h
#define colin_ConfigurableApplication_h

#include <colin/Application.h>

#include <colin/real.h>

namespace colin
{

template<class ProblemT>
class ConfigurableApplication_Base : public Application<ProblemT>
{};


//---------------------------------------------------------------------
/// Allow for publicly setting Objective parameters
template<typename ProblemT, bool>
class ConfigurableApplication_ConfigMultiObjective
   : virtual public ConfigurableApplication_Base<ProblemT>
{
public:
   ConfigurableApplication_ConfigMultiObjective()
      : num_objectives(this->_num_objectives.unset_readonly()),
        sense(this->_sense.unset_readonly())
   {}
   
   utilib::Property num_objectives;
   utilib::Property sense;
};

template<typename ProblemT>
class ConfigurableApplication_ConfigMultiObjective<ProblemT, false>
   : virtual public ConfigurableApplication_Base<ProblemT>
{
public:
   ConfigurableApplication_ConfigMultiObjective()
      : sense(this->_sense.unset_readonly())
   {}
   
   utilib::Property sense;
};


//---------------------------------------------------------------------
/// Allow for publicly setting NonD Objective parameters
template<typename ProblemT, bool>
class ConfigurableApplication_ConfigNonD_Objective
   : virtual public ConfigurableApplication_Base<ProblemT>
{
public:
   ConfigurableApplication_ConfigNonD_Objective()
      : nond_objective(this->_nond_objective.unset_readonly())
   {}

   utilib::Property nond_objective;
};

template<typename ProblemT>
class ConfigurableApplication_ConfigNonD_Objective<ProblemT, false>
{};


//---------------------------------------------------------------------
/// Allow for publicly setting Domain parameters
template<typename ProblemT>
class ConfigurableApplication_ConfigDomain
   : virtual public ConfigurableApplication_Base<ProblemT>
{
public:
   ConfigurableApplication_ConfigDomain()
      : enforcing_domain_bounds(this->_enforcing_domain_bounds.unset_readonly())
   {}

   utilib::Property enforcing_domain_bounds;
};


//---------------------------------------------------------------------
/// Allow for publicly setting Real Domain parameters
template<typename ProblemT, bool>
class ConfigurableApplication_ConfigRealDomain
   : virtual public ConfigurableApplication_Base<ProblemT>
{
public:
   ConfigurableApplication_ConfigRealDomain()
      : num_real_vars(this->_num_real_vars.unset_readonly()),
        real_lower_bounds(this->_real_lower_bounds.unset_readonly()),
        real_upper_bounds(this->_real_upper_bounds.unset_readonly()),
        real_lower_bound_types(this->_real_lower_bound_types.unset_readonly()),
        real_upper_bound_types(this->_real_upper_bound_types.unset_readonly()),
        real_labels(this->_real_labels.unset_readonly())
   {}

   utilib::Property num_real_vars;
   utilib::Property real_lower_bounds;
   utilib::Property real_upper_bounds;
   utilib::Property real_lower_bound_types;
   utilib::Property real_upper_bound_types;
   utilib::Property real_labels;

   void setRealLowerBoundType(size_t i, bound_type_enum type)
   { this->Application_RealDomain::_setRealLowerBoundType(i, type); }

   void setRealUpperBoundType(size_t i, bound_type_enum type)
   { this->Application_RealDomain::_setRealUpperBoundType(i, type); }

   void setPeriodicRealBound(size_t i)
   { this->Application_RealDomain::_setPeriodicRealBound(i); }

   void setRealLabel(const size_t i, const std::string &label)
   { this->Application_RealDomain::_setRealLabel(i, label); }
};

template<typename ProblemT>
class ConfigurableApplication_ConfigRealDomain<ProblemT, false>
   {};


//---------------------------------------------------------------------
/// Allow for publicly setting Integer Domain parameters
template<typename ProblemT, bool>
class ConfigurableApplication_ConfigIntDomain
   : virtual public ConfigurableApplication_Base<ProblemT>
{
public:
   ConfigurableApplication_ConfigIntDomain()
      : num_int_vars(this->_num_int_vars.unset_readonly()),
        int_lower_bounds(this->_int_lower_bounds.unset_readonly()),
        int_upper_bounds(this->_int_upper_bounds.unset_readonly()),
        int_lower_bound_types(this->_int_lower_bound_types.unset_readonly()),
        int_upper_bound_types(this->_int_upper_bound_types.unset_readonly()),
        int_labels(this->_int_labels.unset_readonly()),
        num_binary_vars(this->_num_binary_vars.unset_readonly()),
        binary_labels(this->_binary_labels.unset_readonly())
   {}

   utilib::Property num_int_vars;
   utilib::Property int_lower_bounds;
   utilib::Property int_upper_bounds;
   utilib::Property int_lower_bound_types;
   utilib::Property int_upper_bound_types;
   utilib::Property int_labels;

   utilib::Property num_binary_vars;
   utilib::Property binary_labels;

   void setIntLowerBoundType(int i, bound_type_enum type)
   { this->Application_IntDomain::_setIntLowerBoundType(i, type); }

   void setIntUpperBoundType(int i, bound_type_enum type)
   { this->Application_IntDomain::_setIntUpperBoundType(i, type); }

   void setPeriodicIntBound(size_t i)
   { this->Application_IntDomain::_setPeriodicIntBound(i); }

   void setIntLabel(const size_t i, const std::string &label)
   { this->Application_IntDomain::_setIntLabel(i, label); }

   void setBinaryLabel(const size_t i, const std::string &label)
   { this->Application_IntDomain::_setBinaryLabel(i, label); }

};

template<typename ProblemT>
class ConfigurableApplication_ConfigIntDomain<ProblemT, false>
   {};


//---------------------------------------------------------------------
/// Allow for publicly setting Nonlinear Constraint parameters
template<typename ProblemT, bool, bool>
class ConfigurableApplication_ConfigNonlinearConstraints
   : public ConfigurableApplication_ConfigNonlinearConstraints<ProblemT, true, false>
{
public:
   ConfigurableApplication_ConfigNonlinearConstraints()
   {}
};

template<typename ProblemT>
class ConfigurableApplication_ConfigNonlinearConstraints<ProblemT, true, false>
   : virtual public ConfigurableApplication_Base<ProblemT>
{
public:
   ConfigurableApplication_ConfigNonlinearConstraints()
      : num_nonlinear_constraints(this->_num_nonlinear_constraints.unset_readonly()),
        nonlinear_constraint_lower_bounds(this->_nonlinear_constraint_lower_bounds.unset_readonly()),
        nonlinear_constraint_upper_bounds(this->_nonlinear_constraint_upper_bounds.unset_readonly()),
        nonlinear_constraint_labels(this->_nonlinear_constraint_labels.unset_readonly())
   {}

   utilib::Property num_nonlinear_constraints;
   utilib::Property nonlinear_constraint_lower_bounds;
   utilib::Property nonlinear_constraint_upper_bounds;
   utilib::Property nonlinear_constraint_labels;

   void setNonlinearConstraintLabel(const size_t i, const std::string &label)
   {
      this->Application_NonlinearConstraints::
         _setNonlinearConstraintLabel(i, label);
   }
};


template<typename ProblemT, bool grad>
class ConfigurableApplication_ConfigNonlinearConstraints<ProblemT, false, grad>
   {};



//---------------------------------------------------------------------
/// Allow for publicly setting Linear Constraint parameters
template<typename ProblemT, bool, bool>
class ConfigurableApplication_ConfigLinearConstraints
   : public ConfigurableApplication_ConfigLinearConstraints<ProblemT, true, false>
{
public:
   ConfigurableApplication_ConfigLinearConstraints()
      : linear_constraint_matrix(this->_linear_constraint_matrix.unset_readonly())
   {}

   utilib::Property linear_constraint_matrix;
};

template<typename ProblemT>
class ConfigurableApplication_ConfigLinearConstraints<ProblemT, true, false>
   : virtual public ConfigurableApplication_Base<ProblemT>
{
public:
   ConfigurableApplication_ConfigLinearConstraints()
      : num_linear_constraints(this->_num_linear_constraints.unset_readonly()),
        linear_constraint_lower_bounds(this->_linear_constraint_lower_bounds.unset_readonly()),
        linear_constraint_upper_bounds(this->_linear_constraint_upper_bounds.unset_readonly()),
        linear_constraint_labels(this->_linear_constraint_labels.unset_readonly())
   {}

   utilib::Property num_linear_constraints;
   utilib::Property linear_constraint_lower_bounds;
   utilib::Property linear_constraint_upper_bounds;
   utilib::Property linear_constraint_labels;

   void setLinearConstraintLabel(const size_t i, const std::string &label)
   {
      this->Application_LinearConstraints::_setLinearConstraintLabel(i, label);
   }
};

template<typename ProblemT, bool grad>
class ConfigurableApplication_ConfigLinearConstraints<ProblemT, false, grad>
   {};


//---------------------------------------------------------------------
/// Allow for publicly setting NonD Constraint parameters
template<typename ProblemT, bool, bool>
class ConfigurableApplication_ConfigNonD_Constraints
   : public ConfigurableApplication_ConfigNonD_Constraints<ProblemT, true, false>
{
public:
   ConfigurableApplication_ConfigNonD_Constraints()
   {}
};

template<typename ProblemT>
class ConfigurableApplication_ConfigNonD_Constraints<ProblemT, true, false>
   : virtual public ConfigurableApplication_Base<ProblemT>
{
public:
   ConfigurableApplication_ConfigNonD_Constraints()
      : num_nondeterministic_constraints(this->_num_nondeterministic_constraints.unset_readonly()),
        nondeterministic_constraint_lower_bounds(this->_nondeterministic_constraint_lower_bounds.unset_readonly()),
        nondeterministic_constraint_upper_bounds(this->_nondeterministic_constraint_upper_bounds.unset_readonly()),
        nondeterministic_constraint_labels(this->_nondeterministic_constraint_labels.unset_readonly())
   {}

   utilib::Property num_nondeterministic_constraints;
   utilib::Property nondeterministic_constraint_lower_bounds;
   utilib::Property nondeterministic_constraint_upper_bounds;
   utilib::Property nondeterministic_constraint_labels;

   void setNondeterministicConstraintLabel( const size_t i, 
                                            const std::string &label )
   {
      this->Application_NonD_Constraints::
         _setNondeterministicConstraintLabel(i, label);
   }
};


template<typename ProblemT, bool grad>
class ConfigurableApplication_ConfigNonD_Constraints<ProblemT, false, grad>
   {};




//---------------------------------------------------------------------
//---------------------------------------------------------------------

// forward declaration
template<typename ProblemT> class ConfigurableApplication;


namespace ConfigurableApplication_Helpers
{
///
template<typename ProblemT, bool>
class ConfigReal
{
   friend class colin::ConfigurableApplication<ProblemT>;
   static void set(ConfigurableApplication<ProblemT> *app,
                      size_t nreal, std::vector<real> &lb, std::vector<real> &ub)
   {
      app->num_real_vars = nreal;
      app->real_lower_bounds = lb;
      app->real_upper_bounds = ub;
   }
};

template<typename ProblemT>
class ConfigReal<ProblemT, false>
{
   friend class ConfigurableApplication<ProblemT>;
   static void set(ConfigurableApplication<ProblemT> *app,
                      size_t nreal, std::vector<real> &lb, std::vector<real> &ub)
   {
      static_cast<void>(app);
      static_cast<void>(nreal);
      static_cast<void>(lb);
      static_cast<void>(ub);
   }
};


///
template<typename ProblemT, bool>
class ConfigInt
{
   friend class ConfigurableApplication<ProblemT>;
   static void set(ConfigurableApplication<ProblemT> *app,
                      size_t nint, size_t nbit,
                      std::vector<int> &lb, std::vector<int> &ub)
   {
      app->set_num_int_vars(nint, nbit);
      app->set_int_bounds(lb, ub);
   }
};

template<typename ProblemT>
class ConfigInt<ProblemT, false>
{
   friend class ConfigurableApplication<ProblemT>;
   static void set(ConfigurableApplication<ProblemT> *app,
                      size_t nint, size_t nbit,
                      std::vector<int> &lb, std::vector<int> &ub)
   {
      static_cast<void>(app);
      static_cast<void>(nint);
      static_cast<void>(nbit);
      static_cast<void>(lb);
      static_cast<void>(ub);
   }
};

} // namespace ConfigurableApplication_Helpers




//---------------------------------------------------------------------

/**
  * This application class calls the function \c derived_compute_response to
  * perform the evaluation of the user's objective function FuncT.
  */
template <class ProblemT>
class ConfigurableApplication
   : public ConfigurableApplication_ConfigMultiObjective
        < ProblemT, HasProblemTrait(ProblemT, multiple_objectives) >,
     public ConfigurableApplication_ConfigNonD_Objective
        < ProblemT, HasProblemTrait(ProblemT, nondeterministic_response) >,
     public ConfigurableApplication_ConfigDomain
        < ProblemT >,
     public ConfigurableApplication_ConfigRealDomain
        < ProblemT, HasProblemTrait(ProblemT, reals) >,
     public ConfigurableApplication_ConfigIntDomain
        < ProblemT, HasProblemTrait(ProblemT, integers) >,
     public ConfigurableApplication_ConfigLinearConstraints
        < ProblemT, 
          HasProblemTrait(ProblemT, linear_constraints),
          HasProblemTrait(ProblemT, gradients) >,
     public ConfigurableApplication_ConfigNonlinearConstraints
        < ProblemT, 
          HasProblemTrait(ProblemT, nonlinear_constraints),
          HasProblemTrait(ProblemT, gradients) >,
     public ConfigurableApplication_ConfigNonD_Constraints
        < ProblemT, 
          HasProblemTrait(ProblemT, nondeterministic_response) 
             && HasProblemTrait(ProblemT, linear_constraints),
          HasProblemTrait(ProblemT, gradients) >
{
public:

   ///
   void set_bounds(const char* format)
   {
      if (format == NULL)
         return;

      int i, tmp, flen, ndx;
      int nint = 0, nreal = 0, nbit = 0;

      //
      // Process the 'format' string
      //
      flen = strlen(format);
      int info_len = 0;
      for (i = 0; i < flen; i++)
      {
         if (format[i] == ']')
         { info_len++; }
      }

      std::vector<std::vector<double> > Domain_Info(info_len);
      ndx = 0;

      while ((ndx < flen) && (format[ndx] != '*') && (format[ndx] != '['))
      { ndx++; }

      for (i = 0; i < info_len; i++)
      {
         while ((ndx < flen) && (format[ndx] != '['))
         { ndx++; }
         if (ndx == flen)
         { break; }

         double tmp1, tmp2;
         if (sscanf(&(format[++ndx]), "%lf,%lf", &tmp1, &tmp2) != 2)
         {
            EXCEPTION_MNGR(std::runtime_error,
                           "ConfigurableApplication::set_bounds - "
                           "Domain error at position " << ndx <<
                           " in string  \"" << format << "\".");
         }
         else
         {
            Domain_Info[i].resize(4);
            Domain_Info[i][0] = tmp1;
            Domain_Info[i][1] = tmp2;
         }
         int flag = false;
         while ((ndx < flen) && (format[ndx] != ']'))
         {
            if (format[ndx] == '.')
            { flag = true; }
            ndx++;
         }
         if (ndx == flen)
         { break; }

         ndx++;
         if (format[ndx] != '^')
         { tmp = 1; }
         else if (sscanf(&(format[++ndx]), "%d", &tmp) != 1)
         {
            EXCEPTION_MNGR(std::runtime_error,
                           "ConfigurableApplication::set_bounds: "
                           "Domain error at position " << ndx <<
                           " in string \"" << format << "\".");
         }
         Domain_Info[i][2] = tmp;

         if (flag)
         {
            Domain_Info[i][3] = 1; //RealDomain;
            nreal += (int) Domain_Info[i][2];
         }
         else
         {
            if ((Domain_Info[i][0] == 0.0) && (Domain_Info[i][1] == 1.0) &&
                  HasProblemTrait(ProblemT, binarys))
            {
               Domain_Info[i][3] = 3; // BitDomain;
               nbit += (int) Domain_Info[i][2];
            }
            else
            {
               // Default to real bound
               if (! HasProblemTrait(ProblemT, integers))
               {
                  Domain_Info[i][3] = 1; // RealDomainT;
                  nreal += (int) Domain_Info[i][2];
               }
               else
               {
                  Domain_Info[i][3] = 2; // IntDomainT;
                  nint += (int) Domain_Info[i][2];
               }
            }
         }
      }

      //
      // Process the Domain_Info
      //
      std::vector<int> int_lower_bounds(nint);
      std::vector<int> int_upper_bounds(nint);
      if (nint)
      {
         ndx = 0;
         for (i = 0; i < info_len; i++)
         {
            if (Domain_Info[i][3] == 2) //IntDomainT
            {
               for (int j = 0; j < Domain_Info[i][2]; j++, ndx++)
               {
                  int_lower_bounds[ndx] = (int) Domain_Info[i][0];
                  int_upper_bounds[ndx] = (int) Domain_Info[i][1];
               }
            }
         }
      }

      std::vector<real> real_lower_bounds(nreal);
      std::vector<real> real_upper_bounds(nreal);
      if (nreal)
      {
         ndx = 0;
         for (i = 0; i < info_len; i++)
         {
            if (Domain_Info[i][3] == 1) //RealDomainT
            {
               for (int j = 0; j < Domain_Info[i][2]; j++, ndx++)
               {
                  real_lower_bounds[ndx] = Domain_Info[i][0];
                  real_upper_bounds[ndx] = Domain_Info[i][1];
               }
            }
         }
      }


      ConfigurableApplication_Helpers::ConfigReal
      < ProblemT, HasProblemTrait(ProblemT, reals) >
      ::set(this, nreal, real_lower_bounds, real_upper_bounds);
      ConfigurableApplication_Helpers::ConfigInt
      < ProblemT, HasProblemTrait(ProblemT, integers) >
      ::set(this, nint, nbit, int_lower_bounds, int_upper_bounds);
   }

};



} // namespace colin


#endif // colin_ConfigurableApplication_h
