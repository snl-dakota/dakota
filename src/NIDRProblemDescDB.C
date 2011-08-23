/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

#include "NIDRProblemDescDB.H"
#include "ParallelLibrary.H"
#include "filesystem_utils.h"  // for DAK_MKDIR and not_executable(...)
#include "pecos_stat_util.hpp"
#include <functional>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef DAKOTA_DL_SOLVER
#ifdef _WIN32
#include <windows.h>
#define dlopen(x,y) LoadLibrary(x)
#else
#include <dlfcn.h>
#endif
#endif /*DAKOTA_DL_SOLVER*/

extern "C" void nidr_lib_cleanup(void);

char nidr_please_refer[] =
	"Please refer to the DAKOTA Reference and/or User's Manual or the\n"
	"dakota.input.summary file distributed with this executable.";

namespace Dakota {

extern "C" FILE *nidrin;
extern "C" int nidr_parse(const char*, FILE*);

int NIDRProblemDescDB::nerr = 0;
NIDRProblemDescDB* NIDRProblemDescDB::pDDBInstance(NULL);

 void NIDRProblemDescDB::
botch(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	std::fprintf(stderr, "\nError: ");
	std::vfprintf(stderr, fmt, ap);
	std::fputs(".\n", stderr);
	va_end(ap);
	abort_handler(-1);
	}

 void NIDRProblemDescDB::
squawk(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	std::fprintf(stderr, "\nError: ");
	std::vfprintf(stderr, fmt, ap);
	std::fputs(".\n", stderr);
	va_end(ap);
	++nerr;
	}

 void NIDRProblemDescDB::
warn(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	std::fprintf(stderr, "\nWarning: ");
	std::vfprintf(stderr, fmt, ap);
	std::fputs(".\n", stderr);
	va_end(ap);
	}


/** Parse the input file using the Input Deck Reader (IDR) parsing system.
    IDR populates the IDRProblemDescDB object with the input file data. */
void NIDRProblemDescDB::
derived_parse_inputs(const char* dakota_input_file, const char* parser_options)
{
  // set the pDDBInstance
  pDDBInstance = this;

  // Open the dakota input file passed in and "attach" it to stdin
  // (required by nidr_parse)
  if (dakota_input_file) {
    if (dakota_input_file[0] == '-' && !dakota_input_file[1])
      nidrin = stdin;
    else if( !(nidrin = std::fopen(dakota_input_file, "r")) )
      botch("cannot open \"%s\"", dakota_input_file);
  }

  // nidr_parse parses the input file and invokes the keyword handlers
  if (nidr_parse(parser_options,0)) {
    //Cerr << "\nErrors parsing input file." << std::endl;
    abort_handler(-1); // allows proper abort in parallel case
  }
  if (nerr)
    abort_handler(-1);
  if (parallel_library().command_line_run()) {
	const char *s;
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
	if (parallel_library().mpirun_flag()) {
		std::list<DataInterface>::iterator
			Ifl  = dataInterfaceList.begin(),
			Ifle = dataInterfaceList.end();
		DataInterfaceRep *Ir;
		struct stat sb;

		for(; Ifl != Ifle; Ifl++) {
			Ir = Ifl->dataIfaceRep;
			if ((s = Ir->workDir) && *s && stat(s,&sb))
				DAK_MKDIR(s, 0700);
			}
		}
	}
}


void NIDRProblemDescDB::derived_broadcast()
{ check_variables(&dataVariablesList); check_responses(&dataResponsesList); }
// check_variables() invokes var_stop1, either directly or after some manip of
// interval/histogram/correlation data.  var_stop1 does label processing (?),
// followed by additional processing in make_variable_defaults() below, which
// calls BuildLabels().  Need to understand the reason for this step.

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


NIDRProblemDescDB::NIDRProblemDescDB(ParallelLibrary& parallel_lib):
  ProblemDescDB(BaseConstructor(), parallel_lib)
{}


#define Squawk NIDRProblemDescDB::squawk
#define Warn NIDRProblemDescDB::warn

 typedef unsigned long UL;

 struct
Iface_Info {
	DataInterfaceRep *di;
	DataInterface *di0;
	};

 struct
Iface_mp_Rlit {
	String DataInterfaceRep::* sp;
	RealVector DataInterfaceRep::* rv;
	const char *lit;
	};

 struct
Iface_mp_ilit {
	String DataInterfaceRep::* sp;
	int DataInterfaceRep::* iv;
	const char *lit;
	};

 struct
Iface_mp_lit {
	String DataInterfaceRep::* sp;
	const char *lit;
	};

 struct
Meth_Info {
	DataMethodRep *dme;
	DataMethod *dme0;
	};

 struct
Method_mp_ilit2 {
	String DataMethodRep::* sp;
	int DataMethodRep::* ip;
	const char *lit;
	};

 struct
Method_mp_ilit2z {
	String DataMethodRep::* sp;
	size_t DataMethodRep::* ip;
	const char *lit;
	};

 struct
Method_mp_lit {
	String DataMethodRep::* sp;
	const char *lit;
	};

 struct
Method_mp_lit2 {
	String DataMethodRep::* sp;
	String DataMethodRep::* sp2;
	const char *lit;
	const char *lit2;
	};

 struct
Method_mp_litc {
	String DataMethodRep::* sp;
	Real DataMethodRep::* rp;
	const char *lit;
	};

 struct
Method_mp_litrv {
	String DataMethodRep::* sp;
	RealVector DataMethodRep::* rp;
	const char *lit;
	};

 struct
Method_mp_slit2 {
	String DataMethodRep::* sp;
	String DataMethodRep::* sp2;
	const char *lit;
	};

 struct
Method_mp_type {
	short DataMethodRep::* sp;
	short type;
	};

 struct
Mod_Info {
	DataModelRep *dmo;
	DataModel *dmo0;
	};

 struct
Model_mp_lit {
	String DataModelRep::* sp;
	const char *lit;
	};

 struct
Model_mp_ord {
	short DataModelRep::* sp;
	int ord;
	};

 struct
Model_mp_type {
	short DataModelRep::* sp;
	short type;
	};

 struct
Resp_Info {
	DataResponsesRep *dr;
	DataResponses *dr0;
	};

 struct
Resp_mp_lit {
	String DataResponsesRep::* sp;
	const char *lit;
	};

 struct
Strategy_mp_lit {
	String DataStrategyRep::* sp;
	const char *lit;
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
	DAUIVar_Nkinds = 5	// number of kinds of dauiv variables
	};

 enum { // kinds of discrete aleatory uncertain real variables
	DAURVar_histogram_point = 0,
	DAURVar_Nkinds = 1	// number of kinds of daurv variables
	};

 enum { // kinds of continuous epistemic uncertain variables
	CEUVar_interval = 0,
	CEUVar_Nkinds = 1	// number of kinds of cauv variables
	};

// enum { // kinds of discrete epistemic uncertain integer variables
//	DEUIVar_Nkinds = 0	// number of kinds of deuiv variables
//	};

// enum { // kinds of discrete epistemic uncertain real variables
//	DEURVar_Nkinds = 0	// number of kinds of deurv variables
//	};

 enum { // kinds of discrete set variables
	DiscSetVar_design_set_int = 0,
	DiscSetVar_design_set_real = 1,
	DiscSetVar_state_set_int = 2,
	DiscSetVar_state_set_real = 3,
	DiscSetVar_Nkinds = 4	// number of kinds of discrete set variables
	};

 struct
VarLabel {
	size_t n;
	const char **s;
	};

 struct
Var_Info {
	DataVariablesRep *dv;
	DataVariables *dv0;
	VarLabel CAUv[CAUVar_Nkinds], CEUv[CEUVar_Nkinds];
	VarLabel DAUIv[DAUIVar_Nkinds], DAURv[DAURVar_Nkinds];
	// VarLabel, DEUIv[DEUIVar_Nkinds], DEURv[DEURVar_Nkinds];
        IntArray *dsvi, *ndsvi, *ndsvr, *nIv, *nbp, *npp, *nssvi, *nssvr, *ssvi;
        RealVector *dsvr, *Ivb, *Ivp, *ba, *bo, *bc, *pa, *pc, *ucm, *ssvr;
	};

 struct
Var_bchk
{
	const char *name;
	size_t DataVariablesRep::* n;
	void (*vbgen)(DataVariablesRep*, size_t);
	RealVector DataVariablesRep::* L;
	RealVector DataVariablesRep::* U;
	RealVector DataVariablesRep::* V;	// initial guess, possibly null
	StringArray DataVariablesRep::* Lbl;	// possibly null
	int no_init;
	};

 struct
Var_bgen
{
	const char *name;
	size_t DataVariablesRep::* n;
	void (*vbgen)(DataVariablesRep*, size_t);
	};

 struct
VarBgen {
	Var_bgen *bgen;
	size_t nbgen;
	};

 struct
Var_ibchk
{
	const char *name;
	size_t DataVariablesRep::* n;
	IntVector DataVariablesRep::* L;
	IntVector DataVariablesRep::* U;
	IntVector DataVariablesRep::* V;
	StringArray DataVariablesRep::* Lbl;
	};

 struct
Var_uinfo {
	const char *lbl;
	const char *vkind;
	size_t DataVariablesRep::* n;
	void(*vadj)(DataVariablesRep*, size_t, Var_Info*);
	};

 struct
Var_brv {
	RealVector DataVariablesRep::* rv;
	Real b;
	};

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
	if (n <= 0)
		botch("%s must be positive", keyname);
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
	if (!(ii->di0 = new DataInterface))
		goto Botch;
	ii->di = ii->di0->dataIfaceRep;
	*g = (void*)ii;
	}

 void NIDRProblemDescDB::
iface_true(const char *keyname, Values *val, void **g, void *v)
{
	(*(Iface_Info**)g)->di->**(bool DataInterfaceRep::**)v = true;
	}

 void NIDRProblemDescDB::
iface_stop(const char *keyname, Values *val, void **g, void *v)
{
	Iface_Info *ii = *(Iface_Info**)g;
	DataInterfaceRep *di = ii->di;
	StringArray *sa;
	const char *s;
	int ac, ec, j, nd, same;
	void (*Complain)(const char *fmt, ...);
	size_t i, n;
	extern ProblemDescDB *Dak_pddb;
	static const char samefmt[] =
		"template_directory and work_directory name are both \"%s\"";
	static const char samefmt2[] =
		"work_directory template file %d are both \"%s\"";
	struct stat sb1;
#ifndef _WIN32
	struct stat sb2;
#endif

	nd = di->analysisDrivers.size();
	ac = di->asynchLocalAnalysisConcurrency;
	ec = di->asynchLocalEvalConcurrency;

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
		di->interfaceSynchronization = "synchronous";
		}
	if ((di->interfaceType == "system" || di->interfaceType == "fork")
	 && (Complain = Dak_pddb->parallel_library().command_line_check() ? warn
			: Dak_pddb->parallel_library().command_line_run() ? squawk : 0)) {
		sa = &di->analysisDrivers;
		n = sa->size();
 		for(i = 0; i < n; ++i)
 			if ((j = not_executable(s = (*sa)[i], di->templateDir)))
 				Complain("analysis driver \"%s\" %s", s,
 					j == 1	? "not found"
 						: "exists but is not executable");
		}
	same = 0;
	if (di->workDir.length() > 0) {
		if (di->templateDir.length() > 0) {
			same = di->templateDir == di->workDir;
#ifndef _WIN32
			if (!same
			 && !stat(di->workDir.data(), &sb1)
			 && !stat(di->templateDir.data(), &sb2))
				same = sb1.st_dev == sb2.st_dev
					&& sb1.st_rdev == sb2.st_rdev
					&& sb1.st_ino == sb2.st_ino;
#endif
			if (same)
				squawk(samefmt, di->workDir.data());
			}
		else if ((n = di->templateFiles.size() > 0)
				&& !stat(di->workDir.data(), &sb1)) {
			for(i = 0; i < n; ++i) {
				if (di->templateFiles[i] == di->workDir
#ifndef _WIN32
				 || (!stat(di->templateFiles[i].data(), &sb2)
				 && sb1.st_dev == sb2.st_dev
				 && sb1.st_rdev == sb2.st_rdev
				 && sb1.st_ino == sb2.st_ino)
#endif
					) {
					squawk(samefmt2, (int)(i+1), di->workDir.data());
					break;
					}
				}
			}
		}
	pDDBInstance->dataInterfaceList.push_back(*ii->di0);
	delete ii->di0;
	delete ii;
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

	// This is just for analysisComponents -- the only String2DArray in a DataInterfaceRep.

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
	StringArray *sa = &((*(Iface_Info**)g)->di->**(StringArray DataInterfaceRep::**)v);
	const char **s = val->s;
	size_t i, n = val->n;

	sa->resize(n);
	for(i = 0; i < n; i++)
		(*sa)[i] = s[i];
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

// MSE: This COLINY function mirrors method-dependent default assignments
// from old IDRProblemDescDB.C.  Could be moved to COLINOptimizer ctor for
// greater consistency with, e.g., maxIterations method-dependent default.
 void NIDRProblemDescDB::
method_coliny_ea(const char *keyname, Values *val, void **g, void *v)
{
	DataMethodRep *dm = (*(Meth_Info**)g)->dme;
	dm->methodName = "coliny_ea";
	dm->fitnessType = "linear_rank";
	dm->replacementType = "elitist";
	dm->numberRetained = 1;
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
method_intDL(const char *keyname, Values *val, void **g, void *v)
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
method_lit2(const char *keyname, Values *val, void **g, void *v)
{
	DataMethodRep *dm = (*(Meth_Info**)g)->dme;
	dm->*((Method_mp_lit2*)v)->sp  = ((Method_mp_lit2*)v)->lit;
	dm->*((Method_mp_lit2*)v)->sp2 = ((Method_mp_lit2*)v)->lit2;
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
method_litpp(const char *keyname, Values *val, void **g, void *v)
{
	DataMethodRep *dm = (*(Meth_Info**)g)->dme;
	dm->*((Method_mp_lit*)v)->sp = ((Method_mp_lit*)v)->lit;
	dm->stepLenToBoundary = dm->centeringParam = -1.;
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

// MSE: This OPT++ function mirrors method-dependent default assignments
// from old IDRProblemDescDB.C.  Could be moved to SNLLOptimizer ctor for
// greater consistency with, e.g., maxIterations method-dependent default.
 void NIDRProblemDescDB::
method_litpp_final(const char *keyname, Values *val, void **g, void *v)
{
	DataMethodRep *dm = (*(Meth_Info**)g)->dme;

	if (dm->stepLenToBoundary == -1.) {
		if (dm->meritFn == "el_bakry")
			dm->stepLenToBoundary = 0.8;
		else if (dm->meritFn == "argaez_tapia")
			dm->stepLenToBoundary = 0.99995;
		else if (dm->meritFn == "van_shanno")
			dm->stepLenToBoundary = 0.95;
		else
 Botch:			botch("Unexpected meritFn in method_litpp_final");
		}
	if (dm->centeringParam == -1.) {
		if (dm->meritFn == "el_bakry")
			dm->centeringParam = 0.2;
		else if (dm->meritFn == "argaez_tapia")
			dm->centeringParam = 0.2;
		else if (dm->meritFn == "van_shanno")
			dm->centeringParam = 0.1;
		else
			goto Botch;
		}
	}

 void NIDRProblemDescDB::
method_meritFn(const char *keyname, Values *val, void **g, void *v)
{
	const char *s;

	(*(Meth_Info**)g)->dme->**(String DataMethodRep::**)v = s = *val->s;
	if (strcmp(s,"el_bakry")
	 && strcmp(s,"argaez_tapia")
	 && strcmp(s,"van_shanno"))
		botch("merit_function %s not supported", s);
	}

// MSE: These moga functions mirror method-dependent default assignments
// from old IDRProblemDescDB.C.  Could be moved to JEGAOptimizer ctor for
// greater consistency with, e.g., maxIterations method-dependent default.
 void NIDRProblemDescDB::
method_moga_begin(const char *keyname, Values *val, void **g, void *v)
{
	DataMethodRep *dm = (*(Meth_Info**)g)->dme;
	dm->methodName = "moga";
	dm->randomSeed = -1;	// MOGA should "randomly" choose a seed.
	dm->crossoverRate = 0.75;
	dm->mutationType = "";
	}

 void NIDRProblemDescDB::
method_moga_final(const char *keyname, Values *val, void **g, void *v)
{
	DataMethodRep *dm = (*(Meth_Info**)g)->dme;

	if (dm->mutationType == "") {
		dm->mutationType = "replace_uniform";
		dm->mutationRate = 0.1;
		}
	}

 void NIDRProblemDescDB::
method_nnint(const char *keyname, Values *val, void **g, void *v)
{
	int n = *val->i;
	if (n < 0)
		botch("%s must be non-negative", keyname);
	(*(Meth_Info**)g)->dme->**(int DataMethodRep::**)v = n;
	}

 void NIDRProblemDescDB::
method_nnintz(const char *keyname, Values *val, void **g, void *v)
{
	int n = *val->i;
#if 0
	if (n < 0)
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
	if (n <= 0)
		botch("%s must be positive", keyname);
	(*(Meth_Info**)g)->dme->**(int DataMethodRep::**)v = n;
	}

 void NIDRProblemDescDB::
method_pintz(const char *keyname, Values *val, void **g, void *v)
{
	int n = *val->i;
#if 0
	if (n <= 0)
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
method_ushintL(const char *keyname, Values *val, void **g, void *v)
{
	UShortArray *usa
	  = &((*(Meth_Info**)g)->dme->**(UShortArray DataMethodRep::**)v);
	int *z = val->i;
	size_t i, n = val->n;

	usa->resize(n);
	for(i = 0; i < n; i++)
		(*usa)[i] = z[i];
	}

 void NIDRProblemDescDB::
method_slit2(const char *keyname, Values *val, void **g, void *v)
{
	DataMethodRep *dm = (*(Meth_Info**)g)->dme;

	dm->*((Method_mp_slit2*)v)->sp  = ((Method_mp_slit2*)v)->lit;
	dm->*((Method_mp_slit2*)v)->sp2 = *val->s;
	}

// MSE: These soga functions mirror method-dependent default assignments
// from old IDRProblemDescDB.C.  Could be moved to JEGAOptimizer ctor for
// greater consistency with, e.g., maxIterations method-dependent default.
 void NIDRProblemDescDB::
method_soga_begin(const char *keyname, Values *val, void **g, void *v)
{
	DataMethodRep *dm = (*(Meth_Info**)g)->dme;
	dm->methodName = "soga";
	dm->constraintPenalty = 1.;
	dm->crossoverType = "multi_point_parameterized_binary";
	dm->fitnessType = "";
	dm->mutationType = "";
	dm->randomSeed = -1;	// SOGA should "randomly" choose a seed.
	}

 void NIDRProblemDescDB::
method_soga_final(const char *keyname, Values *val, void **g, void *v)
{
	DataMethodRep *dm = (*(Meth_Info**)g)->dme;

	if (dm->fitnessType == "") {
		dm->fitnessType = "merit_function";
		dm->constraintPenalty = 1.0;
		}
	if (dm->mutationType == "") {
		dm->mutationType == "replace_uniform";
		dm->mutationRate = 0.1;
		}
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
		NIDRProblemDescDB::squawk("\"%s\" cannot appear in %s_scale_types", s, what);
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
	static const char *mr_scaletypes[] = { "auto", "none", 0 };
	Meth_Info *mi = *(Meth_Info**)g;
	DataMethodRep *dm = mi->dme;
	scale_chk(dm->linearIneqScaleTypes, dm->linearIneqScales,
		"linear_inequality", mr_scaletypes);
	scale_chk(dm->linearEqScaleTypes, dm->linearEqScales,
		"linear_equality", mr_scaletypes);
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
	(*(Meth_Info**)g)->dme->*((Method_mp_type*)v)->sp = ((Method_mp_type*)v)->type;
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
model_int(const char *keyname, Values *val, void **g, void *v)
{
	(*(Mod_Info**)g)->dmo->**(int DataModelRep::**)v = *val->i;
	}

 void NIDRProblemDescDB::
model_intset(const char *keyname, Values *val, void **g, void *v)
{
	IntSet *is = &((*(Mod_Info**)g)->dmo->**(IntSet DataModelRep::**)v);
	int *z = val->i;
	size_t i, n = val->n;

	for(i = 0; i < n; i++)
		is->insert(z[i]-1);
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
model_type(const char *keyname, Values *val, void **g, void *v)
{
	(*(Mod_Info**)g)->dmo->*((Model_mp_type*)v)->sp = ((Model_mp_type*)v)->type;
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
resp_intL(const char *keyname, Values *val, void **g, void *v)
{
	DataResponsesRep *dr = (*(Resp_Info**)g)->dr;
	int *z = val->i;
	IntList *il = &(dr->**(IntList DataResponsesRep::**)v);
	size_t i, n = val->n;

	for(i = 0; i < n; i++)
		il->push_back(z[i]);
	}

 void NIDRProblemDescDB::
resp_lit(const char *keyname, Values *val, void **g, void *v)
{
	(*(Resp_Info**)g)->dr->*((Resp_mp_lit*)v)->sp = ((Resp_mp_lit*)v)->lit;
	}

 void NIDRProblemDescDB::
resp_nnintz(const char *keyname, Values *val, void **g, void *v)
{
	int n = *val->i;
#if 0
	if (n < 0)
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

 static int
flist_check(IntList *L, int n, IntArray *iv, const char *what)
{
	int nbad, j;

	nbad = 0;
	for(ILCIter it = L->begin(), ite = L->end(); it != ite; ++it) {
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
flist_check2(size_t n, IntArray *iv, const char *what)
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
		iv->resize(nf);
		iv->assign(nf, 0);
		if (!(flist_check(&dr->idAnalyticGrads, ni, iv, "id_analytic_gradients")
		    + flist_check(&dr->idNumericalGrads, ni, iv, "id_numerical_gradients")))
			flist_check2(nf, iv, "gradient");
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
			dr->nonlinearIneqLowerBnds = -DBL_MAX;
			}
		if (dr->nonlinearIneqUpperBnds.length() == 0) {
			dr->nonlinearIneqUpperBnds.sizeUninitialized(n);
			dr->nonlinearIneqUpperBnds = 0.;
			// default is a one-sided inequality <= 0.0
			}
		}
	if (dr->hessianType == "mixed") {
		iv = new IntArray;
		iv->resize(nf);
		iv->assign(nf, 0);
		if (!(flist_check(&dr->idAnalyticHessians, ni, iv, "id_analytic_hessians")
		    + flist_check(&dr->idNumericalHessians, ni, iv, "id_numerical_hessians")
		    + flist_check(&dr->idQuasiHessians, ni, iv, "id_quasi_hessians")))
			flist_check2(nf, iv, "Hessian");
		delete iv;
		}
	if (nerr)
		abort_handler(-1);
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
	StringArray *sa = &((*(Resp_Info**)g)->dr->**(StringArray DataResponsesRep::**)v);
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

 void NIDRProblemDescDB::
strategy_Real(const char *keyname, Values *val, void **g, void *v)
{
	(*(DataStrategyRep**)g)->**(Real DataStrategyRep::**)v = *val->r;
	}

 void NIDRProblemDescDB::
strategy_RealL(const char *keyname, Values *val, void **g, void *v)
{
	Real *r = val->r;
	RealVector *rdv = &((*(DataStrategyRep**)g)->**(RealVector DataStrategyRep::**)v);
	size_t i, n = val->n;

	rdv->sizeUninitialized(n);
	for(i = 0; i < n; i++)
		(*rdv)[i] = r[i];
	}

 void NIDRProblemDescDB::
strategy_true(const char *keyname, Values *val, void **g, void *v)
{
	(*(DataStrategyRep**)g)->**(bool DataStrategyRep::**)v = true;
	}

 void NIDRProblemDescDB::
strategy_int(const char *keyname, Values *val, void **g, void *v)
{
	(*(DataStrategyRep**)g)->**(int DataStrategyRep::**)v = *val->i;
	}

 void NIDRProblemDescDB::
strategy_lit(const char *keyname, Values *val, void **g, void *v)
{
	(*(DataStrategyRep**)g)->*((Strategy_mp_lit*)v)->sp = ((Strategy_mp_lit*)v)->lit;
	}

 void NIDRProblemDescDB::
strategy_start(const char *keyname, Values *val, void **g, void *v)
{
	*g = (void*)pDDBInstance->strategySpec.dataStratRep;
	}

 void NIDRProblemDescDB::
strategy_str(const char *keyname, Values *val, void **g, void *v)
{
	(*(DataStrategyRep**)g)->**(String DataStrategyRep::**)v = *val->s;
	}

 void NIDRProblemDescDB::
strategy_strL(const char *keyname, Values *val, void **g, void *v)
{
	StringArray *sa = &((*(DataStrategyRep**)g)->**(StringArray DataStrategyRep::**)v);
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
      if ( data_method.surrBasedLocalTRInitSize <= 0.0 ||
	   data_method.surrBasedLocalTRInitSize >  1.0 )
		botch("initial_size must be in (0,1]");
      if ( data_method.surrBasedLocalTRMinSize <= 0.0 ||
	   data_method.surrBasedLocalTRMinSize >  1.0 ||
	   data_method.surrBasedLocalTRMinSize >
	   data_method.surrBasedLocalTRInitSize )
		botch("minimum_size must be in (0,1]");
      if( data_method.surrBasedLocalTRContractTrigger <= 0.0 ||
	  data_method.surrBasedLocalTRContractTrigger >
	    data_method.surrBasedLocalTRExpandTrigger        ||
	  data_method.surrBasedLocalTRExpandTrigger   >  1.0 )
		botch("expand/contract threshold values must satisfy\n\t"
			"0 < contract_threshold <= expand_threshold < 1");
      if ( data_method.surrBasedLocalTRContract == 1.0 )
		warn("contraction_factor = 1.0 is valid, but should be < 1\n\t"
			"to assure convergence of the surrrogate_based_opt method");
      if ( data_method.surrBasedLocalTRContract <= 0.0 ||
	   data_method.surrBasedLocalTRContract >  1.0 )
		botch("contraction_factor must be in (0,1]");
      if (data_method.surrBasedLocalTRExpand < 1.0)
		botch("expansion_factor must be >= 1");
    }


 void NIDRProblemDescDB::
var_RealLd(const char *keyname, Values *val, void **g, void *v)
{
	Real *r = val->r;
	RealVector *rv = &((*(Var_Info**)g)->dv->**(RealVector DataVariablesRep::**)v);
	size_t i, n = val->n;

	rv->sizeUninitialized(n);
	for(i = 0; i < n; i++)
		(*rv)[i] = r[i];
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
var_vrl(const char *keyname, Values *val, void **g, void *v)
{
	Var_Info *vi = *(Var_Info**)g;
	Real *r;
	RealVector *rv;
	size_t i, n = val->n;

	if (!(rv = new RealVector))
		botch("new failure in var_vrl");
	vi->**(RealVector *Var_Info::**)v = rv;
	rv->sizeUninitialized(n);
	r = val->r;
	for(i = 0; i < n; i++)
		(*rv)[i] = r[i];
	}

 void NIDRProblemDescDB::
var_intDL(const char *keyname, Values *val, void **g, void *v)
{
	IntVector *iv = &((*(Var_Info**)g)->dv->**(IntVector DataVariablesRep::**)v);
	int *z = val->i;
	size_t i, n = val->n;

	iv->sizeUninitialized(n);
	for(i = 0; i < n; i++)
		(*iv)[i] = z[i];
	}

 void NIDRProblemDescDB::
var_intL(const char *keyname, Values *val, void **g, void *v)
{
	IntArray *iv = &((*(Var_Info**)g)->dv->**(IntArray DataVariablesRep::**)v);
	int *z = val->i;
	size_t i, n = val->n;

	iv->resize(n);
	for(i = 0; i < n; i++)
		(*iv)[i] = z[i];
	}

 void NIDRProblemDescDB::
var_intz(const char *keyname, Values *val, void **g, void *v)
{
	int n = *val->i;
#if 0		/* now handled in the .nspec file */
	if (n <= 0)
		botch("%s must be positive", keyname);
#endif
	(*(Var_Info**)g)->dv->**(size_t DataVariablesRep::**)v = n;
	}

 void NIDRProblemDescDB::
var_vil(const char *keyname, Values *val, void **g, void *v)
{
	Var_Info *vi = *(Var_Info**)g;
	IntArray *iv;
	int *z;
	size_t i, n = val->n;

	if (!(iv = new IntArray))
		botch("new failure in var_vil");
	vi->**(IntArray *Var_Info::**)v = iv;
	iv->resize(n);
	z = val->i;
	for(i = 0; i < n; i++)
		(*iv)[i] = z[i];
	}

 void NIDRProblemDescDB::
var_start(const char *keyname, Values *val, void **g, void *v)
{
	Var_Info *vi;

	if (!(vi = new Var_Info))
 Botch:		botch("new failure in var_start");
	memset(vi, 0, sizeof(Var_Info));
	if (!(vi->dv0 = new DataVariables))
		goto Botch;
	vi->dv = vi->dv0->dataVarsRep;
	*g = (void*)vi;
	}

 void NIDRProblemDescDB::
var_true(const char *keyname, Values *val, void **g, void *v)
{
	(*(Var_Info**)g)->dv->**(bool DataVariablesRep::**)v = true;
	}

 static int
wronglen(size_t n, RealVector *V, const char *what)
{
	size_t n1 = V->length();
	if (n != n1) {
		Squawk("Expected %d numbers for %s, but got %d", (int)n, what, (int)n1);
		return 1;
		}
	return 0;
	}

 static int
wronglen2(size_t n, IntVector *V, const char *what)
{
	size_t n1 = V->length();
	if (n != n1) {
		Squawk("Expected %d numbers for %s, but got %d", (int)n, what, (int)n1);
		return 1;
		}
	return 0;
	}

 static void
Vcopyup(RealVector *V, RealVector *M, size_t i, size_t n)
{
	size_t j;
	for(j = 0; j < n; ++i, ++j)
		(*V)[i] = (*M)[j];
	}

 static void
Set_rdv(RealVector *V, double d, size_t n)
{
	size_t i;
	V->sizeUninitialized(n);
	for(i = 0; i < n; ++i)
		(*V)[i] = d;
	}

 static void
Vadj_Normal(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t n;
	RealVector *B, *M, *Sd;

	n = dv->numNormalUncVars;
	if (wronglen(n, M = &dv->normalUncMeans, "nuv_means")
	 || wronglen(n, Sd = &dv->normalUncStdDevs, "nuv_std_deviations"))
		return;
	B = &dv->normalUncLowerBnds;
	if (B->length() && wronglen(n, B, "nuv_lower_bounds"))
		return;
	B = &dv->normalUncUpperBnds;
	if (B->length())
		wronglen(n, B, "nuv_upper_bounds");
	}

 static void
Vbgen_normalUnc(DataVariablesRep *dv, size_t i0)
{
	int bds;
	size_t j, n;
	Real stdev, w;
	RealVector *B, *L, *M, *Sd, *U, *V;

	n  = dv->numNormalUncVars;
	L = &dv->normalUncLowerBnds;
	M  = &dv->normalUncMeans;
	Sd = &dv->normalUncStdDevs;
	V  = &dv->continuousAleatoryUncVars;
	U = &dv->normalUncUpperBnds;
	Vcopyup(V, M, i0, n);
	B = &dv->continuousAleatoryUncLowerBnds;
	if (!L->length()) {
		Set_rdv(L, -DBL_MAX, n);
		for(j = 0; j < n; ++j)
			(*B)[i0+j] = (*M)[j] - 3.*(*Sd)[j];
		bds = 0;
		}
	else {
		Vcopyup(B, L, i0, n);
		bds = 1;
		}
	B = &dv->continuousAleatoryUncUpperBnds;
	if (!U->length()) {
		Set_rdv(U, DBL_MAX, n);
		for(j = 0; j < n; ++j)
			(*B)[i0+j] = (*M)[j] + 3.*(*Sd)[j];
		}
	else {
		Vcopyup(B, U, i0, n);
		bds += 2;
		}

	// now bds is 0, 1, 2, or 3
	// bds == 0 ==> no bounds given and nothing more to do

	switch(bds) {
	  case 1: // only lower bounds given
		for(j = 0; j < n; ++j, ++i0) {
			if ((*V)[i0] <= (*L)[j])
				(*V)[i0] = (*L)[j] + 0.5*(*Sd)[j];
			}
		break;

	  case 2: // only upper bounds given
		for(j = 0; j < n; ++j, ++i0) {
			if ((*V)[i0] >= (*U)[j])
				(*V)[i0] = (*U)[j] - 0.5*(*Sd)[j];
			}
		break;

	  case 3: // both lower and upper bounds given
		for(j = 0; j < n; ++j, ++i0) {
			if ((*V)[i0] <= (*L)[j]) {
				w = (*U)[j] - (*L)[j];
				if ((stdev = (*Sd)[j]) > w)
					stdev = w;
				(*V)[i0] = (*L)[j] + 0.5*stdev;
				}
			else if ((*V)[i0] >= (*U)[j]) {
				w = (*U)[j] - (*L)[j];
				if ((stdev = (*Sd)[j]) > w)
					stdev = w;
				(*V)[i0] = (*U)[j] - 0.5*stdev;
				}
			}
	  }
	}

 static void
Vadj_Lognormal(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t n;
	RealVector *B, *L, *M, *Sd, *Z;

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
		if (Sd->length() && wronglen(n, Sd, "lnuv_std_deviations"))
			return;
		}
	else if (wronglen(n, &dv->lognormalUncErrFacts, "lnuv_error_factors"))
		return;

	// lower bounds
	B = &dv->lognormalUncLowerBnds;
	if (B->length() && wronglen(n, B, "lnuv_lower_bounds"))
		return;

	// upper bounds
	B = &dv->lognormalUncUpperBnds;
	if (B->length())
		wronglen(n, B, "lnuv_upper_bounds");
	}

 static void
Vbgen_lognormalUnc(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n, num_Sd, num_L;
	Real mean, stdev, t;
	RealVector *B, *Ef, *Lam, *L, *M, *Sd, *U, *V, *Z;

	// lambda/zeta, mean/std_deviation, or mean/error_factor
	n = dv->numLognormalUncVars;
	Lam = &dv->lognormalUncLambdas;
	M = &dv->lognormalUncMeans;
	V = &dv->continuousAleatoryUncVars;
	num_Sd = 0;
	Sd = &dv->lognormalUncStdDevs;
	if (!(num_L = Lam->length()) && M->length()) {
		num_Sd = Sd->length();
		Vcopyup(V, M, i0, n);
		}

	// lower bounds
	L = &dv->lognormalUncLowerBnds;
	if (!L->length())
		L->size(n); // sets L to all zeros
	Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, i0, n);

	// upper bounds
	U = &dv->lognormalUncUpperBnds;
	B = &dv->continuousAleatoryUncUpperBnds;
	Ef = &dv->lognormalUncErrFacts;
	Z = &dv->lognormalUncZetas;
	if (!U->length()) {
		Set_rdv(U, DBL_MAX, n);
		if (num_L) {
			for(i = i0, j = 0; j < n; ++i, ++j) {
				Pecos::moments_from_lognormal_params((*Lam)[j], (*Z)[j], mean, stdev);
				(*B)[i] = mean + 3.*stdev;
				(*V)[i] = mean;
				}
			}
		else if (num_Sd)
			for(i = i0, j = 0; j < n; ++i, ++j) {
				(*V)[i] = mean = (*M)[j];
				(*B)[i] = mean + 3.*(*Sd)[j];
				}
		else {
			for(i = i0, j = 0; j < n; ++i, ++j) {
				(*V)[i] = mean = (*M)[j];
				Pecos::lognormal_std_deviation_from_err_factor(mean, (*Ef)[i], stdev);
				(*B)[i] = mean + 3.*stdev;
				}
			}
		}
	else {
		Vcopyup(B, U, i0, n);
		for(i = i0, j = 0; j < n; ++i, ++j) {
			if (num_L)
				Pecos::moments_from_lognormal_params((*Lam)[j], (*Z)[j], mean, stdev);
			else {
				mean = (*M)[j];
				if (num_Sd)
					stdev = (*Sd)[j];
				else
					Pecos::lognormal_std_deviation_from_err_factor(mean,
						(*Ef)[j], stdev);
				}
			if (mean <= (*L)[j]) {
				mean = (*L)[j];
				if (stdev > (t = (*U)[j] - mean))
					stdev = t;
				mean += 0.5 * stdev;
				}
			else if (mean >= (*U)[j]) {
				mean= (*U)[j];
				if (stdev > (t = mean - (*L)[j]))
					stdev = t;
				mean -= 0.5 * stdev;
				}
			(*V)[i] = mean;
			}
		}
	}

 static void
Vadj_Uniform(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t n;
	RealVector *L, *U;

	n = dv->numUniformUncVars;
	L = &dv->uniformUncLowerBnds;
	U = &dv->uniformUncUpperBnds;
	if (wronglen(n, L, "uuv_lower_bounds"))
		return;
	wronglen(n, U, "uuv_upper_bounds");
	}

 static void
Vbgen_uniformUnc(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n;
	Real stdev;
	RealVector *L, *U, *V;

	n = dv->numUniformUncVars;
	L = &dv->uniformUncLowerBnds;
	U = &dv->uniformUncUpperBnds;
	Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, i0, n);
	Vcopyup(&dv->continuousAleatoryUncUpperBnds, U, i0, n);
	V = &dv->continuousAleatoryUncVars;
	for(i = i0, j = 0; j < n; ++i, ++j)
		Pecos::moments_from_uniform_params((*L)[j], (*U)[j],
						   (*V)[i], stdev);
	}

 static void
Vadj_Loguniform(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t j, n;
	Real Lj, Uj;
	RealVector *L, *U;

	n = dv->numLoguniformUncVars;
	L = &dv->loguniformUncLowerBnds;
	U = &dv->loguniformUncUpperBnds;
	if (wronglen(n, L, "luuv_lower_bounds")
	 || wronglen(n, U, "luuv_upper_bounds"))
		return;
	for(j = 0; j < n; ++j) {
		Lj = (*L)[j];
		Uj = (*U)[j];
		if (Lj <= 0. || Uj <= 0.) {
			Squawk("loguniform bounds must be positive");
			return;
			}
		if (Lj >= DBL_MAX || Uj >= DBL_MAX) {
			Squawk("loguniform bounds must be finite");
			return;
			}
		if (Lj > Uj) {
			Squawk("loguniform lower bound greater than upper bound");
			return;
			}
		}
	}

 static void
Vbgen_loguniformUnc(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n;
	Real stdev;
	RealVector *L, *U, *V;

	n = dv->numLoguniformUncVars;
	L = &dv->loguniformUncLowerBnds;
	U = &dv->loguniformUncUpperBnds;
	Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, i0, n);
	Vcopyup(&dv->continuousAleatoryUncUpperBnds, U, i0, n);
	V = &dv->continuousAleatoryUncVars;
	for(i = i0, j = 0; j < n; ++i, ++j)
		Pecos::moments_from_loguniform_params((*L)[j], (*U)[j], (*V)[i],
						      stdev);
	}

 static void
Vadj_Triangular(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t j, n;
	Real Lj, Mj, Uj;
	RealVector *L, *M, *U;

	n = dv->numTriangularUncVars;
	L = &dv->triangularUncLowerBnds;
	M = &dv->triangularUncModes;
	U = &dv->triangularUncUpperBnds;
	if (wronglen(n, L, "tuv_lower_bounds")
	 || wronglen(n, M, "tuv_modes")
	 || wronglen(n, U, "tuv_upper_bounds"))
		return;
	for(j = 0; j < n; ++j) {
		Lj = (*L)[j];
		Mj = (*M)[j];
		Uj = (*U)[j];
		if (Lj > Mj || Mj > Uj) {
			Squawk("triangular uncertain variables must have\n\t"
				"tuv_lower_bounds <= tuv_modes <= tuv_upper_bounds");
			return;
			}
		}
	}

 static void
Vbgen_triangularUnc(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n;
	Real stdev;
	RealVector *L, *M, *U, *V;

	n = dv->numTriangularUncVars;
	L = &dv->triangularUncLowerBnds;
	M = &dv->triangularUncModes;
	U = &dv->triangularUncUpperBnds;
	Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, i0, n);
	Vcopyup(&dv->continuousAleatoryUncUpperBnds, U, i0, n);
	V = &dv->continuousAleatoryUncVars;
	for(i = i0, j = 0; j < n; ++i, ++j)
		Pecos::moments_from_triangular_params((*L)[j], (*U)[j], (*M)[j],
						      (*V)[i], stdev);
	}

 static void
Vadj_Exponential(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t n;
	RealVector *B;

	n = dv->numExponentialUncVars;
	B = &dv->exponentialUncBetas;
	if (wronglen(n, B, "euv_betas"))
		return;
	}

 static void
Vbgen_Exponential(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n;
	Real mean, stdev;
	RealVector *B, *L, *U, *V;

	n = dv->numExponentialUncVars;
	B = &dv->exponentialUncBetas;
	L = &dv->continuousAleatoryUncLowerBnds;
	U = &dv->continuousAleatoryUncUpperBnds;
	V = &dv->continuousAleatoryUncVars;
	for(i = i0, j = 0; j < n; ++i, ++j) {
		Pecos::moments_from_exponential_params((*B)[j], mean, stdev);
		(*L)[i] = 0.;
		(*U)[i] = mean + 3.*stdev;
		(*V)[i] = mean;
		}
	}

 static void
Vadj_Beta(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t n;
	RealVector *A, *B, *L, *U;

	n = dv->numBetaUncVars;
	A = &dv->betaUncAlphas;
	B = &dv->betaUncBetas;
	L = &dv->betaUncLowerBnds;
	U = &dv->betaUncUpperBnds;
	if (wronglen(n, A, "buv_alphas")
	 || wronglen(n, B, "buv_betas")
	 || wronglen(n, L, "buv_lower_bounds")
	 || wronglen(n, U, "buv_upper_bounds"))
		return;
	}

 static void
Vbgen_betaUnc(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n;
	Real stdev;
	RealVector *A, *B, *L, *U, *V;

	n = dv->numBetaUncVars;
	A = &dv->betaUncAlphas;
	B = &dv->betaUncBetas;
	L = &dv->betaUncLowerBnds;
	U = &dv->betaUncUpperBnds;
	V = &dv->continuousAleatoryUncVars;
	Vcopyup(&dv->continuousAleatoryUncLowerBnds, L, i0, n);
	Vcopyup(&dv->continuousAleatoryUncUpperBnds, U, i0, n);
	for(i = i0, j = 0; j < n; ++i, ++j)
		Pecos::moments_from_beta_params((*L)[j], (*U)[j], (*A)[j],
						(*B)[j], (*V)[i], stdev);
	}

 static void
Vadj_Gamma(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t n;
	RealVector *A, *B;

	n = dv->numGammaUncVars;
	A = &dv->gammaUncAlphas;
	B = &dv->gammaUncBetas;
	if (wronglen(n, A, "gauv_alphas")
	 || wronglen(n, B, "gauv_betas"))
		return;
	}

 static void
Vbgen_Gamma(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n;
	Real mean, stdev;
	RealVector *A, *B, *L, *U, *V;

	n = dv->numGammaUncVars;
	A = &dv->gammaUncAlphas;
	B = &dv->gammaUncBetas;
	L = &dv->continuousAleatoryUncLowerBnds;
	U = &dv->continuousAleatoryUncUpperBnds;
	V = &dv->continuousAleatoryUncVars;
	for(i = i0, j = 0; j < n; ++i, ++j) {
		Pecos::moments_from_gamma_params((*A)[j], (*B)[j], mean, stdev);
		(*L)[i] = 0.;
		(*U)[i] = mean + 3.*stdev;
		(*V)[i] = mean;
		}
	}

 static void
Vadj_Gumbel(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t n;
	RealVector *A, *B;

	n = dv->numGumbelUncVars;
	A = &dv->gumbelUncAlphas;
	B = &dv->gumbelUncBetas;
	if (wronglen(n, A, "guuv_alphas")
	 || wronglen(n, B, "guuv_betas"))
		return;
	}

 static void
Vbgen_Gumbel(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n;
	Real mean, stdev;
	RealVector *A, *B, *L, *U, *V;

	n = dv->numGumbelUncVars;
	A = &dv->gumbelUncAlphas;
	B = &dv->gumbelUncBetas;
	L = &dv->continuousAleatoryUncLowerBnds;
	U = &dv->continuousAleatoryUncUpperBnds;
	V = &dv->continuousAleatoryUncVars;
	for(i = i0, j = 0; j < n; ++i, ++j) {
		Pecos::moments_from_gumbel_params((*A)[j], (*B)[j],
						  mean, stdev);
		(*L)[i] = mean - 3.*stdev;
		(*U)[i] = mean + 3.*stdev;
		(*V)[i] = mean;
		}
	}

 static void
Vadj_Frechet(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t n;
	RealVector *A, *B;

	n = dv->numFrechetUncVars;
	A = &dv->frechetUncAlphas;
	B = &dv->frechetUncBetas;
	if (wronglen(n, A, "fuv_alphas")
	 || wronglen(n, B, "fuv_betas"))
		return;
	}

 static void
Vbgen_Frechet(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n;
	Real mean, stdev;
	RealVector *A, *B, *L, *U, *V;

	n = dv->numFrechetUncVars;
	A = &dv->frechetUncAlphas;
	B = &dv->frechetUncBetas;
	L = &dv->continuousAleatoryUncLowerBnds;
	U = &dv->continuousAleatoryUncUpperBnds;
	V = &dv->continuousAleatoryUncVars;
	for(i = i0, j = 0; j < n; ++i, ++j) {
		Pecos::moments_from_frechet_params((*A)[j], (*B)[j],
						   mean, stdev);
		(*L)[i] = 0.;
		(*U)[i] = mean + 3.*stdev;
		(*V)[i] = mean;
		}
	}

 static void
Vadj_Weibull(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
	size_t n;
	RealVector *A, *B;

	n = dv->numWeibullUncVars;
	A = &dv->weibullUncAlphas;
	B = &dv->weibullUncBetas;
	if (wronglen(n, A, "wuv_alphas")
	 || wronglen(n, B, "wuv_betas"))
		return;
	}

 static void
Vbgen_Weibull(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, n;
	Real mean, stdev;
	RealVector *A, *B, *L, *U, *V;

	n = dv->numWeibullUncVars;
	A = &dv->weibullUncAlphas;
	B = &dv->weibullUncBetas;
	L = &dv->continuousAleatoryUncLowerBnds;
	U = &dv->continuousAleatoryUncUpperBnds;
	V = &dv->continuousAleatoryUncVars;
	for(i = i0, j = 0; j < n; ++i, ++j) {
		Pecos::moments_from_weibull_params((*A)[j], (*B)[j],
						   mean, stdev);
		(*L)[i] = 0.;
		(*U)[i] = mean + 3.*stdev;
		(*V)[i] = mean;
		}
	}

 static void
Vadj_HistogramBin(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
  IntArray *nbp;
  RealVector *ba, *bo, *bc, *bpi;
  RealVectorArray *hbp;
  int nbpi, avg_nbpi;
  size_t i, j, num_a, num_o, num_c, m, n, totbp, cntr;
  Real x, y, bin_width, count_sum;

  if (ba = vi->ba) { // abscissas are required
    num_a = ba->length();                         // abscissas
    bo = vi->bo; num_o = (bo) ? bo->length() : 0; // ordinates
    bc = vi->bc; num_c = (bc) ? bc->length() : 0; // counts
    if (num_o && num_o != num_a) {
      Squawk("Expected %d ordinates, not %d", num_a, num_o);
      return;
    }
    if (num_c && num_c != num_a) {
      Squawk("Expected %d counts, not %d", num_a, num_c);
      return;
    }
    bool key;
    if (nbp = vi->nbp) {
      key = true;
      m = nbp->size();
      //dv->numHistogramBinUncVars = m;
      for(i=totbp=0; i<m; ++i) {
	totbp += nbpi = (*nbp)[i];
	if (nbpi < 2) {
	  Squawk("num_pairs must be >= 2");
	  return;
	}
      }
      if (num_a != totbp) {
	Squawk("Expected %d abscissas, not %d", totbp, num_a);
	return;
      }
    }
    else {
      key = false;
      m = dv->numHistogramBinUncVars;
      if (num_a % m) {
	Squawk("Number of abscissas (%d) not evenly divisible by number of variables (%d); Use num_pairs for unequal apportionment", num_a, m);
	return;
      }
      else
	avg_nbpi = num_a / m;
    }
    hbp = &dv->histogramUncBinPairs;
    hbp->resize(m);
    for (i=cntr=0; i<m; ++i) {
      nbpi = (key) ? (*nbp)[i] : avg_nbpi;
      bpi  = &((*hbp)[i]);
      bpi->sizeUninitialized(2*nbpi);
      count_sum = 0.;
      for (j=0; j<nbpi; ++j, ++cntr) {
	Real x = (*ba)[cntr];                         // abscissas
	Real y = (num_o) ? (*bo)[cntr] : (*bc)[cntr]; // ordinates/counts
	if (j<nbpi-1) {
	  Real bin_width = (*ba)[cntr+1] - x;
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
	(*bpi)[2*j]   = x; // abscissa
	(*bpi)[2*j+1] = y; // count
      }
      // normalize counts to sum to 1
      for (j=0; j<nbpi-1; ++j)
	(*bpi)[2*j+1] /= count_sum;
    }
  }
}

 static void
Vadj_Poisson(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
  size_t n;
  RealVector *A;

  n = dv->numPoissonUncVars;
  A = &dv->poissonUncLambdas;
  if (wronglen(n, A, "lambdas"))
    return;
}

 static void
Vadj_Binomial(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
  size_t n;
  RealVector *A;
  IntVector  *B;

  n = dv->numBinomialUncVars;
  A = &dv->binomialUncProbPerTrial;
  B = &dv->binomialUncNumTrials;
  if (wronglen(n,  A, "prob_per_trial") ||
      wronglen2(n, B, "num_trials"))
    return;
}

 static void
Vadj_NegBinomial(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
  size_t n;
  RealVector *A;
  IntVector  *B;

  n = dv->numNegBinomialUncVars;
  A = &dv->negBinomialUncProbPerTrial;
  B = &dv->negBinomialUncNumTrials;
  if (wronglen(n,  A, "prob_per_trial") ||
      wronglen2(n, B, "num_trials"))
    return;
}

 static void
Vadj_Geometric(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
  size_t n;
  RealVector *A;

  n = dv->numGeometricUncVars;
  A = &dv->geometricUncProbPerTrial;
  if (wronglen(n, A, "prob_per_trial"))
    return;
}

 static void
Vadj_HyperGeom(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
  size_t n;
  IntVector *A, *B, *C;

  n = dv->numHyperGeomUncVars;
  A = &dv->hyperGeomUncTotalPop;
  B = &dv->hyperGeomUncSelectedPop;
  C = &dv->hyperGeomUncNumDrawn;
  if (wronglen2(n, A, "total_population") ||
      wronglen2(n, B, "selected_population") ||
      wronglen2(n, C, "num_drawn"))
    return;
}

 static void
Vadj_HistogramPt(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
  IntArray *npp;
  RealVector *pa, *pc, *ppi;
  RealVectorArray *hpp;
  int nppi, avg_nppi;
  size_t i, j, num_a, num_c, m, n, totpp, cntr;
  Real x, y, bin_width, count_sum;

  if (pa = vi->pa) {
    num_a = pa->length();              // abscissas
    pc = vi->pc; num_c = pc->length(); // counts
    if (num_c != num_a) {
      Squawk("Expected %d point counts, not %d", num_a, num_c);
      return;
    }
    bool key;
    if (npp = vi->npp) {
      key = true;
      m = npp->size();
      //dv->numHistogramPtUncVars = m;
      for(i=totpp=0; i<m; ++i) {
	totpp += nppi = (*npp)[i];
	if (nppi < 1) {
	  Squawk("num_pairs must be >= 1");
	  return;
	}
      }
      if (num_a != totpp) {
	Squawk("Expected %d point abscissas, not %d", totpp, num_a);
	return;
      }
    }
    else {
      key = false;
      m = dv->numHistogramPtUncVars;
      if (num_a % m) {
	Squawk("Number of abscissas (%d) not evenly divisible by number of variables (%d); Use num_pairs for unequal apportionment", num_a, m);
	return;
      }
      else
	avg_nppi = num_a / m;
    }
    hpp = &dv->histogramUncPointPairs;
    hpp->resize(m);
    for (i=cntr=0; i<m; ++i) {
      nppi = (key) ? (*npp)[i] : avg_nppi;
      ppi  = &((*hpp)[i]);
      ppi->sizeUninitialized(2*nppi);
      count_sum = 0.;
      for (j=0; j<nppi; ++j, ++cntr) {
	Real x = (*pa)[cntr]; // abscissas
	Real y = (*pc)[cntr]; // counts
	if (j<nppi-1 && x >= (*pa)[cntr+1]) {
	  Squawk("histogram point x values must increase");
	  return;
	}
	if (y <= 0.) {
	  Squawk("nonpositive intermediate histogram point y value");
	  return;
	}
	(*ppi)[2*j]   = x; // abscissa
	(*ppi)[2*j+1] = y; // count
	count_sum += y;
      }
      // normalize counts to sum to 1
      for (j=0; j<nppi; ++j)
	(*ppi)[2*j+1] /= count_sum;
    }
  }
}

 static void
Vbgen_HistogramBin(DataVariablesRep *dv, size_t i0)
{
	RealVector *L, *U, *V, *r;
	RealVectorArray *A;
	size_t i, m;
	Real mean, stdev;

	L = &dv->continuousAleatoryUncLowerBnds;
	U = &dv->continuousAleatoryUncUpperBnds;
	V = &dv->continuousAleatoryUncVars;
	A = &dv->histogramUncBinPairs;
	if ((m = A->size())) {
		for(i = 0; i < m; i++) {
			r = &((*A)[i]);
			(*L)[i0] = (*r)[0];
			(*U)[i0] = (*r)[r->length() - 2];
			Pecos::moments_from_histogram_bin_params(*r, (*V)[i0++], stdev);
			}
		}
	}

 static void
Vbgen_HistogramPt(DataVariablesRep *dv, size_t i0)
{
	RealVector *L, *U, *V, *r;
	RealVectorArray *A;
	size_t i, j, je, m;
	Real mean, stdev;

	L = &dv->discreteRealAleatoryUncLowerBnds;
	U = &dv->discreteRealAleatoryUncUpperBnds;
	V = &dv->discreteRealAleatoryUncVars;
	A = &dv->histogramUncPointPairs;
	if ((m = A->size())) {
		for(i = 0; i < m; ++i) {
			r = &((*A)[i]);
			je = r->length() - 2;
			(*L)[i0] = (*r)[0];
			(*U)[i0] = (*r)[je];
			Pecos::moments_from_histogram_pt_params(*r, mean, stdev);
			for(j = 0; j < je && (*r)[j+2] <= mean; j += 2);
			if (j < je && mean - (*r)[j] > (*r)[j+2] - mean)
				j += 2;
			(*V)[i0++] = (*r)[j];
			}
		}
	}

 static void
Vadj_Interval(DataVariablesRep *dv, size_t i0, Var_Info *vi)
{
  size_t i, j, j2, k, k2, m, num_p, num_b;
  IntArray *nIv;
  int totni, nii, avg_nii;
  Real lb, lbj, ub, ubj;
  RealVector *Bi, *Ivb, *Ivp, *Pi;
  RealVectorArray *B, *P;

  if ((Ivp = vi->Ivp) && (Ivb = vi->Ivb)) {
    num_p = Ivp->length(); // interval_probs
    num_b = Ivb->length(); // interval_bounds
    if (num_b != 2*num_p) {
      Squawk("Expected twice as many bounds (%d) as probabilities (%d)",
	     num_b, num_p);
      return;
    }
    bool key;
    if (nIv = vi->nIv) {
      key = true;
      m = nIv->size();
      if (m != dv->numIntervalUncVars) {
	Squawk("Expected %d numbers for num_intervals, but got %d",
	       dv->numIntervalUncVars, m);
	return;
      }
      for(i=totni=0; i<m; ++i) {
	totni += nii = (*nIv)[i];
	if (nii < 1) {
	  Squawk("num_intervals values should be positive");
	  return;
	}
      }
      if (wronglen(  totni, Ivp, "interval_probs") ||
	  wronglen(2*totni, Ivb, "interval_bounds"))
	return;
    }
    else {
      key = false;
      m = dv->numIntervalUncVars;
      if (num_p % m) {
	Squawk("Number of probabilities (%d) not evenly divisible by number of variables (%d); Use num_intervals for unequal apportionment", num_p, m);
	return;
      }
      else
	avg_nii = num_p / m;
    }
    B = &dv->intervalUncBounds;
    P = &dv->intervalUncBasicProbs;
    B->resize(m);
    P->resize(m);
    for(i = k = k2 = 0; i < m; ++i) {
      Bi = &((*B)[i]);
      Pi = &((*P)[i]);
      nii = (key) ? (*nIv)[i] : avg_nii;
      Pi->sizeUninitialized(nii);
      Bi->sizeUninitialized(2*nii);
      ub = -(lb = DBL_MAX);
      for(j = j2 = 0; j < nii; ++j, ++k, j2 += 2, k2 += 2) {
	(*Pi)[j]          = (*Ivp)[k];
	(*Bi)[j2]   = lbj = (*Ivb)[k2];
	(*Bi)[j2+1] = ubj = (*Ivb)[k2+1];
	if (lb > lbj)
	  lb = lbj;
	if (ub < ubj)
	  ub = ubj;
      }
      if (lb > ub)
	Squawk("Inconsistent interval uncertain bounds: %g > %g",
	       lb, ub);
    }
  }
}

 static void
Vbgen_Interval(DataVariablesRep *dv, size_t i0)
{
	size_t i, j, j2, k, m, n;
	Real lb, lbj, ub, ubj, stdev;
	RealVector *Bi, *L, *Pi, *U, *V;
	RealVectorArray *B, *P;

	n = dv->numIntervalUncVars;
	L = &dv->continuousEpistemicUncLowerBnds;
	U = &dv->continuousEpistemicUncUpperBnds;
	V = &dv->continuousEpistemicUncVars;
	B = &dv->intervalUncBounds;
	P = &dv->intervalUncBasicProbs;
	for(i = k = 0; i < n; ++i0, ++i) {
		Bi = &((*B)[i]);
		Pi = &((*P)[i]);
		m = Pi->length();
		ub = -(lb = DBL_MAX);
		for(j = j2 = 0; j < m; ++j, ++k, j2 += 2) {
			lbj = (*Bi)[j2];
			ubj = (*Bi)[j2+1];
			if (lb > lbj)
				lb = lbj;
			if (ub < ubj)
				ub = ubj;
			}
		(*L)[i0] = lb;
		(*U)[i0] = ub;
		Pecos::moments_from_uniform_params(lb, ub, (*V)[i0], stdev);
		}
	}

 static void
Vbgen_Poisson(DataVariablesRep *dv, size_t i0)
{
	IntVector *L, *U, *V;
	Real mean, std_dev;
	RealVector *Lam;
	size_t i, n;

	L = &dv->discreteIntAleatoryUncLowerBnds;
	U = &dv->discreteIntAleatoryUncUpperBnds;
	V = &dv->discreteIntAleatoryUncVars;
	Lam = &dv->poissonUncLambdas;
	n = dv->numPoissonUncVars;

	for(i = 0; i < n; ++i, ++i0) {
		Pecos::moments_from_poisson_params((*Lam)[i], mean, std_dev);
		(*L)[i0] = 0;
		(*U)[i0] = (int)std::ceil(mean + 3.*std_dev);
		(*V)[i0] = (int)mean;
		}
	}

 static void
Vbgen_Binomial(DataVariablesRep *dv, size_t i0)
{
	IntVector *L, *NT, *U, *V;
        Real mean, std_dev;
	RealVector *Pr;
	size_t i, n;

	L = &dv->discreteIntAleatoryUncLowerBnds;
	U = &dv->discreteIntAleatoryUncUpperBnds;
	V = &dv->discreteIntAleatoryUncVars;
	NT = &dv->binomialUncNumTrials;
	Pr = &dv->binomialUncProbPerTrial;
	n = dv->numBinomialUncVars;

	for(i = 0; i < n; ++i, ++i0) {
		(*L)[i0] = 0;
		Pecos::moments_from_binomial_params((*Pr)[i], (*NT)[i], mean, std_dev);
                (*V)[i0] = (int)mean;
		(*U)[i0] = (*NT)[i];
		}
	}

 static void
Vbgen_NegBinomial(DataVariablesRep *dv, size_t i0)
{
	IntVector *L, *NT, *U, *V;
	Real mean, std_dev;
	RealVector *Pr;
	size_t i, n;

	L = &dv->discreteIntAleatoryUncLowerBnds;
	U = &dv->discreteIntAleatoryUncUpperBnds;
	V = &dv->discreteIntAleatoryUncVars;
	NT = &dv->negBinomialUncNumTrials;
	Pr = &dv->negBinomialUncProbPerTrial;
	n = dv->numNegBinomialUncVars;

	for(i = 0; i < n; ++i, ++i0) {
		(*L)[i0] = (*NT)[i];
		Pecos::moments_from_negative_binomial_params((*Pr)[i], (*NT)[i], mean, std_dev);
		(*U)[i0] = (int)std::ceil(mean + 3.*std_dev);
		(*V)[i0] = (int)mean;
		}
	}

 static void
Vbgen_Geometric(DataVariablesRep *dv, size_t i0)
{
	IntVector *L, *U, *V;
	Real mean, std_dev;
	RealVector *Pr;
	size_t i, n;

	L = &dv->discreteIntAleatoryUncLowerBnds;
	U = &dv->discreteIntAleatoryUncUpperBnds;
	V = &dv->discreteIntAleatoryUncVars;
	Pr = &dv->geometricUncProbPerTrial;
	n = dv->numGeometricUncVars;

	for(i = 0; i < n; ++i, ++i0) {
		(*L)[i0] = 0;
		Pecos::moments_from_geometric_params((*Pr)[i], mean, std_dev);
		(*U)[i0] = (int)std::ceil(mean + 3.*std_dev);
		(*V)[i0] = (int)mean;
		}
	}

 static void
Vbgen_HyperGeom(DataVariablesRep *dv, size_t i0)
{
	IntVector *L, *ND, *NS, *TP, *U, *V;
        Real mean, std_dev;
	int j,k;
	size_t i, n;

	L = &dv->discreteIntAleatoryUncLowerBnds;
	U = &dv->discreteIntAleatoryUncUpperBnds;
	V = &dv->discreteIntAleatoryUncVars;
	ND = &dv->hyperGeomUncNumDrawn;
	NS = &dv->hyperGeomUncSelectedPop;
	TP = &dv->hyperGeomUncTotalPop;
	n = dv->numHyperGeomUncVars;

	for(i = 0; i < n; ++i, ++i0) {
		(*L)[i0] = 0;
		Pecos::moments_from_hypergeometric_params((*TP)[i], (*NS)[i], (*ND)[i],  mean, std_dev);
                (*V)[i0] = (int)mean;
		j = (*ND)[i];
                k = (*NS)[i];
                if (k < j)
		  j = k;
		(*U)[i0] = j;
		}
	}

 static void
DIset(size_t n, IntSetArray *a, IntVector *L, IntVector *U, IntVector *V)
{
	IntSet *s;
	IntSet::const_iterator ie, it;
	Real t, t1;
	int i, k, km, kx;
	size_t m;

	L->sizeUninitialized(n);
	U->sizeUninitialized(n);
	if (V->length() == n)
		V = 0;
	else
		V->sizeUninitialized(n);
	for(i = 0; i < n; ++i) {
		s = &(*a)[i];
		it = s->begin();
		ie = s->end();
		(*L)[i] = *it;
		(*U)[i] = *(--ie);
		if (!V)
			continue;
		if ((m = s->size()) <= 1)
			(*V)[i] = *it;
		else {
			for(t = 0., ++ie; it != ie; ++it)
				t += *it;
			t /= m;
			kx = INT_MAX;
			km = -kx;
			for(it = s->begin(); it != ie; ++it) {
				if ((t1 = k = *it) > t) {
					if (kx > k)
						kx = k;
					}
				else if (t1 < t) {
					if (km < k)
						km = k;
					}
				else {
					km = kx = k;
					break;
					}
				}
			if (kx - t < t - km)
				km = kx;
			(*V)[i] = km;
			}
		}
	}

 static void
DRset(size_t n, RealSetArray *a, RealVector *L, RealVector *U, RealVector *V)
{
	Real t, t1, tm, tx;
	RealSet *s;
	RealSet::const_iterator ie, it;
	int i;
	size_t m;

	L->sizeUninitialized(n);
	U->sizeUninitialized(n);
	if (V->length() == n)
		V = 0;
	else
		V->sizeUninitialized(n);
	for(i = 0; i < n; ++i) {
		s = &(*a)[i];
		it = s->begin();
		ie = s->end();
		(*L)[i] = *it;
		(*U)[i] = *(--ie);
		if (!V)
			continue;
		if ((m = s->size()) <= 1)
			(*V)[i] = *it;
		else {
			for(t = 0., ++ie; it != ie; ++it)
				t += *it;
			t /= m;
			tx = DBL_MAX;
			tm = -tx;
			for(it = s->begin(); it != ie; ++it) {
				if ((t1 = *it) > t) {
					if (tx > t1)
						tx = t1;
					}
				else if (t1 < t) {
					if (tm < t1)
						tm = t1;
					}
				else {
					tm = tx = t1;
					break;
					}
				}
			if (tx - t < t - tm)
				tm = tx;
			(*V)[i] = tm;
			}
		}
	}

 static void
Vbgen_DDSI(DataVariablesRep *dv, size_t n)
{
	DIset(n,
		&dv->discreteDesignSetInt,
		&dv->discreteDesignSetIntLowerBnds,
		&dv->discreteDesignSetIntUpperBnds,
		&dv->discreteDesignSetIntVars);
	}

 static void
Vbgen_DDSR(DataVariablesRep *dv, size_t n)
{
	DRset(n,
		&dv->discreteDesignSetReal,
		&dv->discreteDesignSetRealLowerBnds,
		&dv->discreteDesignSetRealUpperBnds,
		&dv->discreteDesignSetRealVars);
	}

 static void
Vbgen_DSSI(DataVariablesRep *dv, size_t n)
{
	DIset(n,
		&dv->discreteStateSetInt,
		&dv->discreteStateSetIntLowerBnds,
		&dv->discreteStateSetIntUpperBnds,
		&dv->discreteStateSetIntVars);
	}

 static void
Vbgen_DSSR(DataVariablesRep *dv, size_t n)
{
	DRset(n,
		&dv->discreteStateSetReal,
		&dv->discreteStateSetRealLowerBnds,
		&dv->discreteStateSetRealUpperBnds,
		&dv->discreteStateSetRealVars);
	}

 static void
not_div(const char *kind, size_t nsv, size_t m)
{
	Squawk("Number of %s set_values (%d)\n"
		"not evenly divisible by numberof variables (%d).\n"
		"Use num_set_values for unequal apportionment",
		kind, (int)nsv, (int)m);
	}

 static void
wrong_number(const char *what, const char *kind, size_t nsv, size_t m)
{
	Squawk("Expected %d %s for %s, not %d", (int)nsv, what, kind, (int)m);
	}

 static void
too_small(const char *kind)
{
	Squawk("num_set_values values for %s must be >= 1", kind);
	}

 static void
suppressed(const char *kind, int ndup, int *ip, Real *rp)
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
		else
			Squawk("Duplicate %s value %.17g", kind, rp[i]);
	if (nother) {
		s = "s" + (nother == 1);
		Squawk("Warning%s of %d other duplicate %s value%s suppressed",
			s, nother, kind, s);
		}
	}

 static void
bad_initial_ivalue(const char *kind, int val)
{
	Squawk("invalid initial value %d for %s", val, kind);
	}

 static void
bad_initial_rvalue(const char *kind, Real val)
{
	Squawk("invalid initial value %.17g for %s", val, kind);
	}

 static void
Vadj_DiscreteDesSetReal(DataVariablesRep *dv, size_t i0, Var_Info *vi)
// This should be combined suitably with Vadj_DiscreteStateSetReal
{
  IntArray *num_dsvr;
  Real dupval[2], val;
  RealVector *dsvr, *dsvr_ip;
  RealSet *dsvr_all_i;
  RealSetArray *dsvr_all;
  int avg_num_dsvr, ndup, num_dsvr_i, total_dsvr;
  size_t i, j, dsvr_len, m, cntr;
  static char kind[] = "discrete_design_set_real";

  if ((dsvr = vi->dsvr)) {
    dsvr_len = dsvr->length();

    bool key;
    if ((num_dsvr = vi->ndsvr)) {
      key = true;
      m = num_dsvr->size();
      if (m != dv->numDiscreteDesSetRealVars) {
	wrong_number("num_set_values value(s)", kind, dv->numDiscreteDesSetRealVars, m);
	return;
      }
      for(i = total_dsvr = 0; i < m; ++i) {
	total_dsvr += num_dsvr_i = (*num_dsvr)[i];
	if (num_dsvr_i < 1) {
	  too_small(kind);
	  return;
	}
      }
      if (dsvr_len != total_dsvr) {
	wrong_number("set_values", kind, total_dsvr, dsvr_len);
	return;
      }
    }
    else { // num_set_pairs is optional; use average len if no spec
      key = false;
      m = dv->numDiscreteDesSetRealVars;
      if (dsvr_len % m) {
	not_div(kind, dsvr_len, m);
	return;
      }
      else
	avg_num_dsvr = dsvr_len / m;
    }

    dsvr_all = &dv->discreteDesignSetReal;
    dsvr_all->resize(m);
    ndup = 0;
    for (i=cntr=0; i<m; ++i) {
      num_dsvr_i = (key) ? (*num_dsvr)[i] : avg_num_dsvr;
      dsvr_all_i  = &((*dsvr_all)[i]);
      for (j=0; j<num_dsvr_i; ++j, ++cntr)
	if (!dsvr_all_i->insert(val = (*dsvr)[cntr]).second) {
		if (++ndup <= 2)
			dupval[ndup-1] = val;
		}
    }
    if (ndup)
   	 suppressed(kind, ndup, 0, dupval);
    dsvr_ip = &dv->discreteDesignSetRealVars;
    if ((i = dsvr_ip->length()) > 0) {
	if (i != m)
		wrong_number("initial_point value(s)", kind, m, i);
	else
		for(i = 0; i < m; ++i) {
			dsvr_all_i  = &((*dsvr_all)[i]);
			if (dsvr_all_i->find(val = (*dsvr_ip)[i]) == dsvr_all_i->end())
				bad_initial_rvalue(kind, val);
			}
	}
#if 0 // now done in Vbgen_DDSR
    else {
	dsvr_ip->sizeUninitialized(m);
	for (i=0; i<m; ++i)
		(*dsvr_ip)[i] = *(*dsvr_all)[i].begin(); // initialize to 1st value in set
	}
#endif
  }
}

 static void
Vadj_DiscreteDesSetInt(DataVariablesRep *dv, size_t i0, Var_Info *vi)
// This should be combined suitably with Vadj_DiscreteStateSetInt
{
  IntArray *num_dsvi, *dsvi;
  IntVector *dsvi_ip;
  IntSet *dsvi_all_i;
  IntSetArray *dsvi_all;
  int avg_num_dsvi, dupval[2], ndup, num_dsvi_i, total_dsvi, val;
  size_t i, j, dsvi_len, m, cntr;
  static char kind[] = "discrete_design_set_integer";

  if (dsvi = vi->dsvi) {
    dsvi_len = dsvi->size();

    bool key;
    if ((num_dsvi = vi->ndsvi)) {
      key = true;
      m = num_dsvi->size();
      if (m != dv->numDiscreteDesSetIntVars) {
	wrong_number("num_set_values value(s)", kind, dv->numDiscreteDesSetIntVars, m);
	return;
      }
      for(i = total_dsvi = 0; i < m; ++i) {
	total_dsvi += num_dsvi_i = (*num_dsvi)[i];
	if (num_dsvi_i < 1) {
	  too_small(kind);
	  return;
	}
      }
      if (dsvi_len != total_dsvi) {
	wrong_number("set_values", kind, total_dsvi, dsvi_len);
	return;
      }
    }
    else { // num_set_pairs is optional; use average len if no spec
      key = false;
      m = dv->numDiscreteDesSetIntVars;
      if (dsvi_len % m) {
	not_div(kind, dsvi_len, m);
	return;
      }
      else
	avg_num_dsvi = dsvi_len / m;
    }

    dsvi_all = &dv->discreteDesignSetInt;
    dsvi_all->resize(m);
    ndup = 0;
    for (i=cntr=0; i<m; ++i) {
      num_dsvi_i = (key) ? (*num_dsvi)[i] : avg_num_dsvi;
      dsvi_all_i  = &((*dsvi_all)[i]);
      for (j=0; j<num_dsvi_i; ++j, ++cntr)
	if (!dsvi_all_i->insert(val = (*dsvi)[cntr]).second)
		if (++ndup <= 2)
			dupval[ndup-1] = val;
    }
    if (ndup)
   	 suppressed(kind, ndup, dupval, 0);
    dsvi_ip = &dv->discreteDesignSetIntVars;
    if ((i = dsvi_ip->length()) > 0) {
	if (i != m)
		wrong_number("initial_point value(s)", kind, m, i);
	else
		for(i = 0; i < m; ++i) {
			dsvi_all_i  = &((*dsvi_all)[i]);
			if (dsvi_all_i->find(val = (*dsvi_ip)[i]) == dsvi_all_i->end())
				bad_initial_rvalue(kind, val);
			}
	}
#if 0 // now done in Vbgen_DDSI
    else {
	dsvi_ip->sizeUninitialized(m);
	for (i=0; i<m; ++i)
		(*dsvi_ip)[i] = *(*dsvi_all)[i].begin(); // initialize to 1st value in set
	}
#endif
  }
}

 static void
Vadj_DiscreteStateSetReal(DataVariablesRep *dv, size_t i0, Var_Info *vi)
// This should be combined suitably with Vadj_DiscreteDesSetReal
{
  IntArray *num_ssvr;
  Real dupval[2], val;
  RealVector *ssvr, *ssvr_ip;
  RealSet *ssvr_all_i;
  RealSetArray *ssvr_all;
  int avg_num_ssvr, ndup, num_ssvr_i, total_ssvr;
  size_t i, j, ssvr_len, m, cntr;
  static char kind[] = "discrete_state_set_real";

  if (ssvr = vi->ssvr) {
    ssvr_len = ssvr->length();

    bool key;
    if ((num_ssvr = vi->nssvr)) {
      key = true;
      m = num_ssvr->size();
      if (m != dv->numDiscreteStateSetRealVars) {
	wrong_number("num_set_values value(s)", kind, dv->numDiscreteStateSetRealVars, m);
	return;
      }
      for(i = total_ssvr = 0; i < m; ++i) {
	total_ssvr += num_ssvr_i = (*num_ssvr)[i];
	if (num_ssvr_i < 1) {
	  too_small(kind);
	  return;
	}
      }
      if (ssvr_len != total_ssvr) {
	wrong_number("set_values", kind, total_ssvr, ssvr_len);
	return;
      }
    }
    else { // num_set_pairs is optional; use average len if no spec
      key = false;
      m = dv->numDiscreteStateSetRealVars;
      if (ssvr_len % m) {
	not_div(kind, ssvr_len, m);
	return;
      }
      else
	avg_num_ssvr = ssvr_len / m;
    }

    ssvr_all = &dv->discreteStateSetReal;
    ssvr_all->resize(m);
    ndup = 0;
    for (i=cntr=0; i<m; ++i) {
      num_ssvr_i = (key) ? (*num_ssvr)[i] : avg_num_ssvr;
      ssvr_all_i  = &((*ssvr_all)[i]);
      for (j=0; j<num_ssvr_i; ++j, ++cntr)
	if (!ssvr_all_i->insert(val = (*ssvr)[cntr]).second)
		if (++ndup <= 2)
			dupval[ndup-1] = val;
    }
    if (ndup)
   	 suppressed(kind, ndup, 0, dupval);
    ssvr_ip = &dv->discreteStateSetRealVars;
    if ((i = ssvr_ip->length()) > 0) {
	if (i != m)
		wrong_number("initial_point value(s)", kind, m, i);
	else
		for(i = 0; i < m; ++i) {
			ssvr_all_i  = &((*ssvr_all)[i]);
			if (ssvr_all_i->find(val = (*ssvr_ip)[i]) == ssvr_all_i->end())
				bad_initial_rvalue(kind, val);
			}
	}
#if 0 // now done in Vbgen_DSSR
    else {
	ssvr_ip->sizeUninitialized(m);
	for (i=0; i<m; ++i)
		(*ssvr_ip)[i] = *(*ssvr_all)[i].begin(); // initialize to 1st value in set
	}
#endif
  }
}

 static void
Vadj_DiscreteStateSetInt(DataVariablesRep *dv, size_t i0, Var_Info *vi)
// This should be combined suitably with Vadj_DiscreteDesSetInt
{
  IntArray *num_ssvi, *ssvi;
  IntVector *ssvi_ip;
  IntSet *ssvi_all_i;
  IntSetArray *ssvi_all;
  int avg_num_ssvi, dupval[2], ndup, num_ssvi_i, total_ssvi, val;
  size_t i, j, ssvi_len, m, cntr;
  static char kind[] = "discrete_state_set_integer";

  if (ssvi = vi->ssvi) {
    ssvi_len = ssvi->size();

    bool key;
    if ((num_ssvi = vi->nssvi)) {
      key = true;
      m = num_ssvi->size();
      if (m != dv->numDiscreteStateSetIntVars) {
	wrong_number("num_set_values value(s)", kind, dv->numDiscreteStateSetIntVars, m);
	return;
      }
      for(i = total_ssvi = 0; i < m; ++i) {
	total_ssvi += num_ssvi_i = (*num_ssvi)[i];
	if (num_ssvi_i < 1) {
	  too_small(kind);
	  return;
	}
      }
      if (ssvi_len != total_ssvi) {
	wrong_number("set_values", kind, total_ssvi, ssvi_len);
	return;
      }
    }
    else { // num_set_pairs is optional; use average len if no spec
      key = false;
      m = dv->numDiscreteStateSetIntVars;
      if (ssvi_len % m) {
	not_div(kind, ssvi_len, m);
	return;
      }
      else
	avg_num_ssvi = ssvi_len / m;
    }

    ssvi_all = &dv->discreteStateSetInt;
    ssvi_all->resize(m);
    ndup = 0;
    for (i=cntr=0; i<m; ++i) {
      num_ssvi_i = (key) ? (*num_ssvi)[i] : avg_num_ssvi;
      ssvi_all_i  = &((*ssvi_all)[i]);
      for (j=0; j<num_ssvi_i; ++j, ++cntr)
	if (!ssvi_all_i->insert(val = (*ssvi)[cntr]).second)
		if (++ndup <= 2)
			dupval[ndup-1] = val;
    }
    if (ndup)
   	 suppressed(kind, ndup, dupval, 0);
    ssvi_ip = &dv->discreteStateSetIntVars;
    if ((i = ssvi_ip->length()) > 0) {
	if (i != m)
		wrong_number("initial_point value(s)", kind, m, i);
	else
		for(i = 0; i < m; ++i) {
			ssvi_all_i  = &((*ssvi_all)[i]);
			if (ssvi_all_i->find(val = (*ssvi_ip)[i]) == ssvi_all_i->end())
				bad_initial_rvalue(kind, val);
			}
	}
#if 0 // now done in Vbgen_DSSI
    else {
	ssvi_ip->sizeUninitialized(m);
	for (i=0; i<m; ++i)
		(*ssvi_ip)[i] = *(*ssvi_all)[i].begin(); // initialize to 1st value in set
	}
#endif
  }
}

#define UncInfo(a,b)     { #a, #b, &DataVariablesRep::num##b##UncVars, Vadj_##b }
 static Var_uinfo CAUVLbl[CAUVar_Nkinds] = {
	UncInfo(nuv_, Normal),
	UncInfo(lnuv_, Lognormal),
	UncInfo(uuv_, Uniform),
	UncInfo(luuv_, Loguniform),
	UncInfo(tuv_, Triangular),
	UncInfo(euv_, Exponential),
	UncInfo(beuv_, Beta),
	UncInfo(gauv_, Gamma),
	UncInfo(guuv_, Gumbel),
	UncInfo(fuv_, Frechet),
	UncInfo(wuv_, Weibull),
	UncInfo(hbuv_, HistogramBin)
	};
 static Var_uinfo DAUIVLbl[DAUIVar_Nkinds] = {
	UncInfo(puv_, Poisson),
        UncInfo(biuv_, Binomial),
        UncInfo(nbuv_, NegBinomial),
        UncInfo(geuv_, Geometric),
        UncInfo(hguv_, HyperGeom)
	};
 static Var_uinfo DAURVLbl[DAURVar_Nkinds] = {
	UncInfo(hpuv_, HistogramPt)
	};
 static Var_uinfo CEUVLbl[CEUVar_Nkinds] = {
	UncInfo(iuv_, Interval)
	};
// static Var_uinfo DEUIVLbl[DEUIVar_Nkinds] = {
//	};
// static Var_uinfo DEURVLbl[DEURVar_Nkinds] = {
//	};
#undef UncInfo

#define DiscSetInfo(a,b)     { #a, #b, &DataVariablesRep::numDiscrete##b##Vars, Vadj_Discrete##b }
 static Var_uinfo DiscSetLbl[DiscSetVar_Nkinds] = {
	DiscSetInfo(ddsiv_, DesSetInt),
	DiscSetInfo(ddsrv_, DesSetReal),
	DiscSetInfo(dssiv_, StateSetInt),
	DiscSetInfo(dssrv_, StateSetReal)
	};
#undef DiscSetInfo

 void NIDRProblemDescDB::
var_stop(const char *keyname, Values *val, void **g, void *v)
{
	Var_Info *vi = *(Var_Info**)g;
	DataVariablesRep *dv = vi->dv;
	scale_chk(dv->continuousDesignScaleTypes, dv->continuousDesignScales, "cdv", aln_scaletypes);
	pDDBInstance->VIL.push_back(vi);
	pDDBInstance->dataVariablesList.push_back(*vi->dv0);
	delete vi->dv0;
	}

struct VarLabelChk {
	size_t DataVariablesRep::* n;
	StringArray DataVariablesRep::* sa;
	const char *stub;
	const char *name;
	};

#define AVI &DataVariablesRep::
	static VarLabelChk Vlch[] = {
	 { AVI numContinuousDesVars, AVI continuousDesignLabels, "cdv_", "cdv_descriptors" },
	 { AVI numDiscreteDesRangeVars,	AVI discreteDesignRangeLabels, "ddriv_", "ddriv_descriptors" },
	 { AVI numDiscreteDesSetIntVars, AVI discreteDesignSetIntLabels, "ddsiv_", "ddsiv_descriptors" },
	 { AVI numDiscreteDesSetRealVars, AVI discreteDesignSetRealLabels, "ddsrv_", "ddsrv_descriptors" },
	 { AVI numContinuousStateVars, AVI continuousStateLabels, "csv_", "csv_descriptors" },
	 { AVI numDiscreteStateRangeVars, AVI discreteStateRangeLabels, "dsriv_", "dsriv_descriptors" },
	 { AVI numDiscreteStateSetIntVars, AVI discreteStateSetIntLabels, "dssiv_", "dssiv_descriptors" },
	 { AVI numDiscreteStateSetRealVars, AVI discreteStateSetRealLabels, "dssrv_", "dssrv_descriptors" },
	 { AVI numContinuousDesVars, AVI continuousDesignScaleTypes, 0, "cdv_scale_types" }
	 };
#undef  AVI

 struct VLstuff {
	int n;
	int isreal;
	VarLabel Var_Info::* VL; // should be "VarLabel *Var_Info::* VL"
				 // but g++ is buggy (versions 4.3.1, 4.4.2 anyway)
	Var_uinfo *vui;
	StringArray DataVariablesRep::* Labels;
	RealVector DataVariablesRep::* LowerBnds;
	RealVector DataVariablesRep::* UpperBnds;
	RealVector DataVariablesRep::* UncVars;
	};

enum { N_VLS = 4 };

#define AVI (VarLabel Var_Info::*) &Var_Info::	// cast to bypass g++ bug
#define DVR &DataVariablesRep::
#define RDVR (RealVector DataVariablesRep::*) DVR

static VLstuff VLS[N_VLS] = {
 {CAUVar_Nkinds,  1, AVI CAUv,  CAUVLbl,
	DVR continuousAleatoryUncLabels,
	DVR continuousAleatoryUncLowerBnds,
	DVR continuousAleatoryUncUpperBnds,
	DVR continuousAleatoryUncVars},
 {CEUVar_Nkinds,  1, AVI CEUv,  CEUVLbl,
	DVR continuousEpistemicUncLabels,
	DVR continuousEpistemicUncLowerBnds,
	DVR continuousEpistemicUncUpperBnds,
	DVR continuousEpistemicUncVars},
 {DAUIVar_Nkinds, 0, AVI DAUIv, DAUIVLbl,
	DVR  discreteIntAleatoryUncLabels,
	RDVR discreteIntAleatoryUncLowerBnds,
	RDVR discreteIntAleatoryUncUpperBnds,
	RDVR discreteIntAleatoryUncVars},
 {DAURVar_Nkinds, 1, AVI DAURv, DAURVLbl,
	DVR discreteRealAleatoryUncLabels,
	DVR discreteRealAleatoryUncLowerBnds,
	DVR discreteRealAleatoryUncUpperBnds,
	DVR discreteRealAleatoryUncVars}};

#undef RDVR
#undef	DVR
#undef  AVI

static int VL_aleatory[N_VLS] = { 1, 0, 1, 1 };


 void NIDRProblemDescDB::
make_variable_defaults(std::list<DataVariables>* dvl)
{
	DataVariablesRep *dv;
	IntVector *IL, *IU, *IV;
	RealVector *L, *U, *V;
	StringArray *sa;
	VLstuff *vls;
	VarLabel *vl;
	VarLabelChk *vlc, *vlce;
	Var_uinfo *vui, *vuie;
	char buf[32];
	size_t i, j, k, n, nu, nusave[N_VLS];
	static const char Inconsistent_bounds[] =
		"Inconsistent bounds on %s uncertain variables";

	std::list<DataVariables>::iterator It = dvl->begin(), Ite = dvl->end();
	for(; It != Ite; ++It) {
		dv = It->dataVarsRep;
		for(k = 0; k < N_VLS; ++k) {
			vls = &VLS[k];
			vui = vls->vui;
			vuie = vui + vls->n;
			for(nu = 0; vui < vuie; ++vui)
				nu += dv->*vui->n;
			nusave[k] = nu;
			if (!nu)
				continue;
			if (vls->isreal) {
				L = &(dv->*vls->LowerBnds);
				U = &(dv->*vls->UpperBnds);
				V = &(dv->*vls->UncVars);
				L->sizeUninitialized(nu);
				U->sizeUninitialized(nu);
				V->sizeUninitialized(nu);
				}
			else {
				#define IVp IntVector DataVariablesRep::*
				IL = &(dv->*(IVp)vls->LowerBnds);
				IU = &(dv->*(IVp)vls->UpperBnds);
				IV = &(dv->*(IVp)vls->UncVars);
				IL->sizeUninitialized(nu);
				IU->sizeUninitialized(nu);
				IV->sizeUninitialized(nu);
				#undef IVp
				}
			}
		Var_boundgen(dv);
		Var_iboundgen(dv);
		for(k = 0; k < N_VLS; ++k) {
			nu = nusave[k];
			if (!nu)
				continue;
			vls = &VLS[k];
			vui = vls->vui;
			vuie = vui + vls->n;
			if (vls->isreal) {
				L = &(dv->*vls->LowerBnds);
				U = &(dv->*vls->UpperBnds);
				V = &(dv->*vls->UncVars);
				}
			else {
				L = 0;
				#define IVp IntVector DataVariablesRep::*
				IL = &(dv->*(IVp)vls->LowerBnds);
				IU = &(dv->*(IVp)vls->UpperBnds);
				IV = &(dv->*(IVp)vls->UncVars);
				#undef IVp
				}
			sa = &(dv->*vls->Labels);
			if (!sa->size())
				sa->resize(nu);
			i = 0;
			for(vui = vls->vui; vui < vuie; ++vui) {
				if ((n = dv->*vui->n) == 0)
					continue;
				if (L) {
				    for(j = 0; j < n; ++j)
					if ((*L)[i+j] > (*U)[i+j]) {
						squawk(Inconsistent_bounds, vui->vkind);
						break;
						}
				    }
				else {
				    for(j = 0; j < n; ++j)
					if ((*IL)[i+j] > (*IU)[i+j]) {
						squawk(Inconsistent_bounds, vui->vkind);
						break;
						}
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
		for(vlc = Vlch, vlce = vlc + Numberof(Vlch); vlc < vlce; ++vlc)
			if (vlc->stub && (n = dv->*vlc->n)) {
				sa = &(dv->*vlc->sa);
				if (sa->size() == 0)
					BuildLabels(sa, n, 0, n, vlc->stub);
				}
		}
	}

 void NIDRProblemDescDB::
var_stop1(void *v)
{
	DataVariablesRep *dv;
	IntArray *Iv;
	RealSymMatrix *Rm;
	RealVector *Rv;
	StringArray *sa;
	VLstuff *vls;
	VarLabel *vl;
	VarLabelChk *vlc, *vlce;
	Var_Info *vi;
	Var_uinfo *vui, *vuie;
	const char **sp;
	int havelabels;
	size_t i, j, k, n, nd, nu, nuk, nutot, nv;
#define AVI &Var_Info::
	static IntArray *Var_Info::* Ivzap[] = { AVI nIv, AVI nbp, AVI npp };
	static RealVector *Var_Info::* Rvzap[] = { AVI Ivb, AVI Ivp, AVI ba, AVI bo,
						   AVI bc, AVI pa, AVI pc, AVI ucm };
#undef AVI
	static char ucmerr[] = "Got %lu entries for the uncertain_correlation_matrix\n\
	but needed %lu for %lu uncertain variables";

	vi = (Var_Info*)v;
	dv = vi->dv;

	/* check and generate labels for design and state variables */

	nv = 0;
	for(vlc = Vlch, vlce = vlc + Numberof(Vlch); vlc < vlce; ++vlc)
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

	/* now deal with uncertain variables */

	for(k = nu = nutot = 0; k < N_VLS; ++k) {
		vls = &VLS[k];
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
			if (VL_aleatory[k])
				nu += nuk;
			if (havelabels)
				(sa = &(dv->*vls->Labels))->resize(nuk);
			i = 0;
			vl = &(vi->*vls->VL);
			for(vui = vls->vui; vui < vuie; ++vl, ++vui) {
				if ((n = dv->*vui->n) == 0)
					continue;
				vui->vadj(dv,i,vi);
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

	if ((Rv = vi->ucm)) { // uncertain_correlation_matrix
		n = Rv->length();
		if (n != nu*nu)
			squawk(ucmerr, (UL)n, (UL)nu*nu, (UL)nu);
		else {
			Rm = &dv->uncertainCorrelations;
			Rm->reshape(nu);
			for(i = k = 0; i < nu; ++i) {
				for(j = 0; j < nu;)
					(*Rm)(i,j++) = (*Rv)[k++];
				}
			}
		}

	/* check discrete variables */

	vuie = DiscSetLbl + DiscSetVar_Nkinds;
	nd = 0;
	for(vui = DiscSetLbl; vui < vuie; ++vui) {
		if ((n = dv->*vui->n) > 0) {
			vui->vadj(dv,0,vi);
			nd += n;
			}
		}

	if (nd + nv + nutot == 0)
		squawk("at least one variable must be specified");

	/* check bounds */

	Var_boundchk(dv);
	Var_iboundchk(dv);


	/* finish up and clean up */

	n = Numberof(Ivzap);
	for(i = 0; i < n; i++) {
		if ((Iv = vi->*Ivzap[i]))
			delete Iv;
		}
	n = Numberof(Rvzap);
	for(i = 0; i < n; i++) {
		if ((Rv = vi->*Rvzap[i]))
			delete Rv;
		}
	delete vi;
	if (nerr)
		abort_handler(-1);
	}

 static void
Rdv_copy(RealVector **prdv, RealVectorArray *rdva)
{
	size_t i, j, k, m, n;
	RealVector *rdv, *rdv1;

	*prdv = rdv = new RealVector();
	m = rdva->size();
	for(i = n = 0; i < m; ++i)
		n += (*rdva)[i].length();
	rdv->sizeUninitialized(n);
	for(i = k = 0; i < m; ++i) {
		rdv1 = &(*rdva)[i];
		n = rdv1->length();
		for(j = 0; j < n; ++j)
			(*rdv)[k++] = (*rdv1)[j];
		}
	}

 void NIDRProblemDescDB::
check_variables(std::list<DataVariables>* dvl)
{
  if (pDDBInstance) {
    std::list<void*>::iterator It  = pDDBInstance->VIL.begin(),
                          Ite = pDDBInstance->VIL.end();
    for(; It != Ite; ++It)
      var_stop1(*It);
    pDDBInstance->VIL.clear();
  }
  else {
    // library mode with manual provision of everything
    DataVariablesRep *dv;
    IntArray *iv;
    RealSymMatrix *rsdm;
    RealVector *rdv, *rva, *rvc;
    RealVectorArray *rdva;
    Var_Info *vi;
    size_t i, j, m, n, cntr;
    int num_prs_i, total_prs;

    // copy from DataVariables into Var_Info so that var_stop1 can go the
    // other direction.  TO DO: can we eliminate this circular update?
    std::list<DataVariables>::iterator It = dvl->begin(), Ite = dvl->end();
    for(; It != Ite; ++It) {
      vi = new Var_Info;
      memset(vi, 0, sizeof(Var_Info));
      vi->dv0 = &*It;
      vi->dv = dv = It->dataVarsRep;
      if ((n = dv->numIntervalUncVars)) {
	rdva = &dv->intervalUncBasicProbs;
	m = rdva->size();
	vi->nIv = iv = new IntArray(m);
	for(i = 0; i < m; ++i)
	  (*iv)[i] = (*rdva)[i].length();
	Rdv_copy(&vi->Ivb, &dv->intervalUncBounds);
	Rdv_copy(&vi->Ivp, rdva);
      }
      rdva = &dv->histogramUncBinPairs;
      if ((m = rdva->size())) {
	vi->nbp = iv = new IntArray(m);
	for(i = 0; i < m; ++i)
	  total_prs += (*iv)[i] = (*rdva)[i].length() / 2;
	vi->ba = rva = new RealVector(total_prs); // abscissas
	vi->bc = rvc = new RealVector(total_prs); // counts
	vi->bo = NULL;                            // no ordinates
	for(i = cntr = 0; i < m; ++i) {
	  num_prs_i = (*iv)[i];
	  for(j = 0; j < num_prs_i; ++j, ++cntr) {
	    (*rva)[cntr] = (*rdva)[i][2*j];   // abscissas
	    (*rvc)[cntr] = (*rdva)[i][2*j+1]; // counts only (no ordinates)
	  }
	  // normalization occurs in Vadj_HistogramBin going the other direction
	}
      }
      rdva = &dv->histogramUncPointPairs;
      if ((m = rdva->size())) {
	vi->npp = iv = new IntArray(m);
	for(i = 0; i < m; ++i)
	  total_prs += (*iv)[i] = (*rdva)[i].length() / 2;
	vi->pa = rva = new RealVector(total_prs); // abscissas
	vi->pc = rvc = new RealVector(total_prs); // counts
	for(i = cntr = 0; i < m; ++i) {
	  num_prs_i = (*iv)[i];
	  for(j = 0; j < num_prs_i; ++j, ++cntr) {
	    (*rva)[cntr] = (*rdva)[i][2*j];   // abscissas
	    (*rvc)[cntr] = (*rdva)[i][2*j+1]; // counts
	  }
	  // normalization occurs in Vadj_HistogramPt going the other direction
	}
      }
      rsdm = &dv->uncertainCorrelations;
      if ((m = rsdm->numRows())) {
	vi->ucm = rdv = new RealVector(m*m, false);
	for(i = n = 0; i < m; ++i)
	  for(j = 0; j < m; ++j)
	    (*rdv)[n++] = (*rsdm)(i,j);
      }
      var_stop1((void*)vi);
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
var_dailbl(const char *keyname, Values *val, void **g, void *v)
{
	VarLabel *vl = &(*(Var_Info**)g)->DAUIv[(char*)v - (char*)0];
	var_iulbl(keyname, val, vl);
	}

 void NIDRProblemDescDB::
var_darlbl(const char *keyname, Values *val, void **g, void *v)
{
	VarLabel *vl = &(*(Var_Info**)g)->DAURv[(char*)v - (char*)0];
	var_iulbl(keyname, val, vl);
	}

#define MP_(x) DataInterfaceRep::* iface_mp_##x = &DataInterfaceRep::x
#define MP2(x,y) iface_mp_##x##_##y = {&DataInterfaceRep::x,#y}
#define MP3(x,y,z) iface_mp_3##x##_##z = {&DataInterfaceRep::x,&DataInterfaceRep::y,#z}

static Iface_mp_Rlit
	MP3(failAction,recoveryFnVals,recover);

static Iface_mp_ilit
	MP3(failAction,retryLimit,retry);

static Iface_mp_lit
	MP2(analysisScheduling,self),
	MP2(analysisScheduling,static),
	MP2(evalScheduling,self),
	MP2(evalScheduling,static),
	MP2(failAction,abort),
	MP2(failAction,continuation),
	MP2(interfaceSynchronization,asynchronous),
	MP2(interfaceType,direct),
	MP2(interfaceType,fork),
	MP2(interfaceType,grid),
	MP2(interfaceType,system),
	MP2(asynchLocalEvalScheduling,self),
	MP2(asynchLocalEvalScheduling,static);

static String
	MP_(algebraicMappings),
	MP_(idInterface),
	MP_(inputFilter),
	MP_(outputFilter),
	MP_(parametersFile),
	MP_(resultsFile),
	MP_(templateDir),
	MP_(workDir);

static String2DArray
	MP_(analysisComponents);

static StringArray
	MP_(analysisDrivers),
	MP_(templateFiles);

static bool
	MP_(activeSetVectorFlag),
	MP_(allowExistingResultsFlag),
	MP_(apreproFlag),
	MP_(dirSave),
	MP_(dirTag),
	MP_(evalCacheFlag),
	MP_(fileSaveFlag),
	MP_(fileTagFlag),
	MP_(restartFileFlag),
	MP_(templateCopy),
	MP_(templateReplace),
	MP_(useWorkdir),
	MP_(verbatimFlag);

static int
	MP_(analysisServers),
	MP_(asynchLocalAnalysisConcurrency),
	MP_(asynchLocalEvalConcurrency),
	MP_(evalServers),
	MP_(procsPerAnalysis);

#undef MP3
#undef MP2
#undef MP_

#define MP_(x) DataMethodRep::* method_mp_##x = &DataMethodRep::x
#define MP2(x,y) method_mp_##x##_##y = {&DataMethodRep::x,#y}
#define MP2s(x,y) method_mp_##x##_##y = {&DataMethodRep::x,y}
#define MP2p(x,y) method_mp_##x##_##y = {&DataMethodRep::x,Pecos::y}
#define MP3(x,y,z) method_mp_3##x##_##z = {&DataMethodRep::x,&DataMethodRep::y,#z}
#define MP4(w,x,y,z) method_mp_##w##_##y = {&DataMethodRep::w,&DataMethodRep::x,#y,z}

static IntVector
	MP_(primeBase),
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
	MP2(boxDivision,all_dimensions),
	MP2(boxDivision,major_dimension),
	MP2(collocPtReuse,all),
	MP2(convergenceType,average_fitness_tracker),
	MP2(convergenceType,best_fitness_tracker),
	MP2(convergenceType,metric_tracker),
	MP2(crossoverType,blend),
	MP2(crossoverType,two_point),
	MP2(crossoverType,uniform),
	MP2(distributionType,complementary),
	MP2(distributionType,cumulative),
	MP2(evalSynchronization,blocking),
	MP2(evalSynchronization,nonblocking),
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
	MP2(initializationType,random),
	MP2(initializationType,unique_random),
	MP2(integrationRefine,ais),
	MP2(integrationRefine,is),
	MP2(integrationRefine,mmais),
        MP2(meritFunction,merit_max),
        MP2(meritFunction,merit_max_smooth),
        MP2(meritFunction,merit1),
        MP2(meritFunction,merit1_smooth),
        MP2(meritFunction,merit2),
        MP2(meritFunction,merit2_smooth),
        MP2(meritFunction,merit2_squared),
	MP2(methodName,asynch_pattern_search),
	MP2(methodName,coliny_cobyla),
	MP2(methodName,coliny_direct),
	MP2(methodName,coliny_pattern_search),
	MP2(methodName,coliny_solis_wets),
	MP2(methodName,conmin_frcg),
	MP2(methodName,conmin_mfd),
	MP2(methodName,dace),
	MP2(methodName,dot_bfgs),
	MP2(methodName,dot_frcg),
	MP2(methodName,dot_mmfd),
	MP2(methodName,dot_slp),
	MP2(methodName,dot_sqp),
	MP2(methodName,efficient_global),
	MP2(methodName,fsu_cvt),
	MP2(methodName,fsu_halton),
	MP2(methodName,fsu_hammersley),
	MP2(methodName,ncsu_direct),
	MP2(methodName,nl2sol),
	MP2(methodName,nlpql_sqp),
	MP2(methodName,nlssol_sqp),
	MP2(methodName,nond_bayes_calibration),
	MP2(methodName,nond_global_evidence),
        MP2(methodName,nond_global_interval_est),
	MP2(methodName,nond_global_reliability),
        MP2(methodName,nond_importance_sampling),
 	MP2(methodName,nond_local_evidence),
        MP2(methodName,nond_local_interval_est),
	MP2(methodName,nond_polynomial_chaos),
	MP2(methodName,nond_sampling),
	MP2(methodName,nond_stoch_collocation),
	MP2(methodName,nonlinear_cg),
	MP2(methodName,npsol_sqp),
	MP2(methodName,optpp_cg),
	MP2(methodName,optpp_fd_newton),
	MP2(methodName,optpp_g_newton),
	MP2(methodName,optpp_newton),
	MP2(methodName,optpp_pds),
	MP2(methodName,optpp_q_newton),
	MP2(methodName,psuade_moat),
	MP2(methodName,richardson_extrap),
	MP2(methodName,surrogate_based_global),
	MP2(methodName,surrogate_based_local),
	MP2(methodName,vector_parameter_study),
	MP2(methodName,list_parameter_study),
	MP2(methodName,centered_parameter_study),
	MP2(methodName,multidim_parameter_study),
	MP2(metropolisType,adaptive),
	MP2(metropolisType,hastings),
	MP2(minMaxType,maximize),
	MP2(minMaxType,minimize),
	MP2(mutationType,bit_random),
	MP2(mutationType,offset_cauchy),
	MP2(mutationType,offset_normal),
	MP2(mutationType,offset_uniform),
	MP2(mutationType,replace_uniform),
	MP2(nondOptAlgorithm,nip),
	MP2(nondOptAlgorithm,sqp),
	MP2(nondOptAlgorithm,lhs),
	MP2(nondOptAlgorithm,ego),
	MP2(patternBasis,coordinate),
	MP2(patternBasis,simplex),
	MP2(rejectionType,standard),
	MP2(rejectionType,delayed),
	MP2(reliabilityIntegration,first_order),
	MP2(reliabilityIntegration,second_order),
	MP2(reliabilitySearchType,amv_plus_u),
	MP2(reliabilitySearchType,amv_plus_x),
	MP2(reliabilitySearchType,amv_u),
	MP2(reliabilitySearchType,amv_x),
	MP2(reliabilitySearchType,egra_u),
	MP2(reliabilitySearchType,egra_x),
	MP2(reliabilitySearchType,no_approx),
	MP2(reliabilitySearchType,tana_u),
	MP2(reliabilitySearchType,tana_x),
	MP2(replacementType,elitist),
	MP2(replacementType,favor_feasible),
	MP2(replacementType,roulette_wheel),
	MP2(replacementType,unique_roulette_wheel),
	MP2(responseLevelMappingType,gen_reliabilities),
	MP2(responseLevelMappingType,probabilities),
	MP2(responseLevelMappingType,reliabilities),
	MP2(rngName,mt19937),
	MP2(rngName,rnum2),
	MP2(sampleType,incremental_lhs),
	MP2(sampleType,incremental_random),
	MP2(sampleType,lhs),
	MP2(sampleType,random),
	MP2(searchMethod,gradient_based_line_search),
	MP2(searchMethod,tr_pds),
	MP2(searchMethod,trust_region),
	MP2(searchMethod,value_based_line_search),
	MP2(subMethodName,box_behnken),
	MP2(subMethodName,central_composite),
	MP2(subMethodName,gpmsa),
	MP2(subMethodName,grid),
	MP2(subMethodName,lhs),
	MP2(subMethodName,oa_lhs),
	MP2(subMethodName,oas),
	MP2(subMethodName,queso),
	MP2(subMethodName,random),
	MP2(subMethodName,converge_order),
	MP2(subMethodName,converge_qoi),
	MP2(subMethodName,estimate_order),
	MP2(trialType,grid),
	MP2(trialType,halton),
	MP2(trialType,random);

static Method_mp_lit2
	MP4(methodName,reliabilitySearchType,nond_local_reliability,"mv");

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
	MP3(nichingType,nicheVector,radial),
	MP3(postProcessorType,distanceVector,distance_postprocessor);

static Method_mp_slit2
	MP3(initializationType,flatFile,flat_file),
	MP3(methodName,dlDetails,dl_solver);

static Real
	MP_(absConvTol),
	MP_(centeringParam),
	MP_(collocationRatio),
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
	MP_(initDelta),
	MP_(initStepLength),
	MP_(initTRRadius),
	MP_(likelihoodScale),
	MP_(lineSearchTolerance),
	MP_(localBalanceParam),
	MP_(maxBoxSize),
	MP_(maxStep),
	MP_(minBoxSize),
	MP_(mutationRate),
	MP_(mutationScale),
	MP_(proposalCovScale),
	MP_(refinementRate),
	MP_(shrinkagePercent),	// should be called shrinkageFraction
	MP_(singConvTol),
	MP_(singRadius),
        MP_(smoothFactor),
 	MP_(solnTarget),
	MP_(stepLenToBoundary),
	MP_(surrBasedLocalTRContract),
	MP_(surrBasedLocalTRContractTrigger),
	MP_(surrBasedLocalTRExpand),
	MP_(surrBasedLocalTRExpandTrigger),
	MP_(surrBasedLocalTRInitSize),
	MP_(surrBasedLocalTRMinSize),
	MP_(threshDelta),
	MP_(threshStepLength),
	MP_(vbdDropTolerance),
	MP_(volBoxSize),
	MP_(xConvTol);

static RealVector
	MP_(anisoGridDimPref),
	MP_(finalPoint),
	MP_(linearEqConstraintCoeffs),
	MP_(linearEqScales),
	MP_(linearEqTargets),
	MP_(linearIneqConstraintCoeffs),
	MP_(linearIneqLowerBnds),
	MP_(linearIneqUpperBnds),
	MP_(linearIneqScales),
	MP_(listOfPoints),
	MP_(stepVector);

static RealVectorArray
	MP_(genReliabilityLevels),
	MP_(probabilityLevels),
	MP_(reliabilityLevels),
	MP_(responseLevels);

static unsigned short
	MP_(cubIntOrder);

static UShortArray
        MP_(expansionOrder),
        MP_(quadratureOrder),
	MP_(sparseGridLevel),
	MP_(varPartitions);

static String
	MP_(centralPath),
	MP_(expansionImportFile),
	MP_(idMethod),
	MP_(logFile),
	MP_(meritFn),
	MP_(modelPointer),
	MP_(subMethodName),
	MP_(subMethodPointer),
	MP_(xObsDataFile),
	MP_(yObsDataFile),
	MP_(yStdDataFile);

static StringArray
	MP_(linearEqScaleTypes),
	MP_(linearIneqScaleTypes),
        MP_(miscOptions);

static bool
	MP_(allVarsFlag),
	MP_(constantPenalty),
	MP_(expansionFlag),
	MP_(fixedSeedFlag),
	MP_(fixedSequenceFlag),
	MP_(latinizeFlag),
	MP_(mainEffectsFlag),
	MP_(methodScaling),
	MP_(methodUseDerivsFlag),
	MP_(mutationAdaptive),
	MP_(printPopFlag),
	MP_(probCollocFlag),
	MP_(randomizeOrderFlag),
	MP_(regressDiag),
	MP_(showMiscOptions),
	MP_(speculativeFlag),
	MP_(surrBasedGlobalReplacePts),
	MP_(surrBasedLocalLayerBypass),
	MP_(vbdFlag),
	MP_(volQualityFlag);

static short
        MP_(expansionType),
	MP_(nestingOverride),
	MP_(refinementType);

static int
	MP_(collocationPoints),
	MP_(contractAfterFail),
	MP_(covarianceType),
	MP_(emulatorSamples),
	MP_(expandAfterSuccess),
	MP_(expansionSamples),
	MP_(expansionTerms),
	MP_(maxFunctionEvaluations),
	MP_(maxIterations),
	MP_(mutationRange),
	MP_(newSolnsGenerated),
	MP_(numSamples),
	MP_(numSteps),
	MP_(numSymbols),
	MP_(numTrials),
	MP_(populationSize),
	MP_(previousSamples),
	MP_(randomSeed),
	MP_(searchSchemeSize),
	MP_(surrBasedLocalSoftConvLimit),
	MP_(totalPatternSize),
	MP_(verifyLevel);

static size_t
        MP_(numFinalSolutions),
	MP_(numGenerations),
	MP_(numOffspring),
	MP_(numParents);

static Method_mp_type
	MP2s(emulatorType,GAUSSIAN_PROCESS),
	MP2s(emulatorType,POLYNOMIAL_CHAOS),
	MP2s(emulatorType,STOCHASTIC_COLLOCATION),
	MP2s(expansionType,ASKEY_U),
	MP2s(expansionType,PIECEWISE_U),
	MP2s(expansionType,STD_NORMAL_U),
	MP2p(growthOverride,RESTRICTED),             // Pecos enumeration
	MP2p(growthOverride,UNRESTRICTED),           // Pecos enumeration
	MP2s(methodOutput,DEBUG_OUTPUT),
	MP2s(methodOutput,NORMAL_OUTPUT),
	MP2s(methodOutput,QUIET_OUTPUT),
	MP2s(methodOutput,SILENT_OUTPUT),
	MP2s(methodOutput,VERBOSE_OUTPUT),
	MP2p(nestingOverride,NESTED),                // Pecos enumeration
	MP2p(nestingOverride,NON_NESTED),            // Pecos enumeration
	MP2p(refinementControl,DIMENSION_ADAPTIVE_GENERALIZED_SPARSE),  // Pecos
	MP2p(refinementControl,DIMENSION_ADAPTIVE_SPECTRAL_DECAY), // Pecos enum
	MP2p(refinementControl,DIMENSION_ADAPTIVE_TOTAL_SOBOL),    // Pecos enum
	MP2p(refinementControl,UNIFORM_CONTROL),                   // Pecos enum
	MP2p(refinementType,P_REFINEMENT),           // Pecos enumeration
        MP2p(refinementType,H_REFINEMENT),           // Pecos enumeration
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
	MP2p(vbdControl,UNIVARIATE_VBD);             // Pecos enumeration

#undef MP4
#undef MP3
#undef MP2s
#undef MP2p
#undef MP2
#undef MP_

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
	MP2(modelType,nested),
	MP2(modelType,single),
	MP2(modelType,surrogate),
	MP2(surrogateType,hierarchical),
	MP2(surrogateType,global_gaussian),
	MP2(surrogateType,global_kriging),
	MP2(surrogateType,global_mars),
	MP2(surrogateType,global_moving_least_squares),
	MP2(surrogateType,global_neural_network),
	MP2(surrogateType,global_polynomial),
	MP2(surrogateType,global_radial_basis),
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
        MP2s(pointsManagement,RECOMMENDED_POINTS);

static Real
	MP_(annRange);

static RealVector
	MP_(krigingCorrelations),
//	MP_(krigingMaxCorrelations),
//	MP_(krigingMinCorrelations),
	MP_(primaryRespCoeffs),
	MP_(secondaryRespCoeffs);

static String
	MP_(approxPointReuseFile),
	MP_(idModel),
	MP_(interfacePointer),
	MP_(krigingOptMethod),
	MP_(lowFidelityModelPointer),
	MP_(optionalInterfRespPointer),
	MP_(responsesPointer),
	MP_(subMethodPointer),
	MP_(truthModelPointer),
	MP_(variablesPointer);

static StringArray
	MP_(primaryVarMaps),
        MP_(secondaryVarMaps),
        MP_(diagMetrics);

static bool
	MP_(modelUseDerivsFlag),
	MP_(pointSelection);

static short
	MP_(annNodes),
	MP_(annRandomWeight),
	MP_(krigingMaxTrials),
	MP_(marsMaxBases),
	MP_(mlsPolyOrder),
	MP_(mlsWeightFunction),
	MP_(rbfBases),
	MP_(rbfMaxPts),
	MP_(rbfMaxSubsets),
	MP_(rbfMinPartition);

static int
        MP_(pointsTotal);

#undef MP2s
#undef MP2
#undef MP_

#define MP_(x) DataResponsesRep::* resp_mp_##x = &DataResponsesRep::x
#define MP2(x,y) resp_mp_##x##_##y = {&DataResponsesRep::x,#y}

static IntList
	MP_(idAnalyticGrads),
	MP_(idAnalyticHessians),
	MP_(idNumericalGrads),
	MP_(idNumericalHessians),
	MP_(idQuasiHessians);

static RealVector
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
	MP2(quasiHessianType,bfgs),
	MP2(quasiHessianType,damped_bfgs),
	MP2(quasiHessianType,sr1);

static String
	MP_(idResponses),
	MP_(leastSqDataFile);

static StringArray
	MP_(primaryRespFnScaleTypes),
	MP_(nonlinearEqScaleTypes),
	MP_(nonlinearIneqScaleTypes),
	MP_(responseLabels);

static bool
	MP_(centralHess),
	MP_(ignoreBounds);

static size_t
	MP_(numLeastSqTerms),
	MP_(numNonlinearEqConstraints),
	MP_(numNonlinearIneqConstraints),
	MP_(numObjectiveFunctions),
	MP_(numResponseFunctions);

#undef MP2
#undef MP_

#define MP_(x) DataStrategyRep::* strategy_mp_##x = &DataStrategyRep::x
#define MP2(x,y) strategy_mp_##x##_##y = {&DataStrategyRep::x,#y}

static Real
      //MP_(hybridProgThresh),
	MP_(hybridLSProb);

static RealVector
	MP_(concurrentParameterSets);

static Strategy_mp_lit
      //MP2(hybridCollabType,abo),
      //MP2(hybridCollabType,hops),
	MP2(hybridType,collaborative),
	MP2(hybridType,embedded),
	MP2(hybridType,sequential),
      //MP2(hybridType,sequential_adaptive),
	MP2(iteratorScheduling,self),
	MP2(iteratorScheduling,static),
	MP2(strategyType,hybrid),
	MP2(strategyType,multi_start),
	MP2(strategyType,pareto_set),
	MP2(strategyType,single_method);

static String
	MP_(hybridGlobalMethodPointer),
	MP_(hybridLocalMethodPointer),
	MP_(methodPointer),
	MP_(tabularDataFile);

static StringArray
	MP_(hybridMethodList);

static bool
	MP_(graphicsFlag),
	MP_(tabularDataFlag);

static int
	MP_(concurrentRandomJobs),
	MP_(concurrentSeed),
        MP_(iteratorServers),
        MP_(outputPrecision);

#undef MP2
#undef MP_

#define MP_(x) DataVariablesRep::* var_mp_##x = &DataVariablesRep::x
#define MP2s(x,y) var_mp_##x = {&DataVariablesRep::x,y}
#define Vchu(x,y,z) {#x,&DataVariablesRep::y,Vbgen_##z,&DataVariablesRep::z##LowerBnds,&DataVariablesRep::z##UpperBnds}
#define Vchu1(x,y,z) {#x,&DataVariablesRep::y,Vbgen_##z,&DataVariablesRep::z##LowerBnds,&DataVariablesRep::z##UpperBnds,0,0,1}
#define Vchu0(x,y,z) {#x,&DataVariablesRep::y,Vbgen_##z}
#define Vchv(x,y,z) {#x,&DataVariablesRep::y,0,&DataVariablesRep::z##LowerBnds,&DataVariablesRep::z##UpperBnds,&DataVariablesRep::z##Vars,&DataVariablesRep::z##Labels}
#define Vchi(x,y,z) {#x,&DataVariablesRep::y,&DataVariablesRep::z##LowerBnds,&DataVariablesRep::z##UpperBnds,&DataVariablesRep::z##Vars,&DataVariablesRep::z##Labels}
#define VP_(x) *Var_Info::* var_mp_Var_Info_##x = &Var_Info::x

static size_t
	MP_(numBetaUncVars),
	MP_(numBinomialUncVars),
	MP_(numContinuousDesVars),
	MP_(numContinuousStateVars),
	MP_(numDiscreteDesRangeVars),
	MP_(numDiscreteDesSetIntVars),
	MP_(numDiscreteDesSetRealVars),
	MP_(numDiscreteStateRangeVars),
	MP_(numDiscreteStateSetIntVars),
	MP_(numDiscreteStateSetRealVars),
	MP_(numExponentialUncVars),
	MP_(numFrechetUncVars),
	MP_(numGammaUncVars),
	MP_(numGeometricUncVars),
	MP_(numGumbelUncVars),
	MP_(numHistogramBinUncVars),
	MP_(numHistogramPtUncVars),
	MP_(numHyperGeomUncVars),
	MP_(numIntervalUncVars),
	MP_(numLognormalUncVars),
	MP_(numLoguniformUncVars),
	MP_(numNegBinomialUncVars),
	MP_(numNormalUncVars),
	MP_(numPoissonUncVars),
	MP_(numTriangularUncVars),
	MP_(numUniformUncVars),
	MP_(numWeibullUncVars);

static IntVector
        MP_(binomialUncNumTrials),
        MP_(hyperGeomUncTotalPop),
        MP_(hyperGeomUncSelectedPop),
        MP_(hyperGeomUncNumDrawn),
        MP_(negBinomialUncNumTrials),
	MP_(discreteDesignRangeLowerBnds),
	MP_(discreteDesignRangeUpperBnds),
	MP_(discreteDesignRangeVars),
	MP_(discreteDesignSetIntVars),
	MP_(discreteStateRangeLowerBnds),
	MP_(discreteStateRangeUpperBnds),
	MP_(discreteStateRangeVars),
	MP_(discreteStateSetIntVars);

static IntArray
	VP_(dsvi),
	VP_(ndsvi),
	VP_(ndsvr),
	VP_(nIv),
	VP_(nbp),
	VP_(npp),
	VP_(nssvi),
	VP_(nssvr),
	VP_(ssvi);

static RealVector
	MP_(betaUncLowerBnds),
	MP_(betaUncUpperBnds),
        MP_(binomialUncProbPerTrial),
        MP_(continuousDesignLowerBnds),
	MP_(continuousDesignUpperBnds),
	MP_(continuousDesignVars),
	MP_(continuousDesignScales),
	MP_(continuousStateLowerBnds),
	MP_(continuousStateUpperBnds),
	MP_(continuousStateVars),
	MP_(discreteDesignSetRealVars),
	MP_(discreteStateSetRealVars),
	MP_(frechetUncBetas),
        MP_(geometricUncProbPerTrial),
	MP_(gumbelUncBetas),
        MP_(negBinomialUncProbPerTrial),
	MP_(normalUncLowerBnds),
	MP_(normalUncMeans),
	MP_(normalUncUpperBnds),
        MP_(poissonUncLambdas),
	MP_(triangularUncModes),
	VP_(dsvr),
	VP_(Ivb),
	VP_(Ivp),
	VP_(ba),
	VP_(bo),
	VP_(bc),
	VP_(pa),
	VP_(pc),
	VP_(ucm),
	VP_(ssvr);

static String
	MP_(idVariables);

static StringArray
	MP_(continuousDesignLabels),
	MP_(continuousDesignScaleTypes),
	MP_(continuousStateLabels),
	MP_(discreteDesignRangeLabels),
	MP_(discreteDesignSetIntLabels),
	MP_(discreteDesignSetRealLabels),
	MP_(discreteStateRangeLabels),
	MP_(discreteStateSetIntLabels),
	MP_(discreteStateSetRealLabels);

static Var_bgen

	var_mp_bgen[] = {
		Vchu0(gamma_uncertain,numGammaUncVars,Gamma),
		Vchu0(gumbel_uncertain,numGumbelUncVars,Gumbel),
		Vchu0(frechet_uncertain,numFrechetUncVars,Frechet),
		Vchu0(weibull_uncertain,numWeibullUncVars,Weibull),
		Vchu0(histogram_bin_uncertain,numHistogramBinUncVars,HistogramBin)
		},

	var_mp_bgen_audr[] = {
		Vchu0(histogram_point_uncertain,numHistogramPtUncVars,HistogramPt)
		},

	var_mp_bgen_audi[] = {
		Vchu0(poisson_uncertain,numPoissonUncVars,Poisson),
		Vchu0(binomial_uncertain,numBinomialUncVars,Binomial),
		Vchu0(negative_binomial_uncertain,numNegBinomialUncVars,NegBinomial),
		Vchu0(geometric_uncertain,numGeometricUncVars,Geometric),
		Vchu0(hypergeometric_uncertain,numHyperGeomUncVars,HyperGeom)
		},

	var_mp_bgen_eu[] = {
		Vchu0(interval_uncertain,numIntervalUncVars,Interval)
		},

	var_mp_bgen_dis[] = {
		Vchu0(discrete_design_set_integer,numDiscreteDesSetIntVars,DDSI),
		Vchu0(discrete_design_set_real,numDiscreteDesSetRealVars,DDSR),
		Vchu0(discrete_state_set_integer,numDiscreteStateSetIntVars,DSSI),
		Vchu0(discrete_state_set_real,numDiscreteStateSetRealVars,DSSR)
		};

#define BgenInit(x) {x, Numberof(x)}
 static VarBgen Bgen[] = {
	BgenInit(var_mp_bgen_audr),
	BgenInit(var_mp_bgen_audi),
	BgenInit(var_mp_bgen_eu)};
#undef BgenInit

static Var_bchk var_mp_bndchk[] = {
	Vchv(continuous_design,numContinuousDesVars,continuousDesign),
	Vchu1(normal_uncertain,numNormalUncVars,normalUnc),
	Vchu1(lognormal_uncertain,numLognormalUncVars,lognormalUnc),
	Vchu(uniform_uncertain,numUniformUncVars,uniformUnc),
	Vchu(loguniform_uncertain,numLoguniformUncVars,loguniformUnc),
	Vchu(triangular_uncertain,numTriangularUncVars,triangularUnc),
	Vchu0(exponential_uncertain,numExponentialUncVars,Exponential),
	Vchu(beta_uncertain,numBetaUncVars,betaUnc),
	Vchv(continuous_state,numContinuousStateVars,continuousState)
	};

static Var_ibchk var_mp_ibndchk[] = {
	Vchi(discrete_design_range,numDiscreteDesRangeVars,discreteDesignRange),
	Vchi(discrete_state_range,numDiscreteStateRangeVars,discreteStateRange)
	};

static Var_brv //* change _ to 2s
	MP2s(betaUncAlphas,0.),
	MP2s(betaUncBetas,0.),
	MP2s(exponentialUncBetas,0.),
	MP2s(frechetUncAlphas,2.),
	MP2s(gammaUncAlphas,0.),
	MP2s(gammaUncBetas,0.),
	MP2s(gumbelUncAlphas,0.),
	MP2s(lognormalUncErrFacts,1.),
	MP2s(lognormalUncLambdas,0.),
	MP2s(lognormalUncLowerBnds,0.),
	MP2s(lognormalUncMeans,0.),
	MP2s(lognormalUncStdDevs,0.),
	MP2s(lognormalUncUpperBnds,DBL_MAX),
	MP2s(lognormalUncZetas,0.),
	MP2s(loguniformUncLowerBnds,0.),
	MP2s(loguniformUncUpperBnds,DBL_MAX),
	MP2s(normalUncStdDevs,0.),
	MP2s(triangularUncLowerBnds, -DBL_MAX),
	MP2s(triangularUncUpperBnds, DBL_MAX),
	MP2s(uniformUncLowerBnds, -DBL_MAX),
	MP2s(uniformUncUpperBnds, DBL_MAX),
	MP2s(weibullUncAlphas,0.),
	MP2s(weibullUncBetas,0.);

#undef VP_
#undef Vchi
#undef Vchv
#undef Vchu0
#undef Vchu
#undef MP2s
#undef MP_

 static const char *
Var_Name(StringArray *sa, char *buf, size_t i)
{
	if (sa)
		return (*sa)[i].data();
	std::sprintf(buf,"%lu", (UL)(i+1));
	return (const char*)buf;
	}

 void NIDRProblemDescDB::
Var_boundchk(DataVariablesRep *dv)
{
	Var_bchk *b, *be;
	RealVector *L, *U, *V;
	StringArray *sa;
	char namebuf[32];
	int i, n, ndflt;	// length() values here are int rather than size_t

	b = var_mp_bndchk;
	be = b + Numberof(var_mp_bndchk);
	for(; b < be; ++b) {
		if ((n = dv->*b->n) == 0)
			continue;
		ndflt = -1;
		if (b->L) {
			ndflt = 0;
			L = &(dv->*b->L);
			if (L->length() == 0)
				++ndflt;
			else if (L->length() != n) {
				squawk("%s_lower_bounds needs %lu elements, not %lu",
					b->name, (UL)n, (UL)L->length());
				continue;
				}
			U = &(dv->*b->U);
			if (U->length() == 0)
				++ndflt;
			else if (U->length() != n) {
				squawk("%s_upper_bounds needs %lu elements, not %lu",
					b->name, (UL)n, (UL)L->length());
				continue;
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
			  squawk("lower bound for %s variable %s exceeds upper bound",
					b->name, Var_Name(sa,namebuf,i));
			}
		if (b->V == 0)
			continue;
		V = &(dv->*b->V);
		if (V->length() == 0)
			continue;
		if (V->length() != n) {
			squawk("initial point for %s needs %lu elements, not %lu",
				b->name, (UL)n, (UL)V->length());
			continue;
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
	}

 void NIDRProblemDescDB::
Var_boundgen(DataVariablesRep *dv)
{
	Var_bchk *b, *be;
	Var_bgen *b1, *b1e;
	Real t;
	RealVector *L, *U, *V;
	size_t i, i0, n;

	b = var_mp_bndchk;
	be = b + Numberof(var_mp_bndchk);
	i0 = 0;
	for(; b < be; ++b) {
		if ((n = dv->*b->n) == 0)
			continue;
		L = U = 0;
		if (b->L && !b->no_init) {
			L = &(dv->*b->L);
			if (L->length() == 0)
				Set_rdv(L, -DBL_MAX, n);
			U = &(dv->*b->U);
			if (U->length() == 0)
				Set_rdv(U, DBL_MAX, n);
			}
		if (b->vbgen) {
			(*b->vbgen)(dv, i0);
			i0 += n;
			}
		if (b->V == 0)
			continue;
		V = &(dv->*b->V);
		if (V->length() == 0) {
			V->sizeUninitialized(n);
			for(i = 0; i < n; i++) {
				t = 0.;
				if ((*L)[i] > t)
					t = (*L)[i];
				else if ((*U)[i] < t)
					t = (*U)[i];
				(*V)[i] = t;
				}
			}
		}
	b1 = var_mp_bgen;
	b1e = b1 + Numberof(var_mp_bgen);
	for(; b1 < b1e; ++b1) {
		if ((n = dv->*b1->n) == 0)
			continue;
		(*b1->vbgen)(dv, i0);
		i0 += n;
		}
	}

 void NIDRProblemDescDB::
Var_iboundchk(DataVariablesRep *dv)
{
	Var_ibchk *b, *be;
	IntVector *L, *U, *V;
	StringArray *sa;
	char namebuf[32];
	int i, n, ndflt;	// length() values here are int rather than size_t

	b = var_mp_ibndchk;
	be = b + Numberof(var_mp_ibndchk);
	for(; b < be; ++b) {
		if ((n = dv->*b->n) == 0)
			continue;
		L = &(dv->*b->L);
		ndflt = 0;
		if (L->length() == 0)
			++ndflt;
		else if (L->length() != n) {
			squawk("%s_lower_bounds needs %lu elements, not %lu",
				b->name, (UL)n, (UL)L->length());
			continue;
			}
		U = &(dv->*b->U);
		if (U->length() == 0)
			++ndflt;
		else if (U->length() != n) {
			squawk("%s_upper_bounds needs %lu elements, not %lu",
				b->name, (UL)n, (UL)L->length());
			continue;
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
			  squawk("lower bound for %s variable %s exceeds upper bound",
					b->name, Var_Name(sa,namebuf,i));
			}
		if (b->V == 0)
			continue;	// won't happen for discrete variables
		V = &(dv->*b->V);
		if (V->length() == 0)
			continue;
		if (V->length() != n) {
			squawk("initial point for %s needs %lu elements, not %lu",
				b->name, (UL)n, (UL)V->length());
			continue;
			}
		if (L->length() > 0) {
		    for(i = 0; i < n; i++)
			if ((*V)[i] < (*L)[i]) {
				warn("Setting initial_value for %s variable %s to its lower bound",
					b->name, Var_Name(sa,namebuf,i));
				(*V)[i] = (*L)[i];
				}
			}
		if (U->length() > 0) {
		    for(i = 0; i < n; i++)
			if ((*V)[i] > (*U)[i]) {
				warn("Setting initial_value for %s variable %s to its upper bound",
					b->name, Var_Name(sa,namebuf,i));
				(*V)[i] = (*U)[i];
				}
			}
		}
	}

 void NIDRProblemDescDB::
Var_iboundgen(DataVariablesRep *dv)
{
	VarBgen *g, *ge;
	Var_bgen *b1, *b1e;
	Var_ibchk *b, *be;
	IntVector *L, *U, *V;
	int t;
	size_t i, j, n;

	b = var_mp_ibndchk;
	be = b + Numberof(var_mp_ibndchk);
	for(; b < be; ++b) {
		if ((n = dv->*b->n) == 0)
			continue;
		L = &(dv->*b->L);
		if (L->length() == 0) {
			L->sizeUninitialized(n);
			*L = INT_MIN;
			}
		U = &(dv->*b->U);
		if (U->length() == 0) {
			U->sizeUninitialized(n);
			*U = INT_MAX;
			}
		if (b->V == 0)
			continue;	// won't happen for discrete variables
		V = &(dv->*b->V);
		if (V->length() == 0) {
			V->sizeUninitialized(n);
			for(i = 0; i < n; i++) {
				t = 0;
				if ((*L)[i] > t)
					t = (*L)[i];
				else if ((*U)[i] < t)
					t = (*U)[i];
				(*V)[i] = t;
				}
			}
		}

	ge = Bgen + Numberof(Bgen);
	for(g = Bgen; g < ge; ++g) {
		b1 = g->bgen;
		b1e = b1 + g->nbgen;
		j = 0;
		for(; b1 < b1e; ++b1)
			if ((n = dv->*b1->n) > 0) {
				(*b1->vbgen)(dv, j);
				j += n;
				}
		}

	b1e = var_mp_bgen_dis + Numberof(var_mp_bgen_dis);
	for(b1 = var_mp_bgen_dis; b1 < b1e; ++b1)
		if ((n = dv->*b1->n) > 0)
				(*b1->vbgen)(dv, n);
	}

NIDRProblemDescDB::~NIDRProblemDescDB()
{
#ifndef NO_NIDR_DYNLIB
	nidr_lib_cleanup();	// close any explicitly opened shared libraries
#endif
	}

} // namespace Dakota

#undef Warn
#undef Squawk

#define N_ifm(x,y)	NIDRProblemDescDB::iface_##x,&iface_mp_##y
#define N_ifm3(x,y,z)	NIDRProblemDescDB::iface_##x,y,NIDRProblemDescDB::iface_##z
#define N_mdm(x,y)	NIDRProblemDescDB::method_##x,&method_mp_##y
#define N_mdf(x,y)	N_mdm(x,y),NIDRProblemDescDB::method_##x##_final
#define N_mdm3(x,y,z)	NIDRProblemDescDB::method_##x,y,NIDRProblemDescDB::method_##z
#define N_mom(x,y)	NIDRProblemDescDB::model_##x,&model_mp_##y
#define N_mof(x,y)	N_mom(x,y),NIDRProblemDescDB::model_##x##_final
#define N_mom3(x,y,z)	NIDRProblemDescDB::model_##x,y,NIDRProblemDescDB::model_##z
#define N_rem(x,y)	NIDRProblemDescDB::resp_##x,&resp_mp_##y
#define N_rem3(x,y,z)	NIDRProblemDescDB::resp_##x,y,NIDRProblemDescDB::resp_##z
#define N_stm(x,y)	NIDRProblemDescDB::strategy_##x,&strategy_mp_##y
#define N_vae(x,y)	NIDRProblemDescDB::var_##x,(void*)y
#define N_vam(x,y)	NIDRProblemDescDB::var_##x,&var_mp_##y
#define N_vam3(x,y,z)	NIDRProblemDescDB::var_##x,y,NIDRProblemDescDB::var_##z

#include "NIDR_keywds.H"
