/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        NIDRProblemDescDB
//- Description:  Problem description database populated by new IDR parser
//- Owner:        David M. Gay

#ifndef NIDR_PROBLEM_DESC_DB_H
#define NIDR_PROBLEM_DESC_DB_H

#include "ProblemDescDB.hpp"
#include <nidr/nidr.h>

namespace Dakota {

class ParallelLibrary;

/// The derived input file database utilizing the new IDR parser.

/** The NIDRProblemDescDB class is derived from ProblemDescDB for use
    by the NIDR parser in processing DAKOTA input file data.
    For information on modifying the NIDR input parsing procedures,
    refer to Dakota/docs/Dev_Spec_Change.dox.  For more on the parsing
    technology, see "Specifying and Reading Program Input with NIDR"
    by David M. Gay (report SAND2008-2261P, which is available in PDF
    form as http://www.sandia.gov/~dmgay/nidr08.pdf).  Source for the
    routines declared herein is NIDRProblemDescDB.cpp, in which most
    routines are so short that a description seems unnecessary. */

class NIDRProblemDescDB: public ProblemDescDB
{
public:

  //
  //- Heading: Constructors and Destructor
  //

  /// constructor
  NIDRProblemDescDB(ParallelLibrary& parallel_lib);
  /// destructor
  ~NIDRProblemDescDB();

  //
  //- Heading: Member methods
  //

  /// parses the input file and populates the problem description
  /// database using NIDR.
  void derived_parse_inputs(const char* dakota_input_file,
			    const char* parser_options);
  /// perform any data processing that must be coordinated with DB buffer
  /// broadcasting (performed prior to broadcasting the DB buffer on rank 0
  /// and after receiving the DB buffer on other processor ranks)
  void derived_broadcast();
  /// perform any additional data post-processing
  void derived_post_process();

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static kwhandler
  /// functions in order to avoid the need for static data
  static NIDRProblemDescDB* pDDBInstance;

private:

  std::list<void*> VIL;
  static void check_variables_node(void*);

public:

  static int nerr;
  static void botch(const char *fmt, ...);
  static void check_variables(std::list<DataVariables>*);
  static void check_responses(std::list<DataResponses>*);
  static void make_variable_defaults(std::list<DataVariables>*);
  static void make_response_defaults(std::list<DataResponses>*);
  static void squawk(const char *fmt, ...);
  static void warn(const char *fmt, ...);

  //
  //- Heading: Keyword handlers called by nidr
  //

#define KWH(x) static void x(const char *keyname, Values *val, void **g, void *v)

  KWH(iface_Rlit);
  KWH(iface_false);
  KWH(iface_ilit);
  KWH(iface_pint);
  KWH(iface_lit);
  KWH(iface_start);
  KWH(iface_stop);
  KWH(iface_str);
  KWH(iface_str2D);
  KWH(iface_strL);
  KWH(iface_true);

  KWH(method_Ii);
  KWH(method_Real);
  KWH(method_Real01);
  KWH(method_RealDL);
  KWH(method_RealLlit);
  KWH(method_Realp);
  KWH(method_Realz);
  KWH(method_Ri);
  KWH(method_false);
  KWH(method_ilit2);
  KWH(method_ilit2p);
  KWH(method_int);
  KWH(method_ivec);
  KWH(method_lit);
  KWH(method_lit2);
  KWH(method_litc);
  KWH(method_liti);
  KWH(method_litp);
  KWH(method_litr);
  KWH(method_litz);
  KWH(method_nnint);
  KWH(method_nnintz);
  KWH(method_num_resplevs);
  KWH(method_piecewise);
  KWH(method_pint);
  KWH(method_pintz);
  KWH(method_resplevs);
  KWH(method_resplevs01);
  KWH(method_shint);
  KWH(method_slit2);
  KWH(method_start);
  KWH(method_stop);
  KWH(method_str);
  KWH(method_strL);
  KWH(method_true);
  KWH(method_tr_final);
  KWH(method_ushint);
  KWH(method_usharray);
  KWH(method_type);

  KWH(model_Real);
  KWH(model_RealDL);
  KWH(model_false);
  KWH(model_int);
  KWH(model_intset);
  KWH(model_lit);
  KWH(model_order);
  KWH(model_shint);
  KWH(model_start);
  KWH(model_stop);
  KWH(model_str);
  KWH(model_strL);
  KWH(model_true);
  KWH(model_type);

  KWH(resp_RealDL);
  KWH(resp_RealL);
  KWH(resp_false);
  KWH(resp_intL);
  KWH(resp_lit);
  KWH(resp_nnintz);
  KWH(resp_start);
  KWH(resp_stop);
  KWH(resp_str);
  KWH(resp_strL);
  KWH(resp_true);

  KWH(strategy_Real);
  KWH(strategy_RealL);
  KWH(strategy_int);
  KWH(strategy_lit);
  KWH(strategy_start);
  KWH(strategy_str);
  KWH(strategy_strL);
  KWH(strategy_true);

  KWH(var_RealLb);
  KWH(var_RealUb);
  KWH(var_caulbl);
  KWH(var_dauilbl);
  KWH(var_daurlbl);
  KWH(var_ceulbl);
  KWH(var_deuilbl);
  KWH(var_deurlbl);
  KWH(var_pintz);
  KWH(var_start);
  KWH(var_stop);
  KWH(var_str);
  KWH(var_strL);
  KWH(var_true);
  KWH(var_newiarray);
  KWH(var_newivec);
  KWH(var_newrvec);
  KWH(var_ivec);
  KWH(var_rvec);
  KWH(var_type);

#undef KWH
};

} // namespace Dakota

#endif
