/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Variables
//- Description: Base class for variables.  Using the foundational
//-              cdv/ddv/uv/csv/dsv arrays from an input specification,
//-              derived classes use different views (All, Distinct, Merged)
//-              to define active and inactive variable sets.
//- Owner:       Mike Eldred
//- Version: $Id: DakotaVariables.H 7037 2010-10-23 01:18:08Z mseldre $

#ifndef DAKOTA_VARIABLES_H
#define DAKOTA_VARIABLES_H

#include "data_types.h"
#include "global_defs.h"  // for BaseConstructor
#include "SharedVariablesData.H"
#include "DakotaString.H"


namespace Dakota {

// forward declarations
class ProblemDescDB;
class BiStream;
class BoStream;
class MPIPackBuffer;
class MPIUnpackBuffer;


/// Base class for the variables class hierarchy.

/** The Variables class is the base class for the class hierarchy
    providing design, uncertain, and state variables for continuous
    and discrete domains within a Model.  Using the fundamental
    arrays from the input specification, different derived classes
    define different views of the data.  For memory efficiency and
    enhanced polymorphism, the variables hierarchy employs the
    "letter/envelope idiom" (see Coplien "Advanced C++", p. 133), for
    which the base class (Variables) serves as the envelope and one of
    the derived classes (selected in Variables::get_variables())
    serves as the letter. */

class Variables
{
  //
  //- Heading: Friends
  //

  /// equality operator
  friend bool operator==(const Variables& vars1, const Variables& vars2);
  /// inequality operator
  friend bool operator!=(const Variables& vars1, const Variables& vars2);

  /// hash_value
  friend std::size_t hash_value(const Variables& vars);

  /// binary_equal_to (since 'operator==' is not suitable for boost/hash_set)
  friend bool binary_equal_to(const Variables& vars1, const Variables& vars2);

public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  Variables();
  /// standard constructor
  Variables(const ProblemDescDB& problem_db);
  /// alternate constructor for instantiations on the fly
  Variables(const SharedVariablesData& svd);
  /// copy constructor
  Variables(const Variables& vars);

  /// destructor
  virtual ~Variables();

  /// assignment operator
  Variables operator=(const Variables& vars);

  //
  //- Heading: Virtual functions
  //

  /// reshapes an existing Variables object based on the incoming
  /// variablesComponents
  virtual void reshape(const SizetArray& vc_totals);

  // INPUT/OUTPUT

  /// read a variables object from an std::istream
  virtual void read(std::istream& s);
  /// write a variables object to an std::ostream
  virtual void write(std::ostream& s) const;
  /// write a variables object to an std::ostream in aprepro format
  virtual void write_aprepro(std::ostream& s) const;

  // For neutral file I/O:
  /// read a variables object in annotated format from an istream
  virtual void read_annotated(std::istream& s);
  /// write a variables object in annotated format to an std::ostream
  virtual void write_annotated(std::ostream& s) const;

  /// read a variables object in tabular format from an istream
  virtual void read_tabular(std::istream& s);
  /// write a variables object in tabular format to an std::ostream
  virtual void write_tabular(std::ostream& s) const;

  /// read a variables object from the binary restart stream
  virtual void read(BiStream& s);
  /// write a variables object to the binary restart stream
  virtual void write(BoStream& s) const;

  /// read a variables object from a packed MPI buffer
  virtual void read(MPIUnpackBuffer& s);
  /// write a variables object to a packed MPI buffer
  virtual void write(MPIPackBuffer& s) const;

  //
  //- Heading: Member functions
  //

  size_t tv()         const; ///< total number of vars
  size_t cv()         const; ///< number of active continuous vars
  size_t cv_start()   const; ///< start index of active continuous vars
  size_t div()        const; ///< number of active discrete int vars
  size_t div_start()  const; ///< start index of active discrete int vars
  size_t drv()        const; ///< number of active discrete real vars
  size_t drv_start()  const; ///< start index of active discrete real vars
  size_t icv()        const; ///< number of inactive continuous vars
  size_t icv_start()  const; ///< start index of inactive continuous vars
  size_t idiv()       const; ///< number of inactive discrete int vars
  size_t idiv_start() const; ///< start index of inactive discrete int vars
  size_t idrv()       const; ///< number of inactive discrete real vars
  size_t idrv_start() const; ///< start index of inactive discrete real vars
  size_t acv()        const; ///< total number of continuous vars
  size_t adiv()       const; ///< total number of discrete integer vars
  size_t adrv()       const; ///< total number of discrete real vars

  /// return sharedVarsData
  const SharedVariablesData& shared_data() const;

  // ACTIVE VARIABLES

  /// return an active continuous variable
  const Real& continuous_variable(size_t index) const;
  /// return the active continuous variables
  const RealVector& continuous_variables() const;
  /// set an active continuous variable
  void continuous_variable(const Real& c_var, size_t index);
  /// set the active continuous variables
  void continuous_variables(const RealVector& c_vars);
  /// return an active discrete integer variable
  int discrete_int_variable(size_t index) const;
  /// return the active discrete integer variables
  const IntVector& discrete_int_variables() const;
  /// set an active discrete integer variable
  void discrete_int_variable(int di_var, size_t index);
  /// set the active discrete integer variables
  void discrete_int_variables(const IntVector& di_vars);
  /// return an active discrete real variable
  const Real& discrete_real_variable(size_t index) const;
  /// return the active discrete real variables
  const RealVector& discrete_real_variables() const;
  /// set an active discrete real variable
  void discrete_real_variable(const Real& dr_var, size_t index);
  /// set the active discrete real variables
  void discrete_real_variables(const RealVector& dr_vars);

  /// return the active continuous variable labels
  StringMultiArrayConstView continuous_variable_labels() const;
  /// set the active continuous variable labels
  void continuous_variable_labels(StringMultiArrayConstView cv_labels);
  /// set an active continuous variable label
  void continuous_variable_label(const String& cv_label, size_t index);
  /// return the active discrete integer variable labels
  StringMultiArrayConstView discrete_int_variable_labels() const;
  /// set the active discrete integer variable labels
  void discrete_int_variable_labels(StringMultiArrayConstView div_labels);
  /// set an active discrete integer variable label
  void discrete_int_variable_label(const String& div_label, size_t index);
  /// return the active discrete real variable labels
  StringMultiArrayConstView discrete_real_variable_labels() const;
  /// set the active discrete real variable labels
  void discrete_real_variable_labels(StringMultiArrayConstView drv_labels);
  /// set an active discrete real variable label
  void discrete_real_variable_label(const String& drv_label, size_t index);

  /// return the active continuous variable types
  UShortMultiArrayConstView continuous_variable_types() const;
  /// return the active discrete integer variable types
  UShortMultiArrayConstView discrete_int_variable_types() const;
  /// return the active discrete real variable types
  UShortMultiArrayConstView discrete_real_variable_types() const;

  /// return the active continuous variable position identifiers
  SizetMultiArrayConstView continuous_variable_ids() const;

  /// returns the list of discrete variables merged into a continuous array
  const SizetArray& merged_discrete_ids() const;

  // INACTIVE VARIABLES

  /// return the inactive continuous variables
  const RealVector& inactive_continuous_variables() const;
  /// set the inactive continuous variables
  void inactive_continuous_variables(const RealVector& ic_vars);
  /// return the inactive discrete variables
  const IntVector& inactive_discrete_int_variables() const;
  /// set the inactive discrete variables
  void inactive_discrete_int_variables(const IntVector& idi_vars);
  /// return the inactive discrete variables
  const RealVector& inactive_discrete_real_variables() const;
  /// set the inactive discrete variables
  void inactive_discrete_real_variables(const RealVector& idr_vars);

  /// return the inactive continuous variable labels
  StringMultiArrayConstView inactive_continuous_variable_labels() const;
  /// set the inactive continuous variable labels
  void inactive_continuous_variable_labels(StringMultiArrayConstView ic_vars);
  /// return the inactive discrete variable labels
  StringMultiArrayConstView inactive_discrete_int_variable_labels() const;
  /// set the inactive discrete variable labels
  void inactive_discrete_int_variable_labels(
    StringMultiArrayConstView idi_vars);
  /// return the inactive discrete variable labels
  StringMultiArrayConstView inactive_discrete_real_variable_labels() const;
  /// set the inactive discrete variable labels
  void inactive_discrete_real_variable_labels(
    StringMultiArrayConstView idr_vars);

  /// return the inactive continuous variable types
  UShortMultiArrayConstView inactive_continuous_variable_types() const;
  /// return the inactive discrete integer variable types
  UShortMultiArrayConstView inactive_discrete_int_variable_types() const;
  /// return the inactive discrete real variable types
  UShortMultiArrayConstView inactive_discrete_real_variable_types() const;

  /// return the inactive continuous variable position identifiers
  SizetMultiArrayConstView inactive_continuous_variable_ids() const;

  // ALL VARIABLES

  /// returns a single array with all continuous variables
  const RealVector& all_continuous_variables() const;
  /// sets all continuous variables using a single array
  void all_continuous_variables(const RealVector& ac_vars);
  /// set a variable within the all continuous array
  void all_continuous_variable(const Real& ac_var, size_t index);
  /// returns a single array with all discrete variables
  const IntVector& all_discrete_int_variables() const;
  /// sets all discrete variables using a single array
  void all_discrete_int_variables(const IntVector& adi_vars);
  /// set a variable within the all discrete array
  void all_discrete_int_variable(int adi_var, size_t index);
  /// returns a single array with all discrete variables
  const RealVector& all_discrete_real_variables() const;
  /// sets all discrete variables using a single array
  void all_discrete_real_variables(const RealVector& adr_vars);
  /// set a variable within the all discrete array
  void all_discrete_real_variable(const Real& adr_var, size_t index);

  /// returns a single array with all continuous variable labels
  StringMultiArrayView all_continuous_variable_labels() const;
  /// sets all continuous variable labels using a single array
  void all_continuous_variable_labels(StringMultiArrayConstView acv_labels);
  /// set a label within the all continuous label array
  void all_continuous_variable_label(const String& acv_label,size_t index);
  /// returns a single array with all discrete variable labels
  StringMultiArrayView all_discrete_int_variable_labels() const;
  /// sets all discrete variable labels using a single array
  void all_discrete_int_variable_labels(StringMultiArrayConstView adiv_labels);
  /// set a label within the all discrete label array
  void all_discrete_int_variable_label(const String& adiv_label, size_t index);
  /// returns a single array with all discrete variable labels
  StringMultiArrayView all_discrete_real_variable_labels() const;
  /// sets all discrete variable labels using a single array
  void all_discrete_real_variable_labels(StringMultiArrayConstView adrv_labels);
  /// set a label within the all discrete label array
  void all_discrete_real_variable_label(const String& adrv_label, size_t index);

  /// return all continuous variable types
  UShortMultiArrayConstView all_continuous_variable_types() const;
  /// return all discrete variable types
  UShortMultiArrayConstView all_discrete_int_variable_types() const;
  /// return all discrete variable types
  UShortMultiArrayConstView all_discrete_real_variable_types() const;

  /// return all continuous variable position identifiers
  SizetMultiArrayConstView all_continuous_variable_ids() const;

  /// for use when a deep copy is needed (the representation is _not_ shared)
  Variables copy() const;

  /// returns variablesView
  const std::pair<short,short>& view() const;
  /// defines variablesView from problem_db attributes
  std::pair<short,short> get_view(const ProblemDescDB& problem_db) const;
  /// sets the inactive view based on higher level (nested) context
  void inactive_view(short view2);

  /// returns the variables identifier string
  const String& variables_id() const;

  /// returns the number of variables for each of the constitutive components
  const SizetArray& variables_components_totals() const;

  /// function to check variablesRep (does this envelope contain a letter)
  bool is_null() const;

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Variables(BaseConstructor, const ProblemDescDB& problem_db,
	    const std::pair<short,short>& view);
  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Variables(BaseConstructor, const SharedVariablesData& svd);

  //
  //- Heading: Virtual functions
  //

  /// construct active views of all variables arrays
  virtual void build_active_views();
  /// construct inactive views of all variables arrays
  virtual void build_inactive_views();

  //
  //- Heading: Member functions
  //

  /// construct active/inactive views of all variables arrays
  void build_views();

  //
  //- Heading: Data
  //

  /// reference-counted instance of shared variables data: id's, labels, counts
  SharedVariablesData sharedVarsData;

  /// array combining all of the continuous variables (design, uncertain, state)
  RealVector allContinuousVars;
  /// array combining all of the discrete integer variables (design, state)
  IntVector allDiscreteIntVars;
  /// array combining all of the discrete real variables (design, state)
  RealVector allDiscreteRealVars;

  //
  //- Heading: Data views
  //

  /// start index of active continuous variables within allContinuousVars
  size_t cvStart;
  /// start index of active discrete integer variables within allDiscreteIntVars
  size_t divStart;
  /// start index of active discrete real variables within allDiscreteRealVars
  size_t drvStart;
  /// start index of inactive continuous variables within allContinuousVars
  size_t icvStart;
  /// start index of inactive discrete integer variables w/i allDiscreteIntVars
  size_t idivStart;
  /// start index of inactive discrete real variables within allDiscreteRealVars
  size_t idrvStart;
  size_t numCV;   ///< number of active continuous variables
  size_t numDIV;  ///< number of active discrete integer variables
  size_t numDRV;  ///< number of active discrete real variables
  size_t numICV;  ///< number of inactive continuous variables
  size_t numIDIV; ///< number of inactive discrete integer variables
  size_t numIDRV; ///< number of inactive discrete real variables

  /// the active continuous variables array view
  RealVector continuousVars;
  /// the active discrete integer variables array view
  IntVector discreteIntVars;
  /// the active discrete real variables array view
  RealVector discreteRealVars;
  /// the inactive continuous variables array view
  RealVector inactiveContinuousVars;
  /// the inactive discrete integer variables array view
  IntVector inactiveDiscreteIntVars;
  /// the inactive discrete real variables array view
  RealVector inactiveDiscreteRealVars;

private:

  //
  //- Heading: Member functions
  //

  /// Used by the standard envelope constructor to instantiate the
  /// correct letter class
  Variables* get_variables(const ProblemDescDB& problem_db);
  /// Used by the alternate envelope constructors, by read functions,
  /// and by copy() to instantiate a new letter class
  Variables* get_variables(const SharedVariablesData& svd) const;

  /// perform sanity checks on view.first and view.second after update
  void check_view_compatibility();

  //
  //- Heading: Data
  //

  /// pointer to the letter (initialized only for the envelope)
  Variables* variablesRep;
  /// number of objects sharing variablesRep
  int referenceCount;
};


// nonvirtual functions can access letter attributes directly (only need to fwd
// member function call when the function could be redefined).
inline size_t Variables::tv() const
{
  return (variablesRep) ? variablesRep->allContinuousVars.length() +
    variablesRep->allDiscreteIntVars.length() +
    variablesRep->allDiscreteRealVars.length() :
    allContinuousVars.length() + allDiscreteIntVars.length() +
    allDiscreteRealVars.length();
}


inline size_t Variables::cv() const
{ return (variablesRep) ? variablesRep->numCV : numCV; }


inline size_t Variables::cv_start() const
{ return (variablesRep) ? variablesRep->cvStart : cvStart; }


inline size_t Variables::div() const
{ return (variablesRep) ? variablesRep->numDIV : numDIV; }


inline size_t Variables::div_start() const
{ return (variablesRep) ? variablesRep->divStart : divStart; }


inline size_t Variables::drv() const
{ return (variablesRep) ? variablesRep->numDRV : numDRV; }


inline size_t Variables::drv_start() const
{ return (variablesRep) ? variablesRep->drvStart : drvStart; }


inline size_t Variables::icv() const
{ return (variablesRep) ? variablesRep->numICV : numICV; }


inline size_t Variables::icv_start() const
{ return (variablesRep) ? variablesRep->icvStart : icvStart; }


inline size_t Variables::idiv() const
{ return (variablesRep) ? variablesRep->numIDIV : numIDIV; }


inline size_t Variables::idiv_start() const
{ return (variablesRep) ? variablesRep->idivStart : idivStart; }


inline size_t Variables::idrv() const
{ return (variablesRep) ? variablesRep->numIDRV : numIDRV; }


inline size_t Variables::idrv_start() const
{ return (variablesRep) ? variablesRep->idrvStart : idrvStart; }


inline size_t Variables::acv() const
{
  return (variablesRep) ?
    variablesRep->allContinuousVars.length() : allContinuousVars.length();
}


inline size_t Variables::adiv() const
{
  return (variablesRep) ?
    variablesRep->allDiscreteIntVars.length() : allDiscreteIntVars.length();
}


inline size_t Variables::adrv() const
{
  return (variablesRep) ?
    variablesRep->allDiscreteRealVars.length() : allDiscreteRealVars.length();
}


inline const SharedVariablesData& Variables::shared_data() const
{ return (variablesRep) ? variablesRep->sharedVarsData : sharedVarsData; }


inline void Variables::continuous_variable(const Real& c_var, size_t index)
{
  if (variablesRep)
    variablesRep->continuousVars[index] = c_var;
  else
    continuousVars[index] = c_var;
}


inline const Real& Variables::continuous_variable(size_t index) const
{
  if (variablesRep)
    return variablesRep->continuousVars[index];
  else
    return continuousVars[index];
}


inline const RealVector& Variables::continuous_variables() const
{ return (variablesRep) ? variablesRep->continuousVars : continuousVars; }


inline void Variables::
continuous_variables(const RealVector& c_vars)
{
  if (variablesRep)
    variablesRep->continuousVars.assign(c_vars);
  else
    continuousVars.assign(c_vars);
}


inline void Variables::discrete_int_variable(int di_var, size_t index)
{
  if (variablesRep)
    variablesRep->discreteIntVars[index] = di_var;
  else
    discreteIntVars[index] = di_var;
}


inline int Variables::discrete_int_variable(size_t index) const
{
  if (variablesRep)
    return variablesRep->discreteIntVars[index];
  else
    return discreteIntVars[index];
}


inline const IntVector& Variables::discrete_int_variables() const
{ return (variablesRep) ? variablesRep->discreteIntVars : discreteIntVars; }


inline void Variables::discrete_int_variables(const IntVector& di_vars)
{
  if (variablesRep)
    variablesRep->discreteIntVars.assign(di_vars);
  else
    discreteIntVars.assign(di_vars);
}


inline void Variables::
discrete_real_variable(const Real& dr_var, size_t index)
{
  if (variablesRep)
    variablesRep->discreteRealVars[index] = dr_var;
  else
    discreteRealVars[index] = dr_var;
}


inline const Real& Variables::discrete_real_variable(size_t index) const
{
  if (variablesRep)
    return variablesRep->discreteRealVars[index];
  else
    return discreteRealVars[index];
}


inline const RealVector& Variables::discrete_real_variables() const
{ return (variablesRep) ? variablesRep->discreteRealVars : discreteRealVars; }


inline void Variables::discrete_real_variables(const RealVector& dr_vars)
{
  if (variablesRep)
    variablesRep->discreteRealVars.assign(dr_vars);
  else
    discreteRealVars.assign(dr_vars);
}


inline StringMultiArrayConstView Variables::continuous_variable_labels() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_continuous_labels(variablesRep->cvStart,
						       variablesRep->numCV) :
    sharedVarsData.all_continuous_labels(cvStart, numCV);
}


inline void Variables::
continuous_variable_labels(StringMultiArrayConstView cv_labels)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_continuous_labels(cv_labels,
      variablesRep->cvStart, variablesRep->numCV);
  else
    sharedVarsData.all_continuous_labels(cv_labels, cvStart, numCV);
}


inline void Variables::
continuous_variable_label(const String& cv_label, size_t index)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_continuous_label(cv_label,
      variablesRep->cvStart+index);
  else
    sharedVarsData.all_continuous_label(cv_label, cvStart+index);
}


inline StringMultiArrayConstView Variables::discrete_int_variable_labels() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_int_labels(
      variablesRep->divStart, variablesRep->numDIV) :
    sharedVarsData.all_discrete_int_labels(divStart, numDIV);
}


inline void Variables::
discrete_int_variable_labels(StringMultiArrayConstView div_labels)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_discrete_int_labels(div_labels,
      variablesRep->divStart, variablesRep->numDIV);
  else
    sharedVarsData.all_discrete_int_labels(div_labels, divStart, numDIV);
}


inline void Variables::
discrete_int_variable_label(const String& div_label, size_t index)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_discrete_int_label(div_label,
      variablesRep->divStart+index);
  else
    sharedVarsData.all_discrete_int_label(div_label, divStart+index);
}


inline StringMultiArrayConstView Variables::
discrete_real_variable_labels() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_real_labels(
      variablesRep->drvStart, variablesRep->numDRV) :
    sharedVarsData.all_discrete_real_labels(drvStart, numDRV);
}


inline void Variables::
discrete_real_variable_labels(StringMultiArrayConstView drv_labels)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_discrete_real_labels(drv_labels,
      variablesRep->drvStart, variablesRep->numDRV);
  else
    sharedVarsData.all_discrete_real_labels(drv_labels, drvStart, numDRV);
}


inline void Variables::
discrete_real_variable_label(const String& drv_label, size_t index)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_discrete_real_label(drv_label,
      variablesRep->drvStart+index);
  else
    sharedVarsData.all_discrete_real_label(drv_label, drvStart+index);
}


inline UShortMultiArrayConstView Variables::continuous_variable_types() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_continuous_types(
      variablesRep->cvStart, variablesRep->numCV) :
    sharedVarsData.all_continuous_types(cvStart, numCV);
}


inline UShortMultiArrayConstView Variables::discrete_int_variable_types() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_int_types(
      variablesRep->divStart, variablesRep->numDIV) :
    sharedVarsData.all_discrete_int_types(divStart, numDIV);
}


inline UShortMultiArrayConstView Variables::discrete_real_variable_types() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_real_types(
      variablesRep->drvStart, variablesRep->numDRV) :
    sharedVarsData.all_discrete_real_types(drvStart, numDRV);
}


inline SizetMultiArrayConstView Variables::continuous_variable_ids() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_continuous_ids(
      variablesRep->cvStart, variablesRep->numCV) :
    sharedVarsData.all_continuous_ids(cvStart, numCV);
}


inline const SizetArray& Variables::merged_discrete_ids() const
{
  return (variablesRep) ? variablesRep->sharedVarsData.merged_discrete_ids() :
    sharedVarsData.merged_discrete_ids();
}


inline const RealVector& Variables::inactive_continuous_variables() const
{
  return (variablesRep) ? variablesRep->inactiveContinuousVars :
    inactiveContinuousVars;
}


inline void Variables::
inactive_continuous_variables(const RealVector& ic_vars)
{
  if (variablesRep)
    variablesRep->inactiveContinuousVars.assign(ic_vars);
  else
    inactiveContinuousVars.assign(ic_vars);
}


inline const IntVector& Variables::inactive_discrete_int_variables() const
{
  return (variablesRep) ?
    variablesRep->inactiveDiscreteIntVars : inactiveDiscreteIntVars;
}


inline void Variables::
inactive_discrete_int_variables(const IntVector& idi_vars)
{
  if (variablesRep)
    variablesRep->inactiveDiscreteIntVars.assign(idi_vars);
  else
    inactiveDiscreteIntVars.assign(idi_vars);
}


inline const RealVector& Variables::inactive_discrete_real_variables() const
{
  return (variablesRep) ?
    variablesRep->inactiveDiscreteRealVars : inactiveDiscreteRealVars;
}


inline void Variables::
inactive_discrete_real_variables(const RealVector& idr_vars)
{
  if (variablesRep)
    variablesRep->inactiveDiscreteRealVars.assign(idr_vars);
  else
    inactiveDiscreteRealVars.assign(idr_vars);
}


inline StringMultiArrayConstView Variables::
inactive_continuous_variable_labels() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_continuous_labels(variablesRep->icvStart,
						       variablesRep->numICV) :
    sharedVarsData.all_continuous_labels(icvStart, numICV);
}


inline void Variables::
inactive_continuous_variable_labels(StringMultiArrayConstView icv_labels)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_continuous_labels(icv_labels,
      variablesRep->icvStart, variablesRep->numICV);
  else
    sharedVarsData.all_continuous_labels(icv_labels, icvStart, numICV);
}


inline StringMultiArrayConstView Variables::
inactive_discrete_int_variable_labels() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_int_labels(
      variablesRep->idivStart, variablesRep->numIDIV) :
    sharedVarsData.all_discrete_int_labels(idivStart, numIDIV);
}


inline void Variables::
inactive_discrete_int_variable_labels(StringMultiArrayConstView idiv_labels)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_discrete_int_labels(idiv_labels,
      variablesRep->idivStart, variablesRep->numIDIV);
  else
    sharedVarsData.all_discrete_int_labels(idiv_labels, idivStart, numIDIV);
}


inline StringMultiArrayConstView Variables::
inactive_discrete_real_variable_labels() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_real_labels(
      variablesRep->idrvStart, variablesRep->numIDRV) :
    sharedVarsData.all_discrete_real_labels(idrvStart, numIDRV);
}


inline void Variables::
inactive_discrete_real_variable_labels(StringMultiArrayConstView idrv_labels)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_discrete_real_labels(idrv_labels,
      variablesRep->idrvStart, variablesRep->numIDRV);
  else
    sharedVarsData.all_discrete_real_labels(idrv_labels, idrvStart, numIDRV);
}


inline UShortMultiArrayConstView Variables::
inactive_continuous_variable_types() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_continuous_types(
      variablesRep->icvStart, variablesRep->numICV) :
    sharedVarsData.all_continuous_types(icvStart, numICV);
}


inline UShortMultiArrayConstView Variables::
inactive_discrete_int_variable_types() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_int_types(
      variablesRep->idivStart, variablesRep->numIDIV) :
    sharedVarsData.all_discrete_int_types(idivStart, numIDIV);
}


inline UShortMultiArrayConstView Variables::
inactive_discrete_real_variable_types() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_real_types(
      variablesRep->idrvStart, variablesRep->numIDRV) :
    sharedVarsData.all_discrete_real_types(idrvStart, numIDRV);
}


inline SizetMultiArrayConstView Variables::
inactive_continuous_variable_ids() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_continuous_ids(
      variablesRep->icvStart, variablesRep->numICV) :
    sharedVarsData.all_continuous_ids(icvStart, numICV);
}


inline const RealVector& Variables::all_continuous_variables() const
{ return (variablesRep) ? variablesRep->allContinuousVars : allContinuousVars; }


inline void Variables::all_continuous_variables(const RealVector& ac_vars)
{
  if (variablesRep)
    variablesRep->allContinuousVars.assign(ac_vars);
  else
    allContinuousVars.assign(ac_vars);
}


inline void Variables::
all_continuous_variable(const Real& ac_var, size_t index)
{
  if (variablesRep)
    variablesRep->allContinuousVars[index] = ac_var;
  else
    allContinuousVars[index] = ac_var;
}


inline const IntVector& Variables::all_discrete_int_variables() const
{
  return (variablesRep) ? variablesRep->allDiscreteIntVars
                        : allDiscreteIntVars;
}


inline void Variables::all_discrete_int_variables(const IntVector& adi_vars)
{
  if (variablesRep)
    variablesRep->allDiscreteIntVars.assign(adi_vars);
  else
    allDiscreteIntVars.assign(adi_vars);
}


inline void Variables::all_discrete_int_variable(int adi_var, size_t index)
{
  if (variablesRep)
    variablesRep->allDiscreteIntVars[index] = adi_var;
  else
    allDiscreteIntVars[index] = adi_var;
}


inline const RealVector& Variables::all_discrete_real_variables() const
{
  return (variablesRep) ? variablesRep->allDiscreteRealVars
                        : allDiscreteRealVars;
}


inline void Variables::all_discrete_real_variables(const RealVector& adr_vars)
{
  if (variablesRep)
    variablesRep->allDiscreteRealVars.assign(adr_vars);
  else
    allDiscreteRealVars.assign(adr_vars);
}


inline void Variables::
all_discrete_real_variable(const Real& adr_var, size_t index)
{
  if (variablesRep)
    variablesRep->allDiscreteRealVars[index] = adr_var;
  else
    allDiscreteRealVars[index] = adr_var;
}


inline StringMultiArrayView Variables::all_continuous_variable_labels() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_continuous_labels(0, acv()) :
    sharedVarsData.all_continuous_labels(0, acv());
}


inline void Variables::
all_continuous_variable_labels(StringMultiArrayConstView acv_labels)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_continuous_labels(acv_labels, 0, acv());
  else
    sharedVarsData.all_continuous_labels(acv_labels, 0, acv());
}


inline void Variables::
all_continuous_variable_label(const String& acv_label, size_t index)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_continuous_label(acv_label, index);
  else
    sharedVarsData.all_continuous_label(acv_label, index);
}


inline StringMultiArrayView Variables::all_discrete_int_variable_labels() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_int_labels(0, adiv()) :
    sharedVarsData.all_discrete_int_labels(0, adiv());
}


inline void Variables::
all_discrete_int_variable_labels(StringMultiArrayConstView adiv_labels)
{
  if (variablesRep)
    variablesRep->
      sharedVarsData.all_discrete_int_labels(adiv_labels, 0, adiv());
  else
    sharedVarsData.all_discrete_int_labels(adiv_labels, 0, adiv());
}


inline void Variables::
all_discrete_int_variable_label(const String& adiv_label, size_t index)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_discrete_int_label(adiv_label, index);
  else
    sharedVarsData.all_discrete_int_label(adiv_label, index);
}


inline StringMultiArrayView Variables::all_discrete_real_variable_labels() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_real_labels(0, adrv()) :
    sharedVarsData.all_discrete_real_labels(0, adrv());
}


inline void Variables::
all_discrete_real_variable_labels(StringMultiArrayConstView adrv_labels)
{
  if (variablesRep)
    variablesRep->
      sharedVarsData.all_discrete_real_labels(adrv_labels, 0, adrv());
  else
    sharedVarsData.all_discrete_real_labels(adrv_labels, 0, adrv());
}


inline void Variables::
all_discrete_real_variable_label(const String& adrv_label, size_t index)
{
  if (variablesRep)
    variablesRep->sharedVarsData.all_discrete_real_label(adrv_label, index);
  else
    sharedVarsData.all_discrete_real_label(adrv_label, index);
}


inline UShortMultiArrayConstView Variables::
all_continuous_variable_types() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_continuous_types(0, acv()) :
    sharedVarsData.all_continuous_types(0, acv());
}


inline UShortMultiArrayConstView Variables::
all_discrete_int_variable_types() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_int_types(0, adiv()) :
    sharedVarsData.all_discrete_int_types(0, adiv());
}


inline UShortMultiArrayConstView Variables::
all_discrete_real_variable_types() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_discrete_real_types(0, adrv()) :
    sharedVarsData.all_discrete_real_types(0, adrv());
}


inline SizetMultiArrayConstView Variables::all_continuous_variable_ids() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.all_continuous_ids(0, acv()) :
    sharedVarsData.all_continuous_ids(0, acv());
}


inline const std::pair<short,short>& Variables::view() const
{
  return (variablesRep) ? variablesRep->sharedVarsData.view() :
    sharedVarsData.view();
}


inline const String& Variables::variables_id() const
{
  return (variablesRep) ? variablesRep->sharedVarsData.id() :
    sharedVarsData.id();
}


inline const SizetArray& Variables::variables_components_totals() const
{
  return (variablesRep) ?
    variablesRep->sharedVarsData.components_totals() :
    sharedVarsData.components_totals();
}


inline bool Variables::is_null() const
{ return (variablesRep) ? false : true; }


inline void Variables::build_views()
{
  // called only from letters
  const std::pair<short,short>& view = sharedVarsData.view();
  if (view.first)  //!= EMPTY)
    build_active_views();
  if (view.second) //!= EMPTY)
    build_inactive_views();
}


/// global comparison function for Variables
inline bool variables_id_compare(const Variables& vars, const void* id)
{ return ( *(const String*)id == vars.variables_id() ); }


// Having overloaded operators call read/write means that the operators need 
// not be a friend to private data because read/write functions are public.

/// std::istream extraction operator for Variables.
inline std::istream& operator>>(std::istream& s, Variables& vars)
{ vars.read(s); return s; }


/// std::ostream insertion operator for Variables.
inline std::ostream& operator<<(std::ostream& s, const Variables& vars)
{ vars.write(s); return s; }


/// BiStream extraction operator for Variables.
inline BiStream& operator>>(BiStream& s, Variables& vars)
{ vars.read(s); return s; }


/// BoStream insertion operator for Variables.
inline BoStream& operator<<(BoStream& s, const Variables& vars)
{ vars.write(s); return s; }


/// MPIUnpackBuffer extraction operator for Variables.
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, Variables& vars)
{ vars.read(s); return s; }


/// MPIPackBuffer insertion operator for Variables.
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const Variables& vars)
{ vars.write(s); return s; }

/// inequality operator for Variables
inline bool operator!=(const Variables& vars1, const Variables& vars2)
{ return !(vars1 == vars2); }

} // namespace Dakota

#endif
