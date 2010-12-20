/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SharedVariablesData
//- Description: Encapsulation of data that can be shared among Variables
//-              instances.
//- Owner:       Mike Eldred
//- Version: $Id: SharedVariablesData.H 6784 2010-05-18 21:10:35Z mseldre $

#ifndef SHARED_VARIABLES_DATA_H
#define SHARED_VARIABLES_DATA_H

#include "data_types.h"
#include "DakotaString.H"


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

private:

  //
  //- Heading: Constructor and destructor
  //

  /// standard constructor
  SharedVariablesDataRep(const ProblemDescDB& problem_db,
			 const std::pair<short,short>& view);
  /// lightweight constructor
  SharedVariablesDataRep(const std::pair<short,short>& view,
			 const SizetArray& vars_comps_totals);
  /// destructor
  ~SharedVariablesDataRep();

  //
  //- Heading: Member functions
  //

  /// size allContinuousLabels, with or without discrete relaxation
  void size_all_continuous_labels(bool relax);
  /// initialize allContinuousTypes, with or without discrete relaxation
  void initialize_all_continuous_types(bool relax);
  /// initialize allContinuousIds, with or without discrete relaxation
  void initialize_all_continuous_ids(bool relax);
  /// size allDiscreteIntLabels
  void size_all_discrete_int_labels();
  /// initialize allDiscreteIntTypes
  void initialize_all_discrete_int_types();
  /// size allDiscreteRealLabels
  void size_all_discrete_real_labels();
  /// initialize allDiscreteRealTypes
  void initialize_all_discrete_real_types();

  /// retrieve the count within variablesComponents corresponding to key
  size_t vc_lookup(unsigned short key) const;

  //
  //- Heading: Data
  //

  /// variables identifier string from the input file
  String idVariables;

  /// the variables view pair containing active (first) and inactive (second)
  /// view enumerations
  std::pair<short,short> variablesView;
  /// map linking variable types to counts
  std::map<unsigned short, size_t> variablesComponents;
  /// totals for variable type counts for
  /// {continuous,discrete integer,discrete real}
  /// {design,aleatory uncertain,epistemic uncertain,state}
  SizetArray variablesCompsTotals;

  /// array of variable labels for all of the continuous variables
  StringMultiArray allContinuousLabels;
  /// array of variable labels for all of the discrete integer variables
  StringMultiArray allDiscreteIntLabels;
  /// array of variable labels for all of the discrete real variables
  StringMultiArray allDiscreteRealLabels;

  /// array of variable types for all of the continuous variables
  UShortMultiArray allContinuousTypes;
  /// array of variable types for all of the discrete integer variables
  UShortMultiArray allDiscreteIntTypes;
  /// array of variable types for all of the discrete real variables
  UShortMultiArray allDiscreteRealTypes;

  /// array of 1-based position identifiers for the all continuous
  /// variables array
  /** These identifiers define positions of the all continuous
      variables array within the total variable sequence. */
  SizetMultiArray allContinuousIds;

  /// array of discrete variable identifiers for which the discrete
  /// requirement is relaxed by merging them into a continuous array
  SizetArray mergedDiscreteIds;

  /// number of handle objects sharing svdRep
  int referenceCount;
};


inline SharedVariablesDataRep::~SharedVariablesDataRep()
{ }


inline size_t SharedVariablesDataRep::vc_lookup(unsigned short key) const
{
  std::map<unsigned short, size_t>::const_iterator cit
    = variablesComponents.find(key);
  return (cit == variablesComponents.end()) ? 0 : cit->second;
}


inline void SharedVariablesDataRep::size_all_continuous_labels(bool relax)
{
  size_t num_acv = variablesCompsTotals[0] + variablesCompsTotals[3] +
    variablesCompsTotals[6] + variablesCompsTotals[9];
  if (relax)
    num_acv += variablesCompsTotals[1] + variablesCompsTotals[2] +
      variablesCompsTotals[4]  + variablesCompsTotals[5] +
      variablesCompsTotals[7]  + variablesCompsTotals[8] +
      variablesCompsTotals[10] + variablesCompsTotals[11];
  allContinuousLabels.resize(boost::extents[num_acv]);
}


inline void SharedVariablesDataRep::size_all_discrete_int_labels()
{
  size_t num_adiv = variablesCompsTotals[1] + variablesCompsTotals[4] + 
    variablesCompsTotals[7] + variablesCompsTotals[10];
  allDiscreteIntLabels.resize(boost::extents[num_adiv]);
}


inline void SharedVariablesDataRep::size_all_discrete_real_labels()
{
  size_t num_adrv = variablesCompsTotals[2] + variablesCompsTotals[5] +
    variablesCompsTotals[8] + variablesCompsTotals[11];
  allDiscreteRealLabels.resize(boost::extents[num_adrv]);
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
  //- Heading: Constructors, destructor, and operators
  //

  /// default constructor
  SharedVariablesData();
  /// standard constructor
  SharedVariablesData(const ProblemDescDB& problem_db,
		      const std::pair<short,short>& view);
  /// lightweight constructor
  SharedVariablesData(const std::pair<short,short>& view,
		      const SizetArray& vars_comps_totals);
  /// copy constructor
  SharedVariablesData(const SharedVariablesData& svd);
  /// destructor
  ~SharedVariablesData();

  /// assignment operator
  SharedVariablesData& operator=(const SharedVariablesData& svd);

  //
  //- Heading: member functions
  //

  /// size labels for all of the continuous variables, with or without
  /// discrete relaxation
  void size_all_continuous_labels(bool relax);
  /// initialize types for all of the continuous variables, with or without
  /// discrete relaxation
  void initialize_all_continuous_types(bool relax);
  /// initialize ids for all of the continuous variables, with or without
  /// discrete relaxation
  void initialize_all_continuous_ids(bool relax);
  /// size labels for all of the discrete integer variables
  void size_all_discrete_int_labels();
  /// initialize types for all of the discrete integer variables
  void initialize_all_discrete_int_types();
  /// size labels for all of the discrete real variables
  void size_all_discrete_real_labels();
  /// initialize types for all of the discrete real variables
  void initialize_all_discrete_real_types();

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
  /// get num_items discrete integer types beginning at index start
  UShortMultiArrayConstView
    all_discrete_int_types(size_t start, size_t num_items) const;
  /// get num_items discrete real types beginning at index start
  UShortMultiArrayConstView
    all_discrete_real_types(size_t start, size_t num_items) const;

  /// get num_items continuous ids beginning at index start
  SizetMultiArrayConstView
    all_continuous_ids(size_t start, size_t num_items) const;
  /// get ids of discrete variables that have been merged into continuous arrays
  const SizetArray& merged_discrete_ids() const;

  /// retreive the Variables view
  const std::pair<short,short>& view() const;
  /// set the inactive Variables view
  void inactive_view(short view2);

  /// return the user-provided or default Variables identifier
  const String& id() const;
  /// return variable type counts for
  /// {continuous,discrete integer,discrete real}
  /// {design,aleatory uncertain,epistemic uncertain,state}
  const SizetArray& components_totals() const;

  /// retrieve the variables type count within
  /// svdRep->variablesComponents corresponding to key
  size_t vc_lookup(unsigned short key) const;

private:

  //
  //- Heading: Private data members
  //
 
  /// pointer to the body (handle-body idiom)
  SharedVariablesDataRep* svdRep;
};


inline SharedVariablesData::SharedVariablesData(): svdRep(NULL)
{ }


inline SharedVariablesData::
SharedVariablesData(const ProblemDescDB& problem_db,
		    const std::pair<short,short>& view):
  svdRep(new SharedVariablesDataRep(problem_db, view))
{ }


inline SharedVariablesData::
SharedVariablesData(const std::pair<short,short>& view,
		    const SizetArray& vars_comps_totals):
  svdRep(new SharedVariablesDataRep(view, vars_comps_totals))
{ }


inline SharedVariablesData::SharedVariablesData(const SharedVariablesData& svd)
{
  // Increment new (no old to decrement)
  svdRep = svd.svdRep;
  if (svdRep) // Check for an assignment of NULL
    ++svdRep->referenceCount;
}


inline SharedVariablesData::~SharedVariablesData()
{
  if (svdRep) { // Check for NULL
    --svdRep->referenceCount; // decrement
    if (svdRep->referenceCount == 0)
      delete svdRep;
  }
}


inline SharedVariablesData& SharedVariablesData::
operator=(const SharedVariablesData& svd)
{
  if (svdRep != svd.svdRep) { // normal case: old != new
    // Decrement old
    if (svdRep) // Check for NULL
      if ( --svdRep->referenceCount == 0 ) 
	delete svdRep;
    // Assign new
    svdRep = svd.svdRep;
  }
  // Increment new
  if (svdRep) // Check for an assignment of NULL
    ++svdRep->referenceCount;
  return *this;
}


inline void SharedVariablesData::size_all_continuous_labels(bool relax)
{ svdRep->size_all_continuous_labels(relax); }


inline void SharedVariablesData::initialize_all_continuous_types(bool relax)
{ svdRep->initialize_all_continuous_types(relax); }


inline void SharedVariablesData::initialize_all_continuous_ids(bool relax)
{ svdRep->initialize_all_continuous_ids(relax); }


inline void SharedVariablesData::size_all_discrete_int_labels()
{ svdRep->size_all_discrete_int_labels(); }


inline void SharedVariablesData::initialize_all_discrete_int_types()
{ svdRep->initialize_all_discrete_int_types(); }


inline void SharedVariablesData::size_all_discrete_real_labels()
{ svdRep->size_all_discrete_real_labels(); }


inline void SharedVariablesData::initialize_all_discrete_real_types()
{ svdRep->initialize_all_discrete_real_types(); }


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


inline UShortMultiArrayConstView SharedVariablesData::
all_discrete_int_types(size_t start, size_t num_items) const
{
  return svdRep->
    allDiscreteIntTypes[boost::indices[idx_range(start, start+num_items)]];
}


inline UShortMultiArrayConstView SharedVariablesData::
all_discrete_real_types(size_t start, size_t num_items) const
{
  return svdRep->
    allDiscreteRealTypes[boost::indices[idx_range(start, start+num_items)]];
}


inline SizetMultiArrayConstView SharedVariablesData::
all_continuous_ids(size_t start, size_t num_items) const
{
  return svdRep->
    allContinuousIds[boost::indices[idx_range(start, start+num_items)]];
}


inline const SizetArray& SharedVariablesData::merged_discrete_ids() const
{ return svdRep->mergedDiscreteIds; }


inline const std::pair<short,short>& SharedVariablesData::view() const
{ return svdRep->variablesView; }


inline void SharedVariablesData::inactive_view(short view2)
{ svdRep->variablesView.second = view2; }


inline const String& SharedVariablesData::id() const
{ return svdRep->idVariables; }


inline const SizetArray& SharedVariablesData::components_totals() const
{ return svdRep->variablesCompsTotals; }


inline size_t SharedVariablesData::vc_lookup(unsigned short key) const
{ return svdRep->vc_lookup(key); }

} // namespace Dakota

#endif
