/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        NIDRProblemDescDB
//- Description:  Problem description database populated by new IDR parser
//- Owner:        David M. Gay

#ifndef NIDR_PROBLEM_DESC_DB_H
#define NIDR_PROBLEM_DESC_DB_H

#include "ProblemDescDB.hpp"
#include <external/nidr/nidr.h>

namespace Dakota {

class ParallelLibrary;

/// The derived input file database utilizing the new IDR parser.

/** The NIDRProblemDescDB class is derived from ProblemDescDB for use
    by the NIDR parser in processing DAKOTA input file data.
    For information on modifying the NIDR input parsing procedures,
    refer to Dakota/docs/Dev_Spec_Change.dox.  For more on the parsing
    technology, see "Specifying and Reading Program Input with NIDR"
    by David M. Gay (report SAND2008-2261P, which is available in PDF
    form as http://dakota.sandia.gov/papers/nidr08.pdf).  Source for the
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
  void derived_parse_inputs(const ProgramOptions& prog_opts);
  /// perform any data processing that must be coordinated with DB buffer
  /// broadcasting (performed prior to broadcasting the DB buffer on rank 0
  /// and after receiving the DB buffer on other processor ranks)
  void derived_broadcast();
  /// perform any additional data post-processing
  void derived_post_process();

  //
  //- Heading: Data
  //

  /// Pointer to the active object instance used within the static
  /// kwhandler functions in order to avoid the need for static data.
  /// Only initialized when parsing an input file; will be NULL for
  /// cases of direct DB population only.
  static NIDRProblemDescDB* pDDBInstance;

private:

  /// List of Var_Info pointers, one per Variables instance
  std::list<void*> VIL;

  /// check a single variables node; input argument v is Var_Info*
  static void check_variables_node(void* v);
  
  /// tokenize and try to validate the presence of an analysis driver,
  /// potentially included in the linked or copied template files
  static int check_driver(const String& an_driver,
			  const StringArray& link_files,
			  const StringArray& copy_files);

public:

  /// number of parse error encountered
  static int nerr;
  /// print and error message and immediately abort
  static void botch(const char *fmt, ...);
  /// check each node in a list of DataVariables, first mapping
  /// DataVariables members back to flat NIDR arrays if needed.
  static void check_variables(std::list<DataVariables>*);
  static void check_responses(std::list<DataResponses>*);
  /// Validate user-supplied descriptors
  static void check_descriptors(const StringArray &labels);
  /// Bounds and initial point check and inferred bounds generation
  static void make_variable_defaults(std::list<DataVariables>*);
  static void make_response_defaults(std::list<DataResponses>*);
  /// print an error message and increment nerr, but continue
  static void squawk(const char *fmt, ...);
  /// print a warning
  static void warn(const char *fmt, ...);

  //
  //- Heading: Keyword handlers called by nidr
  //

#define KWH(x) static void x(const char *keyname, Values *val, void **g, void *v)

  KWH(iface_Real);
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
  KWH(iface_type);

  KWH(method_Ii);
  KWH(method_Real);
  KWH(method_Real01);
  KWH(method_RealDL);
  KWH(method_RealLlit);
  KWH(method_Realp);
  KWH(method_Realz);
  KWH(method_Ri);
  KWH(method_false);
  KWH(method_szarray);
  KWH(method_ilit2);
  KWH(method_ilit2p);
  KWH(method_int);
  KWH(method_ivec);
  KWH(method_lit);
  KWH(method_litc);
  KWH(method_liti);
  KWH(method_litp);
  KWH(method_litr);
  KWH(method_litz);
  KWH(method_order);
  KWH(method_nnint);
  KWH(method_num_resplevs);
  KWH(method_piecewise);
  KWH(method_pint);
  KWH(method_pintz);
  KWH(method_resplevs);
  KWH(method_resplevs01);
  KWH(method_shint);
  KWH(method_sizet);
  KWH(method_slit2);
  KWH(method_start);
  KWH(method_stop);
  KWH(method_str);
  KWH(method_strL);
  KWH(method_true);
  KWH(method_tr_final);
  KWH(method_type);
  KWH(method_usharray);
  KWH(method_ushint);
  KWH(method_utype);
  KWH(method_augment_utype);
  KWH(method_utype_lit);

  KWH(model_Real);
  KWH(model_RealDL);
  KWH(model_ivec);
  KWH(model_false);
  KWH(model_int);
  KWH(model_intsetm1);
  KWH(model_lit);
  KWH(model_order);
  KWH(model_pint);
  KWH(model_shint);
  KWH(model_start);
  KWH(model_stop);
  KWH(model_str);
  KWH(model_strL);
  KWH(model_true);
  KWH(model_type);
  KWH(model_utype);
  KWH(model_augment_utype);

  KWH(resp_RealDL);
  KWH(resp_RealL);
  KWH(resp_false);
  KWH(resp_intset);
  KWH(resp_ivec);
  KWH(resp_lit);
  KWH(resp_sizet);
  KWH(resp_start);
  KWH(resp_stop);
  KWH(resp_str);
  KWH(resp_strL);
  KWH(resp_true);
  KWH(resp_utype);
  KWH(resp_augment_utype);

  //KWH(env_Real);
  //KWH(env_RealL);
  KWH(env_int);
  //KWH(env_lit);
  KWH(env_start);
  KWH(env_str);
  KWH(env_strL);
  KWH(env_true);
  KWH(env_utype);
  KWH(env_augment_utype);

  KWH(var_RealLb);
  KWH(var_RealUb);
  KWH(var_IntLb);
  KWH(var_categorical);
  KWH(var_caulbl);
  KWH(var_dauilbl);
  KWH(var_dauslbl);
  KWH(var_daurlbl);
  KWH(var_ceulbl);
  KWH(var_deuilbl);
  KWH(var_deuslbl);
  KWH(var_deurlbl);
  KWH(var_pintz);
  KWH(var_start);
  KWH(var_stop);
  KWH(var_str);
  KWH(var_strL);
  KWH(var_true);
  KWH(var_newiarray);
  KWH(var_newsarray);
  KWH(var_newivec);
  KWH(var_newrvec);
  KWH(var_ivec);
  KWH(var_svec);
  KWH(var_rvec);
  KWH(var_type);

#undef KWH
};


/// Free convenience function that flatten sizes of an array of std
/// containers; takes an array of containers and returns an IntArray
/// containing the sizes of each container in the input array.  Note:
/// Did not specialize for vector<RealVector> as no current use cases.
template <class ContainerT>
inline void 
flatten_num_array(const std::vector<ContainerT>& input_array, IntArray** pia)
{
  size_t input_len = input_array.size();
  IntArray *ia;

  *pia = ia = new IntArray(input_len);
  for(size_t i = 0; i < input_len; ++i)
    (*ia)[i] = input_array[i].size();
}


} // namespace Dakota

#endif
