/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Variables
//- Description: Base class for variables
//- Owner:       Mike Eldred
//- Version: $Id: DakotaVariables.H 7037 2010-10-23 01:18:08Z mseldre $

#ifndef DAKOTA_VARIABLES_H
#define DAKOTA_VARIABLES_H

#include "data_types.h"
#include "global_defs.h"  // for BaseConstructor
#include "SharedVariablesData.H"


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
  /// return sharedVarsData
  SharedVariablesData& shared_data();

  // ACTIVE VARIABLES

  /// return an active continuous variable
  Real continuous_variable(size_t index) const;
  /// return the active continuous variables (Note: returns a view by const
  /// reference, but initializing a RealVector from this reference invokes
  /// the Teuchos matrix copy constructor to create a Teuchos::Copy instance;
  /// to obtain a mutable view, use continuous_variables_view())
  const RealVector& continuous_variables() const;
  /// set an active continuous variable
  void continuous_variable(Real c_var, size_t index);
  /// set the active continuous variables
  void continuous_variables(const RealVector& c_vars);
  /// return an active discrete integer variable
  int discrete_int_variable(size_t index) const;
  /// return the active discrete integer variables (Note: returns a view by
  /// const reference, but initializing an IntVector from this reference
  /// invokes the Teuchos matrix copy constructor to create a Teuchos::Copy
  /// instance; to obtain a mutable view, use discrete_int_variables_view())
  const IntVector& discrete_int_variables() const;
  /// set an active discrete integer variable
  void discrete_int_variable(int di_var, size_t index);
  /// set the active discrete integer variables
  void discrete_int_variables(const IntVector& di_vars);
  /// return an active discrete real variable
  Real discrete_real_variable(size_t index) const;
  /// return the active discrete real variables (Note: returns a view by
  /// const reference, but initializing a RealVector from this reference
  /// invokes the Teuchos matrix copy constructor to create a Teuchos::Copy
  /// instance; to obtain a mutable view, use discrete_real_variables_view())
  const RealVector& discrete_real_variables() const;
  /// set an active discrete real variable
  void discrete_real_variable(Real dr_var, size_t index);
  /// set the active discrete real variables
  void discrete_real_variables(const RealVector& dr_vars);

  /// copy the active cv/div/drv variables from vars
  void active_variables(const Variables& vars);

  /// return a mutable view of the active continuous variables
  RealVector continuous_variables_view() const;
  /// return a mutable view of the active discrete integer variables
  IntVector discrete_int_variables_view() const;
  /// return a mutable view of the active discrete real variables
  RealVector discrete_real_variables_view() const;

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

  /// returns the set of discrete variable ids relaxed into a continuous array
  const SizetArray& relaxed_discrete_ids() const;

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
  void all_continuous_variable(Real ac_var, size_t index);
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
  void all_discrete_real_variable(Real adr_var, size_t index);

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

  /// infer domain from method selection
  short method_map(short view_spec, bool relaxed) const;
  /// infer domain from method selection
  short method_domain(const ProblemDescDB& problem_db) const;
  /// infer view from method selection
  short method_view(const ProblemDescDB& problem_db) const;
  /// infer view from type of response data set
  short response_view(const ProblemDescDB& problem_db) const;

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


inline const SharedVariablesData& Variables::shared_data() const
{ return (variablesRep) ? variablesRep->sharedVarsData : sharedVarsData; }


inline SharedVariablesData& Variables::shared_data()
{ return (variablesRep) ? variablesRep->sharedVarsData : sharedVarsData; }


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
{ return shared_data().cv(); }


inline size_t Variables::cv_start() const
{ return shared_data().cv_start(); }


inline size_t Variables::div() const
{ return shared_data().div(); }


inline size_t Variables::div_start() const
{ return shared_data().div_start(); }


inline size_t Variables::drv() const
{ return shared_data().drv(); }


inline size_t Variables::drv_start() const
{ return shared_data().drv_start(); }


inline size_t Variables::icv() const
{ return shared_data().icv(); }


inline size_t Variables::icv_start() const
{ return shared_data().icv_start(); }


inline size_t Variables::idiv() const
{ return shared_data().idiv(); }


inline size_t Variables::idiv_start() const
{ return shared_data().idiv_start(); }


inline size_t Variables::idrv() const
{ return shared_data().idrv(); }


inline size_t Variables::idrv_start() const
{ return shared_data().idrv_start(); }


inline size_t Variables::acv() const
{
  return (variablesRep) ? variablesRep->allContinuousVars.length() :
    allContinuousVars.length();
}


inline size_t Variables::adiv() const
{
  return (variablesRep) ? variablesRep->allDiscreteIntVars.length() :
    allDiscreteIntVars.length();
}


inline size_t Variables::adrv() const
{
  return (variablesRep) ? variablesRep->allDiscreteRealVars.length() :
    allDiscreteRealVars.length();
}


inline void Variables::continuous_variable(Real c_var, size_t index)
{
  if (variablesRep) variablesRep->continuousVars[index] = c_var;
  else              continuousVars[index] = c_var;
}


inline Real Variables::continuous_variable(size_t index) const
{
  if (variablesRep) return variablesRep->continuousVars[index];
  else              return continuousVars[index];
}


inline const RealVector& Variables::continuous_variables() const
{ return (variablesRep) ? variablesRep->continuousVars : continuousVars; }


inline void Variables::
continuous_variables(const RealVector& c_vars)
{
  if (variablesRep) variablesRep->continuousVars.assign(c_vars);
  else              continuousVars.assign(c_vars);
}


inline void Variables::discrete_int_variable(int di_var, size_t index)
{
  if (variablesRep) variablesRep->discreteIntVars[index] = di_var;
  else              discreteIntVars[index] = di_var;
}


inline int Variables::discrete_int_variable(size_t index) const
{
  if (variablesRep) return variablesRep->discreteIntVars[index];
  else              return discreteIntVars[index];
}


inline const IntVector& Variables::discrete_int_variables() const
{ return (variablesRep) ? variablesRep->discreteIntVars : discreteIntVars; }


inline void Variables::discrete_int_variables(const IntVector& di_vars)
{
  if (variablesRep) variablesRep->discreteIntVars.assign(di_vars);
  else              discreteIntVars.assign(di_vars);
}


inline void Variables::
discrete_real_variable(Real dr_var, size_t index)
{
  if (variablesRep) variablesRep->discreteRealVars[index] = dr_var;
  else              discreteRealVars[index] = dr_var;
}


inline Real Variables::discrete_real_variable(size_t index) const
{
  if (variablesRep) return variablesRep->discreteRealVars[index];
  else              return discreteRealVars[index];
}


inline const RealVector& Variables::discrete_real_variables() const
{ return (variablesRep) ? variablesRep->discreteRealVars : discreteRealVars; }


inline void Variables::discrete_real_variables(const RealVector& dr_vars)
{
  if (variablesRep) variablesRep->discreteRealVars.assign(dr_vars);
  else              discreteRealVars.assign(dr_vars);
}


inline void Variables::active_variables(const Variables& vars) 
{
  // Set active variables only, leaving remainder of data unchanged (e.g.,
  // so that inactive vars can vary between iterators/models w/i a strategy).
  if (variablesRep)
    variablesRep->active_variables(vars);
  else {
    if (vars.cv())  continuous_variables(vars.continuous_variables());
    if (vars.div()) discrete_int_variables(vars.discrete_int_variables());
    if (vars.drv()) discrete_real_variables(vars.discrete_real_variables());
  }
}


inline RealVector Variables::continuous_variables_view() const
{
  return (variablesRep) ?
    RealVector(Teuchos::View, variablesRep->continuousVars.values(),
	       variablesRep->continuousVars.length()) :
    RealVector(Teuchos::View, continuousVars.values(), continuousVars.length());
}


inline IntVector Variables::discrete_int_variables_view() const
{
  return (variablesRep) ?
    IntVector(Teuchos::View, variablesRep->discreteIntVars.values(),
	      variablesRep->discreteIntVars.length()) :
    IntVector(Teuchos::View, discreteIntVars.values(),
	      discreteIntVars.length());
}


inline RealVector Variables::discrete_real_variables_view() const
{
  return (variablesRep) ?
    RealVector(Teuchos::View, variablesRep->discreteRealVars.values(),
	       variablesRep->discreteRealVars.length()) :
    RealVector(Teuchos::View, discreteRealVars.values(),
	       discreteRealVars.length());
}


inline StringMultiArrayConstView Variables::continuous_variable_labels() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_continuous_labels(svd.cv_start(), svd.cv());
}


inline void Variables::
continuous_variable_labels(StringMultiArrayConstView cv_labels)
{
  SharedVariablesData& svd = shared_data();
  svd.all_continuous_labels(cv_labels, svd.cv_start(), svd.cv());
}


inline void Variables::
continuous_variable_label(const String& cv_label, size_t index)
{
  SharedVariablesData& svd = shared_data();
  svd.all_continuous_label(cv_label, svd.cv_start()+index);
}


inline StringMultiArrayConstView Variables::discrete_int_variable_labels() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_discrete_int_labels(svd.div_start(), svd.div());
}


inline void Variables::
discrete_int_variable_labels(StringMultiArrayConstView div_labels)
{
  SharedVariablesData& svd = shared_data();
  svd.all_discrete_int_labels(div_labels, svd.div_start(), svd.div());
}


inline void Variables::
discrete_int_variable_label(const String& div_label, size_t index)
{
  SharedVariablesData& svd = shared_data();
  svd.all_discrete_int_label(div_label, svd.div_start()+index);
}


inline StringMultiArrayConstView Variables::
discrete_real_variable_labels() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_discrete_real_labels(svd.drv_start(), svd.drv());
}


inline void Variables::
discrete_real_variable_labels(StringMultiArrayConstView drv_labels)
{
  SharedVariablesData& svd = shared_data();
  svd.all_discrete_real_labels(drv_labels, svd.drv_start(), svd.drv());
}


inline void Variables::
discrete_real_variable_label(const String& drv_label, size_t index)
{
  SharedVariablesData& svd = shared_data();
  svd.all_discrete_real_label(drv_label, svd.drv_start()+index);
}


inline UShortMultiArrayConstView Variables::continuous_variable_types() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_continuous_types(svd.cv_start(), svd.cv());
}


inline UShortMultiArrayConstView Variables::discrete_int_variable_types() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_discrete_int_types(svd.div_start(), svd.div());
}


inline UShortMultiArrayConstView Variables::discrete_real_variable_types() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_discrete_real_types(svd.drv_start(), svd.drv());
}


inline SizetMultiArrayConstView Variables::continuous_variable_ids() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_continuous_ids(svd.cv_start(), svd.cv());
}


inline const SizetArray& Variables::relaxed_discrete_ids() const
{ return shared_data().relaxed_discrete_ids(); }


inline const RealVector& Variables::inactive_continuous_variables() const
{
  return (variablesRep) ? variablesRep->inactiveContinuousVars :
    inactiveContinuousVars;
}


inline void Variables::
inactive_continuous_variables(const RealVector& ic_vars)
{
  if (variablesRep) variablesRep->inactiveContinuousVars.assign(ic_vars);
  else              inactiveContinuousVars.assign(ic_vars);
}


inline const IntVector& Variables::inactive_discrete_int_variables() const
{
  return (variablesRep) ? variablesRep->inactiveDiscreteIntVars :
    inactiveDiscreteIntVars;
}


inline void Variables::
inactive_discrete_int_variables(const IntVector& idi_vars)
{
  if (variablesRep) variablesRep->inactiveDiscreteIntVars.assign(idi_vars);
  else              inactiveDiscreteIntVars.assign(idi_vars);
}


inline const RealVector& Variables::inactive_discrete_real_variables() const
{
  return (variablesRep) ? variablesRep->inactiveDiscreteRealVars :
    inactiveDiscreteRealVars;
}


inline void Variables::
inactive_discrete_real_variables(const RealVector& idr_vars)
{
  if (variablesRep) variablesRep->inactiveDiscreteRealVars.assign(idr_vars);
  else              inactiveDiscreteRealVars.assign(idr_vars);
}


inline StringMultiArrayConstView Variables::
inactive_continuous_variable_labels() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_continuous_labels(svd.icv_start(), svd.icv());
}


inline void Variables::
inactive_continuous_variable_labels(StringMultiArrayConstView icv_labels)
{
  SharedVariablesData& svd = shared_data();
  svd.all_continuous_labels(icv_labels, svd.icv_start(), svd.icv());
}


inline StringMultiArrayConstView Variables::
inactive_discrete_int_variable_labels() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_discrete_int_labels(svd.idiv_start(), svd.idiv());
}


inline void Variables::
inactive_discrete_int_variable_labels(StringMultiArrayConstView idiv_labels)
{
  SharedVariablesData& svd = shared_data();
  svd.all_discrete_int_labels(idiv_labels, svd.idiv_start(), svd.idiv());
}


inline StringMultiArrayConstView Variables::
inactive_discrete_real_variable_labels() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_discrete_real_labels(svd.idrv_start(), svd.idrv());
}


inline void Variables::
inactive_discrete_real_variable_labels(StringMultiArrayConstView idrv_labels)
{
  SharedVariablesData& svd = shared_data();
  svd.all_discrete_real_labels(idrv_labels, svd.idrv_start(), svd.idrv());
}


inline UShortMultiArrayConstView Variables::
inactive_continuous_variable_types() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_continuous_types(svd.icv_start(), svd.icv());
}


inline UShortMultiArrayConstView Variables::
inactive_discrete_int_variable_types() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_discrete_int_types(svd.idiv_start(), svd.idiv());
}


inline UShortMultiArrayConstView Variables::
inactive_discrete_real_variable_types() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_discrete_real_types(svd.idrv_start(), svd.idrv());
}


inline SizetMultiArrayConstView Variables::
inactive_continuous_variable_ids() const
{
  const SharedVariablesData& svd = shared_data();
  return svd.all_continuous_ids(svd.icv_start(), svd.icv());
}


inline const RealVector& Variables::all_continuous_variables() const
{ return (variablesRep) ? variablesRep->allContinuousVars : allContinuousVars; }


inline void Variables::all_continuous_variables(const RealVector& ac_vars)
{
  if (variablesRep) variablesRep->allContinuousVars.assign(ac_vars);
  else              allContinuousVars.assign(ac_vars);
}


inline void Variables::
all_continuous_variable(Real ac_var, size_t index)
{
  if (variablesRep) variablesRep->allContinuousVars[index] = ac_var;
  else              allContinuousVars[index] = ac_var;
}


inline const IntVector& Variables::all_discrete_int_variables() const
{
  return (variablesRep) ? variablesRep->allDiscreteIntVars
                        : allDiscreteIntVars;
}


inline void Variables::all_discrete_int_variables(const IntVector& adi_vars)
{
  if (variablesRep) variablesRep->allDiscreteIntVars.assign(adi_vars);
  else              allDiscreteIntVars.assign(adi_vars);
}


inline void Variables::all_discrete_int_variable(int adi_var, size_t index)
{
  if (variablesRep) variablesRep->allDiscreteIntVars[index] = adi_var;
  else              allDiscreteIntVars[index] = adi_var;
}


inline const RealVector& Variables::all_discrete_real_variables() const
{
  return (variablesRep) ? variablesRep->allDiscreteRealVars :
    allDiscreteRealVars;
}


inline void Variables::all_discrete_real_variables(const RealVector& adr_vars)
{
  if (variablesRep) variablesRep->allDiscreteRealVars.assign(adr_vars);
  else              allDiscreteRealVars.assign(adr_vars);
}


inline void Variables::
all_discrete_real_variable(Real adr_var, size_t index)
{
  if (variablesRep) variablesRep->allDiscreteRealVars[index] = adr_var;
  else              allDiscreteRealVars[index] = adr_var;
}


inline StringMultiArrayView Variables::all_continuous_variable_labels() const
{ return shared_data().all_continuous_labels(0, acv()); }


inline void Variables::
all_continuous_variable_labels(StringMultiArrayConstView acv_labels)
{ shared_data().all_continuous_labels(acv_labels, 0, acv()); }


inline void Variables::
all_continuous_variable_label(const String& acv_label, size_t index)
{ shared_data().all_continuous_label(acv_label, index); }


inline StringMultiArrayView Variables::all_discrete_int_variable_labels() const
{ return shared_data().all_discrete_int_labels(0, adiv()); }


inline void Variables::
all_discrete_int_variable_labels(StringMultiArrayConstView adiv_labels)
{ shared_data().all_discrete_int_labels(adiv_labels, 0, adiv()); }


inline void Variables::
all_discrete_int_variable_label(const String& adiv_label, size_t index)
{ shared_data().all_discrete_int_label(adiv_label, index); }


inline StringMultiArrayView Variables::all_discrete_real_variable_labels() const
{ return shared_data().all_discrete_real_labels(0, adrv()); }


inline void Variables::
all_discrete_real_variable_labels(StringMultiArrayConstView adrv_labels)
{ shared_data().all_discrete_real_labels(adrv_labels, 0, adrv()); }


inline void Variables::
all_discrete_real_variable_label(const String& adrv_label, size_t index)
{ shared_data().all_discrete_real_label(adrv_label, index); }


inline UShortMultiArrayConstView Variables::
all_continuous_variable_types() const
{ return shared_data().all_continuous_types(0, acv()); }


inline UShortMultiArrayConstView Variables::
all_discrete_int_variable_types() const
{ return shared_data().all_discrete_int_types(0, adiv()); }


inline UShortMultiArrayConstView Variables::
all_discrete_real_variable_types() const
{ return shared_data().all_discrete_real_types(0, adrv()); }


inline SizetMultiArrayConstView Variables::all_continuous_variable_ids() const
{ return shared_data().all_continuous_ids(0, acv()); }


inline const std::pair<short,short>& Variables::view() const
{ return shared_data().view(); }


inline const String& Variables::variables_id() const
{ return shared_data().id(); }


inline const SizetArray& Variables::variables_components_totals() const
{ return shared_data().components_totals(); }


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
