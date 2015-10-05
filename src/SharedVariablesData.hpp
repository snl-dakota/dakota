/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SharedVariablesData
//- Description: Encapsulation of data that can be shared among Variables
//-              instances.
//- Owner:       Mike Eldred
//- Version: $Id: SharedVariablesData.hpp 6784 2010-05-18 21:10:35Z mseldre $

#ifndef SHARED_VARIABLES_DATA_H
#define SHARED_VARIABLES_DATA_H

#include "dakota_data_types.hpp"
#include "DataVariables.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>

namespace Dakota {

// forward declarations
class ProblemDescDB;


/// The representation of a SharedVariablesData instance.  This representation,
/// or body, may be shared by multiple SharedVariablesData handle instances.

/** The SharedVariablesData/SharedVariablesDataRep pairs utilize a
    handle-body idiom (Coplien, Advanced C++). */

class SharedVariablesDataRep
{
  //
  //- Heading: Friends
  //

  friend class SharedVariablesData;

  /// allow boost access to serialize this class
  friend class boost::serialization::access;

public:

  /// destructor must be public for shared_ptr
  ~SharedVariablesDataRep();

private:

  //
  //- Heading: Constructor and destructor
  //

  /// standard constructor
  SharedVariablesDataRep(const ProblemDescDB& problem_db,
			 const std::pair<short,short>& view);
  /// medium weight constructor providing detailed variable counts
  SharedVariablesDataRep(const std::pair<short,short>& view,
			 const std::map<unsigned short, size_t>& vars_comps,
			 const BitArray& all_relax_di,
			 const BitArray& all_relax_dr);
  /// lightweight constructor providing variable count totals
  SharedVariablesDataRep(const std::pair<short,short>& view,
			 const SizetArray& vars_comps_totals,
			 const BitArray& all_relax_di,
			 const BitArray& all_relax_dr);
  /// default constructor
  SharedVariablesDataRep();

  //
  //- Heading: Member functions
  //

  /// populate variables{Components,CompsTotals} from user variable
  /// type and count specifications
  void initialize_components_totals(const ProblemDescDB& problem_db);
  /// update variablesCompsTotals from variablesComponents
  void components_to_totals();

  /// populate allRelaxedDiscrete{Int,Real} from user specifications
  /// (relax variables that are not declared as categorical)
  void relax_noncategorical(const ProblemDescDB& problem_db);

  /// Set the ard_cntr entry in the all-relaxed-discrete integer or
  /// real container ard_container, based on user-specification of
  /// categorical, accounting for empty
  void set_relax(const BitArray& user_cat_spec, size_t ucs_index,
		 size_t ard_cntr, BitArray& ard_container);
 
  /// compute all variables sums from variablesCompsTotals
  void all_counts(size_t& num_acv, size_t& num_adiv, size_t& num_adsv,
		  size_t& num_adrv) const;
  /// adjust counts based on allRelaxedDiscrete{Int,Real}
  void relax_counts(size_t& num_cv, size_t& num_div, size_t& num_drv,
		    size_t offset_di, size_t offset_dr) const;
  /// compute design variables sums from variablesCompsTotals
  void design_counts(size_t& num_cdv, size_t& num_ddiv, size_t& num_ddsv,
		     size_t& num_ddrv) const;
  /// compute aleatory uncertain variables sums from variablesCompsTotals
  void aleatory_uncertain_counts(size_t& num_cauv,  size_t& num_dauiv,
				 size_t& num_dausv, size_t& num_daurv) const;
  /// compute epistemic uncertain variables sums from variablesCompsTotals
  void epistemic_uncertain_counts(size_t& num_ceuv,  size_t& num_deuiv,
				  size_t& num_deusv, size_t& num_deurv) const;
  /// compute uncertain variables sums from variablesCompsTotals
  void uncertain_counts(size_t& num_cuv,  size_t& num_duiv,
			size_t& num_dusv, size_t& num_durv) const;
  /// compute state variables sums from variablesCompsTotals
  void state_counts(size_t& num_csv,  size_t& num_dsiv,
		    size_t& num_dssv, size_t& num_dsrv) const;

  /// define start indices and counts for active variables based on view
  void view_start_counts(short view, size_t& cv_start, size_t& div_start,
			 size_t& dsv_start, size_t& drv_start,
			 size_t& num_cv, size_t& num_div, size_t& num_dsv,
			 size_t& num_drv) const;

  /// size all{Continuous,DiscreteInt,DiscreteString,DiscreteReal}Labels,
  /// with or without discrete relaxation
  void size_all_labels();
  /// size all{Continuous,DiscreteInt,DiscreteString,DiscreteReal}Types,
  /// with or without discrete relaxation
  void size_all_types();

  /// aggregate all{Continuous,DiscreteInt,DiscreteString,DiscreteReal}Labels
  /// from user specification or defaults
  void initialize_all_labels(const ProblemDescDB& problem_db);
  /// initialize all{Continuous,DiscreteInt,DiscreteString,DiscreteReal}Types,
  /// with or without discrete relaxation
  void initialize_all_types();
  /// initialize allContinuousIds (discrete not currently needed),
  /// with or without discrete relaxation
  void initialize_all_ids();

  /// initialize {c,di,ds,dr}vStart and num{D,DI,DS,DR}V
  void initialize_active_start_counts();
  /// initialize i{c,di,ds,dr}vStart and numI{D,DI,DS,DR}V
  void initialize_inactive_start_counts();

  /// initialize activeVarsCompsTotals given {c,di,dr}vStart and num{C,DI,DR}V
  void initialize_active_components();
  /// initialize inactiveVarsCompsTotals given i{c,di,dr}vStart and
  /// numI{C,DI,DR}V
  void initialize_inactive_components();

  /// retrieve the count within variablesComponents corresponding to key
  size_t vc_lookup(unsigned short key) const;

  /// copy the data from svd_rep to the current representation
  void copy_rep(SharedVariablesDataRep* svd_rep);

  /// serialize the core shared variables data
  template<class Archive>
  void save(Archive& ar, const unsigned int version) const;

  /// load the core shared variables data and restore class state
  template<class Archive>
  void load(Archive& ar, const unsigned int version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()


  //
  //- Heading: Data
  //

  /// variables identifier string from the input file
  String variablesId;

  /// map linking variable types to counts
  std::map<unsigned short, size_t> variablesComponents;
  /// totals for variable type counts for
  /// {continuous,discrete integer,discrete string,discrete real}
  /// {design,aleatory uncertain,epistemic uncertain,state}.
  /** This data reflects the variable counts as originally specified
      and is not altered by relaxation. */
  SizetArray variablesCompsTotals;
  /// totals for active variable type counts for
  /// {continuous,discrete integer,discrete string,discrete real}
  /// {design,aleatory uncertain,epistemic uncertain,state}.
  /** This data reflects the variable counts as originally specified
      and is not altered by relaxation. */
  SizetArray activeVarsCompsTotals;
  /// totals for inactive variable type counts for
  /// {continuous,discrete integer,discrete string,discrete real}
  /// {design,aleatory uncertain,epistemic uncertain,state}.
  /** This data reflects the variable counts as originally specified
      and is not altered by relaxation. */
  SizetArray inactiveVarsCompsTotals;

  /// the variables view pair containing active (first) and inactive (second)
  /// view enumerations
  std::pair<short,short> variablesView;
  /// start index of active continuous variables within allContinuousVars
  size_t cvStart;
  /// start index of active discrete integer variables within allDiscreteIntVars
  size_t divStart;
  /// start index of active discrete string vars within allDiscreteStringVars
  size_t dsvStart;
  /// start index of active discrete real variables within allDiscreteRealVars
  size_t drvStart;
  /// start index of inactive continuous variables within allContinuousVars
  size_t icvStart;
  /// start index of inactive discrete integer vars within allDiscreteIntVars
  size_t idivStart;
  /// start index of inactive discrete string vars within allDiscreteStringVars
  size_t idsvStart;
  /// start index of inactive discrete real variables within allDiscreteRealVars
  size_t idrvStart;
  size_t numCV;   ///< number of active continuous variables
  size_t numDIV;  ///< number of active discrete integer variables
  size_t numDSV;  ///< number of active discrete string variables
  size_t numDRV;  ///< number of active discrete real variables
  size_t numICV;  ///< number of inactive continuous variables
  size_t numIDIV; ///< number of inactive discrete integer variables
  size_t numIDSV; ///< number of inactive discrete string variables
  size_t numIDRV; ///< number of inactive discrete real variables

  /// array of variable labels for all of the continuous variables
  StringMultiArray allContinuousLabels;
  /// array of variable labels for all of the discrete integer variables
  StringMultiArray allDiscreteIntLabels;
  /// array of variable labels for all of the discrete string variables
  StringMultiArray allDiscreteStringLabels;
  /// array of variable labels for all of the discrete real variables
  StringMultiArray allDiscreteRealLabels;

  /// array of variable types for all of the continuous variables
  UShortMultiArray allContinuousTypes;
  /// array of variable types for all of the discrete integer variables
  UShortMultiArray allDiscreteIntTypes;
  /// array of variable types for all of the discrete string variables
  UShortMultiArray allDiscreteStringTypes;
  /// array of variable types for all of the discrete real variables
  UShortMultiArray allDiscreteRealTypes;

  /// array of 1-based position identifiers for the all continuous
  /// variables array
  /** These identifiers define positions of the all continuous variables
      array within the total variable sequence.  A primary use case is
      for defining derivative ids (DVV) based on an active subset. */
  SizetMultiArray allContinuousIds;

  /// array of booleans to indicate relaxation (promotion from
  /// DiscreteInt to Continuous) for all specified discrete int variables
  /// Note: container will be empty when not relaxing variables
  BitArray allRelaxedDiscreteInt;
  /// array of booleans to indicate relaxation (promotion from
  /// DiscreteReal to Continuous) for all specified discrete real variables
  /// Note: container will be empty when not relaxing variables
  BitArray allRelaxedDiscreteReal;

};


inline SharedVariablesDataRep::SharedVariablesDataRep():
  cvStart(0), divStart(0), dsvStart(0), drvStart(0), icvStart(0), idivStart(0),
  idsvStart(0), idrvStart(0), numCV(0), numDIV(0), numDSV(0), numDRV(0),
  numICV(0), numIDIV(0), numIDSV(0), numIDRV(0)
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesDataRep::SharedVariablesDataRep() called to build "
       << "empty body object." << std::endl;
#endif
}


inline SharedVariablesDataRep::~SharedVariablesDataRep()
{ }


inline size_t SharedVariablesDataRep::vc_lookup(unsigned short key) const
{
  std::map<unsigned short, size_t>::const_iterator cit
    = variablesComponents.find(key);
  return (cit == variablesComponents.end()) ? 0 : cit->second;
}


inline void SharedVariablesDataRep::
all_counts(size_t& num_acv, size_t& num_adiv, size_t& num_adsv,
	   size_t& num_adrv) const
{
  num_acv = variablesCompsTotals[TOTAL_CDV]  + variablesCompsTotals[TOTAL_CAUV]
          + variablesCompsTotals[TOTAL_CEUV] + variablesCompsTotals[TOTAL_CSV];
  // num_adsv is always categorical
  // num_adiv and num_adrv are updated below for relaxed non-categorical
  num_adiv
    = variablesCompsTotals[TOTAL_DDIV]  + variablesCompsTotals[TOTAL_DAUIV]
    + variablesCompsTotals[TOTAL_DEUIV] + variablesCompsTotals[TOTAL_DSIV];
  num_adsv
    = variablesCompsTotals[TOTAL_DDSV]  + variablesCompsTotals[TOTAL_DAUSV]
    + variablesCompsTotals[TOTAL_DEUSV] + variablesCompsTotals[TOTAL_DSSV];
  num_adrv
    = variablesCompsTotals[TOTAL_DDRV]  + variablesCompsTotals[TOTAL_DAURV]
    + variablesCompsTotals[TOTAL_DEURV] + variablesCompsTotals[TOTAL_DSRV];

  if (allRelaxedDiscreteInt.any() || allRelaxedDiscreteReal.any()) {
    size_t num_relax_int  = allRelaxedDiscreteInt.count(),
           num_relax_real = allRelaxedDiscreteReal.count();
    num_acv  += num_relax_int + num_relax_real;
    num_adiv -= num_relax_int;
    num_adrv -= num_relax_real;
  }
}


inline void SharedVariablesDataRep::
relax_counts(size_t& num_cv, size_t& num_div, size_t& num_drv,
	     size_t offset_di, size_t offset_dr) const
{
  size_t i, num_relax_int  = 0, num_relax_real = 0,
    end_di = num_div + offset_di, end_dr = num_drv + offset_dr;
  for (i=offset_di; i<end_di; ++i)
    if (allRelaxedDiscreteInt[i])
      ++num_relax_int;
  for (i=offset_dr; i<end_dr; ++i)
    if (allRelaxedDiscreteReal[i])
      ++num_relax_real;
  num_cv  += num_relax_int + num_relax_real;
  num_div -= num_relax_int;
  num_drv -= num_relax_real;
}


inline void SharedVariablesDataRep::
design_counts(size_t& num_cdv, size_t& num_ddiv, size_t& num_ddsv,
	      size_t& num_ddrv) const
{
  num_cdv  = variablesCompsTotals[TOTAL_CDV];
  num_ddiv = variablesCompsTotals[TOTAL_DDIV];
  num_ddsv = variablesCompsTotals[TOTAL_DDSV];
  num_ddrv = variablesCompsTotals[TOTAL_DDRV];

  if (allRelaxedDiscreteInt.any() || allRelaxedDiscreteReal.any())
    relax_counts(num_cdv, num_ddiv, num_ddrv, 0, 0);
}


inline void SharedVariablesDataRep::
aleatory_uncertain_counts(size_t& num_cauv,  size_t& num_dauiv,
			  size_t& num_dausv, size_t& num_daurv) const
{
  num_cauv  = variablesCompsTotals[TOTAL_CAUV];
  num_dauiv = variablesCompsTotals[TOTAL_DAUIV];
  num_dausv = variablesCompsTotals[TOTAL_DAUSV];
  num_daurv = variablesCompsTotals[TOTAL_DAURV];

  if (allRelaxedDiscreteInt.any() || allRelaxedDiscreteReal.any())
    relax_counts(num_cauv, num_dauiv, num_daurv,
		 variablesCompsTotals[TOTAL_DDIV],
		 variablesCompsTotals[TOTAL_DDRV]);
}


inline void SharedVariablesDataRep::
epistemic_uncertain_counts(size_t& num_ceuv,  size_t& num_deuiv,
			   size_t& num_deusv, size_t& num_deurv) const
{
  num_ceuv  = variablesCompsTotals[TOTAL_CEUV];
  num_deuiv = variablesCompsTotals[TOTAL_DEUIV];
  num_deusv = variablesCompsTotals[TOTAL_DEUSV];
  num_deurv = variablesCompsTotals[TOTAL_DEURV];

  if (allRelaxedDiscreteInt.any() || allRelaxedDiscreteReal.any())
    relax_counts(num_ceuv, num_deuiv, num_deurv,
		 variablesCompsTotals[TOTAL_DDIV] +
		 variablesCompsTotals[TOTAL_DAUIV],
		 variablesCompsTotals[TOTAL_DDRV] +
		 variablesCompsTotals[TOTAL_DAURV]);
}


inline void SharedVariablesDataRep::
uncertain_counts(size_t& num_cuv,  size_t& num_duiv,
		 size_t& num_dusv, size_t& num_durv) const
{
  num_cuv  = variablesCompsTotals[TOTAL_CAUV]
           + variablesCompsTotals[TOTAL_CEUV];
  num_duiv = variablesCompsTotals[TOTAL_DAUIV]
           + variablesCompsTotals[TOTAL_DEUIV];
  num_dusv = variablesCompsTotals[TOTAL_DAUSV]
           + variablesCompsTotals[TOTAL_DEUSV];
  num_durv = variablesCompsTotals[TOTAL_DAURV]
           + variablesCompsTotals[TOTAL_DEURV];

  if (allRelaxedDiscreteInt.any() || allRelaxedDiscreteReal.any())
    relax_counts(num_cuv, num_duiv, num_durv, variablesCompsTotals[TOTAL_DDIV],
		 variablesCompsTotals[TOTAL_DDRV]);
}


inline void SharedVariablesDataRep::
state_counts(size_t& num_csv,  size_t& num_dsiv,
	     size_t& num_dssv, size_t& num_dsrv) const
{
  num_csv  = variablesCompsTotals[TOTAL_CSV];
  num_dsiv = variablesCompsTotals[TOTAL_DSIV];
  num_dssv = variablesCompsTotals[TOTAL_DSSV];
  num_dsrv = variablesCompsTotals[TOTAL_DSRV];

  if (allRelaxedDiscreteInt.any() || allRelaxedDiscreteReal.any())
    relax_counts(num_csv, num_dsiv, num_dsrv, variablesCompsTotals[TOTAL_DDIV] +
		 variablesCompsTotals[TOTAL_DAUIV] +
		 variablesCompsTotals[TOTAL_DEUIV],
		 variablesCompsTotals[TOTAL_DDRV]  +
		 variablesCompsTotals[TOTAL_DAURV] +
		 variablesCompsTotals[TOTAL_DEURV]);
}


inline void SharedVariablesDataRep::size_all_labels()
{
  size_t num_acv, num_adiv, num_adsv, num_adrv;
  all_counts(num_acv, num_adiv, num_adsv, num_adrv);

  allContinuousLabels.resize(boost::extents[num_acv]);    // updated size
  allDiscreteIntLabels.resize(boost::extents[num_adiv]);  // updated size
  allDiscreteStringLabels.resize(boost::extents[num_adsv]);
  allDiscreteRealLabels.resize(boost::extents[num_adrv]); // updated size
}


inline void SharedVariablesDataRep::size_all_types()
{
  size_t num_acv, num_adiv, num_adsv, num_adrv;
  all_counts(num_acv, num_adiv, num_adsv, num_adrv);

  allContinuousTypes.resize(boost::extents[num_acv]);
  allDiscreteIntTypes.resize(boost::extents[num_adiv]);
  allDiscreteStringTypes.resize(boost::extents[num_adsv]);
  allDiscreteRealTypes.resize(boost::extents[num_adrv]);
}


inline void SharedVariablesDataRep::initialize_active_start_counts()
{
  view_start_counts(variablesView.first, cvStart, divStart, dsvStart, drvStart,
		    numCV, numDIV, numDSV, numDRV);
}


inline void SharedVariablesDataRep::initialize_inactive_start_counts()
{
  view_start_counts(variablesView.second, icvStart, idivStart, idsvStart,
		    idrvStart, numICV, numIDIV, numIDSV, numIDRV);
}


/// Container class encapsulating variables data that can be shared
/// among a set of Variables instances.

/** An array of Variables objects (e.g., Analyzer::allVariables) contains
    repeated configuration data (id's, labels, counts).  SharedVariablesData 
    employs a handle-body idiom to allow this shared data to be managed 
    in a single object with many references to it, one per Variables 
    object in the array.  This allows scaling to larger sample sets. */

class SharedVariablesData
{
public:

  //
  //- Heading: Friends
  //

  /// allow boost access to serialize this class
  friend class boost::serialization::access;


  //
  //- Heading: Constructors, destructor, and operators
  //

  /// default constructor
  SharedVariablesData();
  /// standard constructor
  SharedVariablesData(const ProblemDescDB& problem_db,
		      const std::pair<short,short>& view);
  /// medium weight constructor providing detailed variable counts
  SharedVariablesData(const std::pair<short,short>& view,
		      const std::map<unsigned short, size_t>& vars_comps,
		      const BitArray& all_relax_di = BitArray(),
		      const BitArray& all_relax_dr = BitArray());
  /// lightweight constructor providing variable count totals
  SharedVariablesData(const std::pair<short,short>& view,
		      const SizetArray& vars_comps_totals,
		      const BitArray& all_relax_di = BitArray(),
		      const BitArray& all_relax_dr = BitArray());
  /// copy constructor
  SharedVariablesData(const SharedVariablesData& svd);
  /// destructor
  ~SharedVariablesData();

  /// assignment operator
  SharedVariablesData& operator=(const SharedVariablesData& svd);

  //
  //- Heading: member functions
  //

  /// create a deep copy of the current object and return by value
  SharedVariablesData copy() const;

  /// compute all variables sums from
  /// SharedVariablesDataRep::variablesCompsTotals and
  /// SharedVariablesDataRep::allRelaxedDiscrete{Int,Real}
  void all_counts(size_t& num_acv, size_t& num_adiv, size_t& num_adsv,
		  size_t& num_adrv) const;
  /// compute design variables sums from
  /// SharedVariablesDataRep::variablesCompsTotals and
  /// SharedVariablesDataRep::allRelaxedDiscrete{Int,Real}
  void design_counts(size_t& num_cdv, size_t& num_ddiv, size_t& num_ddsv,
		     size_t& num_ddrv) const;
  /// compute aleatory uncertain variables sums from
  /// SharedVariablesDataRep::variablesCompsTotals and
  /// SharedVariablesDataRep::allRelaxedDiscrete{Int,Real}
  void aleatory_uncertain_counts(size_t& num_cauv,  size_t& num_dauiv,
				 size_t& num_dausv, size_t& num_daurv) const;
  /// compute epistemic uncertain variables sums from
  /// SharedVariablesDataRep::variablesCompsTotals and
  /// SharedVariablesDataRep::allRelaxedDiscrete{Int,Real}
  void epistemic_uncertain_counts(size_t& num_ceuv,  size_t& num_deuiv,
				  size_t& num_deusv, size_t& num_deurv) const;
  /// compute uncertain variables sums from
  /// SharedVariablesDataRep::variablesCompsTotals and
  /// SharedVariablesDataRep::allRelaxedDiscrete{Int,Real}
  void uncertain_counts(size_t& num_cuv,  size_t& num_duiv,
			size_t& num_dusv, size_t& num_durv) const;
  /// compute state variables sums from
  /// SharedVariablesDataRep::variablesCompsTotals and
  /// SharedVariablesDataRep::allRelaxedDiscrete{Int,Real}
  void state_counts(size_t& num_csv,  size_t& num_dsiv,
		    size_t& num_dssv, size_t& num_dsrv) const;

  /// initialize start index and counts for active variables
  void initialize_active_start_counts();
  /// initialize start index and counts for inactive variables
  void initialize_inactive_start_counts();

  /// initialize the active components totals given active variable counts
  void initialize_active_components();
  /// initialize the inactive components totals given inactive variable counts
  void initialize_inactive_components();

  /// return allRelaxedDiscreteInt
  const BitArray& all_relaxed_discrete_int() const;
  /// return allRelaxedDiscreteReal
  const BitArray& all_relaxed_discrete_real() const;

  /// get num_items continuous labels beginning at index start 
  StringMultiArrayView
    all_continuous_labels(size_t start, size_t num_items) const;
  /// set num_items continuous labels beginning at index start 
  void all_continuous_labels(StringMultiArrayConstView cv_labels, size_t start,
			     size_t num_items);
  /// set continuous label at index start 
  void all_continuous_label(const String& cv_label, size_t index);

  /// get num_items discrete integer labels beginning at index start 
  StringMultiArrayView
    all_discrete_int_labels(size_t start, size_t num_items) const;
  /// set num_items discrete integer labels beginning at index start 
  void all_discrete_int_labels(StringMultiArrayConstView div_labels,
			       size_t start, size_t num_items);
  /// set discrete integer label at index start 
  void all_discrete_int_label(const String& div_label, size_t index);

  /// get num_items discrete string labels beginning at index start 
  StringMultiArrayView
    all_discrete_string_labels(size_t start, size_t num_items) const;
  /// set num_items discrete string labels beginning at index start 
  void all_discrete_string_labels(StringMultiArrayConstView dsv_labels,
				  size_t start, size_t num_items);
  /// set discrete string label at index start 
  void all_discrete_string_label(const String& dsv_label, size_t index);

  /// get num_items discrete real labels beginning at index start 
  StringMultiArrayView
    all_discrete_real_labels(size_t start, size_t num_items) const;
  /// set num_items discrete real labels beginning at index start 
  void all_discrete_real_labels(StringMultiArrayConstView drv_labels,
				size_t start, size_t num_items);
  /// set discrete real label at index start 
  void all_discrete_real_label(const String& drv_label, size_t index);

  /// get num_items continuous types beginning at index start
  UShortMultiArrayConstView
    all_continuous_types(size_t start, size_t num_items) const;
  /// set num_items continuous types beginning at index start
  void all_continuous_types(UShortMultiArrayConstView cv_types,
			    size_t start, size_t num_items);
  /// set continuous type at index
  void all_continuous_type(unsigned short cv_type, size_t index);

  /// get num_items discrete integer types beginning at index start
  UShortMultiArrayConstView
    all_discrete_int_types(size_t start, size_t num_items) const;
  /// set num_items discrete integer types beginning at index start
  void all_discrete_int_types(UShortMultiArrayConstView div_types,
			      size_t start, size_t num_items);
  /// set discrete integer type at index
  void all_discrete_int_type(unsigned short div_type, size_t index);

  /// get num_items discrete string types beginning at index start
  UShortMultiArrayConstView
    all_discrete_string_types(size_t start, size_t num_items) const;
  /// set num_items discrete string types beginning at index start
  void all_discrete_string_types(UShortMultiArrayConstView dsv_types,
			       size_t start, size_t num_items);
  /// set discrete string type at index
  void all_discrete_string_type(unsigned short dsv_type, size_t index);

  /// get num_items discrete real types beginning at index start
  UShortMultiArrayConstView
    all_discrete_real_types(size_t start, size_t num_items) const;
  /// set num_items discrete real types beginning at index start
  void all_discrete_real_types(UShortMultiArrayConstView drv_types,
			       size_t start, size_t num_items);
  /// set discrete real type at index
  void all_discrete_real_type(unsigned short drv_type, size_t index);

  /// get num_items continuous ids beginning at index start
  SizetMultiArrayConstView
    all_continuous_ids(size_t start, size_t num_items) const;
  /// set num_items continuous ids beginning at index start
  void all_continuous_ids(SizetMultiArrayConstView cv_ids,
			  size_t start, size_t num_items);
  /// set num_items continuous ids beginning at index start
  void all_continuous_id(size_t id, size_t index);

  // get ids of discrete variables that have been relaxed into
  // continuous variable arrays
  //const SizetArray& relaxed_discrete_ids() const;

  /// return the user-provided or default Variables identifier
  const String& id() const;

  /// return variable type counts for
  /// {continuous,discrete integer,discrete real}
  /// {design,aleatory uncertain,epistemic uncertain,state}
  const SizetArray& components_totals() const;
  /// return active variable type counts for
  /// {continuous,discrete integer,discrete real}
  /// {design,aleatory uncertain,epistemic uncertain,state}
  const SizetArray& active_components_totals() const;
  /// return inactive variable type counts for
  /// {continuous,discrete integer,discrete real}
  /// {design,aleatory uncertain,epistemic uncertain,state}
  const SizetArray& inactive_components_totals() const;

  /// retrieve the variables type count within
  /// svdRep->variablesComponents corresponding to (a fine-grain
  /// variables type) key
  size_t vc_lookup(unsigned short key) const;

  /// retreive the Variables view
  const std::pair<short,short>& view() const;
  /// set the inactive Variables view
  void inactive_view(short view2);

  size_t cv()         const; ///< get number of active continuous vars
  size_t cv_start()   const; ///< get start index of active continuous vars
  size_t div()        const; ///< get number of active discrete int vars
  size_t div_start()  const; ///< get start index of active discrete int vars
  size_t dsv()        const; ///< get number of active discrete string vars
  size_t dsv_start()  const; ///< get start index of active discrete string vars
  size_t drv()        const; ///< get number of active discrete real vars
  size_t drv_start()  const; ///< get start index of active discrete real vars
  size_t icv()        const; ///< get number of inactive continuous vars
  size_t icv_start()  const; ///< get start index of inactive continuous vars
  size_t idiv()       const; ///< get number of inactive discrete int vars
  size_t idiv_start() const; ///< get start index of inactive discrete int vars
  size_t idsv()       const; ///< get number of inactive discrete string vars
  size_t idsv_start() const; ///< get start index of inactive discr string vars
  size_t idrv()       const; ///< get number of inactive discrete real vars
  size_t idrv_start() const; ///< get start index of inactive discrete real vars

  void cv(size_t ncv);         ///< set number of active continuous vars
  void cv_start(size_t cvs);   ///< set start index of active continuous vars
  void div(size_t ndiv);       ///< set number of active discrete int vars
  void div_start(size_t divs); ///< set start index of active discrete int vars
  void dsv(size_t ndsv);       ///< set number of active discrete string vars
  void dsv_start(size_t dsvs); ///< set start index of active discr string vars
  void drv(size_t ndrv);       ///< set number of active discrete real vars
  void drv_start(size_t drvs); ///< set start index of active discrete real vars
  void icv(size_t nicv);       ///< set number of inactive continuous vars
  void icv_start(size_t icvs); ///< set start index of inactive continuous vars
  void idiv(size_t nidiv);     ///< set number of inactive discrete int vars
  void idiv_start(size_t idivs); ///< set start index of inactive discr int vars
  void idsv(size_t nidsv);       ///< set number of inactive discr string vars
  void idsv_start(size_t idsvs); ///< set start index of inact discr string vars
  void idrv(size_t nidrv);       ///< set number of inactive discrete real vars
  void idrv_start(size_t idrvs); ///< set start index of inact discr real vars

private:

  /// serialize through the pointer, which requires object tracking:
  /// write and read are symmetric for this class
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version);

  //
  //- Heading: Private data members
  //
 
  /// pointer to the body (handle-body idiom)
  boost::shared_ptr<SharedVariablesDataRep> svdRep;
};


inline SharedVariablesData::SharedVariablesData()
{ 
#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesData::SVD(); default constructed handle.\n";
  Cout << "  svdRep use_count = " << svdRep.use_count() << std::endl;
#endif
}


inline SharedVariablesData::
SharedVariablesData(const ProblemDescDB& problem_db,
		    const std::pair<short,short>& view):
  svdRep(new SharedVariablesDataRep(problem_db, view))
{ 
#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesData::SVD(PDB) problem DB constructed handle.\n";
  Cout << "  svdRep use_count = " << svdRep.use_count() << std::endl;
#endif
}


inline SharedVariablesData::
SharedVariablesData(const std::pair<short,short>& view,
		    const std::map<unsigned short, size_t>& vars_comps,
		    const BitArray& all_relax_di, const BitArray& all_relax_dr):
  svdRep(new SharedVariablesDataRep(view, vars_comps, all_relax_di,
				    all_relax_dr))
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesData::SVD() vars_comps constructed handle.\n";
  Cout << "  svdRep use_count = " << svdRep.use_count() << std::endl;
#endif
}


inline SharedVariablesData::
SharedVariablesData(const std::pair<short,short>& view,
		    const SizetArray& vars_comps_totals,
		    const BitArray& all_relax_di, const BitArray& all_relax_dr):
  svdRep(new SharedVariablesDataRep(view, vars_comps_totals, all_relax_di,
				    all_relax_dr))
{ 
#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesData::SVD() vars_comps_totals constructed handle.\n";
  Cout << "  svdRep use_count = " << svdRep.use_count() << std::endl;
#endif
}


inline SharedVariablesData::SharedVariablesData(const SharedVariablesData& svd)
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesData::SVD(SVD) copy constructor.\n";
  Cout << "  svdRep use_count before = " << svdRep.use_count() << std::endl;
#endif
  // share the representation (body)
  svdRep = svd.svdRep;
#ifdef REFCOUNT_DEBUG
  Cout << "  svdRep use_count after  = " << svdRep.use_count() << std::endl;
#endif
}


inline SharedVariablesData& SharedVariablesData::
operator=(const SharedVariablesData& svd)
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesData::operator=.\n";
  Cout << "  svdRep use_count before = " << svdRep.use_count() << std::endl;
#endif
  // share the inbound representation (body) by copying the pointer
  svdRep = svd.svdRep;
#ifdef REFCOUNT_DEBUG
  Cout << "  svdRep use_count after  = " << svdRep.use_count() << std::endl;
#endif
  return *this;
}


inline SharedVariablesData::~SharedVariablesData()
{
  /* empty dtor in case we add virtual functions */
#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesData::~SVD called.\n";
  Cout << "  svdRep use_count = " << svdRep.use_count() << std::endl;
#endif
}


inline void SharedVariablesData::
all_counts(size_t& num_acv, size_t& num_adiv, size_t& num_adsv,
	   size_t& num_adrv) const
{ svdRep->all_counts(num_acv, num_adiv, num_adsv, num_adrv); }


inline void SharedVariablesData::
design_counts(size_t& num_cdv, size_t& num_ddiv, size_t& num_ddsv,
	      size_t& num_ddrv) const
{ svdRep->design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv); }


inline void SharedVariablesData::
aleatory_uncertain_counts(size_t& num_cauv,  size_t& num_dauiv,
			  size_t& num_dausv, size_t& num_daurv) const
{
  svdRep->aleatory_uncertain_counts(num_cauv, num_dauiv, num_dausv, num_daurv);
}


inline void SharedVariablesData::
epistemic_uncertain_counts(size_t& num_ceuv,  size_t& num_deuiv,
			   size_t& num_deusv, size_t& num_deurv) const
{
  svdRep->epistemic_uncertain_counts(num_ceuv, num_deuiv, num_deusv, num_deurv);
}


inline void SharedVariablesData::
uncertain_counts(size_t& num_cuv, size_t& num_duiv, size_t& num_dusv,
		 size_t& num_durv) const
{ svdRep->uncertain_counts(num_cuv, num_duiv, num_dusv, num_durv); }


inline void SharedVariablesData::
state_counts(size_t& num_csv, size_t& num_dsiv, size_t& num_dssv,
	     size_t& num_dsrv) const
{ svdRep->state_counts(num_csv, num_dsiv, num_dssv, num_dsrv); }


inline void SharedVariablesData::initialize_active_start_counts()
{ svdRep->initialize_active_start_counts(); }


inline void SharedVariablesData::initialize_inactive_start_counts()
{ svdRep->initialize_inactive_start_counts(); }


inline void SharedVariablesData::initialize_active_components()
{ svdRep->initialize_active_components(); }


inline void SharedVariablesData::initialize_inactive_components()
{ svdRep->initialize_inactive_components(); }


inline const BitArray& SharedVariablesData::all_relaxed_discrete_int() const
{ return svdRep->allRelaxedDiscreteInt; }


inline const BitArray& SharedVariablesData::all_relaxed_discrete_real() const
{ return svdRep->allRelaxedDiscreteReal; }


inline StringMultiArrayView SharedVariablesData::
all_continuous_labels(size_t start, size_t num_items) const
{
  return svdRep->
    allContinuousLabels[boost::indices[idx_range(start, start+num_items)]];
}


inline void SharedVariablesData::
all_continuous_labels(StringMultiArrayConstView cv_labels, size_t start,
		      size_t num_items)
{
  svdRep->allContinuousLabels[boost::indices[idx_range(start, start+num_items)]]
    = cv_labels;
}


inline void SharedVariablesData::
all_continuous_label(const String& cv_label, size_t index)
{ svdRep->allContinuousLabels[index] = cv_label; }


inline StringMultiArrayView SharedVariablesData::
all_discrete_int_labels(size_t start, size_t num_items) const
{
  return svdRep->
    allDiscreteIntLabels[boost::indices[idx_range(start, start+num_items)]];
}


inline void SharedVariablesData::
all_discrete_int_labels(StringMultiArrayConstView div_labels, size_t start,
		      size_t num_items)
{
  svdRep->
    allDiscreteIntLabels[boost::indices[idx_range(start, start+num_items)]]
    = div_labels;
}


inline void SharedVariablesData::
all_discrete_int_label(const String& div_label, size_t index)
{ svdRep->allDiscreteIntLabels[index] = div_label; }


inline StringMultiArrayView SharedVariablesData::
all_discrete_string_labels(size_t start, size_t num_items) const
{
  return svdRep->
    allDiscreteStringLabels[boost::indices[idx_range(start, start+num_items)]];
}


inline void SharedVariablesData::
all_discrete_string_labels(StringMultiArrayConstView dsv_labels, size_t start,
			   size_t num_items)
{
  svdRep->
    allDiscreteStringLabels[boost::indices[idx_range(start, start+num_items)]]
    = dsv_labels;
}


inline void SharedVariablesData::
all_discrete_string_label(const String& dsv_label, size_t index)
{ svdRep->allDiscreteStringLabels[index] = dsv_label; }


inline StringMultiArrayView SharedVariablesData::
all_discrete_real_labels(size_t start, size_t num_items) const
{
  return svdRep->
    allDiscreteRealLabels[boost::indices[idx_range(start, start+num_items)]];
}


inline void SharedVariablesData::
all_discrete_real_labels(StringMultiArrayConstView drv_labels, size_t start,
			 size_t num_items)
{
  svdRep->
    allDiscreteRealLabels[boost::indices[idx_range(start, start+num_items)]]
    = drv_labels;
}


inline void SharedVariablesData::
all_discrete_real_label(const String& drv_label, size_t index)
{ svdRep->allDiscreteRealLabels[index] = drv_label; }


inline UShortMultiArrayConstView SharedVariablesData::
all_continuous_types(size_t start, size_t num_items) const
{
  return svdRep->
    allContinuousTypes[boost::indices[idx_range(start, start+num_items)]];
}


inline void SharedVariablesData::
all_continuous_types(UShortMultiArrayConstView cv_types,
		     size_t start, size_t num_items)
{
  svdRep->allContinuousTypes[boost::indices[idx_range(start, start+num_items)]]
    = cv_types;
}


inline void SharedVariablesData::
all_continuous_type(unsigned short cv_type, size_t index)
{ svdRep->allContinuousTypes[index] = cv_type; }


inline UShortMultiArrayConstView SharedVariablesData::
all_discrete_int_types(size_t start, size_t num_items) const
{
  return svdRep->
    allDiscreteIntTypes[boost::indices[idx_range(start, start+num_items)]];
}


inline void SharedVariablesData::
all_discrete_int_types(UShortMultiArrayConstView div_types,
		       size_t start, size_t num_items)
{
  svdRep->allDiscreteIntTypes[boost::indices[idx_range(start, start+num_items)]]
    = div_types;
}


inline void SharedVariablesData::
all_discrete_int_type(unsigned short div_type, size_t index)
{ svdRep->allDiscreteIntTypes[index] = div_type; }


inline UShortMultiArrayConstView SharedVariablesData::
all_discrete_string_types(size_t start, size_t num_items) const
{
  return svdRep->
    allDiscreteStringTypes[boost::indices[idx_range(start, start+num_items)]];
}


inline void SharedVariablesData::
all_discrete_string_types(UShortMultiArrayConstView dsv_types,
			  size_t start, size_t num_items)
{
  svdRep->
    allDiscreteStringTypes[boost::indices[idx_range(start, start+num_items)]]
    = dsv_types;
}


inline void SharedVariablesData::
all_discrete_string_type(unsigned short dsv_type, size_t index)
{ svdRep->allDiscreteStringTypes[index] = dsv_type; }


inline UShortMultiArrayConstView SharedVariablesData::
all_discrete_real_types(size_t start, size_t num_items) const
{
  return svdRep->
    allDiscreteRealTypes[boost::indices[idx_range(start, start+num_items)]];
}


inline void SharedVariablesData::
all_discrete_real_types(UShortMultiArrayConstView drv_types,
			size_t start, size_t num_items)
{
  svdRep->
    allDiscreteRealTypes[boost::indices[idx_range(start, start+num_items)]]
    = drv_types;
}


inline void SharedVariablesData::
all_discrete_real_type(unsigned short drv_type, size_t index)
{ svdRep->allDiscreteRealTypes[index] = drv_type; }


inline SizetMultiArrayConstView SharedVariablesData::
all_continuous_ids(size_t start, size_t num_items) const
{
  return svdRep->
    allContinuousIds[boost::indices[idx_range(start, start+num_items)]];
}


inline void SharedVariablesData::
all_continuous_ids(SizetMultiArrayConstView cv_ids,
		   size_t start, size_t num_items)
{
  svdRep->allContinuousIds[boost::indices[idx_range(start, start+num_items)]]
    = cv_ids;
}


inline void SharedVariablesData::all_continuous_id(size_t cv_id, size_t index)
{ svdRep->allContinuousIds[index] = cv_id; }


inline const String& SharedVariablesData::id() const
{ return svdRep->variablesId; }


inline const SizetArray& SharedVariablesData::components_totals() const
{ return svdRep->variablesCompsTotals; }


inline const SizetArray& SharedVariablesData::active_components_totals() const
{ return svdRep->activeVarsCompsTotals; }


inline const SizetArray& SharedVariablesData::inactive_components_totals() const
{ return svdRep->inactiveVarsCompsTotals; }


inline size_t SharedVariablesData::vc_lookup(unsigned short key) const
{ return svdRep->vc_lookup(key); }


inline const std::pair<short,short>& SharedVariablesData::view() const
{ return svdRep->variablesView; }


inline void SharedVariablesData::inactive_view(short view2)
{ svdRep->variablesView.second = view2; }


inline size_t SharedVariablesData::cv() const
{ return svdRep->numCV; }


inline size_t SharedVariablesData::cv_start() const
{ return svdRep->cvStart; }


inline size_t SharedVariablesData::div() const
{ return svdRep->numDIV; }


inline size_t SharedVariablesData::div_start() const
{ return svdRep->divStart; }


inline size_t SharedVariablesData::dsv() const
{ return svdRep->numDSV; }


inline size_t SharedVariablesData::dsv_start() const
{ return svdRep->dsvStart; }


inline size_t SharedVariablesData::drv() const
{ return svdRep->numDRV; }


inline size_t SharedVariablesData::drv_start() const
{ return svdRep->drvStart; }


inline size_t SharedVariablesData::icv() const
{ return svdRep->numICV; }


inline size_t SharedVariablesData::icv_start() const
{ return svdRep->icvStart; }


inline size_t SharedVariablesData::idiv() const
{ return svdRep->numIDIV; }


inline size_t SharedVariablesData::idiv_start() const
{ return svdRep->idivStart; }


inline size_t SharedVariablesData::idsv() const
{ return svdRep->numIDSV; }


inline size_t SharedVariablesData::idsv_start() const
{ return svdRep->idsvStart; }


inline size_t SharedVariablesData::idrv() const
{ return svdRep->numIDRV; }


inline size_t SharedVariablesData::idrv_start() const
{ return svdRep->idrvStart; }


inline void SharedVariablesData::cv(size_t ncv)
{ svdRep->numCV = ncv; }


inline void SharedVariablesData::cv_start(size_t cvs)
{ svdRep->cvStart = cvs; }


inline void SharedVariablesData::div(size_t ndiv)
{ svdRep->numDIV = ndiv; }


inline void SharedVariablesData::div_start(size_t divs)
{ svdRep->divStart = divs; }


inline void SharedVariablesData::dsv(size_t ndsv)
{ svdRep->numDSV = ndsv; }


inline void SharedVariablesData::dsv_start(size_t dsvs)
{ svdRep->dsvStart = dsvs; }


inline void SharedVariablesData::drv(size_t ndrv)
{ svdRep->numDRV = ndrv; }


inline void SharedVariablesData::drv_start(size_t drvs)
{ svdRep->drvStart = drvs; }


inline void SharedVariablesData::icv(size_t nicv)
{ svdRep->numICV = nicv; }


inline void SharedVariablesData::icv_start(size_t icvs)
{ svdRep->icvStart = icvs; }


inline void SharedVariablesData::idiv(size_t nidiv)
{ svdRep->numIDIV = nidiv; }


inline void SharedVariablesData::idiv_start(size_t idivs)
{ svdRep->idivStart = idivs; }


inline void SharedVariablesData::idsv(size_t nidsv)
{ svdRep->numIDSV = nidsv; }


inline void SharedVariablesData::idsv_start(size_t idsvs)
{ svdRep->idsvStart = idsvs; }


inline void SharedVariablesData::idrv(size_t nidrv)
{ svdRep->numIDRV = nidrv; }


inline void SharedVariablesData::idrv_start(size_t idrvs)
{ svdRep->idrvStart = idrvs; }

} // namespace Dakota


// Since we may serialize this class through a temporary, force
// serialization mode and no tracking.  We allow tracking on
// SharedVariablesDataRep as we want to serialize each unique pointer
// exactly once (may need to revisit this).
BOOST_CLASS_IMPLEMENTATION(Dakota::SharedVariablesData, 
 			   boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(Dakota::SharedVariablesData, 
  		     boost::serialization::track_never)

#endif
