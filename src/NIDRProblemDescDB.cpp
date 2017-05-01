/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// Class:        NIDRProblemDescDB
//- Description: Implementation code for the New IDR ProblemDescDB class.
//-              It defines the keyword handlers that yacc calls to populate
//-              the database based on the parsed input.
//- Owner:       David M. Gay
//- Checked by:
//- Version: $Id$

#include "NIDRProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "WorkdirHelper.hpp"     // for copy/link file op utilities
#include "dakota_data_util.hpp"
#include "pecos_stat_util.hpp"
#include <functional>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>
#ifdef HAVE_OPTPP
#include "globals.h"
#endif

#ifdef DAKOTA_DL_SOLVER
#ifdef _WIN32
#include "dakota_windows.h"
#define dlopen(x,y) LoadLibrary(x)
#else
#include <dlfcn.h>
#endif
#endif /*DAKOTA_DL_SOLVER*/

/// Set input to NIDR via string argument instead of input file
extern "C" void nidr_set_input_string(const char *);

extern "C" void nidr_lib_cleanup(void);

namespace Dakota {
extern ProblemDescDB *Dak_pddb;

extern "C" FILE *nidrin;
extern "C" int nidr_parse(const char*, FILE*);

/// maximum error length is roughly 100 lines at 80 char; using fixed
/// error length instead of investing in converting to vsnprintf (C++11)
const size_t NIDR_MAX_ERROR_LEN = 8192;

int NIDRProblemDescDB::nerr = 0;
NIDRProblemDescDB* NIDRProblemDescDB::pDDBInstance(NULL);


NIDRProblemDescDB::NIDRProblemDescDB(ParallelLibrary& parallel_lib):
  ProblemDescDB(BaseConstructor(), parallel_lib)
{}


NIDRProblemDescDB::~NIDRProblemDescDB()
{
#ifndef NO_NIDR_DYNLIB
  nidr_lib_cleanup(); // close any explicitly opened shared libraries
#endif
}

void NIDRProblemDescDB::botch(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char msg[NIDR_MAX_ERROR_LEN];
  std::vsprintf(msg, fmt, ap);
  va_end(ap);
  Cerr << "\nError: " << msg << ".\n";

  abort_handler(PARSE_ERROR);
}

void NIDRProblemDescDB::squawk(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char msg[NIDR_MAX_ERROR_LEN];
  std::vsprintf(msg, fmt, ap);
  va_end(ap);
  Cerr << "\nError: " << msg << ".\n";

  ++nerr;
}

void NIDRProblemDescDB::warn(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char msg[NIDR_MAX_ERROR_LEN];
  std::vsprintf(msg, fmt, ap);
  va_end(ap);
  Cerr << "\nWarning: " << msg << ".\n";
}

/** Parse the input file using the Input Deck Reader (IDR) parsing system.
    IDR populates the IDRProblemDescDB object with the input file data. */
void NIDRProblemDescDB::
derived_parse_inputs(const ProgramOptions& prog_opts)
{
  // set the pDDBInstance
  pDDBInstance = this;

  const String& dakota_input_file = prog_opts.input_file();
  const String& dakota_input_string = prog_opts.input_string();
  const String& parser_options = prog_opts.parser_options();

  // Open the dakota input file passed in and "attach" it to stdin
  // (required by nidr_parse)
  if (!dakota_input_file.empty()) {

    Cout << "Using Dakota input file '" << dakota_input_file << "'" << std::endl;
    if (dakota_input_file.size() == 1 && dakota_input_file[0] == '-')
      nidrin = stdin;
    else if( !(nidrin = std::fopen(dakota_input_file.c_str(), "r")) )
      botch("cannot open \"%s\"", dakota_input_file.c_str());

  } 
  else if (!dakota_input_string.empty()) {

    Cout << "Using provided Dakota input string" << std::endl;
    // BMA TODO: output the string contents if verbose
    nidr_set_input_string(dakota_input_string.c_str());

  }
  else {
    Cerr << "\nError: NIDR parser called with no input." << std::endl;
    abort_handler(PARSE_ERROR);
  }

  // // nidr_parse parses the input file and invokes the keyword handlers
  // // NIDR expects a null pointer if unused, not an empty string
  // const char* ptr_parse_opts = NULL;
  // if (!parser_options.empty())
  //   ptr_parse_opts = parser_options.c_str();
  FILE *dump_file = NULL;
  if (nidr_parse(parser_options.c_str(), dump_file)) {
    //Cerr << "\nErrors parsing input file." << std::endl;
    abort_handler(PARSE_ERROR); // allows proper abort in parallel case
  }
  if (nerr)
    abort_handler(PARSE_ERROR);
  if (parallel_library().command_line_run()) {
    const char *s;
    // manage dynamic solver plugins specified in the input
#ifdef DAKOTA_DL_SOLVER
    std::list<DataMethod>::iterator
      Dml  = dataMethodList.begin(),
      Dmle = dataMethodList.end();
    DataMethodRep *Dr;
    const char *s0;
    char *s1;
    size_t L;

    for(; Dml != Dmle; ++Dml) {
      Dr = Dml->dataMethodRep;
      if ((s0 = Dr->dlDetails) && *(s = s0) > ' ') {
	while(*s > ' ')
	  ++s;
	s1 = 0;
	if (*s) {
	  L = s - s0;
	  s1 = new char[L+1];
	  memcpy(s1, s0, L);
	  s1[L] = 0;
	  s0 = s1;
	}
	if (!(Dr->dlLib = dlopen(s0, RTLD_NOW))) {
#ifndef _WIN32
	  const char *se;
	  if ((se = dlerror()))
	    squawk("Cannot open dl_solver \"%s\":\n\t%s", s0, se);
	  else
#endif
	    squawk("Cannot open dl_solver \"%s\"", s0);
	}
	if (s1)
	  delete[] s1;
      }
    }
#endif
  }
}


void NIDRProblemDescDB::derived_broadcast()
{ 
  check_variables(&dataVariablesList); 
  check_responses(&dataResponsesList); 
}
// check_variables() invokes check_variables_node(), either directly or after
// some manip of interval/histogram/correlation data.  check_variables_node
// does label processing (?) followed by additional processing in
// make_variable_defaults() below, which calls BuildLabels().  Need to
// understand the reason for this step.

// Basic flow should be:
// (1) db.parse_inputs(file), db.insert_node(), db.set(), or some combination
//     only on rank 0.
// (2) perform sanity checking (once) on sets of vector inputs across all kw's,
//     but for only those vector inputs that have been provided (see 4).
//     Note: if only sanity checking (no set up), then derived_broadcast() only
//           as a pre-processor on rank 0.
// (3) send (minimal) db buffer from rank 0 to all other ranks
// (4) define (large-scale) vector defaults across all keywords for all ranks,
//     retaining sanity from 2
//
// Q: is 2 really needed, or would it be OK to perform all checks in 4?
//    Evolution: checks/defaults as part of parsing (original)
//           --> checks after parsing/mixed input and defaults after DB
//               send/receive (current)
//           --> checks/defaults after DB send/receive (proposed new)


void NIDRProblemDescDB::derived_post_process()
{
  // finish processing dataVariableList
  make_variable_defaults(&dataVariablesList);
  // finish processing dataResponsesList
  make_response_defaults(&dataResponsesList);
}


#define Squawk NIDRProblemDescDB::squawk
#define Warn NIDRProblemDescDB::warn

typedef unsigned long UL;

struct Iface_Info {
  DataInterfaceRep *di;
  DataInterface *di_handle;
};

struct Iface_mp_Rlit {
  String DataInterfaceRep::* sp;
  RealVector DataInterfaceRep::* rv;
  const char *lit;
};

struct Iface_mp_ilit {
  String DataInterfaceRep::* sp;
  int DataInterfaceRep::* iv;
  const char *lit;
};

struct Iface_mp_lit {
  String DataInterfaceRep::* sp;
  const char *lit;
};

struct Iface_mp_type {
  short DataInterfaceRep::* sp;
  short type;
};

struct Iface_mp_utype {
  unsigned short DataInterfaceRep::* sp;
  unsigned short type;
};

struct Meth_Info {
  DataMethodRep *dme;
  DataMethod *dme0;
};

struct Method_mp_ilit2 {
  String DataMethodRep::* sp;
  int DataMethodRep::* ip;
  const char *lit;
};

struct Method_mp_ilit2z {
  String DataMethodRep::* sp;
  size_t DataMethodRep::* ip;
  const char *lit;
};

struct Method_mp_lit {
  String DataMethodRep::* sp;
  const char *lit;
};

struct Method_mp_litc {
  String DataMethodRep::* sp;
  Real DataMethodRep::* rp;
  const char *lit;
};

struct Method_mp_litrv {
  String DataMethodRep::* sp;
  RealVector DataMethodRep::* rp;
  const char *lit;
};

struct Method_mp_slit2 {
  String DataMethodRep::* sp;
  String DataMethodRep::* sp2;
  const char *lit;
};

struct Method_mp_utype_lit {
  unsigned short DataMethodRep::* ip;
  String DataMethodRep::* sp;
  unsigned short utype;
};

struct Method_mp_ord {
  short DataMethodRep::* sp;
  int ord;
};

struct Method_mp_type {
  short DataMethodRep::* ip;
  short type;
};

struct Method_mp_utype {
  unsigned short DataMethodRep::* ip;
  unsigned short utype;
};

struct Mod_Info {
  DataModelRep *dmo;
  DataModel *dmo0;
};

struct Model_mp_lit {
  String DataModelRep::* sp;
  const char *lit;
};

struct Model_mp_ord {
  short DataModelRep::* sp;
  int ord;
};

struct Model_mp_type {
  short DataModelRep::* sp;
  short type;
};

struct Model_mp_utype {
  unsigned short DataModelRep::* sp;
  unsigned short utype;
};

struct Resp_Info {
  DataResponsesRep *dr;
  DataResponses *dr0;
};

struct Resp_mp_lit {
  String DataResponsesRep::* sp;
  const char *lit;
};

struct Resp_mp_utype {
  unsigned short DataResponsesRep::* sp;
  unsigned short utype;
};

//struct Env_mp_lit {
//  String DataEnvironmentRep::* sp;
//  const char *lit;
//};

struct Env_mp_utype {
  unsigned short DataEnvironmentRep::* sp;
  unsigned short utype;
};

enum { // kinds of continuous aleatory uncertain variables
  CAUVar_normal = 0,
  CAUVar_lognormal = 1,
  CAUVar_uniform = 2,
  CAUVar_loguniform = 3,
  CAUVar_triangular = 4,
  CAUVar_exponential = 5,
  CAUVar_beta = 6,
  CAUVar_gamma = 7,
  CAUVar_gumbel = 8,
  CAUVar_frechet = 9,
  CAUVar_weibull = 10,
  CAUVar_histogram_bin = 11,
  CAUVar_Nkinds = 12	// number of kinds of cauv variables
};

enum { // kinds of discrete aleatory uncertain integer variables
  DAUIVar_poisson = 0,
  DAUIVar_binomial = 1,
  DAUIVar_negative_binomial = 2,
  DAUIVar_geometric = 3,
  DAUIVar_hypergeometric = 4,
  DAUIVar_histogram_point_int = 5,
  DAUIVar_Nkinds = 6	// number of kinds of dauiv variables
};

enum { // kinds of discrete aleatory uncertain string variables
  DAUSVar_histogram_point_str = 0,
  DAUSVar_Nkinds = 1	// number of kinds of dausv variables
};

enum { // kinds of discrete aleatory uncertain real variables
  DAURVar_histogram_point_real = 0,
  DAURVar_Nkinds = 1	// number of kinds of daurv variables
};

enum { // kinds of continuous epistemic uncertain variables
  CEUVar_interval = 0,
  CEUVar_Nkinds = 1	// number of kinds of cauv variables
};

enum { // kinds of discrete epistemic uncertain integer variables
  DEUIVar_interval = 0,
  DEUIVar_set_int = 1,
  DEUIVar_Nkinds = 2	// number of kinds of deuiv variables
};

enum { // kinds of discrete epistemic uncertain string variables
  DEUSVar_set_str = 0,
  DEUSVar_Nkinds = 1	// number of kinds of deusv variables
};

enum { // kinds of discrete epistemic uncertain real variables
  DEURVar_set_real = 0,
  DEURVar_Nkinds = 1	// number of kinds of deurv variables
};

enum { // kinds of discrete set variables
  DiscSetVar_design_set_int = 0,
  DiscSetVar_design_set_str = 1,
  DiscSetVar_design_set_real = 2,
  DiscSetVar_state_set_int = 3,
  DiscSetVar_state_set_str = 4,
  DiscSetVar_state_set_real = 5,
  DiscSetVar_Nkinds = 6	// number of kinds of discrete set variables
};

struct VarLabel {
  size_t n;
  const char **s;
};

struct Var_Info {
  DataVariablesRep *dv;
  DataVariables    *dv_handle;
  VarLabel  CAUv[ CAUVar_Nkinds],  CEUv[ CEUVar_Nkinds];
  VarLabel DAUIv[DAUIVar_Nkinds], DAUSv[DAUSVar_Nkinds], DAURv[DAURVar_Nkinds];
  VarLabel DEUIv[DEUIVar_Nkinds], DEUSv[DEUSVar_Nkinds], DEURv[DEURVar_Nkinds];
  IntArray   *nddsi, *nddss, *nddsr, *nCI, *nDI, *nhbp,
             *nhpip, *nhpsp, *nhprp, 
             *ndusi, *nduss, *ndusr,
             *ndssi, *ndsss, *ndssr;
             
  RealVector *ddsr, *CIlb, *CIub, *CIp, *DIp, *DSIp, *DSSp, *DSRp, *dusr,
             *hba, *hbo, *hbc, 
             *hpic, *hpsc, *hpra, *hprc,
             *ucm, *dssr;
  IntVector  *ddsi, *DIlb, *DIub, *hpia, *dusi, *dssi, *ddsia, *ddssa, *ddsra;
  StringArray *ddss, *hpsa, *duss, *dsss;
};

struct Var_check
{
  const char *name;
  size_t DataVariablesRep::* n;
  void (*vgen)(DataVariablesRep*, size_t);
};

/// structure for verifying bounds and initial point for real-valued vars
struct Var_rcheck
{
  const char *name;
  size_t DataVariablesRep::* n;
  void (*vgen)(DataVariablesRep*, size_t);
  RealVector  DataVariablesRep::* L;   // when static, initialized to NULL
  RealVector  DataVariablesRep::* U;   // when static, initialized to NULL
  RealVector  DataVariablesRep::* V;   // when static, initialized to NULL
  StringArray DataVariablesRep::* Lbl; // when static, initialized to NULL
};

/// structure for verifying bounds and initial point for string-valued vars
// struct Var_scheck
// {
//   const char *name;
//   size_t DataVariablesRep::* n;
//   void (*vgen)(DataVariablesRep*, size_t);
//   StringArray DataVariablesRep::* L;   // when static, initialized to NULL
//   StringArray DataVariablesRep::* U;   // when static, initialized to NULL
//   StringArray DataVariablesRep::* V;   // when static, initialized to NULL
//   StringArray DataVariablesRep::* Lbl; // when static, initialized to NULL
// };

/// structure for verifying bounds and initial point for integer-valued vars
struct Var_icheck
{
  const char *name;
  size_t DataVariablesRep::* n;
  void (*vgen)(DataVariablesRep*, size_t);
  IntVector   DataVariablesRep::* L;   // when static, initialized to NULL
  IntVector   DataVariablesRep::* U;   // when static, initialized to NULL
  IntVector   DataVariablesRep::* V;   // when static, initialized to NULL
  StringArray DataVariablesRep::* Lbl; // when static, initialized to NULL
};

struct Var_uinfo {
  const char *lbl;
  const char *vkind;
  size_t DataVariablesRep::* n;
  void(*vchk)(DataVariablesRep*, size_t, Var_Info*);
};

struct Var_brv {
  RealVector DataVariablesRep::* rv;
  Real b;
};

struct Var_biv {
  IntVector DataVariablesRep::* iv;
  int b;
};

struct Var_mp_type {
  short DataVariablesRep::* sp;
  short type;
};

void NIDRProblemDescDB::
iface_Real(const char *keyname, Values *val, void **g, void *v)
{
  (*(Iface_Info**)g)->di->**(Real DataInterfaceRep::**)v = *val->r;
}

void NIDRProblemDescDB::
iface_Rlit(const char *keyname, Values *val, void **g, void *v)
{
  DataInterfaceRep *di = (*(Iface_Info**)g)->di;
  Iface_mp_Rlit *R = (Iface_mp_Rlit*)v;
  Real *r;
  RealVector *rv;
  size_t i, n;

  di->*R->sp = R->lit;
  rv = &(di->*R->rv);
  n = val->n;
  rv->sizeUninitialized(n);
  r = val->r;
  for(i = 0; i < n; ++i)
    (*rv)[i] = r[i];
}

void NIDRProblemDescDB::
iface_false(const char *keyname, Values *val, void **g, void *v)
{
  (*(Iface_Info**)g)->di->**(bool DataInterfaceRep::**)v = false;
}

void NIDRProblemDescDB::
iface_ilit(const char *keyname, Values *val, void **g, void *v)
{
  DataInterfaceRep *di = (*(Iface_Info**)g)->di;
  Iface_mp_ilit *I = (Iface_mp_ilit*)v;

  di->*I->sp = I->lit;
  di->*I->iv = *val->i;
}

void NIDRProblemDescDB::
iface_pint(const char *keyname, Values *val, void **g, void *v)
{
  int n = *val->i;
#ifdef REDUNDANT_INT_CHECKS
  if (n <= 0) /* now handled by INTEGER > 0 in the .nspec file */
    botch("%s must be positive", keyname);
#endif
  (*(Iface_Info**)g)->di->**(int DataInterfaceRep::**)v = n;
}

void NIDRProblemDescDB::
iface_lit(const char *keyname, Values *val, void **g, void *v)
{
  (*(Iface_Info**)g)->di->*((Iface_mp_lit*)v)->sp = ((Iface_mp_lit*)v)->lit;
}

void NIDRProblemDescDB::
iface_start(const char *keyname, Values *val, void **g, void *v)
{
  Iface_Info *ii;

  if (!(ii = new Iface_Info))
  Botch:		botch("new failure in iface_start");
  if (!(ii->di_handle = new DataInterface))
    goto Botch;
  ii->di = ii->di_handle->dataIfaceRep;
  *g = (void*)ii;
}

void NIDRProblemDescDB::
iface_true(const char *keyname, Values *val, void **g, void *v)
{
  (*(Iface_Info**)g)->di->**(bool DataInterfaceRep::**)v = true;
}


#ifdef DEBUG_LEGACY_WORKDIR

extern const char** arg_list_adjust(const char **, void **);

/*
 *  not_executable(const char *driver_name) checks whether driver_name is an 
 *  executable file appearing somewhere in $PATH and returns 0 if so,
 *  1 if not found, 2 if found but not executable.
 */
int not_executable(const char *driver_name, const char *tdir)
{
  static const char *p0;
  struct stat sb;
  const char *p;
  const char *p1;
  char *b, buf[2048];
  const char *a2[2], **al;
  int rc, sv;
  size_t clen, dlen, plen, tlen;
  void *a0;
  std::string cwd = boost::filesystem::current_path().string();

#ifdef _WIN32
  char dbuf[128];
#else
  static uid_t myuid;
  static gid_t mygid;
#endif

  /* allow shell assignments and quotes around executable names */
  /* that may involve blanks */
  a2[0] = driver_name;
  a2[1] = 0;
  al = arg_list_adjust(a2,&a0);
  driver_name = al[0];

  rc = 0;
  if (!p0) {
    p0 = std::getenv("PATH");
#ifdef _WIN32
    if (!p0)
      p0 = std::getenv("Path");
#else
    myuid = geteuid();
    mygid = getegid();
#endif
    if (p0)
      while(*p0 <= ' ' && *p0)
	++p0;
    else
      p0 = "";
  }
#ifdef _WIN32
  // make sure we have a suitable suffix
  if ((p = strrchr(driver_name, '.'))) {
    if (std::strlen(++p) != 3)
      p = 0;
    else {
      for(b = dbuf; *p; ++b, ++p)
	*b = tolower(*p);
      *b = 0;
      if (std::strcmp(dbuf, "exe") && std::strcmp(dbuf, "bat") &&
	  std::strcmp(dbuf, "cmd"))
	p = 0;
    }
  }
  if (!p) {
    dlen = std::strlen(driver_name);
    if (dlen + 5 > sizeof(dbuf)) {
      rc = 1;
      goto ret;
    }
    std::strcpy(dbuf, driver_name);
    std::strcpy(dbuf+dlen, ".exe");
    driver_name = dbuf;
  }

  // . is always implicitly in $Path under MS Windows; check it now
  if (!stat(driver_name, &sb))
    goto ret;
#endif

  cwd = boost::filesystem::current_path().string();
  clen = cwd.size();
  dlen = std::strlen(driver_name);
  tlen = std::strlen(tdir);
  rc = 1;
  p = p0;
  if (std::strchr(driver_name, '/')
#ifdef _WIN32
      || std::strchr(driver_name, '\\')
      || (dlen > 2 && driver_name[1] == ':')
#endif
      )
    p = "";

  else if (clen + dlen + 2 < sizeof(buf)) {
    std::memcpy(buf,cwd.c_str(),clen);
    buf[clen] = '/';
    std::strcpy(buf+clen+1, driver_name);
    sv = stat(buf,&sb);
    if (sv == 0)
      goto stat_check;
  }
  else if (tdir && *tdir && tlen + dlen + 2 < sizeof(buf)) {
    std::memcpy(buf,tdir,tlen);
    buf[tlen] = '/';
    std::strcpy(buf+tlen+1, driver_name);
    sv = stat(buf,&sb);
    if (sv == 0)
      goto stat_check;
  }
  for(;;) {
    for(p1 = p;; ++p1) {
      switch(*p1) {
      case 0:
#ifdef _WIN32
      case ';':
#else
      case ':':
#endif
	goto break2;
      }
    }
  break2:
    if (p1 == p || (p1 == p + 1 && *p == '.'))
      sv = stat(driver_name, &sb);
    else {
      plen = p1 - p;
      while(plen && p[plen-1] <= ' ')
	--plen;
      if (plen + dlen + 2 > sizeof(buf))
	sv = 1;
      else {
	std::strncpy(buf,p,plen);
	b = buf + plen;
	*b++ = '/';
	std::strcpy(b, driver_name);
	sv = stat(buf, &sb);
      }
    }
    if (!sv) {
    stat_check:
#ifdef __CYGWIN__
      rc = 2;
      if (sb.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) {
	rc = 0;
	goto ret;
      }
#elif defined(_WIN32) || defined(_WIN64)
      rc = 0;
      goto ret;
#else
      rc = 2;
      if (sb.st_uid == myuid) {
	if (sb.st_mode & S_IXUSR)
	  goto ret0;
      }
      else if (sb.st_gid == mygid) {
	if (sb.st_mode & S_IXGRP)
	  goto ret0;
      }
      else if (sb.st_mode & S_IXOTH) {
      ret0:                          rc = 0;
	goto ret;
      }
#endif
    }
    if (p1 == 0)
      goto ret;
    else if (!*p1)
      break;
    for(p = p1 + 1; *p <= ' '; ++p)
      while(*p <= ' ' && *p)
	if (!*p)
	  goto ret;
  }
 ret:
  if (a0)
    std::free(a0);
  return rc;
}

#endif  // DEBUG_LEGACY_WORKDIR


void NIDRProblemDescDB::
iface_stop(const char *keyname, Values *val, void **g, void *v)
{
  Iface_Info *ii = *(Iface_Info**)g;
  DataInterfaceRep *di = ii->di;

  StringArray& analysis_drivers = di->analysisDrivers;
  int nd = analysis_drivers.size();
  int ac = di->asynchLocalAnalysisConcurrency;
  int ec = di->asynchLocalEvalConcurrency;

  if (di->algebraicMappings == "" && nd == 0)
    squawk("interface specification must provide algebraic_mappings,\n\t"
	   "analysis_drivers, or both");
  if (nd > 0 && di->asynchLocalAnalysisConcurrency > nd) {
    warn("analysis_concurrency specification greater than length of\n\t"
	 "analysis_drivers list.  Truncating analysis_concurrency to %d",nd);
    di->asynchLocalAnalysisConcurrency = nd;
  }
  if (ec && ec < 2 && ac && ac < 2) {
    warn("asynchronous option not required for evaluation and analysis.\n\t"
	 "Concurrency limited to %d and %d.\n\t"
	 "Synchronous operations will be used", ec, ac);
    di->interfaceSynchronization = SYNCHRONOUS_INTERFACE;
  }

  // validate each of the analysis_drivers
  if ( di->interfaceType == SYSTEM_INTERFACE || 
       di->interfaceType == FORK_INTERFACE )
    for(size_t i = 0; i < nd; ++i) {
      // trim any leading whitespace from the driver, in place
      boost::trim(analysis_drivers[i]);
      check_driver(analysis_drivers[i], di->linkFiles, di->copyFiles);
    }

  if (!di->workDir.empty()) {

#if defined(_WIN32) || defined(_WIN64)
    // Note: some Windows versions may support symlinks, if files and
    // directories are managed separately.
    if (!di->linkFiles.empty()) {
      Cerr << "\nError: link_files not supported on Windows; use copy_files."
	   << std::endl;
      ++nerr;
    }
#endif

  // Check to make sure none of the linkFiles nor copyFiles are the
  // same as the workDir (could combine into single loop with above)
    if (WorkdirHelper::check_equivalent_dest(di->linkFiles, di->workDir) ||
	WorkdirHelper::check_equivalent_dest(di->copyFiles, di->workDir))
      ++nerr;
  }

  pDDBInstance->dataInterfaceList.push_back(*ii->di_handle);
  delete ii->di_handle;
  delete ii;
}

/** returns 1 if not found, 2 if found, but not executable, 0 if found (no error) in case we want to return to error on not found... */
int NIDRProblemDescDB::check_driver(const String& an_driver,
				    const StringArray& link_files,
				    const StringArray& copy_files)
{
  StringArray driver_and_args = WorkdirHelper::tokenize_driver(an_driver);
  if (driver_and_args.size() == 0)
    squawk("Empty analysis_driver string");
  else {

    // the executable program name to check
    const String& program_name = driver_and_args[0];

    if (program_name.empty())
      squawk("Empty analysis_driver string");
    else {

      // Drivers can be found in $PATH:WORKDIR(.):RUNDIR
      // Therefore have to check PATH, link/copy files, PWD

      // check PATH and RUNDIR (since . is already on the search path)
      bfs::path driver_found = WorkdirHelper::which(program_name);
      if ( !driver_found.empty() )
	return 0;

      // check against link/copy files that will appear in workdir

      // TODO: if they are specified with ./subdirA/subdir1/foo.sh
      // would have been found above which might be an error if subdir
      // is not linked or copied file

      if (WorkdirHelper::find_driver(link_files, program_name))
	return 0;

      if (WorkdirHelper::find_driver(copy_files, program_name))
	return 0;

      const char* s = program_name.c_str();
      warn("analysis driver \"%s\" %s", s, "not found");

      // BMA TODO: check whether the driver is executable and if not, return 2
      // : "exists but is not executable");

    }

  }

  return 1;
}

void NIDRProblemDescDB::
iface_str(const char *keyname, Values *val, void **g, void *v)
{
  (*(Iface_Info**)g)->di->**(String DataInterfaceRep::**)v = *val->s;
}

void NIDRProblemDescDB::
iface_str2D(const char *keyname, Values *val, void **g, void *v)
{
  DataInterfaceRep *di = (*(Iface_Info**)g)->di;
  String2DArray *sa2 = &(di->**(String2DArray DataInterfaceRep::**)v);
  StringArray *sa;
  const char **s = val->s;
  size_t i, j, k, n, nc, nd;

  // This is for analysisComponents -- only String2DArray in a DataInterfaceRep

  nd = di->analysisDrivers.size();
  n = val->n;
  if (nd <= 0)
    botch("num_drivers = %d in iface_str2D", (int)nd);
  if (n % nd) {
    squawk("number of analysis_components not evenly divisible "
	   "by number of analysis_drivers");
    return;
  }
  nc = n / nd;
  sa2->resize(nd);
  for(i = k = 0; i < nd; i++) {
    sa = &((*sa2)[i]);
    sa->resize(nc);
    for(j = 0; j < nc; ++j, ++k)
      (*sa)[j] = s[k];
  }
}

void NIDRProblemDescDB::
iface_strL(const char *keyname, Values *val, void **g, void *v)
{
  StringArray *sa
    = &((*(Iface_Info**)g)->di->**(StringArray DataInterfaceRep::**)v);
  const char **s = val->s;
  size_t i, n = val->n;

  sa->resize(n);
  for(i = 0; i < n; i++)
    (*sa)[i] = s[i];
}

void NIDRProblemDescDB::
iface_type(const char *keyname, Values *val, void **g, void *v)
{
  (*(Iface_Info**)g)->di->*((Iface_mp_type*)v)->sp = ((Iface_mp_type*)v)->type;
}

void NIDRProblemDescDB::
method_Real(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->**(Real DataMethodRep::**)v = *val->r;
}

void NIDRProblemDescDB::
method_Real01(const char *keyname, Values *val, void **g, void *v)
{
  Real t = *val->r;
  if (t < 0. || t > 1.)
    botch("%s must be in [0, 1]", keyname);
  (*(Meth_Info**)g)->dme->**(Real DataMethodRep::**)v = t;
}

void NIDRProblemDescDB::
method_RealDL(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  RealVector DataMethodRep::* sa = *(RealVector DataMethodRep::**)v;
  Real *r = val->r;
  size_t i, n = val->n;

  (dm->*sa).sizeUninitialized(n);
  for(i = 0; i < n; i++)
    (dm->*sa)[i] = r[i];
}

void NIDRProblemDescDB::
method_RealLlit(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  Real *r;
  RealVector *rv = &(dm->*((Method_mp_litrv*)v)->rp);
  size_t i, n;
  dm->*((Method_mp_litrv*)v)->sp = ((Method_mp_litrv*)v)->lit;
  r = val->r;
  n = val->n;
  rv->resize(n);
  for(i = 0; i < n; i++)
    (*rv)[i] = r[i];
}

void NIDRProblemDescDB::
method_Realp(const char *keyname, Values *val, void **g, void *v)
{
  Real t = *val->r;
  if (t <= 0.)
    botch("%s must be positive", keyname);
  (*(Meth_Info**)g)->dme->**(Real DataMethodRep::**)v = t;
}

void NIDRProblemDescDB::
method_Realz(const char *keyname, Values *val, void **g, void *v)
{
  Real t = *val->r;
  if (t < 0.)
    botch("%s must be nonnegative", keyname);
  (*(Meth_Info**)g)->dme->**(Real DataMethodRep::**)v = t;
}

// MSE: This function just sets two values for one keyword.
void NIDRProblemDescDB::
method_piecewise(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  dm->expansionType  = STD_UNIFORM_U;
  dm->piecewiseBasis = true;
}

void NIDRProblemDescDB::
method_false(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->**(bool DataMethodRep::**)v = false;
}

void NIDRProblemDescDB::
method_int(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->**(int DataMethodRep::**)v = *val->i;
}

void NIDRProblemDescDB::
method_ivec(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  int *z = val->i;
  IntVector *iv = &(dm->**(IntVector DataMethodRep::**)v);
  size_t i, n = val->n;

  iv->resize(n);
  for(i = 0; i < n; i++)
    (*iv)[i] = z[i];
}

void NIDRProblemDescDB::
method_ilit2(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  dm->*((Method_mp_ilit2*)v)->sp = ((Method_mp_ilit2*)v)->lit;
  dm->*((Method_mp_ilit2*)v)->ip = *val->i;
}

void NIDRProblemDescDB::
method_ilit2p(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  dm->*((Method_mp_ilit2z*)v)->sp = ((Method_mp_ilit2z*)v)->lit;
  if ((dm->*((Method_mp_ilit2z*)v)->ip = *val->i) <= 0)
    botch("%s must be positive", keyname);
}

void NIDRProblemDescDB::
method_lit(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->*((Method_mp_lit*)v)->sp = ((Method_mp_lit*)v)->lit;
}

void NIDRProblemDescDB::
method_litc(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  if (dm->*((Method_mp_litc*)v)->rp > 0.)
    dm->*((Method_mp_litc*)v)->sp = ((Method_mp_litc*)v)->lit;
}

void NIDRProblemDescDB::
method_litp(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  Real t = *val->r;
  if (t <= 0.)
    botch("%s must be positive",keyname);
  dm->*((Method_mp_litc*)v)->sp = ((Method_mp_litc*)v)->lit;
  dm->*((Method_mp_litc*)v)->rp = t;
}

void NIDRProblemDescDB::
method_litz(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  Real t = *val->r;
  if (t < 0.)
    botch("%s must be nonnegative",keyname);
  if ((dm->*((Method_mp_litc*)v)->rp = t) == 0.)
    dm->*((Method_mp_litc*)v)->sp = ((Method_mp_litc*)v)->lit;
}

void NIDRProblemDescDB::
method_order(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->*((Method_mp_ord*)v)->sp = ((Method_mp_ord*)v)->ord;
}

void NIDRProblemDescDB::
method_nnint(const char *keyname, Values *val, void **g, void *v)
{
  int n = *val->i;
#ifdef REDUNDANT_INT_CHECKS
  if (n < 0) /* now handled by INTEGER >= 0 in the .nspec file */
    botch("%s must be non-negative", keyname);
#endif
  (*(Meth_Info**)g)->dme->**(int DataMethodRep::**)v = n;
}

void NIDRProblemDescDB::
method_sizet(const char *keyname, Values *val, void **g, void *v)
{
  int n = *val->i; // test value as int, prior to storage as size_t
#ifdef REDUNDANT_INT_CHECKS
  if (n < 0) /* now handled by INTEGER >= 0 in the .nspec file */
    botch("%s must be non-negative", keyname);
#endif
  (*(Meth_Info**)g)->dme->**(size_t DataMethodRep::**)v = n;
}

void NIDRProblemDescDB::
method_num_resplevs(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  Real *r;
  RealVectorArray *rva = &(dm->**(RealVectorArray DataMethodRep::**)v);
  RealVector *ra = &(*rva)[0];
  int m, *z = val->i;
  size_t i, i1, j, je, k, n;

  n = val->n;
  for(i = k = 0; i < n; ++i)
    k += z[i];
  if (ra->length() != (int)k)
    botch("number of %s = %u does not match\n"
	  "%s specification of %u response levels",
	  keyname+4, (Uint)ra->length(), keyname, (Uint)k);
  r = new Real[k];
  for(i = 0; i < k; i++)
    r[i] = (*ra)[i];
  (*rva).resize(n);
  for(i = j = je = 0; i < n; i++) {
    m = z[i];
    (*rva)[i].resize(m);
    ra = &(*rva)[i];
    for(i1 = 0, je += m; j < je; ++i1, ++j)
      (*ra)[i1] = r[j];
  }
  delete[] r;
}

void NIDRProblemDescDB::
method_pint(const char *keyname, Values *val, void **g, void *v)
{
  int n = *val->i;
#ifdef REDUNDANT_INT_CHECKS
  if (n <= 0) /* now handled by INTEGER > 0 in the .nspec file */
    botch("%s must be positive", keyname);
#endif
  (*(Meth_Info**)g)->dme->**(int DataMethodRep::**)v = n;
}

void NIDRProblemDescDB::
method_pintz(const char *keyname, Values *val, void **g, void *v)
{
  int n = *val->i; // test value as int, prior to storage as size_t
#ifdef REDUNDANT_INT_CHECKS
  if (n <= 0) /* now handled by INTEGER > 0 in the .nspec file */
    botch("%s must be positive", keyname);
#endif
  (*(Meth_Info**)g)->dme->**(size_t DataMethodRep::**)v = n;
}

void NIDRProblemDescDB::
method_resplevs(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  Real *r = val->r;
  RealVector *ra;
  RealVectorArray *rva = &(dm->**(RealVectorArray DataMethodRep::**)v);
  size_t i, n = val->n;

  (*rva).resize(1);
  ra = &(*rva)[0];
  ra->resize(n);
  for(i = 0; i < n; ++i)
    (*ra)[i] = r[i];
}

void NIDRProblemDescDB::
method_resplevs01(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;
  Real *r = val->r, t;
  RealVector *ra;
  RealVectorArray *rva = &(dm->**(RealVectorArray DataMethodRep::**)v);
  size_t i, n = val->n;

  (*rva).resize(1);
  ra = &(*rva)[0];
  ra->resize(n);
  for(i = 0; i < n; ++i) {
    (*ra)[i] = t = r[i];
    if (t < 0. || t > 1.)
      botch("%s must be between 0 and 1", keyname);
  }
}

void NIDRProblemDescDB::
method_shint(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->**(short DataMethodRep::**)v = *val->i;
}

void NIDRProblemDescDB::
method_ushint(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->**(unsigned short DataMethodRep::**)v = *val->i;
}

void NIDRProblemDescDB::
method_usharray(const char *keyname, Values *val, void **g, void *v)
{
  UShortArray *usa
    = &((*(Meth_Info**)g)->dme->**(UShortArray DataMethodRep::**)v);
  int *z = val->i;
  size_t i, n = val->n;

  usa->resize(n);
  for (i=0; i<n; ++i)
    if (z[i] >= 0)
      (*usa)[i] = z[i];
    else
      botch("%s must have non-negative values", keyname);
}

void NIDRProblemDescDB::
method_szarray(const char *keyname, Values *val, void **g, void *v)
{
  SizetArray *sza
    = &((*(Meth_Info**)g)->dme->**(SizetArray DataMethodRep::**)v);
  int *z = val->i;
  size_t i, n = val->n;

  sza->resize(n);
  for (i=0; i<n; ++i)
    if (z[i] >= 0)
      (*sza)[i] = z[i];
    else
      botch("%s must have non-negative values", keyname);
}

void NIDRProblemDescDB::
method_slit2(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;

  dm->*((Method_mp_slit2*)v)->sp  = ((Method_mp_slit2*)v)->lit;
  dm->*((Method_mp_slit2*)v)->sp2 = *val->s;
}

void NIDRProblemDescDB::
method_utype_lit(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep *dm = (*(Meth_Info**)g)->dme;

  (*(Meth_Info**)g)->dme->*((Method_mp_utype*)v)->ip
    = ((Method_mp_utype*)v)->utype;
  dm->*((Method_mp_utype_lit*)v)->sp = *val->s;
}

void NIDRProblemDescDB::
method_start(const char *keyname, Values *val, void **g, void *v)
{
  Meth_Info *mi;

  if (!(mi = new Meth_Info))
  Botch:		botch("new failure in method_start");
  if (!(mi->dme0 = new DataMethod))
    goto Botch;
  mi->dme = mi->dme0->dataMethodRep;
  *g = (void*)mi;
}

static void
scale_chk(StringArray &ST, RealVector &S, const char *what, const char **univ)
{
  const char *s, **u;
  size_t i, n, nbad, vseen;

  n = ST.size();
  for(i = nbad = vseen = 0; i < n; ++i) {
    s = ST[i].data();
    if (!strcmp(s,"value")) {
      ++vseen;
      goto break2;
    }
    for(u = univ; *u; ++u)
      if (!strcmp(s,*u)) {
	goto break2;
      }
    NIDRProblemDescDB::squawk("\"%s\" cannot appear in %s_scale_types",
			      s, what);
    ++nbad;
  break2:	;
  }
  if (vseen && S.length() <= 0)
    NIDRProblemDescDB::squawk(
      "\"value\" in %s_scale_types requires at least one value for %s_scales",
      what, what);
}

static const char *aln_scaletypes[] = { "auto", "log", "none", 0 };

void NIDRProblemDescDB::
method_stop(const char *keyname, Values *val, void **g, void *v)
{
  Meth_Info *mi = *(Meth_Info**)g;
  //DataMethodRep *dm = mi->dme;
  // ... any checks ...
  pDDBInstance->dataMethodList.push_back(*mi->dme0);
  delete mi->dme0;
  delete mi;
}

void NIDRProblemDescDB::
method_str(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->**(String DataMethodRep::**)v = *val->s;
}

void NIDRProblemDescDB::
method_strL(const char *keyname, Values *val, void **g, void *v)
{
  StringArray *sa = &((*(Meth_Info**)g)->dme->**(StringArray DataMethodRep::**)v);
  const char **s = val->s;
  size_t i, n = val->n;

  sa->resize(n);
  for(i = 0; i < n; i++)
    (*sa)[i] = s[i];
}

void NIDRProblemDescDB::
method_true(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->**(bool DataMethodRep::**)v = true;
}

void NIDRProblemDescDB::
method_type(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->*((Method_mp_type*)v)->ip
    = ((Method_mp_type*)v)->type;
}

void NIDRProblemDescDB::
method_utype(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->*((Method_mp_utype*)v)->ip
    = ((Method_mp_utype*)v)->utype;
}

void NIDRProblemDescDB::
method_augment_utype(const char *keyname, Values *val, void **g, void *v)
{
  (*(Meth_Info**)g)->dme->*((Method_mp_utype*)v)->ip
    |= ((Method_mp_utype*)v)->utype;
}

void NIDRProblemDescDB::
model_Real(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->**(Real DataModelRep::**)v = *val->r;
}

void NIDRProblemDescDB::
model_RealDL(const char *keyname, Values *val, void **g, void *v)
{
  Real *r = val->r;
  RealVector *rdv = &((*(Mod_Info**)g)->dmo->**(RealVector DataModelRep::**)v);
  size_t i, n = val->n;

  rdv->sizeUninitialized(n);
  for(i = 0; i < n; i++)
    (*rdv)[i] = r[i];
}

void NIDRProblemDescDB::
model_ivec(const char *keyname, Values *val, void **g, void *v)
{
  DataModelRep *dm = (*(Mod_Info**)g)->dmo;
  IntVector *iv = &(dm->**(IntVector DataModelRep::**)v);
  size_t i, n = val->n;
  iv->sizeUninitialized(n);

  int *z = val->i;
  for(i = 0; i < n; i++)
    (*iv)[i] = z[i];
}

void NIDRProblemDescDB::
model_false(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->**(bool DataModelRep::**)v = false;
}

void NIDRProblemDescDB::
model_int(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->**(int DataModelRep::**)v = *val->i;
}

void NIDRProblemDescDB::
model_intsetm1(const char *keyname, Values *val, void **g, void *v)
{
  IntSet *is = &((*(Mod_Info**)g)->dmo->**(IntSet DataModelRep::**)v);
  int *z = val->i;
  size_t i, n = val->n;

  for(i = 0; i < n; i++)
    is->insert(z[i] - 1); // model converts ids -> indices
}

void NIDRProblemDescDB::
model_lit(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->*((Model_mp_lit*)v)->sp = ((Model_mp_lit*)v)->lit;
}

void NIDRProblemDescDB::
model_order(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->*((Model_mp_ord*)v)->sp = ((Model_mp_ord*)v)->ord;
}

void NIDRProblemDescDB::
model_pint(const char *keyname, Values *val, void **g, void *v)
{
  int n = *val->i;
#ifdef REDUNDANT_INT_CHECKS
  if (n <= 0) /* now handled by INTEGER > 0 in the .nspec file */
    botch("%s must be positive", keyname);
#endif
  (*(Mod_Info**)g)->dmo->**(int DataModelRep::**)v = n;
}

void NIDRProblemDescDB::
model_type(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->*((Model_mp_type*)v)->sp = ((Model_mp_type*)v)->type;
}

void NIDRProblemDescDB::
model_utype(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->*((Model_mp_utype*)v)->sp = 
    ((Model_mp_utype*)v)->utype;
}

void NIDRProblemDescDB::
model_augment_utype(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->*((Model_mp_utype*)v)->sp |= 
    ((Model_mp_utype*)v)->utype;
}

void NIDRProblemDescDB::
model_shint(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->**(short DataModelRep::**)v = (short)*val->i;
}

void NIDRProblemDescDB::
model_start(const char *keyname, Values *val, void **g, void *v)
{
  DataModelRep *dm;
  Mod_Info *mi;

  if (!(mi = new Mod_Info))
  Botch:		botch("new failure in model_start");
  if (!(mi->dmo0 = new DataModel))
    goto Botch;
  dm = mi->dmo = mi->dmo0->dataModelRep;
  *g = (void*)mi;
}

void NIDRProblemDescDB::
model_stop(const char *keyname, Values *val, void **g, void *v)
{
  Mod_Info *mi = *(Mod_Info**)g;
  pDDBInstance->dataModelList.push_back(*mi->dmo0);
  delete mi->dmo0;
  delete mi;
}

void NIDRProblemDescDB::
model_str(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->**(String DataModelRep::**)v = *val->s;
}

void NIDRProblemDescDB::
model_strL(const char *keyname, Values *val, void **g, void *v)
{
  DataModelRep *dm = (*(Mod_Info**)g)->dmo;
  StringArray DataModelRep::* sa = *(StringArray DataModelRep::**)v;
  const char **s = val->s;
  size_t i, n = val->n;

  (dm->*sa).resize(n);
  for(i = 0; i < n; i++)
    (dm->*sa)[i] = s[i];
}

void NIDRProblemDescDB::
model_true(const char *keyname, Values *val, void **g, void *v)
{
  (*(Mod_Info**)g)->dmo->**(bool DataModelRep::**)v = true;
}

void NIDRProblemDescDB::
resp_RealDL(const char *keyname, Values *val, void **g, void *v)
{
  Real *r = val->r;
  RealVector *rv = &((*(Resp_Info**)g)->dr->**(RealVector DataResponsesRep::**)v);
  size_t i, n = val->n;

  rv->sizeUninitialized(n);
  for(i = 0; i < n; i++)
    (*rv)[i] = r[i];
}

void NIDRProblemDescDB::
resp_RealL(const char *keyname, Values *val, void **g, void *v)
{
  NIDRProblemDescDB::resp_RealDL(keyname, val, g, v);
}

void NIDRProblemDescDB::
resp_intset(const char *keyname, Values *val, void **g, void *v)
{
  IntSet *is = &((*(Resp_Info**)g)->dr->**(IntSet DataResponsesRep::**)v);
  int *z = val->i;
  size_t i, n = val->n;

  for (i=0; i<n; ++i)
    is->insert(z[i]);
}

void NIDRProblemDescDB::
resp_ivec(const char *keyname, Values *val, void **g, void *v)
{
  DataResponsesRep *dr = (*(Resp_Info**)g)->dr;
  IntVector *iv = &(dr->**(IntVector DataResponsesRep::**)v);
  size_t i, n = val->n;
  iv->sizeUninitialized(n);

  int *z = val->i;
  for(i = 0; i < n; i++)
    (*iv)[i] = z[i];
}

void NIDRProblemDescDB::
resp_lit(const char *keyname, Values *val, void **g, void *v)
{
  (*(Resp_Info**)g)->dr->*((Resp_mp_lit*)v)->sp = ((Resp_mp_lit*)v)->lit;
}

void NIDRProblemDescDB::
resp_utype(const char *keyname, Values *val, void **g, void *v)
{
  (*(Resp_Info**)g)->dr->*((Resp_mp_utype*)v)->sp
    = ((Resp_mp_utype*)v)->utype;
}

void NIDRProblemDescDB::
resp_augment_utype(const char *keyname, Values *val, void **g, void *v)
{
  (*(Resp_Info**)g)->dr->*((Resp_mp_utype*)v)->sp
    |= ((Resp_mp_utype*)v)->utype;
}


void NIDRProblemDescDB::
resp_sizet(const char *keyname, Values *val, void **g, void *v)
{
  int n = *val->i; // test value as int, prior to storage as size_t
#ifdef REDUNDANT_INT_CHECKS
  if (n < 0) /* now handled by INTEGER >= 0 in the .nspec file */
    botch("%s must be non-negative", keyname);
#endif
  (*(Resp_Info**)g)->dr->**(size_t DataResponsesRep::**)v = n;
}

void NIDRProblemDescDB::
resp_start(const char *keyname, Values *val, void **g, void *v)
{
  Resp_Info *ri;

  if (!(ri = new Resp_Info))
  Botch:		botch("new failure in resp_start");
  if (!(ri->dr0 = new DataResponses))
    goto Botch;
  ri->dr = ri->dr0->dataRespRep;
  *g = (void*)ri;
}

static void
BuildLabels(StringArray *sa, size_t nsa, size_t n1, size_t n2, const char *stub)
{
  char buf[64];
  size_t i, n0;
  if (nsa)
    sa->resize(nsa);
  i = n0 = n1;
  while(n1 < n2) {
    std::sprintf(buf, "%s%lu", stub, (UL)(++n1 - n0));
    (*sa)[i++] = buf;
  }
}

static int mixed_check(IntSet *S, int n, IntArray *iv, const char *what)
{
  int nbad, j;

  nbad = 0;
  for(ISCIter it = S->begin(), ite = S->end(); it != ite; ++it) {
    j = *it;
    if (j < 1 || j > n) {
      if (!nbad++)
	Squawk("%s values must be between 1 and %d", what, n);
    }
    else {
      ++(iv->operator[](j-1));
    }
  }
  return nbad;
}

static void
mixed_check2(size_t n, IntArray *iv, const char *what)
{
  int j;
  size_t i;

  for(i = 0; i < n; ) {
    j = (*iv)[i++];
    if (j == 0)
      Squawk("Function %lu missing from mixed %s lists", (UL)i);
    else if (j > 1)
      Squawk("Function %lu replicated in mixed %s lists", (UL)i);
  }
}

void NIDRProblemDescDB::
resp_stop(const char *keyname, Values *val, void **g, void *v)
{
  size_t k, n;
  static const char *osc[] = { "log", "none", 0 };
  Resp_Info *ri = *(Resp_Info**)g;
  DataResponsesRep *dr = ri->dr;
  scale_chk(dr->primaryRespFnScaleTypes, dr->primaryRespFnScales,
	    dr->numLeastSqTerms ? "least_squares_term" : "objective_function", osc);
  scale_chk(dr->nonlinearIneqScaleTypes, dr->nonlinearIneqScales,
	    "nonlinear_inequality", aln_scaletypes);
  scale_chk(dr->nonlinearEqScaleTypes, dr->nonlinearEqScales,
	    "nonlinear_equality", aln_scaletypes);
  if ( dr->primaryRespFnWeights.length() > 0 && dr->varianceType.size() > 0 ) {
    squawk("Specify calibration weights or experimental errors, not both.");
  }
  if ((n = dr->responseLabels.size()) > 0) {
    if (!(k = dr->numResponseFunctions)) {
      if (!(k = dr->numObjectiveFunctions))
	k = dr->numLeastSqTerms;
      k += dr->numNonlinearIneqConstraints + dr->numNonlinearEqConstraints;
    }
    if (n != k)
      squawk("Expected %ld response descriptors but found %ld",
	     (long)k, (long) n);
  }
  pDDBInstance->dataResponsesList.push_back(*ri->dr0);
  delete ri->dr0;
  delete ri;
}


void NIDRProblemDescDB::
check_responses(std::list<DataResponses>* drl)
{
  // TO DO: move Schk from below?

  // validate descriptors. The string arrays are empty unless the user
  // explicitly set descriptors.
  std::list<DataResponses>::iterator It = drl->begin(), Ite = drl->end();
  for(; It != Ite; ++It) {
    const DataResponsesRep* drr = It->data_rep();
    check_descriptors(drr->responseLabels);
  }
}


void NIDRProblemDescDB::
make_response_defaults(std::list<DataResponses>* drl)
{
  IntArray *iv;
  StringArray *rl;
  int ni;
  size_t i, n, n1, nf, no, nrl;
  struct RespStr_chk {
    const char *what;
    size_t DataResponsesRep::* n;
    StringArray DataResponsesRep::* sa;
  } *sc;
  struct RespDVec_chk {
    const char *what;
    size_t DataResponsesRep::* n;
    RealVector DataResponsesRep::* rv;
  } *rdvc;
#define Schk(a,b,c) {#a,&DataResponsesRep::b, &DataResponsesRep::c}
  static RespStr_chk Str_chk[] = {  // for StringArray checking
    Schk(least_squares_term_scale_types,numLeastSqTerms,primaryRespFnScaleTypes),
    Schk(nonlinear_equality_scale_types,numNonlinearEqConstraints,nonlinearEqScaleTypes),
    Schk(nonlinear_inequality_scale_types,numNonlinearIneqConstraints,nonlinearIneqScaleTypes),
    Schk(objective_function_scale_types,numObjectiveFunctions,primaryRespFnScaleTypes)
  };
  static RespDVec_chk RespVec_chk_Bound[] = {// Bounds:  length must be right
    Schk(least_squares_weights,numLeastSqTerms,primaryRespFnWeights),
    Schk(multi_objective_weights,numObjectiveFunctions,primaryRespFnWeights),
    Schk(nonlinear_equality_targets,numNonlinearEqConstraints,nonlinearEqTargets),
    Schk(nonlinear_inequality_lower_bounds,numNonlinearIneqConstraints,nonlinearIneqLowerBnds),
    Schk(nonlinear_inequality_upper_bounds,numNonlinearIneqConstraints,nonlinearIneqUpperBnds)
  };
  static RespDVec_chk RespVec_chk_Scale[] = {// Scales:  length must be right
    Schk(least_squares_term_scales,numLeastSqTerms,primaryRespFnScales),
    Schk(nonlinear_equality_scales,numNonlinearEqConstraints,nonlinearEqScales),
    Schk(nonlinear_inequality_scales,numNonlinearIneqConstraints,nonlinearIneqScales),
    Schk(objective_function_scales,numObjectiveFunctions,primaryRespFnScales)
  };
#undef Schk
#define Numberof(x) sizeof(x)/sizeof(x[0])

  Real dbl_inf = std::numeric_limits<Real>::infinity();
  std::list<DataResponses>::iterator It = drl->begin(), Ite = drl->end();
  for(; It != Ite; It++) {

    DataResponsesRep *dr = It->dataRespRep;

    for(sc = Str_chk, i = 0; i < Numberof(Str_chk); ++sc, ++i)
      if ((n1 = dr->*sc->n) && (n = (dr->*sc->sa).size()) > 0
	  && n != n1 && n != 1)
	squawk("%s must have length 1 or %lu, not %lu",
	       sc->what, (UL)n1, (UL)n);
    rl = &dr->responseLabels;
    nrl = rl->size();
    no = dr->numObjectiveFunctions;
    if ((n = dr->numLeastSqTerms)) {
      nf = n + dr->numNonlinearEqConstraints + dr->numNonlinearIneqConstraints;
      if (!nrl) {
	BuildLabels(rl, nf, 0, n, "least_sq_term_");
	n1 = n + dr->numNonlinearIneqConstraints;
	BuildLabels(rl, 0, n, n1, "nln_ineq_con_");
	BuildLabels(rl, 0, n1, nf, "nln_eq_con_");
      }
    }
    else if ((nf = no + dr->numNonlinearEqConstraints + dr->numNonlinearIneqConstraints)) {
      if (!nrl) {
	rl->resize(nf);
	if (no == 1)
	  (*rl)[0] = "obj_fn";
	else
	  BuildLabels(rl, 0, 0, no, "obj_fn_");
	n1 = no + dr->numNonlinearIneqConstraints;
	BuildLabels(rl, 0, no, n1, "nln_ineq_con_");
	BuildLabels(rl, 0, n1, nf, "nln_eq_con_");
      }
    }
    else if ((nf = dr->numResponseFunctions)) {
      if (!nrl)
	BuildLabels(rl, nf, 0, nf, "response_fn_");
    }
    for(rdvc = RespVec_chk_Bound, i = 0; i < Numberof(RespVec_chk_Bound); ++rdvc, ++i)
      if ((n1 = dr->*rdvc->n) && (n = (dr->*rdvc->rv).length()) && n != n1)
	squawk("%s needs %lu elements, not %lu",
	       rdvc->what, (UL)(dr->*rdvc->n), (UL)n);
    for(rdvc = RespVec_chk_Scale, i = 0; i < Numberof(RespVec_chk_Scale); ++rdvc, ++i)
      if ((n1 = dr->*rdvc->n) && (n = (dr->*rdvc->rv).length())
	  && n != n1 && n != 1)
	squawk("%s needs %lu elements (or just one), not %lu",
	       rdvc->what, (UL)n1, (UL)n);
    if (dr->methodSource == "vendor" && dr->fdGradStepSize.length() > 1)
      squawk("vendor numerical gradients only support a single fd_gradient_step_size");

    ni = (int)nf;
    if (dr->gradientType == "mixed") {
      iv = new IntArray;
      //iv->resize(nf);
      iv->assign(nf, 0);
      if (!(mixed_check(&dr->idAnalyticGrads, ni, iv, "id_analytic_gradients")
	    + mixed_check(&dr->idNumericalGrads, ni, iv, "id_numerical_gradients")))
	mixed_check2(nf, iv, "gradient");
      delete iv;
    }
    if ((n = dr->numNonlinearEqConstraints) > 0
	&& dr->nonlinearEqTargets.length() == 0) {
      dr->nonlinearEqTargets.sizeUninitialized(n);
      dr->nonlinearEqTargets = 0.;
    }
    if ((n = dr->numNonlinearIneqConstraints) > 0) {
      if (dr->nonlinearIneqLowerBnds.length() == 0) {
	dr->nonlinearIneqLowerBnds.sizeUninitialized(n);
	dr->nonlinearIneqLowerBnds = -dbl_inf;
      }
      if (dr->nonlinearIneqUpperBnds.length() == 0) {
	dr->nonlinearIneqUpperBnds.sizeUninitialized(n);
	dr->nonlinearIneqUpperBnds = 0.;
	// default is a one-sided inequality <= 0.0
      }
    }
    if (dr->hessianType == "mixed") {
      iv = new IntArray;
      //iv->resize(nf);
      iv->assign(nf, 0);
      if (!(mixed_check(&dr->idAnalyticHessians, ni, iv, "id_analytic_hessians")
	    + mixed_check(&dr->idNumericalHessians, ni, iv, "id_numerical_hessians")
	    + mixed_check(&dr->idQuasiHessians, ni, iv, "id_quasi_hessians")))
	mixed_check2(nf, iv, "Hessian");
      delete iv;
    }
    if (nerr)
      abort_handler(PARSE_ERROR);
  }
}

void NIDRProblemDescDB::
resp_str(const char *keyname, Values *val, void **g, void *v)
{
  (*(Resp_Info**)g)->dr->**(String DataResponsesRep::**)v = *val->s;
}

void NIDRProblemDescDB::
resp_strL(const char *keyname, Values *val, void **g, void *v)
{
  StringArray *sa
    = &((*(Resp_Info**)g)->dr->**(StringArray DataResponsesRep::**)v);
  const char **s = val->s;
  size_t i, n = val->n;

  sa->resize(n);
  for(i = 0; i < n; i++)
    (*sa)[i] = s[i];
}

void NIDRProblemDescDB::
resp_false(const char *keyname, Values *val, void **g, void *v)
{
  (*(Resp_Info**)g)->dr->**(bool DataResponsesRep::**)v = false;
}

void NIDRProblemDescDB::
resp_true(const char *keyname, Values *val, void **g, void *v)
{
  (*(Resp_Info**)g)->dr->**(bool DataResponsesRep::**)v = true;
}

// void NIDRProblemDescDB::
// env_Real(const char *keyname, Values *val, void **g, void *v)
// {
//   (*(DataEnvironmentRep**)g)->**(Real DataEnvironmentRep::**)v = *val->r;
// }

// void NIDRProblemDescDB::
// env_RealL(const char *keyname, Values *val, void **g, void *v)
// {
//   Real *r = val->r;
//   RealVector *rdv
//     = &((*(DataEnvironmentRep**)g)->**(RealVector DataEnvironmentRep::**)v);
//   size_t i, n = val->n;

//   rdv->sizeUninitialized(n);
//   for(i = 0; i < n; i++)
//     (*rdv)[i] = r[i];
// }

void NIDRProblemDescDB::
env_true(const char *keyname, Values *val, void **g, void *v)
{
  (*(DataEnvironmentRep**)g)->**(bool DataEnvironmentRep::**)v = true;
}

void NIDRProblemDescDB::
env_int(const char *keyname, Values *val, void **g, void *v)
{
  (*(DataEnvironmentRep**)g)->**(int DataEnvironmentRep::**)v = *val->i;
}

// void NIDRProblemDescDB::
// env_lit(const char *keyname, Values *val, void **g, void *v)
// {
//   (*(DataEnvironmentRep**)g)->*((Environment_mp_lit*)v)->sp
//     = ((Environment_mp_lit*)v)->lit;
// }

void NIDRProblemDescDB::
env_start(const char *keyname, Values *val, void **g, void *v)
{
  *g = (void*)pDDBInstance->environmentSpec.dataEnvRep;
}

void NIDRProblemDescDB::
env_str(const char *keyname, Values *val, void **g, void *v)
{
  (*(DataEnvironmentRep**)g)->**(String DataEnvironmentRep::**)v = *val->s;
}

/// set a value for an unsigned short type
void NIDRProblemDescDB::
env_utype(const char *keyname, Values *val, void **g, void *v)
{
  (*(DataEnvironmentRep**)g)->*((Env_mp_utype*)v)->sp = 
    ((Env_mp_utype*)v)->utype;
}

/// augment an unsigned short type with |=
void NIDRProblemDescDB::
env_augment_utype(const char *keyname, Values *val, void **g, void *v)
{
  (*(DataEnvironmentRep**)g)->*((Env_mp_utype*)v)->sp |= 
    ((Env_mp_utype*)v)->utype;
}

void NIDRProblemDescDB::
env_strL(const char *keyname, Values *val, void **g, void *v)
{
  StringArray *sa
    = &((*(DataEnvironmentRep**)g)->**(StringArray DataEnvironmentRep::**)v);
  const char **s = val->s;
  size_t i, n = val->n;

  sa->resize(n);
  for(i = 0; i < n; i++)
    (*sa)[i] = s[i];
}

void NIDRProblemDescDB::
method_tr_final(const char *keyname, Values *val, void **g, void *v)
{
  DataMethodRep &data_method = *(*(Meth_Info**)g)->dme;

  // sanity checks on trust region user-defined values
  size_t i, num_init = data_method.trustRegionInitSize.length();
  Real min_init_size = 1.;
  for (i=0; i<num_init; ++i) {
    Real init_size_i = data_method.trustRegionInitSize[i];
    if ( init_size_i <= 0. || init_size_i > 1. )
      botch("specified initial TR size must be in (0,1]");
    if (init_size_i < min_init_size)
      min_init_size = init_size_i;
  }
  if ( data_method.trustRegionMinSize > min_init_size ) {
    if (num_init) botch("specified initial TR size less than minimum TR size");
    else          botch("minimum TR size must be <= 1.");
  }
  // allow 0 for min size spec (conv control becomes inactive)
  if ( data_method.trustRegionMinSize < 0. ||
       data_method.trustRegionMinSize > 1. )
    botch("specified minimum TR size must be in [0,1]");
  if( data_method.trustRegionContractTrigger <= 0. ||
      data_method.trustRegionContractTrigger >
      data_method.trustRegionExpandTrigger         ||
      data_method.trustRegionExpandTrigger   >  1. )
    botch("expand/contract threshold values must satisfy\n\t"
	  "0 < contract_threshold <= expand_threshold <= 1");
  if ( data_method.trustRegionContract <= 0. ||
       data_method.trustRegionContract >  1. )
    botch("contraction_factor must be in (0,1]");
  else if ( data_method.trustRegionContract == 1. )
    warn("contraction_factor = 1.0 is valid, but should be < 1\n\t"
	 "to assure convergence of the surrogate_based_opt method");
  if ( data_method.trustRegionExpand < 1. )
    botch("expansion_factor must be >= 1");
}


void NIDRProblemDescDB::
var_RealLb(const char *keyname, Values *val, void **g, void *v)
{
  Real b, *r;
  RealVector *rv;
  Var_brv *V;
  size_t i, n;

  V = (Var_brv*)v;
  rv = &((*(Var_Info**)g)->dv->*V->rv);
  b = V->b;
  r = val->r;
  n = val->n;
  for(i = 0; i < n; i++)
    if (r[i] <= b) {
      squawk("%s values must be > %g", keyname, b);
      break;
    }
  rv->sizeUninitialized(n);
  for(i = 0; i < n; i++)
    (*rv)[i] = r[i];
}

void NIDRProblemDescDB::
var_RealUb(const char *keyname, Values *val, void **g, void *v)
{
  Real b, *r;
  RealVector *rv;
  Var_brv *V;
  size_t i, n;

  V = (Var_brv*)v;
  rv = &((*(Var_Info**)g)->dv->*V->rv);
  b = V->b;
  r = val->r;
  n = val->n;
  for(i = 0; i < n; i++)
    if (r[i] >= b) {
      squawk("%s values must be < %g", keyname, b);
      break;
    }
  rv->sizeUninitialized(n);
  for(i = 0; i < n; i++)
    (*rv)[i] = r[i];
}

void NIDRProblemDescDB::
var_IntLb(const char *keyname, Values *val, void **g, void *v)
{
  Var_biv *V = (Var_biv*)v;
  IntVector *iv = &((*(Var_Info**)g)->dv->*V->iv);
  int b = V->b;
  int *z = val->i;
  size_t i, n = val->n;
  for(i = 0; i < n; i++)
    if (z[i] <= b) {
      squawk("%s values must be > %g", keyname, b);
      break;
    }
  iv->sizeUninitialized(n);
  for(i = 0; i < n; i++)
    (*iv)[i] = z[i];
}


/// Map an NIDR STRINGLIST to a BoolDeque based on string values; for
/// now we require user to specify all N values
void NIDRProblemDescDB::
var_categorical(const char *keyname, Values *val, void **g, void *v)
{
  BitArray *ba
    = &((*(Var_Info**)g)->dv->**(BitArray DataVariablesRep::**)v);
  const char **s = val->s;
  size_t i, n = val->n;

  // allow strings beginning y Y or T t (yes/true)
  ba->resize(n);
  for(i = 0; i < n; i++) {
    String str_lower(strtolower(s[i]));
    (*ba)[i] = strbegins(str_lower, "y") || strbegins(str_lower, "t");
  }
}


void NIDRProblemDescDB::
var_newrvec(const char *keyname, Values *val, void **g, void *v)
{
  Var_Info *vi = *(Var_Info**)g;
  RealVector *rv;
  size_t i, n = val->n;
  Real *r = val->r;

  if (!(rv = new RealVector(n, false)))
    botch("new failure in var_newrvec");
  vi->**(RealVector *Var_Info::**)v = rv;
  for(i = 0; i < n; i++)
    (*rv)[i] = r[i];
}

void NIDRProblemDescDB::
var_newivec(const char *keyname, Values *val, void **g, void *v)
{
  Var_Info *vi = *(Var_Info**)g;
  IntVector *iv;
  size_t i, n = val->n;
  int *z = val->i;

  if (!(iv = new IntVector(n, false)))
    botch("new failure in var_newivec");
  vi->**(IntVector *Var_Info::**)v = iv;
  for(i = 0; i < n; i++)
    (*iv)[i] = z[i];
}

void NIDRProblemDescDB::
var_newiarray(const char *keyname, Values *val, void **g, void *v)
{
  Var_Info *vi = *(Var_Info**)g;
  IntArray *iv;
  size_t i, n = val->n;
  int *z = val->i;

  if (!(iv = new IntArray(n)))
    botch("new failure in var_intarray");
  vi->**(IntArray *Var_Info::**)v = iv;
  for(i = 0; i < n; i++)
    (*iv)[i] = z[i];
}

void NIDRProblemDescDB::
var_newsarray(const char *keyname, Values *val, void **g, void *v)
{
  Var_Info *vi = *(Var_Info**)g;
  StringArray *sa;
  size_t i, n = val->n;
  const char **z = val->s;

  if (!(sa = new StringArray(n)))
    botch("new failure in var_newsarray");
  vi->**(StringArray *Var_Info::**)v = sa;
  for(i = 0; i < n; i++)
    (*sa)[i] = z[i];
}

void NIDRProblemDescDB::
var_rvec(const char *keyname, Values *val, void **g, void *v)
{
  RealVector *rv
    = &((*(Var_Info**)g)->dv->**(RealVector DataVariablesRep::**)v);
  size_t i, n = val->n;
  rv->sizeUninitialized(n);

  Real *r = val->r;
  for(i = 0; i < n; i++)
    (*rv)[i] = r[i];
}

void NIDRProblemDescDB::
var_ivec(const char *keyname, Values *val, void **g, void *v)
{
  IntVector *iv = &((*(Var_Info**)g)->dv->**(IntVector DataVariablesRep::**)v);
  size_t i, n = val->n;
  iv->sizeUninitialized(n);

  int *z = val->i;
  for(i = 0; i < n; i++)
    (*iv)[i] = z[i];
}

// BMA TODO: I believe this duplicative of var_strL.
// void NIDRProblemDescDB::
// var_svec(const char *keyname, Values *val, void **g, void *v)
// {
//   StringArray *sa = &((*(Var_Info**)g)->dv->**(StringArray DataVariablesRep::**)v);
//   size_t i, n = val->n;
//   sa->resize(n);

//   const char **z = val->s;
//   for(i = 0; i < n; i++)
//     (*sa)[i] = z[i];
// }

void NIDRProblemDescDB::
var_pintz(const char *keyname, Values *val, void **g, void *v)
{
  int n = *val->i; // test value as int, prior to storage as size_t
#ifdef REDUNDANT_INT_CHECKS
  if (n <= 0) /* now handled by INTEGER > 0 in the .nspec file */
    botch("%s must be positive", keyname);
#endif
  (*(Var_Info**)g)->dv->**(size_t DataVariablesRep::**)v = n;
}

void NIDRProblemDescDB::
var_type(const char *keyname, Values *val, void **g, void *v)
{
  (*(Var_Info**)g)->dv->*((Var_mp_type*)v)->sp = ((Var_mp_type*)v)->type;
}

void NIDRProblemDescDB::
var_start(const char *keyname, Values *val, void **g, void *v)
{
  Var_Info *vi;

  if (!(vi = new Var_Info))
  Botch:		botch("new failure in var_start");
  memset(vi, 0, sizeof(Var_Info));
  if (!(vi->dv_handle = new DataVariables))
    goto Botch;
  vi->dv = vi->dv_handle->dataVarsRep;
  *g = (void*)vi;
}

void NIDRProblemDescDB::
var_true(const char *keyname, Values *val, void **g, void *v)
{
  (*(Var_Info**)g)->dv->**(bool DataVariablesRep::**)v = true;
}

static int wronglen(size_t n, RealVector *V, const char *what)
{
  size_t n1 = V->length();
  if (n != n1) {
    Squawk("Expected %d numbers for %s, but got %d", (int)n, what, (int)n1);
    return 1;
  }
  return 0;
}

static int wronglen(size_t n, IntVector *V, const char *what)
{
  size_t n1 = V->length();
  if (n != n1) {
    Squawk("Expected %d numbers for %s, but got %d", (int)n, what, (int)n1);
    return 1;
  }
  return 0;
}

static void Vcopyup(RealVector *V, RealVector *M, size_t i, size_t n)
{
  size_t j;
  for(j = 0; j < n; ++i, ++j)
    (*V)[i] = (*M)[j];
}

static void Set_rv(RealVector *V, double d, size_t n)
{
  size_t i;
  V->sizeUninitialized(n);
  for(i = 0; i < n; ++i)
    (*V)[i] = d;
}

static void Set_iv(IntVector *V, int d, size_t n)
{
  size_t i;
  V->sizeUninitialized(n);
  for(i = 0; i < n; ++i)
    (*V)[i] = d;
}

static void
wrong_number(const char *what, const char *kind, size_t nsv, size_t m)
{
  Squawk("Expected %d %s for %s, not %d", (int)nsv, what, kind, (int)m);
}

static void too_small(const char *kind)
{
  Squawk("num_set_values values for %s must be >= 1", kind);
}

static void not_div(const char *kind, size_t nsv, size_t m)
{
  Squawk("Number of %s set_values (%d) not evenly divisible by number of variables (%d); use num_set_values for unequal apportionment",
	 kind, (int)nsv, (int)m);
}

static void suppressed(const char *kind, int ndup, int *ip, String *sp, Real *rp)
{
  const char *s;
  int i, nother;

  nother = 0;
  if (ndup > 2) {
    nother = ndup - 1;
    ndup = 1;
  }
  for(i = 0; i < ndup; ++i)
    if (ip)
      Squawk("Duplicate %s value %d", kind, ip[i]);
    else if (sp)
      Squawk("Duplicate %s value %s", kind, sp[i].c_str());
    else
      Squawk("Duplicate %s value %.17g", kind, rp[i]);
  if (nother) {
    s = "s" + (nother == 1);
    Squawk("Warning%s of %d other duplicate %s value%s suppressed",
	   s, nother, kind, s);
  }
}

static void bad_initial_ivalue(const char *kind, int val)
{
  Squawk("invalid initial value %d for %s", val, kind);
}

static void bad_initial_svalue(const char *kind, String val)
{
  Squawk("invalid initial value %s for %s", val.c_str(), kind);
}

static void bad_initial_rvalue(const char *kind, Real val)
{
  Squawk("invalid initial value %.17g for %s", val, kind);
}

// *****************************************************************************
//  Vchk functions called earlier from within check_variables_node(), which
//   immediately {precedes,follows} DB buffer {send,receive} in broadcast().
// Vgen functions called later from within make_variable_defaults()
//   (from within post_process() following broadcast()).
// As documented in ProblemDescDB::manage_inputs(), Vchk applies to minimal
//   spec data, whereas Vgen constructs any large inferred vectors.
// *****************************************************************************
static void Vgen_ContinuousDes(DataVariablesRep *dv, size_t offset)
{
  RealVector *L, *U, *V;
  size_t i, n = dv->numContinuousDesVars;
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  L = &dv->continuousDesignLowerBnds;
  U = &dv->continuousDesignUpperBnds;
  V = &dv->continuousDesignVars;
  if (L->length() == 0)
    Set_rv(L, -dbl_inf, n);
  if (U->length() == 0)
    Set_rv(U, dbl_inf, n);
  if (V->length() == 0) {
    V->sizeUninitialized(n);
    for(i = 0; i < n; i++) { // init to 0, repairing to bounds if needed
      if      ((*L)[i] > 0.) (*V)[i] = (*L)[i];
      else if ((*U)[i] < 0.) (*V)[i] = (*U)[i];
      else                   (*V)[i] = 0;
    }
  }
}

static void Vgen_DiscreteDesRange(DataVariablesRep *dv, size_t offset)
{
  IntVector *L, *U, *V;
  size_t i, n = dv->numDiscreteDesRangeVars;

  L = &dv->discreteDesignRangeLowerBnds;
  U = &dv->discreteDesignRangeUpperBnds;
  V = &dv->discreteDesignRangeVars;
  if (L->length() == 0)
    Set_iv(L, INT_MIN, n);
  if (U->length() == 0)
    Set_iv(U, INT_MAX, n);
  if (V->length() == 0) {
    V->sizeUninitialized(n);
    for(i = 0; i < n; ++i) { // init to 0, repairing to bounds if needed
      if      ((*L)[i] > 0) (*V)[i] = (*L)[i];
      else if ((*U)[i] < 0) (*V)[i] = (*U)[i];
      else                  (*V)[i] = 0;
    }
  }
}

static void Vgen_ContinuousState(DataVariablesRep *dv, size_t offset)
{
  RealVector *L, *U, *V;
  size_t i, n = dv->numContinuousStateVars;
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  L = &dv->continuousStateLowerBnds;
  U = &dv->continuousStateUpperBnds;
  V = &dv->continuousStateVars;
  if (L->length() == 0)
    Set_rv(L, -dbl_inf, n);
  if (U->length() == 0)
    Set_rv(U, dbl_inf, n);
  if (V->length() == 0) {
    V->sizeUninitialized(n);
    for(i = 0; i < n; i++) { // init to 0, repairing to bounds if needed
      if      ((*L)[i] > 0.) (*V)[i] = (*L)[i];
      else if ((*U)[i] < 0.) (*V)[i] = (*U)[i];
      else                   (*V)[i] = 0;
    }
  }
}

static void Vgen_DiscreteStateRange(DataVariablesRep *dv, size_t offset)
{
  IntVector *L, *U, *V;
  size_t i, n = dv->numDiscreteStateRangeVars;

  L = &dv->discreteStateRangeLowerBnds;
  U = &dv->discreteStateRangeUpperBnds;
  V = &dv->discreteStateRangeVars;
  if (L->length() == 0)
    Set_iv(L, INT_MIN, n);
  if (U->length() == 0)
    Set_iv(U, INT_MAX, n);
  if (V->length() == 0) {
    V->sizeUninitialized(n);
    for(i = 0; i < n; ++i) { // init to 0, repairing to bounds if needed
      if      ((*L)[i] > 0) (*V)[i] = (*L)[i];
      else if ((*U)[i] < 0) (*V)[i] = (*U)[i];
      else                  (*V)[i] = 0;
    }
  }
}

static void Vchk_NormalUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t n;
  RealVector *B, *M, *Sd;

  n = dv->numNormalUncVars;
  if (wronglen(n,  M = &dv->normalUncMeans,   "nuv_means") ||
      wronglen(n, Sd = &dv->normalUncStdDevs, "nuv_std_deviations"))
    return;
  B = &dv->normalUncLowerBnds;
  if (B->length() && wronglen(n, B, "nuv_lower_bounds"))
    return;
  B = &dv->normalUncUpperBnds;
  if (B->length() && wronglen(n, B, "nuv_upper_bounds"))
    return;
}

static void Vgen_NormalUnc(DataVariablesRep *dv, size_t offset)
{
  short bds = 0; // 2 bits indicated LB and UB specs
  size_t i, j, n;
  Real mean, stdev, nudge, lower, upper;
  RealVector *B, *L, *M, *Sd, *U, *V, *IP;
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  n  =  dv->numNormalUncVars;
  M  = &dv->normalUncMeans;      Sd = &dv->normalUncStdDevs;
  L  = &dv->normalUncLowerBnds;  U  = &dv->normalUncUpperBnds;
  IP = &dv->normalUncVars;       V  = &dv->continuousAleatoryUncVars;

  // process lower bounds
  B = &dv->continuousAleatoryUncLowerBnds;
  if (L->length()) {
    Vcopyup(B, L, offset, n); // global = distribution
    bds |= 1;
  }
  else {
    Set_rv(L, -dbl_inf, n); // distribution
    for(j = 0; j < n; ++j)
      (*B)[offset+j] = (*M)[j] - 3.*(*Sd)[j]; // inferred global
  }

  // process upper bounds
  B = &dv->continuousAleatoryUncUpperBnds;
  if (U->length()) {
    Vcopyup(B, U, offset, n); // global = distribution
    bds |= 2;
  }
  else {
    Set_rv(U, dbl_inf, n); // distribution
    for(j = 0; j < n; ++j)
      (*B)[offset+j] = (*M)[j] + 3.*(*Sd)[j]; // inferred global
  }

  // Set initial values and repair to bounds, if needed
  if (IP->length()) { // in this case, don't nudge since not a default value

    dv->uncertainVarsInitPt = true;

    for (i = offset, j = 0; j < n; ++i, ++j)
      if      ((*IP)[j] < (*L)[j]) (*V)[i] =  (*L)[j];
      else if ((*IP)[j] > (*U)[j]) (*V)[i] =  (*U)[j];
      else                         (*V)[i] = (*IP)[j];
  }
  else { // > bds is 0, 1, 2, or 3 (0 ==> no bounds given, nothing more to do)

    // Note: in the case of BoundedNormalRandomVariable, we are initializing
    // to the gaussMean parameter of the unbounded normal and repairing to the
    // bounds.  To assign to the mean of the bounded normal (bounds repair not
    // necessary), use BoundedNormalRandomVariable::mean().
    switch(bds) {
    case 0: // no bounds
      Vcopyup(V, M, offset, n);
      break;
    case 1: // only lower bounds given
      for (i = offset, j = 0; j < n; ++i, ++j)
	if ((*M)[j] <= (*L)[j]) (*V)[i] = (*L)[j] + 0.5*(*Sd)[j];
	else                    (*V)[i] = (*M)[j];
      break;

    case 2: // only upper bounds given
      for (i = offset, j = 0; j < n; ++i, ++j)
	if ((*M)[j] >= (*U)[j]) (*V)[i] = (*U)[j] - 0.5*(*Sd)[j];
	else                    (*V)[i] = (*M)[j];
      break;

    case 3: // both lower and upper bounds given
      for (i = offset, j = 0; j < n; ++i, ++j) {
	lower  = (*L)[j]; upper = (*U)[j];
	nudge  = 0.5 * std::min((*Sd)[j], upper - lower);
	lower += nudge;   upper -= nudge;  mean = (*M)[j];
 	if      (mean < lower) (*V)[i] = lower;
	else if (mean > upper) (*V)[i] = upper;
	else                   (*V)[i] = mean;
      }
      break;
    }
  }
}

static void Vchk_LognormalUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t n;
  RealVector *B, *L, *M, *Sd, *Ef, *Z;

  // lambda/zeta, mean/std_deviation, or mean/error_factor
  n = dv->numLognormalUncVars;
  L = &dv->lognormalUncLambdas;
  M = &dv->lognormalUncMeans;
  if (L->length()) {
    if (wronglen(n, L, "lnuv_lambdas"))
      return;
    if (wronglen(n, Z = &dv->lognormalUncZetas, "lnuv_zetas"))
      return;
  }
  else if (M->length()) {
    if (wronglen(n, M, "lnuv_means"))
      return;
    Sd = &dv->lognormalUncStdDevs;
    Ef = &dv->lognormalUncErrFacts;
    if (Sd->length())
      { if (wronglen(n, Sd, "lnuv_std_deviations")) return; }
    else if (Ef->length())
      { if (wronglen(n, Ef, "lnuv_error_factors"))  return; }
    //else error, but this should be trapped elsewhere
  }

  // lower bounds
  B = &dv->lognormalUncLowerBnds;
  if (B->length() && wronglen(n, B, "lnuv_lower_bounds"))
    return;

  // upper bounds
  B = &dv->lognormalUncUpperBnds;
  if (B->length() && wronglen(n, B, "lnuv_upper_bounds"))
    return;
}

static void Vgen_LognormalUnc(DataVariablesRep *dv, size_t offset)
{
  size_t i, j, n;
  Real mean, stdev, nudge, lower, upper;
  RealVector *B, *Ef, *Lam, *L, *M, *Sd, *U, *V, *Z, *IP;
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  // lambda/zeta, mean/std_deviation, or mean/error_factor
  n  = dv->numLognormalUncVars;    Lam = &dv->lognormalUncLambdas;
  Z  = &dv->lognormalUncZetas;     Ef  = &dv->lognormalUncErrFacts;
  M  = &dv->lognormalUncMeans;     Sd  = &dv->lognormalUncStdDevs;
  L  = &dv->lognormalUncLowerBnds; U   = &dv->lognormalUncUpperBnds; 
  IP = &dv->lognormalUncVars;      V   = &dv->continuousAleatoryUncVars;

  size_t num_Sd = Sd->length(), num_Lam = Lam->length(), num_IP = IP->length(),
    num_L = L->length(), num_U = U->length();

  if (num_IP) dv->uncertainVarsInitPt = true;

  // manage distribution and global bounds.  Global are inferred if
  // distribution are not specified.
  if (!num_L) L->size(n); // inits L to zeros --> default {dist,global}
  Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, offset, n); // global = dist
  B = &dv->continuousAleatoryUncUpperBnds;
  if (num_U) Vcopyup(B, U, offset, n); // global = dist
  else       Set_rv(U, dbl_inf, n);    // default dist; global inferred below

  for (i = offset, j = 0; j < n; ++i, ++j) {

    // extract mean & stdev, if needed
    if (!num_IP || !num_U) {
      if (num_Lam)  // lambda/zeta
	Pecos::LognormalRandomVariable::
	  moments_from_params((*Lam)[j], (*Z)[j], mean, stdev);
      else {
	mean = (*M)[j];
	if (num_Sd) // mean/std_deviation
	  stdev = (*Sd)[j];
	else        // mean/error_factor
	  Pecos::LognormalRandomVariable::
	    std_deviation_from_error_factor(mean, (*Ef)[j], stdev);
      }
    }

    // Repair initial values to bounds, if needed
    if (num_IP) { // in this case, don't nudge since not a default value

      // TO DO: set user spec flag

      if      ((*IP)[j] < (*L)[j]) (*V)[i] =  (*L)[j];
      else if ((*IP)[j] > (*U)[j]) (*V)[i] =  (*U)[j];
      else                         (*V)[i] = (*IP)[j];
    }
    else {
      // repair bounds exceedance in default value, if needed
      lower  = (*L)[j]; upper = (*U)[j];
      nudge  = 0.5 * std::min(stdev, upper - lower);
      lower += nudge;   upper -= nudge;
      if      (mean < lower) (*V)[i] = lower;
      else if (mean > upper) (*V)[i] = upper;
      else                   (*V)[i] = mean;
      // Note: in the case of BoundedLognormalRandomVariable, we are
      // initializing to the mean parameter of the unbounded lognormal
      // and repairing to the bounds.  To assign to the mean of the
      // bounded lognormal (bounds repair not necessary), use
      // BoundedLognormalRandomVariable::mean().
    }

    // infer global bounds if no distribution bounds spec
    if (!num_U)
      (*B)[i] = mean + 3.*stdev;
  }
}

static void Vchk_UniformUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t n;
  RealVector *L, *U;

  n = dv->numUniformUncVars;
  L = &dv->uniformUncLowerBnds;
  U = &dv->uniformUncUpperBnds;
  if (wronglen(n, L, "uuv_lower_bounds") || wronglen(n, U, "uuv_upper_bounds"))
    return;
}

static void Vgen_UniformUnc(DataVariablesRep *dv, size_t offset)
{
  size_t i, j, n;
  Real stdev;
  RealVector *L, *U, *V, *IP;

  n = dv->numUniformUncVars;   IP = &dv->uniformUncVars;
  L = &dv->uniformUncLowerBnds; U = &dv->uniformUncUpperBnds;
  Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, offset, n);
  Vcopyup(&dv->continuousAleatoryUncUpperBnds, U, offset, n);
  V = &dv->continuousAleatoryUncVars;
  if (IP->length()) {
    dv->uncertainVarsInitPt = true;
    for (i = offset, j = 0; j < n; ++i, ++j)
      if      ((*IP)[j] < (*L)[j]) (*V)[i] =  (*L)[j];
      else if ((*IP)[j] > (*U)[j]) (*V)[i] =  (*U)[j];
      else                         (*V)[i] = (*IP)[j];
  }
  else
    for(i = offset, j = 0; j < n; ++i, ++j)
      Pecos::UniformRandomVariable::
	moments_from_params((*L)[j], (*U)[j], (*V)[i], stdev);
}

static void 
Vchk_LoguniformUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t j, n;
  Real Lj, Uj;
  RealVector *L, *U;
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  n = dv->numLoguniformUncVars;
  L = &dv->loguniformUncLowerBnds;
  U = &dv->loguniformUncUpperBnds;
  if (wronglen(n, L, "luuv_lower_bounds") ||
      wronglen(n, U, "luuv_upper_bounds"))
    return;
  for(j = 0; j < n; ++j) {
    Lj = (*L)[j];
    Uj = (*U)[j];
    if (Lj <= 0. || Uj <= 0.) {
      Squawk("loguniform bounds must be positive");
      return;
    }
    if (Lj == dbl_inf || Uj == dbl_inf) {
      Squawk("loguniform bounds must be finite");
      return;
    }
    if (Lj > Uj) {
      Squawk("loguniform lower bound greater than upper bound");
      return;
    }
  }
}

static void Vgen_LoguniformUnc(DataVariablesRep *dv, size_t offset)
{
  size_t i, j, n;
  Real stdev;
  RealVector *L, *U, *V, *IP;

  n = dv->numLoguniformUncVars;   IP = &dv->loguniformUncVars;
  L = &dv->loguniformUncLowerBnds; U = &dv->loguniformUncUpperBnds;
  Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, offset, n);
  Vcopyup(&dv->continuousAleatoryUncUpperBnds, U, offset, n);
  V = &dv->continuousAleatoryUncVars;
  if (IP->length()) {
    dv->uncertainVarsInitPt = true;
    for (i = offset, j = 0; j < n; ++i, ++j)
      if      ((*IP)[j] < (*L)[j]) (*V)[i] =  (*L)[j];
      else if ((*IP)[j] > (*U)[j]) (*V)[i] =  (*U)[j];
      else                         (*V)[i] = (*IP)[j];
  }
  else
    for(i = offset, j = 0; j < n; ++i, ++j)
      Pecos::LoguniformRandomVariable::
	moments_from_params((*L)[j], (*U)[j], (*V)[i], stdev);
}

static void Vchk_TriangularUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t j, n;
  Real Lj, Mj, Uj;
  RealVector *L, *M, *U;

  n = dv->numTriangularUncVars;    M = &dv->triangularUncModes;
  L = &dv->triangularUncLowerBnds; U = &dv->triangularUncUpperBnds;
  if (wronglen(n, L, "tuv_lower_bounds") || wronglen(n, M, "tuv_modes") ||
      wronglen(n, U, "tuv_upper_bounds"))
    return;
  for(j = 0; j < n; ++j) {
    Lj = (*L)[j]; Mj = (*M)[j]; Uj = (*U)[j];
    if (Lj > Mj || Mj > Uj) {
      Squawk("triangular uncertain variables must have\n\t"
	     "tuv_lower_bounds <= tuv_modes <= tuv_upper_bounds");
      return;
    }
  }
}

static void Vgen_TriangularUnc(DataVariablesRep *dv, size_t offset)
{
  size_t i, j, n;
  Real stdev;
  RealVector *L, *M, *U, *V, *IP;

  n = dv->numTriangularUncVars;   IP = &dv->triangularUncVars;
  L = &dv->triangularUncLowerBnds; U = &dv->triangularUncUpperBnds;
  Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, offset, n);
  Vcopyup(&dv->continuousAleatoryUncUpperBnds, U, offset, n);
  V = &dv->continuousAleatoryUncVars;
  if (IP->length()) {
    dv->uncertainVarsInitPt = true;
    for (i = offset, j = 0; j < n; ++i, ++j)
      if      ((*IP)[j] < (*L)[j]) (*V)[i] =  (*L)[j];
      else if ((*IP)[j] > (*U)[j]) (*V)[i] =  (*U)[j];
      else                         (*V)[i] = (*IP)[j];
  }
  else {
    M = &dv->triangularUncModes;
    for(i = offset, j = 0; j < n; ++i, ++j)
      Pecos::TriangularRandomVariable::
	moments_from_params((*L)[j], (*M)[j], (*U)[j], (*V)[i], stdev);
  }
}

static void 
Vchk_ExponentialUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t n = dv->numExponentialUncVars;
  RealVector *B = &dv->exponentialUncBetas;
  if (wronglen(n, B, "euv_betas"))
    return;
}

static void Vgen_ExponentialUnc(DataVariablesRep *dv, size_t offset)
{
  Real mean, stdev;
  RealVector *B, *L, *U, *V, *IP;

  B = &dv->exponentialUncBetas; IP = &dv->exponentialUncVars;
  V = &dv->continuousAleatoryUncVars;
  L = &dv->continuousAleatoryUncLowerBnds;
  U = &dv->continuousAleatoryUncUpperBnds;
  size_t i, j, n = dv->numExponentialUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    Pecos::ExponentialRandomVariable::moments_from_params((*B)[j], mean, stdev);
    (*L)[i] = 0.;
    (*U)[i] = mean + 3.*stdev;
    if (num_IP) (*V)[i] = (*IP)[j];
    else        (*V)[i] = mean;
  }
}

static void Vchk_BetaUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  RealVector *A, *B, *L, *U;

  A = &dv->betaUncAlphas;    B = &dv->betaUncBetas;
  L = &dv->betaUncLowerBnds; U = &dv->betaUncUpperBnds;

  size_t n = dv->numBetaUncVars;
  if (wronglen(n, A, "buv_alphas")       || wronglen(n, B, "buv_betas") ||
      wronglen(n, L, "buv_lower_bounds") || wronglen(n, U, "buv_upper_bounds"))
    return;
}

static void Vgen_BetaUnc(DataVariablesRep *dv, size_t offset)
{
  Real stdev;
  RealVector *A, *B, *L, *U, *V, *IP;

  size_t i, j, n = dv->numBetaUncVars;
  A = &dv->betaUncAlphas;    B = &dv->betaUncBetas;
  L = &dv->betaUncLowerBnds; U = &dv->betaUncUpperBnds;
  V = &dv->continuousAleatoryUncVars; IP = &dv->betaUncVars;
  Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, offset, n);
  Vcopyup(&dv->continuousAleatoryUncUpperBnds, U, offset, n);
  if (IP->length()) {
    dv->uncertainVarsInitPt = true;
    for (i = offset, j = 0; j < n; ++i, ++j)
      if      ((*IP)[j] < (*L)[j]) (*V)[i] =  (*L)[j];
      else if ((*IP)[j] > (*U)[j]) (*V)[i] =  (*U)[j];
      else                         (*V)[i] = (*IP)[j];
  }
  else
    for(i = offset, j = 0; j < n; ++i, ++j)
      Pecos::BetaRandomVariable::
	moments_from_params((*A)[j], (*B)[j], (*L)[j], (*U)[j], (*V)[i], stdev);
}

static void Vchk_GammaUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  RealVector *A, *B;

  size_t n = dv->numGammaUncVars;
  A = &dv->gammaUncAlphas; B = &dv->gammaUncBetas;
  if (wronglen(n, A, "gauv_alphas") || wronglen(n, B, "gauv_betas"))
    return;
}

static void Vgen_GammaUnc(DataVariablesRep *dv, size_t offset)
{
  Real mean, stdev;
  RealVector *A, *B, *L, *U, *V, *IP;

  A = &dv->gammaUncAlphas; B = &dv->gammaUncBetas;
  L = &dv->continuousAleatoryUncLowerBnds;
  U = &dv->continuousAleatoryUncUpperBnds;
  V = &dv->continuousAleatoryUncVars; IP = &dv->gammaUncVars;
  size_t i, j, n = dv->numGammaUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    Pecos::GammaRandomVariable::
      moments_from_params((*A)[j], (*B)[j], mean, stdev);
    (*L)[i] = 0.;
    (*U)[i] = mean + 3.*stdev;
    if (num_IP) (*V)[i] = (*IP)[j];
    else        (*V)[i] = mean;
  }
}

static void Vchk_GumbelUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  RealVector *A, *B;

  size_t n = dv->numGumbelUncVars;
  A = &dv->gumbelUncAlphas; B = &dv->gumbelUncBetas;
  if (wronglen(n, A, "guuv_alphas") || wronglen(n, B, "guuv_betas"))
    return;
}

static void Vgen_GumbelUnc(DataVariablesRep *dv, size_t offset)
{
  Real mean, stdev;
  RealVector *A, *B, *L, *U, *V, *IP;

  A = &dv->gumbelUncAlphas; B = &dv->gumbelUncBetas;
  L = &dv->continuousAleatoryUncLowerBnds;
  U = &dv->continuousAleatoryUncUpperBnds;
  V = &dv->continuousAleatoryUncVars; IP = &dv->gumbelUncVars;
  size_t i, j, n = dv->numGumbelUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    Pecos::GumbelRandomVariable::
      moments_from_params((*A)[j], (*B)[j], mean, stdev);
    (*L)[i] = mean - 3.*stdev;
    (*U)[i] = mean + 3.*stdev;
    if (num_IP) (*V)[i] = (*IP)[j];
    else        (*V)[i] = mean;
  }
}

static void Vchk_FrechetUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  RealVector *A, *B;

  size_t n = dv->numFrechetUncVars;
  A = &dv->frechetUncAlphas; B = &dv->frechetUncBetas;
  if (wronglen(n, A, "fuv_alphas") || wronglen(n, B, "fuv_betas"))
    return;
}

static void Vgen_FrechetUnc(DataVariablesRep *dv, size_t offset)
{
  Real mean, stdev;
  RealVector *A, *B, *L, *U, *V, *IP;

  A = &dv->frechetUncAlphas; B = &dv->frechetUncBetas;
  L = &dv->continuousAleatoryUncLowerBnds;
  U = &dv->continuousAleatoryUncUpperBnds;
  V = &dv->continuousAleatoryUncVars; IP = &dv->frechetUncVars;
  size_t i, j, n = dv->numFrechetUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    Pecos::FrechetRandomVariable::
      moments_from_params((*A)[j], (*B)[j], mean, stdev);
    (*L)[i] = 0.;
    (*U)[i] = mean + 3.*stdev;
    if (num_IP) (*V)[i] = (*IP)[j];
    else        (*V)[i] = mean;
  }
}

static void Vchk_WeibullUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  RealVector *A, *B;

  size_t n = dv->numWeibullUncVars;
  A = &dv->weibullUncAlphas; B = &dv->weibullUncBetas;
  if (wronglen(n, A, "wuv_alphas") || wronglen(n, B, "wuv_betas"))
    return;
}

static void Vgen_WeibullUnc(DataVariablesRep *dv, size_t offset)
{
  Real mean, stdev;
  RealVector *A, *B, *L, *U, *V, *IP;

  A = &dv->weibullUncAlphas; B = &dv->weibullUncBetas;
  L = &dv->continuousAleatoryUncLowerBnds;
  U = &dv->continuousAleatoryUncUpperBnds;
  V = &dv->continuousAleatoryUncVars; IP = &dv->weibullUncVars;
  size_t i, j, n = dv->numWeibullUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    Pecos::WeibullRandomVariable::
      moments_from_params((*A)[j], (*B)[j], mean, stdev);
    (*L)[i] = 0.;
    (*U)[i] = mean + 3.*stdev;
    if (num_IP) (*V)[i] = (*IP)[j];
    else        (*V)[i] = mean;
  }
}


/// Check the histogram bin input data, normalize the counts and
/// populate the histogramUncBinPairs map data structure; map keys are
/// guaranteed unique since the abscissas must increase
static void 
Vchk_HistogramBinUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  IntArray *nhbp;
  RealVector *hba, *hbo, *hbc;
  int nhbpi, avg_nhbpi;
  size_t i, j, num_a, num_o, num_c, m, n, tothbp, cntr;
  Real x, y, bin_width, count_sum;

  if (hba = vi->hba) { // abscissas are required
    num_a = hba->length();                         // abscissas
    hbo = vi->hbo; num_o = (hbo) ? hbo->length() : 0; // ordinates
    hbc = vi->hbc; num_c = (hbc) ? hbc->length() : 0; // counts
    if (num_o && num_o != num_a) {
      Squawk("Expected %d ordinates, not %d", num_a, num_o);
      return;
    }
    if (num_c && num_c != num_a) {
      Squawk("Expected %d counts, not %d", num_a, num_c);
      return;
    }
    bool key;
    if (nhbp = vi->nhbp) {
      key = true;
      m = nhbp->size();
      //dv->numHistogramBinUncVars = m;
      for(i=tothbp=0; i<m; ++i) {
	tothbp += nhbpi = (*nhbp)[i];
	if (nhbpi < 2) {
	  Squawk("pairs_per_variable must be >= 2");
	  return;
	}
      }
      if (num_a != tothbp) {
	Squawk("Expected %d abscissas, not %d", tothbp, num_a);
	return;
      }
    }
    else {
      key = false;
      m = dv->numHistogramBinUncVars;
      if (num_a % m) {
	Squawk("Number of abscissas (%d) not evenly divisible by number of variables (%d); Use pairs_per_variable for unequal apportionment", num_a, m);
	return;
      }
      else
	avg_nhbpi = num_a / m;
    }
    RealRealMapArray& hbp = dv->histogramUncBinPairs;
    hbp.resize(m);
    for (i=cntr=0; i<m; ++i) {
      nhbpi = (key) ? (*nhbp)[i] : avg_nhbpi;
      // hbpi is map<Real value, Real probability> for a single variable i
      RealRealMap& hbpi = hbp[i];
      count_sum = 0.;
      for (j=0; j<nhbpi; ++j, ++cntr) {
	Real x = (*hba)[cntr];                          // abscissas
	Real y = (num_o) ? (*hbo)[cntr] : (*hbc)[cntr]; // ordinates/counts
	if (j<nhbpi-1) {
	  Real bin_width = (*hba)[cntr+1] - x;
	  if (bin_width <= 0.) {
	    Squawk("histogram bin x values must increase");
	    return;
	  }
	  if (y <= 0.) {
	    Squawk("nonpositive intermediate histogram bin y value");
	    return;
	  }
	  if (num_o) // convert from ordinates (probability density) to counts
	    y *= bin_width;
	  count_sum += y;
	}
	else if (y != 0) {
	  Squawk("histogram bin y values must end with 0");
	  return;
	}
	// insert without checking since keys (abscissas) must increase
	hbpi[x] = y;
      }
      // normalize counts to sum to 1, omitting last value
      RRMCIter it_end = --(hbpi.end());
      for (RRMIter it = hbpi.begin(); it != it_end; ++it)
	it->second /= count_sum;
    }
  }
}

/// Infer lower/upper bounds for histogram and set initial variable
/// values based on initial_point or moments, snapping to bounds as
/// needed.  (Histogram bin doesn't have lower/upper bounds specifcation)
static void Vgen_HistogramBinUnc(DataVariablesRep *dv, size_t offset)
{
  RealVector *L, *U, *V, *IP;
  Real mean, stdev;

  L = &dv->continuousAleatoryUncLowerBnds;
  U = &dv->continuousAleatoryUncUpperBnds;
  V = &dv->continuousAleatoryUncVars; IP = &dv->histogramBinUncVars;
  const RealRealMapArray& A = dv->histogramUncBinPairs;
  size_t i, j, n = dv->numHistogramBinUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    // the pairs are sorted, so take the first and next to last
    // (omitting the trailing zero)
    const RealRealMap& hist_bin_pairs = A[j];
    (*L)[i] = hist_bin_pairs.begin()->first;
    (*U)[i] = (--hist_bin_pairs.end())->first;
    if (num_IP) {
      if      ((*IP)[j] < (*L)[i]) (*V)[i] =  (*L)[i];
      else if ((*IP)[j] > (*U)[i]) (*V)[i] =  (*U)[i];
      else                         (*V)[i] = (*IP)[j];
    }
    else
      Pecos::HistogramBinRandomVariable::
	moments_from_params(hist_bin_pairs, (*V)[i], stdev);
  }
}

static void Vchk_PoissonUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t n = dv->numPoissonUncVars;
  RealVector *A = &dv->poissonUncLambdas;
  if (wronglen(n, A, "lambdas"))
    return;
}

static void Vgen_PoissonUnc(DataVariablesRep *dv, size_t offset)
{
  IntVector *L, *U, *V, *IP;
  Real mean, std_dev;
  RealVector *Lam;

  L = &dv->discreteIntAleatoryUncLowerBnds;
  U = &dv->discreteIntAleatoryUncUpperBnds;
  V = &dv->discreteIntAleatoryUncVars;
  IP = &dv->poissonUncVars; Lam = &dv->poissonUncLambdas;
  size_t i, j, n = dv->numPoissonUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    Pecos::PoissonRandomVariable::moments_from_params((*Lam)[j], mean, std_dev);
    (*L)[i] = 0;
    (*U)[i] = (int)std::ceil(mean + 3.*std_dev);
    if (num_IP) (*V)[i] = (*IP)[j];
    else        (*V)[i] = (int)mean;
  }
}

static void Vchk_BinomialUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t n = dv->numBinomialUncVars;
  RealVector *A = &dv->binomialUncProbPerTrial;
  IntVector  *B = &dv->binomialUncNumTrials;
  if (wronglen(n, A, "prob_per_trial") ||  wronglen(n, B, "num_trials"))
    return;
}

static void Vgen_BinomialUnc(DataVariablesRep *dv, size_t offset)
{
  IntVector *L, *NT, *U, *V, *IP;
  Real mean, std_dev;
  RealVector *Pr;

  L = &dv->discreteIntAleatoryUncLowerBnds;
  U = &dv->discreteIntAleatoryUncUpperBnds;
  V = &dv->discreteIntAleatoryUncVars; IP = &dv->binomialUncVars;
  NT = &dv->binomialUncNumTrials;      Pr = &dv->binomialUncProbPerTrial;
  size_t i, j, n = dv->numBinomialUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    (*L)[i] = 0;
    (*U)[i] = (*NT)[j];
    if (num_IP) {
      if ((*IP)[j] > (*U)[i]) (*V)[i] =  (*U)[i];
      else                    (*V)[i] = (*IP)[j];
    }
    else {
      Pecos::BinomialRandomVariable::
	moments_from_params((*NT)[j], (*Pr)[j], mean, std_dev);
      (*V)[i] = (int)mean;
    }
  }
}

static void 
Vchk_NegBinomialUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t n = dv->numNegBinomialUncVars;
  RealVector *A = &dv->negBinomialUncProbPerTrial;
  IntVector  *B = &dv->negBinomialUncNumTrials;
  if (wronglen(n, A, "prob_per_trial") || wronglen(n, B, "num_trials"))
    return;
}

static void Vgen_NegBinomialUnc(DataVariablesRep *dv, size_t offset)
{
  IntVector *L, *NT, *U, *V, *IP;
  Real mean, std_dev;
  RealVector *Pr;

  L = &dv->discreteIntAleatoryUncLowerBnds;
  U = &dv->discreteIntAleatoryUncUpperBnds;
  V = &dv->discreteIntAleatoryUncVars; IP = &dv->negBinomialUncVars;
  NT = &dv->negBinomialUncNumTrials;   Pr = &dv->negBinomialUncProbPerTrial;
  size_t i, j, n = dv->numNegBinomialUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    Pecos::NegBinomialRandomVariable::
      moments_from_params((*NT)[j], (*Pr)[j], mean, std_dev);
    (*L)[i] = (*NT)[j];
    (*U)[i] = (int)std::ceil(mean + 3.*std_dev);
    if (num_IP) {
      if ((*IP)[j] < (*L)[i]) (*V)[i] =  (*L)[i];
      else                    (*V)[i] = (*IP)[j];
    }
    else                      (*V)[i] = (int)mean;
  }
}

static void Vchk_GeometricUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t n = dv->numGeometricUncVars;
  RealVector *A = &dv->geometricUncProbPerTrial;
  if (wronglen(n, A, "prob_per_trial"))
    return;
}

static void Vgen_GeometricUnc(DataVariablesRep *dv, size_t offset)
{
  IntVector *L, *U, *V, *IP;
  Real mean, std_dev;
  RealVector *Pr;

  L = &dv->discreteIntAleatoryUncLowerBnds;
  U = &dv->discreteIntAleatoryUncUpperBnds;
  V = &dv->discreteIntAleatoryUncVars; IP = &dv->geometricUncVars;
  Pr = &dv->geometricUncProbPerTrial;
  size_t i, j, n = dv->numGeometricUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    (*L)[i] = 0;
    Pecos::GeometricRandomVariable::
      moments_from_params((*Pr)[j], mean, std_dev);
    (*U)[i] = (int)std::ceil(mean + 3.*std_dev);
    if (num_IP) (*V)[i] = (*IP)[j];
    else        (*V)[i] = (int)mean;
  }
}

static void Vchk_HyperGeomUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  IntVector *A, *B, *C;

  size_t n = dv->numHyperGeomUncVars;
  A = &dv->hyperGeomUncTotalPop;
  B = &dv->hyperGeomUncSelectedPop;
  C = &dv->hyperGeomUncNumDrawn;
  if (wronglen(n, A, "total_population")    ||
      wronglen(n, B, "selected_population") || wronglen(n, C, "num_drawn"))
    return;
}

static void Vgen_HyperGeomUnc(DataVariablesRep *dv, size_t offset)
{
  IntVector *L, *ND, *NS, *TP, *U, *V, *IP;
  Real mean, std_dev;

  L = &dv->discreteIntAleatoryUncLowerBnds;
  U = &dv->discreteIntAleatoryUncUpperBnds;
  V = &dv->discreteIntAleatoryUncVars;
  ND = &dv->hyperGeomUncNumDrawn; NS = &dv->hyperGeomUncSelectedPop;
  TP = &dv->hyperGeomUncTotalPop; IP = &dv->hyperGeomUncVars;
  size_t i, j, n = dv->numHyperGeomUncVars, num_IP = IP->length();
  int d, s;
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    (*L)[i] = 0;
    d = (*ND)[j]; s = (*NS)[j];
    (*U)[i] = std::min(d, s);
    if (num_IP) {
      if ((*IP)[j] > (*U)[i]) (*V)[i] =  (*U)[i];
      else                    (*V)[i] = (*IP)[j];
    }
    else {
      Pecos::HypergeometricRandomVariable::
	moments_from_params((*TP)[j], s, d, mean, std_dev);
      (*V)[i] = (int)mean;
    }
  }
}


/// Check the histogram point integer input data, normalize the
/// counts, and populate DataVariables::histogramUncPointIntPairs; map
/// keys are guaranteed unique since the abscissas must increase
static void 
Vchk_HistogramPtIntUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  IntArray *nhpip;
  IntVector *hpia;
  RealVector *hpic;
  int nhppi, avg_nhppi;
  size_t i, j, num_a, num_c, m, n, tothpp, cntr;
  Real y, bin_width, count_sum;
  int x;

  if (hpia = vi->hpia) {
    num_a = hpia->length();              // abscissas
    hpic = vi->hpic; num_c = hpic->length(); // counts
    if (num_c != num_a) {
      Squawk("Expected %d point counts, not %d", num_a, num_c);
      return;
    }
    bool key;
    if (nhpip = vi->nhpip) {
      key = true;
      m = nhpip->size();
      //dv->numHistogramPtUncVars = m;
      for(i=tothpp=0; i<m; ++i) {
	tothpp += nhppi = (*nhpip)[i];
	if (nhppi < 1) {
	  Squawk("pairs_per_variable must be >= 1");
	  return;
	}
      }
      if (num_a != tothpp) {
	Squawk("Expected %d point abscissas, not %d", tothpp, num_a);
	return;
      }
    }
    else {
      key = false;
      m = dv->numHistogramPtIntUncVars;
      if (num_a % m) {
	Squawk("Number of abscissas (%d) not evenly divisible by number of variables (%d); Use pairs_per_variable for unequal apportionment", num_a, m);
	return;
      }
      else
	avg_nhppi = num_a / m;
    }
    IntRealMapArray& hpp = dv->histogramUncPointIntPairs;
    hpp.resize(m);
    for (i=cntr=0; i<m; ++i) {
      nhppi = (key) ? (*nhpip)[i] : avg_nhppi;
      // hbpi is map<Int value, Real probability> for a single variable i
      IntRealMap& hppi = hpp[i];
      count_sum = 0.;
      for (j=0; j<nhppi; ++j, ++cntr) {
	int x = (*hpia)[cntr]; // abscissas
	Real y = (*hpic)[cntr]; // counts
	if (j<nhppi-1 && x >= (*hpia)[cntr+1]) {
	  Squawk("histogram point x values must increase");
	  return;
	}
	if (y <= 0.) {
	  Squawk("nonpositive intermediate histogram point y value");
	  return;
	}
	hppi[x] = y;
	count_sum += y;
      }
      // normalize counts to sum to 1
      IRMCIter it_end = hppi.end();
      for (IRMIter it = hppi.begin(); it != it_end; ++it)
	it->second /= count_sum;
    }
  }
}


/// Use the integer-valued point histogram data to initialize the lower,
/// upper, and initial values of the variables, using value closest to
/// mean if no initial point.
static void Vgen_HistogramPtIntUnc(DataVariablesRep *dv, size_t offset)
{
  const IntRealMapArray& A = dv->histogramUncPointIntPairs; 

  IntVector& L = dv->discreteIntAleatoryUncLowerBnds;
  IntVector& U = dv->discreteIntAleatoryUncUpperBnds;
  IntVector& V = dv->discreteIntAleatoryUncVars;
  IntVector&  IP = dv->histogramPointIntUncVars;

  size_t i, j, k, last, n = dv->numHistogramPtIntUncVars;
  size_t num_IP = IP.length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    const IntRealMap& hist_pt_prs = A[j];
    L[i] = hist_pt_prs.begin()->first;
    U[i] = (--hist_pt_prs.end())->first;
    if (num_IP) {
      if      (IP[j] < L[i]) V[i] =  L[i];
      else if (IP[j] > U[i]) V[i] =  U[i];
      else                   V[i] = IP[j];
    }
    else {
      Real mean, stdev;
      Pecos::HistogramPtRandomVariable::
	moments_from_params(hist_pt_prs, mean, stdev);
      if (hist_pt_prs.size() == 1)
	V[i] = hist_pt_prs.begin()->first;
      else {
	IRMCIter it = hist_pt_prs.begin(), it_end = hist_pt_prs.end();
	// find value immediately right of mean (can't be past the end)
	for( ; it != it_end, it->first <= mean; ++it);
	// bracket the mean
	int right_val = it->first;
	int left_val = (--it)->first;
	// initialize with value closest to mean
	V[i] = (mean - right_val < left_val - mean) ? right_val : left_val;
      }
    }
  }
}

/// Check the histogram point string input data, normalize the counts,
/// and populate DataVariables::histogramUncPointStrPairs; map keys
/// are guaranteed unique since the abscissas must increase
/// (lexicographically)
static void 
Vchk_HistogramPtStrUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  IntArray *nhpsp;
  StringArray *hpsa;
  RealVector *hpsc;
  int nhppi, avg_nhppi;
  size_t i, j, num_a, num_c, m, n, tothpp, cntr;
  Real x, y, bin_width, count_sum;

  if (hpsa = vi->hpsa) {
    num_a = hpsa->size();              // abscissas
    hpsc = vi->hpsc; num_c = hpsc->length(); // counts
    if (num_c != num_a) {
      Squawk("Expected %d point counts, not %d", num_a, num_c);
      return;
    }
    bool key;
    if (nhpsp = vi->nhpsp) {
      key = true;
      m = nhpsp->size();
      //dv->numHistogramPtUncVars = m;
      for(i=tothpp=0; i<m; ++i) {
	tothpp += nhppi = (*nhpsp)[i];
	if (nhppi < 1) {
	  Squawk("pairs_per_variable must be >= 1");
	  return;
	}
      }
      if (num_a != tothpp) {
	Squawk("Expected %d point abscissas, not %d", tothpp, num_a);
	return;
      }
    }
    else {
      key = false;
      m = dv->numHistogramPtStrUncVars;
      if (num_a % m) {
	Squawk("Number of abscissas (%d) not evenly divisible by number of variables (%d); Use pairs_per_variable for unequal apportionment", num_a, m);
	return;
      }
      else
	avg_nhppi = num_a / m;
    }
    StringRealMapArray& hpp = dv->histogramUncPointStrPairs;
    hpp.resize(m);
    for (i=cntr=0; i<m; ++i) {
      nhppi = (key) ? (*nhpsp)[i] : avg_nhppi;
      // hbpi is map<Real value, Real probability> for a single variable i
      StringRealMap& hppi = hpp[i];
      count_sum = 0.;
      for (j=0; j<nhppi; ++j, ++cntr) {
	String x = (*hpsa)[cntr]; // abscissas
	Real y = (*hpsc)[cntr]; // counts
	if (j<nhppi-1 && x >= (*hpsa)[cntr+1]) {
	  Squawk("histogram point x values must increase");
	  return;
	}
	if (y <= 0.) {
	  Squawk("nonpositive intermediate histogram point y value");
	  return;
	}
	hppi[x] = y;
	count_sum += y;
      }
      // normalize counts to sum to 1
      SRMCIter it_end = hppi.end();
      for (SRMIter it = hppi.begin(); it != it_end; ++it)
	it->second /= count_sum;
    }
  }
}


/// Use the string-valued point histogram data to initialize the lower,
/// upper, and initial values of the variables, using index closest to
/// mean index if no initial point.
static void Vgen_HistogramPtStrUnc(DataVariablesRep *dv, size_t offset)
{
  const StringRealMapArray& A = dv->histogramUncPointStrPairs; 

  StringArray& L = dv->discreteStrAleatoryUncLowerBnds;
  StringArray& U = dv->discreteStrAleatoryUncUpperBnds;
  StringArray& V = dv->discreteStrAleatoryUncVars;
  StringArray& IP = dv->histogramPointStrUncVars;

  size_t i, j, k, last, n = dv->numHistogramPtStrUncVars;
  size_t num_IP = IP.size();

  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    const StringRealMap& hist_pt_prs = A[j];
    L[i] = hist_pt_prs.begin()->first;
    U[i] = (--hist_pt_prs.end())->first;
    if (num_IP) {
      if      (IP[j] < L[i]) V[i] =  L[i];
      else if (IP[j] > U[i]) V[i] =  U[i];
      else                   V[i] = IP[j];
    }
    else {
      // for string-valued histograms, mean and stddev are of
      // zero-based indices from beginning of the map
      Real mean, stdev;
      Pecos::HistogramPtRandomVariable::
	moments_from_params(hist_pt_prs, mean, stdev);
      if (hist_pt_prs.size() == 1)
	V[i] = hist_pt_prs.begin()->first;
      else {
	size_t mean_index = boost::math::iround(mean);
	SRMCIter it = hist_pt_prs.begin();
	std::advance(it, mean_index);
	// initialize with value closest to mean
	V[i] = it->first;
      }
    }
  }
}


/// Check the histogram point integer real data, normalize the counts,
/// and populate DataVariables::histogramUncPointRealPairs; map keys
/// are guaranteed unique since the abscissas must increase
static void 
Vchk_HistogramPtRealUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  IntArray *nhprp;
  RealVector *hpra, *hprc;
  int nhppi, avg_nhppi;
  size_t i, j, num_a, num_c, m, n, tothpp, cntr;
  Real x, y, bin_width, count_sum;

  if (hpra = vi->hpra) {
    num_a = hpra->length();              // abscissas
    hprc = vi->hprc; num_c = hprc->length(); // counts
    if (num_c != num_a) {
      Squawk("Expected %d point counts, not %d", num_a, num_c);
      return;
    }
    bool key;
    if (nhprp = vi->nhprp) {
      key = true;
      m = nhprp->size();
      //dv->numHistogramPtUncVars = m;
      for(i=tothpp=0; i<m; ++i) {
	tothpp += nhppi = (*nhprp)[i];
	if (nhppi < 1) {
	  Squawk("pairs_per_variable must be >= 1");
	  return;
	}
      }
      if (num_a != tothpp) {
	Squawk("Expected %d point abscissas, not %d", tothpp, num_a);
	return;
      }
    }
    else {
      key = false;
      m = dv->numHistogramPtRealUncVars;
      if (num_a % m) {
	Squawk("Number of abscissas (%d) not evenly divisible by number of variables (%d); Use pairs_per_variable for unequal apportionment", num_a, m);
	return;
      }
      else
	avg_nhppi = num_a / m;
    }
    RealRealMapArray& hpp = dv->histogramUncPointRealPairs;
    hpp.resize(m);
    for (i=cntr=0; i<m; ++i) {
      nhppi = (key) ? (*nhprp)[i] : avg_nhppi;
      // hbpi is map<Real value, Real probability> for a single variable i
      RealRealMap& hppi = hpp[i];
      count_sum = 0.;
      for (j=0; j<nhppi; ++j, ++cntr) {
	Real x = (*hpra)[cntr]; // abscissas
	Real y = (*hprc)[cntr]; // counts
	if (j<nhppi-1 && x >= (*hpra)[cntr+1]) {
	  Squawk("histogram point x values must increase");
	  return;
	}
	if (y <= 0.) {
	  Squawk("nonpositive intermediate histogram point y value");
	  return;
	}
	hppi[x] = y;
	count_sum += y;
      }
      // normalize counts to sum to 1
      RRMCIter it_end = hppi.end();
      for (RRMIter it = hppi.begin(); it != it_end; ++it)
	it->second /= count_sum;
    }
  }
}


/// Use the real-valued point histogram data to initialize the lower,
/// upper, and initial values of the variables, using value closest to
/// mean if no initial point.
static void Vgen_HistogramPtRealUnc(DataVariablesRep *dv, size_t offset)
{
  const RealRealMapArray& A = dv->histogramUncPointRealPairs; 

  RealVector& L = dv->discreteRealAleatoryUncLowerBnds;
  RealVector& U = dv->discreteRealAleatoryUncUpperBnds;
  RealVector& V = dv->discreteRealAleatoryUncVars;
  RealVector& IP = dv->histogramPointRealUncVars;

  size_t i, j, k, last, n = dv->numHistogramPtRealUncVars;
  size_t num_IP = IP.length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    const RealRealMap& hist_pt_prs = A[j];
    L[i] = hist_pt_prs.begin()->first;
    U[i] = (--hist_pt_prs.end())->first;
    if (num_IP) {
      if      (IP[j] < L[i]) V[i] =  L[i];
      else if (IP[j] > U[i]) V[i] =  U[i];
      else                   V[i] = IP[j];
    }
    else {
      Real mean, stdev;
      Pecos::HistogramPtRandomVariable::
	moments_from_params(hist_pt_prs, mean, stdev);
      if (hist_pt_prs.size() == 1)
	V[i] = hist_pt_prs.begin()->first;
      else {
	RRMCIter it = hist_pt_prs.begin(), it_end = hist_pt_prs.end();
	// find value immediately right of mean (can't be past the end)
	for( ; it != it_end, it->first <= mean; ++it);
	// bracket the mean
	Real right_val = it->first;
	Real left_val = (--it)->first;
	// initialize with value closest to mean
	V[i] = (mean - right_val < left_val - mean) ? right_val : left_val;
      }
    }
  }
}


/// Check the continuous interval uncertain input data and populate
/// DataVariables::continuousIntervalUncBasicProbs; map keys (real
/// intervals) are checked for uniqueness because we don't have a
/// theoretically sound way to combine duplicate intervals
static void
Vchk_ContinuousIntervalUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t i, j, k, m, num_p = 0, num_lb, num_ub;
  IntArray *nI;
  int tot_nI, nIi, avg_nI;
  Real lb, lbj, ub, ubj, default_p;
  RealVector *Ilb, *Iub, *Ip;
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  if ((Ilb = vi->CIlb) && (Iub = vi->CIub)) {
    num_lb = Ilb->length(); // interval lower_bounds
    num_ub = Iub->length(); // interval upper_bounds

    // error check on array lengths; bounds are reqd, probs are optional
    if ((Ip = vi->CIp)) {
      num_p = Ip->length(); // interval_probs
      if (num_lb != num_p || num_ub != num_p) {
	Squawk("Expected as many lower bounds (%d) and upper bounds (%d) as probabilities (%d)", num_lb, num_ub, num_p);
	return;
      }
    }
    else if (num_lb != num_ub) {
      Squawk("Expected as many lower bounds (%d) as upper bounds (%d)", num_lb, num_ub);
      return;
    }

    // define apportionment
    bool key;
    if (nI = vi->nCI) {
      key = true;
      m = nI->size();
      if (m != dv->numContinuousIntervalUncVars) {
	Squawk("Expected %d numbers for num_intervals, but got %d",
	       dv->numContinuousIntervalUncVars, m);
	return;
      }
      for(i=tot_nI=0; i<m; ++i) {
	tot_nI += nIi = (*nI)[i];
	if (nIi < 1) {
	  Squawk("num_intervals values should be positive");
	  return;
	}
      }
      if ( (num_p && wronglen(tot_nI,  Ip, "interval_probs") ) ||
	   wronglen(tot_nI, Ilb, "interval lower_bounds") ||
	   wronglen(tot_nI, Iub, "interval upper_bounds"))
	return;
    }
    else {
      key = false;
      m = dv->numContinuousIntervalUncVars;
      if (num_lb % m) {
	Squawk("Number of bounds (%d) not evenly divisible by number of variables (%d); Use num_intervals for unequal apportionment", num_lb, m);
	return;
      }
      else
	avg_nI = num_lb / m;
    }
    RealRealPairRealMapArray& P = dv->continuousIntervalUncBasicProbs;   
    P.resize(m);
    for(i = k = 0; i < m; ++i) {
      nIi = (key) ? (*nI)[i] : avg_nI;
      RealRealPairRealMap& Pi = P[i];  // map from an interval to a probability
      lb = dbl_inf;
      ub = -dbl_inf;
      if (!num_p) 
        default_p = 1./nIi; // default = equal probability per cell
      else {
        double total_prob=0.0; 
        size_t s = k;
        for(j=0; j<nIi; ++j, ++s) {  // normalize the probabilities to sum to one
          total_prob+=(*Ip)[s];
        }         
        if (fabs(total_prob-1.0) > 1.E-10) {
          s = k;
          {for(j=0; j<nIi; ++j,++s)  // normalize the probabilities to sum to one
            (*Ip)[s]/=total_prob;
          }       
          Warn("Renormalized probability assignments to sum to one for variable %d",i);
        } 
      }
      for(j=0; j<nIi; ++j, ++k) {
	lbj = (*Ilb)[k];
	ubj = (*Iub)[k];
	RealRealPair interval(lbj, ubj);
	Real probability = (num_p) ? (*Ip)[k] : default_p;
	if (!Pi.insert(make_pair(interval, probability)).second)
	  Squawk("Continuous interval [%g, %g] specified more than once for variable %d", interval.first, interval.second, i);
	if (lb > lbj) lb = lbj;
	if (ub < ubj) ub = ubj;
        if (lbj > ubj)
	  Squawk("Upper bound less than lower bound: [%g, %g] for interval variable %d", lbj, ubj,i);
      }
      if (lb > ub)
	Squawk("Inconsistent interval uncertain bounds: %g > %g", lb, ub);
    }
  }
}

static void Vgen_ContinuousIntervalUnc(DataVariablesRep *dv, size_t offset)
{
  Real lb, lbk, ub, ubk, stdev;
  RealVector *ceuLB, *ceuUB, *V, *IP;
  Real dbl_inf = std::numeric_limits<Real>::infinity();

  ceuLB = &dv->continuousEpistemicUncLowerBnds;
  ceuUB = &dv->continuousEpistemicUncUpperBnds;
  V     = &dv->continuousEpistemicUncVars;
  const RealRealPairRealMapArray& P = dv->continuousIntervalUncBasicProbs;
  IP    = &dv->continuousIntervalUncVars;
  size_t i, j, n = dv->numContinuousIntervalUncVars,
    num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    lb = dbl_inf; ub = -dbl_inf;
    const RealRealPairRealMap& Pj = P[j];
    RealRealPairRealMap::const_iterator it = Pj.begin();
    RealRealPairRealMap::const_iterator it_end = Pj.end();
    for ( ; it != it_end; ++it) {
      const RealRealPair& interval = it->first;
      lbk = interval.first; 
      ubk = interval.second;
      if (lb > lbk) lb = lbk;
      if (ub < ubk) ub = ubk;
    }
    (*ceuLB)[i] = lb; (*ceuUB)[i] = ub;
    if (num_IP) {
      if      ((*IP)[j] < lb) (*V)[i] = lb;
      else if ((*IP)[j] > ub) (*V)[i] = ub;
      else                    (*V)[i] = (*IP)[j];
    }
    else
      Pecos::UniformRandomVariable::moments_from_params(lb, ub, (*V)[i], stdev);
    // TO DO: if disjoint cells, repair V[i] to lie inside nearest cell
  }
}

/// Check the discrete interval uncertain input data and populate
/// DataVariables::discreteIntervalUncBasicProbs; map keys (integer
/// intervals) are checked for uniqueness because we don't have a
/// theoretically sound way to combine duplicate intervals
static void 
Vchk_DiscreteIntervalUnc(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  size_t i, j, k, m, num_p = 0, num_lb, num_ub;
  IntArray *nI;
  int tot_nI, nIi, avg_nI, lb, lbj, ub, ubj;
  Real default_p;
  RealVector *Ip;
  IntVector *Ilb, *Iub;

  if ((Ilb = vi->DIlb) && (Iub = vi->DIub)) {
    num_lb = Ilb->length(); // interval lower_bounds
    num_ub = Iub->length(); // interval upper_bounds

    // error check on array lengths; bounds are reqd, probs are optional
    if ((Ip = vi->DIp)) {
      num_p = Ip->length(); // interval_probs
      if (num_lb != num_p || num_ub != num_p) {
	Squawk("Expected as many lower bounds (%d) and upper bounds (%d) as probabilities (%d)", num_lb, num_ub, num_p);
	return;
      }
    }
    else if (num_lb != num_ub) {
      Squawk("Expected as many lower bounds (%d) as upper bounds (%d)", num_lb, num_ub);
      return;
    }

    // define apportionment
    bool key;
    if (nI = vi->nDI) {
      key = true;
      m = nI->size();
      if (m != dv->numDiscreteIntervalUncVars) {
	Squawk("Expected %d numbers for num_intervals, but got %d",
	       dv->numDiscreteIntervalUncVars, m);
	return;
      }
      for(i=tot_nI=0; i<m; ++i) {
	tot_nI += nIi = (*nI)[i];
	if (nIi < 1) {
	  Squawk("num_intervals values should be positive");
	  return;
	}
      }
      if ( (num_p && wronglen(tot_nI,  Ip, "interval_probs") ) ||
	   wronglen(tot_nI, Ilb, "interval lower_bounds") ||
	   wronglen(tot_nI, Iub, "interval upper_bounds"))
	return;
    }
    else {
      key = false;
      m = dv->numDiscreteIntervalUncVars;
      if (num_lb % m) {
	Squawk("Number of bounds (%d) not evenly divisible by number of variables (%d); Use num_intervals for unequal apportionment", num_lb, m);
	return;
      }
      else
	avg_nI = num_lb / m;
    }
    IntIntPairRealMapArray& P = dv->discreteIntervalUncBasicProbs;   
    P.resize(m);
    for(i = k = 0; i < m; ++i) {
      nIi = (key) ? (*nI)[i] : avg_nI;
      IntIntPairRealMap& Pi = P[i];   // map from an interval to a probability
      lb = INT_MAX; ub = INT_MIN;
      if (!num_p) default_p = 1./nIi; // default = equal probability per cell
      for(j=0; j<nIi; ++j, ++k) {
	lbj = (*Ilb)[k];      
	ubj = (*Iub)[k];
	IntIntPair interval(lbj, ubj);
	Real probability =  (num_p) ? (*Ip)[k] : default_p;
	if (!Pi.insert(make_pair(interval, probability)).second)
	  Squawk("Discrete interval [%d, %d] specified more than once for variable %d", interval.first, interval.second, i);
	if (lb > lbj) lb = lbj;
	if (ub < ubj) ub = ubj;
      }
      if (lb > ub)
	Squawk("Inconsistent interval uncertain bounds: %g > %g", lb, ub);
    }
  }
}

static void Vgen_DiscreteIntervalUnc(DataVariablesRep *dv, size_t offset)
{
  int lb, lbk, ub, ubk, stdev;
  IntVector *deuLB, *deuUB, *V, *IP;

  deuLB = &dv->discreteIntEpistemicUncLowerBnds;
  deuUB = &dv->discreteIntEpistemicUncUpperBnds;
  V     = &dv->discreteIntEpistemicUncVars;
  const IntIntPairRealMapArray& P = dv->discreteIntervalUncBasicProbs;
  IP    = &dv->discreteIntervalUncVars;
  size_t i, j, n = dv->numDiscreteIntervalUncVars, num_IP = IP->length();
  if (num_IP) dv->uncertainVarsInitPt = true;

  for(i = offset, j = 0; j < n; ++i, ++j) {
    ub = INT_MIN; lb = INT_MAX;
    const IntIntPairRealMap& Pj = P[j];
    IntIntPairRealMap::const_iterator it = Pj.begin();
    IntIntPairRealMap::const_iterator it_end = Pj.end();
    for ( ; it != it_end; ++it) {
      const IntIntPair& interval = it->first;
      lbk = interval.first; 
      ubk = interval.second;
      if (lb > lbk) lb = lbk;
      if (ub < ubk) ub = ubk;
    }    
    (*deuLB)[i] = lb; (*deuUB)[i] = ub;
    if (num_IP) {
      if      ((*IP)[j] < lb) (*V)[i] = lb;
      else if ((*IP)[j] > ub) (*V)[i] = ub;
      else                    (*V)[i] = (*IP)[j];
    }
    else
      (*V)[i] = (lb + ub) / 2; // int truncation if odd sum
    // TO DO: if disjoint cells, repair V[offset] to lie inside nearest cell
  }
}


/// validate the number of set elements (values) given the number of
/// variables and an optional apportionment with
/// elements_per_variable; return the average number per variable if
/// equally distributed
static bool 
check_set_keys(size_t num_v, size_t ds_len, const char *kind,
	       IntArray *input_nds, int& avg_num_ds)
{
  // Process num_set_values key or define default allocation
  bool key = (input_nds);
  if (key) {
    if (input_nds->size() != num_v) {
      wrong_number("num_set_values value(s)", kind, num_v, input_nds->size());
      return key;
    }
    int num_ds_i, total_ds = 0;
    for (size_t i=0; i<num_v; ++i) {
      total_ds += num_ds_i = (*input_nds)[i];
      if (num_ds_i < 1)
	{ too_small(kind); return key; }
    }
    if (ds_len != total_ds)
      { wrong_number("set_values", kind, total_ds, ds_len); return key; }
  }
  else { // num_set_values is optional; use average len if no spec
    if (ds_len % num_v)
      { not_div(kind, ds_len, num_v); return key; }
    else
      avg_num_ds = ds_len / num_v;
  }
  return key;
}


/// check discrete sets of integers (design and state variables);
/// error if a duplicate value is specified
/// error if not ordered to prevent user confusion
static void 
Vchk_DIset(size_t num_v, const char *kind, IntArray *input_ndsi,
	   IntVector *input_dsi, IntSetArray& dsi_all, IntVector& dsi_init_pt)
{
  if (!input_dsi)
    return;

  bool misordered = false;
  int avg_num_dsi, ndup, dupval[2], num_dsi_i, val;
  size_t i, j, cntr, dsi_len = input_dsi->length();

  // Process num_set_values key or define default allocation
  bool key = check_set_keys(num_v, dsi_len, kind, input_ndsi, avg_num_dsi);

  // Insert values into the IntSetArray
  dsi_all.resize(num_v);
  for (i=cntr=ndup=0; i<num_v; ++i) {
    num_dsi_i = (key) ? (*input_ndsi)[i] : avg_num_dsi;
    IntSet& dsi_all_i = dsi_all[i];
    for (j=0; j<num_dsi_i; ++j, ++cntr) {
      val = (*input_dsi)[cntr];
      if (!dsi_all_i.insert(val).second) { // insert returns pair<iterator,bool>
	if (++ndup <= 2) // warnings suppressed beyond two duplicates
	  dupval[ndup-1] = val;
      }
      if (j<num_dsi_i-1 && val >= (*input_dsi)[cntr+1])
	misordered = true;
    }
  }
  if (ndup)
    suppressed(kind, ndup, dupval, 0, 0);
  if (misordered)
    Squawk("Set values for each %s variable must increase", kind);

  // Checks on user-specified initial pt array
  if (!dsi_init_pt.empty()) {
    if (dsi_init_pt.length() != num_v)
      wrong_number("initial_point value(s)", kind, num_v, dsi_init_pt.length());
    else // check within admissible set for specified initial pt
      for (i=0; i<num_v; ++i) {
	IntSet& dsi_all_i = dsi_all[i]; val = dsi_init_pt[i];
	if (dsi_all_i.find(val) == dsi_all_i.end())
	  bad_initial_ivalue(kind, val);
      }
  }
  //else: default initialization performed in Vgen_DIset
}

/// check discrete sets of integers (uncertain variables);
/// error if a duplicate value is specified
/// error if not ordered to prevent user confusion
static void 
Vchk_DIset(size_t num_v, const char *kind, IntArray *input_ndsi,
	   IntVector *input_dsi, RealVector *input_dsip,
	   IntRealMapArray& dsi_vals_probs, IntVector& dsi_init_pt)
{
  if (!input_dsi)
    return;

  bool misordered = false;
  int avg_num_dsi, ndup, dupval[2], num_dsi_i, val;
  size_t i, j, cntr, dsi_len = input_dsi->length(),
    num_p = (input_dsip) ? input_dsip->length() : 0;
  if (num_p && num_p != dsi_len)
    wrong_number("set_probabilities", kind, dsi_len, num_p);
  Real prob, default_p;

  // Process num_set_values key or define default allocation
  bool key = check_set_keys(num_v, dsi_len, kind, input_ndsi, avg_num_dsi);

  // Insert values into the IntRealMapArray
  dsi_vals_probs.resize(num_v);
  for (i=cntr=ndup=0; i<num_v; ++i) {
    IntRealMap& dsi_v_p_i = dsi_vals_probs[i];
    num_dsi_i = (key) ? (*input_ndsi)[i] : avg_num_dsi;
    if (!num_p) default_p = 1./num_dsi_i;
    for (j=0; j<num_dsi_i; ++j, ++cntr) {
      val  = (*input_dsi)[cntr];
      prob = (num_p) ? (*input_dsip)[cntr] : default_p;
      if (dsi_v_p_i.find(val) == dsi_v_p_i.end())
	dsi_v_p_i[val]  = prob; // insert new
      else {
	// don't want to aggregate the probability; just error
	//dsi_v_p_i[val] += prob; // add to existing
	if (++ndup <= 2) // warnings suppressed beyond two duplicates
	  dupval[ndup-1] = val;
      }
      if (j<num_dsi_i-1 && val >= (*input_dsi)[cntr+1])
	misordered = true;
    }
  }
  if (ndup)
    suppressed(kind, ndup, dupval, 0, 0);
  if (misordered)
    Squawk("Set values for each %s variable must increase", kind);

  // Checks on user-specified initial pt array
  if (!dsi_init_pt.empty()) {
    if (dsi_init_pt.length() != num_v)
      wrong_number("initial_point value(s)", kind, num_v, dsi_init_pt.length());
    else // check within admissible set for specified initial pt
      for (i=0; i<num_v; ++i) {
	IntRealMap& dsi_v_p_i = dsi_vals_probs[i]; val = dsi_init_pt[i];
	if (dsi_v_p_i.find(val) == dsi_v_p_i.end())
	  bad_initial_ivalue(kind, val);
      }
  }
  //else: default initialization performed in Vgen_DIset
}

static void 
Vchk_DSset(size_t num_v, const char *kind, IntArray *input_ndss,
	   StringArray *input_dss, StringSetArray& dss_all, StringArray& dss_init_pt)
{
  if (!input_dss)
    return;

  std::vector<bool> misordered(num_v,false);
  int avg_num_dss, ndup, num_dss_i;
  String dupval[2], val;
  size_t i, j, cntr, dss_len = input_dss->size();

  // Process num_set_values key or define default allocation
  bool key = check_set_keys(num_v, dss_len, kind, input_ndss, avg_num_dss);

  // Insert values into the StringSetArray
  dss_all.resize(num_v);
  for (i=cntr=ndup=0; i<num_v; ++i) {
    num_dss_i = (key) ? (*input_ndss)[i] : avg_num_dss;
    StringSet& dss_all_i = dss_all[i];
    for (j=0; j<num_dss_i; ++j, ++cntr) {
      val = (*input_dss)[cntr];
      if (!dss_all_i.insert(val).second) { // insert returns pair<iterator,bool>
	if (++ndup <= 2) // warnings suppressed beyond two duplicates
	  dupval[ndup-1] = val;
      }
      if (j<num_dss_i-1 && val >= (*input_dss)[cntr+1])
	misordered[i] = true;
    }
  }
  if (ndup)
    suppressed(kind, ndup, 0, dupval, 0);
  // Check for misordered elements and print out in the expected order
  for(i=0; i<num_v; ++i) {
    if (misordered[i]) {
	std::stringstream mss;
	std::copy(dss_all[i].begin(), dss_all[i].end(),
	    std::ostream_iterator<std::string>(mss, " "));
        Squawk("Elements of %s variables must be provided in ascending order ( %s)",
	    kind, mss.str().c_str());
     }
  }

  // Checks on user-specified initial pt array
  if (!dss_init_pt.empty()) {
    if (dss_init_pt.size() != num_v)
      wrong_number("initial_point value(s)", kind, num_v, dss_init_pt.size());
    else // check within admissible set for specified initial pt
      for (i=0; i<num_v; ++i) {
	StringSet& dss_all_i = dss_all[i]; val = dss_init_pt[i];
	if (dss_all_i.find(val) == dss_all_i.end())
	  bad_initial_svalue(kind, val);
      }
  }
  //else: default initialization performed in Vgen_DSset
}

static void 
Vchk_DSset(size_t num_v, const char *kind, IntArray *input_ndss,
	   StringArray *input_dss, RealVector *input_dssp,
	   StringRealMapArray& dss_vals_probs, StringArray& dss_init_pt)
{
  if (!input_dss)
    return;

  bool misordered = false;
  int avg_num_dss, ndup, num_dss_i;
  String dupval[2], val;
  size_t i, j, cntr, dss_len = input_dss->size(),
    num_p = (input_dssp) ? input_dssp->length() : 0;
  if (num_p && num_p != dss_len)
    wrong_number("set_probabilities", kind, dss_len, num_p);
  Real prob, default_p;

  // Process num_set_values key or define default allocation
  bool key = check_set_keys(num_v, dss_len, kind, input_ndss, avg_num_dss);

  // Insert values into the StringRealMapArray
  dss_vals_probs.resize(num_v);
  for (i=cntr=ndup=0; i<num_v; ++i) {
    StringRealMap& dss_v_p_i = dss_vals_probs[i];
    num_dss_i = (key) ? (*input_ndss)[i] : avg_num_dss;
    if (!num_p) default_p = 1./num_dss_i;
    for (j=0; j<num_dss_i; ++j, ++cntr) {
      val  = (*input_dss)[cntr];
      prob = (num_p) ? (*input_dssp)[cntr] : default_p;
      if (dss_v_p_i.find(val) == dss_v_p_i.end())
	dss_v_p_i[val]  = prob; // insert new
      else {
	// don't want to aggregate the probability; just error
	//dss_v_p_i[val] += prob; // add to existing
	if (++ndup <= 2) // warnings suppressed beyond two duplicates
	  dupval[ndup-1] = val;
      }
      if (j<num_dss_i-1 && val >= (*input_dss)[cntr+1])
	misordered = true;
    }
  }
  if (ndup)
    suppressed(kind, ndup, 0, dupval, 0);
  if (misordered)
    Squawk("Set values for each %s variable must increase", kind);

  // Checks on user-specified initial pt array
  if (!dss_init_pt.empty()) {
    if (dss_init_pt.size() != num_v)
      wrong_number("initial_point value(s)", kind, num_v, dss_init_pt.size());
    else // check within admissible set for specified initial pt
      for (i=0; i<num_v; ++i) {
	StringRealMap& dss_v_p_i = dss_vals_probs[i]; val = dss_init_pt[i];
	if (dss_v_p_i.find(val) == dss_v_p_i.end())
	  bad_initial_svalue(kind, val);
      }
  }
  //else: default initialization performed in Vgen_DIset
}


static void 
Vchk_DRset(size_t num_v, const char *kind, IntArray  *input_ndsr,
	   RealVector *input_dsr, RealSetArray& dsr_all,
	   RealVector& dsr_init_pt)
{
  if (!input_dsr)
    return;

  bool misordered = false;
  int avg_num_dsr, ndup, num_dsr_i;
  Real dupval[2], val;
  size_t i, j, cntr, dsr_len = input_dsr->length();

  // Process num_set_values key or define default allocation
  bool key = check_set_keys(num_v, dsr_len, kind, input_ndsr, avg_num_dsr);

  // Insert values into the RealSetArray
  dsr_all.resize(num_v);
  for (i=cntr=ndup=0; i<num_v; ++i) {
    num_dsr_i = (key) ? (*input_ndsr)[i] : avg_num_dsr;
    RealSet& dsr_all_i = dsr_all[i];
    for (j=0; j<num_dsr_i; ++j, ++cntr) {
      val = (*input_dsr)[cntr];
      if (!dsr_all_i.insert(val).second) { // insert returns pair<iterator,bool>
	if (++ndup <= 2) // warnings suppressed beyond two duplicates
	  dupval[ndup-1] = val;
      }
      if (j<num_dsr_i-1 && val >= (*input_dsr)[cntr+1])
	misordered = true;
    }
  }
  if (ndup)
    suppressed(kind, ndup, 0, 0, dupval);
  if (misordered)
    Squawk("Set values for each %s variable must increase", kind);

  // Checks on user-specified initial pt array
  if (!dsr_init_pt.empty()) {
    if (dsr_init_pt.length() != num_v)
      wrong_number("initial_point value(s)", kind, num_v, dsr_init_pt.length());
    else
      for (i=0; i<num_v; ++i) {
	RealSet& dsr_all_i = dsr_all[i]; val = dsr_init_pt[i];
	if (dsr_all_i.find(val) == dsr_all_i.end())
	  bad_initial_rvalue(kind, val);
      }
  }
  //else: default initialization performed in Vgen_DRset
}

static void 
Vchk_DRset(size_t num_v, const char *kind, IntArray  *input_ndsr,
	   RealVector *input_dsr, RealVector* input_dsrp,
	   RealRealMapArray& dsr_vals_probs, RealVector& dsr_init_pt)
{
  if (!input_dsr)
    return;

  bool misordered = false;
  size_t i, j, cntr, dsr_len = input_dsr->length();
  int avg_num_dsr, ndup, num_dsr_i,
    num_p = (input_dsrp) ? input_dsrp->length() : 0;
  if (num_p && num_p != dsr_len)
    wrong_number("set_probabilities", kind, dsr_len, num_p);
  Real prob, default_p, dupval[2], val;

  // Process num_set_values key or define default allocation
  bool key = check_set_keys(num_v, dsr_len, kind, input_ndsr, avg_num_dsr);

  // Insert values into the RealRealMapArray
  dsr_vals_probs.resize(num_v);
  for (i=cntr=ndup=0; i<num_v; ++i) {
    RealRealMap& dsr_v_p_i = dsr_vals_probs[i];
    num_dsr_i = (key) ? (*input_ndsr)[i] : avg_num_dsr;
    if (!num_p) default_p = 1./num_dsr_i;
    for (j=0; j<num_dsr_i; ++j, ++cntr) {
      val  = (*input_dsr)[cntr];
      prob = (num_p) ? (*input_dsrp)[cntr] : default_p;
      if (dsr_v_p_i.find(val) == dsr_v_p_i.end())
	dsr_v_p_i[val]  = prob; // insert new
      else {
	// don't want to aggregate the probability; just error
	//dsr_v_p_i[val] += prob; // add to existing
	if (++ndup <= 2) // warnings suppressed beyond two duplicates
	  dupval[ndup-1] = val;
      }
      if (j<num_dsr_i-1 && val >= (*input_dsr)[cntr+1])
	misordered = true;
    }
  }
  if (ndup)
    suppressed(kind, ndup, 0, 0, dupval);
  if (misordered)
    Squawk("Set values for each %s variable must increase", kind);

  // Checks on user-specified initial pt array
  if (!dsr_init_pt.empty()) {
    if (dsr_init_pt.length() != num_v)
      wrong_number("initial_point value(s)", kind, num_v, dsr_init_pt.length());
    else // check within admissible set for specified initial pt
      for (i=0; i<num_v; ++i) {
	RealRealMap& dsr_v_p_i = dsr_vals_probs[i]; val = dsr_init_pt[i];
	if (dsr_v_p_i.find(val) == dsr_v_p_i.end())
	  bad_initial_rvalue(kind, val);
      }
  }
  //else: default initialization performed in Vgen_DIset
}

// Validate adjacency matrices 
static void 
Vchk_Adjacency(size_t num_v, const char *kind, const IntArray &num_e,
		const IntVector &input_ddsa, RealMatrixArray &dda_all)  {
  size_t expected_size = 0;
  for(size_t i = 0; i < num_v; ++i)
    expected_size += num_e[i]*num_e[i];
    if(expected_size != input_ddsa.length())
      Squawk("adjacency list for %s has incorrect length", kind);
    else {
      size_t e_ctr = 0;
      for(size_t i = 0; i < num_v; ++i) {
        RealMatrix a_tmp(num_e[i],num_e[i]);
        for(size_t j = 0; j < num_e[i]; ++j)  
          for(size_t k = 0; k < num_e[i]; ++k) 
            a_tmp[j][k] = input_ddsa[e_ctr++];
        dda_all.push_back(a_tmp);
      }
    }
}

static bool 
check_LUV_size(size_t num_v, IntVector& L, IntVector& U, IntVector& V,
	       bool aggregate_LUV, size_t offset)
{
  bool init_V = true;
  if (aggregate_LUV) {
    int max_index = offset + num_v - 1;
    if (max_index >= L.length() || max_index >= U.length() ||
	max_index >= V.length())
      Squawk("max index %d out of range for aggregate updates in Vgen_DIset",
	     max_index);
  }
  else {
    if (offset)
      Squawk("unexpected offset (%d) for non-aggregate mode in Vgen_DIset",
	     (int)offset);
    L.sizeUninitialized(num_v);
    U.sizeUninitialized(num_v);
    if (V.length() == num_v) // user spec --> already assigned by var_ivec()
      init_V = false;
    else
      V.sizeUninitialized(num_v);
  }
  return init_V;
}

static bool 
check_LUV_size(size_t num_v, StringArray& L, StringArray& U, StringArray& V,
	       bool aggregate_LUV, size_t offset)
{
  bool init_V = true;
  if (aggregate_LUV) {
    int max_index = offset + num_v - 1;
    if (max_index >= L.size() || max_index >= U.size() ||
	max_index >= V.size())
      Squawk("max index %d out of range for aggregate updates in Vgen_DSset",
	     max_index);
  }
  else {
    if (offset)
      Squawk("unexpected offset (%d) for non-aggregate mode in Vgen_DSset",
	     (int)offset);
    L.resize(num_v);
    U.resize(num_v);
    if (V.size() == num_v) // user spec --> already assigned by var_ivec()
      init_V = false;
    else
      V.resize(num_v);
  }
  return init_V;
}



static bool 
check_LUV_size(size_t num_v, RealVector& L, RealVector& U, RealVector& V,
	       bool aggregate_LUV, size_t offset)
{
  bool init_V = true;
  if (aggregate_LUV) {
    int max_index = offset + num_v - 1;
    if (max_index >= L.length() || max_index >= U.length() ||
	max_index >= V.length())
      Squawk("max index %d out of range for aggregate updates in Vgen_DRset",
	     max_index);
  }
  else {
    if (offset)
      Squawk("unexpected offset (%d) for non-aggregate mode in Vgen_DRset",
	     (int)offset);
    L.sizeUninitialized(num_v);
    U.sizeUninitialized(num_v);
    if (V.length() == num_v) // user spec --> already assigned by var_rvec()
      init_V = false;
    else
      V.sizeUninitialized(num_v);
  }
  return init_V;
}

static void 
Vgen_DIset(size_t num_v, IntSetArray& sets, IntVector& L, IntVector& U,
	   IntVector& V, bool aggregate_LUV = false, size_t offset = 0)
{
  ISCIter ie, it;
  Real avg_val, r_val;
  int i, i_val, i_left, i_right;
  size_t num_set_i;

  bool init_V = check_LUV_size(num_v, L, U, V, aggregate_LUV, offset);

  for(i = 0; i < num_v; ++i, ++offset) {
    IntSet& set_i = sets[i];
    it = set_i.begin(); ie = set_i.end(); num_set_i = set_i.size();
    if (num_set_i == 0) // should not occur
      L[offset] = U[offset] = V[offset] = 0;
    else if (num_set_i == 1)
      L[offset] = U[offset] = V[offset] = *it;
    else {
      L[offset] = *it;     // lower bound is first value
      U[offset] = *(--ie); // upper bound is final value
      if (init_V) {
	// select the initial value to be closest set value to avg_val
	for(avg_val = 0., ++ie; it != ie; ++it)
	  avg_val += *it;
	avg_val /= num_set_i;
	// bracket avg_val between [i_left,i_right]
	i_left = L[offset]; i_right = U[offset];
	for(it = set_i.begin(); it != ie; ++it) {
	  r_val = i_val = *it;
	  if (r_val > avg_val) {      // update nearest neighbor to right
	    if (i_val < i_right)
	      i_right = i_val;
	  }
	  else if (r_val < avg_val) { // update nearest neighbor to left
	    if (i_val > i_left)
	      i_left = i_val;
	  }
	  else { // r_val equals avg_val
	    i_left = i_right = i_val;
	    break;
	  }
	}
	V[offset] = (i_right - avg_val < avg_val - i_left) ? i_right : i_left;
      }
    }
  }
}


/// generate lower, upper, and initial point for string-valued sets
static void 
Vgen_DSset(size_t num_v, StringSetArray& sets, StringArray& L, StringArray& U,
	   StringArray& V, bool aggregate_LUV = false, size_t offset = 0)
{
  SSCIter ie, it;
  int i;
  size_t num_set_i;

  bool init_V = check_LUV_size(num_v, L, U, V, aggregate_LUV, offset);

  for(i = 0; i < num_v; ++i, ++offset) {
    StringSet& set_i = sets[i];
    it = set_i.begin(); ie = set_i.end(); num_set_i = set_i.size();
    if (num_set_i == 0) // should not occur
      L[offset] = U[offset] = V[offset] = "";
    else if (num_set_i == 1)
      L[offset] = U[offset] = V[offset] = *it;
    else {
      L[offset] = *it;     // lower bound is first value
      U[offset] = *(--ie); // upper bound is final value
      if (init_V) {
	size_t mid_index = 0;
	// initial value is at middle index or the one directly below
	if ( (num_set_i % 2 == 0) )
	  // initial value is to the left of middle
	  mid_index = num_set_i / 2 - 1;
	else 
	  mid_index = (num_set_i + 1) / 2 - 1;
	std::advance(it, mid_index);
	V[offset] = *it;
      }
    }
  }
}

static void 
Vgen_DIset(size_t num_v, IntRealMapArray& vals_probs, IntVector& IP,
	   IntVector& L, IntVector& U, IntVector& V,
	   bool aggregate_LUV = false, size_t offset = 0)
{
  IRMCIter ite, it;
  Real avg_val, r_val;
  int i_val, i_left, i_right;
  size_t i, j, num_vp_j, num_IP = IP.length();

  bool init_V = check_LUV_size(num_v, L, U, V, aggregate_LUV, offset);

  for(i = offset, j = 0; j < num_v; ++i, ++j) {
    IntRealMap& vp_j = vals_probs[j];
    it = vp_j.begin(); ite = vp_j.end(); num_vp_j = vp_j.size();
    if (num_vp_j == 0) { // should not occur
      L[i] = U[i] = 0;
      V[i] = (num_IP) ? IP[j] : 0;
    }
    else if (num_vp_j == 1) {
      L[i] = U[i] = it->first;
      V[i] = (num_IP) ? IP[j] : it->first;
    }
    else {
      L[i] = it->first;      // lower bound is first value
      U[i] = (--ite)->first; // upper bound is final value
      if (num_IP) V[i] = IP[j]; // presence of value w/i set already checked
      else if (init_V) {
	// select the initial value to be closest set value to avg_val
	for(avg_val = 0., ++ite; it != ite; ++it)
	  avg_val += it->first;
	avg_val /= num_vp_j;
	// bracket avg_val between [i_left,i_right]
	i_left = L[offset]; i_right = U[offset];
	for(it = vp_j.begin(); it != ite; ++it) {
	  r_val = i_val = it->first;
	  if (r_val > avg_val) {      // update nearest neighbor to right
	    if (i_val < i_right)
	      i_right = i_val;
	  }
	  else if (r_val < avg_val) { // update nearest neighbor to left
	    if (i_val > i_left)
	      i_left = i_val;
	  }
	  else { // r_val equals avg_val
	    i_left = i_right = i_val;
	    break;
	  }
	}
	V[i] = (i_right - avg_val < avg_val - i_left) ? i_right : i_left;
      }
    }
  }
}

static void 
Vgen_DRset(size_t num_v, RealSetArray& sets, RealVector& L, RealVector& U,
	   RealVector& V, bool aggregate_LUV = false, size_t offset = 0)
{
  Real avg_val, set_val, s_left, s_right;
  RSCIter ie, it;
  int i;
  size_t num_set_i;

  bool init_V = check_LUV_size(num_v, L, U, V, aggregate_LUV, offset);

  for(i = 0; i < num_v; ++i, ++offset) {
    RealSet& set_i = sets[i];
    it = set_i.begin(); ie = set_i.end(); num_set_i = set_i.size();
    if (num_set_i == 0) // should not occur
      L[offset] = U[offset] = V[offset] = 0.;
    else if (num_set_i == 1)
      L[offset] = U[offset] = V[offset] = *it;
    else {
      L[offset] = *it;     // lower bound is first value
      U[offset] = *(--ie); // upper bound is final value
      if (init_V) {
	// select the initial value to be closest set value to avg_val
	for(avg_val = 0., ++ie; it != ie; ++it)
	  avg_val += *it;
	avg_val /= num_set_i;
	// bracket avg_val between [s_left,s_right]
	s_left = L[offset]; s_right = U[offset];
	for(it = set_i.begin(); it != ie; ++it) {
	  set_val = *it;
	  if (set_val > avg_val) {      // update nearest neighbor to right
	    if (set_val < s_right)
	      s_right = set_val;
	  }
	  else if (set_val < avg_val) { // update nearest neighbor to left
	    if (set_val > s_left)
	      s_left = set_val;
	  }
	  else { // set_val equals avg_val
	    s_left = s_right = set_val;
	    break;
	  }
	}
	V[offset] = (s_right - avg_val < avg_val - s_left) ? s_right : s_left;
      }
    }
  }
}

static void 
Vgen_DRset(size_t num_v, RealRealMapArray& vals_probs, RealVector& IP,
	   RealVector& L, RealVector& U, RealVector& V,
	   bool aggregate_LUV = false, size_t offset = 0)
{
  Real avg_val, set_val, s_left, s_right;
  RRMCIter ite, it;
  size_t i, j, num_vp_j, num_IP = IP.length();

  bool init_V = check_LUV_size(num_v, L, U, V, aggregate_LUV, offset);

  for(i = offset, j = 0; j < num_v; ++i, ++j) {
    RealRealMap& vp_j = vals_probs[j];
    it = vp_j.begin(); ite = vp_j.end(); num_vp_j = vp_j.size();
    if (num_vp_j == 0) { // should not occur
      L[i] = U[i] = V[i] = 0.;
      V[i] = (num_IP) ? IP[j] : 0.;
    }
    else if (num_vp_j == 1) {
      L[i] = U[i] = it->first;
      V[i] = (num_IP) ? IP[j] : it->first;
    }
    else {
      L[i] = it->first;      // lower bound is first value
      U[i] = (--ite)->first; // upper bound is final value
      if (num_IP) V[i] = IP[j];
      else if (init_V) {
	// select the initial value to be closest set value to avg_val
	for(avg_val = 0., ++ite; it != ite; ++it)
	  avg_val += it->first;
	avg_val /= num_vp_j;
	// bracket avg_val between [s_left,s_right]
	s_left = L[i]; s_right = U[i];
	for(it = vp_j.begin(); it != ite; ++it) {
	  set_val = it->first;
	  if (set_val > avg_val) {      // update nearest neighbor to right
	    if (set_val < s_right)
	      s_right = set_val;
	  }
	  else if (set_val < avg_val) { // update nearest neighbor to left
	    if (set_val > s_left)
	      s_left = set_val;
	  }
	  else { // set_val equals avg_val
	    s_left = s_right = set_val;
	    break;
	  }
	}
	V[i] = (s_right - avg_val < avg_val - s_left) ? s_right : s_left;
      }
    }
  }
}

static void 
Vgen_DSset(size_t num_v, StringRealMapArray& vals_probs, StringArray& IP,
	   StringArray& L, StringArray& U, StringArray& V,
	   bool aggregate_LUV = false, size_t offset = 0)
{
  Real avg_val, set_val, s_left, s_right;
  SRMCIter ite, it;
  size_t i, j, num_vp_j, num_IP = IP.size();

  bool init_V = check_LUV_size(num_v, L, U, V, aggregate_LUV, offset);

  for(i = offset, j = 0; j < num_v; ++i, ++j) {
    StringRealMap& vp_j = vals_probs[j];
    it = vp_j.begin(); ite = vp_j.end(); num_vp_j = vp_j.size();
    if (num_vp_j == 0) { // should not occur
      L[i] = U[i] = V[i] = "";
      V[i] = (num_IP) ? IP[j] : "";
    }
    else if (num_vp_j == 1) {
      L[i] = U[i] = it->first;
      V[i] = (num_IP) ? IP[j] : it->first;
    }
    else {
      L[i] = it->first;     // lower bound is first value
      U[i] = (--ite)->first; // upper bound is final value
      if (num_IP) V[i] = IP[j];
      else if (init_V) {
	size_t mid_index = 0;
	// initial value is at middle index or the one directly below
	if ( (num_vp_j % 2 == 0) )
	  // initial value is to the left of middle
	  mid_index = num_vp_j / 2 - 1;
	else 
	  mid_index = (num_vp_j + 1) / 2 - 1;
	std::advance(it, mid_index);
	V[i] = it->first;
      }
    }
  }
}

static void 
Vchk_DiscreteDesSetInt(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  static char kind[] = "discrete_design_set_integer";
  Vchk_DIset(dv->numDiscreteDesSetIntVars, kind, vi->nddsi, vi->ddsi,
	     dv->discreteDesignSetInt, dv->discreteDesignSetIntVars);
  if(vi->ddsia) {
    IntArray num_e;
    for(size_t i = 0; i < dv->numDiscreteDesSetIntVars; i++)
      num_e.push_back(dv->discreteDesignSetInt[i].size());
      Vchk_Adjacency(dv->numDiscreteDesSetIntVars, kind, num_e, *vi->ddsia,
		      dv->discreteDesignSetIntAdj);
  }
}

static void Vgen_DiscreteDesSetInt(DataVariablesRep *dv, size_t offset)
{
  Vgen_DIset(dv->numDiscreteDesSetIntVars, dv->discreteDesignSetInt,
	     dv->discreteDesignSetIntLowerBnds,
	     dv->discreteDesignSetIntUpperBnds,
	     dv->discreteDesignSetIntVars); // no offset, not aggregate L/U/V
}

static void 
Vchk_DiscreteDesSetStr(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  static char kind[] = "discrete_design_set_string";
  Vchk_DSset(dv->numDiscreteDesSetStrVars, kind, vi->nddss, vi->ddss,
	     dv->discreteDesignSetStr, dv->discreteDesignSetStrVars);
  if(vi->ddssa) {
    IntArray num_e;
    for(size_t i = 0; i < dv->numDiscreteDesSetStrVars; i++)
	    num_e.push_back(dv->discreteDesignSetStr[i].size());
    Vchk_Adjacency(dv->numDiscreteDesSetStrVars, kind, num_e, *vi->ddssa,
		    dv->discreteDesignSetStrAdj);
  }
}

static void Vgen_DiscreteDesSetStr(DataVariablesRep *dv, size_t offset)
{
  Vgen_DSset(dv->numDiscreteDesSetStrVars, dv->discreteDesignSetStr,
	     dv->discreteDesignSetStrLowerBnds,
	     dv->discreteDesignSetStrUpperBnds,
	     dv->discreteDesignSetStrVars); // no offset, not aggregate L/U/V
}

static void 
Vchk_DiscreteDesSetReal(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  static char kind[] = "discrete_design_set_real";
  Vchk_DRset(dv->numDiscreteDesSetRealVars, kind, vi->nddsr, vi->ddsr,
	     dv->discreteDesignSetReal, dv->discreteDesignSetRealVars);
  if(vi->ddsra) {
    IntArray num_e;
    for(size_t i = 0; i < dv->numDiscreteDesSetRealVars; i++)
      num_e.push_back(dv->discreteDesignSetReal[i].size());
    Vchk_Adjacency(dv->numDiscreteDesSetRealVars, kind, num_e,
		    *vi->ddsra, dv->discreteDesignSetRealAdj);
  }
}

static void Vgen_DiscreteDesSetReal(DataVariablesRep *dv, size_t offset)
{
  Vgen_DRset(dv->numDiscreteDesSetRealVars, dv->discreteDesignSetReal,
	     dv->discreteDesignSetRealLowerBnds,
	     dv->discreteDesignSetRealUpperBnds,
	     dv->discreteDesignSetRealVars); // no offset, not aggregate L/U/V
}

static void 
Vchk_DiscreteUncSetInt(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  static char kind[] = "discrete_uncertain_set_integer";
  Vchk_DIset(dv->numDiscreteUncSetIntVars, kind, vi->ndusi, vi->dusi, vi->DSIp,
	     dv->discreteUncSetIntValuesProbs, dv->discreteUncSetIntVars);
}

static void Vgen_DiscreteUncSetInt(DataVariablesRep *dv, size_t offset)
{
  Vgen_DIset(dv->numDiscreteUncSetIntVars, dv->discreteUncSetIntValuesProbs,
	     dv->discreteUncSetIntVars, dv->discreteIntEpistemicUncLowerBnds,
	     dv->discreteIntEpistemicUncUpperBnds,
	     dv->discreteIntEpistemicUncVars, true, offset);
  if (dv->discreteUncSetIntVars.length()) dv->uncertainVarsInitPt = true;
}

static void 
Vchk_DiscreteUncSetStr(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  static char kind[] = "discrete_uncertain_set_string";
  Vchk_DSset(dv->numDiscreteUncSetStrVars, kind, vi->nduss, vi->duss, vi->DSSp,
	     dv->discreteUncSetStrValuesProbs, dv->discreteUncSetStrVars);
}

static void Vgen_DiscreteUncSetStr(DataVariablesRep *dv, size_t offset)
{
  Vgen_DSset(dv->numDiscreteUncSetStrVars, dv->discreteUncSetStrValuesProbs,
	     dv->discreteUncSetStrVars, dv->discreteStrEpistemicUncLowerBnds,
	     dv->discreteStrEpistemicUncUpperBnds,
	     dv->discreteStrEpistemicUncVars, true, offset);
  if (dv->discreteUncSetStrVars.size()) dv->uncertainVarsInitPt = true;
}

static void 
Vchk_DiscreteUncSetReal(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  static char kind[] = "discrete_uncertain_set_real";
  Vchk_DRset(dv->numDiscreteUncSetRealVars, kind, vi->ndusr, vi->dusr, vi->DSRp,
	     dv->discreteUncSetRealValuesProbs, dv->discreteUncSetRealVars);
}

static void Vgen_DiscreteUncSetReal(DataVariablesRep *dv, size_t offset)
{
  Vgen_DRset(dv->numDiscreteUncSetRealVars, dv->discreteUncSetRealValuesProbs,
	     dv->discreteUncSetRealVars, dv->discreteRealEpistemicUncLowerBnds,
	     dv->discreteRealEpistemicUncUpperBnds,
	     dv->discreteRealEpistemicUncVars, true, offset);
  if (dv->discreteUncSetRealVars.length()) dv->uncertainVarsInitPt = true;
}

static void 
Vchk_DiscreteStateSetInt(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  static char kind[] = "discrete_state_set_integer";
  Vchk_DIset(dv->numDiscreteStateSetIntVars, kind, vi->ndssi, vi->dssi, 
	     dv->discreteStateSetInt, dv->discreteStateSetIntVars);
}

static void Vgen_DiscreteStateSetInt(DataVariablesRep *dv, size_t offset)
{
  Vgen_DIset(dv->numDiscreteStateSetIntVars, dv->discreteStateSetInt,
	     dv->discreteStateSetIntLowerBnds,
	     dv->discreteStateSetIntUpperBnds,
	     dv->discreteStateSetIntVars); // no offset, not aggregate L/U/V
}

static void 
Vchk_DiscreteStateSetStr(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  static char kind[] = "discrete_state_set_string";
  Vchk_DSset(dv->numDiscreteStateSetStrVars, kind, vi->ndsss, vi->dsss, 
	     dv->discreteStateSetStr, dv->discreteStateSetStrVars);
}

static void Vgen_DiscreteStateSetStr(DataVariablesRep *dv, size_t offset)
{
  Vgen_DSset(dv->numDiscreteStateSetStrVars, dv->discreteStateSetStr,
	     dv->discreteStateSetStrLowerBnds,
	     dv->discreteStateSetStrUpperBnds,
	     dv->discreteStateSetStrVars); // no offset, not aggregate L/U/V
}

static void 
Vchk_DiscreteStateSetReal(DataVariablesRep *dv, size_t offset, Var_Info *vi)
{
  static char kind[] = "discrete_state_set_real";
  Vchk_DRset(dv->numDiscreteStateSetRealVars, kind, vi->ndssr, vi->dssr,
	     dv->discreteStateSetReal, dv->discreteStateSetRealVars);
}

static void Vgen_DiscreteStateSetReal(DataVariablesRep *dv, size_t offset)
{
  Vgen_DRset(dv->numDiscreteStateSetRealVars, dv->discreteStateSetReal,
	     dv->discreteStateSetRealLowerBnds,
	     dv->discreteStateSetRealUpperBnds,
	     dv->discreteStateSetRealVars); // no offset, not aggregate L/U/V
}

static const char *
Var_Name(StringArray *sa, char *buf, size_t i)
{
  if (sa)
    return (*sa)[i].data();
  std::sprintf(buf,"%lu", (UL)(i+1));
  return (const char*)buf;
}

/// For real-valued variables: verify lengths of bounds and initial
/// point, validate bounds and adjust initial point to bounds
static void Var_RealBoundIPCheck(DataVariablesRep *dv, Var_rcheck *b)
{
  RealVector *L, *U, *V;
  StringArray *sa;
  char namebuf[32];
  int i, n, ndflt; // length() values here are int rather than size_t

  if ((n = dv->*b->n) == 0)
    return;
  ndflt = -1;
  if (b->L) {
    ndflt = 0;
    L = &(dv->*b->L);
    if (L->length() == 0)
      ++ndflt;
    else if (L->length() != n) {
      Squawk("%s_lower_bounds needs %lu elements, not %lu",
	     b->name, (UL)n, (UL)L->length());
      return;
    }
    U = &(dv->*b->U);
    if (U->length() == 0)
      ++ndflt;
    else if (U->length() != n) {
      Squawk("%s_upper_bounds needs %lu elements, not %lu",
	     b->name, (UL)n, (UL)U->length());
      return;
    }
  }
  sa = 0;
  if (b->Lbl) {
    sa = &(dv->*b->Lbl);
    if (sa->size() == 0)
      sa = 0;
  }
  if (ndflt == 0)
    for(i = 0; i < n; i++) {
      if ((*L)[i] > (*U)[i])
	Squawk("lower bound for %s variable %s exceeds upper bound",
	       b->name, Var_Name(sa,namebuf,i));
    }
  if (b->V == 0)
    return;
  V = &(dv->*b->V);
  if (V->length() == 0)
    return;
  if (V->length() != n) {
    Squawk("initial point for %s needs %lu elements, not %lu",
	   b->name, (UL)n, (UL)V->length());
    return;
  }
  if (L->length() > 0) {
    for(i = 0; i < n; i++)
      if ((*V)[i] < (*L)[i]) {
	Warn("Setting initial_value for %s variable %s to its lower bound",
	     b->name, Var_Name(sa,namebuf,i));
	(*V)[i] = (*L)[i];
      }
  }
  if (U->length() > 0) {
    for(i = 0; i < n; i++)
      if ((*V)[i] > (*U)[i]) {
	Warn("Setting initial_value for %s variable %s to its upper bound",
	     b->name, Var_Name(sa,namebuf,i));
	(*V)[i] = (*U)[i];
      }
  }
}

/// For integer-valued variables: verify lengths of bounds and initial
/// point, validate bounds and initial point against bounds
static void Var_IntBoundIPCheck(DataVariablesRep *dv, Var_icheck *ib)
{
  IntVector *L, *U, *V;
  StringArray *sa;
  char namebuf[32];
  int i, n, ndflt; // length() values here are int rather than size_t

  if ((n = dv->*ib->n) == 0)
    return;
  L = &(dv->*ib->L);
  ndflt = 0;
  if (L->length() == 0)
    ++ndflt;
  else if (L->length() != n) {
    Squawk("%s_lower_bounds needs %lu elements, not %lu",
	   ib->name, (UL)n, (UL)L->length());
    return;
  }
  U = &(dv->*ib->U);
  if (U->length() == 0)
    ++ndflt;
  else if (U->length() != n) {
    Squawk("%s_upper_bounds needs %lu elements, not %lu",
	   ib->name, (UL)n, (UL)L->length());
    return;
  }
  sa = 0;
  if (ib->Lbl) {
    sa = &(dv->*ib->Lbl);
    if (sa->size() == 0)
      sa = 0;
  }
  if (ndflt == 0)
    for(i = 0; i < n; i++) {
      if ((*L)[i] > (*U)[i])
	Squawk("lower bound for %s variable %s exceeds upper bound",
	       ib->name, Var_Name(sa,namebuf,i));
    }
  if (ib->V == 0)
    return;	// won't happen for discrete variables
  V = &(dv->*ib->V);
  if (V->length() == 0)
    return;
  if (V->length() != n) {
    Squawk("initial point for %s needs %lu elements, not %lu",
	   ib->name, (UL)n, (UL)V->length());
    return;
  }
  if (L->length() > 0) {
    for(i = 0; i < n; i++)
      if ((*V)[i] < (*L)[i]) {
	Warn("Setting initial_value for %s variable %s to its lower bound",
	     ib->name, Var_Name(sa,namebuf,i));
	(*V)[i] = (*L)[i];
      }
  }
  if (U->length() > 0) {
    for(i = 0; i < n; i++)
      if ((*V)[i] > (*U)[i]) {
	Warn("Setting initial_value for %s variable %s to its upper bound",
	     ib->name, Var_Name(sa,namebuf,i));
	(*V)[i] = (*U)[i];
      }
  }
}


// setup the Vchk functions for each of the uncertain variable
// contiguous containers, and for discrete sets
#define VarLabelInfo(a,b)     { #a, #b, &DataVariablesRep::num##b##Vars, Vchk_##b }
static Var_uinfo CAUVLbl[CAUVar_Nkinds] = {
  VarLabelInfo(nuv_, NormalUnc),
  VarLabelInfo(lnuv_, LognormalUnc),
  VarLabelInfo(uuv_, UniformUnc),
  VarLabelInfo(luuv_, LoguniformUnc),
  VarLabelInfo(tuv_, TriangularUnc),
  VarLabelInfo(euv_, ExponentialUnc),
  VarLabelInfo(beuv_, BetaUnc),
  VarLabelInfo(gauv_, GammaUnc),
  VarLabelInfo(guuv_, GumbelUnc),
  VarLabelInfo(fuv_, FrechetUnc),
  VarLabelInfo(wuv_, WeibullUnc),
  VarLabelInfo(hbuv_, HistogramBinUnc)
};
static Var_uinfo DAUIVLbl[DAUIVar_Nkinds] = {
  VarLabelInfo(puv_, PoissonUnc),
  VarLabelInfo(biuv_, BinomialUnc),
  VarLabelInfo(nbuv_, NegBinomialUnc),
  VarLabelInfo(geuv_, GeometricUnc),
  VarLabelInfo(hguv_, HyperGeomUnc),
  VarLabelInfo(hpiuv_, HistogramPtIntUnc)
};
static Var_uinfo DAUSVLbl[DAUSVar_Nkinds] = {
  VarLabelInfo(hpsuv_, HistogramPtStrUnc)
};
static Var_uinfo DAURVLbl[DAURVar_Nkinds] = {
  VarLabelInfo(hpruv_, HistogramPtRealUnc)
};
static Var_uinfo CEUVLbl[CEUVar_Nkinds] = {
  VarLabelInfo(ciuv_, ContinuousIntervalUnc)
};
static Var_uinfo DEUIVLbl[DEUIVar_Nkinds] = {
  VarLabelInfo(diuv_, DiscreteIntervalUnc),
  VarLabelInfo(dusiv_, DiscreteUncSetInt)
};
static Var_uinfo DEUSVLbl[DEUSVar_Nkinds] = {
  VarLabelInfo(dussv_, DiscreteUncSetStr)
};
static Var_uinfo DEURVLbl[DEURVar_Nkinds] = {
  VarLabelInfo(dusrv_, DiscreteUncSetReal)
};
static Var_uinfo DiscSetLbl[DiscSetVar_Nkinds] = {
  VarLabelInfo(ddsiv_, DiscreteDesSetInt),
  VarLabelInfo(ddssv_, DiscreteDesSetStr),
  VarLabelInfo(ddsrv_, DiscreteDesSetReal),
  VarLabelInfo(dssiv_, DiscreteStateSetInt),
  VarLabelInfo(dsssv_, DiscreteStateSetStr),
  VarLabelInfo(dssrv_, DiscreteStateSetReal)
};
#undef VarLabelInfo

void NIDRProblemDescDB::
var_stop(const char *keyname, Values *val, void **g, void *v)
{
  static const char *mr_scaletypes[] = { "auto", "none", 0 };

  Var_Info *vi = *(Var_Info**)g;
  DataVariablesRep *dv = vi->dv;

  scale_chk(dv->continuousDesignScaleTypes, dv->continuousDesignScales,
	    "cdv", aln_scaletypes);
  scale_chk(dv->linearIneqScaleTypes, dv->linearIneqScales,
	    "linear_inequality", mr_scaletypes);
  scale_chk(dv->linearEqScaleTypes, dv->linearEqScales,
	    "linear_equality", mr_scaletypes);

  pDDBInstance->VIL.push_back(vi);
  pDDBInstance->dataVariablesList.push_back(*vi->dv_handle);
  delete vi->dv_handle;
}

struct VarLabelChk {
  size_t DataVariablesRep::* n;
  StringArray DataVariablesRep::* sa;
  const char *stub;
  const char *name;
};


// For validation to be performed in check_variables_node(), each
// variable type must be included in at least one of
// DesignAndStateLabelsCheck, LUncertainInt, VLUncertainStr,
// VLUncertainReal, or DiscSetLbl.  Design and state discrete sets are
// currently in both DesignAndStateLabelsCheck (check lengths) and
// DiscSetLbl (duplicates and STL population)

#define AVI &DataVariablesRep::
/// Variables label array designations for design and state.  All
/// non-uncertain variables need to be in this array.  Used in
/// check_variables_node to check lengths and make_variable_defaults
/// to build labels.
static VarLabelChk DesignAndStateLabelsCheck[] = {
  { AVI numContinuousDesVars, AVI continuousDesignLabels, "cdv_", "cdv_descriptors" },
  { AVI numDiscreteDesRangeVars, AVI discreteDesignRangeLabels, "ddriv_", "ddriv_descriptors" },
  { AVI numDiscreteDesSetIntVars, AVI discreteDesignSetIntLabels, "ddsiv_", "ddsiv_descriptors" },
  { AVI numDiscreteDesSetStrVars, AVI discreteDesignSetStrLabels, "ddssv_", "ddssv_descriptors" },
  { AVI numDiscreteDesSetRealVars, AVI discreteDesignSetRealLabels, "ddsrv_", "ddsrv_descriptors" },
  { AVI numContinuousStateVars, AVI continuousStateLabels, "csv_", "csv_descriptors" },
  { AVI numDiscreteStateRangeVars, AVI discreteStateRangeLabels, "dsriv_", "dsriv_descriptors" },
  { AVI numDiscreteStateSetIntVars, AVI discreteStateSetIntLabels, "dssiv_", "dssiv_descriptors" },
  { AVI numDiscreteStateSetStrVars, AVI discreteStateSetStrLabels, "dsssv_", "dsssv_descriptors" },
  { AVI numDiscreteStateSetRealVars, AVI discreteStateSetRealLabels, "dssrv_", "dssrv_descriptors" },
  { AVI numContinuousDesVars, AVI continuousDesignScaleTypes, 0, "cdv_scale_types" }
};
#undef  AVI

/// structure for validating real uncertain variable labels, bounds, values
struct VLreal {
  int n;
  VarLabel Var_Info::* VL; // should be "VarLabel *Var_Info::* VL"
  // but g++ is buggy (versions 4.3.1, 4.4.2 anyway)
  Var_uinfo *vui;
  StringArray DataVariablesRep::* Labels;
  RealVector DataVariablesRep::* LowerBnds;
  RealVector DataVariablesRep::* UpperBnds;
  RealVector DataVariablesRep::* UncVars;
};

/// structure for validating integer uncertain variable labels, bounds, values
struct VLint {
  int n;
  VarLabel Var_Info::* VL; // should be "VarLabel *Var_Info::* VL"
  // but g++ is buggy (versions 4.3.1, 4.4.2 anyway)
  Var_uinfo *vui;
  StringArray DataVariablesRep::* Labels;
  IntVector DataVariablesRep::* LowerBnds;
  IntVector DataVariablesRep::* UpperBnds;
  IntVector DataVariablesRep::* UncVars;
};

/// structure for validating string uncertain variable labels, bounds, values
struct VLstr {
  int n;
  VarLabel Var_Info::* VL; // should be "VarLabel *Var_Info::* VL"
  // but g++ is buggy (versions 4.3.1, 4.4.2 anyway)
  Var_uinfo *vui;
  StringArray DataVariablesRep::* Labels;
  StringArray DataVariablesRep::* LowerBnds;
  StringArray DataVariablesRep::* UpperBnds;
  StringArray DataVariablesRep::* UncVars;
};

/// number of real-valued   uncertain contiguous containers
enum { NUM_UNC_REAL_CONT = 4 };
/// number of int-valued    uncertain contiguous containers
enum { NUM_UNC_INT_CONT = 2 }; 
/// number of string-valued uncertain contiguous containers
enum { NUM_UNC_STR_CONT = 2 }; 

#define AVI (VarLabel Var_Info::*) &Var_Info::	// cast to bypass g++ bug
#define DVR &DataVariablesRep::

/// Variables labels/bounds/values check array for real-valued
/// uncertain variables; one array entry per contiguous container.
/// These associate the individual variables given by, e.g., CAUVLbl,
/// with the contiguous container in which they are stored.
static VLreal VLUncertainReal[NUM_UNC_REAL_CONT] = {
 {CAUVar_Nkinds,  AVI CAUv,  CAUVLbl,
	DVR continuousAleatoryUncLabels,
	DVR continuousAleatoryUncLowerBnds,
	DVR continuousAleatoryUncUpperBnds,
	DVR continuousAleatoryUncVars},
 {CEUVar_Nkinds,  AVI CEUv,  CEUVLbl,
	DVR continuousEpistemicUncLabels,
	DVR continuousEpistemicUncLowerBnds,
	DVR continuousEpistemicUncUpperBnds,
	DVR continuousEpistemicUncVars},
 {DAURVar_Nkinds, AVI DAURv, DAURVLbl,
	DVR discreteRealAleatoryUncLabels,
	DVR discreteRealAleatoryUncLowerBnds,
	DVR discreteRealAleatoryUncUpperBnds,
        DVR discreteRealAleatoryUncVars},
 {DEURVar_Nkinds, AVI DEURv, DEURVLbl,
	DVR discreteRealEpistemicUncLabels,
	DVR discreteRealEpistemicUncLowerBnds,
	DVR discreteRealEpistemicUncUpperBnds,
	DVR discreteRealEpistemicUncVars}};

/// Variables labels/bounds/values check array for integer-valued
/// uncertain variables; one array entry per contiguous container.
/// These associate the individual variables given by, e.g., DAUIVLbl,
/// with the contiguous container in which they are stored.
static VLint VLUncertainInt[NUM_UNC_INT_CONT] = {
 {DAUIVar_Nkinds, AVI DAUIv, DAUIVLbl,
	DVR discreteIntAleatoryUncLabels,
        DVR discreteIntAleatoryUncLowerBnds,
        DVR discreteIntAleatoryUncUpperBnds,
        DVR discreteIntAleatoryUncVars},
 {DEUIVar_Nkinds, AVI DEUIv, DEUIVLbl,
	DVR discreteIntEpistemicUncLabels,
	DVR discreteIntEpistemicUncLowerBnds,
	DVR discreteIntEpistemicUncUpperBnds,
	DVR discreteIntEpistemicUncVars}};

/// Variables labels/bounds/values check array for string-valued
/// uncertain variables; one array entry per contiguous container.
/// These associate the individual variables given by, e.g., DAUSVLbl,
/// with the contiguous container in which they are stored.
static VLstr VLUncertainStr[NUM_UNC_STR_CONT] = {
 {DAUSVar_Nkinds, AVI DAUSv, DAUSVLbl,
	DVR discreteStrAleatoryUncLabels,
	DVR discreteStrAleatoryUncLowerBnds,
	DVR discreteStrAleatoryUncUpperBnds,
        DVR discreteStrAleatoryUncVars},
 {DEUSVar_Nkinds, AVI DEUSv, DEUSVLbl,
	DVR discreteStrEpistemicUncLabels,
	DVR discreteStrEpistemicUncLowerBnds,
	DVR discreteStrEpistemicUncUpperBnds,
	DVR discreteStrEpistemicUncVars}};

//#undef RDVR
#undef	DVR
#undef  AVI

/// which uncertain real check array containers are aleatory (true = 1)
static int VLR_aleatory[NUM_UNC_REAL_CONT] = { 1, 0, 1, 0 };
/// which uncertain integer check array containers are aleatory (true = 1)
static int VLI_aleatory[NUM_UNC_INT_CONT] = { 1, 0 };
/// which uncertain string check array containers are aleatory (true = 1)
static int VLS_aleatory[NUM_UNC_STR_CONT] = { 1, 0 };


/// Generate check data for variables with just name, size, and a
/// generator function
#define Vchk_3(x,y) {#x,&DataVariablesRep::num##y##Vars,Vgen_##y}

/// Generate check data for variables additionally with lower and
/// upper bounds.  Some compilers in debug mode (MSVC) don't
/// initialize the trailing two entries to NULL, so be explicit:
#define Vchk_5(x,y,z) {#x,&DataVariablesRep::num##y##Vars,Vgen_##y,&DataVariablesRep::z##LowerBnds,&DataVariablesRep::z##UpperBnds,NULL,NULL}

/// Generate check data for variables additionally with initial point
/// and labels
#define Vchk_7(x,y,z) {#x,&DataVariablesRep::num##y##Vars,Vgen_##y,&DataVariablesRep::z##LowerBnds,&DataVariablesRep::z##UpperBnds,&DataVariablesRep::z##Vars,&DataVariablesRep::z##Labels}

// Trailing pointers in these initialization lists will be NULL. From C++ 2003:
//   "Objects with static storage shall be zero-initialized before any other
//    initialization takes place."

// These are used within make_variable_defaults(): Vgen_##y is applied
// to generate bounds, adjust initial values
static Var_check
  var_mp_check_cv[] = {
	Vchk_3(continuous_design,ContinuousDes),
	Vchk_3(continuous_state,ContinuousState) },
  var_mp_check_dset[] = {
	Vchk_3(discrete_design_set_integer,DiscreteDesSetInt),
	Vchk_3(discrete_design_set_string,DiscreteDesSetStr),
	Vchk_3(discrete_design_set_real,DiscreteDesSetReal),
	Vchk_3(discrete_state_set_integer,DiscreteStateSetInt),
	Vchk_3(discrete_state_set_string,DiscreteStateSetStr),
	Vchk_3(discrete_state_set_real,DiscreteStateSetReal) },
  var_mp_check_cau[] = {
	Vchk_3(normal_uncertain,NormalUnc),
	Vchk_3(lognormal_uncertain,LognormalUnc),
	Vchk_3(uniform_uncertain,UniformUnc),
	Vchk_3(loguniform_uncertain,LoguniformUnc),
	Vchk_3(triangular_uncertain,TriangularUnc),
	Vchk_3(exponential_uncertain,ExponentialUnc),
	Vchk_3(beta_uncertain,BetaUnc),
	Vchk_3(gamma_uncertain,GammaUnc),
	Vchk_3(gumbel_uncertain,GumbelUnc),
	Vchk_3(frechet_uncertain,FrechetUnc),
	Vchk_3(weibull_uncertain,WeibullUnc),
	Vchk_3(histogram_bin_uncertain,HistogramBinUnc) },
  var_mp_check_daui[] = {
	Vchk_3(poisson_uncertain,PoissonUnc),
	Vchk_3(binomial_uncertain,BinomialUnc),
	Vchk_3(negative_binomial_uncertain,NegBinomialUnc),
	Vchk_3(geometric_uncertain,GeometricUnc),
	Vchk_3(hypergeometric_uncertain,HyperGeomUnc),
	Vchk_3(histogram_point_int_uncertain,HistogramPtIntUnc) },
  var_mp_check_daus[] = {
	Vchk_3(histogram_point_str_uncertain,HistogramPtStrUnc) },
  var_mp_check_daur[] = {
	Vchk_3(histogram_point_real_uncertain,HistogramPtRealUnc) },
  var_mp_check_ceu[] = {
	Vchk_3(continuous_interval_uncertain,ContinuousIntervalUnc) },
  var_mp_check_deui[] = {
	Vchk_3(discrete_interval_uncertain,DiscreteIntervalUnc),
	Vchk_3(discrete_uncertain_set_integer,DiscreteUncSetInt) },
  var_mp_check_deus[] = {
        Vchk_3(discrete_uncertain_set_string,DiscreteUncSetStr) },
  var_mp_check_deur[] = {
	Vchk_3(discrete_uncertain_set_real,DiscreteUncSetReal) };

/// This is used within check_variables_node(): Var_RealBoundIPCheck()
/// is applied to validate bounds and initial points
static Var_rcheck
  var_mp_cbound[] = {
	Vchk_7(continuous_design,ContinuousDes,continuousDesign),
	Vchk_7(continuous_state,ContinuousState,continuousState),
	// BMA: I believe these should these be promoted to Vchk_7,
	// but not easy to do since their labels are stored in an
	// aggregated array instead of individually
	Vchk_5(normal_uncertain,NormalUnc,normalUnc),
	Vchk_5(lognormal_uncertain,LognormalUnc,lognormalUnc),
	Vchk_5(uniform_uncertain,UniformUnc,uniformUnc),
	Vchk_5(loguniform_uncertain,LoguniformUnc,loguniformUnc),
	Vchk_5(triangular_uncertain,TriangularUnc,triangularUnc),
	Vchk_5(beta_uncertain,BetaUnc,betaUnc) };
// gamma, gumbel, frechet, weibull, histogram_bin don't support bounds
// from user input, so are omitted here?

/// This is used in check_variables_node(): Var_IntBoundIPCheck() is
/// applied to validate bounds and initial points, and in
/// make_variable_defaults(): Vgen_* is called to infer bounds.
static Var_icheck
  var_mp_drange[] = {
	Vchk_7(discrete_design_range,DiscreteDesRange,discreteDesignRange),
	Vchk_7(discrete_state_range,DiscreteStateRange,discreteStateRange) };

// would be used to check initial point, but those are covered in DiscSetLbl
// static Var_scheck
// var_mp_sbound[] = {
//   Vchk_7(discrete_design_set_string,DiscreteDesSetStr,discreteDesignSetStr) };

#undef Vchk_7
#undef Vchk_5
#undef Vchk_3


/** Size arrays for contiguous storage of aggregated uncertain types.
    For each variable type, call Vgen_* to generate inferred bounds
    and initial point, repairing initial if needed. */
void NIDRProblemDescDB::
make_variable_defaults(std::list<DataVariables>* dvl)
{
  DataVariablesRep *dv;
  IntVector *IL, *IU, *IV;
  StringArray *SL, *SU, *SV;
  RealVector *L, *U, *V;
  StringArray *sa;
  VLreal *vlr;
  VLint  *vli;
  VLstr  *vls;
  VarLabel *vl;
  VarLabelChk *vlc, *vlce;
  Var_uinfo *vui, *vuie;
  char buf[32];
  size_t i, j, k, n, nu, nursave[NUM_UNC_REAL_CONT], nuisave[NUM_UNC_INT_CONT], 
    nussave[NUM_UNC_STR_CONT];
  static const char Inconsistent_bounds[] =
    "Inconsistent bounds on %s uncertain variables";

  /// size the aggregate arrays for uncertain (design and state are
  /// stored separately
  std::list<DataVariables>::iterator It = dvl->begin(), Ite = dvl->end();
  for(; It != Ite; ++It) {
    dv = It->dataVarsRep;
    // size the aggregate labels, bounds, values arrays for
    // real-valued uncertain
    for(k = 0; k < NUM_UNC_REAL_CONT; ++k) {
      vlr = &VLUncertainReal[k];
      vui = vlr->vui;
      vuie = vui + vlr->n;
      for(nu = 0; vui < vuie; ++vui)
	nu += dv->*vui->n;
      nursave[k] = nu;
      if (!nu)
	continue;
      L = &(dv->*vlr->LowerBnds);
      U = &(dv->*vlr->UpperBnds);
      V = &(dv->*vlr->UncVars);
      L->sizeUninitialized(nu);
      U->sizeUninitialized(nu);
      V->sizeUninitialized(nu);
    }
    // size the aggregate labels, bounds, values arrays for
    // integer-valued uncertain
    for(k = 0; k < NUM_UNC_INT_CONT; ++k) {
      vli = &VLUncertainInt[k];
      vui = vli->vui;
      vuie = vui + vli->n;
      for(nu = 0; vui < vuie; ++vui)
	nu += dv->*vui->n;
      nuisave[k] = nu;
      if (!nu)
	continue;
      IL = &(dv->*vli->LowerBnds);
      IU = &(dv->*vli->UpperBnds);
      IV = &(dv->*vli->UncVars);
      IL->sizeUninitialized(nu);
      IU->sizeUninitialized(nu);
      IV->sizeUninitialized(nu);
    }
    // size the aggregate labels, bounds, values arrays for
    // string-valued uncertain
    for(k = 0; k < NUM_UNC_STR_CONT; ++k) {
      vls = &VLUncertainStr[k];
      vui = vls->vui;
      vuie = vui + vls->n;
      for(nu = 0; vui < vuie; ++vui)
	nu += dv->*vui->n;
      nussave[k] = nu;
      if (!nu)
	continue;
      SL = &(dv->*vls->LowerBnds);
      SU = &(dv->*vls->UpperBnds);
      SV = &(dv->*vls->UncVars);
      SL->resize(nu);
      SU->resize(nu);
      SV->resize(nu);
    }

    // inferred bound generation for continuous variable types;
    // populates the individual (design/state) or aggregate
    // (uncertain) arrays of bounds and values by applying Vgen_##y

    // loop over cdv/csv
    Var_check *c, *ce;
    // the offset into the aggregated aleatory or epistemic arrays
    size_t offset = 0; 
    for(c=var_mp_check_cv, ce = c + Numberof(var_mp_check_cv); c < ce; ++c)
      if ((n = dv->*c->n) > 0)
	(*c->vgen)(dv, offset); // offset not used
    // loop over nuv/lnuv/uuv/luuv/truv/euv/buv/gauv/guuv/fuv/wuv/hbuv
    // working with continuousAleatoryUnc*
    for(c=var_mp_check_cau, ce=c + Numberof(var_mp_check_cau); c < ce; ++c)
      if ((n = dv->*c->n) > 0)
	{ (*c->vgen)(dv, offset); offset += n; }
    // continuous epistemic uncertain
    offset = 0; // reset for combined epistemic arrays
    for(c=var_mp_check_ceu, ce=c + Numberof(var_mp_check_ceu); c < ce; ++c)
      if ((n = dv->*c->n) > 0)
	{ (*c->vgen)(dv, offset); offset += n; }

    // inferred bound generation for discrete variable types;
    // populates the individual (design/state) or aggregate
    // (uncertain) arrays of bounds and values by applying Vgen_##y

    // discrete design,state ranges
    Var_icheck *ic, *ice;
    offset = 0;
    for(ic=var_mp_drange, ice=ic + Numberof(var_mp_drange); ic<ice; ++ic)
      if ((n = dv->*ic->n) > 0)
	{ (*ic->vgen)(dv, offset); } // offset not used
    // discrete int aleatory uncertain use offset passed into Vgen_*Unc
    // working with discreteIntAleatoryUnc*
    for(c=var_mp_check_daui, ce=c + Numberof(var_mp_check_daui); c<ce; ++c)
      if ((n = dv->*c->n) > 0)
	{ (*c->vgen)(dv, offset); offset += n; }
    // discrete string aleatory uncertain use offset passed into Vgen_*Unc
    // working with discreteStrAleatoryUnc*
    offset = 0.;
    for(c=var_mp_check_daus, ce=c + Numberof(var_mp_check_daus); c<ce; ++c)
      if ((n = dv->*c->n) > 0)
	{ (*c->vgen)(dv, offset); offset += n; }
    // discrete real aleatory uncertain use offset passed into Vgen_*Unc
    // working with discreteRealAleatoryUnc*
    offset = 0;
    for(c=var_mp_check_daur, ce=c + Numberof(var_mp_check_daur); c<ce; ++c)
      if ((n = dv->*c->n) > 0)
	{ (*c->vgen)(dv, offset); offset += n; }
    // discrete int epistemic uncertain use offset passed into Vgen_*Unc
    // working with discreteIntEpistemicUnc*
    offset = 0;
    for(c=var_mp_check_deui, ce=c + Numberof(var_mp_check_deui); c<ce; ++c)
      if ((n = dv->*c->n) > 0)
	{ (*c->vgen)(dv, offset); offset += n; }
    // discrete string epistemic uncertain use offset passed into Vgen_*Unc
    // working with discreteStrEpistemicUnc*
    offset = 0;
    for(c=var_mp_check_deus, ce=c + Numberof(var_mp_check_deus); c<ce; ++c)
      if ((n = dv->*c->n) > 0)
	{ (*c->vgen)(dv, offset); offset += n; }
    // discrete real epistemic uncertain use offset passed into Vgen_*Unc
    // working with discreteRealEpistemicUnc*
    offset = 0;
    for(c=var_mp_check_deur, ce=c + Numberof(var_mp_check_deur); c<ce; ++c)
      if ((n = dv->*c->n) > 0)
	{ (*c->vgen)(dv, offset); offset += n; }
    
    // check discrete design and state set types
    // these don't use an offset passed into Vgen_Discrete*Set*
    offset = 0;
    for(c=var_mp_check_dset, ce=c + Numberof(var_mp_check_dset); c<ce; ++c)
      if ((n = dv->*c->n) > 0)
	(*c->vgen)(dv, offset); // offset not used


    // Validate bounds (again?) for uncertain variables and set
    // default labels for all types

    // uncertain real: bounds check and label generation
    for(k = 0; k < NUM_UNC_REAL_CONT; ++k) {
      nu = nursave[k];
      if (!nu)
	continue;
      vlr = &VLUncertainReal[k];
      vui = vlr->vui;
      vuie = vui + vlr->n;
      L = &(dv->*vlr->LowerBnds);
      U = &(dv->*vlr->UpperBnds);
      V = &(dv->*vlr->UncVars);
      sa = &(dv->*vlr->Labels);
      if (!sa->size())
	sa->resize(nu);
      i = 0;
      for(vui = vlr->vui; vui < vuie; ++vui) {
	if ((n = dv->*vui->n) == 0)
	  continue;
	for(j = 0; j < n; ++j)
	  if ((*L)[i+j] > (*U)[i+j]) {
	    squawk(Inconsistent_bounds, vui->vkind);
	    break;
	  }
	if ((*sa)[i] == "")
	  for(j = 1; j <= n; ++j) {
	    std::sprintf(buf, "%s%d", vui->lbl, (int)j);
	    (*sa)[i++] = buf;
	  }
	else
	  i += n;
      }
    }

    // uncertain integer: bounds check and label generation
    for(k = 0; k < NUM_UNC_INT_CONT; ++k) {
      nu = nuisave[k];
      if (!nu)
	continue;
      vli = &VLUncertainInt[k];
      vui = vli->vui;
      vuie = vui + vli->n;
      IL = &(dv->*vli->LowerBnds);
      IU = &(dv->*vli->UpperBnds);
      IV = &(dv->*vli->UncVars);
      sa = &(dv->*vli->Labels);
      if (!sa->size())
	sa->resize(nu);
      i = 0;
      for(vui = vli->vui; vui < vuie; ++vui) {
	if ((n = dv->*vui->n) == 0)
	  continue;
	for(j = 0; j < n; ++j)
	  if ((*IL)[i+j] > (*IU)[i+j]) {
	    squawk(Inconsistent_bounds, vui->vkind);
	    break;
	  }
	if ((*sa)[i] == "")
	  for(j = 1; j <= n; ++j) {
	    std::sprintf(buf, "%s%d", vui->lbl, (int)j);
	    (*sa)[i++] = buf;
	  }
	else
	  i += n;
      }
    }

    // uncertain string: bounds check and label generation
    for(k = 0; k < NUM_UNC_STR_CONT; ++k) {
      nu = nussave[k];
      if (!nu)
	continue;
      vls = &VLUncertainStr[k];
      vui = vls->vui;
      vuie = vui + vls->n;
      SL = &(dv->*vls->LowerBnds);
      SU = &(dv->*vls->UpperBnds);
      SV = &(dv->*vls->UncVars);
      sa = &(dv->*vls->Labels);
      if (!sa->size())
	sa->resize(nu);
      i = 0;
      for(vui = vls->vui; vui < vuie; ++vui) {
	if ((n = dv->*vui->n) == 0)
	  continue;
	for(j = 0; j < n; ++j)
	  if ((*SL)[i+j] > (*SU)[i+j]) {
	    squawk(Inconsistent_bounds, vui->vkind);
	    break;
	  }
	if ((*sa)[i] == "")
	  for(j = 1; j <= n; ++j) {
	    std::sprintf(buf, "%s%d", vui->lbl, (int)j);
	    (*sa)[i++] = buf;
	  }
	else
	  i += n;
      }
    }

    // build labels for all design and state variables if needed
    for(vlc = DesignAndStateLabelsCheck, vlce = vlc + Numberof(DesignAndStateLabelsCheck); vlc < vlce; ++vlc)
      if (vlc->stub && (n = dv->*vlc->n)) {
	sa = &(dv->*vlc->sa);
	if (sa->size() == 0)
	  BuildLabels(sa, n, 0, n, vlc->stub);
      }
  }
}

void NIDRProblemDescDB::check_variables_node(void *v)
{
  IntArray *Ia; IntVector *Iv; RealVector *Rv; RealSymMatrix *Rm;
  StringArray *sa, *Sa;
  VLreal *vlr; VLint *vli; VLstr *vls;
  VarLabel *vl;
  VarLabelChk *vlc, *vlce;
  Var_uinfo *vui, *vuie;
  const char **sp;
  int havelabels;
  size_t i, j, k, n, nd, nu, nuk, nutot, nv;

#define AVI &Var_Info::
  // Used for deallocation of Var_Info temporary data
  static IntArray   *Var_Info::* Ia_delete[]
    = { AVI nddsi, AVI nddss, AVI nddsr, AVI nCI, AVI nDI, AVI nhbp, 
	AVI nhpip, AVI nhpsp, AVI nhprp, AVI ndusi, AVI nduss, AVI ndusr, 
	AVI ndssi, AVI ndssr };
  static RealVector *Var_Info::* Rv_delete[]
    = { AVI ddsr, AVI CIlb, AVI CIub, AVI CIp, AVI DIp,
	AVI DSIp, AVI DSSp, AVI DSRp,
	AVI dusr, AVI hba, AVI hbo, AVI hbc, 
	AVI hpic, AVI hpsc, AVI hpra, AVI hprc, 
	AVI ucm,
	AVI dssr };
  static IntVector *Var_Info::* Iv_delete[]
    = { AVI ddsi, AVI DIlb, AVI DIub, AVI dusi, AVI dssi };
  static StringArray *Var_Info::* Sa_delete[]
    = { AVI ddss, AVI duss, AVI dsss };
#undef AVI

  Var_Info *vi = (Var_Info*)v;
  DataVariablesRep *dv = vi->dv;

  // check label lengths for design and state variables, if present
  nv = 0;
  for(vlc = DesignAndStateLabelsCheck, vlce = vlc + Numberof(DesignAndStateLabelsCheck); 
      vlc < vlce; ++vlc)
    if ((n = dv->*vlc->n)) {
      if (vlc->stub)
	nv += n;
      sa = &(dv->*vlc->sa);
      if ((nu = sa->size())) {
	if (nu != n) {
	  if (nu == 1 && !vlc->stub)
	    continue;
	  squawk("Found %lu rather than %lu %s values",
		 (UL)nu, (UL)n, vlc->name);
	}
      }
    }

  // Now check uncertain variables.  Calls the Vchk_* functions (e.g.,
  // Vchk_NormalUnc) for each variable type, which for simple
  // uncertain variables check lengths, but for histograms and
  // intervals, translate the data into arrays of maps.

  // uncertain real
  for(k = nu = nutot = 0; k < NUM_UNC_REAL_CONT; ++k) {
    vlr = &VLUncertainReal[k];
    havelabels = 0;
    nuk = 0;
    vl = &(vi->*vlr->VL);	// "&(...)" to bypass a g++ bug
    vui = vlr->vui;
    for(vuie = vui + vlr->n; vui < vuie; ++vl, ++vui) {
      nuk += dv->*vui->n;
      if (vl->s)
	++havelabels;
    }
    if (nuk > 0) {
      nutot += nuk;
      if (VLR_aleatory[k])
	nu += nuk;
      if (havelabels)
	(sa = &(dv->*vlr->Labels))->resize(nuk);
      i = 0;
      vl = &(vi->*vlr->VL);
      for(vui = vlr->vui; vui < vuie; ++vl, ++vui) {
	if ((n = dv->*vui->n) == 0)
	  continue;
	vui->vchk(dv,i,vi);
	if ((sp = vl->s)) {
	  if (vl->n != n)
	    squawk("Expected %d %s_descriptors, but got %d",
		   n, vui->lbl, vl->n);
	  else
	    for(j = 0; j < n; ++i, ++j)
	      (*sa)[i] = sp[j];
	  free(sp);
	}
	else
	  i += n;
      }
    }
  }

  // uncertain integer
  for(k = 0; k < NUM_UNC_INT_CONT; ++k) {
    vli = &VLUncertainInt[k];
    havelabels = 0;
    nuk = 0;
    vl = &(vi->*vli->VL);	// "&(...)" to bypass a g++ bug
    vui = vli->vui;
    for(vuie = vui + vli->n; vui < vuie; ++vl, ++vui) {
      nuk += dv->*vui->n;
      if (vl->s)
	++havelabels;
    }
    if (nuk > 0) {
      nutot += nuk;
      if (VLI_aleatory[k])
	nu += nuk;
      if (havelabels)
	(sa = &(dv->*vli->Labels))->resize(nuk);
      i = 0;
      vl = &(vi->*vli->VL);
      for(vui = vli->vui; vui < vuie; ++vl, ++vui) {
	if ((n = dv->*vui->n) == 0)
	  continue;
	vui->vchk(dv,i,vi);
	if ((sp = vl->s)) {
	  if (vl->n != n)
	    squawk("Expected %d %s_descriptors, but got %d",
		   n, vui->lbl, vl->n);
	  else
	    for(j = 0; j < n; ++i, ++j)
	      (*sa)[i] = sp[j];
	  free(sp);
	}
	else
	  i += n;
      }
    }
  }

  // uncertain string
  for(k = 0; k < NUM_UNC_STR_CONT; ++k) {
    vls = &VLUncertainStr[k];
    havelabels = 0;
    nuk = 0;
    vl = &(vi->*vls->VL);	// "&(...)" to bypass a g++ bug
    vui = vls->vui;
    for(vuie = vui + vls->n; vui < vuie; ++vl, ++vui) {
      nuk += dv->*vui->n;
      if (vl->s)
	++havelabels;
    }
    if (nuk > 0) {
      nutot += nuk;
      if (VLS_aleatory[k])
	nu += nuk;
      if (havelabels)
	(sa = &(dv->*vls->Labels))->resize(nuk);
      i = 0;
      vl = &(vi->*vls->VL);
      for(vui = vls->vui; vui < vuie; ++vl, ++vui) {
	if ((n = dv->*vui->n) == 0)
	  continue;
	vui->vchk(dv,i,vi);
	if ((sp = vl->s)) {
	  if (vl->n != n)
	    squawk("Expected %d %s_descriptors, but got %d",
		   n, vui->lbl, vl->n);
	  else
	    for(j = 0; j < n; ++i, ++j)
	      (*sa)[i] = sp[j];
	  free(sp);
	}
	else
	  i += n;
      }
    }
  }

  // uncertain_correlation_matrix
  if ((Rv = vi->ucm)) {
    n = Rv->length();
    if (n != nu*nu) {
      static char ucmerr[]
	= "Got %lu entries for the uncertain_correlation_matrix\n\
	but needed %lu for %lu uncertain variables";
      squawk(ucmerr, (UL)n, (UL)nu*nu, (UL)nu);
    }
    else {
      Rm = &dv->uncertainCorrelations;
      Rm->reshape(nu);
      for(i = k = 0; i < nu; ++i) {
	for(j = 0; j < nu;)
	  (*Rm)(i,j++) = (*Rv)[k++];
      }
    }
  }

  // The above functions don't validate the initial point; those below do

  // Check discrete design and state set variables.  Call Vchk_* for
  // the discrete design and state set (integer/string/real) types,
  // e.g, Vchk_DiscreteDesSetInt; these check for duplicates, validate
  // that initial point is in the set, and populate arrays of maps or
  // other STL data structures
  nd = 0;
  for(vui=DiscSetLbl, vuie=DiscSetLbl+DiscSetVar_Nkinds; vui<vuie; ++vui) {
    if ((n = dv->*vui->n) > 0) {
      vui->vchk(dv,0,vi);
      nd += n;
    }
  }

  if (nd + nv + nutot == 0)
    squawk("at least one variable must be specified");

  // Check bounds and adjust initial points for remaining types.

  // TODO: these don't support explicit bounds, but their initial
  // point could still be validated here: gamma, gumbel, frechet,
  // weibull, histogram_bin, poisson, binomial, negative_binomial,
  // geometric, hypergeometric, histogram_point

  // TODO: continuous and discrete interval, initial point not checked
  // anywhere?

  // Continuous bound specs: cdv/csv/nuv/lnuv/uuv/luuv/truv/buv
  Var_rcheck *rc, *rce;
  for(rc = var_mp_cbound, rce = rc + Numberof(var_mp_cbound); rc < rce; ++rc)
    Var_RealBoundIPCheck(dv, rc);

  // Discrete bound specs: design,state ranges
  Var_icheck *ic, *ice;
  for(ic = var_mp_drange, ice = ic + Numberof(var_mp_drange); ic < ice; ++ic)
    Var_IntBoundIPCheck(dv, ic);
  

  // deallocate temporary Var_Info data
  n = Numberof(Ia_delete);
  for(i = 0; i < n; i++)
    if ((Ia = vi->*Ia_delete[i]))
      delete Ia;
  n = Numberof(Rv_delete);
  for(i = 0; i < n; i++)
    if ((Rv = vi->*Rv_delete[i]))
      delete Rv;
  n = Numberof(Iv_delete);
  for(i = 0; i < n; i++)
    if ((Iv = vi->*Iv_delete[i]))
      delete Iv;
  n = Numberof(Sa_delete);
  for(i = 0; i < n; i++)
    if ((Sa = vi->*Sa_delete[i]))
      delete Sa;
  delete vi;

  if (nerr)
    abort_handler(PARSE_ERROR);
}


static void flatten_rva(RealVectorArray *rva, RealVector **prv)
{
  size_t i, j, k, m, n;
  RealVector *rv, *rv_i;

  m = rva->size();
  for(i = n = 0; i < m; ++i)
    n += (*rva)[i].length();
  *prv = rv = new RealVector(n, false);
  for(i = k = 0; i < m; ++i) {
    rv_i = &(*rva)[i];
    n = rv_i->length();
    for(j = 0; j < n; ++j, ++k)
      (*rv)[k] = (*rv_i)[j];
  }
}

static void flatten_iva(IntVectorArray *iva, IntVector **piv)
{
  size_t i, j, k, m, n;
  IntVector *iv, *iv_i;

  m = iva->size();
  for(i = n = 0; i < m; ++i)
    n += (*iva)[i].length();
  *piv = iv = new IntVector(n, false);
  for(i = k = 0; i < m; ++i) {
    iv_i = &(*iva)[i];
    n = iv_i->length();
    for(j = 0; j < n; ++j, ++k)
      (*iv)[k] = (*iv_i)[j];
  }
}

static void flatten_rsm(RealSymMatrix *rsm, RealVector **prv)
{
  size_t i, j, m, n;
  RealVector *rv;

  m = rsm->numRows();
  *prv = rv = new RealVector(m*m, false);
  for(i = n = 0; i < m; ++i)
    for(j = 0; j < m; ++j, ++n)
      (*rv)[n] = (*rsm)(i,j);
}

static void flatten_rsa(RealSetArray *rsa, RealVector **prv)
{
  size_t i, j, k, m, n;
  RealVector *rv;
  RealSet *rs_i;
  RealSet::iterator rs_it, rs_ite;

  m = rsa->size();
  for(i = n = 0; i < m; ++i)
    n += (*rsa)[i].size();
  *prv = rv = new RealVector(n, false);
  for(i = k = 0; i < m; ++i) {
    rs_i = &(*rsa)[i];
    for(rs_it=rs_i->begin(), rs_ite=rs_i->end(); rs_it!=rs_ite; ++rs_it, ++k)
      (*rv)[k] = *rs_it;
  }
}

static void flatten_ssa(StringSetArray *ssa, StringArray **psa)
{
  size_t i, j, k, m, n;
  StringArray *sa;
  StringSet *ss_i;
  SSIter ss_it, ss_ite;

  m = ssa->size();
  for(i = n = 0; i < m; ++i)
    n += (*ssa)[i].size();
  *psa = sa = new StringArray(n);
  for(i = k = 0; i < m; ++i) {
    ss_i = &(*ssa)[i];
    for(ss_it=ss_i->begin(), ss_ite=ss_i->end(); ss_it!=ss_ite; ++ss_it, ++k)
      (*sa)[k] = *ss_it;
  }
}

static void flatten_isa(IntSetArray *isa, IntVector **piv)
{
  size_t i, j, k, m, n;
  IntVector *iv;
  IntSet *is_i;
  IntSet::iterator is_it, is_ite;

  m = isa->size();
  for(i = n = 0; i < m; ++i)
    n += (*isa)[i].size();
  *piv = iv = new IntVector(n, false);
  for(i = k = 0; i < m; ++i) {
    is_i = &(*isa)[i];
    for(is_it=is_i->begin(), is_ite=is_i->end(); is_it!=is_ite; ++is_it, ++k)
      (*iv)[k] = *is_it;
  }
}

static void flatten_rrma_keys(RealRealMapArray *rrma, RealVector **prv)
{
  size_t i, j, k, m, n;
  RealVector *rv;
  RealRealMap *rrm_i;
  RealRealMap::iterator rrm_it, rrm_ite;

  m = rrma->size();
  for(i = n = 0; i < m; ++i)
    n += (*rrma)[i].size();
  *prv = rv = new RealVector(n, false);
  for(i = k = 0; i < m; ++i) {
    rrm_i = &(*rrma)[i];
    for (rrm_it=rrm_i->begin(), rrm_ite=rrm_i->end();
	 rrm_it!=rrm_ite; ++rrm_it, ++k)
      (*rv)[k] = rrm_it->first;
  }
}

static void flatten_rrma_values(RealRealMapArray *rrma, RealVector **prv)
{
  size_t i, j, k, m, n;
  RealVector *rv;
  RealRealMap *rrm_i;
  RealRealMap::iterator rrm_it, rrm_ite;

  m = rrma->size();
  for(i = n = 0; i < m; ++i)
    n += (*rrma)[i].size();
  *prv = rv = new RealVector(n, false);
  for(i = k = 0; i < m; ++i) {
    rrm_i = &(*rrma)[i];
    for (rrm_it=rrm_i->begin(), rrm_ite=rrm_i->end();
	 rrm_it!=rrm_ite; ++rrm_it, ++k)
      (*rv)[k] = rrm_it->second;
  }
}

static void flatten_irma_keys(IntRealMapArray *irma, IntVector **piv)
{
  size_t i, j, k, m, n;
  IntVector *iv;
  IntRealMap *irm_i;
  IntRealMap::iterator irm_it, irm_ite;

  m = irma->size();
  for(i = n = 0; i < m; ++i)
    n += (*irma)[i].size();
  *piv = iv = new IntVector(n, false);
  for(i = k = 0; i < m; ++i) {
    irm_i = &(*irma)[i];
    for (irm_it=irm_i->begin(), irm_ite=irm_i->end();
	 irm_it!=irm_ite; ++irm_it, ++k)
      (*iv)[k] = irm_it->first;
  }
}

static void flatten_irma_values(IntRealMapArray *irma, RealVector **prv)
{
  size_t i, j, k, m, n;
  RealVector *rv;
  IntRealMap *irm_i;
  IntRealMap::iterator irm_it, irm_ite;

  m = irma->size();
  for(i = n = 0; i < m; ++i)
    n += (*irma)[i].size();
  *prv = rv = new RealVector(n, false);
  for(i = k = 0; i < m; ++i) {
    irm_i = &(*irma)[i];
    for (irm_it=irm_i->begin(), irm_ite=irm_i->end();
	 irm_it!=irm_ite; ++irm_it, ++k)
      (*rv)[k] = irm_it->second;
  }
}

static void flatten_srma_keys(StringRealMapArray *srma, StringArray **psa)
{
  size_t i, j, k, m, n;
  StringArray *sa;
  StringRealMap *srm_i;
  StringRealMap::iterator srm_it, srm_ite;

  m = srma->size();
  for(i = n = 0; i < m; ++i)
    n += (*srma)[i].size();
  *psa = sa = new StringArray(n);
  for(i = k = 0; i < m; ++i) {
    srm_i = &(*srma)[i];
    for (srm_it=srm_i->begin(), srm_ite=srm_i->end();
	 srm_it!=srm_ite; ++srm_it, ++k)
      (*sa)[k] = srm_it->first;
  }
}

static void flatten_srma_values(StringRealMapArray *srma, RealVector **prv)
{
  size_t i, j, k, m, n;
  RealVector *rv;
  StringRealMap *srm_i;
  StringRealMap::iterator srm_it, srm_ite;

  m = srma->size();
  for(i = n = 0; i < m; ++i)
    n += (*srma)[i].size();
  *prv = rv = new RealVector(n, false);
  for(i = k = 0; i < m; ++i) {
    srm_i = &(*srma)[i];
    for (srm_it=srm_i->begin(), srm_ite=srm_i->end();
	 srm_it!=srm_ite; ++srm_it, ++k)
      (*rv)[k] = srm_it->second;
  }
}

/// Flatten real-valued interval uncertain variable intervals and
/// probabilities back into separate arrays.
static void flatten_real_intervals(const RealRealPairRealMapArray& rrprma, 
				   RealVector **probs, 
				   RealVector **lb, RealVector** ub)
{
  size_t i, j, k, m, n;
  RealVector *rvp, *rvlb, *rvub;

  m = rrprma.size();
  for(i = n = 0; i < m; ++i)
    n += rrprma[i].size();
  *probs = rvp = new RealVector(n, false);
  *lb = rvlb = new RealVector(n, false);
  *ub = rvub = new RealVector(n, false);
  for(i = k = 0; i < m; ++i) {
    const RealRealPairRealMap& rrprm_i = (rrprma)[i];
    RealRealPairRealMap::const_iterator rrprm_it = rrprm_i.begin();
    RealRealPairRealMap::const_iterator rrprm_ite = rrprm_i.end();
    for ( ; rrprm_it != rrprm_ite; ++rrprm_it, ++k) {
      const RealRealPair& interval = rrprm_it->first;
      Real prob = rrprm_it->second;
      (*rvp)[k] = prob;
      (*rvlb)[k] = interval.first;
      (*rvub)[k] = interval.second;
    }
  }
}

/// Flatten integer-valued interval uncertain variable intervals and
/// probabilities back into separate arrays.
static void flatten_int_intervals(const IntIntPairRealMapArray& iiprma, 
				  RealVector **probs, 
				  IntVector **lb, IntVector** ub)
{
  size_t i, j, k, m, n;
  RealVector *ivp;
  IntVector *ivlb, *ivub;

  m = iiprma.size();
  for(i = n = 0; i < m; ++i)
    n += iiprma[i].size();
  *probs = ivp = new RealVector(n, false);
  *lb = ivlb = new IntVector(n, false);
  *ub = ivub = new IntVector(n, false);
  for(i = k = 0; i < m; ++i) {
    const IntIntPairRealMap& iiprm_i = iiprma[i];
    IntIntPairRealMap::const_iterator iiprm_it = iiprm_i.begin();
    IntIntPairRealMap::const_iterator iiprm_ite = iiprm_i.end();
    for ( ; iiprm_it != iiprm_ite; ++iiprm_it, ++k) {
      const IntIntPair& interval = iiprm_it->first;
      Real prob = iiprm_it->second;
      (*ivp)[k] = prob;
      (*ivlb)[k] = interval.first;
      (*ivub)[k] = interval.second;
    }
  }
}


void NIDRProblemDescDB::
check_descriptors(const StringArray& labels) {
  StringArray::const_iterator li = labels.begin();
  String::const_iterator si;
  for(; li != labels.end(); ++li) {
    // error if descriptor contains whitespace
    for(si = li->begin(); si != li->end(); ++si) {
      if(isspace(*si)) {
        Squawk("Descriptor \"%s\" is invalid: whitespace not permitted", 
              li->c_str());
        break;
      }
    }
    if(isfloat(*li)) {
      Squawk("Descriptor \"%s\" is invalid: floating point numbers not permitted",
          li->c_str());
    }
  }
}


void NIDRProblemDescDB::
check_variables(std::list<DataVariables>* dvl)
{
  // BMA: If parse was called, then the Var_Info objects have already
  // been populated; in the case of pure DB insertion in library mode,
  // they won't be and some data from DataVariables needs to be mapped
  // back to the flat Var_Info structures.  Not sure if this correctly
  // captures the case where a parse is followed by a DB update of an
  // input keyword not accounted for below


  // validate descriptors. The string arrays are empty unless the user
  // explicitly set descriptors.
  std::list<DataVariables>::iterator It = dvl->begin(), Ite = dvl->end();
  for(; It != Ite; ++It) {
    const DataVariablesRep* dvr = It->data_rep();
    check_descriptors(dvr->continuousDesignLabels);
    check_descriptors(dvr->discreteDesignRangeLabels);
    check_descriptors(dvr->discreteDesignSetIntLabels);
    check_descriptors(dvr->discreteDesignSetStrLabels);
    check_descriptors(dvr->discreteDesignSetRealLabels);
    check_descriptors(dvr->continuousStateLabels);
    check_descriptors(dvr->discreteStateRangeLabels);
    check_descriptors(dvr->discreteStateSetIntLabels);
    check_descriptors(dvr->discreteStateSetStrLabels);
    check_descriptors(dvr->discreteStateSetRealLabels);
    check_descriptors(dvr->continuousAleatoryUncLabels);
    check_descriptors(dvr->discreteIntAleatoryUncLabels);
    check_descriptors(dvr->discreteStrAleatoryUncLabels);
    check_descriptors(dvr->discreteRealAleatoryUncLabels);
    check_descriptors(dvr->continuousEpistemicUncLabels);
    check_descriptors(dvr->discreteIntEpistemicUncLabels);
    check_descriptors(dvr->discreteStrEpistemicUncLabels);
    check_descriptors(dvr->discreteRealEpistemicUncLabels);
  }

  if (pDDBInstance) {
    std::list<void*>::iterator It, Ite = pDDBInstance->VIL.end();
    for(It = pDDBInstance->VIL.begin(); It != Ite; ++It)
      check_variables_node(*It);
    pDDBInstance->VIL.clear();
  }
  else {
    // library mode with manual provision of everything
    // map all data back into NIDR Var_Info structures
    DataVariablesRep *dv;
    IntArray *ia;
    RealSymMatrix *rsm;
    IntVector *iv_a;
    StringArray *sa_a;
    RealVector *rv, *rv_a, *rv_c;
    RealVectorArray *rva;
    Var_Info *vi;
    size_t i, j, m, n, cntr;
    int num_prs_i, total_prs;

    // copy from DataVariables into Var_Info so that check_variables_node() can
    // go the other direction.  TO DO: can we eliminate this circular update?
    std::list<DataVariables>::iterator It = dvl->begin(), Ite = dvl->end();
    for(; It != Ite; ++It) {

      // create new Var_Info instance to hold DataVariables data

      vi = new Var_Info;
      memset(vi, 0, sizeof(Var_Info));
      vi->dv_handle = &*It;
      vi->dv = dv = It->dataVarsRep;

      // flatten 2D {Real,Int}{Vector,Set}Arrays back into Var_Info 1D arrays

      // discrete design set int vars
      if ((n = dv->numDiscreteDesSetIntVars)) {
	// Note: set consolidation and/or reordering cannot be undone
	flatten_num_array(dv->discreteDesignSetInt, &vi->nddsi);
	flatten_isa(&dv->discreteDesignSetInt,     &vi->ddsi);
      }
      // discrete design set string vars
      if ((n = dv->numDiscreteDesSetStrVars)) {
	// Note: set consolidation and/or reordering cannot be undone
	flatten_num_array(dv->discreteDesignSetStr, &vi->nddss);
	flatten_ssa(&dv->discreteDesignSetStr,     &vi->ddss);
      }
      // discrete design set real vars
      if ((n = dv->numDiscreteDesSetRealVars)) {
	// Note: set consolidation and/or reordering cannot be undone
	flatten_num_array(dv->discreteDesignSetReal, &vi->nddsr);
	flatten_rsa(&dv->discreteDesignSetReal,     &vi->ddsr);
      }
      // histogram bin uncertain vars
      // convert RealRealMapArray to RealVectors of abscissas and counts
      const RealRealMapArray& hbp = dv->histogramUncBinPairs;
      if ((m = hbp.size())) {
	vi->nhbp = ia = new IntArray(m);
	for(i = 0; i < m; ++i)
	  total_prs += (*ia)[i] = hbp[i].size();
	vi->hba = rv_a = new RealVector(total_prs); // abscissas
	vi->hbc = rv_c = new RealVector(total_prs); // counts
	vi->hbo = NULL;                            // no ordinates
	for(i = cntr = 0; i < m; ++i) {
	  RRMCIter it = hbp[i].begin();
	  RRMCIter it_end = hbp[i].end();
	  for( ; it != it_end; ++cntr) {
	    (*rv_a)[cntr] = it->first;   // abscissas
	    (*rv_c)[cntr] = it->second; // counts only (no ordinates)
	  }
	  // normalization occurs in Vchk_HistogramBinUnc going other direction
	}
      }

      // histogram point int uncertain vars
      // convert IntRealMapArray to Int/RealVectors of abscissas and counts
      const IntRealMapArray& hpip = dv->histogramUncPointIntPairs;
      if ((m = hpip.size())) {
	vi->nhpip = ia = new IntArray(m);
	for(i = 0; i < m; ++i)
	  total_prs += (*ia)[i] = hpip[i].size();
	vi->hpia = iv_a = new IntVector(total_prs); // abscissas
	vi->hpic = rv_c = new RealVector(total_prs); // counts
	for(i = cntr = 0; i < m; ++i) {
	  IRMCIter it = hpip[i].begin();
	  IRMCIter it_end = hpip[i].end();
	  for( ; it != it_end; ++cntr) {
	    (*iv_a)[cntr] = it->first;   // abscissas
	    (*rv_c)[cntr] = it->second; // counts only (no ordinates)
	  }
	  // normalization occurs in Vchk_HistogramPtUnc going other direction
	}
      }

      // histogram point string uncertain vars
      // convert StringRealMapArray to String/RealVectors of abscissas and counts
      const StringRealMapArray& hpsp = dv->histogramUncPointStrPairs;
      if ((m = hpsp.size())) {
	vi->nhpsp = ia = new IntArray(m);
	for(i = 0; i < m; ++i)
	  total_prs += (*ia)[i] = hpsp[i].size();
	vi->hpsa = sa_a = new StringArray(total_prs); // abscissas
	vi->hpsc = rv_c = new RealVector(total_prs); // counts
	for(i = cntr = 0; i < m; ++i) {
	  SRMCIter it = hpsp[i].begin();
	  SRMCIter it_end = hpsp[i].end();
	  for( ; it != it_end; ++cntr) {
	    (*sa_a)[cntr] = it->first;   // abscissas
	    (*rv_c)[cntr] = it->second; // counts only (no ordinates)
	  }
	  // normalization occurs in Vchk_HistogramPtUnc going other direction
	}
      }

      // histogram point real uncertain vars
      // convert RealRealMapArray to RealVectors of abscissas and counts
      const RealRealMapArray& hprp = dv->histogramUncPointRealPairs;
      if ((m = hprp.size())) {
	vi->nhprp = ia = new IntArray(m);
	for(i = 0; i < m; ++i)
	  total_prs += (*ia)[i] = hprp[i].size();
	vi->hpra = rv_a = new RealVector(total_prs); // abscissas
	vi->hprc = rv_c = new RealVector(total_prs); // counts
	for(i = cntr = 0; i < m; ++i) {
	  RRMCIter it = hprp[i].begin();
	  RRMCIter it_end = hprp[i].end();
	  for( ; it != it_end; ++cntr) {
	    (*rv_a)[cntr] = it->first;   // abscissas
	    (*rv_c)[cntr] = it->second; // counts only (no ordinates)
	  }
	  // normalization occurs in Vchk_HistogramPtUnc going other direction
	}
      }

      // uncertain correlation matrix
      if (dv->uncertainCorrelations.numRows())
	flatten_rsm(&dv->uncertainCorrelations, &vi->ucm);
      // continuous interval uncertain vars
      if ((n = dv->numContinuousIntervalUncVars)) {
	flatten_num_array(dv->continuousIntervalUncBasicProbs, &vi->nCI);
	// unroll the array of maps in to separate variables (p, lb, ub)
	flatten_real_intervals(dv->continuousIntervalUncBasicProbs, 
			       &vi->CIp, &vi->CIlb, &vi->CIub);
      }
      // discrete interval uncertain vars
      if ((n = dv->numDiscreteIntervalUncVars)) {
	flatten_num_array(dv->discreteIntervalUncBasicProbs, &vi->nDI);
	// unroll the array of maps in to separate variables (p, lb, ub)
	flatten_int_intervals(dv->discreteIntervalUncBasicProbs, 
			      &vi->DIp, &vi->DIlb, &vi->DIub);
      }
      // discrete uncertain set int vars
      if ((n = dv->numDiscreteUncSetIntVars)) {
	// Note: map consolidation and/or reordering cannot be undone
	flatten_num_array(dv->discreteUncSetIntValuesProbs,    &vi->ndusi);
	flatten_irma_keys(&dv->discreteUncSetIntValuesProbs,   &vi->dusi);
	flatten_irma_values(&dv->discreteUncSetIntValuesProbs, &vi->DSIp);
      }
      // discrete uncertain set str vars
      if ((n = dv->numDiscreteUncSetStrVars)) {
	// Note: map consolidation and/or reordering cannot be undone
	flatten_num_array(dv->discreteUncSetStrValuesProbs,    &vi->nduss);
	flatten_srma_keys(&dv->discreteUncSetStrValuesProbs,   &vi->duss);
	flatten_srma_values(&dv->discreteUncSetStrValuesProbs, &vi->DSSp);
      }
      // discrete uncertain set real vars
      if ((n = dv->numDiscreteUncSetRealVars)) {
	// Note: map consolidation and/or reordering cannot be undone
	flatten_num_array(dv->discreteUncSetRealValuesProbs,    &vi->ndusr);
	flatten_rrma_keys(&dv->discreteUncSetRealValuesProbs,   &vi->dusr);
	flatten_rrma_values(&dv->discreteUncSetRealValuesProbs, &vi->DSRp);
      }
      // discrete state set int vars
      if ((n = dv->numDiscreteStateSetIntVars)) {
	// Note: set consolidation and/or reordering cannot be undone
	flatten_num_array(dv->discreteStateSetInt, &vi->ndssi);
	flatten_isa(&dv->discreteStateSetInt,     &vi->dssi);
      }
      // discrete state set string vars
      if ((n = dv->numDiscreteStateSetStrVars)) {
	// Note: set consolidation and/or reordering cannot be undone
	flatten_num_array(dv->discreteStateSetStr, &vi->ndsss);
	flatten_ssa(&dv->discreteStateSetStr,     &vi->dsss);
      }
      // discrete state set real vars
      if ((n = dv->numDiscreteStateSetRealVars)) {
	// Note: set consolidation and/or reordering cannot be undone
	flatten_num_array(dv->discreteStateSetReal, &vi->ndssr);
	flatten_rsa(&dv->discreteStateSetReal,     &vi->dssr);
      }

      check_variables_node((void*)vi);
    }
  }
}

void NIDRProblemDescDB::
var_str(const char *keyname, Values *val, void **g, void *v)
{
  (*(Var_Info**)g)->dv->**(String DataVariablesRep::**)v = *val->s;
}

void NIDRProblemDescDB::
var_strL(const char *keyname, Values *val, void **g, void *v)
{
  StringArray *sa = &((*(Var_Info**)g)->dv->**(StringArray DataVariablesRep::**)v);
  const char **s = val->s;
  size_t i, n = val->n;

  sa->resize(n);
  for(i = 0; i < n; i++)
    (*sa)[i] = s[i];
}

static void
var_iulbl(const char *keyname, Values *val, VarLabel *vl)
{
  char *t;
  const char **s, **sl;
  size_t i, L, n;

  L = n = val->n;	// n for null
  s = val->s;
  for(i = 0; i < n; i++)
    L += strlen(s[i]);
  vl->s = sl = (const char **)malloc(n*sizeof(char*) + L);
  if (!sl)
    NIDRProblemDescDB::botch("malloc failure in var_ulbl");
  vl->n = n;
  t = (char*)(sl + n);
  for(i = 0; i < n; i++) {
    strcpy(t, s[i]);
    sl[i] = (const char*)t;
    t += strlen(t) + 1;
  }
}

void NIDRProblemDescDB::
var_caulbl(const char *keyname, Values *val, void **g, void *v)
{
  // IRIX disallows (int)v, so we use the circumlocution (char*)v - (char*)0.
  VarLabel *vl = &(*(Var_Info**)g)->CAUv[(char*)v - (char*)0];
  var_iulbl(keyname, val, vl);
}

void NIDRProblemDescDB::
var_ceulbl(const char *keyname, Values *val, void **g, void *v)
{
  VarLabel *vl = &(*(Var_Info**)g)->CEUv[(char*)v - (char*)0];
  var_iulbl(keyname, val, vl);
}

void NIDRProblemDescDB::
var_dauilbl(const char *keyname, Values *val, void **g, void *v)
{
  VarLabel *vl = &(*(Var_Info**)g)->DAUIv[(char*)v - (char*)0];
  var_iulbl(keyname, val, vl);
}

void NIDRProblemDescDB::
var_deuilbl(const char *keyname, Values *val, void **g, void *v)
{
  VarLabel *vl = &(*(Var_Info**)g)->DEUIv[(char*)v - (char*)0];
  var_iulbl(keyname, val, vl);
}

void NIDRProblemDescDB::
var_dauslbl(const char *keyname, Values *val, void **g, void *v)
{
  VarLabel *vl = &(*(Var_Info**)g)->DAUSv[(char*)v - (char*)0];
  var_iulbl(keyname, val, vl);
}

void NIDRProblemDescDB::
var_deuslbl(const char *keyname, Values *val, void **g, void *v)
{
  VarLabel *vl = &(*(Var_Info**)g)->DEUSv[(char*)v - (char*)0];
  var_iulbl(keyname, val, vl);
}

void NIDRProblemDescDB::
var_daurlbl(const char *keyname, Values *val, void **g, void *v)
{
  VarLabel *vl = &(*(Var_Info**)g)->DAURv[(char*)v - (char*)0];
  var_iulbl(keyname, val, vl);
}

void NIDRProblemDescDB::
var_deurlbl(const char *keyname, Values *val, void **g, void *v)
{
  VarLabel *vl = &(*(Var_Info**)g)->DEURv[(char*)v - (char*)0];
  var_iulbl(keyname, val, vl);
}

#define MP_(x) DataInterfaceRep::* iface_mp_##x = &DataInterfaceRep::x
#define MP2(x,y) iface_mp_##x##_##y = {&DataInterfaceRep::x,#y}
#define MP2s(x,y) iface_mp_##x##_##y = {&DataInterfaceRep::x,y}
#define MP3(x,y,z) iface_mp_TYPE_DATA_##x##_##z = {&DataInterfaceRep::x,&DataInterfaceRep::y,#z}

static Iface_mp_Rlit
	MP3(failAction,recoveryFnVals,recover);

static Iface_mp_ilit
	MP3(failAction,retryLimit,retry);

static Iface_mp_lit
	MP2(failAction,abort),
	MP2(failAction,continuation);

static Iface_mp_type
	MP2s(analysisScheduling,MASTER_SCHEDULING),
	MP2s(analysisScheduling,PEER_SCHEDULING),
      //MP2s(analysisScheduling,PEER_DYNAMIC_SCHEDULING),
      //MP2s(analysisScheduling,PEER_STATIC_SCHEDULING),
	MP2s(evalScheduling,MASTER_SCHEDULING),
	MP2s(evalScheduling,PEER_DYNAMIC_SCHEDULING),
	MP2s(evalScheduling,PEER_STATIC_SCHEDULING),
	MP2s(asynchLocalEvalScheduling,DYNAMIC_SCHEDULING),
        MP2s(asynchLocalEvalScheduling,STATIC_SCHEDULING),
        MP2s(interfaceSynchronization,ASYNCHRONOUS_INTERFACE),
        MP2s(interfaceSynchronization,SYNCHRONOUS_INTERFACE);

static Iface_mp_utype
	MP2s(interfaceType,TEST_INTERFACE),
	MP2s(interfaceType,FORK_INTERFACE),
	MP2s(interfaceType,GRID_INTERFACE),
	MP2s(interfaceType,MATLAB_INTERFACE),
	MP2s(interfaceType,PYTHON_INTERFACE),
	MP2s(interfaceType,SCILAB_INTERFACE),
	MP2s(interfaceType,SYSTEM_INTERFACE),
	//MP2s(resultsFileFormat,FLEXIBLE_RESULTS), // re-enable when more formats added?
	MP2s(resultsFileFormat,LABELED_RESULTS);

static String
	MP_(algebraicMappings),
	MP_(idInterface),
	MP_(inputFilter),
	MP_(outputFilter),
	MP_(parametersFile),
	MP_(resultsFile),
	MP_(workDir);

static String2DArray
	MP_(analysisComponents);

static StringArray
	MP_(analysisDrivers),
        MP_(copyFiles),
	MP_(linkFiles);

static bool
	MP_(activeSetVectorFlag),
	MP_(allowExistingResultsFlag),
	MP_(apreproFlag),
	MP_(dirSave),
	MP_(dirTag),
	MP_(evalCacheFlag),
	MP_(fileSaveFlag),
	MP_(fileTagFlag),
	MP_(nearbyEvalCacheFlag),
	MP_(numpyFlag),
	MP_(restartFileFlag),
	MP_(templateReplace),
	MP_(useWorkdir),
	MP_(verbatimFlag);

static int
	MP_(analysisServers),
	MP_(asynchLocalAnalysisConcurrency),
	MP_(asynchLocalEvalConcurrency),
	MP_(evalServers),
	MP_(procsPerAnalysis),
	MP_(procsPerEval);

static Real
	MP_(nearbyEvalCacheTol);

#undef MP3
#undef MP2s
#undef MP2
#undef MP_


// Macros for Method

#define MP_(x) DataMethodRep::* method_mp_##x = &DataMethodRep::x
#define MP2(x,y) method_mp_##x##_##y = {&DataMethodRep::x,#y}
#define MP2s(x,y) method_mp_##x##_##y = {&DataMethodRep::x,y}
#if HAVE_OPTPP
#  define MP2o(x,y) method_mp_##x##_##y = {&DataMethodRep::x,OPTPP::y}
#else
#  define MP2o(x,y) method_mp_##x##_##y = {NULL, NULL}
#endif
#define MP2p(x,y) method_mp_##x##_##y = {&DataMethodRep::x,Pecos::y}
#define MP3(x,y,z) method_mp_TYPE_DATA_##x##_##z = {&DataMethodRep::x,&DataMethodRep::y,#z}
#define MP3s(x,y,z) method_mp_TYPE_DATA_##x##_##z = {&DataMethodRep::x,&DataMethodRep::y,z}

static IntVector
	MP_(primeBase),
	MP_(refineSamples),
	MP_(sequenceLeap),
	MP_(sequenceStart),
	MP_(stepsPerVariable);

static Method_mp_ilit2
	MP3(replacementType,numberRetained,chc),
	MP3(replacementType,numberRetained,elitist),
	MP3(replacementType,numberRetained,random);

static Method_mp_ilit2z
	MP3(crossoverType,numCrossPoints,multi_point_binary),
	MP3(crossoverType,numCrossPoints,multi_point_parameterized_binary),
	MP3(crossoverType,numCrossPoints,multi_point_real);

static Method_mp_lit
	MP2(batchSelectionType,naive),
	MP2(batchSelectionType,distance_penalty),
	MP2(batchSelectionType,topology),
	MP2(batchSelectionType,constant_liar),
	MP2(boxDivision,all_dimensions),
	MP2(boxDivision,major_dimension),
	MP2(convergenceType,average_fitness_tracker),
	MP2(convergenceType,best_fitness_tracker),
	MP2(convergenceType,metric_tracker),
	MP2(crossoverType,blend),
	MP2(crossoverType,two_point),
	MP2(crossoverType,uniform),
        MP2(dataDistCovInputType,diagonal),
        MP2(dataDistCovInputType,matrix),
      //MP2(dataDistType,gaussian),
      //MP2(dataDistType,user),
	MP2(discrepancyType,global_kriging),
	MP2(discrepancyType,global_polynomial),
	MP2(evalSynchronize,blocking),
	MP2(evalSynchronize,nonblocking),
	MP2(expansionSampleType,incremental_lhs),
	MP2(exploratoryMoves,adaptive),
	MP2(exploratoryMoves,multi_step),
	MP2(exploratoryMoves,simple),
	MP2(fitnessType,domination_count),
	MP2(fitnessType,layer_rank),
	MP2(fitnessType,linear_rank),
	MP2(fitnessType,merit_function),
	MP2(fitnessType,proportional),
	MP2(fitnessMetricType,predicted_variance),
	MP2(fitnessMetricType,distance),
	MP2(fitnessMetricType,gradient),
      //MP2(hybridCollabType,abo),
      //MP2(hybridCollabType,hops),
      //MP2(seqHybridType,adaptive),
	MP2(initializationType,random),
	MP2(initializationType,unique_random),
	MP2(lipschitzType,global),
	MP2(lipschitzType,local),
        MP2(meritFunction,merit_max),
        MP2(meritFunction,merit_max_smooth),
        MP2(meritFunction,merit1),
        MP2(meritFunction,merit1_smooth),
        MP2(meritFunction,merit2),
        MP2(meritFunction,merit2_smooth),
        MP2(meritFunction,merit2_squared),
	MP2(mcmcType,adaptive_metropolis),
	MP2(mcmcType,delayed_rejection),
	MP2(mcmcType,dram),
	MP2(mcmcType,metropolis_hastings),
	MP2(mcmcType,multilevel),
	MP2(mutationType,bit_random),
	MP2(mutationType,offset_cauchy),
	MP2(mutationType,offset_normal),
	MP2(mutationType,offset_uniform),
	MP2(mutationType,replace_uniform),
	MP2(patternBasis,coordinate),
	MP2(patternBasis,simplex),
	MP2(pointReuse,all),
        MP2(proposalCovInputType,diagonal),
        MP2(proposalCovInputType,matrix),
        MP2(proposalCovType,derivatives),
        MP2(proposalCovType,prior),
        MP2(proposalCovType,user),
	MP2(reliabilityIntegration,first_order),
	MP2(reliabilityIntegration,second_order),
	MP2(replacementType,elitist),
	MP2(replacementType,favor_feasible),
	MP2(replacementType,roulette_wheel),
	MP2(replacementType,unique_roulette_wheel),
	MP2(rngName,mt19937),
	MP2(rngName,rnum2),
	MP2(searchMethod,gradient_based_line_search),
	MP2(searchMethod,tr_pds),
	MP2(searchMethod,trust_region),
	MP2(searchMethod,value_based_line_search),
	MP2(trialType,grid),
	MP2(trialType,halton),
	MP2(trialType,random),
        MP2(useSurrogate,inform_search),
        MP2(useSurrogate,optimize);

static Method_mp_litc
	MP3(crossoverType,crossoverRate,shuffle_random),
	MP3(crossoverType,crossoverRate,null_crossover),
	MP3(mutationType,mutationRate,null_mutation),
	MP3(mutationType,mutationRate,offset_cauchy),
	MP3(mutationType,mutationRate,offset_normal),
	MP3(mutationType,mutationRate,offset_uniform),
	MP3(replacementType,fitnessLimit,below_limit);

static Method_mp_litrv
	MP3(nichingType,nicheVector,distance),
	MP3(nichingType,nicheVector,max_designs),
	MP3(nichingType,nicheVector,radial),
	MP3(postProcessorType,distanceVector,distance_postprocessor);

static Method_mp_slit2
	MP3(initializationType,flatFile,flat_file);

static Method_mp_utype_lit
        MP3s(methodName,dlDetails,DL_SOLVER); // struct order: ip, sp, utype

static Method_mp_ord
	MP2s(approxCorrectionOrder,0),
	MP2s(approxCorrectionOrder,1),
	MP2s(approxCorrectionOrder,2);

static Real
	MP_(absConvTol),
	MP_(centeringParam),
	MP_(collocationRatio),
	MP_(collocRatioTermsOrder),
	MP_(constraintPenalty),
	MP_(constrPenalty),
	MP_(constraintTolerance),
	MP_(contractFactor),
	MP_(contractStepLength),
	MP_(convergenceTolerance),
	MP_(crossoverRate),
	MP_(falseConvTol),
	MP_(functionPrecision),
	MP_(globalBalanceParam),
	MP_(gradientTolerance),
      //MP_(hybridProgThresh),
	MP_(hybridLSProb),
	MP_(grThreshold),
	MP_(initDelta),
	MP_(initMeshSize),
	MP_(initStepLength),
	MP_(initTRRadius),
	MP_(lineSearchTolerance),
	MP_(localBalanceParam),
	MP_(maxBoxSize),
	MP_(maxStep),
	MP_(minBoxSize),
	MP_(minMeshSize),
	MP_(mutationRate),
	MP_(mutationScale),
	MP_(percentVarianceExplained),
	MP_(refinementRate),
	MP_(regressionL2Penalty),
	MP_(shrinkagePercent),	// should be called shrinkageFraction
	MP_(singConvTol),
	MP_(singRadius),
        MP_(smoothFactor),
 	MP_(solnTarget),
	MP_(stepLenToBoundary),
	MP_(threshDelta),
	MP_(threshStepLength),
	MP_(trustRegionContract),
	MP_(trustRegionContractTrigger),
	MP_(trustRegionExpand),
	MP_(trustRegionExpandTrigger),
	MP_(trustRegionMinSize),
	MP_(vbdDropTolerance),
	MP_(volBoxSize),
	MP_(vns),
	MP_(wilksConfidenceLevel),
	MP_(xConvTol);

static RealVector
	MP_(anisoDimPref),
	MP_(concurrentParameterSets),
	MP_(dataDistCovariance),
	MP_(dataDistMeans),
	MP_(finalPoint),
	MP_(hyperPriorAlphas),
	MP_(hyperPriorBetas),
	MP_(listOfPoints),
	MP_(predictionConfigList),
	MP_(proposalCovData),
	MP_(regressionNoiseTol),
        MP_(stepVector),
	MP_(trustRegionInitSize);

static RealVectorArray
	MP_(genReliabilityLevels),
	MP_(probabilityLevels),
	MP_(reliabilityLevels),
	MP_(responseLevels);

static unsigned short
	MP_(adaptedBasisAdvancements),
      //MP_(adaptedBasisInitLevel),
	MP_(cubIntOrder),
	MP_(softConvLimit),
	MP_(vbdOrder),
	MP_(wilksOrder);

static SizetArray
	MP_(collocationPoints),
        MP_(expansionSamples),
  	MP_(pilotSamples);

static UShortArray
        MP_(expansionOrder),
        MP_(quadratureOrder),
	MP_(sparseGridLevel),
        MP_(tensorGridOrder),
	MP_(varPartitions);

static String
        MP_(betaSolverName),
        MP_(dataDistFile),
        MP_(displayFormat),
	MP_(exportApproxPtsFile),
	MP_(exportCorrModelFile),
	MP_(exportCorrVarFile),
	MP_(exportDiscrepFile),
	MP_(exportExpansionFile),
	MP_(exportMCMCPtsFile),
	MP_(historyFile),
	MP_(hybridGlobalMethodName),
	MP_(hybridGlobalMethodPointer),
	MP_(hybridGlobalModelPointer),
	MP_(hybridLocalMethodName),
	MP_(hybridLocalMethodPointer),
	MP_(hybridLocalModelPointer),
	MP_(idMethod),
	MP_(importApproxPtsFile),
	MP_(importBuildPtsFile),
	MP_(importCandPtsFile),
	MP_(importExpansionFile),
	MP_(importPredConfigs),
	MP_(logFile),
	MP_(lowFidModelPointer),
	MP_(modelPointer),
        MP_(posteriorDensityExportFilename),
        MP_(posteriorSamplesExportFilename),
        MP_(posteriorSamplesImportFilename),
	MP_(proposalCovFile),
	MP_(pstudyFilename),
        MP_(quesoOptionsFilename),
	MP_(subMethodName),
        MP_(subMethodPointer),
        MP_(subModelPointer);

static StringArray
	MP_(hybridMethodNames),
	MP_(hybridMethodPointers),
	MP_(hybridModelPointers),
        MP_(miscOptions);

static bool
	MP_(adaptExpDesign),
	MP_(adaptPosteriorRefine),
	MP_(backfillFlag),
	MP_(calModelDiscrepancy),
	MP_(constantPenalty),
	MP_(crossValidation),
	MP_(crossValidNoiseOnly),
	MP_(dOptimal),
        MP_(evaluatePosteriorDensity),
	MP_(expansionFlag),
	MP_(exportSampleSeqFlag),
	MP_(fixedSeedFlag),
	MP_(fixedSequenceFlag),
        MP_(generatePosteriorSamples),
	MP_(gpmsaNormalize),
	MP_(importApproxActive),
	MP_(importBuildActive),
	MP_(latinizeFlag),
	MP_(logitTransform),
	MP_(mainEffectsFlag),
	MP_(methodScaling),
	MP_(methodUseDerivsFlag),
	MP_(mutationAdaptive),
	MP_(normalizedCoeffs),
	MP_(pcaFlag),
	MP_(posteriorStatsKL),
	MP_(posteriorStatsMutual),
	MP_(printPopFlag),
	MP_(pstudyFileActive),
	MP_(randomizeOrderFlag),
	MP_(regressDiag),
	MP_(showAllEval),
	MP_(showMiscOptions),
	MP_(speculativeFlag),
	MP_(standardizedSpace),
	MP_(tensorGridFlag),
	MP_(surrBasedGlobalReplacePts),
	MP_(surrBasedLocalLayerBypass),
	MP_(vbdFlag),
	MP_(volQualityFlag),
	MP_(wilksFlag);

static short
        MP_(expansionType),
	MP_(nestingOverride),
	MP_(refinementType),
	MP_(wilksSidedInterval);

static int
	MP_(buildSamples),
	MP_(burnInSamples),
	MP_(chainSamples),
	MP_(concurrentRandomJobs),
	MP_(contractAfterFail),
	MP_(covarianceType),
        MP_(crossoverChainPairs),
        MP_(emulatorOrder),
	MP_(expandAfterSuccess),
        MP_(iteratorServers),
	MP_(jumpStep),
	MP_(maxFunctionEvaluations),
	MP_(maxHifiEvals),
	MP_(maxIterations),
	MP_(maxRefineIterations),
	MP_(maxSolverIterations),
	MP_(mutationRange),
        MP_(neighborOrder),
	MP_(newSolnsGenerated),
	MP_(numChains),
	MP_(numCR),
	MP_(numSamples),
	MP_(numSteps),
	MP_(numSymbols),
	MP_(numTrials),
	MP_(populationSize),
        MP_(procsPerIterator),
        MP_(proposalCovUpdates),
	MP_(randomSeed),
	MP_(samplesOnEmulator),
	MP_(searchSchemeSize),
	MP_(subSamplingPeriod),
	MP_(totalPatternSize),
	MP_(verifyLevel);

static size_t
        MP_(numCandidateDesigns),
	MP_(numCandidates),
        MP_(numDesigns),
        MP_(numFinalSolutions),
	MP_(numGenerations),
	MP_(numOffspring),
	MP_(numParents),
  	MP_(numPredConfigs);

static Method_mp_type
	MP2s(covarianceControl,DIAGONAL_COVARIANCE),
	MP2s(covarianceControl,FULL_COVARIANCE),
	MP2s(distributionType,COMPLEMENTARY),
	MP2s(distributionType,CUMULATIVE),
	MP2s(emulatorType,GP_EMULATOR),
	MP2s(emulatorType,KRIGING_EMULATOR),
	MP2s(emulatorType,PCE_EMULATOR),
	MP2s(emulatorType,SC_EMULATOR),
	MP2s(emulatorType,VPS_EMULATOR),
	MP2p(expansionBasisType,ADAPTED_BASIS_EXPANDING_FRONT),
	MP2p(expansionBasisType,ADAPTED_BASIS_GENERALIZED),
	MP2p(expansionBasisType,HIERARCHICAL_INTERPOLANT),
	MP2p(expansionBasisType,NODAL_INTERPOLANT),
	MP2p(expansionBasisType,TENSOR_PRODUCT_BASIS),
	MP2p(expansionBasisType,TOTAL_ORDER_BASIS),
	MP2s(expansionType,ASKEY_U),
	MP2s(expansionType,STD_NORMAL_U),
	MP2s(finalMomentsType,CENTRAL_MOMENTS),
	MP2s(finalMomentsType,NO_MOMENTS),
	MP2s(finalMomentsType,STANDARD_MOMENTS),
	MP2p(growthOverride,RESTRICTED),                   // Pecos enumeration
	MP2p(growthOverride,UNRESTRICTED),                 // Pecos enumeration
	MP2s(iteratorScheduling,MASTER_SCHEDULING),
	MP2s(iteratorScheduling,PEER_SCHEDULING),
      //MP2s(iteratorScheduling,PEER_DYNAMIC_SCHEDULING),
      //MP2s(iteratorScheduling,PEER_STATIC_SCHEDULING),
	MP2s(lsRegressionType,EQ_CON_LS),
	MP2s(lsRegressionType,SVD_LS),
	MP2o(meritFn,ArgaezTapia),                         // OPTPP enumeration
	MP2o(meritFn,NormFmu),                             // OPTPP enumeration
	MP2o(meritFn,VanShanno),                           // OPTPP enumeration
	MP2s(methodOutput,DEBUG_OUTPUT),
	MP2s(methodOutput,NORMAL_OUTPUT),
	MP2s(methodOutput,QUIET_OUTPUT),
	MP2s(methodOutput,SILENT_OUTPUT),
	MP2s(methodOutput,VERBOSE_OUTPUT),
	MP2p(nestingOverride,NESTED),                      // Pecos enumeration
	MP2p(nestingOverride,NON_NESTED),                  // Pecos enumeration
	MP2p(refinementControl,DIMENSION_ADAPTIVE_CONTROL_GENERALIZED),// Pecos
	MP2p(refinementControl,DIMENSION_ADAPTIVE_CONTROL_DECAY),      // Pecos
	MP2p(refinementControl,DIMENSION_ADAPTIVE_CONTROL_SOBOL),      // Pecos
	MP2p(refinementControl,LOCAL_ADAPTIVE_CONTROL),                // Pecos
	MP2p(refinementControl,UNIFORM_CONTROL),                       // Pecos
	MP2p(refinementType,P_REFINEMENT),                 // Pecos enumeration
        MP2p(refinementType,H_REFINEMENT),                 // Pecos enumeration
	MP2p(regressionType,BASIS_PURSUIT),                // Pecos enumeration
	MP2p(regressionType,BASIS_PURSUIT_DENOISING),      // Pecos enumeration
	MP2p(regressionType,DEFAULT_LEAST_SQ_REGRESSION),  // Pecos enumeration
	MP2p(regressionType,LASSO_REGRESSION),             // Pecos enumeration
	MP2p(regressionType,LEAST_ANGLE_REGRESSION),       // Pecos enumeration
	MP2p(regressionType,ORTHOG_LEAST_INTERPOLATION),   // Pecos enumeration
	MP2p(regressionType,ORTHOG_MATCH_PURSUIT),         // Pecos enumeration
	MP2s(responseLevelTarget,GEN_RELIABILITIES),
	MP2s(responseLevelTarget,PROBABILITIES),
	MP2s(responseLevelTarget,RELIABILITIES),
	MP2s(responseLevelTargetReduce,SYSTEM_PARALLEL),
	MP2s(responseLevelTargetReduce,SYSTEM_SERIES),
	MP2s(surrBasedLocalAcceptLogic,FILTER),
	MP2s(surrBasedLocalAcceptLogic,TR_RATIO),
	MP2s(surrBasedLocalConstrRelax,HOMOTOPY),
	MP2s(surrBasedLocalMeritFn,ADAPTIVE_PENALTY_MERIT),
	MP2s(surrBasedLocalMeritFn,AUGMENTED_LAGRANGIAN_MERIT),
	MP2s(surrBasedLocalMeritFn,LAGRANGIAN_MERIT),
	MP2s(surrBasedLocalMeritFn,PENALTY_MERIT),
	MP2s(surrBasedLocalSubProbCon,LINEARIZED_CONSTRAINTS),
	MP2s(surrBasedLocalSubProbCon,NO_CONSTRAINTS),
	MP2s(surrBasedLocalSubProbCon,ORIGINAL_CONSTRAINTS),
	MP2s(surrBasedLocalSubProbObj,AUGMENTED_LAGRANGIAN_OBJECTIVE),
	MP2s(surrBasedLocalSubProbObj,LAGRANGIAN_OBJECTIVE),
	MP2s(surrBasedLocalSubProbObj,ORIGINAL_PRIMARY),
	MP2s(surrBasedLocalSubProbObj,SINGLE_OBJECTIVE),
	MP2s(wilksSidedInterval,ONE_SIDED_LOWER),
	MP2s(wilksSidedInterval,ONE_SIDED_UPPER),
	MP2s(wilksSidedInterval,TWO_SIDED);

static Method_mp_utype
	MP2s(calibrateErrorMode,CALIBRATE_ONE),
	MP2s(calibrateErrorMode,CALIBRATE_PER_EXPER),
	MP2s(calibrateErrorMode,CALIBRATE_PER_RESP),
	MP2s(calibrateErrorMode,CALIBRATE_BOTH),
        MP2s(exportApproxFormat,TABULAR_NONE),
        MP2s(exportApproxFormat,TABULAR_HEADER),
        MP2s(exportApproxFormat,TABULAR_EVAL_ID),
        MP2s(exportApproxFormat,TABULAR_IFACE_ID),
        MP2s(exportApproxFormat,TABULAR_ANNOTATED),
        MP2s(exportCorrModelFormat,TABULAR_NONE),
        MP2s(exportCorrModelFormat,TABULAR_HEADER),
        MP2s(exportCorrModelFormat,TABULAR_EVAL_ID),
        MP2s(exportCorrModelFormat,TABULAR_IFACE_ID),
        MP2s(exportCorrModelFormat,TABULAR_ANNOTATED),
        MP2s(exportCorrVarFormat,TABULAR_NONE),
        MP2s(exportCorrVarFormat,TABULAR_HEADER),
        MP2s(exportCorrVarFormat,TABULAR_EVAL_ID),
        MP2s(exportCorrVarFormat,TABULAR_IFACE_ID),
        MP2s(exportCorrVarFormat,TABULAR_ANNOTATED),
        MP2s(exportDiscrepFormat,TABULAR_NONE),
        MP2s(exportDiscrepFormat,TABULAR_HEADER),
        MP2s(exportDiscrepFormat,TABULAR_EVAL_ID),
        MP2s(exportDiscrepFormat,TABULAR_IFACE_ID),
        MP2s(exportDiscrepFormat,TABULAR_ANNOTATED),
        MP2s(exportSamplesFormat,TABULAR_NONE),
        MP2s(exportSamplesFormat,TABULAR_HEADER),
        MP2s(exportSamplesFormat,TABULAR_EVAL_ID),
        MP2s(exportSamplesFormat,TABULAR_IFACE_ID),
        MP2s(exportSamplesFormat,TABULAR_ANNOTATED),
        MP2s(importApproxFormat,TABULAR_NONE),
        MP2s(importApproxFormat,TABULAR_HEADER),
        MP2s(importApproxFormat,TABULAR_EVAL_ID),
        MP2s(importApproxFormat,TABULAR_IFACE_ID),
        MP2s(importApproxFormat,TABULAR_ANNOTATED),
        MP2s(importBuildFormat,TABULAR_NONE),
        MP2s(importBuildFormat,TABULAR_HEADER),
        MP2s(importBuildFormat,TABULAR_EVAL_ID),
        MP2s(importBuildFormat,TABULAR_IFACE_ID),
        MP2s(importBuildFormat,TABULAR_ANNOTATED),
        MP2s(importCandFormat,TABULAR_NONE),
        MP2s(importCandFormat,TABULAR_HEADER),
        MP2s(importCandFormat,TABULAR_EVAL_ID),
        MP2s(importCandFormat,TABULAR_IFACE_ID),
        MP2s(importCandFormat,TABULAR_ANNOTATED),
        MP2s(importPredConfigFormat,TABULAR_NONE),
        MP2s(importPredConfigFormat,TABULAR_HEADER),
        MP2s(importPredConfigFormat,TABULAR_EVAL_ID),
        MP2s(importPredConfigFormat,TABULAR_IFACE_ID),
        MP2s(importPredConfigFormat,TABULAR_ANNOTATED),
	MP2s(integrationRefine,AIS),
	MP2s(integrationRefine,IS),
	MP2s(integrationRefine,MMAIS),
	MP2s(methodName,ASYNCH_PATTERN_SEARCH),
	MP2s(methodName,BRANCH_AND_BOUND),
	MP2s(methodName,COLINY_BETA),
	MP2s(methodName,COLINY_COBYLA),
	MP2s(methodName,COLINY_DIRECT),
	MP2s(methodName,COLINY_EA),
	MP2s(methodName,COLINY_PATTERN_SEARCH),
	MP2s(methodName,COLINY_SOLIS_WETS),
	MP2s(methodName,CONMIN_FRCG),
	MP2s(methodName,CONMIN_MFD),
	MP2s(methodName,DACE),
	MP2s(methodName,DATA_FIT_SURROGATE_BASED_LOCAL),
	MP2s(methodName,DOT_BFGS),
	MP2s(methodName,DOT_FRCG),
	MP2s(methodName,DOT_MMFD),
	MP2s(methodName,DOT_SLP),
	MP2s(methodName,DOT_SQP),
	MP2s(methodName,EFFICIENT_GLOBAL),
	MP2s(methodName,FSU_CVT),
	MP2s(methodName,FSU_HALTON),
	MP2s(methodName,FSU_HAMMERSLEY),
	MP2s(methodName,HIERARCH_SURROGATE_BASED_LOCAL),
	MP2s(methodName,HYBRID),
	MP2s(methodName,MESH_ADAPTIVE_SEARCH),
	MP2s(methodName,MOGA),
	MP2s(methodName,MULTI_START),
	MP2s(methodName,NCSU_DIRECT),
	MP2s(methodName,NL2SOL),
	MP2s(methodName,NLPQL_SQP),
	MP2s(methodName,NLSSOL_SQP),
	MP2s(methodName,MIT_NOWPAC),
	MP2s(methodName,MIT_SNOWPAC),
        MP2s(methodName,ADAPTIVE_SAMPLING),
	MP2s(methodName,BAYES_CALIBRATION),
	MP2s(methodName,GENIE_DIRECT),
	MP2s(methodName,GENIE_OPT_DARTS),
        MP2s(methodName,GPAIS),
	MP2s(methodName,GLOBAL_EVIDENCE),
        MP2s(methodName,GLOBAL_INTERVAL_EST),
	MP2s(methodName,GLOBAL_RELIABILITY),
        MP2s(methodName,IMPORTANCE_SAMPLING),
 	MP2s(methodName,LOCAL_EVIDENCE),
        MP2s(methodName,LOCAL_INTERVAL_EST),
	MP2s(methodName,LOCAL_RELIABILITY),
	MP2s(methodName,MULTILEVEL_SAMPLING),
        MP2s(methodName,POF_DARTS),
	MP2s(methodName,RKD_DARTS),
	MP2s(methodName,POLYNOMIAL_CHAOS),
	MP2s(methodName,RANDOM_SAMPLING),
	MP2s(methodName,STOCH_COLLOCATION),
	MP2s(methodName,NONLINEAR_CG),
	MP2s(methodName,NPSOL_SQP),
	MP2s(methodName,OPTPP_CG),
	MP2s(methodName,OPTPP_FD_NEWTON),
	MP2s(methodName,OPTPP_G_NEWTON),
	MP2s(methodName,OPTPP_NEWTON),
	MP2s(methodName,OPTPP_PDS),
	MP2s(methodName,OPTPP_Q_NEWTON),
	MP2s(methodName,PARETO_SET),
	MP2s(methodName,PSUADE_MOAT),
	MP2s(methodName,RICHARDSON_EXTRAP),
	MP2s(methodName,SOGA),
	MP2s(methodName,SURROGATE_BASED_GLOBAL),
	MP2s(methodName,SURROGATE_BASED_LOCAL),
	MP2s(methodName,VECTOR_PARAMETER_STUDY),
	MP2s(methodName,LIST_PARAMETER_STUDY),
	MP2s(methodName,CENTERED_PARAMETER_STUDY),
	MP2s(methodName,MULTIDIM_PARAMETER_STUDY),
	MP2s(preSolveMethod,SUBMETHOD_NIP),
	MP2s(preSolveMethod,SUBMETHOD_NONE),
	MP2s(preSolveMethod,SUBMETHOD_SQP),
	MP2s(pstudyFileFormat,TABULAR_NONE),
        MP2s(pstudyFileFormat,TABULAR_HEADER),
        MP2s(pstudyFileFormat,TABULAR_EVAL_ID),
        MP2s(pstudyFileFormat,TABULAR_IFACE_ID),
        MP2s(pstudyFileFormat,TABULAR_ANNOTATED),
	MP2s(reliabilitySearchType,AMV_PLUS_U),
	MP2s(reliabilitySearchType,AMV_PLUS_X),
	MP2s(reliabilitySearchType,AMV_U),
	MP2s(reliabilitySearchType,AMV_X),
	MP2s(reliabilitySearchType,EGRA_U),
	MP2s(reliabilitySearchType,EGRA_X),
	MP2s(reliabilitySearchType,NO_APPROX),
	MP2s(reliabilitySearchType,TANA_U),
	MP2s(reliabilitySearchType,TANA_X),
	MP2s(sampleType,SUBMETHOD_LHS),
	MP2s(sampleType,SUBMETHOD_RANDOM),
	MP2s(subMethod,SUBMETHOD_COLLABORATIVE),
	MP2s(subMethod,SUBMETHOD_EMBEDDED),
	MP2s(subMethod,SUBMETHOD_SEQUENTIAL),
	MP2s(subMethod,SUBMETHOD_DREAM),
	MP2s(subMethod,SUBMETHOD_WASABI),
	MP2s(subMethod,SUBMETHOD_GPMSA),
	MP2s(subMethod,SUBMETHOD_QUESO),
	MP2s(subMethod,SUBMETHOD_NIP),
	MP2s(subMethod,SUBMETHOD_SQP),
	MP2s(subMethod,SUBMETHOD_EA),
	MP2s(subMethod,SUBMETHOD_EGO),
	MP2s(subMethod,SUBMETHOD_SBO),
	MP2s(subMethod,SUBMETHOD_LHS),
	MP2s(subMethod,SUBMETHOD_RANDOM),
	MP2s(subMethod,SUBMETHOD_OA_LHS),
	MP2s(subMethod,SUBMETHOD_OAS),
	MP2s(subMethod,SUBMETHOD_BOX_BEHNKEN),
	MP2s(subMethod,SUBMETHOD_CENTRAL_COMPOSITE),
	MP2s(subMethod,SUBMETHOD_GRID),
	MP2s(subMethod,SUBMETHOD_CONVERGE_ORDER),
	MP2s(subMethod,SUBMETHOD_CONVERGE_QOI),
        MP2s(subMethod,SUBMETHOD_ESTIMATE_ORDER);

#undef MP3s
#undef MP3
#undef MP2p
#undef MP2o
#undef MP2s
#undef MP2
#undef MP_


// Macros for handling Model data

#define MP_(x) DataModelRep::* model_mp_##x = &DataModelRep::x
#define MP2(x,y) model_mp_##x##_##y = {&DataModelRep::x,#y}
#define MP2s(x,y) model_mp_##x##_##y = {&DataModelRep::x,y}

static IntSet
	MP_(surrogateFnIndices);

static Model_mp_lit
	MP2(approxPointReuse,all),
	MP2(approxPointReuse,none),
	MP2(approxPointReuse,region),
	MP2(marsInterpolation,linear),
	MP2(marsInterpolation,cubic),
	MP2(modelType,active_subspace),
	MP2(modelType,adapted_basis),
	MP2(modelType,nested),
	MP2(modelType,random_field),
	MP2(modelType,simulation),
	MP2(modelType,surrogate),
	MP2(surrogateType,hierarchical),
	MP2(surrogateType,global_gaussian),
	MP2(surrogateType,global_kriging),
	MP2(surrogateType,global_mars),
	MP2(surrogateType,global_moving_least_squares),
	MP2(surrogateType,global_neural_network),
	MP2(surrogateType,global_polynomial),
	MP2(surrogateType,global_radial_basis),
	MP2(surrogateType,global_voronoi_surrogate),
	MP2(surrogateType,local_taylor),
        MP2(surrogateType,multipoint_tana),
        MP2(trendOrder,constant),
        MP2(trendOrder,linear),
        MP2(trendOrder,reduced_quadratic),
        MP2(trendOrder,quadratic);

static Model_mp_ord
	MP2s(approxCorrectionOrder,0),
	MP2s(approxCorrectionOrder,1),
	MP2s(approxCorrectionOrder,2),
	MP2s(polynomialOrder,1),
	MP2s(polynomialOrder,2),
        MP2s(polynomialOrder,3);

static Model_mp_type
	MP2s(approxCorrectionType,ADDITIVE_CORRECTION),
	MP2s(approxCorrectionType,COMBINED_CORRECTION),
	MP2s(approxCorrectionType,MULTIPLICATIVE_CORRECTION),
	MP2s(pointsManagement,MINIMUM_POINTS),
	MP2s(pointsManagement,RECOMMENDED_POINTS),
	MP2s(subMethodScheduling,MASTER_SCHEDULING),
	MP2s(subMethodScheduling,PEER_SCHEDULING);
      //MP2s(subMethodScheduling,PEER_DYNAMIC_SCHEDULING),
      //MP2s(subMethodScheduling,PEER_STATIC_SCHEDULING),
        

static Model_mp_utype
        MP2s(analyticCovIdForm,EXP_L2),
        MP2s(analyticCovIdForm,EXP_L1),
        MP2s(exportApproxFormat,TABULAR_NONE),
        MP2s(exportApproxFormat,TABULAR_HEADER),
        MP2s(exportApproxFormat,TABULAR_EVAL_ID),
        MP2s(exportApproxFormat,TABULAR_IFACE_ID),
        MP2s(exportApproxFormat,TABULAR_ANNOTATED),
      //MP2s(importApproxFormat,TABULAR_NONE),
      //MP2s(importApproxFormat,TABULAR_HEADER),
      //MP2s(importApproxFormat,TABULAR_EVAL_ID),
      //MP2s(importApproxFormat,TABULAR_IFACE_ID),
      //MP2s(importApproxFormat,TABULAR_ANNOTATED),
        MP2s(importBuildFormat,TABULAR_NONE),
        MP2s(importBuildFormat,TABULAR_HEADER),
        MP2s(importBuildFormat,TABULAR_EVAL_ID),
        MP2s(importBuildFormat,TABULAR_IFACE_ID),
        MP2s(importBuildFormat,TABULAR_ANNOTATED),
        MP2s(importChallengeFormat,TABULAR_NONE),
        MP2s(importChallengeFormat,TABULAR_HEADER),
        MP2s(importChallengeFormat,TABULAR_EVAL_ID),
        MP2s(importChallengeFormat,TABULAR_IFACE_ID),
        MP2s(importChallengeFormat,TABULAR_ANNOTATED),
        MP2s(modelExportFormat,TEXT_ARCHIVE),
        MP2s(modelExportFormat,BINARY_ARCHIVE),
        MP2s(modelExportFormat,ALGEBRAIC_FILE),
        MP2s(modelExportFormat,ALGEBRAIC_CONSOLE),
        MP2s(randomFieldIdForm,RF_KARHUNEN_LOEVE),
        MP2s(randomFieldIdForm,RF_PCA_GP),
	      MP2s(subspaceNormalization,SUBSPACE_NORM_MEAN_VALUE),
	      MP2s(subspaceNormalization,SUBSPACE_NORM_MEAN_GRAD),
	      MP2s(subspaceNormalization,SUBSPACE_NORM_LOCAL_GRAD),
	      MP2s(subspaceSampleType,SUBMETHOD_LHS),
	      MP2s(subspaceSampleType,SUBMETHOD_RANDOM),
	      MP2s(subspaceIdCVMethod,MINIMUM_METRIC),
	      MP2s(subspaceIdCVMethod,RELATIVE_TOLERANCE),
	      MP2s(subspaceIdCVMethod,DECREASE_TOLERANCE);

static Real
        MP_(adaptedBasisCollocRatio),
        MP_(annRange),
	MP_(convergenceTolerance),
        MP_(discontGradThresh),
        MP_(discontJumpThresh),
	MP_(krigingNugget),
	MP_(percentFold),
	MP_(truncationTolerance),
	MP_(relTolerance),
	MP_(decreaseTolerance);

static RealVector
	MP_(krigingCorrelations),
      //MP_(krigingMaxCorrelations),
      //MP_(krigingMinCorrelations),
	MP_(primaryRespCoeffs),
	MP_(secondaryRespCoeffs),
  	MP_(solutionLevelCost);

static IntVector
	MP_(refineSamples);

static String
	MP_(actualModelPointer),
	MP_(decompCellType),
	MP_(exportApproxPtsFile),
	MP_(idModel),
      //MP_(importApproxPtsFile),
	MP_(importBuildPtsFile),
	MP_(importChallengePtsFile),
	MP_(interfacePointer),
	MP_(krigingOptMethod),
	MP_(modelExportPrefix),
	MP_(optionalInterfRespPointer),
	MP_(propagationModelPointer),
	MP_(refineCVMetric),
	MP_(responsesPointer),
	MP_(rfDataFileName),
	MP_(solutionLevelControl),
	MP_(subMethodPointer),
	MP_(variablesPointer);

static StringArray
        MP_(diagMetrics),
	MP_(orderedModelPointers),
	MP_(primaryVarMaps),
        MP_(secondaryVarMaps);

static bool
	MP_(autoRefine),
	MP_(crossValidateFlag),
	MP_(decompDiscontDetect),
	MP_(exportSurrogate),
	MP_(hierarchicalTags),
      //MP_(importApproxActive),
	MP_(importBuildActive),
	MP_(importChallengeActive),
	MP_(modelUseDerivsFlag),
        MP_(domainDecomp),
        MP_(pointSelection),
        MP_(pressFlag),
  MP_(subspaceIdBingLi),
  MP_(subspaceIdConstantine),
  MP_(subspaceIdEnergy),
  MP_(subspaceBuildSurrogate),
  MP_(subspaceIdCV),
  MP_(subspaceCVIncremental);

static unsigned short
	MP_(adaptedBasisSparseGridLev),
	MP_(adaptedBasisExpOrder);

static short
	MP_(annNodes),
	MP_(annRandomWeight),
	MP_(krigingFindNugget),
	MP_(krigingMaxTrials),
	MP_(marsMaxBases),
	MP_(mlsWeightFunction),
	MP_(polynomialOrder),
	MP_(rbfBases),
	MP_(rbfMaxPts),
	MP_(rbfMaxSubsets),
	MP_(rbfMinPartition);

static int
        MP_(decompSupportLayers),
        MP_(initialSamples),
        MP_(maxFunctionEvals),
        MP_(maxIterations),
        MP_(numFolds),
        MP_(numReplicates),
        MP_(pointsTotal),
        MP_(refineCVFolds),
        MP_(softConvergenceLimit),
        MP_(subMethodProcs),
        MP_(subMethodServers),
        MP_(subspaceDimension),
        MP_(subspaceCVMaxRank);

#undef MP2s
#undef MP2
#undef MP_

#define MP_(x) DataResponsesRep::* resp_mp_##x = &DataResponsesRep::x
#define MP2(x,y) resp_mp_##x##_##y = {&DataResponsesRep::x,#y}
#define MP2s(x,y) resp_mp_##x##_##y = {&DataResponsesRep::x,y}

static IntSet
	MP_(idAnalyticGrads),
	MP_(idAnalyticHessians),
	MP_(idNumericalGrads),
	MP_(idNumericalHessians),
	MP_(idQuasiHessians);

static IntVector
	MP_(fieldLengths),
	MP_(numCoordsPerField);

static RealVector
	MP_(expConfigVars),
	MP_(expObservations),
	MP_(primaryRespFnWeights),
	MP_(nonlinearEqTargets),
	MP_(nonlinearIneqLowerBnds),
	MP_(nonlinearIneqUpperBnds),
	MP_(fdGradStepSize),
	MP_(fdHessStepSize),
	MP_(primaryRespFnScales),
	MP_(nonlinearEqScales),
	MP_(nonlinearIneqScales);

static Resp_mp_lit
	MP2(gradientType,analytic),
	MP2(gradientType,mixed),
	MP2(gradientType,none),
	MP2(gradientType,numerical),
	MP2(hessianType,analytic),
	MP2(hessianType,mixed),
	MP2(hessianType,none),
	MP2(hessianType,numerical),
	MP2(hessianType,quasi),
	MP2(intervalType,central),
	MP2(intervalType,forward),
	MP2(methodSource,dakota),
	MP2(methodSource,vendor),
        MP2(fdGradStepType,absolute),
        MP2(fdGradStepType,bounds),
        MP2(fdGradStepType,relative),
        MP2(fdHessStepType,absolute),
        MP2(fdHessStepType,bounds),
        MP2(fdHessStepType,relative),
	MP2(quasiHessianType,bfgs),
	MP2(quasiHessianType,damped_bfgs),
	MP2(quasiHessianType,sr1);

static String
	MP_(scalarDataFileName),
        MP_(idResponses);

static StringArray
	MP_(nonlinearEqScaleTypes),
	MP_(nonlinearIneqScaleTypes),
	MP_(primaryRespFnScaleTypes),
	MP_(primaryRespFnSense),
	MP_(responseLabels),
	MP_(varianceType);

static bool
	MP_(calibrationDataFlag),
	MP_(centralHess),
	MP_(interpolateFlag),
        MP_(ignoreBounds),
        MP_(readFieldCoords);

static size_t
	MP_(numExpConfigVars),
        MP_(numExperiments),
	MP_(numFieldLeastSqTerms),
	MP_(numFieldObjectiveFunctions),
	MP_(numFieldResponseFunctions),
	MP_(numLeastSqTerms),
	MP_(numNonlinearEqConstraints),
	MP_(numNonlinearIneqConstraints),
	MP_(numObjectiveFunctions),
	MP_(numResponseFunctions),
	MP_(numScalarLeastSqTerms),
	MP_(numScalarObjectiveFunctions),
	MP_(numScalarResponseFunctions);

static Resp_mp_utype
        MP2s(scalarDataFormat,TABULAR_NONE),
        MP2s(scalarDataFormat,TABULAR_HEADER),
        MP2s(scalarDataFormat,TABULAR_EVAL_ID),
        MP2s(scalarDataFormat,TABULAR_EXPER_ANNOT);
 
#undef MP2s
#undef MP2
#undef MP_

// Macros for Environment

#define MP_(x) DataEnvironmentRep::* env_mp_##x = &DataEnvironmentRep::x
#define MP2s(x,y) env_mp_##x##_##y = {&DataEnvironmentRep::x,y}

//static Env_mp_lit
//      MP2(,);

static Env_mp_utype
        MP2s(postRunInputFormat,TABULAR_NONE),
        MP2s(postRunInputFormat,TABULAR_HEADER),
        MP2s(postRunInputFormat,TABULAR_EVAL_ID),
        MP2s(postRunInputFormat,TABULAR_IFACE_ID),
        MP2s(postRunInputFormat,TABULAR_ANNOTATED),
        MP2s(preRunOutputFormat,TABULAR_NONE),
        MP2s(preRunOutputFormat,TABULAR_HEADER),
        MP2s(preRunOutputFormat,TABULAR_EVAL_ID),
        MP2s(preRunOutputFormat,TABULAR_IFACE_ID),
        MP2s(preRunOutputFormat,TABULAR_ANNOTATED),
        MP2s(tabularFormat,TABULAR_NONE),
        MP2s(tabularFormat,TABULAR_HEADER),
        MP2s(tabularFormat,TABULAR_EVAL_ID),
        MP2s(tabularFormat,TABULAR_IFACE_ID),
        MP2s(tabularFormat,TABULAR_ANNOTATED);

static String
        MP_(errorFile),
        MP_(outputFile),
        MP_(postRunInput),
        MP_(postRunOutput),
        MP_(preRunInput),
        MP_(preRunOutput),
        MP_(readRestart),
        MP_(resultsOutputFile),
        MP_(runInput),
        MP_(runOutput),
	MP_(tabularDataFile),
        MP_(topMethodPointer),
        MP_(writeRestart);

static bool
	MP_(checkFlag),
	MP_(graphicsFlag),
	MP_(postRunFlag),
	MP_(preRunFlag),
        MP_(resultsOutputFlag),
	MP_(runFlag),
	MP_(tabularDataFlag);

static int
        MP_(outputPrecision),
        MP_(stopRestart);

//#undef MP2
#undef MP2s
#undef MP_

#define MP_(x) DataVariablesRep::* var_mp_##x = &DataVariablesRep::x
#define MP2s(x,y) var_mp_##x = {&DataVariablesRep::x,y}
#define VP_(x) *Var_Info::* var_mp_Var_Info_##x = &Var_Info::x
#define Vtype(x,y) var_mp_##x##_##y = {&DataVariablesRep::x,y}

static size_t
	MP_(numBetaUncVars),
	MP_(numBinomialUncVars),
	MP_(numContinuousDesVars),
	MP_(numContinuousIntervalUncVars),
	MP_(numContinuousStateVars),
	MP_(numDiscreteDesRangeVars),
	MP_(numDiscreteDesSetIntVars),
	MP_(numDiscreteDesSetStrVars),
	MP_(numDiscreteDesSetRealVars),
	MP_(numDiscreteIntervalUncVars),
	MP_(numDiscreteStateRangeVars),
	MP_(numDiscreteStateSetIntVars),
	MP_(numDiscreteStateSetStrVars),
	MP_(numDiscreteStateSetRealVars),
	MP_(numDiscreteUncSetIntVars),
	MP_(numDiscreteUncSetStrVars),
	MP_(numDiscreteUncSetRealVars),
	MP_(numExponentialUncVars),
	MP_(numFrechetUncVars),
	MP_(numGammaUncVars),
	MP_(numGeometricUncVars),
	MP_(numGumbelUncVars),
	MP_(numHistogramBinUncVars),
	MP_(numHistogramPtIntUncVars),
	MP_(numHistogramPtStrUncVars),
	MP_(numHistogramPtRealUncVars),
	MP_(numHyperGeomUncVars),
	MP_(numLognormalUncVars),
	MP_(numLoguniformUncVars),
	MP_(numNegBinomialUncVars),
	MP_(numNormalUncVars),
	MP_(numPoissonUncVars),
	MP_(numTriangularUncVars),
	MP_(numUniformUncVars),
	MP_(numWeibullUncVars);

static IntVector
	VP_(ddsi),
	VP_(DIlb),
	MP_(discreteDesignRangeLowerBnds),
	MP_(discreteDesignRangeUpperBnds),
	MP_(discreteDesignRangeVars),
	MP_(discreteDesignSetIntVars),
	MP_(discreteIntervalUncVars),
        MP_(discreteStateRangeLowerBnds),
	MP_(discreteStateRangeUpperBnds),
	MP_(discreteStateRangeVars),
	MP_(discreteStateSetIntVars),
	MP_(discreteUncSetIntVars),
	VP_(DIub),
        MP_(histogramPointIntUncVars),
        VP_(hpia),
        VP_(dssi),
        VP_(ddsia),
        VP_(ddssa),
        VP_(ddsra),
        VP_(dusi);

static IntArray
	VP_(nddsi),
	VP_(nddss),
	VP_(nddsr),
	VP_(ndssi),
	VP_(ndsss),
	VP_(ndssr),
	VP_(ndusi),
	VP_(nduss),
	VP_(ndusr),
	VP_(nhbp),
	VP_(nhpip),
	VP_(nhpsp),
	VP_(nhprp),
	VP_(nCI),
	VP_(nDI);

static RealVector
	MP_(betaUncLowerBnds),
	MP_(betaUncUpperBnds),
	MP_(betaUncVars),
        MP_(binomialUncProbPerTrial),
        MP_(continuousDesignLowerBnds),
	MP_(continuousDesignUpperBnds),
	MP_(continuousDesignVars),
	MP_(continuousDesignScales),
	MP_(continuousIntervalUncVars),
	MP_(continuousStateLowerBnds),
	MP_(continuousStateUpperBnds),
	MP_(continuousStateVars),
	MP_(discreteDesignSetRealVars),
	MP_(discreteStateSetRealVars),
	MP_(discreteUncSetRealVars),
	MP_(frechetUncBetas),
	MP_(frechetUncVars),
        MP_(geometricUncProbPerTrial),
	MP_(gumbelUncBetas),
	MP_(gumbelUncVars),
	MP_(histogramBinUncVars),
        MP_(histogramPointRealUncVars),
	MP_(linearEqConstraintCoeffs),
	MP_(linearEqScales),
	MP_(linearEqTargets),
	MP_(linearIneqConstraintCoeffs),
	MP_(linearIneqLowerBnds),
	MP_(linearIneqUpperBnds),
	MP_(linearIneqScales),
        MP_(negBinomialUncProbPerTrial),
	MP_(normalUncLowerBnds),
	MP_(normalUncMeans),
	MP_(normalUncUpperBnds),
	MP_(normalUncVars),
	MP_(triangularUncModes),
	MP_(triangularUncVars),
	MP_(uniformUncVars),
	MP_(weibullUncVars),
	VP_(ddsr),
	VP_(dssr),
	VP_(dusr),
	VP_(CIlb),
	VP_(CIub),
	VP_(CIp),
	VP_(DIp),
	VP_(DSIp),
	VP_(DSSp),
	VP_(DSRp),
	VP_(hba),
	VP_(hbo),
	VP_(hbc),
	VP_(hpic),
	VP_(hpsc),
	VP_(hpra),
	VP_(hprc),
	VP_(ucm);

static String
	MP_(idVariables);

static StringArray
	MP_(continuousDesignLabels),
	MP_(continuousDesignScaleTypes),
	MP_(continuousStateLabels),
	MP_(discreteDesignRangeLabels),
	MP_(discreteDesignSetIntLabels),
	MP_(discreteDesignSetStrLabels),
	MP_(discreteDesignSetRealLabels),
	MP_(discreteStateRangeLabels),
	MP_(discreteStateSetIntLabels),
	MP_(discreteStateSetStrLabels),
        MP_(discreteStateSetRealLabels),
	MP_(discreteDesignSetStrVars),
	MP_(discreteUncSetStrVars),
	MP_(discreteStateSetStrVars),
        MP_(histogramPointStrUncVars),
	MP_(linearEqScaleTypes),
	MP_(linearIneqScaleTypes),
        VP_(hpsa),
        VP_(ddss),
        VP_(duss),
        VP_(dsss);

static BitArray
        MP_(discreteDesignSetIntCat),
        MP_(discreteDesignSetRealCat),
        MP_(discreteStateSetIntCat),
        MP_(discreteStateSetRealCat),
        MP_(discreteUncSetIntCat),
        MP_(discreteUncSetRealCat);

static Var_brv
	MP2s(betaUncAlphas,0.),
	MP2s(betaUncBetas,0.),
	MP2s(exponentialUncBetas,0.),
	MP2s(exponentialUncVars,0.),
	MP2s(frechetUncAlphas,2.),
	MP2s(gammaUncAlphas,0.),
	MP2s(gammaUncBetas,0.),
	MP2s(gammaUncVars,0.),
	MP2s(gumbelUncAlphas,0.),
	MP2s(lognormalUncErrFacts,1.),
	MP2s(lognormalUncLambdas,0.),
	MP2s(lognormalUncLowerBnds,0.),
	MP2s(lognormalUncMeans,0.),
	MP2s(lognormalUncStdDevs,0.),
	MP2s(lognormalUncUpperBnds,std::numeric_limits<Real>::infinity()),
	MP2s(lognormalUncVars,0.),
	MP2s(lognormalUncZetas,0.),
	MP2s(loguniformUncLowerBnds,0.),
	MP2s(loguniformUncUpperBnds,std::numeric_limits<Real>::infinity()),
	MP2s(loguniformUncVars,0.),
	MP2s(normalUncStdDevs,0.),
	MP2s(poissonUncLambdas,0.),
	MP2s(triangularUncLowerBnds,-std::numeric_limits<Real>::infinity()),
	MP2s(triangularUncUpperBnds, std::numeric_limits<Real>::infinity()),
	MP2s(uniformUncLowerBnds,-std::numeric_limits<Real>::infinity()),
	MP2s(uniformUncUpperBnds, std::numeric_limits<Real>::infinity()),
	MP2s(weibullUncAlphas,0.),
	MP2s(weibullUncBetas,0.);

static Var_biv
	MP2s(binomialUncNumTrials,0),
	MP2s(binomialUncVars,0),
	MP2s(geometricUncVars,0),
	MP2s(hyperGeomUncNumDrawn,0),
	MP2s(hyperGeomUncSelectedPop,0),
	MP2s(hyperGeomUncTotalPop,0),
	MP2s(hyperGeomUncVars,0),
	MP2s(negBinomialUncNumTrials,0),
	MP2s(negBinomialUncVars,0),
	MP2s(poissonUncVars,0);

static Var_mp_type
	Vtype(varsDomain,MIXED_DOMAIN),
	Vtype(varsDomain,RELAXED_DOMAIN),
	Vtype(varsView,ALL_VIEW),
	Vtype(varsView,DESIGN_VIEW),
	Vtype(varsView,UNCERTAIN_VIEW),
	Vtype(varsView,ALEATORY_UNCERTAIN_VIEW),
	Vtype(varsView,EPISTEMIC_UNCERTAIN_VIEW),
        Vtype(varsView,STATE_VIEW);

#undef Vtype
#undef VP_
#undef MP2s
#undef MP_

} // namespace Dakota

#undef Warn
#undef Squawk

#define N_ifm(x,y)	NIDRProblemDescDB::iface_##x,&iface_mp_##y
#define N_ifm3(x,y,z)	NIDRProblemDescDB::iface_##x,y,NIDRProblemDescDB::iface_##z
#define N_mdm(x,y)	NIDRProblemDescDB::method_##x,&method_mp_##y
//#define N_mdf(x,y)	N_mdm(x,y),NIDRProblemDescDB::method_##x##_final
#define N_mdm3(x,y,z)	NIDRProblemDescDB::method_##x,y,NIDRProblemDescDB::method_##z
#define N_mom(x,y)	NIDRProblemDescDB::model_##x,&model_mp_##y
#define N_mof(x,y)	N_mom(x,y),NIDRProblemDescDB::model_##x##_final
#define N_mom3(x,y,z)	NIDRProblemDescDB::model_##x,y,NIDRProblemDescDB::model_##z
#define N_rem(x,y)	NIDRProblemDescDB::resp_##x,&resp_mp_##y
#define N_rem3(x,y,z)	NIDRProblemDescDB::resp_##x,y,NIDRProblemDescDB::resp_##z
#define N_stm(x,y)	NIDRProblemDescDB::env_##x,&env_mp_##y
#define N_vae(x,y)	NIDRProblemDescDB::var_##x,(void*)y
#define N_vam(x,y)	NIDRProblemDescDB::var_##x,&var_mp_##y
#define N_vam3(x,y,z)	NIDRProblemDescDB::var_##x,y,NIDRProblemDescDB::var_##z

#include "NIDR_keywds.hpp"
