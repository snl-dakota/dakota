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
#include <colin/AmplApplication.h>

//#define DEBUG_AMPL_APPLICATION 1

//===========================================================================
//===========================================================================
#if !defined(ACRO_USING_AMPL) || defined(TFLOPS)
// Stub in a non-functional AmplApplication class, but DO NOT REGISTER IT.

namespace colin {
namespace {

bool RegisterAmplApplication()
{ 
   return false; 
}

} // namespace colin::(local)


/// Bogus definition of an empty AmplApplication_Core class
class AmplApplication_Core {};

/// AmplApplication disabled because COLIN compiled without AMPL support
AmplApplication::AmplApplication()
{
   EXCEPTION_MNGR(std::logic_error, "Attempting to create AmplApplication"
                  "but COLIN was compiled without AMPL support");
}

AmplApplication::~AmplApplication()
{}

//
// Bogus function definitions: they will bever be called because the
// constructor will throw an exception.
//

bool
AmplApplication::
map_domain(const utilib::Any &, utilib::Any &, bool) const
{
   return false; 
}

} // namespace colin

//===========================================================================
//===========================================================================
#else // We are building in AMPL support

#include <colin/ApplicationMngr.h>
#include <colin/real.h>

#include <utilib/BasicArray.h>
#include <utilib/MixedIntVars.h>
#include <utilib/SparseMatrix.h>
#include <utilib/TinyXML_helper.h>

#include <boost/bimap.hpp>
#include <boost/algorithm/string.hpp>

// We use this typedef to get around AMPL's "real" #define
typedef colin::real Real_t;

#undef NO
extern "C"
{
#include "asl.h"
};

/** AMPL "emulates" object-oriented programming by defining a state
*  structure (ASL* asl) and then a slew of #defines to wrap what should
*  be methods and properties of a class with the common asl state
*  pointer.  This is problemmatic for real programs where there may be
*  more than one active AMPL problem (think outer and inner problems).
*
*  The workaround for this is yet another #define: by #defining the
*  common asl pointer to be something else, when the preprocessor
*  expands AMPL's internal #defines, we can get AMPL to reference a
*  properly-scoped (and contained) ASL* member of *our* class.
*/
#define asl ASL_ptr

using std::cerr;
using std::endl;
using std::set;
using std::list;
using std::vector;
using std::make_pair;
using std::string;

using utilib::Any;
using utilib::BasicArray;
using utilib::MixedIntVars;
using utilib::RMSparseMatrix;
using utilib::seed_t;

typedef colin::AppRequest::request_map_t request_map_t;
typedef boost::bimap<size_t, string>  labels_t;

namespace colin {

namespace {

bool RegisterAmplApplication()
{
   // ------------------------------------------------------------------
   // Register creatable instances with the ApplicationMngr
   ApplicationMngr().declare_application_type<AmplApplication>("AMPL");

   return true;
}

/// A convenience structure for setting and restoring AMPL's n_conjac flags
struct ConstraintSubset {
   ConstraintSubset(int a, int b, ASL* p)
      : ASL_ptr(p),
        old_a(n_conjac[0]),
        old_b(n_conjac[1])
   {
      n_conjac[0] = a;
      n_conjac[1] = b;
   }

   ~ConstraintSubset()
   {
      n_conjac[0] = old_a;
      n_conjac[1] = old_b;
   }

   ASL* ASL_ptr;
   int old_a;
   int old_b;
};

} // namespace colin::(local)


//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

/** This defines the core relaxed application that directly interfaces
 *  with the AMPL Solver Library.  In general, end users will not create
 *  this class directly.
 */
class AmplApplication_Core : public Synchronous<Application<MO_NLP2_problem> >
{
   friend class AmplApplication;

public:

   struct ProblemInfo;

public:

   ///
   AmplApplication_Core()
      : ASL_ptr(NULL)
   {}

   ///
   virtual ~AmplApplication_Core()
   {
      if (ASL_ptr != NULL)
         ASL_free(&ASL_ptr);
   }

   ///
   utilib::Any set_nl_file(std::string nl_file_name);

   ///
   void write_sol_file(PointSet ps, std::stringstream& msg);

protected:

   /// Convert the domain supplied by the solver to the application domain
   virtual bool map_domain( const utilib::Any &src, utilib::Any &native, 
                            bool forward ) const;

   /// Actually perform the calculation(s) requested by the solver
   virtual void 
   perform_evaluation_impl( const utilib::Any &domain,
                            const AppRequest::request_map_t &requests,
                            utilib::seed_t &seed,
                            AppResponse::response_map_t &responses );
   
private:
   ///
   ASL * ASL_ptr;

   std::vector<int> lin_constr_row_start;
   std::vector<int> lin_constr_row_size;
   std::vector<int> lin_constr_col_index;
   std::vector<int> nonlin_constr_row_start;
   std::vector<int> nonlin_constr_row_size;
   std::vector<int> nonlin_constr_col_index;
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------


utilib::Any
AmplApplication_Core::
set_nl_file(std::string fname)
{
   //
   // Read *.nl file
   //
   if (ASL_ptr != NULL)
      ASL_free(&ASL_ptr);
   ASL_ptr = ASL_alloc(ASL_read_pfgh);

   ASL_ptr->i.congrd_mode = 1;	// sparse constraint gradient

   fint stub_len = (fint)fname.size();
   if ( (stub_len >= 3) && boost::iequals(fname.substr(stub_len-3), ".nl") )
      stub_len -= 3;

   FILE* nl = jac0dim(const_cast<char*>(fname.c_str()), stub_len);

   Any init_point;
   BasicArray<double> &initX = init_point.set<BasicArray<double> > ();
   initX.resize(n_var);
   X0 = initX.data();
   pfgh_read(nl, 0);

#if DEBUG_AMPL_APPLICATION
   cerr << "Ampl problem: " << endl
        << "  n_var = " << n_var << endl
        << "  nbv   = " << nbv << endl
        << "  niv   = " << niv << endl
        << "  nlvb  = " << nlvb << endl
        << "  nlvbi = " << nlvbi << endl
        << "  nlvc  = " << nlvc << endl
        << "  nlvci = " << nlvci << endl
        << "  nlvo  = " << nlvc << endl
        << "  nlvoi = " << nlvci << endl
        << "  nwv   = " << nwv << endl
        << endl
        << "  n_con = " << n_con << endl
        << "  nlc   = " << nlc << endl
        << "  nlnc  = " << nlnc << endl
      ;

   cerr << "Ampl constraint matrix:" << endl;
   int tmp_nnz = 0;
   for(int i = 0; i < n_con; ++i)
   {
      cgrad* cg = Cgrad[i];
      cerr << "  c[" << i << "]: ";
      while ( cg )
      {
         ++tmp_nnz;
         cerr << "{" << cg->varno << "->" << cg->goff << "} ";
         cg = cg->next;
      }
      cerr << endl;
   }
#endif

   // Basic dimensioning
   _num_objectives = n_obj;
   _num_real_vars = n_var;

   //
   // Setup lower and upper bounds on variables
   //
   int N;
   vector<Real_t> bound;
   double* b;

   // Split the LUv vector into lower & upper vectors
   // NB: since we are going to translate the bounds from standard
   // double to utilib::Ereal, it is OK to let AMPL give us everything
   // in a single vector and we can split it apart.
   N = n_var;
   bound.resize(N);

   b = LUv;
   for (int i = 0; i < N; ++i, b += 2)
      bound[i] = (*b == negInfinity ? Real_t::negative_infinity : Real_t(*b));
   _real_lower_bounds = bound;
   
   b = LUv + 1;
   for (int i = 0; i < N; ++i, b += 2)
      bound[i] = (*b == Infinity ? Real_t::positive_infinity : Real_t(*b));
   _real_upper_bounds = bound;


   //
   // Setup lower and upper bounds on constraints
   //
   if (nranges > 0)
   {
      EXCEPTION_MNGR(std::runtime_error, "AmplApplication::set_nl_file(): "
                     "cannot handle range constraints.");
   }
   if (lnc + nlnc > 0)
   {
      EXCEPTION_MNGR(std::runtime_error, "AmplApplication::set_nl_file(): "
                     "cannot handle network constraints.");
   }

   // NB: since we are going to translate the bounds from standard
   // double to utilib::Ereal, it is OK to let AMPL give us everything
   // in a single vector and we can split it apart.

   // general nonlinear constraints
   N = nlc - nlnc;
   bound.resize(N);
   _num_nonlinear_constraints = N;
   
   b = LUrhs;
   for (int i = 0; i < N; ++i, b += 2)
      bound[i] = (*b == negInfinity ? Real_t::negative_infinity : Real_t(*b));
   _nonlinear_constraint_lower_bounds = bound;

   b = LUrhs + 1;
   for (int i = 0; i < N; ++i, b += 2)
      bound[i] = (*b == Infinity ? Real_t::positive_infinity : Real_t(*b));
   _nonlinear_constraint_upper_bounds = bound;

   // general linear constraints
   N = n_con - nlc - lnc;
   bound.resize(N);
   _num_linear_constraints = N;
   
   b = LUrhs + 2*(nlc + lnc);
   for (int i = 0; i < N; ++i, b += 2)
      bound[i] = (*b == negInfinity ? Real_t::negative_infinity : Real_t(*b));
   _linear_constraint_lower_bounds = bound;

   b = LUrhs + 2*(nlc + lnc) + 1;
   for (int i = 0; i < N; ++i, b += 2)
      bound[i] = (*b == Infinity ? Real_t::positive_infinity : Real_t(*b));
   _linear_constraint_upper_bounds = bound;
   

   //
   // Precalculate the sparsity data for constraint gradients
   //

   // nonlinear constraints
   {
      N = nlc - nlnc;
      nonlin_constr_row_start.resize(N);
      nonlin_constr_row_size.resize(N);
      nonlin_constr_col_index.resize(nzc); // this overestimates the nnzeros
      int lastRow = 0;
      for(int i = 0; i < N; ++i)
      {
         nonlin_constr_row_start[i] = lastRow;
         cgrad* cg = Cgrad[i];
         while ( cg )
         {
            nonlin_constr_col_index[lastRow++] = cg->varno;
            cg = cg->next;
         }
         nonlin_constr_row_size[i] = lastRow - nonlin_constr_row_start[i];
      }
      nonlin_constr_col_index.resize(lastRow); // this corrects the nnzeros
   }

   // linear constraints
   {
      N = n_con - nlc - lnc;
      lin_constr_row_start.resize(N);
      lin_constr_row_size.resize(N);
      lin_constr_col_index.resize(nzc); // this overestimates the nnzeros
      int lastRow = 0;
      for(int i = 0; i < N; ++i)
      {
         lin_constr_row_start[i] = lastRow;
         cgrad* cg = Cgrad[i + nlc + lnc];
         while ( cg )
         {
            lin_constr_col_index[lastRow++] = cg->varno;
            cg = cg->next;
         }
         lin_constr_row_size[i] = lastRow - lin_constr_row_start[i];
      }
      lin_constr_col_index.resize(lastRow); // this corrects the nnzeros
   }
   

   //
   // Setup the labels
   //

   // real variables
   labels_t labels;
   for(int i = 0; i < n_var; ++i)
   {
      char* name = var_name(i);
      if ( name )
         labels.insert( labels_t::value_type(i, name) );
   }
   if ( ! labels.empty() )
      _real_labels = labels;

   // nonlinear constraints
   labels.clear();
   for(int i = 0; i < nlc - nlnc; ++i)
   {
      char* name = con_name(i);
      if ( name )
         labels.insert( labels_t::value_type(i, name) );
   }
   if ( ! labels.empty() )
      _nonlinear_constraint_labels = labels;

   // linear constraints
   labels.clear();
   for(int i = nlc + lnc; i < n_con; ++i)
   {
      char* name = con_name(i);
      if ( name )
         labels.insert( labels_t::value_type(i - nlc - lnc, name) );
   }
   if ( ! labels.empty() )
      _linear_constraint_labels = labels;

   return init_point;
}


void
AmplApplication_Core::write_sol_file(PointSet ps, std::stringstream &msg)
{
   amplflag = 1;

   size_t size = ps->size(this);
   if (size >= 1)
   {
      if (size > 1)
         msg << "WARNING: solver returned multiple solutions; "
         "AMPL will only report the first.\n";
      vector<double> final_domain;
      ps.get_point(this, final_domain);

      // Because the PointSet only contains the domain, we need to re-run
      // it through the application to get the objective.  This shouldn't
      // be too bad (the application probably has a cache).
      vector<double> obj;
      EvalMF(eval_mngr(), final_domain, obj);

      if (obj.size() == 1)
         msg << "final f = " << obj[0];
      else
         msg << "final mf = " << obj;

      //cerr << msg.str() << endl;
      //cerr << "  relaxed domain = " << final_domain << endl;
      //cerr << "  objective      = " << obj << endl;
      write_sol(const_cast<char*>(msg.str().c_str()),
                &final_domain[0], NULL, NULL);
   }
   else
   {
      msg << "WARNING: solver returned no solutions.";
      write_sol(const_cast<char*>(msg.str().c_str()),
                NULL, NULL, NULL);
   }
}


bool
AmplApplication_Core::
map_domain(const utilib::Any &src, utilib::Any &native, bool forward) const
{
   static_cast<void>(forward);
   int ans = utilib::TypeManager()->lexical_cast( src, native, 
                                                  typeid(BasicArray<double> ));
   if (native.expose<BasicArray<double> >().size() != n_var)
      EXCEPTION_MNGR(std::runtime_error, "AmplApplication_Core::map_domain(): "
                     "domain size mismatch (" << 
                     native.expose<BasicArray<double> >().size()
                     << " != " << n_var << ")");
   return ans == 0;
}


void
AmplApplication_Core::
perform_evaluation_impl( const utilib::Any &domain,
                         const AppRequest::request_map_t &requests,
                         utilib::seed_t &seed,
                         AppResponse::response_map_t &responses )
{
   if (ASL_ptr == NULL)
      EXCEPTION_MNGR(std::runtime_error, "AmplApplication_Core::"
                     "collect_evaluation(): no active interface to the AMPL "
                     "Solver Library: set_nl_file() has not been called.");

   // this application DOES NOT support random seed control....
   seed = 0;

   ///
   /// Setup point
   ///
   const BasicArray<double> &X = domain.expose<BasicArray<double> > ();
#if DEBUG_AMPL_APPLICATION
   cerr << "Ampl evaluation at X = " << X << endl;
#endif

   //
   // Compute responses
   //
   request_map_t::const_iterator curr = requests.begin();
   request_map_t::const_iterator end  = requests.end();
   for (; curr != end; ++curr)
   {
      if (curr->first == mf_info)
      {
         Any ans;
         vector<double>& tmp = ans.set< vector<double> > ();
         tmp.resize(n_obj);
         fint retcd = 0;
         for (int i = 0; i < n_obj; i++)
         {
            tmp[i] = objval(i, X.data(), &retcd);
            if (retcd != 0)
               EXCEPTION_MNGR( std::runtime_error,
                               "AmplApplication_Core::collect_evaluation(): "
                               "The AMPL function objval returned a "
                               "nonzero error state (" << retcd << ")"
                               " for objective " << i <<
                               " at point " << X );
         }
         responses.insert(make_pair(mf_info, ans));
      }

      else if (curr->first == lcf_info)
      {
         ConstraintSubset cs(nlc + lnc, n_con, ASL_ptr);

         Any ans;
         vector<double>& tmp = ans.set< vector<double> > ();
         if ((n_con - nlc - lnc) > 0)
         {
            fint retcd = 0;
            tmp.resize(n_con - nlc - lnc);
            conval(X.data(), &tmp[0], &retcd);
            if (retcd != 0)
               EXCEPTION_MNGR(std::runtime_error,
                              "AmplApplication_Core::collect_evaluation(): "
                              "The AMPL function conval returned a "
                              "nonzero error state (" << retcd << ")"
                              " for linear constraints "  <<
                              " at point " << X );
         }
         responses.insert(make_pair(lcf_info, ans));
      }

      else if (curr->first == nlcf_info)
      {
         ConstraintSubset cs(0, nlc - nlnc, ASL_ptr);

         Any ans;
         vector<double>& tmp = ans.set< vector<double> > ();
         if ((nlc - nlnc) > 0)
         {
            fint retcd = 0;
            tmp.resize(nlc - nlnc);
            conval(X.data(), &tmp[0], &retcd);
            if (retcd != 0)
               EXCEPTION_MNGR(std::runtime_error,
                              "AmplApplication_Core::collect_evaluation(): "
                              "The AMPL function conval returned a "
                              "nonzero error state (" << retcd << ")"
                              " for nonlinear constraints " << 
                              " at point " << X );
         }
         responses.insert(make_pair(nlcf_info, ans));
      }

      else if (curr->first == g_info)
      {
         Any ans;
         vector<vector<double> >& tmp = ans.set<vector<vector<double> > >();

         tmp.resize(n_obj);
         fint retcd = 0;
         for (int i = 0; i < n_obj; i++)
         {
            tmp[0].resize(X.size());
            objgrd(i, X.data(), &(tmp[i][0]), &retcd);
            if (retcd != 0)
               EXCEPTION_MNGR(std::runtime_error,
                              "AmplApplication_Core::collect_evaluation(): "
                              "The AMPL function objgrd returned a "
                              "nonzero error state (" << retcd << ")"
                              " for objective " << i <<
                              " at point " << X );
         }
         responses.insert(make_pair(g_info, ans));
      }

      else if (curr->first == nlcg_info)
      {
         Any ans;
         RMSparseMatrix<double>& tmp = ans.set<RMSparseMatrix<double> >();
         tmp.get_matbeg() << nonlin_constr_row_start;
         tmp.get_matcnt() << nonlin_constr_row_size;
         tmp.get_matind() << nonlin_constr_col_index;

         int N = nlc - nlnc;
         tmp.initialize(N, n_var, nonlin_constr_col_index.size());
         fint retcd = 0;
         for (int i = 0; i < N; i++)
         {
            congrd( i, 
                    X.data(), 
                    tmp.get_matval().data()+tmp.get_matbeg()[i], 
                    &retcd );
            if (retcd != 0)
               EXCEPTION_MNGR(std::runtime_error,
                              "AmplApplication_Core::collect_evaluation(): "
                              "The AMPL function congrd returned a "
                              "nonzero error state (" << retcd << ")"
                              " for nonlinear constraint " << i <<
                              " at point " << X );
         }
         responses.insert(make_pair(nlcg_info, ans));
      }

      else if (curr->first == lcg_info)
      {
         Any ans;
         RMSparseMatrix<double>& tmp = ans.set<RMSparseMatrix<double> >();
         tmp.get_matbeg() << lin_constr_row_start;
         tmp.get_matcnt() << lin_constr_row_size;
         tmp.get_matind() << lin_constr_col_index;

         int N = n_con - nlc - lnc;
         tmp.initialize(N, n_var, lin_constr_col_index.size());
         fint retcd = 0;
         for (int i = 0; i < N; i++)
         {
            congrd( i + nlc + lnc, 
                    X.data(), 
                    tmp.get_matval().data()+tmp.get_matbeg()[i], 
                    &retcd );
            if (retcd != 0)
               EXCEPTION_MNGR(std::runtime_error,
                              "AmplApplication_Core::collect_evaluation(): "
                              "The AMPL function congrd returned a "
                              "nonzero error state (" << retcd << ")"
                              " for linear constraint " << i <<
                              " at point " << X );
         }
         responses.insert(make_pair(lcg_info, ans));
      }

      else if (curr->first == h_info)
         EXCEPTION_MNGR(std::logic_error,
                        "AmplApplication_Core::collect_evaluation(): "
                        "computing h_info is not yet implemented.");
   }
#if DEBUG_AMPL_APPLICATION
   cerr << "Ampl returned = " << responses << endl;
#endif
}


//=======================================================================
//=======================================================================
//=======================================================================


AmplApplication::AmplApplication()
   : keepNLFile(false),
     numReal(0),
     numInteger(0),
     numBinary(0),
     EPS(1e-8)
{
   initializer("NL").connect
      (boost::bind(&AmplApplication::cb_initialize, this, _1));

   // We want to disable the "BaseProblem" initializer inherited from
   // the Reformulation class
   initializer("BaseProblem").disconnect_all_slots();

   response_transform_signal.connect
      (boost::bind( &AmplApplication::cb_map_g_response, 
                    this, g_info, _1, _2, _3, _4 ));
   response_transform_signal.connect
      (boost::bind( &AmplApplication::cb_map_g_response, 
                    this, lcg_info, _1, _2, _3, _4 ));
   response_transform_signal.connect
      (boost::bind( &AmplApplication::cb_map_g_response, 
                    this, nlcg_info, _1, _2, _3, _4 ));
   response_transform_signal.connect
      (boost::bind( &AmplApplication::cb_map_h_response, 
                    this, _1, _2, _3, _4 ));
}


AmplApplication::~AmplApplication()
{}


utilib::Any AmplApplication::set_nl_file(std::string fname)
{
   relaxed_app = ApplicationHandle::create<AmplApplication_Core>();
   Any base_init_point = relaxed_app.second->set_nl_file(fname);
   ASL* ASL_ptr = relaxed_app.second->ASL_ptr;

   //
   if (keepNLFile && (fname != "dummy"))
   {
      string cmd;
      cmd += "cp ";
      cmd += fname;
      cmd += ".nl dummy.nl";
      system(cmd.c_str());
   }

   //
   // Categorize variables
   //
   var_type.resize(n_var);
   int ctr = 0;
   numBinary = 0;
   numInteger = 0;
   numReal = 0;
   if (nlvb == -1)
      EXCEPTION_MNGR(std::runtime_error, "Cannot apply COLIN to AMPL *.nl "
                     "files generated before AMPL version 19930630");

   //   Continuous appearing nonlinearly in an objective and a constraint
   for (int i = 0; i < (nlvb - nlvbi); i++)
   {
      var_type[ctr++] = 0;
      numReal++;
   }
   //   Integer appearing nonlinearly in an objective and a constraint
   for (int i = 0; i < nlvbi; i++)
   {
      var_type[ctr++] = 1;
      numInteger++;
   }
   //   Continuous appearing nonlinearly in just constraints
   for (int i = 0; i < (nlvc - (nlvb + nlvci)); i++)
   {
      var_type[ctr++] = 0;
      numReal++;
   }
   //   Integer appearing nonlinearly in just constraints
   for (int i = 0; i < nlvci; i++)
   {
      var_type[ctr++] = 1;
      numInteger++;
   }
   //   Continuous appearing nonlinearly in just objectives
   for (int i = 0; i < (nlvo - (nlvc + nlvoi)); i++)
   {
      var_type[ctr++] = 0;
      numReal++;
   }
   //   Integer appearing nonlinearly in just objectives
   for (int i = 0; i < nlvoi; i++)
   {
      var_type[ctr++] = 1;
      numInteger++;
   }
   //   Linear Arcs
   for (int i = 0; i < nwv; i++)
   {
      var_type[ctr++] = 0;
      numReal++;
   }
   //   Other linear
   int n = n_var - (nlvc > nlvo ? nlvc : nlvo) - niv - nbv - nwv;
   for (int i = 0; i < n; i++)
   {
      var_type[ctr++] = 0;
      numReal++;
   }
   //   Binary (NB: this is only *linear* binary variables; nonlinear
   //   binaries show up as nonlinear integers)
   for (int i = 0; i < nbv; i++)
   {
      var_type[ctr++] = 2;
      numBinary++;
   }
   //   Other integer
   for (int i = 0; i < niv; i++)
   {
      var_type[ctr++] = 1;
      numInteger++;
   }

   if ( numReal + numInteger + numBinary != (size_t)n_var )
      EXCEPTION_MNGR(std::runtime_error, "AmplApplication::set_nl_file(): "
                     "something is wrong with our interpretation of "
                     "the variables.");

   size_t idx = 0;
   var_rmap.resize(numReal);
   for(ctr = 0; ctr < n_var; ++ctr)
      if ( var_type[ctr] == 0 )
         var_rmap[idx++] = ctr;

   // We need to set the dimention of the problem before referencing the
   // core application so callbacks (like setting the linear constraint
   // matrix) pass dimentionality tests.
   _num_real_vars = numReal;
   _num_int_vars = numInteger;
   _num_binary_vars = numBinary;

   // clear out any remote refereces from a previous reformulation
   properties.dereference_all();

   set<Any> exclude;
   exclude.insert(ObjectType::get<Application_RealDomain>());
   exclude.insert(ObjectType::get<Application_LinearConstraintGradients>());
   referencePropertiesFrom( relaxed_app.second, exclude, set<string>() );

   // map the variable bounds into the mixed int domain
   vector<Real_t> l = relaxed_app.second->real_lower_bounds;
   vector<Real_t> u = relaxed_app.second->real_upper_bounds;
   vector<Real_t> rl(numReal);
   vector<Real_t> ru(numReal);
   vector<int>    il(numInteger);
   vector<int>    iu(numInteger);
   size_t r = 0;
   size_t i = 0;
   size_t b = 0;
   labels_t labels = relaxed_app.second->real_labels;
   labels_t r_lbl;
   labels_t i_lbl;
   labels_t b_lbl;
   bool bounds_ok = true;
   labels_t::left_map::const_iterator noLbl = labels.left.end();
#if DEBUG_AMPL_APPLICATION
   cerr << "Ampl bound processing:" << endl;
#endif
   for (int idx = 0; idx < n_var; ++idx)
   {
      labels_t::left_map::const_iterator lbl = labels.left.find(idx);
      switch ( var_type[idx] ) {
      case 0:
         rl[r] = l[idx];
         ru[r] = u[idx];
         if ( lbl != noLbl )
            r_lbl.insert( labels_t::value_type(r, lbl->second) );
         ++r;
         break;
      case 1:
         il[i] = static_cast<int>(l[idx] < 0. ? l[idx] - 0.5 : l[idx] + 0.5);
         bounds_ok &= (fabs(static_cast<double>(il[i]) - l[idx]) < EPS);
         iu[i] = static_cast<int>(u[idx] < 0. ? u[idx] - 0.5 : u[idx] + 0.5);
         bounds_ok &= (fabs(static_cast<double>(iu[i]) - u[idx]) < EPS);
         if ( lbl != noLbl )
            i_lbl.insert( labels_t::value_type(i, lbl->second) );
         ++i;
         break;
      case 2:
         if ( lbl != noLbl )
            b_lbl.insert( labels_t::value_type(b, lbl->second) );
         ++b;
         break;
      default:
         EXCEPTION_MNGR(std::runtime_error, "AmplApplication::set_nl_file(): "
                        "invalid variable type found (" << 
                        var_type[idx] << ") for variable " << idx);
      }
#if DEBUG_AMPL_APPLICATION
      cerr << "  bound: " << bounds_ok << ": " 
           << l[idx] << " <= x[" << idx << "] <= " << u[idx] << endl;
#endif
   }
   if (! bounds_ok)
      EXCEPTION_MNGR(std::runtime_error, "AmplApplication::set_nl_file(): "
                     "non-integer bound supplied for integer variable");

   _real_lower_bounds = rl;
   _real_upper_bounds = ru;
   _int_lower_bounds = il;
   _int_upper_bounds = iu;

   _real_labels   = r_lbl;
   _int_labels    = i_lbl;
   _binary_labels = b_lbl;

   Any init_point;
   map_domain(base_init_point, init_point, false);
   return init_point;
}


void
AmplApplication::write_sol_file(PointSet ps, std::stringstream &msg)
{
   relaxed_app.second->write_sol_file(ps, msg);
}


bool
AmplApplication::
map_domain(const utilib::Any &src, utilib::Any &native, bool forward) const
{
   bool ans = true;
   if (forward)
   {
      utilib::Any tmp;
      utilib::TypeManager()->lexical_cast(src, tmp, typeid(MixedIntVars));
      const MixedIntVars &vars = tmp.expose<MixedIntVars>();
      if (vars.Real().size() != numReal)
         EXCEPTION_MNGR(std::runtime_error, "AmplApplication::map_domain(): "
                        "real domain size mismatch (" << vars.Real().size()
                        << " != " << numReal << ")");
      if (vars.Integer().size() != numInteger)
         EXCEPTION_MNGR(std::runtime_error, "AmplApplication::map_domain(): "
                        "integer domain size mismatch ("
                        << vars.Integer().size()
                        << " != " << numInteger << ")");
      if (vars.Binary().size() != numBinary)
         EXCEPTION_MNGR(std::runtime_error, "AmplApplication::map_domain(): "
                        "binary domain size mismatch (" << vars.Binary().size()
                        << " != " << numBinary << ")");

      BasicArray<double> &relaxed = native.set<BasicArray<double> > ();
      relaxed.resize(var_type.size());

      BasicArray<double>::iterator v = relaxed.begin();
      utilib::NumArray<double>::const_iterator r = vars.Real().begin();
      utilib::NumArray<int>::const_iterator i = vars.Integer().begin();
      size_t b = 0;
      vector<char>::const_iterator var = var_type.begin();
      vector<char>::const_iterator varEnd = var_type.end();
      for (; var != varEnd; ++var)
      {
         if (*var == 0)
            *(v++) = *(r++);
         else if (*var == 1)
            *(v++) = *(i++);
         else if (*var == 2)
            *(v++) = vars.Binary()[b++];
         else
            EXCEPTION_MNGR(std::runtime_error, "AmplApplication::map_domain():"
                           " invalid variable type found (" << *var << ")");
      }
   }
   else // reverse map
   {
      utilib::Any tmp;
      utilib::TypeManager()->lexical_cast
          (src, tmp, typeid(BasicArray<double>));
      const BasicArray<double> &relaxed = tmp.expose<BasicArray<double> > ();

      if (relaxed.size() != var_type.size())
         EXCEPTION_MNGR(std::runtime_error, "AmplApplication::map_domain(): "
                        "relaxed domain size mismatch (" << relaxed.size()
                        << " != " << var_type.size() << ")");

      MixedIntVars &vars = native.set<MixedIntVars>();
      vars.Real().resize(numReal);
      vars.Integer().resize(numInteger);
      vars.Binary().resize(numBinary);

      BasicArray<double>::const_iterator v = relaxed.begin();
      utilib::NumArray<double>::iterator r = vars.Real().begin();
      utilib::NumArray<int>::iterator i = vars.Integer().begin();
      size_t b = 0;
      vector<char>::const_iterator var = var_type.begin();
      vector<char>::const_iterator varEnd = var_type.end();
      for (; var != varEnd; ++var)
      { 
         if (*var == 0)
            *(r++) = *(v++);
         else if (*var == 1)
         {
            *i = static_cast<int>(*v < 0. ? *v - 0.5 : *v + 0.5);
            ans &= (fabs(static_cast<double>(*i) - *v) < EPS);
            ++i;
            ++v;
         }
         else if (*var == 2)
         {
            vars.Binary()[b] = static_cast<int>(*v < 0. ? *v - 0.5 : *v + 0.5);
            ans &= (fabs(static_cast<double>(vars.Binary()[b]) - *v) < EPS);
            ++b;
            ++v;
         }
         else
            EXCEPTION_MNGR(std::runtime_error, "AmplApplication::map_domain():"
                           " invalid variable type found (" << *var << ")");
      }
   }

   return ans;
}


int
AmplApplication::
cb_map_g_response( response_info_t info,
                   const utilib::Any &domain,
                   const AppRequest::request_map_t &requests,
                   const AppResponse::response_map_t &sub_response,
                   AppResponse::response_map_t &response )
{
   static_cast<void>(domain);
   static_cast<void>(sub_response);

   AppResponse::response_map_t::iterator g_it = response.find( info );
   if ( g_it == response.end() )
      return requests.count( info ) ? 0 : -1;

   // We need to do column swapping on the gradient returned by
   // AMPL to match the variable ordering that we are presenting
   // here (doubles then ints).
      
   // Brute force implementation...
   Any ans;
   vector<vector<double> > &local = ans.set<vector<vector<double> > >();
   Any g;
   utilib::TypeManager()->lexical_cast( g_it->second, g, 
                                        typeid(vector<vector<double> >) );
   const vector<vector<double> > &base = g.expose<vector<vector<double> > >();

   local.resize(base.size());
   for(size_t r = 0; r < base.size(); ++r)
   { 
      local[r].resize(numReal);
      double* row = &local[r][0];
      const double* src = &base[r][0];
      for(size_t var = 0; var < numReal; ++var)
         row[var] = src[var_rmap[var]];
   }

   response.erase(g_it);
   response.insert(AppResponse::response_pair_t(info, ans));
   return -1;
}


int
AmplApplication::
cb_map_h_response(const utilib::Any &domain,
                  const AppRequest::request_map_t &requests,
                  const AppResponse::response_map_t &sub_response,
                  AppResponse::response_map_t &response)
{
   static_cast<void>(domain);
   static_cast<void>(sub_response);

   AppResponse::response_map_t::iterator h_it = response.find( h_info );
   if ( h_it == response.end() )
      return requests.count( h_info ) ? 0 : -1;

   // We need to do column swapping on the hessian returned by
   // AMPL to match the variable ordering that we are presenting
   // here (doubles then ints).
   EXCEPTION_MNGR(std::runtime_error,
                  "AmplApplication::cb_map_obj_response(): "
                  "we haven't implemented Hessian mapping yet");
   return -1;
}

void AmplApplication::cb_initialize(TiXmlElement* elt)
{
   string nl_fname = "";
   utilib::get_string_attribute(elt, "file", nl_fname);
   set_nl_file(nl_fname);
}


} // namespace colin

#endif // defined(ACRO_USING_AMPL) && !defined(TFLOPS)


// Define the static initializer [false if ! defined(ACRO_USING_AMPL)]

namespace colin {

namespace StaticInitializers {

extern const volatile bool ampl = RegisterAmplApplication();

} // namespace colin::StaticInitializers

} // namespace colin
