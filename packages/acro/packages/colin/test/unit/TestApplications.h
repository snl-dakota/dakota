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

#include <acro_config.h>
#include <colin/ConfigurableApplication.h>
#include <colin/SynchronousApplication.h>
#include <boost/bimap.hpp>

namespace colin {
namespace unittest {
namespace TestApplications {

class ComposableTestApplication
   : virtual public colin::Application_Base
{
public:
   typedef boost::bimap<size_t, std::string>  labels_t;

   ComposableTestApplication() {}

protected:  // data
   typedef 
   std::map<colin::response_info_t,
            boost::function< utilib::Any( const utilib::Any &domain,
                                          utilib::seed_t &seed ) > >
   compute_map_t;

   compute_map_t compute_map;

   boost::signal<void()>  configure_signal;

protected: // methods
   virtual void 
   perform_evaluation_impl( const utilib::Any &domain,
                            const colin::AppRequest::request_map_t &requests,
                            utilib::seed_t &seed,
                            colin::AppResponse::response_map_t &responses )
   {
      colin::AppRequest::request_map_t::const_iterator it = requests.begin();
      colin::AppRequest::request_map_t::const_iterator itEnd = requests.end();
      for( ; it != itEnd; ++it )
      {
         //std::cerr << "Processing request " << it->first << ": " 
         //          << colin::AppResponseInfo().name(it->first) << std::endl;
         compute_map_t::iterator fcn = compute_map.find(it->first);
         if ( fcn == compute_map.end() )
            EXCEPTION_MNGR( std::logic_error, "Unsupported response_info_t: "
                            << it->first << ": "
                            << colin::AppResponseInfo().name(it->first) 
                            << std::endl );

         responses.insert
            ( std::make_pair( it->first, fcn->second(domain, seed) ) );
      }
   }
};


/**
 * Define a continuous domain
 */
class _continuous : virtual public ComposableTestApplication
{
public:
   typedef std::vector<colin::real> domain_t;

   _continuous()
   {
      configure_signal.connect(boost::bind(&_continuous::configure, this));
   }

   virtual bool 
   map_domain(const utilib::Any &src, utilib::Any &native, bool) const
   {
      utilib::TypeManager()->lexical_cast(src, native, typeid(domain_t));
      if (this->properties["domain_size"] != native.expose<domain_t>().size())
         EXCEPTION_MNGR(std::runtime_error, 
                        "_continuous::map_domain: Domain size mismatch: "
                        "provided (" << native.expose<domain_t>().size() << 
                        ") does not match domain_size (" <<
                        this->properties["domain_size"] << ")");
      return true;
   }

private:
   void configure()
   {
      utilib::Privileged_PropertyDict &p = this->properties;

      size_t num_real = p["num_real_vars"];
      labels_t labels;
      domain_t lbounds(num_real);
      domain_t ubounds(num_real);
      for(size_t i = 0; i < num_real; ++i)
      {
         std::stringstream ss;
         ss << "x_" << i;
         labels.insert(labels_t::value_type(i, ss.str()));

         lbounds[i] = -static_cast<double>(i+1);
         ubounds[i] =  static_cast<double>(i+1);
      }
      p.privilegedGet("real_labels") = labels;
      p.privilegedGet("real_lower_bounds") = lbounds;
      p.privilegedGet("real_upper_bounds") = ubounds;

      colin::BoundTypeArray tmp = p["real_lower_bound_types"];
      if ( num_real > 1 )
         tmp[1] = colin::no_bound;
      if ( num_real > 2 )
         tmp[2] = colin::no_bound;
      p.privilegedGet("real_lower_bound_types") = tmp;
      if ( num_real > 3 )
         tmp[3] = colin::no_bound;
      p.privilegedGet("real_upper_bound_types") = tmp;
   }
};


/**
 * Define a integer domain
 */
class _integer : virtual public ComposableTestApplication
{
public:
   typedef std::vector<int>  domain_t;

   _integer()
   {
      configure_signal.connect(boost::bind(&_integer::configure, this));
   }

   virtual bool 
   map_domain(const utilib::Any &src, utilib::Any &native, bool) const
   {
      utilib::TypeManager()->lexical_cast(src, native, typeid(domain_t));
      if (this->properties["domain_size"] != native.expose<domain_t>().size())
         EXCEPTION_MNGR(std::runtime_error, 
                        "_integer::map_domain: Domain size mismatch: "
                        "provided (" << native.expose<domain_t>().size() << 
                        ") does not match domain_size (" <<
                        this->properties["domain_size"] << ")");
      return true;
   }

private:
   void configure()
   {
      utilib::Privileged_PropertyDict &p = this->properties;
      size_t num_int = p["num_int_vars"];
      labels_t labels;
      domain_t lbounds(num_int);
      domain_t ubounds(num_int);
      for(size_t i = 0; i < num_int; ++i)
      {
         std::stringstream ss;
         ss << "i_" << i;
         labels.insert(labels_t::value_type(i, ss.str()));

         lbounds[i] = -static_cast<int>(i+1);
         ubounds[i] =  static_cast<int>(i+1);
      }
      p.privilegedGet("int_labels") = labels;
      p.privilegedGet("int_lower_bounds") = lbounds;
      p.privilegedGet("int_upper_bounds") = ubounds;

      colin::BoundTypeArray tmp = p["int_lower_bound_types"];
      if ( num_int > 1 )
         tmp[1] = colin::no_bound;
      if ( num_int > 2 )
         tmp[2] = colin::no_bound;
      p.privilegedGet("int_lower_bound_types") = tmp;
      if ( num_int > 3 )
         tmp[3] = colin::no_bound;
      p.privilegedGet("int_upper_bound_types") = tmp;
   }
};

/**
 * Define a binary domain
 */
class _binary : virtual public ComposableTestApplication
{
public:
   typedef std::vector<bool> domain_t;

   _binary()
   {
      configure_signal.connect(boost::bind(&_binary::configure, this));
   }

   virtual bool 
   map_domain(const utilib::Any &src, utilib::Any &native, bool) const
   {
      utilib::TypeManager()->lexical_cast(src, native, typeid(domain_t));
      if (this->properties["domain_size"] != native.expose<domain_t>().size())
         EXCEPTION_MNGR(std::runtime_error, 
                        "_binary::map_domain: Domain size mismatch: "
                        "provided (" << native.expose<domain_t>().size() << 
                        ") does not match domain_size (" <<
                        this->properties["domain_size"] << ")");
      return true;
   }

private:
   void configure()
   {
      utilib::Privileged_PropertyDict &p = this->properties;
      size_t num_bin = p["num_binary_vars"];
      labels_t labels;
      for(size_t i = 0; i < num_bin; ++i)
      {
         std::stringstream ss;
         ss << "b_" << i;
         labels.insert(labels_t::value_type(i, ss.str()));
      }
      p.privilegedGet("binary_labels") = labels;
   }
};


/**
 * Define a mixed-integer domain
 */
class _mixed
   : public _continuous,
     public _integer,
     public _binary
{
public:
   typedef utilib::MixedIntVars domain_t;

   virtual bool 
   map_domain(const utilib::Any &src, utilib::Any &native, bool) const
   {
      utilib::TypeManager()->lexical_cast(src, native, typeid(domain_t));
      domain_t &mi = native.expose<domain_t>();
      if ( this->properties["num_real_vars"] != mi.Real().size()
           || this->properties["num_int_vars"] != mi.Integer().size()
           || this->properties["num_binary_vars"] != mi.Binary().size() )
         EXCEPTION_MNGR(std::runtime_error, 
                        "_mixed::map_domain: Domain size mismatch: "
                        "provided (" << mi.Binary().size() << ", " <<
                        mi.Integer().size() << ", " << mi.Real().size()
                        << ") does not match domain_size (" <<
                        this->properties["num_binary_vars"] << ", " << 
                        this->properties["num_int_vars"] << ", " << 
                        this->properties["num_real_vars"] << ")");
      return true;
   }
};



/**
 *  Define a simple objective function
 *     min f(x) = prod_i( x[i] )
 */
class _singleObj : virtual public ComposableTestApplication
{
public:
   _singleObj()
   {
      compute_map[colin::f_info] = 
         boost::bind(&_singleObj::calc_f, this, _1, _2);
      compute_map[colin::g_info] = 
         boost::bind(&_singleObj::calc_g, this, _1, _2);
      compute_map[colin::h_info] = 
         boost::bind(&_singleObj::calc_h, this, _1, _2);
   }

   utilib::Any calc_f( const utilib::Any &domain,
                       utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      double fval = 1.0;
      for (size_t i = 0; i < x.size(); i++)
         fval *= x[i];

      return fval;
   }


   utilib::Any calc_g( const utilib::Any &domain,
                       utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      std::vector<double> ans(x.size());

      for (size_t i = 0; i < x.size(); i++)
      {
         ans[i] = 1.0;
         for (size_t j = 0; j < x.size(); j++)
            if (i != j)
               ans[i] *= x[j];
      }

      colin::Domain::DifferentiableComponent dc;
      utilib::TypeManager()->lexical_cast(domain, dc);
      std::vector<double> final(dc.size());
      for(size_t i = 0; i<dc.size(); ++i)
         final[i] = ans[dc.source_index[i]];
      return final;
   }


   utilib::Any calc_h( const utilib::Any &domain,
                       utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      std::vector< std::vector<double> > ans(x.size());
      for( size_t i=0; i<x.size(); ans[i++].resize(x.size(), 1.) );

      for( size_t i = 0; i < x.size(); ++i )
      {
         ans[i][i] = 0;
         for( size_t j = 0; j < x.size(); ++j )
            for( size_t k = 0; k < x.size(); ++k )
               if (( k != i ) && ( k != j ))
                  ans[i][j] *= x[k];
      }

      colin::Domain::DifferentiableComponent dc;
      utilib::TypeManager()->lexical_cast(domain, dc);
      std::vector<std::vector<double> > final(dc.size());
      for( size_t i=0; i<dc.size(); final[i++].resize(dc.size()) );
      for(size_t i = 0; i<dc.size(); ++i)
         for(size_t j = 0; j<dc.size(); ++j)
            final[i][j] = ans[dc.source_index[i]][dc.source_index[j]];
      return final;
   }
};


/**
 *  Define a multiple objective function
 *     min f(x) = prod_i( x[i] )
 *     max f(x) = -1*sum_i( x[i]^2 )
 */
class _multiObj : virtual public ComposableTestApplication
{
public:
   _multiObj()
   {
      configure_signal.connect(boost::bind(&_multiObj::configure, this));
      compute_map[colin::mf_info] = 
         boost::bind(&_multiObj::calc_mf, this, _1, _2);
      compute_map[colin::g_info] = 
         boost::bind(&_multiObj::calc_g, this, _1, _2);
      compute_map[colin::h_info] = 
         boost::bind(&_multiObj::calc_h, this, _1, _2);
   }

private:

   void configure()
   {
      utilib::Privileged_PropertyDict &p = this->properties;
      p.privilegedGet("num_objectives") = 2;

      std::vector<colin::optimizationSense> sense(2);
      sense[0] = colin::minimization;
      sense[1] = colin::maximization;
      p.privilegedGet("sense") = sense;
   }

public:

   utilib::Any calc_mf( const utilib::Any &domain,
                       utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      std::vector<double> fval(2);
      fval[0] = 1;
      for (size_t i = 0; i < x.size(); i++)
      {
         fval[0] *= x[i];
         fval[1] -= x[i] * x[i];
      }

      return fval;
   }


   utilib::Any calc_g( const utilib::Any &domain,
                       utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      std::vector<std::vector<double> > ans(2);
      ans[0].resize(x.size());
      ans[1].resize(x.size());

      for (size_t i = 0; i < x.size(); i++)
      {
         ans[0][i] = 1.0;
         for (size_t j = 0; j < x.size(); j++)
            if (i != j)
               ans[0][i] *= x[j];

         ans[1][i] = -2.0*x[i];
      }

      colin::Domain::DifferentiableComponent dc;
      utilib::TypeManager()->lexical_cast(domain, dc);
      std::vector<std::vector<double> > final(2);
      for(size_t i=0; i<2; final[i++].resize(dc.size()));
      for(size_t i = 0; i<dc.size(); ++i)
      {
         final[0][i] = ans[0][dc.source_index[i]];
         final[1][i] = ans[1][dc.source_index[i]];
      }
      return final;
   }


   utilib::Any calc_h( const utilib::Any &domain,
                       utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      std::vector< std::vector< std::vector<double> > > ans(2);
      ans[0].resize(x.size());
      ans[1].resize(x.size());
      for( size_t i=0; i<x.size(); i++ )
      {
         ans[0][i].resize(x.size(), 1.);
         ans[1][i].resize(x.size(), 0.);
      }

      for( size_t i = 0; i < x.size(); ++i )
      {
         ans[0][i][i] = 0;
         for( size_t j = 0; j < x.size(); ++j )
            for( size_t k = 0; k < x.size(); ++k )
               if (( k != i ) && ( k != j ))
                  ans[0][i][j] *= x[k];

         ans[1][i][i] = -2;
      }

      colin::Domain::DifferentiableComponent dc;
      utilib::TypeManager()->lexical_cast(domain, dc);
      std::vector< std::vector<std::vector<double> > > final(2);
      for(size_t f=0; f<2; ++f)
      {
         final[f].resize(dc.size());
         for( size_t i=0; i<dc.size(); i++ )
            final[f][i].resize(dc.size());
      }
      for(size_t i = 0; i<dc.size(); ++i)
         for(size_t j = 0; j<dc.size(); ++j)
         {
            final[0][i][j] = ans[0][dc.source_index[i]][dc.source_index[j]];
            final[1][i][j] = ans[1][dc.source_index[i]][dc.source_index[j]];
         }
      return final;
   }
};


/**
 *  Define a small set of dense linear constraints
 *        l[0] = sum_i( x[i] )
 *        l[1] = sum_i( 2*x[i] )
 *        l[2] = sum_i( (i+1)*x[i] )
 */
class _denseLConstr : virtual public ComposableTestApplication
{
public:
   _denseLConstr()
   {
      configure_signal.connect(boost::bind(&_denseLConstr::configure, this));
      compute_map[colin::lcf_info] = 
         boost::bind(&_denseLConstr::calc_lcf, this, _1, _2);
      compute_map[colin::lcg_info] = 
         boost::bind(&_denseLConstr::calc_lcg, this, _1, _2);
   }

private:
   void configure()
   {
      utilib::Privileged_PropertyDict &p = this->properties;
      if ( p.exists("num_linear_constraints") )
      {
         p.privilegedGet("num_linear_constraints") = 3;
         std::vector<colin::real> bound(3);
         bound[0] = 0;
         bound[1] = colin::real::negative_infinity;
         bound[2] = 20;
         p.privilegedGet("linear_constraint_lower_bounds") = bound;
         bound[0] = colin::real::positive_infinity;
         bound[1] = 1;
         bound[2] = 30;
         p.privilegedGet("linear_constraint_upper_bounds") = bound;

         labels_t labels;
         labels.insert(labels_t::value_type(0, "lc[1]"));
         labels.insert(labels_t::value_type(2, "lc[3]"));
         p.privilegedGet("linear_constraint_labels") = labels;
      }
   }

public:

   utilib::Any calc_lcf( const utilib::Any &domain,
                         utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      size_t lc = this->property("num_linear_constraints");
      std::vector<double> ans(lc, 0.);

      if ( lc > 0 )
         for (unsigned int i = 0; i < x.size(); i++)
            ans[0] += x[i];

      if ( lc > 1 )
         for (unsigned int i = 0; i < x.size(); i++)
            ans[1] += 2*x[i];

      if ( lc > 2 )
         for (unsigned int i = 0; i < x.size(); i++)
            ans[2] += (i+1)*x[i];

      return ans;
   }

   utilib::Any calc_lcg( const utilib::Any &domain,
                         utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      std::vector<std::vector<double> > ans;
      size_t lc = this->property("num_linear_constraints");
      ans.resize(lc);
      for(size_t i=0; i<lc; ans[i++].resize(x.size()));

      for(size_t i=0; i<x.size(); i++)
      {
         if ( lc > 0 )
            ans[0][i] = 1;
         if ( lc > 1 )
            ans[1][i] = 2;
         if ( lc > 2 )
            ans[2][i] = i+1;
      }

      colin::Domain::DifferentiableComponent dc;
      utilib::TypeManager()->lexical_cast(domain, dc);
      std::vector<std::vector<double> > final(lc);
      for( size_t i=0; i<lc; final[i++].resize(dc.size()) );
      for(size_t i = 0; i<lc; ++i)
         for(size_t j = 0; j<dc.size(); ++j)
            final[i][j] = ans[i][dc.source_index[j]];
      return final;
   }
};


/**
 *  Define a small set of dense nonlinear constraints
 *       nl[0] = sum_i( x[i]^2 )
 *       nl[1] = sum_i( (i+1)*x[i] ) // yes, this is linear
 *       nl[2] = sum_i( x[i] )       // yes, this is linear
 */
class _denseNLConstr : virtual public ComposableTestApplication
{
public:
   _denseNLConstr()
   {
      configure_signal.connect(boost::bind(&_denseNLConstr::configure, this));
      compute_map[colin::nlcf_info] = 
         boost::bind(&_denseNLConstr::calc_nlcf, this, _1, _2);
      compute_map[colin::nlcg_info] = 
         boost::bind(&_denseNLConstr::calc_nlcg, this, _1, _2);
   }


private:
   void configure()
   {
      utilib::Privileged_PropertyDict &p = this->properties;
      if ( p.exists("num_nonlinear_constraints") )
      {
         p.privilegedGet("num_nonlinear_constraints") = 3;
         std::vector<colin::real> bound(3);
         bound[0] = 0.0;
         bound[1] = colin::real::negative_infinity;
         bound[2] = 15;
         p.privilegedGet("nonlinear_constraint_lower_bounds") = bound;
         bound[0] = colin::real::positive_infinity;
         bound[1] = 1.1;
         bound[2] = 20;
         p.privilegedGet("nonlinear_constraint_upper_bounds") = bound;

         labels_t labels;
         labels.insert(labels_t::value_type(0, "nlc[1]"));
         labels.insert(labels_t::value_type(2, "nlc[3]"));
         p.privilegedGet("nonlinear_constraint_labels") = labels;
      }
   }

public:

   utilib::Any calc_nlcf( const utilib::Any &domain,
                          utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      size_t nlc = this->property("num_nonlinear_constraints");
      std::vector<double> ans(nlc, 0.);
      
      if ( nlc > 0 )
         for (unsigned int i = 0; i < x.size(); i++)
            ans[0] += x[i] * x[i];
      
      if ( nlc > 1 )
         for (unsigned int i = 0; i < x.size(); i++)
            ans[1] += (i + 1) * x[i];
      
      if ( nlc > 2 )
         for (unsigned int i = 0; i < x.size(); i++)
            ans[2] += x[i];
      
      return ans;
   }

   utilib::Any calc_nlcg( const utilib::Any &domain,
                          utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      std::vector<std::vector<double> > ans;
      size_t nlc = this->property("num_nonlinear_constraints");
      ans.resize(nlc);
      for(size_t i=0; i<nlc; ans[i++].resize(x.size()));

      for(size_t i=0; i<x.size(); i++)
      {
         if ( nlc > 0 )
            ans[0][i] = 2.0 * x[i];
         if ( nlc > 1 )
            ans[1][i] = i + 1;
         if ( nlc > 2 )
            ans[2][i] = 1;
      }

      colin::Domain::DifferentiableComponent dc;
      utilib::TypeManager()->lexical_cast(domain, dc);
      std::vector<std::vector<double> > final(nlc);
      for( size_t i=0; i<nlc; final[i++].resize(dc.size()) );
      for(size_t i = 0; i<nlc; ++i)
         for(size_t j = 0; j<dc.size(); ++j)
            final[i][j] = ans[i][dc.source_index[j]];
      return final;
   }
};


/**
 *  Define a small set of dense nondeterministic constraints (same as NL)
 *       nd[0] = sum_i( x[i]^2 )
 *       nd[1] = sum_i( (i+1)*x[i] ) // yes, this is linear
 *       nd[2] = sum_i( x[i] )       // yes, this is linear
 */
class _denseNDConstr : virtual public ComposableTestApplication
{
public:
   _denseNDConstr()
   {
      configure_signal.connect(boost::bind(&_denseNDConstr::configure, this));
      compute_map[colin::ndcf_info] = 
         boost::bind(&_denseNDConstr::calc_ndcf, this, _1, _2);
      compute_map[colin::ndcg_info] = 
         boost::bind(&_denseNDConstr::calc_ndcg, this, _1, _2);
   }


private:
   void configure()
   {
      utilib::Privileged_PropertyDict &p = this->properties;
      if ( p.exists("num_nondeterministic_constraints") )
      {
         p.privilegedGet("num_nondeterministic_constraints") = 3;
         std::vector<colin::real> bound(3);
         bound[0] = 0.0;
         bound[1] = colin::real::negative_infinity;
         bound[2] = 15;
         p.privilegedGet("nondeterministic_constraint_lower_bounds") = bound;
         bound[0] = colin::real::positive_infinity;
         bound[1] = 1.1;
         bound[2] = 20;
         p.privilegedGet("nondeterministic_constraint_upper_bounds") = bound;
      }
   }

public:

   utilib::Any calc_ndcf( const utilib::Any &domain,
                          utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      size_t ndc = this->property("num_nondeterministic_constraints");
      std::vector<double> ans(ndc, 0.);
      
      if ( ndc > 0 )
         for (unsigned int i = 0; i < x.size(); i++)
            ans[0] += x[i] * x[i];
      
      if ( ndc > 1 )
         for (unsigned int i = 0; i < x.size(); i++)
            ans[1] += (i + 1) * x[i];
      
      if ( ndc > 2 )
         for (unsigned int i = 0; i < x.size(); i++)
            ans[2] += x[i];
      
      return ans;
   }

   utilib::Any calc_ndcg( const utilib::Any &domain,
                          utilib::seed_t &/*seed*/ )
   {
      std::vector<double> x;
      utilib::TypeManager()->lexical_cast(domain, x);

      std::vector<std::vector<double> > ans;
      size_t ndc = this->property("num_nondeterministic_constraints");
      ans.resize(ndc);
      for(size_t i=0; i<ndc; ans[i++].resize(x.size()));

      for(size_t i=0; i<x.size(); i++)
      {
         if ( ndc > 0 )
            ans[0][i] = 2.0 * x[i];
         if ( ndc > 1 )
            ans[1][i] = i + 1;
         if ( ndc > 2 )
            ans[2][i] = 1;
      }

      colin::Domain::DifferentiableComponent dc;
      utilib::TypeManager()->lexical_cast(domain, dc);
      std::vector<std::vector<double> > final(ndc);
      for( size_t i=0; i<ndc; final[i++].resize(dc.size()) );
      for(size_t i = 0; i<ndc; ++i)
         for(size_t j = 0; j<dc.size(); ++j)
            final[i][j] = ans[i][dc.source_index[j]];
      return final;
   }
};


/** Implements a single-objective problem with 3 dense constraints and a
 *  continuous domain:
 *
 *  min f(x) = prod_i( x[i] )
 *  s.t.
 *       nl[0] = sum_i( x[i]^2 )
 *       nl[1] = sum_i( (i+1)*x[i] ) // yes, this is linear
 *       nl[2] = sum_i( x[i] )       // yes, this is linear
 *        l[0] = sum_i( x[i] )
 *        l[1] = sum_i( 2*x[i] )
 *        l[2] = sum_i( (i+1)*x[i] )
 */
template<typename PROBLEM_T>
class singleObj_denseCon
   : public _continuous,
     public _singleObj,
     public _denseLConstr,
     public _denseNLConstr,
     public _denseNDConstr,
     public colin::Synchronous<colin::ConfigurableApplication<PROBLEM_T> >
{
public:
   singleObj_denseCon(size_t num_real = 3)
   {
      configure(num_real);
   }

   void configure(size_t num_real)
   {
      this->_num_real_vars = num_real;
      configure_signal();
   }
};


/** Implements a multi-objective problem with 3 dense constraints and a
 *  continuous domain:
 *
 *  min f(x) = prod_i( x[i] )
 *  max f(x) = -1*sum_i( x[i]^2 )
 *  s.t.
 *       nl[0] = sum_i( x[i]^2 )
 *       nl[1] = sum_i( (i+1)*x[i] ) // yes, this is linear
 *       nl[2] = sum_i( x[i] )       // yes, this is linear
 *        l[0] = sum_i( x[i] )
 *        l[1] = sum_i( 2*x[i] )
 *        l[2] = sum_i( (i+1)*x[i] )
 */
template<typename PROBLEM_T>
class multiObj_denseCon
   : public _continuous,
     public _multiObj,
     public _denseLConstr,
     public _denseNLConstr,
     public _denseNDConstr,
     public colin::Synchronous<colin::ConfigurableApplication<PROBLEM_T> >
{
public:
   multiObj_denseCon(size_t num_real = 3)
   {
      configure(num_real);
   }

   void configure(size_t num_real)
   {
      this->_num_real_vars = num_real;
      configure_signal();
   }
};


/** Implements a single-objective problem with 3 dense constraints and a
 *  mixed-integer domain:
 *
 *  min f(x) = prod_i( x[i] )
 *  s.t.
 *       nl[0] = sum_i( x[i]^2 )
 *       nl[1] = sum_i( (i+1)*x[i] ) // yes, this is linear
 *       nl[2] = sum_i( x[i] )       // yes, this is linear
 *        l[0] = sum_i( x[i] )
 *        l[1] = sum_i( 2*x[i] )
 *        l[2] = sum_i( (i+1)*x[i] )
 */
template<typename PROBLEM_T>
class mi_singleObj_denseCon
   : public _mixed,
     public _singleObj,
     public _denseLConstr,
     public _denseNLConstr,
     public _denseNDConstr,
     public colin::Synchronous<colin::ConfigurableApplication<PROBLEM_T> >
{
public:
   mi_singleObj_denseCon(size_t num_vars = 6)
   {
      configure(num_vars);
   }

   void configure(size_t num_vars)
   {
      size_t r = num_vars/2;
      size_t b = (num_vars - r)/2;
      size_t i = (num_vars - r - b);
      this->_num_real_vars = r;
      this->_num_int_vars = i;
      this->_num_binary_vars = b;
      configure_signal();
   }
};


/** Implements a multi-objective problem with 3 dense constraints and a
 *  continuous domain:
 *
 *  min f(x) = prod_i( x[i] )
 *  max f(x) = -1*sum_i( x[i]^2 )
 *  s.t.
 *       nl[0] = sum_i( x[i]^2 )
 *       nl[1] = sum_i( (i+1)*x[i] ) // yes, this is linear
 *       nl[2] = sum_i( x[i] )       // yes, this is linear
 *        l[0] = sum_i( x[i] )
 *        l[1] = sum_i( 2*x[i] )
 *        l[2] = sum_i( (i+1)*x[i] )
 */
template<typename PROBLEM_T>
class mi_multiObj_denseCon
   : public _mixed,
     public _multiObj,
     public _denseLConstr,
     public _denseNLConstr,
     public _denseNDConstr,
     public colin::Synchronous<colin::ConfigurableApplication<PROBLEM_T> >
{
public:
   mi_multiObj_denseCon(size_t num_vars = 6)
   {
      configure(num_vars);
   }

   void configure(size_t num_vars)
   { 
      size_t r = num_vars/2;
      size_t b = (num_vars - r)/2;
      size_t i = (num_vars - r - b);
      this->_num_real_vars = r;
      this->_num_int_vars = i;
      this->_num_binary_vars = b;
      configure_signal();
   }
};


} // namespace colin::unittest::TestApplications
} // namespace colin::unittest
} // namespace colin
