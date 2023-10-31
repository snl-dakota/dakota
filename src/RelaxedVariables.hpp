/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef RELAXED_VARIABLES_H
#define RELAXED_VARIABLES_H

#include "DakotaVariables.hpp"
#include "DataVariables.hpp"


namespace Dakota {

/// Derived class within the Variables hierarchy which employs the
/// relaxation of discrete variables.

/** Derived variables classes take different views of the design,
    uncertain, and state variable types and the continuous and
    discrete domain types.  The RelaxedVariables derived class combines
    continuous and discrete domain types but separates design,
    uncertain, and state variable types.  The
    branch and bound method uses this approach (see
    Variables::get_variables(problem_db)). */

class RelaxedVariables: public Variables
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  RelaxedVariables(const ProblemDescDB& problem_db, const ShortShortPair& view);
  /// lightweight constructor
  RelaxedVariables(const SharedVariablesData& svd);
  /// destructor
  ~RelaxedVariables();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void read(std::istream& s);
  void read_tabular(std::istream& s, unsigned short vars_part = ALL_VARS);

  void write(std::ostream& s, unsigned short vars_part = ALL_VARS) const;
  void write_aprepro(std::ostream& s) const;
  void write_tabular(std::ostream& s,
		     unsigned short vars_part = ALL_VARS) const;
  void write_tabular_partial(std::ostream& s, size_t start_index,
			     size_t num_items) const;

  void write_tabular_labels(std::ostream& s,
                            unsigned short vars_part = ALL_VARS) const;
  void write_tabular_partial_labels(std::ostream& s, size_t start_index,
				    size_t num_items) const;

  /// Implementation of reading various formats using the specified
  /// read handler, accounting for reordering due to relaxation
  template<typename Reader>
  void read_core(std::istream& s, Reader read_handler,
		 unsigned short vars_part);
  /// Implementation of writing various formats using the specified
  /// write handler, accounting for reordering due to relaxation
  template<typename Writer>
  void write_core(std::ostream& s, Writer write_handler,
                  unsigned short vars_part) const;
  /// Implementation for partial writing in various formats using the
  /// specified write handler
  template<typename Writer>
  bool write_partial_core(std::ostream& s, Writer write_handler,
			  size_t start_index, size_t end_index,
			  size_t& acv_offset, size_t& adiv_offset,
			  size_t& adsv_offset, size_t& adrv_offset,
			  size_t& av_cntr, size_t num_cv, size_t num_div,
			  size_t num_dsv, size_t num_drv) const;

private:

  //
  //- Heading: Private data members
  //

};


inline RelaxedVariables::RelaxedVariables(const SharedVariablesData& svd):
  Variables(BaseConstructor(), svd)
{ }


inline RelaxedVariables::~RelaxedVariables()
{ }

} // namespace Dakota

#endif
