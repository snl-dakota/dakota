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
 * \file SubspaceApplication.h
 *
 * Defines the colin::SubspaceApplication class
 */

#ifndef colin_SubspaceApplication
#define colin_SubspaceApplication

#include <acro_config.h>

#include <colin/Application.h>
#include <colin/reformulation/Base.h>
#include <colin/TinyXML.h>

#include <utilib/MixedIntVars.h>

#include <boost/bimap.hpp>

namespace colin
{

template<typename ProblemT> class SubspaceApplication;


class SubspaceApplication_helper
{
public:
   typedef boost::bimap<size_t, std::string>  labels_t;


   template<typename FIXED, typename ARRAY>
   static void merge( const FIXED &fix, const ARRAY& src, ARRAY& dest, 
                      const char* domain_name )
   {
      // this test relies on the fact that maps are sorted...
      dest.resize(fix.size() + src.size());
      if (( ! fix.empty() ) && ( dest.size() <= fix.rbegin()->first ))
         EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication_helper::"
                        "merge(): " << domain_name << 
                        " domain size mismatch: src (" << src.size() << 
                        ") + fixed (" << fix.size() << 
                        ") != base application (" << dest.size() << ")");

      size_t i = 0;
      typename FIXED::const_iterator f_it = fix.begin();
      typename FIXED::const_iterator f_itEnd = fix.end();
      // I'd like to use iterators here, but BitArray does not support them.
      //typename ARRAY::const_iterator s_it = src.begin();
      //typename ARRAY::const_iterator s_itEnd = src.end();
      size_t s_it = 0;
      size_t s_itEnd = src.size();
      
      while ( true )
      {
         if (( f_it == f_itEnd ) || ( f_it->first > i ))
         {
            if ( s_it == s_itEnd )
               break;

            dest[i++] = src[s_it];
            ++s_it;
         }
         else 
         {
            dest[i++] = f_it->second;
            ++f_it;
         }
      }
   }
    

   template<bool TEST>
   struct Equality {
      template<typename T, typename U>
      inline static bool test(const T& a, const U& b)
      { return a == b; }
   };
   

   template<bool TEST, typename FIXED, typename ARRAY>
   static bool split( const FIXED &fix, const ARRAY& src, ARRAY& dest, 
                      const char* domain_name )
   {
      // this test relies on the fact that maps are sorted...
      if (( domain_name ) && ( ! fix.empty() ) && 
          ( src.size() <= fix.rbegin()->first ))
         EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication_helper::"
                        "split(): " << domain_name << 
                        " domain size mismatch: fixed references index (" << 
                        fix.rbegin()->first << 
                        ") past end of src (" << src.size() << ")");

      bool exact = true;
      size_t i = 0;
      dest.resize(src.size() - fix.size());

      typename FIXED::const_iterator f_it = fix.begin();
      typename FIXED::const_iterator f_itEnd = fix.end();
      // I'd like to use iterators here, but BitArray does not support them.
      //typename ARRAY::const_iterator s_it = src.begin();
      //typename ARRAY::const_iterator s_itEnd = src.end();
      size_t s_it = 0;
      size_t s_itEnd = src.size();
      
      while ( true )
      {
         if (( f_it == f_itEnd ) || ( f_it->first > s_it ))
         {
            if ( s_it == s_itEnd )
               break;

            dest[i++] = src[s_it++];
         }
         else 
         {
            exact &= Equality<TEST>::test(src[s_it], f_it->second);
            ++f_it;
            ++s_it;
         }
      }
      return exact;
   }

   template<typename FCN, typename PROB, typename FIXED, typename LABEL >
   static void filter_labels(void(FCN::*set)(const size_t, const std::string&),
                             PROB &me, FIXED &fixed, const LABEL &lbl)
   {
      // NB: This is HORRIBLY inefficient when using the property system!
      size_t offset = 0;
      typename LABEL::const_iterator l_it = lbl.begin();
      typename LABEL::const_iterator l_itEnd = lbl.end();
      typename FIXED::iterator f_it = fixed.begin();
      typename FIXED::iterator f_itEnd = fixed.end();
      while ( l_it != l_itEnd )
      {
         if (( f_it == f_itEnd ) || ( f_it->first > l_it->first ))
         {
            (me.*set)(l_it->first - offset, l_it->second);
            ++l_it;
         }
         else 
         {
            if ( f_it->first == l_it->first )
               ++l_it;
            ++f_it;
            ++offset;
         }
      }
   }


   template<int>
   struct Domain;
};


template<>
struct SubspaceApplication_helper::Equality<false> {
   template<typename T, typename U>
   inline static bool test(const T&, const U&)
   { return true; }
};

/// Domain updater, templated on what portion of the domain is being updated
template<int>
struct SubspaceApplication_helper::Domain {
   template<typename ProblemT>
   static void 
   update_domain_info(ProblemT &/*me*/)
   {}
};

/// Domain updater for continuous domains
template<>
struct SubspaceApplication_helper::Domain<ProblemTrait(reals)> 
{
   template<typename ProblemT>
   inline static 
   void update_domain_info(ProblemT &me)
   {
      size_t num_r = 0;
      labels_t r_lbl;
      std::vector<double> r_lower;
      std::vector<double> r_upper;
      BoundTypeArray r_lower_type;
      BoundTypeArray r_upper_type;

      if ( me.remote_app->problem_type()
           == ProblemT::remote_c_app_t::value )
      {
         Problem<typename ProblemT::remote_c_app_t> p 
            = me.remote_app->get_problem().template expose
            <Problem<typename ProblemT::remote_c_app_t> >();
         
         num_r = p->num_real_vars;
         r_lbl = p->real_labels;
         r_lower = p->real_lower_bounds;
         r_upper = p->real_upper_bounds;
         r_lower_type = p->real_lower_bound_types;
         r_upper_type = p->real_upper_bound_types;
      }
      else
      {
         Problem<typename ProblemT::remote_mi_app_t> p 
            = me.remote_app->get_problem().template expose
            <Problem<typename ProblemT::remote_mi_app_t> >();
         num_r = p->num_real_vars;
         r_lbl = p->real_labels;
         r_lower = p->real_lower_bounds;
         r_upper = p->real_upper_bounds;
         r_lower_type = p->real_lower_bound_types;
         r_upper_type = p->real_upper_bound_types;
      }
      
      // copy over the un-fixed information

      // real data
      if (( ! me.fixed_real.empty() ) && 
          ( num_r <= me.fixed_real.rbegin()->first ))
         EXCEPTION_MNGR(std::runtime_error, 
                        "SubspaceApplication_helper::update_domain_info(): "
                        "fixed real variable outside base application "
                        "domain");
      me._num_real_vars = num_r - me.fixed_real.size();
      me._real_labels = labels_t();
      filter_labels(&ProblemT::_setRealLabel, me, me.fixed_real, r_lbl.left);
      {
         std::vector<double> l;
         std::vector<double> u;
         split<false>(me.fixed_real, r_lower, l, NULL);
         split<false>(me.fixed_real, r_upper, u, NULL);
         me._real_lower_bounds = l;
         me._real_upper_bounds = u;
      }
      {
         BoundTypeArray l;
         BoundTypeArray u;
         split<false>(me.fixed_real, r_lower_type, l, NULL);
         split<false>(me.fixed_real, r_upper_type, u, NULL);
         me._real_lower_bound_types = l;
         me._real_upper_bound_types = u;
      }
   }
};

/// Domain updater for integer domains
template<>
struct SubspaceApplication_helper::Domain<ProblemTrait(integers)> 
{
   template<typename ProblemT>
   inline static 
   void update_domain_info(ProblemT &me)
   {
      size_t num_i = 0;
      labels_t i_lbl;
      std::vector<int> i_lower;
      std::vector<int> i_upper;
      BoundTypeArray i_lower_type;
      BoundTypeArray i_upper_type;

      if ( me.remote_app->problem_type()
           == ProblemT::remote_c_app_t::value )
      {
         // Pass: the base problem has no binary information
      }
      else
      {
         Problem<typename ProblemT::remote_mi_app_t> p 
            = me.remote_app->get_problem().template expose
            <Problem<typename ProblemT::remote_mi_app_t> >();
         num_i = p->num_int_vars;
         i_lbl = p->int_labels;
         i_lower = p->int_lower_bounds;
         i_upper = p->int_upper_bounds;
         i_lower_type = p->int_lower_bound_types;
         i_upper_type = p->int_upper_bound_types;
      }
      
      // copy over the un-fixed information

      // int data
      if (( ! me.fixed_int.empty() ) && 
          ( num_i <= me.fixed_int.rbegin()->first ))
         EXCEPTION_MNGR(std::runtime_error, 
                        "SubspaceApplication_helper::update_domain_info(): "
                        "fixed integer variable outside base application "
                        "domain");
      me._num_int_vars = num_i - me.fixed_int.size();
      me._int_labels = labels_t();
      filter_labels(&ProblemT::_setIntLabel, me, me.fixed_int, i_lbl.left);
      {
         std::vector<int> l;
         std::vector<int> u;
         split<false>(me.fixed_int, i_lower, l, NULL);
         split<false>(me.fixed_int, i_upper, u, NULL);
         me._int_lower_bounds = l;
         me._int_upper_bounds = u;
      }
      {
         BoundTypeArray l;
         BoundTypeArray u;
         split<false>(me.fixed_int, i_lower_type, l, NULL);
         split<false>(me.fixed_int, i_upper_type, u, NULL);
         me._int_lower_bound_types = l;
         me._int_upper_bound_types = u;
      }
   }
};

/// Domain updater for binary domains
template<>
struct SubspaceApplication_helper::Domain<ProblemTrait(binarys)> 
{
   template<typename ProblemT>
   inline static 
   void update_domain_info(ProblemT &me)
   {
      size_t num_b = 0;
      labels_t b_lbl;

      if ( me.remote_app->problem_type()
           == ProblemT::remote_c_app_t::value )
      {
         // Pass: the base problem has no binary information
      }
      else
      {
         Problem<typename ProblemT::remote_mi_app_t> p 
            = me.remote_app->get_problem().template expose
            <Problem<typename ProblemT::remote_mi_app_t> >();
         num_b = p->num_binary_vars;
         b_lbl = p->binary_labels;
      }
      
      // copy over the un-fixed information

      if (( ! me.fixed_binary.empty() ) && 
          ( num_b <= me.fixed_binary.rbegin()->first ))
         EXCEPTION_MNGR(std::runtime_error, 
                        "SubspaceApplication_helper::update_domain_info(): "
                        "fixed binary variable outside base application "
                        "domain");
      me._num_binary_vars = num_b - me.fixed_binary.size();

      // copy binary data
      me._binary_labels = labels_t();
      filter_labels(&ProblemT::_setBinaryLabel, me, me.fixed_binary, 
                    b_lbl.left);
   }
};




/// This is a class for application casts from a mixed integer search domain
/// int an application that searches a subspace of mixed integers.
///
template<class ProblemT>
class SubspaceApplication
   : public Application<ProblemT>,
     public ReformulationApplication
{
   friend class SubspaceApplication_helper;
   friend struct SubspaceApplication_helper::Domain<
      GetProblemTraitValue(ProblemT, reals)>;
   friend struct SubspaceApplication_helper::Domain<
      GetProblemTraitValue(ProblemT, integers)>;
   friend struct SubspaceApplication_helper::Domain<
      GetProblemTraitValue(ProblemT, binarys)>;

   /// The ProblemT for the base application (mixed integer form)
   typedef typename
   GetProblem< ProblemT::value 
                  | ProblemTrait(reals)
                  | ProblemTrait(integers)
                  | ProblemTrait(binarys) >::type  remote_mi_app_t;

   /// The ProblemT for the base application (continuous form)
   typedef typename
   GetProblem< (ProblemT::value | ProblemTrait(reals) )
                  & ~ ProblemTrait(integers)
                  & ~ ProblemTrait(binarys) >::type  remote_c_app_t;


public:

   /// Constructor
   SubspaceApplication()
   {
      constructor();
   }

   /// Constructor
   SubspaceApplication(ApplicationHandle src)
   { 
      constructor();
      // set the base problem that we are wrapping
      reformulate_application(src);
   }

   /// Destructor
   virtual ~SubspaceApplication()
   {}


   /// Convert the domain supplied by the solver to the application domain
   virtual bool
   map_domain(const utilib::Any& src, utilib::Any& native, bool forward) const
   {
      // This is not terribly efficient, as if we are working in a
      // continuous space, this will convert the domain from a vector
      // into a MixedIntVars for no good reason.  But it makes writing
      // this easier.  If efficiency proves to be an issue, reimplement
      // this...
      utilib::Any src_any;
      utilib::TypeManager()->lexical_cast
          (src, src_any, typeid(utilib::MixedIntVars));
      const utilib::MixedIntVars &mi_src
         = src_any.expose<utilib::MixedIntVars>();
      
      utilib::MixedIntVars &mi_dest = native.set<utilib::MixedIntVars>();

      if ( forward )
      {
         // merge the provided domain values with the fixed domain values
         SubspaceApplication_helper
            ::merge(fixed_real, mi_src.Real(), mi_dest.Real(), "Real");
         SubspaceApplication_helper
            ::merge(fixed_int, mi_src.Integer(), mi_dest.Integer(), "Integer");
         SubspaceApplication_helper
            ::merge(fixed_binary, mi_src.Binary(), mi_dest.Binary(), "Binary");

         if (remote_app->problem_type() == remote_c_app_t::value)
         {
            Problem<remote_c_app_t> p = remote_app->get_problem()
               .template expose<Problem<remote_c_app_t> >();

            if ( p->num_real_vars != mi_dest.Real().size() )
               EXCEPTION_MNGR(std::runtime_error,
                              "SubspaceApplication::"
                              "map_domain(): Real domain size mismatch: "
                              "provided (" << mi_src.Real().size() << 
                              ") + fixed (" << fixed_real.size() << 
                              ") != base (" << p->num_real_vars << ")");
            if ( ! mi_dest.Integer().empty() || ! mi_dest.Binary().empty() )
               EXCEPTION_MNGR(std::runtime_error,
                              "SubspaceApplication::"
                              "map_domain(): Provided discrete domain to "
                              " an underlying continuous application");

            // The extra Any is necessary so that the MixedIntVars lives
            // through the assignment process
            utilib::Any tmp = native; 
            native = mi_dest.Real();
         }
         else
         {
            Problem<remote_mi_app_t> p = remote_app->get_problem()
               .template expose<Problem<remote_mi_app_t> >();

            if ( p->num_real_vars != mi_dest.Real().size() )
               EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication::"
                              "map_domain(): Real domain size mismatch: "
                              "provided (" << mi_src.Real().size() << 
                              ") + fixed (" << fixed_real.size() << 
                              ") != base (" << p->num_real_vars << ")");
            if ( p->num_int_vars != mi_dest.Integer().size() )
               EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication::"
                              "map_domain(): Integer domain size mismatch: "
                              "provided (" << mi_src.Integer().size() << 
                              ") + fixed (" << fixed_int.size() << 
                              ") != base (" << p->num_int_vars << ")");
            if ( p->num_binary_vars != mi_dest.Binary().size() )
               EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication::"
                              "map_domain(): Binary domain size mismatch: "
                              "provided (" << mi_src.Binary().size() << 
                              ") + fixed (" << fixed_binary.size() << 
                              ") != base (" << p->num_binary_vars << ")");
         }

         return true;
      }
      else
      {
         // split the fixed domain values from the core domain
         bool exact = SubspaceApplication_helper::split<true>
            (fixed_real, mi_src.Real(), mi_dest.Real(), "Real");
         exact &= SubspaceApplication_helper::split<true>
            (fixed_int, mi_src.Integer(), mi_dest.Integer(), "Integer");
         exact &= SubspaceApplication_helper::split<true>
            (fixed_binary, mi_src.Binary(), mi_dest.Binary(), "Binary");

         // If I am not a mixed int problem, only return the Reals...
         if ( ProblemT::value == remote_c_app_t::value )
         {
            // The extra Any is necessary so that the MixedIntVars lives
            // through the assignment process
            utilib::Any tmp = native;
            native = mi_dest.Real();
         }

         return exact;
      }
   }

   /// Return the fixed real variables
   const std::map<size_t, double>& get_fixed_real_vars() const
      { return fixed_real; }

   /// Returns the fixed integer variables
   const std::map<size_t, int>&  get_fixed_int_vars() const
      { return fixed_int; }

   /// Returns the fixed binary variables
   const std::map<size_t, bool>&  get_fixed_binary_vars() const
      { return fixed_binary; }

   /// Set the variables that this application will fix (and hide)
   void set_fixed_vars(const std::map<size_t, double> &real,
                       const std::map<size_t, int>    &integer,
                       const std::map<size_t, bool>   &binary )
   {
      fixed_real = real;
      fixed_int = integer;
      fixed_binary = binary;
      update_domain_info();
   }


protected:

   virtual void validate_reformulated_application(ApplicationHandle handle)
   {
      // verify this is actually a *downcast* (or at least, not an upcast)
      if (( handle->problem_type() != remote_mi_app_t::value ) &&
          ( handle->problem_type() != remote_c_app_t::value ))
         EXCEPTION_MNGR(std::runtime_error, 
                        "SubspaceApplication::"
                        "set_base_application(): The base problem type ("
                        << handle->problem_type_name() <<
                        ") is not a valid subspace of this problem type ("
                        << ProblemType<ProblemT>::name() << ")");
   }
   
   virtual void configure_reformulated_application()
   {
      // reference all appropriate external info (everything but the
      // domain info)
      std::set<ObjectType> exclude;
      exclude.insert(ObjectType::get<Application_RealDomain>());
      exclude.insert(ObjectType::get<Application_IntDomain>());
      this->reference_reformulated_application_properties(exclude);

      // clear out the fixed maps
      fixed_real.clear();
      fixed_int.clear();
      fixed_binary.clear();
      update_domain_info();
   }


private: // methods

   ///
   void constructor()
   {
      if ( HasProblemTrait(ProblemT, gradients) )
         EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication: "
                        "cannot hande gradients (yet).");

      initializer("FixedDomain").connect
         (boost::bind(&SubspaceApplication::cb_initialize, this, _1));
   }


   void update_domain_info()
   {
      SubspaceApplication_helper
         ::Domain<GetProblemTraitValue(ProblemT, reals)>
         ::update_domain_info(*this);
      SubspaceApplication_helper
         ::Domain<GetProblemTraitValue(ProblemT, integers)>
         ::update_domain_info(*this);
      SubspaceApplication_helper
         ::Domain<GetProblemTraitValue(ProblemT, binarys)>
         ::update_domain_info(*this);
   }


   ///
   void cb_initialize(TiXmlElement* elt)
   {
      if ( remote_app.empty() )
         EXCEPTION_MNGR(std::runtime_error, "SubspaceApplication::"
                        "xml_init_fixed(): cannot set fixed variables before "
                        "setting the base problem (" << utilib::get_element_info(elt)
                        << ")");
      
      size_t num_r = 0;
      size_t num_i = 0;
      size_t num_b = 0;
      SubspaceApplication_helper::labels_t r_lbl;
      SubspaceApplication_helper::labels_t i_lbl;
      SubspaceApplication_helper::labels_t b_lbl;

      // If I am not wrapping a mixed int problem, only get the Reals info...
      if (remote_app->problem_type()==remote_c_app_t::value)
      {
         Problem<remote_c_app_t> p = remote_app->get_problem()
            .template expose<Problem<remote_c_app_t> >();

         num_r = p->num_real_vars;
         r_lbl = p->real_labels;
      }
      else
      {
         Problem<remote_mi_app_t> p = remote_app->get_problem()
            .template expose<Problem<remote_mi_app_t> >();

         num_r = p->num_real_vars;
         r_lbl = p->real_labels;
         num_i = p->num_int_vars;
         i_lbl = p->int_labels;
         num_b = p->num_binary_vars;
         b_lbl = p->binary_labels;
      }

      TiXmlElement* child = elt->FirstChildElement();
      for (; child; child = child->NextSiblingElement())
      {
         const std::string& elt_text = child->ValueStr();
         /*
         if ( elt_text.compare("Integer") == 0)
            process_fixed_vars<int>( child, fixed_int, num_i, i_lbl.left );
         else if (elt_text.compare("Real") == 0)
            process_fixed_vars<double>( child, fixed_real, num_r, r_lbl.left );
         else if (elt_text.compare("Binary") == 0)
            process_fixed_vars<bool>( child, fixed_binary, num_b, b_lbl.left );
         else
         */
            EXCEPTION_MNGR(std::runtime_error,
                           "SubspaceApplication::xml_init_fixed"
                           " - unknown domain type: " << elt_text);
      }

      update_domain_info();
   }

private: // data

   ///
   std::map<size_t, bool> fixed_binary;

   ///
   std::map<size_t, int> fixed_int;

   ///
   std::map<size_t, double> fixed_real;

};

} // namespace colin

#endif // defined colin_SubspaceApplication_h
